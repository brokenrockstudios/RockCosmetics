// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RockCosmeticFunctionLibrary.generated.h"

class UCustomizableObject;
class UCustomizableObjectInstance;
class UCustomizableObjectInstanceUsage;

/**
 * @brief Blueprint Function Library for Rock Cosmetics.
 *
 * This function library provides a collection of static utility functions that facilitate the construction and
 * configuration of actors with customizable objects and their associated usage components. These functions enable
 * developers to easily integrate customizable skeletal mesh components into actors via both C++ and Blueprint workflows.
 *
 * The library includes functions for:
 * - Creating and attaching customizable object instances to actors.
 * - Instantiating customizable objects directly.
 * - Cloning existing customizable object instances to prevent modification of original instances.
 *
 * @see BuildCustomizableActor(...) for details on how customizable actors are constructed.
 */
UCLASS()
class ROCKCOSMETICS_API URockCosmeticFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	 * @brief Attaches and configures a customizable object instance to the given actor.
	 *
	 * This function creates the required skeletal mesh components based on the number of components
	 * in the provided customizable object instance. For the first component, it uses the actor’s root component;
	 * for subsequent components, it creates new skeletal mesh components, attaches them to the actor, and registers them.
	 * Each component is associated with a corresponding usage component that is configured with the customizable instance
	 * and a name retrieved from the customizable object.
	 *
	 * @param Actor The actor to which the customizable object instance will be attached.
	 * @param CustomizableObject The customizable object that defines the component configuration.
	 * @param CustomizableObject The instance of the customizable object to attach to the actor.
	 * @return TArray<TObjectPtr<UCustomizableObjectInstanceUsage>> An array of usage components associated with the actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "RockCosmetics")
	static TArray<UCustomizableObjectInstanceUsage*> AddCustomizableInstanceUsagesToActor(AActor* Actor, const UCustomizableObject* CustomizableObject, UCustomizableObjectInstance* CustomizableObjectInstance);

	/**
	 * @brief Instantiates a customizable object and attaches it to the given actor.
	 *
	 * This overload creates a new instance of the specified customizable object, then delegates to the AddCustomizableInstanceUsagesToActor
	 * function to adding the necessary skeletal mesh and usage components.
	 *
	 * @param Actor The actor to which the customizable object instance will be attached.
	 * @param CustomizableObject The customizable object to instantiate and attach to the actor.
	 * @return TArray<TObjectPtr<UCustomizableObjectInstanceUsage>> An array of usage components associated with the actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "RockCosmetics")
	static TArray<UCustomizableObjectInstanceUsage*> BuildCustomizableActor(AActor* Actor, UCustomizableObject* CustomizableObject);

	/**
	 * @brief Clones an existing customizable object instance and attaches it to the given actor.
	 *
	 * This overload retrieves the customizable object from the provided instance, creates a clone of the instance,
     * and then delegates to the AddCustomizableInstanceUsagesToActor function to configure the actor.
	 *
	 * @param Actor The actor to which the customizable object instance will be attached.
	 * @param CustomizableObjectInstance The existing customizable object instance to clone and attach.
	 * @return TArray<TObjectPtr<UCustomizableObjectInstanceUsage>> An array of usage components associated with the actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "RockCosmetics")
	static TArray<UCustomizableObjectInstanceUsage*> BuildCustomizableActorWithInstance(AActor* Actor, UCustomizableObjectInstance* CustomizableObjectInstance);
};
