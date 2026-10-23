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
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	RebuildCache();
}

void URockAnimSubsystem::Deinitialize()
{
	AnimSets.Empty();
	AnimSetsHandle.Reset();
	bSetsLoading = false;
	PendingRequests.Empty();
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

	// Set before the request: the delegate can run inside LoadPrimaryAssets when the sets are already loaded.
	bSetsLoading = true;
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
			RegisterAnimSet(AnimSet);
			UE_LOG(LogRockCosmetic, Display, TEXT("  + Registered %s -> SkeletonClass: %s, Montages: %d"),
			       *AssetPath.GetAssetName(), *AnimSet->SkeletonClassTag.ToString(), AnimSet->MontageMap.Num());
		}
		else
		{
			UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSubsystem::OnAnimSetsLoaded: Failed to load anim set at path '%s'"), *AssetPath.ToString());
		}
	}

	bSetsLoading = false;

	// Answer what arrived while loading. Moved out first so a callback that queues again cannot change what is walked.
	TArray<FPendingMontageRequest> Requests = MoveTemp(PendingRequests);
	for (FPendingMontageRequest& Request : Requests)
	{
		ResolveMontage(Request.Pawn.Get(), Request.AnimTag, MoveTemp(Request.OnDone));
	}
}

void URockAnimSubsystem::RegisterAnimSet(URockAnimSetDataAsset* AnimSet)
{
	if (AnimSet)
	{
		AnimSets.FindOrAdd(AnimSet->SkeletonClassTag).Sets.AddUnique(AnimSet);
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

void URockAnimSubsystem::RequestMontage(const AActor* Pawn, FGameplayTag AnimTag, TFunction<void(UAnimMontage*)> OnDone)
{
	if (bSetsLoading)
	{
		PendingRequests.Add({Pawn, AnimTag, MoveTemp(OnDone)});
		return;
	}
	ResolveMontage(Pawn, AnimTag, MoveTemp(OnDone));
}

void URockAnimSubsystem::ResolveMontage(const AActor* Pawn, FGameplayTag AnimTag, TFunction<void(UAnimMontage*)> OnDone) const
{
	if (!Pawn)
	{
		OnDone(nullptr);
		return;
	}

	const FRockAnimSetArray* Sets = AnimSets.Find(GetSkeletonClass(Pawn));
	if (Sets)
	{
		// Walk sets, first one whose tag chain has an entry wins (it answers, with null if its montage fails to load)
		for (const TObjectPtr<URockAnimSetDataAsset>& Set : Sets->Sets)
		{
			if (Set && Set->RequestMontage(AnimTag, OnDone))
			{
				return;
			}
		}
	}
	OnDone(nullptr);
}

FGameplayTag URockAnimSubsystem::GetSkeletonClass(const AActor* Pawn) const
{
	if (!Pawn)
	{
		return FGameplayTag::EmptyTag;
	}

	URockPawnComponent_CharacterParts* CharParts = Pawn->GetComponentByClass<URockPawnComponent_CharacterParts>();
	if (CharParts)
	{
		TArray<FGameplayTag> Tags;
		CharParts->GetCombinedTags(RockCosmeticAnimGameplayTags::Cosmetic_SkeletonClass).GetGameplayTagArray(Tags);
		if (!Tags.IsEmpty())
		{
			// Container order follows the order the parts were added, so pick by name to stay deterministic
			Tags.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return A.GetTagName().LexicalLess(B.GetTagName()); });
			if (Tags.Num() > 1 && !WarnedSeveralClassesPawns.Contains(FObjectKey(Pawn)))
			{
				WarnedSeveralClassesPawns.Add(FObjectKey(Pawn));
				UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSubsystem::GetSkeletonClass: %s has %d SkeletonClass tags, using %s"),
				       *Pawn->GetName(), Tags.Num(), *Tags[0].ToString());
			}
			return Tags[0];
		}
	}

	const IRockSkeletonClassInterface* Interface = Cast<IRockSkeletonClassInterface>(Pawn);
	if (Interface)
	{
		const FGameplayTag SkeletonClass = Interface->GetSkeletonClass();
		if (SkeletonClass.IsValid())
		{
			return SkeletonClass;
		}
	}

	if (!WarnedNoClassPawns.Contains(FObjectKey(Pawn)))
	{
		WarnedNoClassPawns.Add(FObjectKey(Pawn));
		UE_LOG(LogRockCosmetic, Warning, TEXT("URockAnimSubsystem::GetSkeletonClass: No SkeletonClass found on %s"), *Pawn->GetName());
	}
	return FGameplayTag::EmptyTag;
}
