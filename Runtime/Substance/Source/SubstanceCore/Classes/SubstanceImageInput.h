// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceImageInput.h

#pragma once
#include "SubstanceGraphInstance.h"
#include "Serialization/BulkData.h"
#include "SubstanceImageInput.generated.h"

/** User data for input images */
struct InputImageData
{
	/** Currently stores USubstanceImageInput. It's left as base for when we want to store other texture objects */
	UObject* ImageUObjectSource;
};

class InputInstanceImage;

UCLASS(hideCategories = Object, BlueprintType)
class SUBSTANCECORE_API USubstanceImageInput : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	/** Image color data */
	FByteBulkData ImageRGB;

	/** Image alpha data */
	FByteBulkData ImageA;

	/** User data stored to be referenced by the framework */
	InputImageData mUserData;

	/** Color channels compression (0: disable, 1: auto, 2-99: manual) */
	UPROPERTY(EditAnywhere, Category = General)
	int32 CompressionLevelRGB;

	/** Alpha channel compression (0: disable, 1: auto, 2-99: manual) */
	UPROPERTY(EditAnywhere, Category = General)
	int32 CompressionLevelAlpha;

	/** Image width */
	UPROPERTY(Category = General, VisibleAnywhere)
	int32 SizeX;

	/** Image height */
	UPROPERTY(Category = General, VisibleAnywhere)
	int32 SizeY;

	/** The number of data channels this uses for GPU data */
	UPROPERTY(Category = General, VisibleAnywhere)
	int32 NumComponents;

	/** Path to used to import image input */
	UPROPERTY(Category = File, VisibleAnywhere, BlueprintReadOnly)
	FString SourceFilePath;

	/** Time stamp of the loaded file */
	UPROPERTY(Category = File, VisibleAnywhere, BlueprintReadOnly)
	FString SourceFileTimestamp;

	/** Graph instances that use this image input */
	UPROPERTY(Transient)
	TArray<USubstanceGraphInstance*> Consumers;

#if WITH_EDITOR
	/** Called when ever a property of this class is changed */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Sets which properties that are exposed to the user */
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

	/** Resets the user data of the Substance image input that references this */
	virtual void BeginDestroy() override;

	/** Serializes this class */
	virtual void Serialize(FArchive& Ar) override;

	/** Gets class description */
	virtual FString GetDesc() override;

	/** Returns the size of this object */
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;
};
