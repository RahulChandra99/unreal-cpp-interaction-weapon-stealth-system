// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DodgeComponent.generated.h"


class UCombatPlayerAnimInstance;

UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	Forward,
	ForwardRight,
	Right,
	BackwardRight,
	Backward,
	BackwardLeft,
	Left,
	ForwardLeft
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBATPROJECT_API UDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDodgeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollForwardMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollForwardRightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollRightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollBackwardRightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollBackwardMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollBackwardLeftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollLeftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	UAnimMontage* RollForwardLeftMontage;

	UPROPERTY()
	ACharacter* OwnerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDodging;
	
	UPROPERTY()
	UCombatPlayerAnimInstance* AnimInstanceRef;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Dodge();

	UFUNCTION()
	void PlayDodge(EDodgeDirection Direction);
};
