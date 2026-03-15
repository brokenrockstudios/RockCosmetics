// Copyright Broken Rock Studios LLC. All Rights Reserved.

#include "AnimSets/RockAnimSubsystem.h"

#include "RockCosmeticAnimGameplayTags.h"
#include "RockCosmeticLogging.h"
#include "AnimSets/RockAnimSetDataAsset.h"
#include "AnimSets/RockSkeletonClassInterface.h"
#include "Components/RockPawnComponent_CharacterParts.h"
#include "Engine/AssetManager.h"

URockAnimSubsystem::URockAnimSubsystem() : Super()
{
}

void URockAnimSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RebuildCache();
}

void URockAnimSubsystem::Deinitialize()
{
	AnimSets.Empty();
	AnimSetsHandle.Reset();
	Super::Deinitialize();
}

void URockAnimSubsystem::RebuildCache()
{
	AnimSets.Empty();
	AnimSetsHandle.Reset();
	UAssetManager& AssetManager = UAssetManager::Get();

	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(URockAnimSetDataAsset::GetStaticAssetType(), AssetIds);

	// TArray<FSoftObjectPath> ToLoad;
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		// 	const FSoftObjectPath AssetPath = Asset.ToSoftObjectPath();
		// 	ToLoad.Add(AssetPath);
		UE_LOG(LogRockCosmetic, Display, TEXT("  + %s"), *AssetId.ToString());
	}

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSubsystem: No anim sets found. Check PrimaryAssetTypesToScan in DefaultGame.ini and asset directory."));
		return;
	}

	AnimSetsHandle = AssetManager.LoadPrimaryAssets(
		AssetIds,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &URockAnimSubsystem::OnAssetsLoaded));

	UE_LOG(LogRockCosmetic, Display, TEXT("URockAnimSubsystem: Requesting async load of %d anim set(s)..."), AssetIds.Num());
}

void URockAnimSubsystem::OnAssetsLoaded()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	TArray<FAssetData> Assets;
	AssetManager.GetPrimaryAssetDataList(URockAnimSetDataAsset::GetStaticAssetType(), Assets);

	UE_LOG(LogRockCosmetic, Display, TEXT("URockAnimSubsystem: Finished loading anim sets. Processing assets... Found %d anim set(s)."), Assets.Num());
	for (const FAssetData& Asset : Assets)
	{
		const FSoftObjectPath AssetPath = Asset.ToSoftObjectPath();

		URockAnimSetDataAsset* AnimSet = Cast<URockAnimSetDataAsset>(Asset.GetAsset());
		if (AnimSet)
		{
			AnimSets.FindOrAdd(AnimSet->SkeletonClassTag).Sets.Add(AnimSet);
			UE_LOG(LogRockCosmetic, Display, TEXT("  + Registered %s -> SkeletonClass: %s, Montages: %d"),
			       *AssetPath.GetAssetName(), *AnimSet->SkeletonClassTag.ToString(), AnimSet->MontageMap.Num());
		}
		else
		{
			UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSubsystem::OnAnimSetsLoaded: Failed to load anim set at path '%s'"), *AssetPath.ToString());
		}
	}
}

UAnimMontage* URockAnimSubsystem::GetMontage(const AActor* Pawn, FGameplayTag AnimTag, bool bForceSyncIfNeeded) const
{
	if (!Pawn)
	{
		return nullptr;
	}

	FGameplayTag SkeletonClass = GetSkeletonClass(Pawn);

	const FRockAnimSetArray* Sets = AnimSets.Find(SkeletonClass);
	if (!Sets)
	{
		return nullptr;
	}

	for (const TObjectPtr<URockAnimSetDataAsset>& Set : Sets->Sets)
	{
		if (UAnimMontage* Found = Set->GetMontage(AnimTag, bForceSyncIfNeeded))
		{
			return Found;
		}
	}
	return nullptr;
}

FGameplayTag URockAnimSubsystem::GetSkeletonClass(const AActor* Pawn) const
{
	URockPawnComponent_CharacterParts* CharParts = Pawn->GetComponentByClass<URockPawnComponent_CharacterParts>();
	// Returns first tag under Cosmetic.SkeletonClass
	if (CharParts)
	{
		FGameplayTagContainer Tags = CharParts->GetCombinedTags(RockCosmeticAnimGameplayTags::Cosmetic_SkeletonClass);
		if (!Tags.IsEmpty())
		{
			return Tags.First();
		}
	}

	const IRockSkeletonClassInterface* Interface = Cast<IRockSkeletonClassInterface>(Pawn);
	if (Interface)
	{
		return Interface->GetSkeletonClass();
	}


	UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSubsystem::GetSkeletonClass: No SkeletonClass found on %s"), *Pawn->GetName());
	return FGameplayTag::EmptyTag;
}
