// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "RockTaggedActor.h"
#include "RockSkeletalTaggedActor.generated.h"

/**
 * 
 */
UCLASS()
class ROCKCOSMETICS_API ARockSkeletalTaggedActor : public ARockTaggedActor
{
	GENERATED_BODY()
public:
	ARockSkeletalTaggedActor();

	class USkeletalMeshComponent* GetSkeletalMeshComponent() const;
	class USkeletalMeshComponent* GetSkeletalMeshComponentAt(int32 Index = 0) const;
	

protected:
	UPROPERTY(Category = SkeletalMeshActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Components|SkeletalMesh,Animation,Physics", AllowPrivateAccess = "true"))
	TArray< TObjectPtr<class USkeletalMeshComponent>> SkeletalMeshComponents;
};
