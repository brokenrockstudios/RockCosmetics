// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#include "Actors/RockMutableTaggedActor.h"

#include "RockCosmeticFunctionLibrary.h"
#include "RockCosmeticLogging.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableObjectInstanceUsage.h"
#include "MuCO/CustomizableObjectSystem.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockMutableTaggedActor)

#define LOCTEXT_NAMESPACE "AEModularCharacterSkin"


void ARockMutableTaggedActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (!CustomizableObject)
	{
		UE_LOG (LogRockCosmetic, Error, TEXT("ARockMutableTaggedActor::PostInitializeComponents() - CustomizableObject is null"));
		return;
	}

#if WITH_EDITOR
	// In the editor the compiled data is loaded a few frames after the asset itself (e.g. on the first PIE of a
	// session), and it is dropped when a dependency changed. Until then the object reports no components, so building
	// now would leave this actor without a mesh, and parameters set on the instance would be lost. Let Mutable finish
	// it first: the request waits for the load and is skipped if the loaded data turns out to be up to date.
	// Cooked builds always ship compiled objects.
	if (!CustomizableObject->IsCompiled())
	{
		bWaitingForCustomizableObject = true;

		FCompileParams Params;
		Params.bSkipIfCompiled = true;
		Params.bSkipIfNotOutOfDate = false;
		Params.CallbackNative.BindUObject(this, &ThisClass::HandleCustomizableObjectCompiled);
		CustomizableObject->Compile(Params);
		return;
	}
#endif

	Usages = URockCosmeticFunctionLibrary::BuildCustomizableActor(this, CustomizableObject);
}

void ARockMutableTaggedActor::BeginPlay()
{
	Super::BeginPlay();

	// A CustomizableObject that is still compiling broadcasts from HandleCustomizableObjectCompiled instead.
	BroadcastInstanceReady();
}

void ARockMutableTaggedActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	bWaitingForCustomizableObject = false;
	OnCustomizableObjectInstanceReady.Clear();
	if (UCustomizableObjectInstanceUsage* usage = GetUsage())
	{
		// CharacterParts likely bound to this
		usage->UpdatedDelegate.Unbind();
	}
}

void ARockMutableTaggedActor::BroadcastInstanceReady()
{
	if (UCustomizableObjectInstance* instance = GetCustomizableObjectInstance())
	{
		// Blueprint first, so the parameters it sets are part of the appearance CharacterParts captures.
		K2_OnCustomizableObjectInstanceReady(instance);
		OnCustomizableObjectInstanceReady.Broadcast(this);
	}
}

#if WITH_EDITOR
void ARockMutableTaggedActor::HandleCustomizableObjectCompiled(const FCompileCallbackParams& Params)
{
	// Cleared by EndPlay: the PIE session ended before the object was ready.
	if (!bWaitingForCustomizableObject || IsActorBeingDestroyed())
	{
		return;
	}
	bWaitingForCustomizableObject = false;

	if (!CustomizableObject || !CustomizableObject->IsCompiled())
	{
		UE_LOG(LogRockCosmetic, Error, TEXT("ARockMutableTaggedActor::HandleCustomizableObjectCompiled() - %s failed to compile, %s stays without a mesh"),
			*GetNameSafe(CustomizableObject), *GetName());
		return;
	}

	Usages = URockCosmeticFunctionLibrary::BuildCustomizableActor(this, CustomizableObject);

	// Ready before BeginPlay (the request may complete synchronously): BeginPlay broadcasts.
	if (HasActorBegunPlay())
	{
		BroadcastInstanceReady();
	}
}
#endif

UCustomizableObjectInstance* ARockMutableTaggedActor::GetCustomizableObjectInstance() const
{
	for (const UCustomizableObjectInstanceUsage* Usage : Usages)
	{
		if (UCustomizableObjectInstance* COInstance = Usage->GetCustomizableObjectInstance())
		{
			return COInstance;
		}
	}
	return nullptr;
}

UCustomizableObjectInstanceUsage* ARockMutableTaggedActor::GetUsage(int32 Index) const
{
	
	if (Index >= 0 && Index < Usages.Num())
	{
		return Usages[Index];
	}
	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult ARockMutableTaggedActor::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (CustomizableObject == nullptr)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NoCustomizableObject", "No Customizable Object assigned!"));
	}
	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE