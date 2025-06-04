// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

using UnrealBuildTool;

public class ENS : ModuleRules
{
	public ENS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "GameplayAbilities"
			, "NavigationSystem"
		});
		PrivateDependencyModuleNames.AddRange(new string[]
			{ "GameplayAbilities", "GameplayTags", "GameplayTasks", "Niagara" });
	}
}
