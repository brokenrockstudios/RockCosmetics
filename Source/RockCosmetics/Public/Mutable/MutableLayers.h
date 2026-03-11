// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mutable/MutableOptions.h"

#include "MutableLayers.generated.h"

// Priority values for Mutable cosmetic layers. Higher values will override lower values.
// These are just suggested example values, the actual values and layers will depend on the specific needs of the game and how the game's cosmetic system is structured.
// Feel free to use these or create your own. It'd probly be wise to keep them organized and consistent. 

namespace RockCosmeticAppearanceLayers
{
	constexpr int32 InitialActorState = 0; // Captured from the TaggedActor
	constexpr int32 UserCustomization = 1000; // Face/Hair/Eye choices made in a menu, character creator, or "barbershop"
	constexpr int32 Equipment = 2000; // Inventory
	constexpr int32 StatusEffects = 3000; // Poison/Frozen
}

USTRUCT()
struct FRockCosmeticHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Handle = INDEX_NONE;
	bool operator==(FRockCosmeticHandle OtherHandle) const
	{
		return Handle == OtherHandle.Handle;
	}
	
};

USTRUCT()
struct FRockMutableCosmeticEntry
{
	GENERATED_BODY()

	UPROPERTY()
	int32 LayerIndex = INDEX_NONE;
	
	UPROPERTY()
	FRockCosmeticHandle CosmeticHandle;

	UPROPERTY()
	FRockMutableOption Option;
	
	// The Magic: Sort by Priority first, then by ID (order of addition)
	bool operator<(const FRockMutableCosmeticEntry& Other) const
	{
		if (LayerIndex != Other.LayerIndex)
		{
			return LayerIndex < Other.LayerIndex;
		}
		return CosmeticHandle.Handle < Other.CosmeticHandle.Handle;
	}
};
