#include "ABaseEnemyCharacter.h"
#include "AttributesComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimInstance.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h" 
#include "WorldCollision.h"
#include "CollisionQueryParams.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

AABaseEnemyCharacter::AABaseEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    Attributes = CreateDefaultSubobject<UAttributesComponent>(TEXT("Attributes"));

    MeleeHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
    if (MeleeHitBox)
    {

        MeleeHitBox->SetupAttachment(GetMesh());
        MeleeHitBox->SetBoxExtent(FVector(12.f, 30.f, 8.f));
        MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        MeleeHitBox->SetCollisionObjectType(ECC_WorldDynamic);
        MeleeHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
        MeleeHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        MeleeHitBox->SetGenerateOverlapEvents(false);

        MeleeHitBox->ComponentTags.Add(FName("MeleeHitBox"));
    }
}

void AABaseEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (Attributes)
    {
        Attributes->OnDeath.AddDynamic(this, &AABaseEnemyCharacter::OnDeath);
    }

    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    PawnState = EPawnState::Idle;

    if (MeleeHitBox && GetMesh())
    {
        const FName SocketName = FName("Hand_L_Socket");
        MeleeHitBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
        MeleeHitBox->SetRelativeLocation(FVector::ZeroVector);
        MeleeHitBox->SetRelativeRotation(FRotator::ZeroRotator);

        MeleeHitBox->OnComponentBeginOverlap.AddDynamic(this, &AABaseEnemyCharacter::OnMeleeHitboxBeginOverlap);

        UE_LOG(LogTemp, Log, TEXT("%s: MeleeHitBox attached to socket %s"), *GetName(), *SocketName.ToString());
    }
}

void AABaseEnemyCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    CheckForPlayer();
}

void AABaseEnemyCharacter::SetPawnState(EPawnState NewState)
{
    PawnState = NewState;
    if (AEnemyAIController* C = Cast<AEnemyAIController>(GetController()))
    {
        if (C->GetBlackboardComp())
        {
            C->GetBlackboardComp()->SetValueAsEnum(C->BB_PawnState, (uint8)PawnState);
        }
    }
}

void AABaseEnemyCharacter::CheckForPlayer()
{
    if (!PlayerPawn) return;

    const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
    const float Dist = ToPlayer.Size();

    if (Dist <= SightRadius)
    {
        if (PawnState == EPawnState::Idle)
        {
            PawnState = EPawnState::InCombat;
        }
    }
    else
    {
        PawnState = EPawnState::Idle;
        StopAttack();
        return;
    }

    if (PawnState == EPawnState::InCombat)
    {
        if (Dist <= AttackRange)
        {
            if (!GetWorldTimerManager().IsTimerActive(AttackTimerHandle))
            {
                StartAttack();
                GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AABaseEnemyCharacter::StartAttack, AttackRate, true);
            }
        }
        else
        {
            StopAttack();
        }
    }
}

void AABaseEnemyCharacter::StartAttack()
{
    if (PawnState == EPawnState::Hit) return;
    if (AttackMontage)
    {
        if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
        {
            AnimInst->Montage_Play(AttackMontage);
        }
    }

    if (bUseAnimNotifyToDealDamage)
    {
        UE_LOG(LogTemp, Log, TEXT("%s start attack (waiting for anim notify)"), *GetName());
        return;
    }

    const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 40.f);
    const FVector Forward = GetActorForwardVector();
    const FVector End = Start + Forward * AttackRange;

    AlreadyHitActors.Empty();

    DoMeleeDamage(Start, End);

    UE_LOG(LogTemp, Log, TEXT("%s starts attack (performed immediate melee sweep)"), *GetName());
}

void AABaseEnemyCharacter::StopAttack()
{
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);
}

void AABaseEnemyCharacter::HandleAttackNotify()
{
    const FName HandSocketName = TEXT("Hand_L_Socket");
    const FTransform SocketTransform = GetMesh()->GetSocketTransform(HandSocketName, RTS_World);

    const FVector Start = SocketTransform.GetLocation();
    const FQuat Rotation = SocketTransform.GetRotation();
    const FVector Forward = Rotation.GetForwardVector();
    const FVector End = Start + Forward * AttackRange;

    AlreadyHitActors.Empty();
    DoMeleeDamage(Start, End, Rotation);
}

