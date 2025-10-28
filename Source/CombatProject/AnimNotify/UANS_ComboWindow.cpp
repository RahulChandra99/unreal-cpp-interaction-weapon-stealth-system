// Fill out your copyright notice in the Description page of Project Settings.


#include "UANS_ComboWindow.h"

#include "CombatProject/Components/CombatComponent.h"

void UUANS_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UCombatComponent* Comp = Owner->FindComponentByClass<UCombatComponent>())
		{
			Comp->OpenComboWindow();
		}
	}
}

void UUANS_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UCombatComponent* Comp = Owner->FindComponentByClass<UCombatComponent>())
		{
			Comp->CloseComboWindow();
		}
	}
}