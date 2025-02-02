// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RockCosmeticDevelopmentStatics.generated.h"

/**
 * A utility class providing static functions for cosmetic development in the RockCosmetics plugin.
 */
UCLASS()
class ROCKCOSMETICS_API URockCosmeticDevelopmentStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Finds the most appropriate play-in-editor world to run 'server' cheats on
	// This might be the only world if running standalone, the listen server, or the dedicated server
	static UWorld* FindPlayInEditorAuthorityWorld();
	static void ForEachAuthorityWorld(TFunctionRef<void(UWorld*)> Func);
	
	// Tries to find a class by a short name (with some heuristics to improve the usability when done via a cheat console)
	static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

	template <typename DesiredClass>
	static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
	{
		return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
	}

private:
	
	static TArray<FAssetData> GetAllBlueprints();
	static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};
