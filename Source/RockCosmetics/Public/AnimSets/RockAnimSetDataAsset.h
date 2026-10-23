// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "RockAnimSetDataAsset.generated.h"

#define UE_API ROCKCOSMETICS_API

UCLASS()
class UE_API URockAnimSetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Cosmetic.SkeletonClass"))
	FGameplayTag SkeletonClassTag;
	
	// Soft refs internally. Hard ref behavior when preloaded, streamed behavior when not
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Anim"))
	TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> MontageMap;
	
	// Synchronous returns null if not loaded 
	UAnimMontage* GetMontage(FGameplayTag AnimTag, bool bForceSyncIfNeeded = false) const;
	
	// Async use for emote wheel preview etc. True if the tag chain has an entry, false if not (OnLoaded is not called then).
	// When true, OnLoaded is always called once, with null if the entry's montage fails to load.
	bool RequestMontage(FGameplayTag AnimTag, TFunction<void(UAnimMontage*)> OnLoaded);

	// Call this to preload entire DA (combat DA on spawn, emote DA on wheel open).
	// The montages stay loaded only while the caller keeps the returned handle (null when the set has no montages).
	TSharedPtr<FStreamableHandle> PreloadAll(FStreamableDelegate OnComplete = FStreamableDelegate());
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetStaticAssetType();
};

#undef UE_API
