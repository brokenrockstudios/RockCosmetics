// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#include "RockCosmeticFunctionLibrary.h"

#include "RockCosmeticLogging.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableObjectInstanceUsage.h"

TArray<UCustomizableObjectInstanceUsage*> URockCosmeticFunctionLibrary::AddCustomizableInstanceUsagesToActor(
	AActor* Actor, const UCustomizableObject* CustomizableObject, UCustomizableObjectInstance* CustomizableObjectInstance)
{
	TArray<TObjectPtr<UCustomizableObjectInstanceUsage>> Usages;
	TArray<USkeletalMeshComponent*> Components;
	
	for ( int32 componentIndex = 0; componentIndex < CustomizableObjectInstance->GetNumComponents(); ++componentIndex)
	{
		FString SkeletalMeshComponentName = FString::Printf(TEXT("Skeletal Mesh Component %d"), componentIndex);

		USkeletalMeshComponent* component;

		// The Root RockSkeletalActor should already have the RootComponent being a SkeletalMeshComponent
		// This will add any additional SkeletalMeshComponents as needed by the CustomizableObjectInstance
		if (componentIndex == 0)
		{
			component = Cast<USkeletalMeshComponent>(Actor->GetRootComponent());
			ensureMsgf(component, TEXT("Failed to get SkeletalMeshComponent as the RootComponent. The RockCosmetics expects the root component to be a SkeletalMeshComponent"));
			component->Rename(*SkeletalMeshComponentName);
		}
		else
		{
			component = NewObject<USkeletalMeshComponent>(Actor, USkeletalMeshComponent::StaticClass(), FName(*SkeletalMeshComponentName));
			component->CreationMethod = EComponentCreationMethod::SimpleConstructionScript;
			component->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			component->RegisterComponent();
		}
		
		if (!component)
		{
			UE_LOG(LogRockCosmetic, Error, TEXT("Failed to create SkeletalMeshComponent %d"), componentIndex);
			continue;
		}

		const FName& ComponentName = CustomizableObject->GetComponentName(componentIndex);

		FString CustomizableComponentName = FString::Printf(TEXT("Customizable Object Instance Usage %d"), componentIndex);
		UCustomizableObjectInstanceUsage* usage = NewObject<UCustomizableObjectInstanceUsage>(Actor, UCustomizableObjectInstanceUsage::StaticClass(), FName(*CustomizableComponentName));
		usage->AttachTo(component);
		usage->SetCustomizableObjectInstance(CustomizableObjectInstance);
		usage->SetComponentName(ComponentName);
		Components.Add(component);
		Usages.Add(usage);
	}
	
	if (!Components.IsEmpty())
	{
		// Skip the first one, the root, it's the leader
		for (int32 ComponentIndex = 1; ComponentIndex < Components.Num(); ++ComponentIndex)
		{
			Components[ComponentIndex]->SetLeaderPoseComponent(Components[0]);
		}
	}
	return Usages;
}

TArray<UCustomizableObjectInstanceUsage*> URockCosmeticFunctionLibrary::BuildCustomizableActor(
	AActor* Actor, UCustomizableObject* CustomizableObject)
{
	UCustomizableObjectInstance* instance = CustomizableObject->CreateInstance();
	return AddCustomizableInstanceUsagesToActor (Actor, CustomizableObject, instance);
}

TArray<UCustomizableObjectInstanceUsage*> URockCosmeticFunctionLibrary::BuildCustomizableActorWithInstance(
	AActor* Actor, UCustomizableObjectInstance* CustomizableObjectInstance)
{
	const UCustomizableObject* customizableObject = CustomizableObjectInstance->GetCustomizableObject();
	UCustomizableObjectInstance* instance = CustomizableObjectInstance->Clone();
	return AddCustomizableInstanceUsagesToActor (Actor, customizableObject, instance);
}
