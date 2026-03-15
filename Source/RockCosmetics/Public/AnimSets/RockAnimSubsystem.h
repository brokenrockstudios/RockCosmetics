// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "RockAnimSubsystem.generated.h"

#define UE_API ROCKCOSMETICS_API

struct FStreamableHandle;
class URockAnimSetDataAsset;

USTRUCT(BlueprintType, Blueprintable, Category = "Animation")
struct UE_API FRockAnimationForSkeletonClass
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere,  Category = "Animation")
	FGameplayTag DesiredAnimationTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	FGameplayTag SkeletonClass;
};


USTRUCT()
struct FRockAnimSetArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<URockAnimSetDataAsset>> Sets;
};

UCLASS()
class UE_API URockAnimSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	URockAnimSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RebuildCache();
	void OnAssetsLoaded();

	UFUNCTION(BlueprintCallable, Category = "Rock|Animation")
	UAnimMontage* GetMontage(const AActor* Pawn, FGameplayTag AnimTag, bool bForceSyncIfNeeded = false) const;

	UFUNCTION(BlueprintCallable, Category = "Rock|Animation")
	FGameplayTag GetSkeletonClass(const AActor* Pawn) const;

private:
	// Config assets injected at game startup/per experience
	// keyed to Anim.Skeleton
	UPROPERTY()
	TMap<FGameplayTag, FRockAnimSetArray> AnimSets;
	TSharedPtr<FStreamableHandle> AnimSetsHandle;
};

#undef UE_API
