#include "ProjectileBase.h"
#include "NiagaraFunctionLibrary.h"
#include "CombatProject/Enemy/Enemy.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision setup
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(15.f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->SetSimulatePhysics(false);
	CollisionComp->SetEnableGravity(true);
	CollisionComp->BodyInstance.bUseCCD = true; // prevents tunneling
	CollisionComp->SetNotifyRigidBodyCollision(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	RootComponent = CollisionComp;
	CollisionComp->IgnoreActorWhenMoving(GetOwner(), true);

	// Visual mesh
	ThrowableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowableMesh"));
	ThrowableMesh->SetupAttachment(CollisionComp);
	ThrowableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Projectile movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->MaxSpeed = 1200.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.4f;
	ProjectileMovement->Friction = 0.2f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	InitialLifeSpan = 10.f;
	
	
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Start fuse timer
	if (bIsProjectileThrowable)
		GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AProjectileBase::Explode, ExplosionTime, false);
	
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bIsProjectileThrowable)
	{
		// Switch from projectile movement to physics roll on first impact
		if (ProjectileMovement && ProjectileMovement->IsActive())
		{
			ProjectileMovement->StopMovementImmediately();
			ProjectileMovement->Deactivate();
		}

		if (CollisionComp && !CollisionComp->IsSimulatingPhysics())
		{
			CollisionComp->SetSimulatePhysics(true);
			CollisionComp->SetPhysicsLinearVelocity(GetVelocity() * 0.5f);
		}
	}
	
	else
	{
		if (OtherActor && OtherActor != GetOwner())
        	{
        		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, DamageType);
        		
        		if (ImpactEffect) 
        			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactEffect, GetActorLocation());
        		
        		if (ImpactSFX) 
        			UGameplayStatics::PlaySoundAtLocation( this, ImpactSFX, GetActorLocation() );
        		
        		Destroy();
        	}
	}
	
}

void AProjectileBase::Explode()
{
	FVector ExplosionLocation = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	ExplosionRadius = 600.f;
	float LaunchForce = 400.f;
	float UpwardBoost = 200.f;

	TArray<AActor*> HitEnemies;

	bool bHit = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		ExplosionLocation,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		Params
	);

	if (!bIsForCoin)
	{
		if (bHit)
		{
			for (const FOverlapResult& Result : Overlaps)
			{
				AActor* HitActor = Result.GetActor();
				if (!HitActor) continue;

				// Check if it's an enemy character
				if (AEnemy* Enemy = Cast<AEnemy>(HitActor))
				{
					// Calculate launch direction
					FVector Direction = (Enemy->GetActorLocation() - ExplosionLocation).GetSafeNormal();
					Direction.Z += UpwardBoost / LaunchForce; // add some lift
					FVector LaunchVelocity = Direction * LaunchForce;

					// Launch first (uses CharacterMovement)
					Enemy->LaunchCharacter(LaunchVelocity, true, true);

					// Call custom death logic
					Enemy->HandleDeath(ExplosionLocation);
				}
			}
		}
	} 
	

	// Notify Blueprints
	ExplodeBP(HitEnemies);
	
}

