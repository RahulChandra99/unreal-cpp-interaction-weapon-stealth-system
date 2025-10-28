#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "CombatProject/Interface/HitInterface.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SC = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	RootComponent = SC;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetupAttachment(GetRootComponent());

	BoxCollision = CreateDefaultSubobject<UBoxComponent>("WeaponBox");
	BoxCollision->SetupAttachment(Mesh);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>("BoxTraceStart");
	BoxTraceStart->SetupAttachment(Mesh);

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>("BoxTraceEnd");
	BoxTraceEnd->SetupAttachment(Mesh);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// If this weapon was previously equipped and had ammo saved, restore it
	if (SavedCurrentAmmo >= 0)
	{
		CurrentAmmo = SavedCurrentAmmo;
	}

	if (SavedReserveAmmo >= 0)
	{
		ReserveAmmo = SavedReserveAmmo;
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAttacking)
	{
		PerformWeaponTrace();
	}
}

void AWeapon::StartAttack()
{
	bIsAttacking = true;
	ActorsHitThisSwing.Empty();
}

void AWeapon::EndAttack()
{
	bIsAttacking = false;
	ActorsHitThisSwing.Empty();
}

void AWeapon::PerformWeaponTrace()
{
	if (!BoxTraceStart || !BoxTraceEnd)
		return;

	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());

	TArray<FHitResult> HitResults;

	bool bHit = UKismetSystemLibrary::BoxTraceMulti(
		this,
		Start,
		End,
		FVector(10.f, 10.f, 10.f),
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResults,
		true
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || ActorsHitThisSwing.Contains(HitActor)) continue;

			ActorsHitThisSwing.Add(HitActor);

			if (HitActor->Implements<UHitInterface>())
			{
				IHitInterface::Execute_GetHit(HitActor, Hit.ImpactPoint, GetOwner());
				GetHitActor(HitActor);

				if (HitActor->ActorHasTag("Breakable"))
				{
					CreateFields(Hit.ImpactPoint);
					PhysicalMaterialBasedResponse(Hit.PhysMaterial.Get(), Hit);
					break;
				}

				if (APawn* InstigatorPawn = Cast<APawn>(GetOwner()))
				{
					if (APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController()))
					{
						if (HitCameraShake)
						{
							PC->ClientStartCameraShake(HitCameraShake);
						}
					}
				}

				ApplyHitStop(0.05f, 0.2f);
				SmoothRestoreTime(0.15f);

				if (HitEnemySound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitEnemySound, Hit.ImpactPoint);
				}

				if (EnemyHitVFX)
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EnemyHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

				if (EnemyHitNiagaraVFX)
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EnemyHitNiagaraVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

				if (HitDecalMaterial)
				{
					UGameplayStatics::SpawnDecalAtLocation(
						GetWorld(),
						HitDecalMaterial,
						FVector(10.f, 10.f, 10.f),
						Hit.ImpactPoint,
						Hit.ImpactNormal.Rotation(),
						5.f
					);
				}
			}
			else
			{
				UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get();
				PhysicalMaterialBasedResponse(PhysMat, Hit);
			}
		}
	}
}

void AWeapon::PhysicalMaterialBasedResponse(UPhysicalMaterial* PhyMaterial, FHitResult Hit)
{
	if (!PhyMaterial) return;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhyMaterial);

	switch (SurfaceType)
	{
	case SurfaceType1:
		UGameplayStatics::PlaySoundAtLocation(this, HitEnvironmentSoundWood, Hit.ImpactPoint);

		if (WoodHitVFX)
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WoodHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

		if (WoodHitNiagaraVFX)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WoodHitNiagaraVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		break;

	case SurfaceType2:
		UGameplayStatics::PlaySoundAtLocation(this, HitEnvironmentSoundMetal, Hit.ImpactPoint);

		if (MetalHitVFX)
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MetalHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

		if (MetalHitNiagaraVFX)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MetalHitNiagaraVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		break;

	case SurfaceType3:
		UGameplayStatics::PlaySoundAtLocation(this, HitEnemySound, Hit.ImpactPoint);

		if (EnemyHitVFX)
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EnemyHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

		if (EnemyHitNiagaraVFX)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EnemyHitNiagaraVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		break;

	default:
		break;
	}
}

