// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatProject/Interface/HitInterface.h"
#include "GameFramework/Actor.h"
#include "BreakableActor.generated.h"

UCLASS()
class COMBATPROJECT_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABreakableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetHit_Implementation(FVector ImpactPoint, AActor* Hitter) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	UGeometryCollectionComponent* GeometryCollection;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	

};
