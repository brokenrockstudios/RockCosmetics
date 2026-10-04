// Copyright Broken Rock Studios LLC. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"
#include "RockCosmeticsTestTypes.h"

#include "Animation/AnimInstance.h"
#include "Cosmetics/RockCharacterPartTypes.h"
#include "Cosmetics/RockCosmeticAnimationTypes.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/StrongObjectPtr.h"

// Pure data: part requests, handles, and the rules that pick a body style or anim layer from cosmetic tags.

TEST_CLASS(RockCharacterPartRequestTests, "BRS.RockCosmetics.PartRequest")
{
	TEST_METHOD(Handle_DefaultIsInvalid)
	{
		FRockCharacterPartHandle Handle;
		ASSERT_THAT(IsFalse(Handle.IsValid()));
	}

	TEST_METHOD(Part_DefaultsToNoCollision)
	{
		FRockCharacterPart Part;
		ASSERT_THAT(IsTrue(Part.CollisionMode == ERockCharacterCustomizationCollisionMode::NoCollision));
		ASSERT_THAT(IsTrue(Part.PartClass == nullptr));
		ASSERT_THAT(IsTrue(Part.SocketName.IsNone()));
	}

	TEST_METHOD(AreEquivalentParts_SameClassAndSocket_IsTrue)
	{
		FRockCharacterPart A;
		A.PartClass = ARCTestHatActor::StaticClass();
		A.SocketName = TEXT("head");
		FRockCharacterPart B = A;
		ASSERT_THAT(IsTrue(FRockCharacterPart::AreEquivalentParts(A, B)));
	}

	TEST_METHOD(AreEquivalentParts_IgnoresCollisionMode)
	{
		FRockCharacterPart A;
		A.PartClass = ARCTestHatActor::StaticClass();
		A.CollisionMode = ERockCharacterCustomizationCollisionMode::NoCollision;
		FRockCharacterPart B = A;
		B.CollisionMode = ERockCharacterCustomizationCollisionMode::UseCollisionFromCharacterPart;
		ASSERT_THAT(IsTrue(FRockCharacterPart::AreEquivalentParts(A, B)));
	}

	TEST_METHOD(AreEquivalentParts_DifferentSocket_IsFalse)
	{
		FRockCharacterPart A;
		A.PartClass = ARCTestHatActor::StaticClass();
		A.SocketName = TEXT("head");
		FRockCharacterPart B = A;
		B.SocketName = TEXT("hand");
		ASSERT_THAT(IsFalse(FRockCharacterPart::AreEquivalentParts(A, B)));
	}

	TEST_METHOD(AreEquivalentParts_DifferentClass_IsFalse)
	{
		FRockCharacterPart A;
		A.PartClass = ARCTestHatActor::StaticClass();
		FRockCharacterPart B = A;
		B.PartClass = ARCTestBootsActor::StaticClass();
		ASSERT_THAT(IsFalse(FRockCharacterPart::AreEquivalentParts(A, B)));
	}
};

