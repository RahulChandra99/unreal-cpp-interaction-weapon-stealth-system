// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectileComponent.generated.h"


class ACombatProjectCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBATPROJECT_API UProjectileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UProjectileComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite ,Category = "Spells")
	TSubclassOf<class AProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category = "Spells")
	class UNiagaraSystem* MuzzleEffect;

	UPROPERTY()
	ACombatProjectCharacter* PlayerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Spells")
	FName SocketName;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Spells")
	void CastProjectile();
};
