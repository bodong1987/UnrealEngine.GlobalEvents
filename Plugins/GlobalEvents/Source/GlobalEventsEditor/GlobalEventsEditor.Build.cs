using UnrealBuildTool;

public class GlobalEventsEditor : ModuleRules
{
	public GlobalEventsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"Engine",
				"UnrealEd",
                "BlueprintGraph",
                "SlateCore",
				"Slate",
                "GraphEditor",
                "BlueprintGraph",
                "ToolMenus",
                "KismetCompiler",
				"GameplayTags",
				"GameplayTagsEditor"
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"GlobalEvents"
			}
		);
	}
}
