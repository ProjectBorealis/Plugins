// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceGraphInstance.h

#pragma once

#ifdef SUBSTANCE_FRAMEWORK_INCLUDED
#include "substance/framework/framework.h"
#endif

#include "Materials/Material.h"
#include "substance/framework/typedefs.h"
#include "SubstanceGraphInstance.generated.h"

/** Forward Declares */
class USubstanceInstanceFactory;
class USubstanceGraphInstance;
class USubstanceImageInput;

namespace SubstanceAir
{
class GraphInstance;
class InputInstanceImage;
struct Preset;
}

/** UE4 related data stored re*/
struct GraphInstanceData
{
	TWeakObjectPtr<USubstanceGraphInstance> ParentGraph;
	bool bHasPendingImageInputRendering;
};

UENUM(BlueprintType)
enum ESubstanceInputType
{
	SIT_Float = 0,
	SIT_Float2 = 1,
	SIT_Float3 = 2,
	SIT_Float4 = 3,
	SIT_Integer = 4,
	SIT_Image = 5,
	SIT_Unused_6 = 6,
	SIT_Unused_7 = 7,
	SIT_Integer2 = 8,
	SIT_Integer3 = 9,
	SIT_Integer4 = 10,
	SIT_MAX = 11,
};

USTRUCT(BlueprintType)
struct FSubstanceInputDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TEnumAsByte<ESubstanceInputType> Type;
};

USTRUCT(BlueprintType)
struct FSubstanceIntInputDesc : public FSubstanceInputDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<int32> Min;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<int32> Max;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<int32> Default;
};

USTRUCT(BlueprintType)
struct FSubstanceFloatInputDesc : public FSubstanceInputDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<float> Min;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<float> Max;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<float> Default;
};

USTRUCT(BlueprintType)
struct FSubstanceInstanceDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	TArray<FSubstanceInputDesc> Inputs;
};

UCLASS(hideCategories = Object, BlueprintType)
class SUBSTANCECORE_API USubstanceGraphInstance : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Framework Graph Instance */
	SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance;

	/** Use for finding and linking the graph instance with other substance objects */
	UPROPERTY()
	FString PackageURL;

	/** The Factory containing the package which created this Instance */
	UPROPERTY()
	USubstanceInstanceFactory* ParentFactory;

	/** Container for linking all of the input images this class uses */
	UPROPERTY()
	TMap<uint32, USubstanceImageInput*> ImageSources;

	/** Material created by the factory */
	UPROPERTY(EditAnywhere, Category = "Substance")
	UMaterial * CreatedMaterial;

	/** Returns a list of all of the input identifiers */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	TArray<FString> GetInputNames();

	/** Gets the type of an input */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	ESubstanceInputType GetInputType(FString InputName);

	/** Set an input with an int value type */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	void SetInputInt(FString Identifier, const TArray<int32>& InputValues);

	/** Set an input with a float value type */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	void SetInputFloat(FString Identifier, const TArray<float>& InputValues);

	/** Set an input with a string value type */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	void SetInputString(FString Identifier, const FString& Value);

	/** Set an input with a color value type */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	void SetInputColor(FString Identifier, const FLinearColor& Color);

	/** Set an input with a bool value type */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	void SetInputBool(FString Identifier, bool Bool);

	/** Get input values in an int value type array */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	TArray<int32> GetInputInt(FString Identifier);

	/** Get input values in a float value type array */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	TArray<float> GetInputFloat(FString Identifier);

	/** Get input values of a string input */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	FString GetInputString(FString Identifier);

	/** Get input values of a color input */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	FLinearColor GetInputColor(FString Identifier);

	/** Get input values of a boolean input */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	bool GetInputBool(FString Identifier);

	/** Get an Int input description converted to a UE wrapper */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	FSubstanceIntInputDesc GetIntInputDesc(FString Identifier);

	/** Get a float input description converted to a UE wrapper */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	FSubstanceFloatInputDesc GetFloatInputDesc(FString Identifier);

	/** Get a graph instance description converted to a UE wrapper */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	FSubstanceInstanceDesc GetInstanceDesc();

	/** Used to store all of the UID/Guid conversions to link legacy textures and output instances */
	UPROPERTY()
	TMap<int32, FGuid> OutputTextureLinkData;

	/** Flag to determine if this is a graph that has been update disabled through the editor */
	UPROPERTY(BlueprintReadOnly, Category = "Substance")
	bool bIsFrozen;

	/** Whether or not this instance has been cooked */
	bool bCooked;

	/** Flag this when an object is set to render but due to changes, needs to update and re-cache */
	bool bDirtyCache;

	/** Used to store UE data in the framework instance */
	GraphInstanceData mUserData;

	/** Input delta values */
	TSharedPtr<SubstanceAir::Preset> InstancePreset;

	/** Checks to see if the generation mode allows for updating and makes sure the graph isn't frozen*/
	bool CanUpdate();

	/** Sets the input image with the given name */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	bool SetInputImg(const FString& InputName, class UObject* Value);

	/** Returns an input image given its name */
	class UObject* GetInputImg(const FString& InputName);

	/** Links the loaded frame work graph instances to this class after load */
	bool LinkLoadedGraphInstance();

	/** Finds a given input instance and links needed data*/
	bool LinkImageInput(SubstanceAir::InputInstanceImage* ImageInput);

	/** Serializes the assets that were created and saved post framework refactor */
	void SerializeCurrent(FArchive& Ar);

	/** Serializes assets that were created before the framework refactor */
	void SerializeLegacy(FArchive& Ar);

	/** Cleans up graph instance memory smoothly */
	void CleanupGraphInstance();

	/** Begin UObject interface */
	virtual void Serialize(FArchive& Ar) override;
	virtual void BeginDestroy() override;
	virtual void PostLoad() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const;
	virtual void PostEditUndo() override;
#endif
	/** End UObject interface */
};
