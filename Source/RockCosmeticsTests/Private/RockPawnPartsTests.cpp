// Copyright Broken Rock Studios LLC. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"
#include "RockCosmeticsTestFixture.h"

#include "Components/RockMutablePawnComponent_CharacterParts.h"

// URockPawnComponent_CharacterParts on the authority: adding, removing and clearing parts spawns and destroys child
// actors, and tells listeners. The test world is standalone, so the "client" half (SpawnActorForEntry on replication) is
// exercised through the list's replication callbacks.

TEST_CLASS(RockPawnPartsTests, "BRS.RockCosmetics.PawnParts")
{
	FRockCosmeticsFixture Fixture;

	BEFORE_EACH()
	{
		Fixture.Init();
	}

	TEST_METHOD(NoParts_HasNoActors)
	{
		ASSERT_THAT(AreEqual(0, Fixture.Parts->GetCharacterPartActors().Num()));
	}

	TEST_METHOD(AddCharacterPart_ReturnsValidHandle)
	{
		const FRockCharacterPartHandle Handle = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		ASSERT_THAT(IsTrue(Handle.IsValid()));
	}

	TEST_METHOD(AddCharacterPart_SpawnsAPartActorOfThatClass)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));

		const TArray<AActor*> Actors = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(1, Actors.Num()));
		ASSERT_THAT(IsTrue(Actors[0]->IsA<ARCTestHatActor>()));
	}

	TEST_METHOD(AddCharacterPart_AttachesPartToOwner)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));

		const TArray<AActor*> Actors = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(1, Actors.Num()));
		ASSERT_THAT(IsTrue(Actors[0]->GetAttachParentActor() == Fixture.Pawn));
	}

	TEST_METHOD(AddCharacterPart_TwiceGivesDistinctHandlesAndTwoActors)
	{
		const FRockCharacterPartHandle Hat = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		const FRockCharacterPartHandle Boots = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));

		ASSERT_THAT(IsTrue(Hat.IsValid()));
		ASSERT_THAT(IsTrue(Boots.IsValid()));
		ASSERT_THAT(AreEqual(2, Fixture.Parts->GetCharacterPartActors().Num()));
	}

	TEST_METHOD(AddCharacterPart_BroadcastsOnceWithTheComponent)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));

		ASSERT_THAT(AreEqual(1, Fixture.Listener->Count));
		ASSERT_THAT(IsTrue(Fixture.Listener->LastComponent == Fixture.Parts));
	}

	TEST_METHOD(AddCharacterPart_NoCollisionMode_DisablesCollision)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass(), ERockCharacterCustomizationCollisionMode::NoCollision));

		const TArray<AActor*> Actors = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(1, Actors.Num()));
		ASSERT_THAT(IsFalse(Actors[0]->GetActorEnableCollision()));
	}

	TEST_METHOD(AddCharacterPart_UseCollisionMode_LeavesCollisionAlone)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass(), ERockCharacterCustomizationCollisionMode::UseCollisionFromCharacterPart));

		const TArray<AActor*> Actors = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(1, Actors.Num()));
		ASSERT_THAT(IsTrue(Actors[0]->GetActorEnableCollision()));
	}

	TEST_METHOD(AddCharacterPart_NullClass_ReturnsHandleButSpawnsNothing)
	{
		TestRunner->AddExpectedMessagePlain(TEXT("PartClass is null"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);

		const FRockCharacterPartHandle Handle = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(nullptr));

		ASSERT_THAT(IsTrue(Handle.IsValid()));
		ASSERT_THAT(AreEqual(0, Fixture.Parts->GetCharacterPartActors().Num()));
		ASSERT_THAT(AreEqual(0, Fixture.Listener->Count));
	}

	TEST_METHOD(RemoveCharacterPart_DestroysOnlyThatPart)
	{
		const FRockCharacterPartHandle Hat = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));
		const TArray<AActor*> Before = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(2, Before.Num()));
		TWeakObjectPtr<AActor> HatActor = Before[0];

		Fixture.Parts->RemoveCharacterPart(Hat);

		const TArray<AActor*> After = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(1, After.Num()));
		ASSERT_THAT(IsTrue(After[0]->IsA<ARCTestBootsActor>()));
		ASSERT_THAT(IsFalse(HatActor.IsValid()));
	}

	TEST_METHOD(RemoveCharacterPart_Broadcasts)
	{
		const FRockCharacterPartHandle Hat = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Listener->Count = 0;

		Fixture.Parts->RemoveCharacterPart(Hat);

		ASSERT_THAT(AreEqual(1, Fixture.Listener->Count));
	}

	TEST_METHOD(RemoveCharacterPart_InvalidHandle_ChangesNothing)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Listener->Count = 0;

		Fixture.Parts->RemoveCharacterPart(FRockCharacterPartHandle());

		ASSERT_THAT(AreEqual(1, Fixture.Parts->GetCharacterPartActors().Num()));
		ASSERT_THAT(AreEqual(0, Fixture.Listener->Count));
	}

	TEST_METHOD(RemoveCharacterPart_TwiceRemovesOnce)
	{
		const FRockCharacterPartHandle Hat = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));

		Fixture.Parts->RemoveCharacterPart(Hat);
		Fixture.Parts->RemoveCharacterPart(Hat);

		ASSERT_THAT(AreEqual(1, Fixture.Parts->GetCharacterPartActors().Num()));
	}

	TEST_METHOD(RemoveAllCharacterParts_DestroysEveryPartAndBroadcastsOnce)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));
		Fixture.Listener->Count = 0;

		Fixture.Parts->RemoveAllCharacterParts();

		ASSERT_THAT(AreEqual(0, Fixture.Parts->GetCharacterPartActors().Num()));
		ASSERT_THAT(AreEqual(1, Fixture.Listener->Count));
	}

	TEST_METHOD(RemoveAllCharacterParts_WhenEmpty_DoesNotBroadcast)
	{
		Fixture.Parts->RemoveAllCharacterParts();
		ASSERT_THAT(AreEqual(0, Fixture.Listener->Count));
	}

	TEST_METHOD(AddCharacterPart_AfterRemoveAll_HandlesStayUnique)
	{
		const FRockCharacterPartHandle Before = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->RemoveAllCharacterParts();
		const FRockCharacterPartHandle After = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));

		// The stale handle must not remove the part that was added later.
		Fixture.Parts->RemoveCharacterPart(Before);
		ASSERT_THAT(AreEqual(1, Fixture.Parts->GetCharacterPartActors().Num()));
	}

	TEST_METHOD(Destroy_AfterBeginPlay_DestroysPartActors)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		TWeakObjectPtr<AActor> HatActor = Fixture.Parts->GetCharacterPartActors()[0];
		Fixture.BeginPlay();
		Fixture.Listener->Count = 0;

		Fixture.Pawn->Destroy();

		ASSERT_THAT(IsFalse(HatActor.IsValid()));
		// EndPlay clears the list without telling listeners (the owner is going away).
		ASSERT_THAT(AreEqual(0, Fixture.Listener->Count));
	}

	TEST_METHOD(GetCombinedTags_UnionsTagsOfAllParts)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));

		const FGameplayTagContainer Tags = Fixture.Parts->GetCombinedTags(FGameplayTag());

		ASSERT_THAT(AreEqual(3, Tags.Num()));
		ASSERT_THAT(IsTrue(Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Hat")))));
		ASSERT_THAT(IsTrue(Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Head")))));
		ASSERT_THAT(IsTrue(Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Boots")))));
	}

	TEST_METHOD(GetCombinedTags_NoParts_IsEmpty)
	{
		ASSERT_THAT(IsTrue(Fixture.Parts->GetCombinedTags(FGameplayTag()).IsEmpty()));
	}

	TEST_METHOD(GetCombinedTags_FiltersToTheRequestedPrefix)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));

		const FGameplayTagContainer Tags = Fixture.Parts->GetCombinedTags(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Boots")));

		ASSERT_THAT(AreEqual(1, Tags.Num()));
		ASSERT_THAT(IsTrue(Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Boots")))));
	}

	TEST_METHOD(GetCombinedTags_DropsTagsOfRemovedParts)
	{
		const FRockCharacterPartHandle Hat = Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestBootsActor::StaticClass()));

		Fixture.Parts->RemoveCharacterPart(Hat);

		const FGameplayTagContainer Tags = Fixture.Parts->GetCombinedTags(FGameplayTag());
		ASSERT_THAT(AreEqual(1, Tags.Num()));
		ASSERT_THAT(IsTrue(Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Boots")))));
	}

	TEST_METHOD(AttachTarget_WithoutMesh_IsTheRootComponent)
	{
		ASSERT_THAT(IsTrue(Fixture.Parts->GetParentMeshComponent() == nullptr));
		ASSERT_THAT(IsTrue(Fixture.Parts->GetSceneComponentToAttachTo() == Fixture.Pawn->GetRootComponent()));
	}

	// What a client runs when the replicated list changes.

	TEST_METHOD(ReplicatedChange_RecreatesThePartActor)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		TWeakObjectPtr<AActor> Original = Fixture.Parts->GetCharacterPartActors()[0];
		Fixture.Listener->Count = 0;

		TArray<int32> Changed = {0};
		Fixture.Parts->List().PostReplicatedChange(Changed, 1);

		const TArray<AActor*> Actors = Fixture.Parts->GetCharacterPartActors();
		ASSERT_THAT(AreEqual(1, Actors.Num()));
		ASSERT_THAT(IsFalse(Original.IsValid()));
		ASSERT_THAT(IsTrue(Actors[0]->IsA<ARCTestHatActor>()));
		ASSERT_THAT(AreEqual(1, Fixture.Listener->Count));
	}

	TEST_METHOD(ReplicatedRemove_DestroysThePartActorAndBroadcasts)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		Fixture.Listener->Count = 0;

		TArray<int32> Removed = {0};
		Fixture.Parts->List().PreReplicatedRemove(Removed, 0);

		ASSERT_THAT(AreEqual(0, Fixture.Parts->GetCharacterPartActors().Num()));
		ASSERT_THAT(AreEqual(1, Fixture.Listener->Count));
	}
};

// The Mutable variant layers cosmetic entries on top of the part list. Without a CustomizableObject instance (the test
// world builds none) only the bookkeeping is observable: entries, and the deferred recompose request.

TEST_CLASS(RockMutablePawnPartsTests, "BRS.RockCosmetics.MutablePawnParts")
{
	FRockCosmeticsFixture Fixture;
	URockMutablePawnComponent_CharacterParts* Mutable = nullptr;
	FRockMutableOption Option;

	BEFORE_EACH()
	{
		Fixture.Init();
		Mutable = FRockCosmeticsFixture::AddComponent<URockMutablePawnComponent_CharacterParts>(*Fixture.Pawn);
	}

	TEST_METHOD(AddCosmeticEntry_AddsAnEntryAndRequestsARecompose)
	{
		const FRockCosmeticHandle Handle = Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::Equipment, Option);

		ASSERT_THAT(IsTrue(Handle.IsValid()));
		ASSERT_THAT(AreEqual(1, Mutable->CosmeticMutableEntries.Num()));
		ASSERT_THAT(IsTrue(Mutable->bRecomposePending));
	}

	TEST_METHOD(AddCosmeticEntry_ManyTimes_StillOnePendingRecompose)
	{
		Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::StatusEffects, Option);

		ASSERT_THAT(AreEqual(2, Mutable->CosmeticMutableEntries.Num()));
		ASSERT_THAT(IsTrue(Mutable->bRecomposePending));
	}

	TEST_METHOD(RemoveCosmeticEntry_RemovesIt)
	{
		const FRockCosmeticHandle Handle = Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::Equipment, Option);

		Mutable->RemoveCosmeticEntry(Handle);

		ASSERT_THAT(AreEqual(0, Mutable->CosmeticMutableEntries.Num()));
	}

	TEST_METHOD(RemoveCosmeticLayer_RemovesOnlyThatLayer)
	{
		Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		Mutable->AddCosmeticEntry(RockCosmeticAppearanceLayers::UserCustomization, Option);

		Mutable->RemoveCosmeticLayer(RockCosmeticAppearanceLayers::Equipment);

		ASSERT_THAT(AreEqual(1, Mutable->CosmeticMutableEntries.Num()));
	}

	TEST_METHOD(ExecuteRecompose_WithoutAnInstance_RetriesAndStaysPending)
	{
		Mutable->ExecuteRecompose();

		ASSERT_THAT(AreEqual(1, Mutable->RecomposeRetryCount));
		ASSERT_THAT(IsTrue(Mutable->bRecomposePending));
	}

	TEST_METHOD(ExecuteRecompose_WithoutAnInstance_GivesUpAfterTenRetries)
	{
		TestRunner->AddExpectedMessagePlain(TEXT("giving up after 10 attempt(s)"), ELogVerbosity::Error, EAutomationExpectedMessageFlags::Contains, 1);

		for (int32 Attempt = 0; Attempt < 11; ++Attempt)
		{
			Mutable->ExecuteRecompose();
		}

		ASSERT_THAT(AreEqual(10, Mutable->RecomposeRetryCount));
		ASSERT_THAT(IsFalse(Mutable->bRecomposePending));
	}

	TEST_METHOD(ResolveCustomizableObjectInstance_WithNoMutablePart_ReturnsNull)
	{
		// A part that is not a Mutable-tagged actor makes BroadcastChanged warn (see DevNotes).
		TestRunner->AddExpectedMessagePlain(TEXT("MutableTaggedActor is null"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);

		Mutable->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestHatActor::StaticClass()));
		ASSERT_THAT(IsTrue(Mutable->ResolveCustomizableObjectInstance() == nullptr));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
