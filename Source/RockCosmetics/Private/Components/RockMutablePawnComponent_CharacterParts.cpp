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

URockMutablePawnComponent_CharacterParts::URockMutablePawnComponent_CharacterParts()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void URockMutablePawnComponent_CharacterParts::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InstanceDescriptor);
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

		CustomizableObjectInstance = mutableTaggedActor->GetCustomizableObjectInstance();
		if (CustomizableObjectInstance == nullptr)
		{
			// This always triggers on first broadcast, so we can skip/ignore this 
			UE_LOG(LogRockCosmetic, Verbose, TEXT("mutableTaggedActor's CustomizableObjectInstance is null"));
			continue;
		}

		if (InitialAppearanceDescriptor.IsEmpty())
		{
			CaptureInitialAppearance();
		}

		if (const auto usage = mutableTaggedActor->GetUsage())
		{
			usage->UpdatedDelegate.BindUObject(this, &ThisClass::OnCustomizableObjectUpdated);
			usage->UpdateSkeletalMeshAsync();
		}
	}
	
	// TODO:
	// Support non TaggedActor character parts, which would require a different way to get the CustomizableObjectInstance and bind to updates. 
	// What if we had an interface that provided us with a list of CustomizableObjectInstances to bind to? That way we could support both the tagged actor 
	// approach and any other actor/component that implements that interface. We would just loop through all provided instances and bind to their update delegates.
}

void URockMutablePawnComponent_CharacterParts::OnRep_InstanceDescriptor()
{
	UCustomizableObjectInstance* customObjectInstance = CustomizableObjectInstance.Get();
	if (!customObjectInstance)
	{
		UE_LOG(LogRockCosmetic, Warning, TEXT("URockPawnComponent_CharacterParts::OnRep_InstanceDescriptor: CustomizableObjectInstance is null"));
		return;
	}
	FMemoryReader Reader(InstanceDescriptor);
	customObjectInstance->LoadDescriptor(Reader);
	customObjectInstance->UpdateSkeletalMeshAsync();
}

void URockMutablePawnComponent_CharacterParts::SerializeAndSendInstanceDescriptor()
{
	UCustomizableObjectInstance* customObjectInstance = CustomizableObjectInstance.Get();
	if (!customObjectInstance)
	{
		UE_LOG(LogRockCosmetic, Warning, TEXT("URockPawnComponent_CharacterParts::SerializeAndSendInstanceDescriptor: CustomizableObjectInstance is null"));
		return;
	}
	InstanceDescriptor.Reset(InstanceDescriptor.Num());
	FMemoryWriter Writer(InstanceDescriptor);
	customObjectInstance->SaveDescriptor(Writer, false);

	// // If we wanted a client change propagate, we must send it to the server to that it replicates it to the other clients
	// if (GetOwnerRole()/*GetLocalRole()*/ < ROLE_Authority)
	// {
	// 	ServerRPCUpdateInstanceDescriptor(InstanceDescriptor);
	// }
}

bool URockMutablePawnComponent_CharacterParts::ServerRPCUpdateInstanceDescriptor_Validate(const TArray<uint8>& NewInstanceDescriptor)
{
	return true;
}

void URockMutablePawnComponent_CharacterParts::ServerRPCUpdateInstanceDescriptor_Implementation(const TArray<uint8>& NewInstanceDescriptor)
{
	InstanceDescriptor = NewInstanceDescriptor;
	ENetMode NetMode = GetWorld()->GetNetMode();

	if (NetMode != ENetMode::NM_DedicatedServer)
	{
		// No need to update the actual instance if this is a dedicated server with no graphics display
		OnRep_InstanceDescriptor();
	}
}

void URockMutablePawnComponent_CharacterParts::OnCustomizableObjectUpdated()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SerializeAndSendInstanceDescriptor();
	}
	else
	{
		UE_LOG(LogRockCosmetic, Warning, TEXT("URockPawnComponent_CharacterParts::OnCustomizableSkeletalUpdated: Not authority"));
	}
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
	}
	// TODO: more...
}

FRockCosmeticHandle URockMutablePawnComponent_CharacterParts::AddCosmeticEntry(int32 LayerIndex, const FRockMutableOption& MutableOption)
{
	CosmeticHandleCount++;
	FRockCosmeticHandle newHandle{CosmeticHandleCount};
	
	FRockMutableCosmeticEntry newEntry = FRockMutableCosmeticEntry{LayerIndex, newHandle, MutableOption};
	CosmeticMutableEntries.Add(newEntry);

	// Keep it sorted so RefreshAppearance is always a simple linear loop
	CosmeticMutableEntries.Sort();

	RequestRecompose();
	return newHandle;
}

void URockMutablePawnComponent_CharacterParts::RemoveCosmeticEntry(const FRockCosmeticHandle& CosmeticHandle)
{
	int32 removedCount = CosmeticMutableEntries.RemoveAll([CosmeticHandle](const FRockMutableCosmeticEntry& Entry)
	{
		return Entry.CosmeticHandle == CosmeticHandle;
	});

	if (removedCount > 0)
	{
		RequestRecompose();
	}
}

void URockMutablePawnComponent_CharacterParts::RemoveCosmeticLayer(int32 LayerIndex)
{
	int32 removedCount = CosmeticMutableEntries.RemoveAll([LayerIndex](const FRockMutableCosmeticEntry& Entry)
	{
		return Entry.LayerIndex == LayerIndex;
	});

	if (removedCount > 0)
	{
		RequestRecompose();
	}
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
	UCustomizableObjectInstance* customObjectInstance = CustomizableObjectInstance.Get();
	if (!customObjectInstance)
	{
		UE_LOG(LogRockCosmetic, Error, TEXT("URockPawnComponent_CharacterParts::RefreshAppearance: CustomizableObjectInstance is null"));
		return;
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

	// the Layers should already be appropriately sorted by priority, so we just loop through them and apply all the options we have stored for each layer
	for (const FRockMutableCosmeticEntry& CosmeticEntry : CosmeticMutableEntries)
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