void AABaseEnemyCharacter::DoMeleeDamage(const FVector& Start, const FVector& End, const FQuat& Rotation)
{
    if (!GetWorld()) return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.bTraceComplex = false;

    FCollisionShape BoxShape = FCollisionShape::MakeBox(MeleeBoxHalfExtent);

    const FVector Center = (Start + End) * 0.5f;

    TArray<FOverlapResult> Overlaps;
    bool bAny = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Center,
        Rotation,
        ECC_Pawn,
        BoxShape,
        Params
    );

    DrawDebugBox(GetWorld(), Center, BoxShape.GetBox(), Rotation, bAny ? FColor::Red : FColor::Green, false, 1.5f, 0, 2.0f);
    DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1.5f, 0, 1.0f);

    if (!bAny) return;

    for (const FOverlapResult& R : Overlaps)
    {
        AActor* Other = R.GetActor();
        if (!Other) continue;
        if (Other == this) continue;
        if (AlreadyHitActors.Contains(Other)) continue;

        AlreadyHitActors.Add(Other);

        const FVector ImpactPoint = Other->GetActorLocation();

        if (Other->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
        {
            ICombatInterface::Execute_GetHit(Other, this, AttackDamage, ImpactPoint);
        }
        else
        {
            UGameplayStatics::ApplyDamage(Other, AttackDamage, GetController(), this, nullptr);
        }

        UE_LOG(LogTemp, Log, TEXT("%s applied %f damage to %s"), *GetNameSafe(this), AttackDamage, *GetNameSafe(Other));
    }
}



void AABaseEnemyCharacter::GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint)
{
    UE_LOG(LogTemp, Log, TEXT("%s received GetHit Damage=%f from %s"), *GetName(), Damage, *GetNameSafe(InstigatorActor));

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s: AnimInstance is NULL! Mesh name: %s"), *GetName(), *GetMesh()->GetName());
    }
    else
    {
        if (HitMontage)
        {
            float PlayResult = AnimInst->Montage_Play(HitMontage);
            UE_LOG(LogTemp, Log, TEXT("Attempted Montage_Play HitMontage on %s. PlayResult = %f"), *GetName(), PlayResult);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy %s: HitMontage not assigned!"), *GetName());
        }
    }

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
    }

    if (Attributes)
    {
        Attributes->ApplyDamage(Damage);
    }

    EnterHitState();
}

void AABaseEnemyCharacter::EnterHitState()
{
    PawnState = EPawnState::Hit;
    StopAttack();

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, [this]()
    {
        if (Attributes && Attributes->GetHealth() > 0.f)
        {
            PawnState = EPawnState::InCombat;
        }
    }, 0.8f, false);
}

void AABaseEnemyCharacter::RotateTowardsActor(AActor* TargetActor)
{
    if (!TargetActor) return;

    FVector Dir = TargetActor->GetActorLocation() - GetActorLocation();
    Dir.Z = 0.f;
    if (Dir.IsNearlyZero()) return;

    FRotator TargetRot = Dir.Rotation();

    TargetRot.Yaw -= YawRotationOffset;
    TargetRot.Yaw = FRotator::NormalizeAxis(TargetRot.Yaw);

    if (AController* C = GetController())
    {
        C->SetControlRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
    }
    else
    {
        SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
    }

    UE_LOG(LogTemp, Log, TEXT("%s rotated instantly to face %s (Yaw=%f, Offset=%f)"),
           *GetName(), *GetNameSafe(TargetActor), TargetRot.Yaw, YawRotationOffset);
}