void AWeapon::ApplyHitStop(float Duration, float Dilation)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetWorldSettings()->SetTimeDilation(Dilation);

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		[World]() { World->GetWorldSettings()->SetTimeDilation(1.0f); },
		Duration,
		false
	);
}

void AWeapon::SmoothRestoreTime(float Duration)
{
	UWorld* World = GetWorld();
	if (!World) return;

	float StartDilation = World->GetWorldSettings()->GetEffectiveTimeDilation();
	const float Target = 1.0f;
	const float Step = (Target - StartDilation) / 10.f;

	for (int i = 1; i <= 10; i++)
	{
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(
			Handle,
			[World, Step]() {
				float Current = World->GetWorldSettings()->GetEffectiveTimeDilation();
				World->GetWorldSettings()->SetTimeDilation(FMath::Clamp(Current + Step, 0.f, 1.f));
			},
			Duration / 10.f,
			false
		);
	}
}

void AWeapon::StartFiring(bool bIsAiming)
{

	bIsFiring = true;

	if (bIsAutomatic && bIsAiming)
	{
		const float Rate = 60.f / FireRate;
		GetWorldTimerManager().SetTimer(
			AutoFireHandle,
			this,
			&AWeapon::AutoFire,
			Rate,
			true
		);
	}
	else
	{
		// Single fire
		FireWeapon(bIsAiming);
	}
}

void AWeapon::AutoFire()
{
	if (!bIsFiring || bIsReloading)
		return;

	if (bCanShoot && CurrentAmmo > 0)
	{
		FireWeapon(true);
	}
	else if (CurrentAmmo <= 0)
	{
		StopFiring();
		Reload();
	}
}

void AWeapon::StopFiring()
{
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(AutoFireHandle);
}

void AWeapon::FireWeapon(bool bIsAiming)
{
	if (!bIsAiming)
		return;

	if (bIsReloading)
		return;

	if (!bCanShoot || CurrentAmmo <= 0)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("Running Empty CLip and then reloding"));
		
		if (EmptyClipSound)
			UGameplayStatics::PlaySoundAtLocation(this, EmptyClipSound, GetActorLocation());

		Reload();

		return;
	}

	HandleFire(bIsAiming);
	bCanShoot = false;

	const float Delay = 60.f / FireRate;
	GetWorldTimerManager().SetTimer(FireDelayHandle, [this]()
	{
		bCanShoot = true;
	}, Delay, false);
}

