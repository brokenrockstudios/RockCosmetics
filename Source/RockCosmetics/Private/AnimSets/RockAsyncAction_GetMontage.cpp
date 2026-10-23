// Copyright Broken Rock Studios LLC. All Rights Reserved.


#include "AnimSets/RockAsyncAction_GetMontage.h"

#include "AnimSets/RockAnimSubsystem.h"
#include "Engine/GameInstance.h"

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
    UWorld* World = GEngine->GetWorldFromContextObject(WeakWorldContext.Get(), EGetWorldErrorMode::LogAndReturnNull);
    URockAnimSubsystem* AnimSubsystem = World ? UGameInstance::GetSubsystem<URockAnimSubsystem>(World->GetGameInstance()) : nullptr;
    if (!Pawn || !AnimSubsystem)
    {
        Finish(nullptr);
        return;
    }

    // The subsystem always answers once (null on a miss), possibly before this returns
    TWeakObjectPtr<URockAsyncAction_GetMontage> WeakThis(this);
    AnimSubsystem->RequestMontage(Pawn, AnimTag, [WeakThis](UAnimMontage* Montage)
    {
        if (URockAsyncAction_GetMontage* This = WeakThis.Get())
        {
            This->Finish(Montage);
        }
    });
}

void URockAsyncAction_GetMontage::Finish(UAnimMontage* Montage)
{
    if (Montage)
    {
        OnLoaded.Broadcast(Montage);
    }
    else
    {
        OnFailed.Broadcast();
    }
    SetReadyToDestroy();
}
