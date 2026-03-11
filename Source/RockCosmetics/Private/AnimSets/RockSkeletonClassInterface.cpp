// Copyright Broken Rock Studios LLC. All Rights Reserved.


#include "AnimSets/RockSkeletonClassInterface.h"

#include "GameplayTagContainer.h"


FGameplayTag IRockSkeletonClassInterface::GetSkeletonClass() const
{
	// Override in your actor and return a tag that matches the skeleton class of the actor. 
	// This is used by the anim subsystem to find the correct anim set for the actor.
	// Especially useful for scenarios where the CharacterPart component isn't being used
	return FGameplayTag::EmptyTag;
}
