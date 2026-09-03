using UnrealBuildTool;
using System.Collections.Generic;

public class InterrogationPrototypeTarget : TargetRules
{
	public InterrogationPrototypeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("InterrogationPrototype");
	}
}
