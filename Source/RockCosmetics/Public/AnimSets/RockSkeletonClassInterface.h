// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RockSkeletonClassInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class URockSkeletonClassInterface : public UInterface
{
	GENERATED_BODY()
};

class ROCKCOSMETICS_API IRockSkeletonClassInterface
{
	GENERATED_BODY()

public:
	FGameplayTag GetSkeletonClass() const;
};
