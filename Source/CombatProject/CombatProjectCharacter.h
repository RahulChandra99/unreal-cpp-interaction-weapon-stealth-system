// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "CombatProjectCharacter.generated.h"

class AProjectileBase;
class UPhysicsConstraintComponent;
class UCombatPlayerAnimInstance;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACombatProjectCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	ACombatProjectCharacter();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bIsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	class UBasicAttributeSet* BasicAttributeSet;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aiming")
	float AimDirection;
	
	UPROPERTY()
	TMap<FName, int32> SavedCurrentAmmoMap;
    
	UPROPERTY()
	TMap<FName, int32> SavedReserveAmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XLookSens = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YLookSens = 0.7f;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatPlayerAnimInstance* PlayerAnimInstance;

	UPROPERTY(BlueprintReadWrite)
	bool bIsWheelOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintImplementableEvent)
	void CalculateAimDirection();

	// Grab input
	UFUNCTION(BlueprintCallable)
	void GrabEnemy(AEnemy* EnemyRef);

	UFUNCTION(BlueprintCallable)
	void ReleaseEnemy();

	// Helper
	void UpdateGrabbedLocation(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grab")
	class UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY(EditAnywhere, Category="Grab")
	float HoldDistance = 150.f;

	UPROPERTY(EditAnywhere, Category="Grab")
	FName GrabBoneName = "pelvis";

	USkeletalMeshComponent* GrabbedMesh = nullptr;



	


protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable)
	bool IsEnemyBehindTarget(AActor* TargetActor);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void DestoryGun(class AWeapon* EquippedWeapon);

	UFUNCTION(BlueprintCallable)
	void SpawnWeapon( AWeapon* EquippedWeapon);

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ThrowSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ThrowScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGrenadeAiming;
	
	UFUNCTION()
	void PredictGrenadePath();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenade(TSubclassOf<AProjectileBase> GrenadeActor);

	UFUNCTION(BlueprintCallable)
	void ActivateSenseVision();

	
};

