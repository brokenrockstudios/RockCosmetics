// Copyright Broken Rock Studios LLC. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"

#include "Mutable/MutableLayers.h"

// The replicated cosmetic entry list, used without an owner component so no world or Mutable instance is involved.

TEST_CLASS(RockMutableLayerListTests, "BRS.RockCosmetics.MutableLayers")
{
	FRockMutableCosmeticEntryList List;
	FRockMutableOption Option;

	TEST_METHOD(Handle_DefaultIsInvalid)
	{
		FRockCosmeticHandle Handle;
		ASSERT_THAT(IsFalse(Handle.IsValid()));
	}

	TEST_METHOD(Layers_AreOrderedLowestToHighest)
	{
		ASSERT_THAT(IsTrue(RockCosmeticAppearanceLayers::InitialActorState < RockCosmeticAppearanceLayers::UserCustomization));
		ASSERT_THAT(IsTrue(RockCosmeticAppearanceLayers::UserCustomization < RockCosmeticAppearanceLayers::Equipment));
		ASSERT_THAT(IsTrue(RockCosmeticAppearanceLayers::Equipment < RockCosmeticAppearanceLayers::StatusEffects));
	}

	TEST_METHOD(AddEntry_ReturnsValidDistinctHandles)
	{
		const FRockCosmeticHandle First = List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		const FRockCosmeticHandle Second = List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		ASSERT_THAT(IsTrue(First.IsValid()));
		ASSERT_THAT(IsTrue(Second.IsValid()));
		ASSERT_THAT(IsFalse(First == Second));
		ASSERT_THAT(AreEqual(2, List.Num()));
	}

	TEST_METHOD(RemoveByHandle_RemovesOnlyThatEntry)
	{
		const FRockCosmeticHandle First = List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		const FRockCosmeticHandle Second = List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);

		ASSERT_THAT(AreEqual(1, List.RemoveByHandle(First)));

		const TArray<FRockMutableCosmeticEntry> Left = List.GetSortedEntries();
		ASSERT_THAT(AreEqual(1, Left.Num()));
		ASSERT_THAT(IsTrue(Left[0].CosmeticHandle == Second));
	}

	TEST_METHOD(RemoveByHandle_UnknownOrInvalidHandle_RemovesNothing)
	{
		List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		FRockCosmeticHandle Invalid;
		FRockCosmeticHandle Unknown;
		Unknown.Handle = 999;

		ASSERT_THAT(AreEqual(0, List.RemoveByHandle(Invalid)));
		ASSERT_THAT(AreEqual(0, List.RemoveByHandle(Unknown)));
		ASSERT_THAT(AreEqual(1, List.Num()));
	}

	TEST_METHOD(RemoveByHandle_TwiceRemovesOnce)
	{
		const FRockCosmeticHandle Handle = List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		ASSERT_THAT(AreEqual(1, List.RemoveByHandle(Handle)));
		ASSERT_THAT(AreEqual(0, List.RemoveByHandle(Handle)));
	}

	TEST_METHOD(RemoveByLayer_RemovesEveryEntryInThatLayer)
	{
		List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		const FRockCosmeticHandle Other = List.AddEntry(RockCosmeticAppearanceLayers::StatusEffects, Option);

		ASSERT_THAT(AreEqual(2, List.RemoveByLayer(RockCosmeticAppearanceLayers::Equipment)));

		const TArray<FRockMutableCosmeticEntry> Left = List.GetSortedEntries();
		ASSERT_THAT(AreEqual(1, Left.Num()));
		ASSERT_THAT(IsTrue(Left[0].CosmeticHandle == Other));
	}

	TEST_METHOD(RemoveByLayer_EmptyLayer_RemovesNothing)
	{
		List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		ASSERT_THAT(AreEqual(0, List.RemoveByLayer(RockCosmeticAppearanceLayers::UserCustomization)));
		ASSERT_THAT(AreEqual(1, List.Num()));
	}

	TEST_METHOD(GetSortedEntries_OrdersByLayerThenInsertion)
	{
		const FRockCosmeticHandle StatusFirst = List.AddEntry(RockCosmeticAppearanceLayers::StatusEffects, Option);
		const FRockCosmeticHandle UserFirst = List.AddEntry(RockCosmeticAppearanceLayers::UserCustomization, Option);
		const FRockCosmeticHandle EquipmentFirst = List.AddEntry(RockCosmeticAppearanceLayers::Equipment, Option);
		const FRockCosmeticHandle UserSecond = List.AddEntry(RockCosmeticAppearanceLayers::UserCustomization, Option);

		const TArray<FRockMutableCosmeticEntry> Sorted = List.GetSortedEntries();
		ASSERT_THAT(AreEqual(4, Sorted.Num()));
		ASSERT_THAT(IsTrue(Sorted[0].CosmeticHandle == UserFirst));
		ASSERT_THAT(IsTrue(Sorted[1].CosmeticHandle == UserSecond));
		ASSERT_THAT(IsTrue(Sorted[2].CosmeticHandle == EquipmentFirst));
		ASSERT_THAT(IsTrue(Sorted[3].CosmeticHandle == StatusFirst));
	}

	TEST_METHOD(GetSortedEntries_Empty_ReturnsEmpty)
	{
		ASSERT_THAT(AreEqual(0, List.GetSortedEntries().Num()));
	}

	TEST_METHOD(Entry_LessThan_ComparesLayerBeforeHandle)
	{
		FRockCosmeticHandle LowHandle;
		LowHandle.Handle = 1;
		FRockCosmeticHandle HighHandle;
		HighHandle.Handle = 2;

		const FRockMutableCosmeticEntry HighLayerLowHandle(2000, LowHandle, Option);
		const FRockMutableCosmeticEntry LowLayerHighHandle(1000, HighHandle, Option);
		const FRockMutableCosmeticEntry LowLayerLowHandle(1000, LowHandle, Option);

		ASSERT_THAT(IsTrue(LowLayerHighHandle < HighLayerLowHandle));
		ASSERT_THAT(IsFalse(HighLayerLowHandle < LowLayerHighHandle));
		ASSERT_THAT(IsTrue(LowLayerLowHandle < LowLayerHighHandle));
		ASSERT_THAT(IsFalse(LowLayerLowHandle < LowLayerLowHandle));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
