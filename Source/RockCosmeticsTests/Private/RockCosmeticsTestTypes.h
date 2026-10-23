// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/RockTaggedActor.h"
#include "AnimSets/RockSkeletonClassInterface.h"
#include "Components/RockPawnComponent_CharacterParts.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"

#include "RockCosmeticsTestTypes.generated.h"

/** A pawn with a plain root scene component, so character parts have something to attach to. */
UCLASS()
class ARCTestPawn : public APawn
{
	GENERATED_BODY()

public:
	ARCTestPawn()
	{
		SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	}
};

/** A part actor with a root component. Subclasses report cosmetic tags (looked up lazily: the tags are registered after the CDOs are built). */
UCLASS()
class ARCTestPartActor : public ARockTaggedActor
{
	GENERATED_BODY()

public:
	ARCTestPartActor()
	{
		SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	}
};

/** Reports Test.RockCosmetics.Hat and Test.RockCosmetics.Head. */
UCLASS()
class ARCTestHatActor : public ARCTestPartActor
{
	GENERATED_BODY()

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Hat")));
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Head")));
	}
};

/** Reports Test.RockCosmetics.Boots. */
UCLASS()
class ARCTestBootsActor : public ARCTestPartActor
{
	GENERATED_BODY()

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Boots")));
	}
};

/** Reports the skeleton class Cosmetic.SkeletonClass.TestRockCosmeticsA. */
UCLASS()
class ARCTestSkeletonAActor : public ARCTestPartActor
{
	GENERATED_BODY()

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsA")));
	}
};

/** Reports the skeleton class Cosmetic.SkeletonClass.TestRockCosmeticsB. */
UCLASS()
class ARCTestSkeletonBActor : public ARCTestPartActor
{
	GENERATED_BODY()

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsB")));
	}
};

/** A pawn that reports Cosmetic.SkeletonClass.TestRockCosmeticsB through IRockSkeletonClassInterface, like AFenNPCPawn. */
UCLASS()
class ARCTestSkeletonClassPawn : public ARCTestPawn, public IRockSkeletonClassInterface
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetSkeletonClass() const override
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsB"));
	}
};

/** Exposes the part list so a test can call the replication callbacks a client would receive. */
UCLASS()
class URCTestPartsComponent : public URockPawnComponent_CharacterParts
{
	GENERATED_BODY()

public:
	FRockCharacterPartList& List() { return CharacterPartList; }
};

/** Counts OnCharacterPartsChanged broadcasts. */
UCLASS()
class URCTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnPartsChanged(URockPawnComponent_CharacterParts* Component)
	{
		++Count;
		LastComponent = Component;
	}

	int32 Count = 0;
	TWeakObjectPtr<URockPawnComponent_CharacterParts> LastComponent;
};
