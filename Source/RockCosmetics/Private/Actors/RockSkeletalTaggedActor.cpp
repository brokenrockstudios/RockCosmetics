// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.


#include "Actors/RockSkeletalTaggedActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RockSkeletalTaggedActor)

ARockSkeletalTaggedActor::ARockSkeletalTaggedActor()
{
	USkeletalMeshComponent* SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComponent");
	RootComponent = SkeletalMeshComponent;
	SkeletalMeshComponents.Add(SkeletalMeshComponent);
}

class USkeletalMeshComponent* ARockSkeletalTaggedActor::GetSkeletalMeshComponentAt(int32 Index) const
{
	if (Index >= 0 && Index < SkeletalMeshComponents.Num())
	{
		return SkeletalMeshComponents[Index];
	}
	checkf(false, TEXT("Invalid index %d"), Index);
	return nullptr;
}

class USkeletalMeshComponent* ARockSkeletalTaggedActor::GetSkeletalMeshComponent() const
{
	return GetSkeletalMeshComponentAt(0);
}
