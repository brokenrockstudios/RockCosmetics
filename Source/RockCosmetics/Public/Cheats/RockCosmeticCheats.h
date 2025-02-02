// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "GameFramework/CheatManager.h"

#include "RockCosmeticCheats.generated.h"

class URockControllerComponent_CharacterParts;
class ULyraControllerComponent_CharacterParts;
class UObject;
struct FFrame;

/** Cheats related to bots */
UCLASS(NotBlueprintable)
class URockCosmeticCheats final : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	URockCosmeticCheats();

	// Adds a character part
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

	// Replaces previous cheat parts with a new one
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

	// Clears any existing cheats
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void ClearCharacterPartOverrides();

private:
	URockControllerComponent_CharacterParts* GetCosmeticComponent() const;
};