TEST_CLASS(RockBodyStyleSelectionTests, "BRS.RockCosmetics.BodyStyle")
{
	TStrongObjectPtr<USkeletalMesh> MeshA;
	TStrongObjectPtr<USkeletalMesh> MeshB;
	TStrongObjectPtr<USkeletalMesh> DefaultMesh;
	FGameplayTag Hat;
	FGameplayTag Head;
	FGameplayTag Boots;

	BEFORE_EACH()
	{
		MeshA.Reset(NewObject<USkeletalMesh>(GetTransientPackage()));
		MeshB.Reset(NewObject<USkeletalMesh>(GetTransientPackage()));
		DefaultMesh.Reset(NewObject<USkeletalMesh>(GetTransientPackage()));
		Hat = FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Hat"));
		Head = FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Head"));
		Boots = FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Boots"));
	}

	FRockAnimBodyStyleSelectionEntry MakeRule(USkeletalMesh* Mesh, const TArray<FGameplayTag>& Tags) const
	{
		FRockAnimBodyStyleSelectionEntry Rule;
		Rule.Mesh = Mesh;
		for (const FGameplayTag& Tag : Tags)
		{
			Rule.RequiredTags.AddTag(Tag);
		}
		return Rule;
	}

	TEST_METHOD(NoRules_ReturnsDefaultMesh)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.DefaultMesh = DefaultMesh.Get();
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer(Hat)) == DefaultMesh.Get()));
	}

	TEST_METHOD(NoRulesAndNoDefault_ReturnsNull)
	{
		FRockAnimBodyStyleSelectionSet Set;
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer()) == nullptr));
	}

	TEST_METHOD(MatchingRule_ReturnsItsMesh)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.DefaultMesh = DefaultMesh.Get();
		Set.MeshRules.Add(MakeRule(MeshA.Get(), {Hat}));
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer(Hat)) == MeshA.Get()));
	}

	TEST_METHOD(NoMatchingRule_ReturnsDefaultMesh)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.DefaultMesh = DefaultMesh.Get();
		Set.MeshRules.Add(MakeRule(MeshA.Get(), {Hat}));
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer(Boots)) == DefaultMesh.Get()));
	}

	TEST_METHOD(Rule_RequiresAllItsTags)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.DefaultMesh = DefaultMesh.Get();
		Set.MeshRules.Add(MakeRule(MeshA.Get(), {Hat, Head}));

		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer(Hat)) == DefaultMesh.Get()));
		FGameplayTagContainer Both;
		Both.AddTag(Hat);
		Both.AddTag(Head);
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(Both) == MeshA.Get()));
	}

	TEST_METHOD(FirstMatchingRuleWins)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.MeshRules.Add(MakeRule(MeshA.Get(), {Hat}));
		Set.MeshRules.Add(MakeRule(MeshB.Get(), {Hat}));
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer(Hat)) == MeshA.Get()));
	}

	TEST_METHOD(RuleWithNoMesh_IsSkipped)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.DefaultMesh = DefaultMesh.Get();
		Set.MeshRules.Add(MakeRule(nullptr, {Hat}));
		Set.MeshRules.Add(MakeRule(MeshB.Get(), {Hat}));
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer(Hat)) == MeshB.Get()));
	}

	TEST_METHOD(RuleWithNoRequiredTags_MatchesAnything)
	{
		FRockAnimBodyStyleSelectionSet Set;
		Set.MeshRules.Add(MakeRule(MeshA.Get(), {}));
		ASSERT_THAT(IsTrue(Set.SelectBestBodyStyle(FGameplayTagContainer()) == MeshA.Get()));
	}
};

TEST_CLASS(RockAnimLayerSelectionTests, "BRS.RockCosmetics.AnimLayer")
{
	// Only the paths that never load an asset: a rule with a layer would LoadSynchronous.

	TEST_METHOD(NoRules_ReturnsDefaultLayer)
	{
		FRockAnimLayerSelectionSet Set;
		Set.DefaultLayer = UAnimInstance::StaticClass();
		ASSERT_THAT(IsTrue(Set.SelectBestLayer(FGameplayTagContainer()) == UAnimInstance::StaticClass()));
	}

	TEST_METHOD(RuleWithNoLayer_IsSkipped)
	{
		FRockAnimLayerSelectionSet Set;
		Set.DefaultLayer = UAnimInstance::StaticClass();
		Set.LayerRules.AddDefaulted();
		ASSERT_THAT(IsTrue(Set.SelectBestLayer(FGameplayTagContainer()) == UAnimInstance::StaticClass()));
	}

	TEST_METHOD(NoRulesAndNoDefault_ReturnsNull)
	{
		FRockAnimLayerSelectionSet Set;
		ASSERT_THAT(IsTrue(Set.SelectBestLayer(FGameplayTagContainer()) == nullptr));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
