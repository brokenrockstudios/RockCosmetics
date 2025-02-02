// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "RockSkeletalTaggedActor.h"
#include "MuCO/CustomizableObjectInstanceUsage.h"
#include "RockMutableTaggedActor.generated.h"

class UCustomizableObjectInstanceUsage;

// Primary Mutable Character actor. Useful for the main player character
UCLASS(ClassGroup = RockCosmetic, Blueprintable, ComponentWrapperClass)
class ROCKCOSMETICS_API ARockMutableTaggedActor : public ARockSkeletalTaggedActor
{
	GENERATED_BODY()
public:
	// ~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End AActor Interface

	
	/** This is the main customizable object that will be used to create the Mutable Character
	 * Please see Mutable Documentation for more information about creating a Customizable Object
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modular Character Skin")
	TObjectPtr<class UCustomizableObject> CustomizableObject = nullptr;
	
	/** This is the instance of the Customizable Object that will be used to create the Mutable Character
	 * This Instance contains all the customization data for the Mutable Character.
	 * CharacterParts fetches this Instance and replicates it using a serialized InstanceDescriptor
	 *
	 * return: The instance represented in all the actor components
	 */
	UFUNCTION(BlueprintCallable, Category = "Modular Character Skin")
	UCustomizableObjectInstance* GetCustomizableObjectInstance() const;
	
	/** Get the usage of the Mutable character. Majority of scenarios will likely only have one usage.
	 * 
	 * Multiple usages are untested at this time and will likely require additional setup
	 * In one of the Mutable sample, they separate the Body and Head thus having 2 usages.
	 * But this plugin has only been tested with one usage as a complete body.
	 */
	UFUNCTION(BlueprintCallable, Category = "Modular Character Skin")
	UCustomizableObjectInstanceUsage* GetUsage(int32 Index = 0) const;
	
protected:
	// CustomizableObjectInstanceUsage is used instead of a UCustomizableComponent to link a UCustomizableObjectInstance
	// and a USkeletalComponent so that the CustomizableObjectSystem takes care of updating it and its LODs, streaming, etc.
	// It's a UObject, so it's cheaper than the Component as it won't have to refresh its transforms every time it's moved.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modular Character Skin")
	TArray<TObjectPtr<UCustomizableObjectInstanceUsage>> Usages;

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

};


