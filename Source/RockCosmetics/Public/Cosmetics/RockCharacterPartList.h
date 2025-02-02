// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RockCharacterPartTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "RockCharacterPartList.generated.h"

class URockMutablePawnComponent_CharacterParts;
class URockPawnComponent_CharacterParts;

// A single applied character part
USTRUCT()
struct FRockAppliedCharacterPartEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FRockAppliedCharacterPartEntry()
	{}

	FString GetDebugString() const;

private:
	friend FRockCharacterPartList;
	friend URockPawnComponent_CharacterParts;
	friend URockMutablePawnComponent_CharacterParts;

private:
	// The character part being represented
	UPROPERTY()
	FRockCharacterPart Part;

	// Handle index we returned to the user (server only)
	UPROPERTY(NotReplicated)
	int32 PartHandle = INDEX_NONE;

	// The spawned actor instance (client only)
	UPROPERTY(NotReplicated)
	TObjectPtr<UChildActorComponent> SpawnedComponent = nullptr;
};

//////////////////////////////////////////////////////////////////////

// Replicated list of applied character parts
USTRUCT(BlueprintType)
struct FRockCharacterPartList : public FFastArraySerializer
{
	GENERATED_BODY()

	FRockCharacterPartList()
		: OwnerComponent(nullptr)
	{
	}

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FRockAppliedCharacterPartEntry, FRockCharacterPartList>(Entries, DeltaParms, *this);
	}

	FRockCharacterPartHandle AddEntry(FRockCharacterPart NewPart);
	void RemoveEntry(FRockCharacterPartHandle Handle);
	void ClearAllEntries(bool bBroadcastChangeDelegate);

	FGameplayTagContainer CollectCombinedTags() const;
	
	void SetOwnerComponent(URockPawnComponent_CharacterParts* InOwnerComponent)
	{
		OwnerComponent = InOwnerComponent;
	}
	
private:
	friend URockPawnComponent_CharacterParts;
	friend URockMutablePawnComponent_CharacterParts;
	
	bool SpawnActorForEntry(FRockAppliedCharacterPartEntry& Entry);
	bool DestroyActorForEntry(FRockAppliedCharacterPartEntry& Entry);

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FRockAppliedCharacterPartEntry> Entries;

	// The component that contains this list
	UPROPERTY(NotReplicated)
	TObjectPtr<URockPawnComponent_CharacterParts> OwnerComponent;

	// Upcounter for handles
	int32 PartHandleCounter = 0;
};

template<>
struct TStructOpsTypeTraits<FRockCharacterPartList> : public TStructOpsTypeTraitsBase2<FRockCharacterPartList>
{
	enum { WithNetDeltaSerializer = true };
};