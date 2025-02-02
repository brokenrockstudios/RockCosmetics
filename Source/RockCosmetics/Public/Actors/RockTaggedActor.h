// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"

#include "RockTaggedActor.generated.h"


// Base TaggedActor inspired from LyraTaggedActor
UCLASS(ClassGroup = RockCosmetic, Blueprintable, ComponentWrapperClass)
class ROCKCOSMETICS_API ARockTaggedActor : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	ARockTaggedActor();

	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface

	//~UObject interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~End of UObject interface

protected:
	// Gameplay-related tags associated with this actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	FGameplayTagContainer StaticGameplayTags;
};

