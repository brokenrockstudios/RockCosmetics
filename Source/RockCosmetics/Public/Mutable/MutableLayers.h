// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mutable/MutableOptions.h"
#include "Net/Serialization/FastArraySerializer.h"

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

	bool IsValid() const { return Handle != INDEX_NONE; }
};

class URockMutablePawnComponent_CharacterParts;

USTRUCT()
struct FRockMutableCosmeticEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FRockMutableCosmeticEntry() {}
	FRockMutableCosmeticEntry(int32 InLayerIndex, FRockCosmeticHandle InHandle, const FRockMutableOption& InOption)
		: LayerIndex(InLayerIndex)
		, CosmeticHandle(InHandle)
		, Option(InOption)
	{}

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

// Replicated list of Mutable cosmetic parameter entries. Uses FFastArraySerializer (like FRockCharacterPartList)
// so that adding/removing a single entry only replicates that delta, not the whole array.
//
// Array order is NOT guaranteed to survive replication: FastArraySerializer only replicates added/removed/changed
// entries, so a server-side reorder of otherwise-unchanged entries is never sent. Anything that needs layer
// priority order (e.g. composing the final descriptor) must be sorted at the point of use. See GetSortedEntries().
USTRUCT()
struct FRockMutableCosmeticEntryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FRockMutableCosmeticEntryList()
		: OwnerComponent(nullptr)
	{}

	//~FFastArraySerializer contract
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FRockMutableCosmeticEntry, FRockMutableCosmeticEntryList>(Entries, DeltaParms, *this);
	}

	void SetOwnerComponent(URockMutablePawnComponent_CharacterParts* InOwnerComponent)
	{
		OwnerComponent = InOwnerComponent;
	}

	int32 Num() const { return Entries.Num(); }

	// Adds an entry and returns a handle that can be used to remove it later.
	FRockCosmeticHandle AddEntry(int32 LayerIndex, const FRockMutableOption& Option);

	// Removes every entry matching Handle/LayerIndex. Returns the number of entries removed.
	int32 RemoveByHandle(FRockCosmeticHandle Handle);
	int32 RemoveByLayer(int32 LayerIndex);

	// Returns a copy of the entries sorted by layer priority then insertion order (see class comment for why
	// this can't just be the live array order).
	TArray<FRockMutableCosmeticEntry> GetSortedEntries() const;

private:
	int32 RemoveByPredicate(TFunctionRef<bool(const FRockMutableCosmeticEntry&)> Predicate);

	UPROPERTY()
	TArray<FRockMutableCosmeticEntry> Entries;

	// The component that contains this list
	UPROPERTY(NotReplicated)
	TObjectPtr<URockMutablePawnComponent_CharacterParts> OwnerComponent = nullptr;

	// counter for handles (server/authority only; not replicated, matches FRockCharacterPartList's PartHandleCounter)
	int32 HandleCounter = 0;
};

template<>
struct TStructOpsTypeTraits<FRockMutableCosmeticEntryList> : public TStructOpsTypeTraitsBase2<FRockMutableCosmeticEntryList>
{
	enum { WithNetDeltaSerializer = true };
};
