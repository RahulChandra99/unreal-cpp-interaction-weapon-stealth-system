#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

UCLASS()
class COMBATPROJECT_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Effects")
	class UNiagaraSystem* ImpactEffect;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Effects")
	class USoundBase* ImpactSFX;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* ThrowableMesh;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Combat")
	float Damage = 50.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Combat")
	float ExplosionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Combat")
	float ExplosionTime = 3.0f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Combat")
	bool bIsProjectileThrowable;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGrenade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsForCoin;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void Explode();

	UFUNCTION(BlueprintImplementableEvent)
	void ExplodeBP(const TArray<AActor*>& HitEnemies);

	FTimerHandle FuseTimerHandle;
};
