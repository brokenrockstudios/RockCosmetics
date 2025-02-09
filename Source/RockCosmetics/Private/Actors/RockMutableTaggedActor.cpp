// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#include "Actors/RockMutableTaggedActor.h"

#include "RockCosmeticFunctionLibrary.h"
#include "RockCosmeticLogging.h"
#include "Misc/DataValidation.h"
#include "MuCO/CustomizableObjectInstanceUsage.h"
#include "MuCO/CustomizableObjectSystem.h"

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
	
	Usages = URockCosmeticFunctionLibrary::BuildCustomizableActor(this, CustomizableObject);
}

void ARockMutableTaggedActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UCustomizableObjectInstanceUsage* usage = GetUsage())
	{
		// CharacterParts likely bound to this
		usage->UpdatedDelegate.Unbind();
	}
}

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