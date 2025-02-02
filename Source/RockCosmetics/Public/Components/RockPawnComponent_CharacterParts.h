// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Cosmetics/RockCharacterPartList.h"
#include "Cosmetics/RockCosmeticAnimationTypes.h"

#include "RockPawnComponent_CharacterParts.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRockSpawnedCharacterPartsChanged, URockPawnComponent_CharacterParts*, ComponentWithChangedParts);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROCKCOSMETICS_API URockPawnComponent_CharacterParts : public UPawnComponent
{
	GENERATED_BODY()

public:
	URockPawnComponent_CharacterParts(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;
	//~End of UActorComponent interface

	// Adds a character part to the actor that owns this customization component, should be called on the authority only
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	FRockCharacterPartHandle AddCharacterPart(const FRockCharacterPart& NewPart);

	// Removes a previously added character part from the actor that owns this customization component, should be called on the authority only
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	void RemoveCharacterPart(FRockCharacterPartHandle Handle);

	// Removes all added character parts, should be called on the authority only
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	void RemoveAllCharacterParts();

	// Gets the list of all spawned character parts from this component
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintCosmetic, Category=Cosmetics)
	TArray<AActor*> GetCharacterPartActors() const;

	// If the parent actor is derived from ACharacter, returns the Mesh component, otherwise nullptr
	USkeletalMeshComponent* GetParentMeshComponent() const;

	// Returns the scene component to attach the spawned actors to
	// If the parent actor is derived from ACharacter, we'll use the Mesh component, otherwise the root component
	USceneComponent* GetSceneComponentToAttachTo() const;

	// Returns the set of combined gameplay tags from attached character parts, optionally filtered to only tags that start with the specified root
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintCosmetic, Category=Cosmetics)
	FGameplayTagContainer GetCombinedTags(FGameplayTag RequiredPrefix) const;

	virtual void BroadcastChanged();

public:
	// Delegate that will be called when the list of spawned character parts has changed
	UPROPERTY(BlueprintAssignable, Category=Cosmetics, BlueprintCallable)
	FRockSpawnedCharacterPartsChanged OnCharacterPartsChanged;

protected:
	// List of character parts
	UPROPERTY(Replicated, Transient)
	FRockCharacterPartList CharacterPartList;

	// Rules for how to pick a body style mesh for animation to play on, based on character part cosmetics tags
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	FRockAnimBodyStyleSelectionSet BodyMeshes;

};
