// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h" 
#include "Weapon.generated.h"



class UBoxComponent;

UCLASS()
class COMBATPROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	
	UFUNCTION(BlueprintCallable)
    	void StartAttack();
    
    	UFUNCTION(BlueprintCallable)
    	void EndAttack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void PerformWeaponTrace();
	void PhysicalMaterialBasedResponse(UPhysicalMaterial* PhyMaterial,FHitResult Hit);
	void ApplyHitStop(float Duration, float Dilation);
	void SmoothRestoreTime(float Duration);
	void PhysicalMaterialBasedResponseForGun(UPhysicalMaterial* PhyMaterial,FHitResult Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* BoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* BoxTraceStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* BoxTraceEnd;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsAttacking;

	UPROPERTY()
	TArray<AActor*> ActorsHitThisSwing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* HitEnemySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* HitEnvironmentSoundWood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* HitEnvironmentSoundMetal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* EnemyHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* WoodHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* MetalHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* EnemyHitNiagaraVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* WoodHitNiagaraVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* MetalHitNiagaraVFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* HitDecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> HitCameraShake;
	FTimerHandle AutoFireHandle;

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

	/* Gun Properties */

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shooting")
	int32 MagazineSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	float WeaponRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	USoundBase* EmptyClipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	UParticleSystem* GunWoodHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	UParticleSystem* GunMetalHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* GunEnemyHitNiagaraVFX;

	// Damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float BaseDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float HeadshotMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float LimbMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	// Spread
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Accuracy")
	float SpreadHipDeg = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Accuracy")
	float SpreadADSDeg = 0.5f;

	// Recoil and shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recoil")
	float RecoilPitchMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recoil")
	float RecoilPitchMax = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recoil")
	float RecoilYawMin = -0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recoil")
	float RecoilYawMax = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recoil")
	TSubclassOf<UCameraShakeBase> RecoilCameraShake;

	// Tracer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	UParticleSystem* TracerFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Persistence")
	int32 SavedCurrentAmmo = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Persistence")
	int32 SavedReserveAmmo = -1;

	


	bool bCanShoot = true;
	FTimerHandle FireDelayHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
	bool bIsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
	bool bIsAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
	bool bIsFiring;



public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
	int32 ReserveAmmo;
	
	UFUNCTION(BlueprintImplementableEvent)
	void GetHitActor(AActor* HitActor);

	UFUNCTION(BlueprintCallable)
	void StartFiring(bool bIsAiming);

	UFUNCTION()
	void AutoFire();

	UFUNCTION(BlueprintCallable)
	void StopFiring();

	UFUNCTION(BlueprintCallable)
	void FireWeapon(bool bIsAiming);

	UFUNCTION(BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentAmmo() const {  return CurrentAmmo;}

	UFUNCTION(BlueprintCallable)
	int32 GetMaxAmmo() const {  return ReserveAmmo;}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponID;

	UFUNCTION(BlueprintImplementableEvent)
	void EnemyHit(float DamageDone, AActor* EnemyActorHit,FHitResult Hit);

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void PlayFireEffects();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShotEnemy(bool bIsHeadshot);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayReloadEffects();

	UFUNCTION(BlueprintCallable)
	void HandleReload();

	UFUNCTION(BlueprintCallable)
	void HandleFire(bool bADS);

	UFUNCTION(BlueprintCallable, Category="Recoil")
	void ApplyRecoil();

	static bool IsHeadBone(const FName& Bone)
	{
		const FString N = Bone.ToString();
		return N.Contains(TEXT("head"), ESearchCase::IgnoreCase);
	}

	static bool IsLimbBone(const FName& Bone)
	{
		const FString N = Bone.ToString();
		return N.Contains(TEXT("hand"), ESearchCase::IgnoreCase)
			|| N.Contains(TEXT("arm"),  ESearchCase::IgnoreCase)
			|| N.Contains(TEXT("leg"),  ESearchCase::IgnoreCase)
			|| N.Contains(TEXT("calf"), ESearchCase::IgnoreCase)
			|| N.Contains(TEXT("foot"), ESearchCase::IgnoreCase);
	}

	void SpawnBloodIfAny(const FHitResult& Hit, UNiagaraSystem* FX)
	{
		if (!FX) return;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			Hit.GetActor() ? Hit.GetActor()->GetWorld() : nullptr,
			FX,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation());
	}

	
	

};
