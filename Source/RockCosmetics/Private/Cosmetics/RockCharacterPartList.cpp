// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#include "Cosmetics/RockCharacterPartList.h"

#include "GameplayTagAssetInterface.h"
#include "RockCosmeticLogging.h"
#include "Components/RockPawnComponent_CharacterParts.h"


//////////////////////////////////////////////////////////////////////

FString FRockAppliedCharacterPartEntry::GetDebugString() const
{
	return FString::Printf(TEXT("(PartClass: %s, Socket: %s, Instance: %s)"), *GetPathNameSafe(Part.PartClass), *Part.SocketName.ToString(), *GetPathNameSafe(SpawnedComponent));
}

//////////////////////////////////////////////////////////////////////


void FRockCharacterPartList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	bool bDestroyedAnyActors = false;
	for (const int32 Index : RemovedIndices)
	{
		FRockAppliedCharacterPartEntry& Entry = Entries[Index];
		bDestroyedAnyActors |= DestroyActorForEntry(Entry);
	}

	if (bDestroyedAnyActors && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

void FRockCharacterPartList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	bool bCreatedAnyActors = false;
	for (int32 Index : AddedIndices)
	{
		FRockAppliedCharacterPartEntry& Entry = Entries[Index];
		bCreatedAnyActors |= SpawnActorForEntry(Entry);
	}

	if (bCreatedAnyActors && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

void FRockCharacterPartList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	bool bChangedAnyActors = false;

	// We don't support dealing with propagating changes, just destroy and recreate
	for (int32 Index : ChangedIndices)
	{
		FRockAppliedCharacterPartEntry& Entry = Entries[Index];

		bChangedAnyActors |= DestroyActorForEntry(Entry);
		bChangedAnyActors |= SpawnActorForEntry(Entry);
	}

	if (bChangedAnyActors && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

FRockCharacterPartHandle FRockCharacterPartList::AddEntry(FRockCharacterPart NewPart)
{
	FRockCharacterPartHandle Result;
	Result.PartHandle = PartHandleCounter++;

	if (ensure(OwnerComponent && OwnerComponent->GetOwner() && OwnerComponent->GetOwner()->HasAuthority()))
	{
		FRockAppliedCharacterPartEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Part = NewPart;
		NewEntry.PartHandle = Result.PartHandle;
	
		if (SpawnActorForEntry(NewEntry))
		{
			OwnerComponent->BroadcastChanged();
		}

		MarkItemDirty(NewEntry);
	}

	return Result;
}

void FRockCharacterPartList::RemoveEntry(FRockCharacterPartHandle Handle)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FRockAppliedCharacterPartEntry& Entry = *EntryIt;
		if (Entry.PartHandle == Handle.PartHandle)
		{
			const bool bDestroyedActor = DestroyActorForEntry(Entry);
			EntryIt.RemoveCurrent();
			MarkArrayDirty();

			if (bDestroyedActor && ensure(OwnerComponent))
			{
				OwnerComponent->BroadcastChanged();
			}

			break;
		}
	}
}

void FRockCharacterPartList::ClearAllEntries(bool bBroadcastChangeDelegate)
{
	bool bDestroyedAnyActors = false;
	for (FRockAppliedCharacterPartEntry& Entry : Entries)
	{
		bDestroyedAnyActors |= DestroyActorForEntry(Entry);
	}
	Entries.Reset();
	MarkArrayDirty();

	if (bDestroyedAnyActors && bBroadcastChangeDelegate && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

FGameplayTagContainer FRockCharacterPartList::CollectCombinedTags() const
{
	FGameplayTagContainer Result;

	for (const FRockAppliedCharacterPartEntry& Entry : Entries)
	{
		if (Entry.SpawnedComponent != nullptr)
		{
			if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Entry.SpawnedComponent->GetChildActor()))
			{
				TagInterface->GetOwnedGameplayTags(/*inout*/ Result);
			}
		}
	}

	return Result;
}

bool FRockCharacterPartList::SpawnActorForEntry(FRockAppliedCharacterPartEntry& Entry)
{
	bool bCreatedAnyActors = false;

	if (ensure(OwnerComponent) && !OwnerComponent->IsNetMode(NM_DedicatedServer))
	{
		if (Entry.Part.PartClass != nullptr)
		{
			UWorld* World = OwnerComponent->GetWorld();

			if (USceneComponent* ComponentToAttachTo = OwnerComponent->GetSceneComponentToAttachTo())
			{
				const FTransform SpawnTransform = ComponentToAttachTo->GetSocketTransform(Entry.Part.SocketName);

				UChildActorComponent* PartComponent = NewObject<UChildActorComponent>(OwnerComponent->GetOwner());

				PartComponent->SetupAttachment(ComponentToAttachTo, Entry.Part.SocketName);
				PartComponent->SetChildActorClass(Entry.Part.PartClass);
				PartComponent->RegisterComponent();

				if (AActor* SpawnedActor = PartComponent->GetChildActor())
				{
					switch (Entry.Part.CollisionMode)
					{
					case ERockCharacterCustomizationCollisionMode::UseCollisionFromCharacterPart:
						// Do nothing
						break;

					case ERockCharacterCustomizationCollisionMode::NoCollision:
						SpawnedActor->SetActorEnableCollision(false);
						break;
					}

					// Set up a direct tick dependency to work around the child actor component not providing one
					if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
					{
						SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
					}
				}
				else
				{
					UE_LOG(LogRockCosmetic, Warning, TEXT("FRockCharacterPartList::SpawnActorForEntry: Failed to get child actor from component"));
					
				}

				Entry.SpawnedComponent = PartComponent;
				bCreatedAnyActors = true;
			}else
			{
				UE_LOG(LogRockCosmetic, Warning, TEXT("FRockCharacterPartList::SpawnActorForEntry: ComponentToAttachTo is null"));
			}
		}
		else
		{
			UE_LOG(LogRockCosmetic, Warning, TEXT("FRockCharacterPartList::SpawnActorForEntry: PartClass is null"));
		}
	}
	else
	{
		UE_LOG(LogRockCosmetic, Warning, TEXT("FRockCharacterPartList::SpawnActorForEntry: OwnerComponent is null or in dedicated server mode"));
	}

	return bCreatedAnyActors;
}

bool FRockCharacterPartList::DestroyActorForEntry(FRockAppliedCharacterPartEntry& Entry)
{
	bool bDestroyedAnyActors = false;

	if (Entry.SpawnedComponent != nullptr)
	{
		Entry.SpawnedComponent->DestroyComponent();
		Entry.SpawnedComponent = nullptr;
		bDestroyedAnyActors = true;
	}

	return bDestroyedAnyActors;
}
