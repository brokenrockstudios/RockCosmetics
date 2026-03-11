// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "UObject/Object.h"

#define UE_API ROCKCOSMETICS_API

namespace RockCosmeticAnimGameplayTags
{
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cosmetic_SkeletonClass);

	// Bipedal humanoid skeleton class. Use this for any skeleton compatible with standard humanoid montages,
	// regardless of proportions (e.g. SK_Mannequin, Quinn, MetaHuman, etc.). Only diverge to a new SkeletonClass
	// when the skeleton requires its own incompatible montage set.
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cosmetic_SkeletonClass_HumanoidStandard);
}

#undef UE_API