void AWeapon::HandleFire(bool bADS)
{
    if (CurrentAmmo <= 0) return;
    CurrentAmmo--;

    PlayFireEffects();
    ApplyRecoil();

    // --- Viewpoint ---
    FVector EyeLoc;
    FRotator EyeRot;
    AController* InstigatorController = GetInstigatorController();
    if (InstigatorController)
    {
        InstigatorController->GetPlayerViewPoint(EyeLoc, EyeRot);
    }
    else if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->GetPlayerViewPoint(EyeLoc, EyeRot);
    }

    // --- Spread ---
    const float SpreadDeg = bADS ? SpreadADSDeg : SpreadHipDeg;
    const float SpreadRad = FMath::DegreesToRadians(SpreadDeg);
    const FVector ShotDir = FMath::VRandCone(EyeRot.Vector(), SpreadRad);
    const FVector End = EyeLoc + ShotDir * WeaponRange;

    // --- Trace ---
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());
    Params.bReturnPhysicalMaterial = true;
    Params.bTraceComplex = true;
	

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, End, ECC_Visibility, Params);

    // --- Tracer ---
    if (TracerFX)
    {
        FVector Start = Mesh ? Mesh->GetSocketLocation("MuzzleFlash") : GetActorLocation();
        FVector Target = bHit ? Hit.ImpactPoint : End;

        const FVector Direction = (Target - Start).GetSafeNormal();
        const FRotator TracerRot = Direction.Rotation();

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, Start, TracerRot);
    }

    if (!bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("No hit detected."));
        return;
    }

    // --- Debug: what we hit ---
    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s | Bone: %s | Location: %s"),
        *Hit.GetActor()->GetName(),
        *Hit.BoneName.ToString(),
        *Hit.ImpactPoint.ToString());

    // --- Surface Response ---
    UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get();
    PhysicalMaterialBasedResponseForGun(PhysMat, Hit);

    float FinalDamage = BaseDamage;

    // --- Enemy detection ---
    if (Hit.GetActor() && Hit.GetActor()->Implements<UHitInterface>())
    {

    	// Now safely use the bone name
    	if (Hit.BoneName != NAME_None)
    	{
    		if (IsHeadBone(Hit.BoneName))
    		{
    			FinalDamage *= HeadshotMultiplier;
    			ShotEnemy(true);
    			UE_LOG(LogTemp, Warning, TEXT("Headshot detected! Bone: %s"), *Hit.BoneName.ToString());
    		}
    		else if (IsLimbBone(Hit.BoneName))
    		{
    			FinalDamage *= LimbMultiplier;
    			ShotEnemy(false);
    			UE_LOG(LogTemp, Warning, TEXT("Limb hit detected! Bone: %s"), *Hit.BoneName.ToString());
    		}
    		else
    		{
    			ShotEnemy(false);
    			UE_LOG(LogTemp, Warning, TEXT("Torso or other bone hit. Bone: %s"), *Hit.BoneName.ToString());
    		}
    	}
    	else
    	{
    		ShotEnemy(false);
    		UE_LOG(LogTemp, Warning, TEXT("No bone detected, checking material type."));
    	}

    	
    	SpawnBloodIfAny(Hit, GunEnemyHitNiagaraVFX);
    	UE_LOG(LogTemp, Warning, TEXT("Blood spawned at %s"), *Hit.ImpactPoint.ToString());

    	EnemyHit(FinalDamage,Hit.GetActor(),Hit);
    }
    else
    {
        // --- Non-enemy surface ---
        if (PhysMat)
        {
            const EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);
            UE_LOG(LogTemp, Warning, TEXT("Hit surface type: %d"), (int32)SurfaceType);

            if (SurfaceType == SurfaceType3)
            {
                SpawnBloodIfAny(Hit, GunEnemyHitNiagaraVFX);
                UE_LOG(LogTemp, Warning, TEXT("Blood spawned on Flesh surface."));
            }
        }
    }
}




void AWeapon::ApplyRecoil()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;

	if (RecoilCameraShake)
	{
		PC->ClientStartCameraShake(RecoilCameraShake);
	}

	const float PitchKick = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
	const float YawKick   = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

	PC->AddPitchInput(-PitchKick);
	PC->AddYawInput(YawKick);
}



void AWeapon::PhysicalMaterialBasedResponseForGun(UPhysicalMaterial* PhyMaterial, FHitResult Hit)
{
	if (!PhyMaterial) return;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhyMaterial);

	switch (SurfaceType)
	{
	case SurfaceType1:
		if (GunWoodHitVFX)
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunWoodHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		break;

	case SurfaceType2:
		if (GunMetalHitVFX)
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunMetalHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		break;

	case SurfaceType3:
		if (GunEnemyHitNiagaraVFX)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GunEnemyHitNiagaraVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		break;

	default:
		break;
	}
}

void AWeapon::Reload()
{
	if (ReserveAmmo <= 0)
		return;

	if (CurrentAmmo >= MagazineSize)
		return;

	PlayReloadEffects();
}

void AWeapon::HandleReload()
{
	if (ReserveAmmo <= 0)
		return;

	if (CurrentAmmo >= MagazineSize)
		return;

	int32 Needed = MagazineSize - CurrentAmmo;
	int32 AmmoToLoad = FMath::Min(ReserveAmmo, Needed);

	CurrentAmmo += AmmoToLoad;
	ReserveAmmo -= AmmoToLoad;
}
