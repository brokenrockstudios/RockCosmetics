// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "MuCO/CustomizableObjectParameterTypeDefinitions.h"

#include "ParameterValues.generated.h"

// We have to use custom ParameterValue instead of the default ones because the one that come with Mutable
// Does not have EditDefaults or any way for the user to edit it.

USTRUCT(BlueprintType)
struct FRockCustomizableObjectIntParameterValue
{
	GENERATED_BODY()
public:
	static constexpr int32 DEFAULT_PARAMETER_VALUE = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterValueName;

	// Same as ParameterValueName but for multidimensional params
	UPROPERTY()
	TArray<FString> ParameterRangeValueNames;
};

USTRUCT(BlueprintType)
struct FRockCustomizableObjectBoolParameterValue
{
	GENERATED_BODY()

	static constexpr bool DEFAULT_PARAMETER_VALUE = false;
	// This is basically an empty string
	inline static const FString DEFAULT_PARAMETER_VALUE_NAME;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = MutableCosmetic)
	FString ParameterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = MutableCosmetic)
	bool ParameterValue = true;
};

USTRUCT(BlueprintType)
struct FRockCustomizableObjectFloatParameterValue
{
	GENERATED_BODY()

	static constexpr float DEFAULT_PARAMETER_VALUE = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	float ParameterValue = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<float> ParameterRangeValues;
};


USTRUCT(BlueprintType)
struct FRockCustomizableObjectTextureParameterValue
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TObjectPtr<UTexture> ParameterValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<TObjectPtr<UTexture>> ParameterRangeValues;
};



USTRUCT(BlueprintType)
struct FRockCustomizableObjectVectorParameterValue
{
	GENERATED_BODY()

	inline static const FLinearColor DEFAULT_PARAMETER_VALUE = FLinearColor::Black;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FLinearColor ParameterValue = FLinearColor(ForceInit);
};


USTRUCT(BlueprintType)
struct FRockCustomizableObjectProjectorParameterValue
{
	GENERATED_BODY()

	inline static const FCustomizableObjectProjector DEFAULT_PARAMETER_VALUE = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FCustomizableObjectProjector Value;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<FCustomizableObjectProjector> RangeValues;
};

