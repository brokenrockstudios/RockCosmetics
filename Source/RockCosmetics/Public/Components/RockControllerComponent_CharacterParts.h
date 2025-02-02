// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ControllerComponent.h"
#include "RockControllerComponent_CharacterParts.generated.h"


enum class ERockCharacterPartSource : uint8;
struct FRockCharacterPart;
class URockPawnComponent_CharacterParts;
struct FRockControllerCharacterPartEntry;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROCKCOSMETICS_API URockControllerComponent_CharacterParts : public UControllerComponent
{
	GENERATED_BODY()

public:
	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	// Adds a character part to the actor that owns this customization component, should be called on the authority only
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	void AddCharacterPart(const FRockCharacterPart& NewPart);

	// Removes a previously added character part from the actor that owns this customization component, should be called on the authority only
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	void RemoveCharacterPart(const FRockCharacterPart& PartToRemove);

	// Removes all added character parts, should be called on the authority only
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	void RemoveAllCharacterParts();

	// Applies relevant developer settings if in PIE
	void ApplyDeveloperSettings();

protected:
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	TArray<FRockControllerCharacterPartEntry> CharacterParts;

private:
	URockPawnComponent_CharacterParts* GetPawnCustomizer() const;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void AddCharacterPartInternal(const FRockCharacterPart& NewPart, ERockCharacterPartSource Source);

	void AddCheatPart(const FRockCharacterPart& NewPart, bool bSuppressNaturalParts);
	void ClearCheatParts();

	void SetSuppressionOnNaturalParts(bool bSuppressed);

	friend class URockCosmeticCheats;
	friend class ULyraCosmeticCheats;
};
