// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	None,
	Light,
	Heavy
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBATPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Settings")
	TArray<UAnimMontage*> LightAttackMontages;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Settings")
	TArray<UAnimMontage*> HeavyAttackMontages;

	UPROPERTY(EditAnywhere, Category="Settings")
	float ComboResetTime = 0.6f;

	UPROPERTY(VisibleAnywhere, Category="Combo|State")
	bool bIsAttacking = false;
	
	UPROPERTY(VisibleAnywhere, Category="Combo|State")
	bool bComboWindowOpen = false;
	
	UPROPERTY(VisibleAnywhere, Category="Combo|State")
	EAttackType QueuedAttack = EAttackType::None;
	
	UPROPERTY(VisibleAnywhere, Category="Combo|State")
	bool bHasQueuedAttack = false;
	
	UPROPERTY(VisibleAnywhere, Category="Combo|State")
	int32 LightIndex = 0;
	
	UPROPERTY(VisibleAnywhere, Category="Combo|State")
	int32 HeavyIndex = 0;

	FTimerHandle ComboResetHandle;

	void TryAttack(EAttackType Type);
	void PlayNextMontage(EAttackType Type);
	UAnimMontage* GetNextMontage(EAttackType Type);
	void AdvanceIndex(EAttackType Type);
	void ResetCombo();


public:

	// Call from input
	UFUNCTION(BlueprintCallable)
	void LightAttack();
	
	UFUNCTION(BlueprintCallable)
	void HeavyAttack();
	
	UFUNCTION(BlueprintCallable)
	void CancelCombo();

	// Called by the AnimNotifyState
	UFUNCTION(BlueprintCallable)
	void OpenComboWindow();
	
	UFUNCTION(BlueprintCallable)
	void CloseComboWindow();

	// Optional stamina or cooldown check hook
	UFUNCTION(BlueprintCallable)
	bool CanAttack(EAttackType Type) const;

	UFUNCTION(BlueprintCallable)
	void OnMontageEnded();

};
