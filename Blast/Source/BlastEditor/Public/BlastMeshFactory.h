#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"

#include "NvBlastExtAuthoringTypes.h"

#include "BlastMeshFactory.generated.h"

class USkeletalMesh;

struct FBlastCollisionHull
{
	FBlastCollisionHull() {};

	TArray<FVector3f>				Points;
	TArray<uint32>					Indices;
	TArray<Nv::Blast::HullPolygon>	PolygonData;
};

class UBlastMesh;
class USkeletalMesh;

UCLASS(hideCategories = Object, MinimalAPI)
class UBlastMeshFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()
public:

	UBlastMeshFactory();

	//~ Begin UFactory Interface
	virtual bool CanCreateNew() const override { return false; }
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	//	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual FText GetDisplayName() const override { return NSLOCTEXT("Blast", "BlastMeshFactoryDescription", "Blast Asset"); }
	//~ Begin UFactory Interface

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	//~ End FReimportHandler Interface

	bool						bReimporting;
	UBlastMesh* ReimportMesh;

	BLASTEDITOR_API static FTransform3f GetTransformUE4ToBlastCoordinateSystem(class UFbxSkeletalMeshImportData* SkeletalMeshImportData);
	BLASTEDITOR_API static FTransform3f GetTransformBlastToUE4CoordinateSystem(class UFbxSkeletalMeshImportData* SkeletalMeshImportData);
	BLASTEDITOR_API static void TransformBlastAssetFromUE4ToBlastCoordinateSystem(struct NvBlastAsset* asset, class UFbxSkeletalMeshImportData* SkeletalMeshImportData);
	BLASTEDITOR_API static void TransformBlastAssetToUE4CoordinateSystem(struct NvBlastAsset* asset, class UFbxSkeletalMeshImportData* SkeletalMeshImportData);
	BLASTEDITOR_API static USkeletalMesh* ImportSkeletalMesh(UBlastMesh* BlastMesh, FName skelMeshName,
		FString path, bool bImportCollisionData, class UFbxImportUI* FBXImportUI,
		FFeedbackContext* Warn, TMap<FName, TArray<FBlastCollisionHull>>& hulls);
	BLASTEDITOR_API static bool RebuildPhysicsAsset(UBlastMesh* BlastMesh, const TMap<FName, TArray<FBlastCollisionHull>>& hulls);

protected:

	UPROPERTY()
	class UBlastImportUI* ImportUI;

	/* Gets a name with suffix, and any "special" characters fixed. */
	FName GetNameFromRoot(FName rootName, FString suffix);
	FString GuessFBXPathFromAsset(const FString& BlastAssetPath);
};
