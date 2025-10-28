// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CombatPlayerAnimInstance.generated.h"

class ACombatProjectCharacter;
/**
 * 
 */
UCLASS()
class COMBATPROJECT_API UCombatPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACombatProjectCharacter* CharacterRef;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CurrentDirection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CurrentVelocity{ 0.0f };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInCombat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrouching;
	
	UFUNCTION(BlueprintCallable)
	void HandleUpdatedTarget(AActor* NewTargetActorRef);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;	
};
	