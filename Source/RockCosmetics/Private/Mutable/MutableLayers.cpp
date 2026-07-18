// Copyright Broken Rock Studios LLC. All Rights Reserved.


#include "Mutable/MutableLayers.h"

#include "Components/RockMutablePawnComponent_CharacterParts.h"

FRockCosmeticHandle FRockMutableCosmeticEntryList::AddEntry(int32 LayerIndex, const FRockMutableOption& Option)
{
	FRockCosmeticHandle NewHandle;
	NewHandle.Handle = ++HandleCounter;

	FRockMutableCosmeticEntry& NewEntry = Entries.Add_GetRef(FRockMutableCosmeticEntry(LayerIndex, NewHandle, Option));
	MarkItemDirty(NewEntry);

	if (OwnerComponent)
	{
		OwnerComponent->RequestRecompose();
	}
	return NewHandle;
}

int32 FRockMutableCosmeticEntryList::RemoveByPredicate(TFunctionRef<bool(const FRockMutableCosmeticEntry&)> Predicate)
{
	int32 RemovedCount = 0;
	for (auto It = Entries.CreateIterator(); It; ++It)
	{
		if (Predicate(*It))
		{
			It.RemoveCurrent();
			++RemovedCount;
		}
	}

	if (RemovedCount > 0)
	{
		MarkArrayDirty();
		if (OwnerComponent)
		{
			OwnerComponent->RequestRecompose();
		}
	}
	return RemovedCount;
}

int32 FRockMutableCosmeticEntryList::RemoveByHandle(FRockCosmeticHandle Handle)
{
	return RemoveByPredicate([Handle](const FRockMutableCosmeticEntry& Entry)
	{
		return Entry.CosmeticHandle == Handle;
	});
}

int32 FRockMutableCosmeticEntryList::RemoveByLayer(int32 LayerIndex)
{
	return RemoveByPredicate([LayerIndex](const FRockMutableCosmeticEntry& Entry)
	{
		return Entry.LayerIndex == LayerIndex;
	});
}

TArray<FRockMutableCosmeticEntry> FRockMutableCosmeticEntryList::GetSortedEntries() const
{
	TArray<FRockMutableCosmeticEntry> Sorted = Entries;
	Sorted.Sort();
	return Sorted;
}

// The authority changed the replicated cosmetic data - rebuild this client's mesh from it. ExecuteRecompose
// composes InitialAppearance + CosmeticMutableEntries against THIS machine's own Mutable instance, so it works
// on clients (which run Mutable) regardless of whether the server (which may not) built anything. If our
// instance isn't resolvable yet, ExecuteRecompose retries itself (see ResolveCustomizableObjectInstance) until
// it is, rather than depending on BroadcastChanged firing again.
void FRockMutableCosmeticEntryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (OwnerComponent)
	{
		OwnerComponent->RequestRecompose();
	}
}

void FRockMutableCosmeticEntryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (OwnerComponent)
	{
		OwnerComponent->RequestRecompose();
	}
}

void FRockMutableCosmeticEntryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (OwnerComponent)
	{
		OwnerComponent->RequestRecompose();
	}
}
