// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPlayerAnimInstance.h"

#include "CombatProjectCharacter.h"


void UCombatPlayerAnimInstance::NativeInitializeAnimation()
{

	APawn* PawnRef = TryGetPawnOwner();
	if (IsValid(PawnRef))
	{
		CharacterRef = Cast<ACombatProjectCharacter>(PawnRef);
	}
	
}

void UCombatPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (CharacterRef)
	{
		bIsCrouching = CharacterRef->bIsCrouching;

		FVector Velocity = CharacterRef->GetVelocity();
		CurrentDirection = CalculateDirection(Velocity, CharacterRef->GetActorRotation());
		CurrentVelocity = Velocity.Length();

	}
	
	
}


void UCombatPlayerAnimInstance::HandleUpdatedTarget(AActor* NewTargetActorRef)
{
	bIsInCombat = IsValid(NewTargetActorRef);
}

