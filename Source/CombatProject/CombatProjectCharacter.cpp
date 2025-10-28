// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatProjectCharacter.h"
#include "CombatProjectCharacter.h"

#include "CombatPlayerAnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "InputActionValue.h"
#include "Enemy/Enemy.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayAbility/AttributeSets/BasicAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Projectile/ProjectileBase.h"
#include "Weapon/Weapon.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACombatProjectCharacter

ACombatProjectCharacter::ACombatProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	
}

void ACombatProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerAnimInstance = Cast<UCombatPlayerAnimInstance>(GetMesh()->GetAnimInstance());
}

void ACombatProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateGrabbedLocation(DeltaSeconds);

	if (bIsGrenadeAiming)
		PredictGrenadePath();

}


void ACombatProjectCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
	}
}

void ACombatProjectCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
	}
}



void ACombatProjectCharacter::DestoryGun(class AWeapon* EquippedWeapon)
{
	if (EquippedWeapon)
	{
		FName WeaponID = EquippedWeapon->WeaponID; // something like "Pistol" or "Rifle"
		SavedCurrentAmmoMap.Add(WeaponID, EquippedWeapon->GetCurrentAmmo());
		SavedReserveAmmoMap.Add(WeaponID, EquippedWeapon->GetMaxAmmo());
	}
}

void ACombatProjectCharacter::SpawnWeapon(class AWeapon* EquippedWeapon)
{
	if (SavedCurrentAmmoMap.Contains(EquippedWeapon->WeaponID))
	{
		EquippedWeapon->CurrentAmmo = SavedCurrentAmmoMap[EquippedWeapon->WeaponID];
		EquippedWeapon->ReserveAmmo = SavedReserveAmmoMap[EquippedWeapon->WeaponID];
	}
}

void ACombatProjectCharacter::PredictGrenadePath()
{
	FVector Start = FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * 50.f;
	FVector LaunchVelocity = FollowCamera->GetForwardVector() * ThrowSpeed;
	LaunchVelocity.Z += 300.f; // optional upward arc

	FPredictProjectilePathParams Params;
	Params.StartLocation = Start;
	Params.LaunchVelocity = LaunchVelocity;
	Params.bTraceWithCollision = true;
	Params.ProjectileRadius = 10.f;
	Params.MaxSimTime = 3.f;
	Params.SimFrequency = 30.f;
	Params.TraceChannel = ECC_Visibility;
	Params.ActorsToIgnore.Add(this);
	Params.DrawDebugType = EDrawDebugTrace::None;

	// Match gravity with ProjectileMovementComponent
	Params.OverrideGravityZ = GetWorld()->GetGravityZ();

	FPredictProjectilePathResult Result;
	if (UGameplayStatics::PredictProjectilePath(this, Params, Result))
	{
		for (int32 i = 0; i < Result.PathData.Num() - 1; i++)
		{
			DrawDebugLine(GetWorld(),
				Result.PathData[i].Location,
				Result.PathData[i + 1].Location,
				FColor::Green, false, 0.f, 0, 2.f);
		}

		DrawDebugSphere(GetWorld(), Result.HitResult.Location, 12.f, 8, FColor::Red, false, 0.f);
	}
}

void ACombatProjectCharacter::ThrowGrenade(TSubclassOf<AProjectileBase> GrenadeActor)
{
	if (!GrenadeActor) return;

	// Get socket transform from the character mesh
	const FName SocketName = TEXT("GrenadeEquipSocket"); // use your actual socket name
	FTransform SocketTransform = GetMesh()->GetSocketTransform(SocketName, RTS_World);

	FVector Forward = FollowCamera->GetForwardVector();
	FVector SpawnLoc = SocketTransform.GetLocation() + Forward * 20.f;
	FRotator SpawnRot = FollowCamera->GetComponentRotation(); // aim direction

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = this; // this makes the player the grenade's owner
	Params.Instigator = this;

	AProjectileBase* Grenade = GetWorld()->SpawnActor<AProjectileBase>(GrenadeActor, SpawnLoc, SpawnRot, Params);
	if (Grenade)
	{
		// Launch in the direction the camera is facing, so the player can aim
		FVector LaunchVel = FollowCamera->GetForwardVector() * ThrowSpeed;
		LaunchVel.Z += ThrowScale * 300.f;

		if (UProjectileMovementComponent* Movement = Grenade->FindComponentByClass<UProjectileMovementComponent>())
		{
			Movement->Velocity = LaunchVel;
		}

		// Optional: small debug sphere to confirm correct spawn position
		//DrawDebugSphere(GetWorld(), SpawnLoc, 10.f, 12, FColor::Yellow, false, 2.f);
	}
}

