// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}


void AEnemy::GetHit_Implementation(FVector ImpactPoint, AActor* Hitter)
{
	// 1. Figure out where the hit came from
	const FName Section = GetHitDirection(ImpactPoint);

	// 2. Play the montage
	PlayHitReactMontage(Section);

	
}

FName AEnemy::GetHitDirection(const FVector& ImpactPoint) const
{
	// Flatten to XY so height difference doesn't matter
	FVector ToHit = ImpactPoint - GetActorLocation();
	ToHit.Z = 0.f;
	ToHit.Normalize();

	// Convert to rotation
	const FRotator ToHitRot = ToHit.Rotation();
	const double YawDiff = FMath::UnwindDegrees(ToHitRot.Yaw - GetActorRotation().Yaw);

	if (YawDiff >= -45.f && YawDiff < 45.f)
		return FName("FromFront");
	else if (YawDiff >= 45.f && YawDiff < 135.f)
		return FName("FromRight");
	else if (YawDiff <= -45.f && YawDiff > -135.f)
		return FName("FromLeft");
	else
		return FName("FromBack");
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	if (!HitReactMontage) return;

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	Anim->Montage_Play(HitReactMontage);
	Anim->Montage_JumpToSection(SectionName, HitReactMontage);
}



// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::HighlightEnemy(bool bEnable)
{
	GetMesh()->SetRenderCustomDepth(bEnable);
	GetMesh()->CustomDepthStencilValue = bEnable ? 1 : 0;
}

void AEnemy::HandleDeath(const FVector& ExplosionLocation)
{
	// Optional short delay to let LaunchCharacter complete
	FTimerHandle RagdollTimer;
	GetWorldTimerManager().SetTimer(RagdollTimer, this, &AEnemy::EnableRagdoll, 0.3f, false);
}

void AEnemy::EnableRagdoll()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	UCapsuleComponent* Capsule = GetCapsuleComponent();

	if (!MeshComp || !Capsule) return;

	// Detach the mesh to simulate freely
	MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Disable capsule collision
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Enable ragdoll
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetSimulatePhysics(true);

	// Optional: small impulse for dramatic effect
	FVector Impulse = GetActorForwardVector() * 200.f + FVector::UpVector * 150.f;
	MeshComp->AddImpulse(Impulse, NAME_None, true);
	
	// Optional: clean up after 5 seconds
	//SetLifeSpan(5.f);

	//dead in blueprints
	RagdollDeath();

	
}




