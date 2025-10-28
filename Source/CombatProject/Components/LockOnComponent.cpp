#include "LockOnComponent.h"
#include "CombatProject/Interface/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerRef = Cast<ACharacter>(GetOwner());
	if (!OwnerRef) return;

	ControllerRef = Cast<APlayerController>(OwnerRef->GetController());
	MovementComp = OwnerRef->GetCharacterMovement();
	SpringArmCompRef = OwnerRef->FindComponentByClass<USpringArmComponent>();

	if (SpringArmCompRef)
	{
		SpringArmCompRef->bDoCollisionTest = false; // prevent camera pushing upward when close
	}
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentTargetActor)
	{
		CheckAutoUnlock();
		SmoothRotateToTarget(DeltaTime);
		UpdateCameraBehavior(DeltaTime);
	}
}

void ULockOnComponent::SmoothRotateToTarget(float DeltaTime)
{
	if (!ControllerRef || !CurrentTargetActor) return;

	FVector CurrentLocation = OwnerRef->GetActorLocation();
	FVector TargetLocation = CurrentTargetActor->GetActorLocation();

	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
	FRotator Smoothed = FMath::RInterpTo(ControllerRef->GetControlRotation(), TargetRot, DeltaTime, RotationInterpSpeed);
	ControllerRef->SetControlRotation(Smoothed);
}

void ULockOnComponent::UpdateCameraBehavior(float DeltaTime)
{
	if (!SpringArmCompRef || !CurrentTargetActor || !OwnerRef) return;

	// --- Step 1: Adjust height offset (clamped) ---
	float TargetZ = CurrentTargetActor->GetActorLocation().Z;
	float OwnerZ = OwnerRef->GetActorLocation().Z;
	float HeightDiff = FMath::Clamp(TargetZ - OwnerZ, -100.f, 150.f);

	FVector DesiredOffset = FVector(50.f, 0.f, HeightDiff * 0.5f + HeightOffset); // small shoulder bias (X = 50)
	SpringArmCompRef->TargetOffset = FMath::VInterpTo(SpringArmCompRef->TargetOffset, DesiredOffset, DeltaTime, 4.f);

	// --- Step 2: Dynamic zoom based on distance ---
	float Distance = FVector::Dist(OwnerRef->GetActorLocation(), CurrentTargetActor->GetActorLocation());
	float DesiredLength = 400.f; // base camera distance
	if (Distance < 250.f)
	{
		float Alpha = (250.f - Distance) / 250.f;
		DesiredLength = FMath::Lerp(400.f, 300.f, Alpha);
	}
	SpringArmCompRef->TargetArmLength = FMath::FInterpTo(SpringArmCompRef->TargetArmLength, DesiredLength, DeltaTime, 6.f);
}

void ULockOnComponent::CheckAutoUnlock()
{
	if (!CurrentTargetActor || !CurrentTargetActor->IsValidLowLevel())
	{
		EndLockOn();
		return;
	}

	float Distance = FVector::Dist(OwnerRef->GetActorLocation(), CurrentTargetActor->GetActorLocation());
	if (Distance >= BreakDistance)
	{
		EndLockOn();
		return;
	}
}

void ULockOnComponent::ToggleLockOn(float Radius)
{
	if (IsValid(CurrentTargetActor))
		EndLockOn();
	else
		StartLockOn(Radius);
}

void ULockOnComponent::StartLockOn(float Radius)
{
	TArray<AActor*> Candidates;
	GetValidTargets(Candidates, Radius);

	if (Candidates.Num() == 0) return;

	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	for (AActor* Candidate : Candidates)
	{
		FVector ToCandidate = (Candidate->GetActorLocation() - OwnerRef->GetActorLocation()).GetSafeNormal();
		float ForwardDot = FVector::DotProduct(OwnerRef->GetActorForwardVector(), ToCandidate);
		float Distance = FVector::Dist(OwnerRef->GetActorLocation(), Candidate->GetActorLocation());
		float Score = (ForwardDot * 1000.f) - Distance;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	if (BestTarget)
		SetLockOnTarget(BestTarget);
}

void ULockOnComponent::SetLockOnTarget(AActor* NewTarget)
{
	if (!NewTarget) return;

	CurrentTargetActor = NewTarget;

	if (ControllerRef)
		ControllerRef->SetIgnoreLookInput(true);

	if (MovementComp)
	{
		MovementComp->bOrientRotationToMovement = false;
		MovementComp->bUseControllerDesiredRotation = true;
	}

	if (SpringArmCompRef)
	{
		SpringArmCompRef->TargetOffset = FVector(50.f, 0.f, HeightOffset); // slightly offset to right shoulder
	}

	IEnemyInterface::Execute_OnLockonSelect(CurrentTargetActor);
	OnUpdatedTargetDelegate.Broadcast(CurrentTargetActor);
}

void ULockOnComponent::EndLockOn()
{
	if (!CurrentTargetActor) return;

	IEnemyInterface::Execute_OnLockonDeSelect(CurrentTargetActor);
	CurrentTargetActor = nullptr;

	if (ControllerRef)
		ControllerRef->SetIgnoreLookInput(false);

	if (MovementComp)
	{
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->bUseControllerDesiredRotation = false;
	}

	if (SpringArmCompRef)
	{
		SpringArmCompRef->TargetOffset = FVector::ZeroVector;
		SpringArmCompRef->TargetArmLength = 400.f;
	}
}

void ULockOnComponent::GetValidTargets(TArray<AActor*>& OutTargets, float Radius) const
{
	FVector CurrentLocation = OwnerRef->GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params(FName("LockOn"), false, OwnerRef);

	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		CurrentLocation,
		CurrentLocation,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		Sphere,
		Params);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* Candidate = Hit.GetActor();
			if (Candidate && Candidate->Implements<UEnemyInterface>())
				OutTargets.AddUnique(Candidate);
		}
	}
}

void ULockOnComponent::SwitchTarget(bool bRight)
{
	if (!CurrentTargetActor) return;

	TArray<AActor*> Candidates;
	GetValidTargets(Candidates);

	if (Candidates.Num() == 0) return;

	FVector CamRight = ControllerRef->GetControlRotation().Vector().RotateAngleAxis(90.f, FVector::UpVector);
	float BestDot = -1.f;
	AActor* BestTarget = nullptr;

	for (AActor* Candidate : Candidates)
	{
		if (Candidate == CurrentTargetActor) continue;

		FVector ToCandidate = (Candidate->GetActorLocation() - OwnerRef->GetActorLocation()).GetSafeNormal();
		float SideDot = FVector::DotProduct(CamRight, ToCandidate);

		if (bRight && SideDot > BestDot)
		{
			BestDot = SideDot;
			BestTarget = Candidate;
		}
		else if (!bRight && -SideDot > BestDot)
		{
			BestDot = -SideDot;
			BestTarget = Candidate;
		}
	}

	if (BestTarget)
	{
		IEnemyInterface::Execute_OnLockonDeSelect(CurrentTargetActor);
		SetLockOnTarget(BestTarget);
	}
}
