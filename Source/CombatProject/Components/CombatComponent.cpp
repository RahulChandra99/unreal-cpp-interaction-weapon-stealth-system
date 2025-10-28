// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		AnimInstance = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>()
			? OwnerPawn->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()
			: nullptr;

		
	}
	
}



bool UCombatComponent::CanAttack(EAttackType Type) const
{
	if (!AnimInstance) return false;

	// Add stamina or cooldown checks here if needed
	const bool bHasMontages = (Type == EAttackType::Light)
		? LightAttackMontages.Num() > 0
		: HeavyAttackMontages.Num() > 0;

	return bHasMontages;
}

void UCombatComponent::LightAttack()
{
	TryAttack(EAttackType::Light);
}

void UCombatComponent::HeavyAttack()
{
	TryAttack(EAttackType::Heavy);
}

void UCombatComponent::TryAttack(EAttackType Type)
{
	if (!CanAttack(Type)) return;

	// If we are not attacking, play immediately
	if (!bIsAttacking)
	{
		PlayNextMontage(Type);
		return;
	}

	// If we are mid attack and inside the window, queue next
	if (bComboWindowOpen)
	{
		QueuedAttack = Type;
		bHasQueuedAttack = true;
	}
	// If outside the window, ignore or buffer
}

void UCombatComponent::PlayNextMontage(EAttackType Type)
{
	if (!AnimInstance) return;

	UAnimMontage* MontageToPlay = GetNextMontage(Type);
	if (!MontageToPlay) return;

	GetWorld()->GetTimerManager().ClearTimer(ComboResetHandle);
	bIsAttacking = true;

	// Root motion and section handling lives inside the montage
	float PlayRate = 1.0f;
	float StartPosition = 0.0f;

	AnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, StartPosition, true);

	AdvanceIndex(Type);
}

UAnimMontage* UCombatComponent::GetNextMontage(EAttackType Type)
{
	if (Type == EAttackType::Light)
	{
		if (LightAttackMontages.Num() == 0) return nullptr;
		int32 SafeIdx = FMath::Clamp(LightIndex, 0, LightAttackMontages.Num() - 1);
		return LightAttackMontages[SafeIdx];
	}
	else if (Type == EAttackType::Heavy)
	{
		if (HeavyAttackMontages.Num() == 0) return nullptr;
		int32 SafeIdx = FMath::Clamp(HeavyIndex, 0, HeavyAttackMontages.Num() - 1);
		return HeavyAttackMontages[SafeIdx];
	}

	return nullptr;
}

void UCombatComponent::AdvanceIndex(EAttackType Type)
{
	if (Type == EAttackType::Light)
	{
		if (LightAttackMontages.Num() > 0)
		{
			LightIndex = (LightIndex + 1) % LightAttackMontages.Num();
		}
	}
	else if (Type == EAttackType::Heavy)
	{
		if (HeavyAttackMontages.Num() > 0)
		{
			HeavyIndex = (HeavyIndex + 1) % HeavyAttackMontages.Num();
		}
	}
}

void UCombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;
}

void UCombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;
}

void UCombatComponent::OnMontageEnded()
{
	// If something is queued, play it now
	if (bHasQueuedAttack)
	{
		const EAttackType NextType = QueuedAttack;
		QueuedAttack = EAttackType::None;
		bHasQueuedAttack = false;
		bComboWindowOpen = false;

		PlayNextMontage(NextType);
		return;
	}

	// No queue, schedule combo reset
	bIsAttacking = false;
	bComboWindowOpen = false;

	GetWorld()->GetTimerManager().SetTimer(
		ComboResetHandle,
		this,
		&UCombatComponent::ResetCombo,
		ComboResetTime,
		false
	);
}

void UCombatComponent::ResetCombo()
{
	LightIndex = 0;
	HeavyIndex = 0;
	QueuedAttack = EAttackType::None;
	bHasQueuedAttack = false;
}

void UCombatComponent::CancelCombo()
{
	if (!AnimInstance) return;

	// Stops any active montage related to our lists
	for (UAnimMontage* M : LightAttackMontages)
	{
		if (M && AnimInstance->Montage_IsPlaying(M))
		{
			AnimInstance->Montage_Stop(0.1f, M);
		}
	}
	for (UAnimMontage* M : HeavyAttackMontages)
	{
		if (M && AnimInstance->Montage_IsPlaying(M))
		{
			AnimInstance->Montage_Stop(0.1f, M);
		}
	}

	ResetCombo();
}

