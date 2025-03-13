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
	//UPROPERTY()
	//TArray<FString> ParameterRangeValueNames;
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

	//UPROPERTY(Category = CustomizableObjectFloatParameterValue, VisibleAnywhere)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	TArray<float> ParameterRangeValues;
};


USTRUCT(BlueprintType)
struct FRockCustomizableObjectProjectorParameterValue
{
	GENERATED_BODY()

	inline static const FCustomizableObjectProjector DEFAULT_PARAMETER_VALUE = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	FString ParameterName;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	//FCustomizableObjectProjector Value;
	
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MutableCosmetic")
	//TArray<FCustomizableObjectProjector> RangeValues;
};

//
// USTRUCT()
// struct CUSTOMIZABLEOBJECT_API FCustomizableObjectProjector
// {
// 	GENERATED_USTRUCT_BODY()
//
// 	UPROPERTY()
// 	FVector3f Position = FVector3f(0, 0, 0);
//
// 	UPROPERTY()
// 	FVector3f Direction = FVector3f(1, 0, 0);
//
// 	UPROPERTY()
// 	FVector3f Up = FVector3f(0, 1, 0);
//
// 	UPROPERTY()
// 	FVector3f Scale = FVector3f(10, 10, 100);
//
// 	UPROPERTY(EditAnywhere, Category = CustomizableObject)
// 	ECustomizableObjectProjectorType ProjectionType = ECustomizableObjectProjectorType::Planar;
//
// 	// Just for cylindrical projectors, in radians
// 	UPROPERTY()
// 	float Angle = 2.0f * PI;
//
// 	bool operator==(const FCustomizableObjectProjector& Other) const = default;
// };