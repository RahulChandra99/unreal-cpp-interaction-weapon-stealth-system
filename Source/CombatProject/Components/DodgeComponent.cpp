// Fill out your copyright notice in the Description page of Project Settings.


#include "DodgeComponent.h"

#include "CombatProject/CombatPlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "UniversalObjectLocators/AnimInstanceLocatorFragment.h"


// Sets default values for this component's properties
UDodgeComponent::UDodgeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDodgeComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerRef = Cast<ACharacter>(GetOwner());
	if (OwnerRef && OwnerRef->GetMesh())
	{
		AnimInstanceRef = Cast<UCombatPlayerAnimInstance>(OwnerRef->GetMesh()->GetAnimInstance());
	}

	
}


// Called every frame
void UDodgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDodgeComponent::Dodge()
{
	if (!AnimInstanceRef) return;

	FVector Input = OwnerRef->GetLastMovementInputVector();
	if (Input.IsNearlyZero()) return; 
    
	Input.Normalize();

	const FVector Forward = OwnerRef->GetActorForwardVector();
	float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, Input)));
	float CrossZ = FVector::CrossProduct(Forward, Input).Z;

	// Adjust angle sign based on cross product
	if (CrossZ < 0)
	{
		Angle = -Angle;
	}

	EDodgeDirection DodgeDirection = EDodgeDirection::Forward;

	if (Angle >= -22.5f && Angle < 22.5f) 
		DodgeDirection = EDodgeDirection::Forward;
	else if (Angle >= 22.5f && Angle < 67.5f) 
		DodgeDirection = EDodgeDirection::ForwardRight;
	else if (Angle >= 67.5f && Angle < 112.5f) 
		DodgeDirection = EDodgeDirection::Right;
	else if (Angle >= 112.5f && Angle < 157.5f) 
		DodgeDirection = EDodgeDirection::BackwardRight;
	else if (Angle >= 157.5f || Angle < -157.5f) 
		DodgeDirection = EDodgeDirection::Backward;
	else if (Angle >= -157.5f && Angle < -112.5f) 
		DodgeDirection = EDodgeDirection::BackwardLeft;
	else if (Angle >= -112.5f && Angle < -67.5f) 
		DodgeDirection = EDodgeDirection::Left;
	else if (Angle >= -67.5f && Angle < -22.5f) 
		DodgeDirection = EDodgeDirection::ForwardLeft;

	PlayDodge(DodgeDirection);
}

void UDodgeComponent::PlayDodge(EDodgeDirection Direction)
{
	UAnimMontage* MontageToPlay = nullptr;

	switch (Direction)
	{
	case EDodgeDirection::Forward:        MontageToPlay = RollForwardMontage; break;
	case EDodgeDirection::ForwardRight:   MontageToPlay = RollForwardRightMontage; break;
	case EDodgeDirection::Right:          MontageToPlay = RollRightMontage; break;
	case EDodgeDirection::BackwardRight:  MontageToPlay = RollBackwardRightMontage; break;
	case EDodgeDirection::Backward:       MontageToPlay = RollBackwardMontage; break;
	case EDodgeDirection::BackwardLeft:   MontageToPlay = RollBackwardLeftMontage; break;
	case EDodgeDirection::Left:           MontageToPlay = RollLeftMontage; break;
	case EDodgeDirection::ForwardLeft:    MontageToPlay = RollForwardLeftMontage; break;
	}

	if (MontageToPlay && AnimInstanceRef && !AnimInstanceRef->Montage_IsPlaying(MontageToPlay))
	{
		bIsDodging = true;
		AnimInstanceRef->Montage_Play(MontageToPlay, 1.0f);
	}

}

