// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#include "Components/RockMutablePawnComponent_CharacterParts.h"

#include "RockCosmeticLogging.h"
#include "Actors/RockMutableTaggedActor.h"
#include "MuCO/CustomizableObjectSystem.h"
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
		if(mutableTaggedActor == nullptr)
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
		
		if (const auto usage = mutableTaggedActor->GetUsage())
		{
			usage->UpdatedDelegate.BindUObject(this, &ThisClass::OnCustomizableObjectUpdated);
			usage->UpdateSkeletalMeshAsync();
		}
	}
}

void URockMutablePawnComponent_CharacterParts::OnRep_InstanceDescriptor()
{
	UCustomizableObjectInstance* customObjectInstance = CustomizableObjectInstance.Get();
	if(!customObjectInstance)
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
	if(!customObjectInstance)
	{
		UE_LOG (LogRockCosmetic, Warning, TEXT("URockPawnComponent_CharacterParts::SerializeAndSendInstanceDescriptor: CustomizableObjectInstance is null"));
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

void URockMutablePawnComponent_CharacterParts::AddCharacterPartFromMutableOption(const FRockMutableOption& NewPart)
{
	auto customObjectInstance = CustomizableObjectInstance.Get();
	if( customObjectInstance)
	{
		 for (const auto& boolParam : NewPart.BoolParameters)
		 {
		// 	// boolParam.Id ?
		 	CustomizableObjectInstance->SetBoolParameterSelectedOption(boolParam.ParameterName, boolParam.ParameterValue);
		 }
		for (const auto& intParam : NewPart.IntParameters)
		{
			// intParam.Id ?
			customObjectInstance->SetIntParameterSelectedOption(intParam.ParameterName, intParam.ParameterValueName);

			// Something with ParameterRangeValueNames?
			// Only useful on multidimensional params
			// for (int32 i= 0; i < intParam.ParameterRangeValueNames.Num(); i++)
			// {
			// 	FString rangeValue = intParam.ParameterRangeValueNames[i];
			// 	CustomizableObjectInstance->SetIntParameterSelectedOption(intParam.ParameterName, rangeValue, i);
			// }
			
		}
		//
		// for (const auto& floatParam : NewPart.FloatParameters)
		// {
		// 	CustomizableObjectInstance->SetFloatParameterSelectedOption(floatParam.ParameterName, floatParam.ParameterValue);
		// 	// Unused ParameterRangeValueNames
		// }
		// for (const auto& textureParam : NewPart.TextureParameters)
		// {
		// 	CustomizableObjectInstance->SetTextureParameterSelectedOption(textureParam.ParameterName, textureParam.ParameterValue.ToString());
		// }
		// for (const auto& vectorParam : NewPart.VectorParameters)
		// {
		// 	// Vector and LinearColor are same thing according to Mutable
		// 	CustomizableObjectInstance->SetColorParameterSelectedOption(vectorParam.ParameterName, vectorParam.ParameterValue);
		// 	//CustomizableObjectInstance->SetVectorParameterSelectedOption(vectorParam.ParameterName, vectorParam.ParameterValue);
		// }
		// for (const auto& projectorParam : NewPart.ProjectorParameters)
		// {
		// 	CustomizableObjectInstance->SetProjectorValue(projectorParam.ParameterName,
		// 		FVector(projectorParam.Value.Position),
		// 		FVector(projectorParam.Value.Direction),
		// 		FVector(projectorParam.Value.Up),
		// 		FVector(projectorParam.Value.Scale),
		// 		projectorParam.Value.Angle);
		// 	// RangeValues could contain a variety of values
		// 	// unused TArray<FCustomizableObjectProjector> RangeValues; 
		// }
		
		customObjectInstance->UpdateSkeletalMeshAsync();
	}
}
void URockMutablePawnComponent_CharacterParts::RemoveCharacterPartFromMutableOption(const FRockMutableOption& NewPart)
{
	UCustomizableObjectInstance* customObjectInstance = CustomizableObjectInstance.Get();
	if( customObjectInstance)
	{
		for (const auto& intParam : NewPart.IntParameters)
		{
			// What if there is no None?  OR is there always one?
			// What should the fallback or how to reset to default?
			// Going back to None is probably fine on clothing, but not as good maybe with other things? I dunno
			// If there is no 'None', then we might need to find option at 0, or equivalent
			customObjectInstance->SetIntParameterSelectedOption(intParam.ParameterName, "None");
		}
	}
}

void URockMutablePawnComponent_CharacterParts::ShowCharacterInformation(bool bFullInfo, bool ShowMaterialInfo)
{
	UCustomizableObjectSystem::GetInstance()->LogShowData(bFullInfo, ShowMaterialInfo);
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
		UE_LOG (LogRockCosmetic, Warning, TEXT("URockPawnComponent_CharacterParts::OnCustomizableSkeletalUpdated: Not authority"));
	}
}

