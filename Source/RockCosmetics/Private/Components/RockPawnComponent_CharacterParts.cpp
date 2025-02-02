// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#include "Components/RockPawnComponent_CharacterParts.h"

#include "GameplayTagContainer.h"
#include "Actors/RockMutableTaggedActor.h"
#include "Cosmetics/RockCharacterPartList.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockPawnComponent_CharacterParts)


URockPawnComponent_CharacterParts::URockPawnComponent_CharacterParts(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void URockPawnComponent_CharacterParts::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CharacterPartList);
}

FRockCharacterPartHandle URockPawnComponent_CharacterParts::AddCharacterPart(const FRockCharacterPart& NewPart)
{
	return CharacterPartList.AddEntry(NewPart);
}

void URockPawnComponent_CharacterParts::RemoveCharacterPart(FRockCharacterPartHandle Handle)
{
	CharacterPartList.RemoveEntry(Handle);
}

void URockPawnComponent_CharacterParts::RemoveAllCharacterParts()
{
	CharacterPartList.ClearAllEntries(/*bBroadcastChangeDelegate=*/ true);
}

void URockPawnComponent_CharacterParts::BeginPlay()
{
	Super::BeginPlay();
}

void URockPawnComponent_CharacterParts::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CharacterPartList.ClearAllEntries(/*bBroadcastChangeDelegate=*/ false);

	Super::EndPlay(EndPlayReason);
}

void URockPawnComponent_CharacterParts::OnRegister()
{
	Super::OnRegister();
	
	if (!IsTemplate())
	{
		CharacterPartList.SetOwnerComponent(this);
	}
}

TArray<AActor*> URockPawnComponent_CharacterParts::GetCharacterPartActors() const
{
	TArray<AActor*> Result;
	Result.Reserve(CharacterPartList.Entries.Num());

	for (const FRockAppliedCharacterPartEntry& Entry : CharacterPartList.Entries)
	{
		if (UChildActorComponent* PartComponent = Entry.SpawnedComponent)
		{
			if (AActor* SpawnedActor = PartComponent->GetChildActor())
			{
				Result.Add(SpawnedActor);
			}
		}
	}

	return Result;
}

USkeletalMeshComponent* URockPawnComponent_CharacterParts::GetParentMeshComponent() const
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor))
		{
			if (USkeletalMeshComponent* MeshComponent = OwningCharacter->GetMesh())
			{
				return MeshComponent;
			}
		}
	}

	return nullptr;
}

USceneComponent* URockPawnComponent_CharacterParts::GetSceneComponentToAttachTo() const
{
	if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
	{
		return MeshComponent;
	}
	else if (AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetRootComponent();
	}
	else
	{
		return nullptr;
	}
}

FGameplayTagContainer URockPawnComponent_CharacterParts::GetCombinedTags(FGameplayTag RequiredPrefix) const
{
	FGameplayTagContainer Result = CharacterPartList.CollectCombinedTags();
	if (RequiredPrefix.IsValid())
	{
		return Result.Filter(FGameplayTagContainer(RequiredPrefix));
	}
	else
	{
		return Result;
	}
}

void URockPawnComponent_CharacterParts::BroadcastChanged()
{
	const bool bReinitPose = true;

	// Check to see if the body type has changed
	if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
	{
		// Determine the mesh to use based on cosmetic part tags
		const FGameplayTagContainer MergedTags = GetCombinedTags(FGameplayTag());
		USkeletalMesh* DesiredMesh = BodyMeshes.SelectBestBodyStyle(MergedTags);

		// Apply the desired mesh (this call is a no-op if the mesh hasn't changed)
		MeshComponent->SetSkeletalMesh(DesiredMesh, /*bReinitPose=*/ bReinitPose);

		// Apply the desired physics asset if there's a forced override independent of the one from the mesh
		if (UPhysicsAsset* PhysicsAsset = BodyMeshes.ForcedPhysicsAsset)
		{
			MeshComponent->SetPhysicsAsset(PhysicsAsset, /*bForceReInit=*/ bReinitPose);
		}
	}

	
	
	// Let observers know, e.g., if they need to apply team coloring or similar
	OnCharacterPartsChanged.Broadcast(this);
}
