// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#include "Cheats/RockCosmeticDeveloperSettings.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Components/RockControllerComponent_CharacterParts.h"
#include "RockCosmeticDevelopmentStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockCosmeticDeveloperSettings)

#define LOCTEXT_NAMESPACE "RockCosmeticCheats"

URockCosmeticDeveloperSettings::URockCosmeticDeveloperSettings()
{
}

FName URockCosmeticDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

#if WITH_EDITOR

void URockCosmeticDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ApplySettings();
}

void URockCosmeticDeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);

	ApplySettings();
}

void URockCosmeticDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ApplySettings();
}

void URockCosmeticDeveloperSettings::ApplySettings()
{
	if (GIsEditor && (GEngine != nullptr))
	{
		ReapplyLoadoutIfInPIE();
	}
}

void URockCosmeticDeveloperSettings::ReapplyLoadoutIfInPIE()
{
#if WITH_SERVER_CODE
	// Update the loadout on all players
	UWorld* ServerWorld = URockCosmeticDevelopmentStatics::FindPlayInEditorAuthorityWorld();
	// @TODO: Switch to the ForEachAuthorityWorld function after testing.
	// URockCosmeticDevelopmentStatics::ForEachAuthorityWorld([&](UWorld* ServerWorld){
	if (ServerWorld != nullptr)
	{
		ServerWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([=]()
			{
				for (TActorIterator<APlayerController> PCIterator(ServerWorld); PCIterator; ++PCIterator)
				{
					if (APlayerController* PC = *PCIterator)
					{
						if (URockControllerComponent_CharacterParts* CosmeticComponent = PC->FindComponentByClass<URockControllerComponent_CharacterParts>())
						{
							CosmeticComponent->ApplyDeveloperSettings();
						}
					}
				}
			}));
	}
	// });
#endif	// WITH_SERVER_CODE
}

void URockCosmeticDeveloperSettings::OnPlayInEditorStarted() const
{
	// Show a notification toast to remind the user that there's an experience override set
	if (CheatCosmeticCharacterParts.Num() > 0)
	{
		FNotificationInfo Info(LOCTEXT("CosmeticOverrideActive", "Applying Cosmetic Override"));
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