void ACombatProjectCharacter::ActivateSenseVision()
{
	FVector PlayerLocation = GetActorLocation();
        float SenseRadius = 200000.f;
    
        TArray<FOverlapResult> Overlaps;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(SenseRadius);
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
    
        bool bHit = GetWorld()->OverlapMultiByChannel(
            Overlaps,
            PlayerLocation,
            FQuat::Identity,
            ECC_Pawn,
            Sphere,
            Params
        );
    
        if (bHit)
        {
            for (auto& Hit : Overlaps)
            {
                AEnemy* Enemy = Cast<AEnemy>(Hit.GetActor());
                if (Enemy)
                {
                    Enemy->HighlightEnemy(true);
                }
            }
        }
    
        // Optional timer to turn off after few seconds
        //GetWorldTimerManager().SetTimer(SenseTimer, this, &AMyCharacter::DeactivateSenseVision, 5.f, false);
}


void ACombatProjectCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACombatProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatProjectCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACombatProjectCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

bool ACombatProjectCharacter::IsEnemyBehindTarget(AActor* TargetActor)
{
	if (!TargetActor) return false;

	FVector ToTarget = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector TargetForward = TargetActor->GetActorForwardVector();

	float Dot = FVector::DotProduct(TargetForward, ToTarget);

	if (Dot > 0.3f)
	{
		return false;
		// In front
	}
	else
	{
		return true;
		// Behind
	}
}



void ACombatProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACombatProjectCharacter::Look(const FInputActionValue& Value)
{
	if (bIsWheelOpen) return;
	
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X * XLookSens);
		AddControllerPitchInput(LookAxisVector.Y * YLookSens);

		CalculateAimDirection();
	}
}

UAbilitySystemComponent* ACombatProjectCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACombatProjectCharacter::GrabEnemy(AEnemy* EnemyRef)
{

	if (!EnemyRef) return;

	USkeletalMeshComponent* EnemyMesh = EnemyRef->GetMesh();
	if (!EnemyMesh || !EnemyMesh->IsSimulatingPhysics()) return;

	// Grab at pelvis bone
	const FVector GrabLocation = EnemyMesh->GetBoneLocation(GrabBoneName);
	const FRotator GrabRotation = EnemyMesh->GetBoneQuaternion(GrabBoneName).Rotator();

	PhysicsHandle->GrabComponentAtLocationWithRotation(EnemyMesh, GrabBoneName, GrabLocation, GrabRotation);
	PhysicsHandle->LinearStiffness = 8000.f;     // higher = stronger pull
	PhysicsHandle->AngularStiffness = 8000.f;
	PhysicsHandle->LinearDamping = 300.f;        // prevents jitter
	PhysicsHandle->AngularDamping = 300.f;
	PhysicsHandle->InterpolationSpeed = 30.f;    // smoother follow
	GrabbedMesh = EnemyMesh;
}

void ACombatProjectCharacter::ReleaseEnemy()
{
	if (PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->ReleaseComponent();
		GrabbedMesh = nullptr;
	}
}

void ACombatProjectCharacter::UpdateGrabbedLocation(float DeltaTime)
{
	if (!GrabbedMesh || !PhysicsHandle->GrabbedComponent) return;

	// Get hand socket location and rotation from your character mesh
	const FVector HandLocation = GetMesh()->GetSocketLocation("hand_r");
	const FRotator HandRotation = GetMesh()->GetSocketRotation("hand_r");

	// Optional small offset to keep body slightly away from player
	const FVector Offset = GetActorForwardVector() * 20.f - GetActorRightVector() * 10.f;
	const FVector TargetLocation = HandLocation + Offset;

	PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, HandRotation);
}