using UnrealBuildTool;
using System.Collections.Generic;

public class InterrogationPrototypeEditorTarget : TargetRules
{
	public InterrogationPrototypeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("InterrogationPrototype");
	}
}
