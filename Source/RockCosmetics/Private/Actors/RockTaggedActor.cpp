// Copyright Epic Games, Inc. All Rights Reserved.
// Modified by Broken Rock Studios
// See the LICENSE file for details.

#include "Actors/RockTaggedActor.h"

#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockTaggedActor)

ARockTaggedActor::ARockTaggedActor()
{
}

void ARockTaggedActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(StaticGameplayTags);
}

#if WITH_EDITOR
bool ARockTaggedActor::CanEditChange(const FProperty* InProperty) const
{
	// Prevent editing of the other tags property
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AActor, Tags))
	{
		return false;
	}

	return Super::CanEditChange(InProperty);
}
#endif

