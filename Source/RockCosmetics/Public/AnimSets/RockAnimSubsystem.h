// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "RockAnimSubsystem.generated.h"

#define UE_API ROCKCOSMETICS_API

struct FStreamableHandle;
class URockAnimSetDataAsset;

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
	void OnAnimSetsLoaded();

	UFUNCTION(BlueprintCallable, Category = "Rock|Animation")
	UAnimMontage* GetMontage(const AActor* Pawn, FGameplayTag AnimTag, bool bForceSyncIfNeeded = false) const;

private:
	// Config assets injected at game startup/per experience
	// keyed to Anim.Skeleton
	UPROPERTY()
	TMap<FGameplayTag, FRockAnimSetArray> AnimSets;
	TSharedPtr<FStreamableHandle> AnimSetsHandle;

	// Internal Helpers.
	FGameplayTag GetSkeletonClass(const AActor* Pawn) const;
};

#undef UE_API
