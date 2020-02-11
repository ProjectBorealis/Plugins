// --------------------------------------------------------------------------
// Defines a type for storing and accessing metadata in UnrealEngine.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "MetadataAsset.generated.h"

/**
 * An array of string values
 */
USTRUCT(BlueprintType)
struct FStrings
{
  GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    TArray<FString> Items;
};

/**
 * An array of intger values
 */
USTRUCT(BlueprintType)
struct FIntegers
{
  GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    TArray<int32> Items;
};

/**
 * An array of float values
 */
USTRUCT(BlueprintType)
struct FFloats
{
  GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    TArray<float> Items;
};

/**
 * Metadata contains mappings for string arrays, integer arrays and float arrays
 */
UCLASS(BlueprintType)
class SCENEPROTOCOL_API UMetadataAsset : public UDataAsset
{
  GENERATED_BODY()
public:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    TMap<FString, FStrings> Strings;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    TMap<FString, FIntegers> Integers;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    TMap<FString, FFloats> Floats;
};
