// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatProject/Interface/EnemyInterface.h"
#include "CombatProject/Interface/HitInterface.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class COMBATPROJECT_API AEnemy : public ACharacter, public IEnemyInterface, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetHit_Implementation(FVector ImpactPoint, AActor* Hitter) override;

protected:
	UFUNCTION(BlueprintCallable, Category="Combat")
	FName GetHitDirection(const FVector& ImpactPoint) const;

	void PlayHitReactMontage(const FName& SectionName);

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAnimMontage* HitReactMontage;
	


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HighlightEnemy(bool bEnable);
	void HandleDeath(const FVector& ExplosionLocation);
	void EnableRagdoll();

	UFUNCTION(BlueprintImplementableEvent)
	void RagdollDeath();
	

	
	
};
