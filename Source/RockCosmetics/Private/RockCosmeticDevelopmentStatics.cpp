// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

#include "RockCosmeticDevelopmentStatics.h"

#include "AssetRegistry/AssetRegistryModule.h"

void URockCosmeticDevelopmentStatics::ForEachAuthorityWorld(TFunctionRef<void(UWorld*)> Func)
{
	// Retrieve all PIE worlds that are "authority."
	TArray<UWorld*> AuthorityWorlds;
#if WITH_EDITOR
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (WorldContext.WorldType == EWorldType::PIE)
		{
			if (UWorld* TestWorld = WorldContext.World())
			{
				// “Authority” typically means not a client. 
				// You can refine this check if you only want dedicated or listen servers:
				// (TestWorld->GetNetMode() == NM_DedicatedServer || TestWorld->GetNetMode() == NM_ListenServer)
				if (TestWorld->GetNetMode() != NM_Client) 
				{
					// This is an authority world—call the passed-in lambda/function
					AuthorityWorlds.Add(TestWorld);
				}
			}
		}
	}
#endif
	
	for (UWorld* World : AuthorityWorlds)
	{
		Func(World);  // run the provided lambda on each
	}
}

//@TODO: Actually want to take a lambda and run on every authority world most of the time...
// BRS: See above for potential implementation refactor. Needs more testing
UWorld* URockCosmeticDevelopmentStatics::FindPlayInEditorAuthorityWorld()
{
	check(GEngine);

	// Find the server world (any PIE world will do, in case they are running without a dedicated server, but the ded. server world is ideal)
	UWorld* ServerWorld = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (WorldContext.WorldType == EWorldType::PIE)
		{
			if (UWorld* TestWorld = WorldContext.World())
			{
				if (WorldContext.RunAsDedicated)
				{
					// Ideal case
					ServerWorld = TestWorld;
					break;
				}
				else if (ServerWorld == nullptr)
				{
					ServerWorld = TestWorld;
				}
				else
				{
					// We already have a candidate, see if this one is 'better'
					if (TestWorld->GetNetMode() < ServerWorld->GetNetMode())
					{
						return ServerWorld;
					}
				}
			}
		}
	}
#endif

	return ServerWorld;
}


TArray<FAssetData> URockCosmeticDevelopmentStatics::GetAllBlueprints()
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FName PluginAssetPath;

	TArray<FAssetData> BlueprintList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);

	return BlueprintList;
}

UClass* URockCosmeticDevelopmentStatics::FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass)
{
	FString TargetName = TargetNameRaw;
	TargetName.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);

	TArray<FAssetData> BlueprintList = URockCosmeticDevelopmentStatics::GetAllBlueprints();
	for (const FAssetData& AssetData : BlueprintList)
	{
		if ((AssetData.AssetName.ToString() == TargetName) || (AssetData.GetObjectPathString() == TargetName))
		{
			if (UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset()))
			{
				if (UClass* GeneratedClass = BP->GeneratedClass)
				{
					if (GeneratedClass->IsChildOf(DesiredBaseClass))
					{
						return GeneratedClass;
					}
				}
			}
		}
	}

	return nullptr;
}

UClass* URockCosmeticDevelopmentStatics::FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures)
{
	check(DesiredBaseClass);

	FString TargetName = SearchToken;

	// Check native classes and loaded assets first before resorting to the asset registry
	bool bIsValidClassName = true;
	if (TargetName.IsEmpty() || TargetName.Contains(TEXT(" ")))
	{
		bIsValidClassName = false;
	}
	else if (!FPackageName::IsShortPackageName(TargetName))
	{
		if (TargetName.Contains(TEXT(".")))
		{
			// Convert type'path' to just path (will return the full string if it doesn't have ' in it)
			TargetName = FPackageName::ExportTextPathToObjectPath(TargetName);

			FString PackageName;
			FString ObjectName;
			TargetName.Split(TEXT("."), &PackageName, &ObjectName);

			const bool bIncludeReadOnlyRoots = true;
			FText Reason;
			if (!FPackageName::IsValidLongPackageName(PackageName, bIncludeReadOnlyRoots, &Reason))
			{
				bIsValidClassName = false;
			}
		}
		else
		{
			bIsValidClassName = false;
		}
	}

	UClass* ResultClass = nullptr;
	if (bIsValidClassName)
	{
		ResultClass = UClass::TryFindTypeSlow<UClass>(TargetName);
	}

	// If we still haven't found anything yet, try the asset registry for blueprints that match the requirements
	if (ResultClass == nullptr)
	{
		ResultClass = FindBlueprintClass(TargetName, DesiredBaseClass);
	}

	// Now validate the class (if we have one)
	if (ResultClass != nullptr)
	{
		if (!ResultClass->IsChildOf(DesiredBaseClass))
		{
			if (bLogFailures)
			{
				UE_LOG(LogConsoleResponse, Warning, TEXT("Found an asset %s but it wasn't of type %s"), *ResultClass->GetPathName(), *DesiredBaseClass->GetName());
			}
			ResultClass = nullptr;
		}
	}
	else
	{
		if (bLogFailures)
		{
			UE_LOG(LogConsoleResponse, Warning, TEXT("Failed to find class of type %s named %s"), *DesiredBaseClass->GetName(), *SearchToken);
		}
	}

	return ResultClass;
}
