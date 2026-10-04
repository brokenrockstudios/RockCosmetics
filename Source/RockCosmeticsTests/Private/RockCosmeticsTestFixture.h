// Copyright Broken Rock Studios LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorTestSpawner.h"
#include "RockCosmeticsTestTypes.h"
#include "UObject/StrongObjectPtr.h"

/**
 * A transient game world with a pawn that owns a URCTestPartsComponent and a listener counting its change broadcasts.
 *
 * Declare it as a TEST_CLASS member and call Init() in BEFORE_EACH (CQTest also constructs test classes at module load,
 * so nothing here can run in a constructor). The test world never begins play: call BeginPlay() when a test needs
 * EndPlay code to run (AActor::RouteEndPlay skips actors that have not begun play).
 */
class FRockCosmeticsFixture
{
public:
	FActorTestSpawner Spawner;
	ARCTestPawn* Pawn = nullptr;
	URCTestPartsComponent* Parts = nullptr;
	TStrongObjectPtr<URCTestListener> Listener;

	void Init()
	{
		Pawn = &Spawner.SpawnActor<ARCTestPawn>();
		Parts = AddComponent<URCTestPartsComponent>(*Pawn);
		Listener.Reset(NewObject<URCTestListener>(GetTransientPackage()));
		Parts->OnCharacterPartsChanged.AddDynamic(Listener.Get(), &URCTestListener::OnPartsChanged);
	}

	template <class T>
	static T* AddComponent(AActor& Owner)
	{
		T* Component = NewObject<T>(&Owner);
		Component->RegisterComponent();
		return Component;
	}

	/** A part request for this actor class, with no socket. */
	static FRockCharacterPart MakePart(TSubclassOf<AActor> PartClass, ERockCharacterCustomizationCollisionMode Collision = ERockCharacterCustomizationCollisionMode::NoCollision)
	{
		FRockCharacterPart Part;
		Part.PartClass = PartClass;
		Part.CollisionMode = Collision;
		return Part;
	}

	void BeginPlay() const
	{
		Pawn->DispatchBeginPlay();
	}
};
