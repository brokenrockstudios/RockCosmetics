// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "RockPawnComponent_CharacterParts.h"
#include "RockMutablePawnComponent_CharacterParts.generated.h"


struct FRockMutableOption;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROCKCOSMETICS_API URockMutablePawnComponent_CharacterParts : public URockPawnComponent_CharacterParts
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URockMutablePawnComponent_CharacterParts();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void BroadcastChanged() override;
	// ================================= MUTABLE =================================
	
	UFUNCTION()
	void OnCustomizableObjectUpdated();
	
	/** The customizable object instance that will be used to customize this character's skeletal mesh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Customization)
	TWeakObjectPtr<class UCustomizableObjectInstance> CustomizableObjectInstance;
	
	/** 
	 * Descriptor containing all the CustomizableObjectInstance customization parameters, optimized to be replicated over the network.
	 * It is not updated automatically, it must be updated manually with the SerializeAndSendInstanceDescriptor() function
	*/
	UPROPERTY(VisibleAnywhere, Category = Customization, ReplicatedUsing = OnRep_InstanceDescriptor)
	TArray<uint8> InstanceDescriptor;

	/** Callback for InstanceDescriptor network changes */
	UFUNCTION()
	void OnRep_InstanceDescriptor();

	/** This function should be called after having changed customization parameters and wanting them to be replicated over the network */
	void SerializeAndSendInstanceDescriptor();

	void AddCharacterPartFromMutableOption(const FRockMutableOption& NewPart);
	void RemoveCharacterPartFromMutableOption(const FRockMutableOption& NewPart);

	UFUNCTION(exec)
	void ShowCharacterInformation(bool bFullInfo, bool ShowMaterialInfo = false);

	/** RPC Call to the server to update its InstanceDescriptor with our dirty one. The server will replicate it to the other clients */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCUpdateInstanceDescriptor(const TArray<uint8>& NewInstanceDescriptor);
	bool ServerRPCUpdateInstanceDescriptor_Validate(const TArray<uint8>& NewInstanceDescriptor);
	void ServerRPCUpdateInstanceDescriptor_Implementation(const TArray<uint8>& NewInstanceDescriptor);
	
};
