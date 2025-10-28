// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"


class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnUpdatedTarget,
	AActor*, NewTargetActorRef
);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBATPROJECT_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULockOnComponent();
	void BeginPlay();
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	// Main interface
	UFUNCTION(BlueprintCallable) void ToggleLockOn(float Radius = 1200.f);
	UFUNCTION(BlueprintCallable) void EndLockOn();
	UFUNCTION(BlueprintCallable) void SwitchTarget(bool bRight);

	UPROPERTY(BlueprintAssignable) FOnUpdatedTarget OnUpdatedTargetDelegate;

protected:
	void StartLockOn(float Radius);
	void SetLockOnTarget(AActor* NewTarget);
	void GetValidTargets(TArray<AActor*>& OutTargets, float Radius = 1200.f) const;
	void SmoothRotateToTarget(float DeltaTime);
	void UpdateCameraBehavior(float DeltaTime);
	void CheckAutoUnlock();

	UPROPERTY(EditAnywhere, Category="LockOn") float BreakDistance = 1800.f;
	UPROPERTY(EditAnywhere, Category="LockOn") float HeightOffset = 100.f;
	UPROPERTY(EditAnywhere, Category="LockOn") float RotationInterpSpeed = 8.f;

	UPROPERTY() ACharacter* OwnerRef = nullptr;
	UPROPERTY() APlayerController* ControllerRef = nullptr;
	UPROPERTY() class UCharacterMovementComponent* MovementComp = nullptr;
	UPROPERTY() class USpringArmComponent* SpringArmCompRef = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="LockOn") AActor* CurrentTargetActor = nullptr;

	
};
