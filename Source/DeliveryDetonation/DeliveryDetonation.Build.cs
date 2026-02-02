using UnrealBuildTool;

public class DeliveryDetonation : ModuleRules
{
    public DeliveryDetonation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",      // Needed for new input system
            "AnimGraphRuntime",   // Needed for animation blueprints
            "UMG",                // Needed for UI
            "Slate",              // Needed for UI
            "SlateCore",          // Needed for UI
            "Niagara",            // Needed for VFX
            "OnlineSubsystem",    // Core networking
            "OnlineSubsystemUtils", // Core networking utils
            "OnlineSubsystemEIK"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}