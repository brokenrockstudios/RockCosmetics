// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "RockCosmeticDeveloperSettings.generated.h"

struct FRockCharacterPart;
struct FPropertyChangedEvent;

class ULyraExperienceDefinition;

UENUM()
enum class ERockCosmeticCheatMode
{
	ReplaceParts,

	AddParts
};

/**
 * Cosmetic developer settings / editor cheats
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class URockCosmeticDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URockCosmeticDeveloperSettings();

	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

public:
	UPROPERTY(Transient, EditAnywhere)
	TArray<FRockCharacterPart> CheatCosmeticCharacterParts;

	UPROPERTY(Transient, EditAnywhere)
	ERockCosmeticCheatMode CheatMode;

#if WITH_EDITOR
public:
	// Called by the editor engine to let us pop reminder notifications when cheats are active
	ROCKCOSMETICS_API void OnPlayInEditorStarted() const;

private:
	void ApplySettings();
	void ReapplyLoadoutIfInPIE();
#endif

public:
	//~UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
#endif
	//~End of UObject interface

private:


};
