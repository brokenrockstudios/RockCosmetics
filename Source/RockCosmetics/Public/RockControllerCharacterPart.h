// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Cosmetics/RockCharacterPartTypes.h"
#include "UObject/Object.h"
#include "RockControllerCharacterPart.generated.h"

enum class ERockCharacterPartSource : uint8
{
	Natural,
	NaturalSuppressedViaCheat,
	AppliedViaDeveloperSettingsCheat,
	AppliedViaCheatManager
};

// A character part requested on a controller component
USTRUCT()
struct FRockControllerCharacterPartEntry
{
	GENERATED_BODY()

	FRockControllerCharacterPartEntry()
	{}

public:
	// The character part being represented
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FRockCharacterPart Part;

	// The handle if already applied to a pawn
	FRockCharacterPartHandle Handle;

	// The source of this part
	ERockCharacterPartSource Source = ERockCharacterPartSource::Natural;
};
