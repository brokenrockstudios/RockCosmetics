// Copyright Broken Rock Studios LLC. All Rights Reserved.

#include "GameplayTagsManager.h"
#include "Modules/ModuleManager.h"

// FNativeGameplayTag ensures when defined outside a Runtime module, so the test cosmetic tags are added through the
// legacy AddNativeGameplayTag path at startup instead. They only exist in builds that load this Developer module.
class FRockCosmeticsTestsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		const FString Comment = TEXT("RockCosmeticsTests cosmetic tag");
		Manager.AddNativeGameplayTag(TEXT("Test.RockCosmetics.Hat"), Comment);
		Manager.AddNativeGameplayTag(TEXT("Test.RockCosmetics.Head"), Comment);
		Manager.AddNativeGameplayTag(TEXT("Test.RockCosmetics.Boots"), Comment);
		// Skeleton classes must sit under Cosmetic.SkeletonClass: URockAnimSubsystem filters the part tags by that prefix.
		Manager.AddNativeGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsA"), Comment);
		Manager.AddNativeGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsB"), Comment);
		Manager.AddNativeGameplayTag(TEXT("Test.RockCosmetics.Anim.Pickup"), Comment);
		Manager.AddNativeGameplayTag(TEXT("Test.RockCosmetics.Anim.Pickup.Heavy"), Comment);
		Manager.AddNativeGameplayTag(TEXT("Test.RockCosmetics.Anim.Missing"), Comment);
	}
};

IMPLEMENT_MODULE(FRockCosmeticsTestsModule, RockCosmeticsTests)
