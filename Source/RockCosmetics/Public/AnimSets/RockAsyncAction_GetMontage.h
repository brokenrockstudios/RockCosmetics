// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UObject/Object.h"
#include "RockAsyncAction_GetMontage.generated.h"

#define UE_API ROCKCOSMETICS_API

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMontageLoaded, UAnimMontage*, Montage);

UCLASS()
class UE_API URockAsyncAction_GetMontage : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnMontageLoaded OnLoaded;

	UFUNCTION(BlueprintCallable, Category = "Rock|Animation", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static URockAsyncAction_GetMontage* RequestMontage(UObject* WorldContextObject, const AActor* Pawn, FGameplayTag AnimTag);

	virtual void Activate() override;

private:
	UPROPERTY()
	TWeakObjectPtr<const AActor> WeakPawn;

	UPROPERTY()
	FGameplayTag AnimTag;

	UPROPERTY()
	TWeakObjectPtr<UObject> WeakWorldContext;
};

#undef UE_API
