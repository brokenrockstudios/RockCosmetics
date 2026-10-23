// Copyright Broken Rock Studios LLC. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"
#include "RockCosmeticsTestFixture.h"

#include "Animation/AnimMontage.h"
#include "AnimSets/RockAnimSetDataAsset.h"
#include "AnimSets/RockAnimSubsystem.h"
#include "Engine/GameInstance.h"

// URockAnimSubsystem without a running game instance: the subsystem is a bare object inside a transient UGameInstance (Initialize never runs, so nothing is loaded
// through the asset manager), sets are transient data assets added with RegisterAnimSet, and montages are transient
// objects, already loaded, so lookups answer before RequestMontage returns. The skeleton class comes from the
// fixture pawn's character parts (ARCTestSkeletonAActor / BActor) or IRockSkeletonClassInterface.

TEST_CLASS(RockAnimSubsystemTests, "BRS.RockCosmetics.AnimSubsystem")
{
	FRockCosmeticsFixture Fixture;
	TStrongObjectPtr<UGameInstance> GameInstance;
	TStrongObjectPtr<URockAnimSubsystem> Subsystem;
	TStrongObjectPtr<UAnimMontage> PickupMontage;

	FGameplayTag ClassA;
	FGameplayTag ClassB;
	FGameplayTag PickupTag;
	FGameplayTag HeavyPickupTag;
	FGameplayTag MissingTag;

	BEFORE_EACH()
	{
		Fixture.Init();
		// Subsystems are ClassWithin UGameInstance; a bare transient one is enough as the outer
		GameInstance.Reset(NewObject<UGameInstance>(GetTransientPackage()));
		Subsystem.Reset(NewObject<URockAnimSubsystem>(GameInstance.Get()));
		PickupMontage.Reset(NewObject<UAnimMontage>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UAnimMontage::StaticClass(), TEXT("RCTestPickup"))));

		ClassA = FGameplayTag::RequestGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsA"));
		ClassB = FGameplayTag::RequestGameplayTag(TEXT("Cosmetic.SkeletonClass.TestRockCosmeticsB"));
		PickupTag = FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Anim.Pickup"));
		HeavyPickupTag = FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Anim.Pickup.Heavy"));
		MissingTag = FGameplayTag::RequestGameplayTag(TEXT("Test.RockCosmetics.Anim.Missing"));
	}

	/** Registers a set for ClassA that maps the pickup tag to PickupMontage. */
	void RegisterPickupSet()
	{
		URockAnimSetDataAsset* Set = NewObject<URockAnimSetDataAsset>(GetTransientPackage());
		Set->SkeletonClassTag = ClassA;
		Set->MontageMap.Add(PickupTag, TSoftObjectPtr<UAnimMontage>(PickupMontage.Get()));
		Subsystem->RegisterAnimSet(Set);
	}

	void AddPart(TSubclassOf<AActor> PartClass)
	{
		Fixture.Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(PartClass));
	}

	/** RequestMontage's answer: how many times OnDone ran and the last montage it got. */
	struct FAnswer
	{
		int32 Calls = 0;
		UAnimMontage* Montage = nullptr;
	};

	TSharedRef<FAnswer> Request(const AActor* Pawn, FGameplayTag AnimTag)
	{
		TSharedRef<FAnswer> Answer = MakeShared<FAnswer>();
		Subsystem->RequestMontage(Pawn, AnimTag, [Answer](UAnimMontage* Montage)
		{
			++Answer->Calls;
			Answer->Montage = Montage;
		});
		return Answer;
	}

	TEST_METHOD(GetSkeletonClass_FromPartTag_ReturnsIt)
	{
		AddPart(ARCTestSkeletonAActor::StaticClass());
		ASSERT_THAT(IsTrue(Subsystem->GetSkeletonClass(Fixture.Pawn) == ClassA));
	}

	TEST_METHOD(GetSkeletonClass_TwoClassTags_PicksLexicallyFirstWhateverTheOrder)
	{
		TestRunner->AddExpectedMessagePlain(TEXT("SkeletonClass tags, using"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 2);

		AddPart(ARCTestSkeletonBActor::StaticClass());
		AddPart(ARCTestSkeletonAActor::StaticClass());
		ASSERT_THAT(IsTrue(Subsystem->GetSkeletonClass(Fixture.Pawn) == ClassA));

		ARCTestPawn& Other = Fixture.Spawner.SpawnActor<ARCTestPawn>();
		URCTestPartsComponent* OtherParts = FRockCosmeticsFixture::AddComponent<URCTestPartsComponent>(Other);
		OtherParts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestSkeletonAActor::StaticClass()));
		OtherParts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestSkeletonBActor::StaticClass()));
		ASSERT_THAT(IsTrue(Subsystem->GetSkeletonClass(&Other) == ClassA));
	}

	TEST_METHOD(GetSkeletonClass_TwoClassTags_WarnsOncePerPawn)
	{
		TestRunner->AddExpectedMessagePlain(TEXT("SkeletonClass tags, using"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);

		AddPart(ARCTestSkeletonAActor::StaticClass());
		AddPart(ARCTestSkeletonBActor::StaticClass());
		Subsystem->GetSkeletonClass(Fixture.Pawn);
		Subsystem->GetSkeletonClass(Fixture.Pawn);
		Subsystem->GetSkeletonClass(Fixture.Pawn);
	}

	TEST_METHOD(GetSkeletonClass_FromInterface_ReturnsIt)
	{
		ARCTestSkeletonClassPawn& Pawn = Fixture.Spawner.SpawnActor<ARCTestSkeletonClassPawn>();
		ASSERT_THAT(IsTrue(Subsystem->GetSkeletonClass(&Pawn) == ClassB));
	}

	TEST_METHOD(GetSkeletonClass_PartTagWinsOverInterface)
	{
		ARCTestSkeletonClassPawn& Pawn = Fixture.Spawner.SpawnActor<ARCTestSkeletonClassPawn>();
		URCTestPartsComponent* Parts = FRockCosmeticsFixture::AddComponent<URCTestPartsComponent>(Pawn);
		Parts->AddCharacterPart(FRockCosmeticsFixture::MakePart(ARCTestSkeletonAActor::StaticClass()));
		ASSERT_THAT(IsTrue(Subsystem->GetSkeletonClass(&Pawn) == ClassA));
	}

	TEST_METHOD(GetSkeletonClass_NoClass_ReturnsEmptyAndWarnsOncePerPawn)
	{
		TestRunner->AddExpectedMessagePlain(TEXT("No SkeletonClass found on"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 2);

		ASSERT_THAT(IsFalse(Subsystem->GetSkeletonClass(Fixture.Pawn).IsValid()));
		Subsystem->GetSkeletonClass(Fixture.Pawn);
		Subsystem->GetSkeletonClass(Fixture.Pawn);

		ARCTestPawn& Other = Fixture.Spawner.SpawnActor<ARCTestPawn>();
		ASSERT_THAT(IsFalse(Subsystem->GetSkeletonClass(&Other).IsValid()));
		Subsystem->GetSkeletonClass(&Other);
	}

	TEST_METHOD(RequestMontage_LoadedEntry_AnswersOnceWithIt)
	{
		RegisterPickupSet();
		AddPart(ARCTestSkeletonAActor::StaticClass());

		const TSharedRef<FAnswer> Answer = Request(Fixture.Pawn, PickupTag);
		ASSERT_THAT(AreEqual(1, Answer->Calls));
		ASSERT_THAT(IsTrue(Answer->Montage == PickupMontage.Get()));
	}

	TEST_METHOD(RequestMontage_ChildTag_FallsBackToParentEntry)
	{
		RegisterPickupSet();
		AddPart(ARCTestSkeletonAActor::StaticClass());

		const TSharedRef<FAnswer> Answer = Request(Fixture.Pawn, HeavyPickupTag);
		ASSERT_THAT(AreEqual(1, Answer->Calls));
		ASSERT_THAT(IsTrue(Answer->Montage == PickupMontage.Get()));
	}

	TEST_METHOD(RequestMontage_TagNotInSet_AnswersOnceWithNull)
	{
		RegisterPickupSet();
		AddPart(ARCTestSkeletonAActor::StaticClass());

		const TSharedRef<FAnswer> Answer = Request(Fixture.Pawn, MissingTag);
		ASSERT_THAT(AreEqual(1, Answer->Calls));
		ASSERT_THAT(IsNull(Answer->Montage));
	}

	TEST_METHOD(RequestMontage_NoSetForClass_AnswersOnceWithNull)
	{
		RegisterPickupSet();
		AddPart(ARCTestSkeletonBActor::StaticClass());

		const TSharedRef<FAnswer> Answer = Request(Fixture.Pawn, PickupTag);
		ASSERT_THAT(AreEqual(1, Answer->Calls));
		ASSERT_THAT(IsNull(Answer->Montage));
	}

	TEST_METHOD(RequestMontage_NoClass_AnswersOnceWithNull)
	{
		TestRunner->AddExpectedMessagePlain(TEXT("No SkeletonClass found on"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
		RegisterPickupSet();

		const TSharedRef<FAnswer> Answer = Request(Fixture.Pawn, PickupTag);
		ASSERT_THAT(AreEqual(1, Answer->Calls));
		ASSERT_THAT(IsNull(Answer->Montage));
	}

	TEST_METHOD(RequestMontage_NullPawn_AnswersOnceWithNull)
	{
		RegisterPickupSet();

		const TSharedRef<FAnswer> Answer = Request(nullptr, PickupTag);
		ASSERT_THAT(AreEqual(1, Answer->Calls));
		ASSERT_THAT(IsNull(Answer->Montage));
	}

	TEST_METHOD(GetMontage_LoadedEntry_ReturnsIt)
	{
		RegisterPickupSet();
		AddPart(ARCTestSkeletonAActor::StaticClass());

		ASSERT_THAT(IsTrue(Subsystem->GetMontage(Fixture.Pawn, HeavyPickupTag) == PickupMontage.Get()));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
