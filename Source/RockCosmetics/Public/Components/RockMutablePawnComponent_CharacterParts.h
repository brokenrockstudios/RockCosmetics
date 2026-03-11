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

	UFUNCTION()
	void OnCustomizableObjectUpdated();

	/** The customizable object instance that will be used to customize this character's skeletal mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Customization)
	TWeakObjectPtr<class UCustomizableObjectInstance> CustomizableObjectInstance;

	/** 
	 * Descriptor containing all the CustomizableObjectInstance customization parameters, optimized to be replicated over the network.
	 * It is not updated automatically, it must be updated manually with the SerializeAndSendInstanceDescriptor() function
	*/
	UPROPERTY(VisibleAnywhere, Category = Customization, ReplicatedUsing = OnRep_InstanceDescriptor)
	TArray<uint8> InstanceDescriptor;

	// Should be saved off from only specific customizable scenarios.
	UPROPERTY()
	TArray<uint8> InitialAppearanceDescriptor;

	UPROPERTY()
	bool bRecomposePending = false;
	UPROPERTY()
	int32 CosmeticHandleCount = 0;
	UPROPERTY()
	TArray<FRockMutableCosmeticEntry> CosmeticMutableEntries;

	// Returns a handle that can be used to remove this entry later. CosmeticHandle is just an int that increments with each addition, it has no meaning other than being a unique identifier for this entry.
	FRockCosmeticHandle AddCosmeticEntry(int32 LayerIndex, const FRockMutableOption& MutableOption);
	void RemoveCosmeticEntry(const FRockCosmeticHandle& CosmeticHandle);

	// Remove all entries in a specific layer.	
	void RemoveCosmeticLayer(int32 LayerIndex);

	void CaptureInitialAppearance();
	void RequestRecompose();
	void ExecuteRecompose();
	void ApplyOptionToDescriptor(FCustomizableObjectInstanceDescriptor& WorkingDescriptor, const FRockMutableOption& Option);

	/** Callback for InstanceDescriptor network changes */
	UFUNCTION()
	void OnRep_InstanceDescriptor();

	/** This function should be called after having changed customization parameters and wanting them to be replicated over the network */
	void SerializeAndSendInstanceDescriptor();

	/** RPC Call to the server to update its InstanceDescriptor with our dirty one. The server will replicate it to the other clients */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCUpdateInstanceDescriptor(const TArray<uint8>& NewInstanceDescriptor);
	bool ServerRPCUpdateInstanceDescriptor_Validate(const TArray<uint8>& NewInstanceDescriptor);
	void ServerRPCUpdateInstanceDescriptor_Implementation(const TArray<uint8>& NewInstanceDescriptor);
};
