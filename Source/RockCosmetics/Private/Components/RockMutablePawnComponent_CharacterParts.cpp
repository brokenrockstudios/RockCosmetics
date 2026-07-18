// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#include "Components/RockMutablePawnComponent_CharacterParts.h"

#include "RockCosmeticLogging.h"
#include "Actors/RockMutableTaggedActor.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableObjectSystem.h"
#include "Mutable/MutableLayers.h"
#include "Mutable/MutableOptions.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockMutablePawnComponent_CharacterParts)

namespace
{
	// Bounds ExecuteRecompose's self-retry while waiting for CustomizableObjectInstance to become resolvable.
	constexpr int32 MaxExecuteRecomposeRetries = 10;
}

URockMutablePawnComponent_CharacterParts::URockMutablePawnComponent_CharacterParts()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void URockMutablePawnComponent_CharacterParts::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicate the cosmetic parameter DATA, not a baked byte descriptor. The authority (including a dedicated
	// server, which never runs Mutable) just mutates this array; each client composes its own mesh from it locally.
	// FRockMutableCosmeticEntryList is a FastArraySerializer, so only added/removed/changed entries are sent.
	DOREPLIFETIME(ThisClass, CosmeticMutableEntries);
}

void URockMutablePawnComponent_CharacterParts::OnRegister()
{
	Super::OnRegister();

	if (!IsTemplate())
	{
		CosmeticMutableEntries.SetOwnerComponent(this);
	}
}

void URockMutablePawnComponent_CharacterParts::BroadcastChanged()
{
	Super::BroadcastChanged();

	for (const FRockAppliedCharacterPartEntry& Entry : CharacterPartList.Entries)
	{
		if (Entry.SpawnedComponent == nullptr)
		{
			UE_LOG(LogRockCosmetic, Warning, TEXT("SpawnedComponent is null"));
			continue;
		}
		// It's a primary character
		const ARockMutableTaggedActor* mutableTaggedActor = Cast<ARockMutableTaggedActor>(Entry.SpawnedComponent->GetChildActor());
		if (mutableTaggedActor == nullptr)
		{
			UE_LOG(LogRockCosmetic, Warning, TEXT("MutableTaggedActor is null"));
			continue;
		}

		UCustomizableObjectInstance* instance = mutableTaggedActor->GetCustomizableObjectInstance();
		if (instance == nullptr)
		{
			// This always triggers on first broadcast, so we can skip/ignore this. ExecuteRecompose will retry
			// on its own (via ResolveCustomizableObjectInstance) until this resolves, so we don't lose anything.
			UE_LOG(LogRockCosmetic, Verbose, TEXT("mutableTaggedActor's CustomizableObjectInstance is null"));
			continue;
		}
		CustomizableObjectInstance = instance;

		if (InitialAppearanceDescriptor.IsEmpty())
		{
			CaptureInitialAppearance();
		}

		// If there are cosmetic entries pending, ExecuteRecompose (triggered below) will call
		// UpdateSkeletalMeshAsync() itself once it's finished composing them. Doing it here too would just be a
		// wasted mesh build that gets immediately superseded. Only do it here when there's nothing to compose.
		if (CosmeticMutableEntries.Num() == 0)
		{
			if (const auto usage = mutableTaggedActor->GetUsage())
			{
				usage->UpdateSkeletalMeshAsync();
			}
		}
	}

	// Flush any cosmetic entries that were applied/replicated before this instance was ready (e.g. equipment
	// applied at spawn, or entries received on a client before its instance was built), so they compose now.
	// This runs even if none of the entries above resolved an instance this pass. ExecuteRecompose retries
	// via ResolveCustomizableObjectInstance until one is available, instead of silently dropping the request.
	if (CosmeticMutableEntries.Num() > 0)
	{
		RequestRecompose();
	}

	// TODO:
	// Support non TaggedActor character parts, which would require a different way to get the CustomizableObjectInstance and bind to updates. 
	// What if we had an interface that provided us with a list of CustomizableObjectInstances to bind to? That way we could support both the tagged actor 
	// approach and any other actor/component that implements that interface. We would just loop through all provided instances and bind to their update delegates.
}

