// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#include "Cheats/RockCosmeticCheats.h"

#include "Components/RockControllerComponent_CharacterParts.h"
#include "Cosmetics/RockCharacterPartTypes.h"


#include "RockCosmeticDevelopmentStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockCosmeticCheats)

//////////////////////////////////////////////////////////////////////
// URockCosmeticCheats

URockCosmeticCheats::URockCosmeticCheats()
{
#if UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[](UCheatManager* CheatManager)
			{
				CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
			}));
	}
#endif
}

void URockCosmeticCheats::AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
	if (URockControllerComponent_CharacterParts* CosmeticComponent = GetCosmeticComponent())
	{
		TSubclassOf<AActor> PartClass = URockCosmeticDevelopmentStatics::FindClassByShortName<AActor>(AssetName);
		if (PartClass != nullptr)
		{
			FRockCharacterPart Part;
			Part.PartClass = PartClass;

			CosmeticComponent->AddCheatPart(Part, bSuppressNaturalParts);
		}
	}
#endif	
}

void URockCosmeticCheats::ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts)
{
	ClearCharacterPartOverrides();
	AddCharacterPart(AssetName, bSuppressNaturalParts);
}

void URockCosmeticCheats::ClearCharacterPartOverrides()
{
#if UE_WITH_CHEAT_MANAGER
	if (URockControllerComponent_CharacterParts* CosmeticComponent = GetCosmeticComponent())
	{
		CosmeticComponent->ClearCheatParts();
	}
#endif	
}

URockControllerComponent_CharacterParts* URockCosmeticCheats::GetCosmeticComponent() const
{
	if (APlayerController* PC = GetPlayerController())
	{
		return PC->FindComponentByClass<URockControllerComponent_CharacterParts>();
	}

	return nullptr;
}

