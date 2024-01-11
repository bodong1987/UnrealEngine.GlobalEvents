using UnrealBuildTool;

public class GlobalEvents : ModuleRules
{
	public GlobalEvents(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"Engine",
				"GameplayTags"
				// ... add other public dependencies that you statically link with here ...
			}
		);
	}
}
