using UnrealBuildTool;

public class InterrogationPrototype : ModuleRules
{
	public InterrogationPrototype(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"EnhancedInput",
			"InputCore"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"AIModule",
			"AudioCapture",
			"AudioCaptureCore",
			"AudioMixer",
			"HTTP",
			"Json",
			"JsonUtilities",
			"UMG"
		});
	}
}
