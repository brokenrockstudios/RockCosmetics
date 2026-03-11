// Copyright Broken Rock Studios LLC. All Rights Reserved.

#include "AnimSets/RockAnimSetDataAsset.h"

#include "RockCosmeticLogging.h"
#include "Engine/AssetManager.h"

UAnimMontage* URockAnimSetDataAsset::GetMontage(FGameplayTag AnimTag, bool bForceSyncIfNeeded) const
{
	FGameplayTag Current = AnimTag;
	while (Current.IsValid())
	{
		if (const TSoftObjectPtr<UAnimMontage>* Found = MontageMap.Find(Current))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}

			// ... sync load + warning
			if (bForceSyncIfNeeded)
			{
				UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSetDataAsset::GetMontage — sync loading %s for tag %s. Should preload. May cause hitches!"),
				       *Found->GetAssetName(),
				       *AnimTag.ToString());

				return Cast<UAnimMontage>(Found->LoadSynchronous());
			}
			// shipping always returns null if not loaded
			return nullptr;
		}
		Current = Current.RequestDirectParent();
	}
	return nullptr;
}

void URockAnimSetDataAsset::RequestMontage(FGameplayTag AnimTag, TFunction<void(UAnimMontage*)> OnLoaded)
{
	// Async loads the montage for the given tag. If not found, walks up the tag 
	// hierarchy (e.g. Anim.Emote.Dance.Floss -> Anim.Emote.Dance -> Anim.Emote)
	// until a match is found. Returns nullptr via callback if nothing found in chain.
	// e.g. Anim.Weapon.Reload.Pistol.Onyx might end up on Anim.Weapon.Reload
	FGameplayTag Current = AnimTag;
	while (Current.IsValid())
	{
		if (const TSoftObjectPtr<UAnimMontage>* Found = MontageMap.Find(Current))
		{
			if (Found->IsValid())
			{
				OnLoaded(Found->Get());
				return;
			}

			TWeakObjectPtr<URockAnimSetDataAsset> WeakThis(this);
			TSoftObjectPtr<UAnimMontage> SoftRef(*Found);

			UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
				SoftRef.ToSoftObjectPath(),
				[WeakThis, SoftRef, OnLoaded]()
				{
					if (WeakThis.IsValid())
					{
						OnLoaded(SoftRef.Get());
					}
				}
			);
			return;
		}
		Current = Current.RequestDirectParent();
	}
	OnLoaded(nullptr);
}

void URockAnimSetDataAsset::PreloadAll(FStreamableDelegate OnComplete)
{
	TArray<FSoftObjectPath> Paths;
	for (auto& [Tag, SoftMontage] : MontageMap)
	{
		if (!SoftMontage.IsNull())
			Paths.Add(SoftMontage.ToSoftObjectPath());
	}

	if (Paths.IsEmpty())
	{
		OnComplete.ExecuteIfBound();
		return;
	}

	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(Paths, OnComplete);
}

FPrimaryAssetId URockAnimSetDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetStaticAssetType(), GetFName());
}

/* static*/ FPrimaryAssetType URockAnimSetDataAsset::GetStaticAssetType()
{
	return FPrimaryAssetType("RockAnimSet");
}
