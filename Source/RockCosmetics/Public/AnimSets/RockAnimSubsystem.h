// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/ObjectKey.h"
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

	// Adds a set under its SkeletonClassTag. OnAssetsLoaded registers every RockAnimSet primary asset this way.
	void RegisterAnimSet(URockAnimSetDataAsset* AnimSet);

	// True while RebuildCache's async load of the sets is in flight. RequestMontage waits for it.
	bool AreSetsLoading() const { return bSetsLoading; }

	UFUNCTION(BlueprintCallable, Category = "Rock|Animation")
	UAnimMontage* GetMontage(const AActor* Pawn, FGameplayTag AnimTag, bool bForceSyncIfNeeded = false) const;

	// Resolves the montage for the pawn's skeleton class, loading it if needed. OnDone is called exactly once, with null
	// on a miss (no class, no set for the class, no tag in the chain, or a failed load). It may be called before this
	// returns. While the sets are still loading the request waits for them.
	void RequestMontage(const AActor* Pawn, FGameplayTag AnimTag, TFunction<void(UAnimMontage*)> OnDone);

	// The pawn's Cosmetic.SkeletonClass tag from its character parts (the lexically first one if they report several),
	// else from IRockSkeletonClassInterface. Warns once per pawn when it has none.
	UFUNCTION(BlueprintCallable, Category = "Rock|Animation")
	FGameplayTag GetSkeletonClass(const AActor* Pawn) const;

private:
	void ResolveMontage(const AActor* Pawn, FGameplayTag AnimTag, TFunction<void(UAnimMontage*)> OnDone) const;

	// Config assets injected at game startup/per experience
	// keyed to Anim.Skeleton
	UPROPERTY()
	TMap<FGameplayTag, FRockAnimSetArray> AnimSets;
	TSharedPtr<FStreamableHandle> AnimSetsHandle;
	bool bSetsLoading = false;

	struct FPendingMontageRequest
	{
		TWeakObjectPtr<const AActor> Pawn;
		FGameplayTag AnimTag;
		TFunction<void(UAnimMontage*)> OnDone;
	};
	TArray<FPendingMontageRequest> PendingRequests;

	mutable TSet<FObjectKey> WarnedNoClassPawns;
	mutable TSet<FObjectKey> WarnedSeveralClassesPawns;
};

#undef UE_API
