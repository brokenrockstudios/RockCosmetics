// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#include "Components/RockControllerComponent_CharacterParts.h"
#include "Components/RockPawnComponent_CharacterParts.h"
#include "GameFramework/CheatManagerDefines.h"
#include "RockControllerCharacterPart.h"
#include "Cheats/RockCosmeticDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockControllerComponent_CharacterParts)

//////////////////////////////////////////////////////////////////////


void URockControllerComponent_CharacterParts::BeginPlay()
{
	Super::BeginPlay();

	// Listen for pawn possession changed events
	if (HasAuthority())
	{
		if (AController* OwningController = GetController<AController>())
		{
			OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);

			if (APawn* ControlledPawn = GetPawn<APawn>())
			{
				OnPossessedPawnChanged(nullptr, ControlledPawn);
			}
		}

		ApplyDeveloperSettings();
	}
}

void URockControllerComponent_CharacterParts::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllCharacterParts();
	Super::EndPlay(EndPlayReason);
}

URockPawnComponent_CharacterParts* URockControllerComponent_CharacterParts::GetPawnCustomizer() const
{
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		return ControlledPawn->FindComponentByClass<URockPawnComponent_CharacterParts>();
	}
	return nullptr;
}

void URockControllerComponent_CharacterParts::AddCharacterPart(const FRockCharacterPart& NewPart)
{
	AddCharacterPartInternal(NewPart, ERockCharacterPartSource::Natural);
}

void URockControllerComponent_CharacterParts::AddCharacterPartInternal(const FRockCharacterPart& NewPart, ERockCharacterPartSource Source)
{
	FRockControllerCharacterPartEntry& NewEntry = CharacterParts.AddDefaulted_GetRef();
	NewEntry.Part = NewPart;
	NewEntry.Source = Source;

	if (URockPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
	{
		if (NewEntry.Source != ERockCharacterPartSource::NaturalSuppressedViaCheat)
		{
			NewEntry.Handle = PawnCustomizer->AddCharacterPart(NewPart);
		}
	}

}

void URockControllerComponent_CharacterParts::RemoveCharacterPart(const FRockCharacterPart& PartToRemove)
{
	for (auto EntryIt = CharacterParts.CreateIterator(); EntryIt; ++EntryIt)
	{
		if (FRockCharacterPart::AreEquivalentParts(EntryIt->Part, PartToRemove))
		{
			if (URockPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
			{
				PawnCustomizer->RemoveCharacterPart(EntryIt->Handle);
			}

			EntryIt.RemoveCurrent();
			break;
		}
	}
}

void URockControllerComponent_CharacterParts::RemoveAllCharacterParts()
{
	if (URockPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
	{
		for (FRockControllerCharacterPartEntry& Entry : CharacterParts)
		{
			PawnCustomizer->RemoveCharacterPart(Entry.Handle);
		}
	}

	CharacterParts.Reset();
}

void URockControllerComponent_CharacterParts::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// Remove from the old pawn
	if (URockPawnComponent_CharacterParts* OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<URockPawnComponent_CharacterParts>() : nullptr)
	{
		for (FRockControllerCharacterPartEntry& Entry : CharacterParts)
		{
			OldCustomizer->RemoveCharacterPart(Entry.Handle);
			Entry.Handle.Reset();
		}
	}

	// Apply to the new pawn
	if (URockPawnComponent_CharacterParts* NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<URockPawnComponent_CharacterParts>() : nullptr)
	{
		for (FRockControllerCharacterPartEntry& Entry : CharacterParts)
		{
			// Don't readd if it's already there, this can get called with a null oldpawn
			if (!Entry.Handle.IsValid() && Entry.Source != ERockCharacterPartSource::NaturalSuppressedViaCheat)
			{
				Entry.Handle = NewCustomizer->AddCharacterPart(Entry.Part);
			}
		}
	}
}

void URockControllerComponent_CharacterParts::ApplyDeveloperSettings()
{
#if UE_WITH_CHEAT_MANAGER
	const URockCosmeticDeveloperSettings* Settings = GetDefault<URockCosmeticDeveloperSettings>();

	// Suppress or unsuppress natural parts if needed
	const bool bSuppressNaturalParts = (Settings->CheatMode == ERockCosmeticCheatMode::ReplaceParts) && (Settings->CheatCosmeticCharacterParts.Num() > 0);
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);

	// Remove anything added by developer settings and re-add it
	URockPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer();
	for (auto It = CharacterParts.CreateIterator(); It; ++It)
	{
		if (It->Source == ERockCharacterPartSource::AppliedViaDeveloperSettingsCheat)
		{
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(It->Handle);
			}
			It.RemoveCurrent();
		}
	}

	// Add new parts
	for (const FRockCharacterPart& PartDesc : Settings->CheatCosmeticCharacterParts)
	{
		AddCharacterPartInternal(PartDesc, ERockCharacterPartSource::AppliedViaDeveloperSettingsCheat);
	}
#endif
}


void URockControllerComponent_CharacterParts::AddCheatPart(const FRockCharacterPart& NewPart, bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);
	AddCharacterPartInternal(NewPart, ERockCharacterPartSource::AppliedViaCheatManager);
#endif
}

void URockControllerComponent_CharacterParts::ClearCheatParts()
{
#if UE_WITH_CHEAT_MANAGER
	URockPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer();

	// Remove anything added by cheat manager cheats
	for (auto It = CharacterParts.CreateIterator(); It; ++It)
	{
		if (It->Source == ERockCharacterPartSource::AppliedViaCheatManager)
		{
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(It->Handle);
			}
			It.RemoveCurrent();
		}
	}

	ApplyDeveloperSettings();
#endif
}

void URockControllerComponent_CharacterParts::SetSuppressionOnNaturalParts(bool bSuppressed)
{
#if UE_WITH_CHEAT_MANAGER
	URockPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer();

	for (FRockControllerCharacterPartEntry& Entry : CharacterParts)
	{
		if ((Entry.Source == ERockCharacterPartSource::Natural) && bSuppressed)
		{
			// Suppress
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(Entry.Handle);
				Entry.Handle.Reset();
			}
			Entry.Source = ERockCharacterPartSource::NaturalSuppressedViaCheat;
		}
		else if ((Entry.Source == ERockCharacterPartSource::NaturalSuppressedViaCheat) && !bSuppressed)
		{
			// Unsuppress
			if (PawnCustomizer != nullptr)
			{
				Entry.Handle = PawnCustomizer->AddCharacterPart(Entry.Part);
			}
			Entry.Source = ERockCharacterPartSource::Natural;
		}
	}
#endif
}
