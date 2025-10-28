// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "CombatProject/CombatProjectCharacter.h"
#include "CombatProject/Projectile/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values for this component's properties
UProjectileComponent::UProjectileComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
}


// Called when the game starts
void UProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<ACombatProjectCharacter>(GetOwner());
	
}


// Called every frame
void UProjectileComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UProjectileComponent::CastProjectile()
{
	if (!ProjectileClass || !PlayerRef) return;

	APlayerController* PC = Cast<APlayerController>(PlayerRef->GetController());
	if (!PC) return;

	// 1. Get the camera location and direction
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// 2. Line trace from the camera to determine where the crosshair hits
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * 10000.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerRef);

	FVector TargetLocation = TraceEnd;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		TargetLocation = HitResult.ImpactPoint;
	}

	// 3. Compute direction from camera to target
	FVector Direction = (TargetLocation - CameraLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	// 4. Set spawn location slightly in front of camera (to avoid hitting yourself)
	FVector SpawnLocation = CameraLocation + (Direction * 100.0f);

	// 5. Spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerRef;
	SpawnParams.Instigator = PlayerRef;

	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	// 6. Optionally apply initial velocity manually if needed
	if (Projectile && Projectile->ProjectileMovement)
	{
		Projectile->ProjectileMovement->Velocity = Direction * Projectile->ProjectileMovement->InitialSpeed;
	}

	// 7. Spawn muzzle VFX at the camera (or use player mesh socket if you want visual origin)
	if (MuzzleEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			MuzzleEffect,
			PlayerRef->GetMesh()->GetSocketLocation(SocketName)
		);
	}
}


