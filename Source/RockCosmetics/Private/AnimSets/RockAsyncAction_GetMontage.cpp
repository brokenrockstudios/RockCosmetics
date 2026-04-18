// Copyright Broken Rock Studios LLC. All Rights Reserved.


#include "AnimSets/RockAsyncAction_GetMontage.h"

#include "AnimSets/RockAnimSetDataAsset.h"
#include "AnimSets/RockAnimSubsystem.h"

URockAsyncAction_GetMontage* URockAsyncAction_GetMontage::RequestMontage(
    UObject* WorldContextObject,
    const AActor* Pawn,
    FGameplayTag AnimTag)
{
    URockAsyncAction_GetMontage* Action = NewObject<URockAsyncAction_GetMontage>();
    Action->WeakPawn = Pawn;
    Action->AnimTag = AnimTag;
    Action->WeakWorldContext = WorldContextObject;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
}

void URockAsyncAction_GetMontage::Activate()
{
    const AActor* Pawn = WeakPawn.Get();
    if (!Pawn)
    {
        SetReadyToDestroy();
        return;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WeakWorldContext.Get(), EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        SetReadyToDestroy();
        return;
    }

    URockAnimSubsystem* AnimSubsystem = UGameInstance::GetSubsystem<URockAnimSubsystem>(World->GetGameInstance());
    if (!AnimSubsystem)
    {
        SetReadyToDestroy();
        return;
    }

    FGameplayTag SkeletonClass = AnimSubsystem->GetSkeletonClass(Pawn);
    const FRockAnimSetArray* Sets = AnimSubsystem->AnimSets.Find(SkeletonClass);
    if (!Sets)
    {
        SetReadyToDestroy();
        return;
    }

    // Walk sets, first found wins
    for (const TObjectPtr<URockAnimSetDataAsset>& Set : Sets->Sets)
    {
        if (!Set)
        {
            continue;
        }

        // Reuse existing RequestMontage - it already handles tag hierarchy walk + already-loaded fast path
        TWeakObjectPtr<URockAsyncAction_GetMontage> WeakThis(this);
        bool bDispatched = Set->RequestMontage(AnimTag, [WeakThis](UAnimMontage* Montage)
        {
            if (!WeakThis.IsValid())
            {
                return; // silent noop - ability went away
            }
            if (Montage)
            {
                WeakThis->OnLoaded.Broadcast(Montage);
                WeakThis->SetReadyToDestroy();
            }
            // null means this set didn't have it - outer loop handles fallthrough
        });

        if (bDispatched)
        {
            return; // async in flight, we're done
        }
    }

    // Nothing found across all sets
    SetReadyToDestroy();
}