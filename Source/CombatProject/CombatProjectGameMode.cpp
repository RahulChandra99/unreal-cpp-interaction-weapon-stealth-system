// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatProjectGameMode.h"
#include "CombatProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACombatProjectGameMode::ACombatProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