void AABaseEnemyCharacter::RotateTowardsActorSmooth(AActor* TargetActor, float InterpSpeed /*=0.f*/)
{
    if (InterpSpeed <= 0.f)
    {
        RotateTowardsActor(TargetActor);
        return;
    }

    if (!TargetActor) return;

    FVector Dir = TargetActor->GetActorLocation() - GetActorLocation();
    Dir.Z = 0.f;
    if (Dir.IsNearlyZero()) return;

    FRotator TargetRot = Dir.Rotation();
    TargetRot.Yaw -= YawRotationOffset;
    TargetRot.Yaw = FRotator::NormalizeAxis(TargetRot.Yaw);

    if (AController* C = GetController())
    {
        FRotator Current = C->GetControlRotation();
        FRotator New = FMath::RInterpTo(Current, FRotator(0.f, TargetRot.Yaw, 0.f), GetWorld()->GetDeltaSeconds(), InterpSpeed);
        C->SetControlRotation(New);
    }
    else
    {
        FRotator Current = GetActorRotation();
        FRotator New = FMath::RInterpTo(Current, FRotator(0.f, TargetRot.Yaw, 0.f), GetWorld()->GetDeltaSeconds(), InterpSpeed);
        SetActorRotation(New);
    }

    UE_LOG(LogTemp, Verbose, TEXT("%s smooth-rotating toward %s (targetYaw=%f, offset=%f)"),
           *GetName(), *GetNameSafe(TargetActor), TargetRot.Yaw, YawRotationOffset);
}



void AABaseEnemyCharacter::OnDeath(AActor* OwningActor)
{
    UE_LOG(LogTemp, Warning, TEXT("%s died"), *GetName());
    if (AEnemyAIController* C = Cast<AEnemyAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = C->GetBlackboardComponent())
        {
            BB->SetValueAsEnum("PawnState", (uint8)PawnState);
        }
    }

    Destroy();
}

void AABaseEnemyCharacter::OnMeleeHitboxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    UE_LOG(LogTemp, Log, TEXT("%s MeleeHitbox overlapped with %s (OtherComp=%s)"), *GetNameSafe(this), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));
    UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, nullptr);
    UE_LOG(LogTemp, Log, TEXT("%s MeleeHitbox applied %f damage to %s"), *GetNameSafe(this), AttackDamage, *GetNameSafe(OtherActor));
}

bool AABaseEnemyCharacter::TryAttack()
{
    if (PawnState == EPawnState::Hit || PawnState == EPawnState::Dead) return false;
    if (!Attributes) return false;

    float Cost = Attributes->StaminaCost.StaminaCost_Attack;
    if (!Attributes->CanPayStaminaCost(Cost)) return false;

    Attributes->PayStamina(Cost);

    PawnState = EPawnState::InCombat;
    SetPawnState(PawnState);

    if (AttackMontage && GetMesh())
    {
        if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &AABaseEnemyCharacter::OnAttackMontageEnded);
            AnimInst->Montage_SetEndDelegate(EndDelegate, AttackMontage);

            AnimInst->Montage_Play(AttackMontage);
            return true;
        }
    }
    return false;
}

void AABaseEnemyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Attributes && Attributes->GetHealth() > 0.f)
    {
        PawnState = EPawnState::Idle;
    }

    if (AController* C = GetController())
    {
        AEnemyAIController* AICon = Cast<AEnemyAIController>(C);
        if (AICon && AICon->BlackboardComp)
        {
            AICon->BlackboardComp->SetValueAsEnum(AICon->BB_PawnState, (uint8)PawnState);
        }
    }
}

void AABaseEnemyCharacter::EnableMeleeHitbox()
{
    if (!MeleeHitBox) return;
    MeleeHitBox->SetGenerateOverlapEvents(true);
    MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UE_LOG(LogTemp, Log, TEXT("ANS: Enabled BoxComponent hitbox on owner %s (component %s)"), *GetNameSafe(this), *GetNameSafe(MeleeHitBox));
}

void AABaseEnemyCharacter::DisableMeleeHitbox()
{
    if (!MeleeHitBox) return;
    MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeleeHitBox->SetGenerateOverlapEvents(false);
    UE_LOG(LogTemp, Log, TEXT("ANS: Disabled BoxComponent hitbox on owner %s (component %s)"), *GetNameSafe(this), *GetNameSafe(MeleeHitBox));
}
