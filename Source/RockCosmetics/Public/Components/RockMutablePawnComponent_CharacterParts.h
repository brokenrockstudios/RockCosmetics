// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "RockPawnComponent_CharacterParts.h"
#include "MuCO/CustomizableObjectInstanceDescriptor.h"
#include "Mutable/MutableLayers.h"
#include "RockMutablePawnComponent_CharacterParts.generated.h"

struct FRockMutableOption;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROCKCOSMETICS_API URockMutablePawnComponent_CharacterParts : public URockPawnComponent_CharacterParts
{
	GENERATED_BODY()
public:
	// Sets default values for this component's properties
	URockMutablePawnComponent_CharacterParts();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BroadcastChanged() override;
	// ================================= MUTABLE =================================

	/** The customizable object instance that will be used to customize this character's skeletal mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Customization)
	TWeakObjectPtr<class UCustomizableObjectInstance> CustomizableObjectInstance;

	// The base appearance descriptor captured from the TaggedActor, used as LAYER 0 of every recompose.
	UPROPERTY()
	TArray<uint8> InitialAppearanceDescriptor;

	UPROPERTY()
	bool bRecomposePending = false;
	UPROPERTY()
	int32 CosmeticHandleCount = 0;

	// The replicated source of truth for cosmetics. The authority mutates it (AddCosmeticEntry / RemoveCosmeticEntry);
	// clients receive it via OnRep_CosmeticMutableEntries and recompose locally. Replicating the parameter DATA (rather
	// than a server-baked InstanceDescriptor) is what makes this work on a dedicated server, which never runs Mutable
	// and so can never build a descriptor to replicate.
	UPROPERTY(ReplicatedUsing = OnRep_CosmeticMutableEntries)
	TArray<FRockMutableCosmeticEntry> CosmeticMutableEntries;

	// Replicated cosmetic data changed on this client — recompose the local mesh.
	UFUNCTION()
	void OnRep_CosmeticMutableEntries();

	// Returns a handle that can be used to remove this entry later. CosmeticHandle is just an int that increments with each addition, it has no meaning other than being a unique identifier for this entry.
	FRockCosmeticHandle AddCosmeticEntry(int32 LayerIndex, const FRockMutableOption& MutableOption);
	void RemoveCosmeticEntry(const FRockCosmeticHandle& CosmeticHandle);

	// Remove all entries in a specific layer.	
	void RemoveCosmeticLayer(int32 LayerIndex);

	void CaptureInitialAppearance();
	void RequestRecompose();
	void ExecuteRecompose();
	void ApplyOptionToDescriptor(FCustomizableObjectInstanceDescriptor& WorkingDescriptor, const FRockMutableOption& Option);
};
