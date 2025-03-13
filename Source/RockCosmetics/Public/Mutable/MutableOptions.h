// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Parameters/ParameterValues.h"
#include "UObject/Object.h"
#include "MutableOptions.generated.h"

/**
 * 
 */

// I'd imagine many scenarios will likely only have 1 of these being modified.
// e.g. Selecting Chest piece #25 or Pants #12 as the only thing being modified
// However we have all the Mutable supported Parameters

struct FCustomizableObjectBoolParameterValue;
struct FCustomizableObjectProjectorParameterValue;
struct FCustomizableObjectVectorParameterValue;
struct FCustomizableObjectTextureParameterValue;

USTRUCT(BlueprintType)
struct FRockMutableOption
{
	GENERATED_BODY()
public:
	// @TODO: If we are able to add a DetailsCustomizer, we should be able to generate the 'dropdown' from the CustomizableObject for the user to select from
	// We probably need to indicate which CustomizableObject we are customizing
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modular Character Skin")
	//TWeakObjectPtr<class UCustomizableObject> CustomizableObject;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FRockCustomizableObjectBoolParameterValue> BoolParameters;

	// Although this contains 2 strings and no ints, its because basically underneath it is an enum|int that the FString is getting mapped too
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FRockCustomizableObjectIntParameterValue> IntParameters;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FRockCustomizableObjectFloatParameterValue> FloatParameters;

	// @TODO
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FCustomizableObjectTextureParameterValue> TextureParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FCustomizableObjectVectorParameterValue> VectorParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FCustomizableObjectProjectorParameterValue> ProjectorParameters;
};


// e.g. To attach this to an inventory item fragment, we could do something like this:
// UCLASS()
// class LYRAGAME_API UAEInventoryFragment_MutableCosmetic : public UArcInventoryItemFragment
// {
// 	GENERATED_BODY()
// public:
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutableCosmetic")
// 	FRockMutableOption MutableOption;
// };