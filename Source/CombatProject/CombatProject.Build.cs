// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CombatProject : ModuleRules
{
	public CombatProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","PhysicsCore","Niagara","GeometryCollectionEngine","GameplayAbilities","GameplayTasks","GameplayTags" });
	}
}