void URockMutablePawnComponent_CharacterParts::ApplyOptionToDescriptor(FCustomizableObjectInstanceDescriptor& WorkingDescriptor, const FRockMutableOption& Option)
{
	for (const auto& boolParam : Option.BoolParameters)
	{
		WorkingDescriptor.SetBoolParameterSelectedOption(boolParam.ParameterName, boolParam.ParameterValue);
	}
	for (const auto& intParam : Option.IntParameters)
	{
		
		WorkingDescriptor.SetIntParameterSelectedOption(intParam.ParameterName, intParam.ParameterValueName);
		// TODO: Do we still leverage the original parameter value when doing this? 
		// Or offset the remaining?  Or do the following instead of the above line?
		// NOTE: Need an actual test case to verify proper use case
		// for (int32 index = 0; index < intParam.ParameterRangeValueNames.Num(); ++index)
		// {
		// 	WorkingDescriptor.SetIntParameterSelectedOption(intParam.ParameterName, intParam.ParameterRangeValueNames[index], index);
		// }
	}
	for (const auto& floatParam : Option.FloatParameters)
	{
		WorkingDescriptor.SetFloatParameterSelectedOption(floatParam.ParameterName, floatParam.ParameterValue);
		
		// for (int32 index = 0; index < floatParam.ParameterRangeValues.Num(); ++index)
		// {
		// 	WorkingDescriptor.SetFloatParameterSelectedOption(floatParam.ParameterName, floatParam.ParameterRangeValues[index], index);
		// }
	}
	for (const auto& textureParam : Option.TextureParameters)
	{
		WorkingDescriptor.SetTextureParameterSelectedOption(textureParam.ParameterName, textureParam.ParameterValue, INDEX_NONE);
		
		// for (int32 index = 0; index < textureParam.ParameterRangeValues.Num(); ++index)
		// {
		// 	WorkingDescriptor.SetTextureParameterSelectedOption(textureParam.ParameterName, textureParam.ParameterRangeValues[index], index);
		// }
	}
	for (const auto& vectorParam : Option.VectorParameters)
	{
		WorkingDescriptor.SetVectorParameterSelectedOption(vectorParam.ParameterName, vectorParam.ParameterValue);
	}
	for (const auto& Param : Option.ProjectorParameters)
	{
		WorkingDescriptor.SetProjectorValue(
			Param.ParameterName,
			FVector(Param.Value.Position),
			FVector(Param.Value.Direction),
			FVector(Param.Value.Up),
			FVector(Param.Value.Scale),
			Param.Value.Angle);
		
		// for (int32 index = 0; index < Param.RangeValues.Num(); index++)
		// {
		// 	WorkingDescriptor.SetProjectorValue(
		// 		Param.ParameterName,
		// 		FVector(Param.RangeValues[index].Position),
		// 		FVector(Param.RangeValues[index].Direction),
		// 		FVector(Param.RangeValues[index].Up),
		// 		FVector(Param.RangeValues[index].Scale),
		// 		Param.RangeValues[index].Angle,
		// 		index);
		// }
	}
}

FRockCosmeticHandle URockMutablePawnComponent_CharacterParts::AddCosmeticEntry(int32 LayerIndex, const FRockMutableOption& MutableOption)
{
	// AddEntry marks the new entry dirty for replication and requests a recompose on our side.
	return CosmeticMutableEntries.AddEntry(LayerIndex, MutableOption);
}

void URockMutablePawnComponent_CharacterParts::RemoveCosmeticEntry(const FRockCosmeticHandle& CosmeticHandle)
{
	CosmeticMutableEntries.RemoveByHandle(CosmeticHandle);
}

void URockMutablePawnComponent_CharacterParts::RemoveCosmeticLayer(int32 LayerIndex)
{
	CosmeticMutableEntries.RemoveByLayer(LayerIndex);
}

UCustomizableObjectInstance* URockMutablePawnComponent_CharacterParts::ResolveCustomizableObjectInstance()
{
	if (UCustomizableObjectInstance* Cached = CustomizableObjectInstance.Get())
	{
		return Cached;
	}

	for (const FRockAppliedCharacterPartEntry& Entry : CharacterPartList.Entries)
	{
		if (Entry.SpawnedComponent == nullptr)
		{
			continue;
		}
		if (const ARockMutableTaggedActor* mutableTaggedActor = Cast<ARockMutableTaggedActor>(Entry.SpawnedComponent->GetChildActor()))
		{
			if (UCustomizableObjectInstance* Instance = mutableTaggedActor->GetCustomizableObjectInstance())
			{
				CustomizableObjectInstance = Instance;
				return Instance;
			}
		}
	}
	return nullptr;
}

void URockMutablePawnComponent_CharacterParts::CaptureInitialAppearance()
{
	if (InitialAppearanceDescriptor.Num() > 0)
	{
		return;
	}

	UCustomizableObjectInstance* customObjectInstance = CustomizableObjectInstance.Get();
	if (customObjectInstance)
	{
		InitialAppearanceDescriptor.Reset(); // Ensure it's clean

		FMemoryWriter Writer(InitialAppearanceDescriptor);
		customObjectInstance->SaveDescriptor(Writer, false);

		UE_LOG(LogRockCosmetic, Log, TEXT("Base appearance captured (%d bytes)"), InitialAppearanceDescriptor.Num());
	}
}

void URockMutablePawnComponent_CharacterParts::RequestRecompose()
{
	if (bRecomposePending)
	{
		return;
	}
	bRecomposePending = true;

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::ExecuteRecompose);
}


void URockMutablePawnComponent_CharacterParts::ExecuteRecompose()
{
	bRecomposePending = false;

	// LAYER 0: Start with our saved Base Appearance
	UCustomizableObjectInstance* customObjectInstance = ResolveCustomizableObjectInstance();
	if (!customObjectInstance)
	{
		// The Mutable-tagged child actor may not have finished spawning yet. We probably want to rehook into the
		// broadcast even from mutable on non-dedicated servers in the future. However, until then this usually happens on the first
		// attempt right after a part is added or replicated. Retry for a bounded number of ticks instead of
		// silently dropping already-replicated cosmetics; a dedicated server never builds an instance, so don't
		// retry there. 
		if (!IsNetMode(NM_DedicatedServer) && RecomposeRetryCount < MaxExecuteRecomposeRetries)
		{
			++RecomposeRetryCount;
			RequestRecompose();
		}
		else
		{
			UE_LOG(LogRockCosmetic, Error, TEXT("URockMutablePawnComponent_CharacterParts::ExecuteRecompose: CustomizableObjectInstance is null, giving up after %d attempt(s)"), RecomposeRetryCount);
		}
		return;
	}
	RecomposeRetryCount = 0;

	if (InitialAppearanceDescriptor.IsEmpty())
	{
		CaptureInitialAppearance();
	}

	UCustomizableObject* CO = customObjectInstance->GetCustomizableObject();
	if (!CO)
	{
		UE_LOG(LogRockCosmetic, Error, TEXT("URockPawnComponent_CharacterParts::RefreshAppearance: CustomizableObject is null"));
		return;
	}

	FCustomizableObjectInstanceDescriptor WorkingDescriptor(*CO);
	{
		FMemoryReader memoryReader(InitialAppearanceDescriptor);
		WorkingDescriptor.LoadDescriptor(memoryReader);
	}

	// CosmeticMutableEntries' own array order isn't guaranteed to match layer priority (FastArraySerializer only
	// replicates added/removed/changed entries, not reorders), so sort here rather than relying on insertion order.
	for (const FRockMutableCosmeticEntry& CosmeticEntry : CosmeticMutableEntries.GetSortedEntries())
	{
		ApplyOptionToDescriptor(WorkingDescriptor, CosmeticEntry.Option);
	}

	// FINISH: Serialize, Load to Instance, and Bake
	{
		// FINISH: Serialize and Load
		TArray<uint8> TempBuffer;
		FMemoryWriter Writer(TempBuffer);
		WorkingDescriptor.SaveDescriptor(Writer, false);

		FMemoryReader Reader(TempBuffer);
		customObjectInstance->LoadDescriptor(Reader);
	}
	customObjectInstance->UpdateSkeletalMeshAsync();
}
