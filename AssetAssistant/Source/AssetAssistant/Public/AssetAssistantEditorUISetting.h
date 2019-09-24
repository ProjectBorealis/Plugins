// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "AssetAssistantEdMode.h"
#include "AssetAssistantEditorUISetting.generated.h"

USTRUCT()
struct FAssetFile
{
	GENERATED_USTRUCT_BODY()

	FAssetFile() {}

	UPROPERTY()
	int32 ItemID;

	UPROPERTY()
	FString AssetName;

	UPROPERTY()
	FString AssetClass;

	UPROPERTY()
	FString AssetPath;

	FORCEINLINE bool operator==(const FAssetFile &Other) const
	{
	return AssetPath == Other.AssetPath;
	}
};

USTRUCT()
struct FComboDirectories
{
	GENERATED_USTRUCT_BODY()

	FComboDirectories() {}

	UPROPERTY()
	TArray<FString> Directories;

	UPROPERTY()
	TArray<FString> Names;
};

UENUM()
enum class EAutosaveRecoverType : uint8
{
	NONE,
	LastAutosave,
	SelectedAssetSet
};

UENUM()
enum class ESetType : uint8
{
	AssetSets,
	AssetAutosaves
};

UENUM()
enum class EFindFrom : uint8
{
	AllFiles,
	SelectedInContentBrowser
};


UENUM()
enum class EModifyFromActorOption : uint8
{
	InstancedStaticMeshActor,
	HierarchicalInstancedStaticMeshActor,
};

UCLASS()
class UAssetAssistantEditorUISetting : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Find
	UPROPERTY(EditAnywhere, Category = "Find")
	FString TextToFind;

	UPROPERTY(Category = "Find", DisplayName = "Find From", EditAnywhere, NonTransactional)
	EFindFrom FindFrom;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Modify
	UPROPERTY(EditAnywhere, Category = "Pattern")
	FString PatternText;

	UPROPERTY(EditAnywhere, Category = "Pattern")
	FString FragmentToReplace;

	UPROPERTY(EditAnywhere, Category = "Pattern", meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"), DisplayName = "Number of copies for Duplicate action")
	int32 MultipleActionCount = 1;

	UPROPERTY(EditAnywhere, Category = "Pattern", DisplayName = "Arr1")
	TArray<FString> Arr1;

	UPROPERTY(EditAnywhere, Category = "Pattern", DisplayName = "Arr2")
	TArray<FString> Arr2;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Macro
	UPROPERTY(EditAnywhere, Category = "Macro")
	FString SetText;

	UPROPERTY(Category = "Macro", DisplayName = "Autosave Recover Type", EditAnywhere, NonTransactional)
	EAutosaveRecoverType AutosaveRecoverType = EAutosaveRecoverType::LastAutosave;

	UPROPERTY(EditAnywhere, Category = "Macro")
	float AutosaveRepeatRate = 2.0f;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extra

	// Plugin hotkey, applied when UE4 project opens
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "1", ClampMax = "10", UIMin = "1", UIMax = "10"))
	int32 PluginHotkey = 2;

	// Preview of available classes
	UPROPERTY(EditAnywhere, Category = "Extra")
	UClass* Class;

	FTimerHandle AutosaveHandle;
	bool recoverFlag = false;

	int32 LastSetNR;
	int32 LastAutosaveSetNR;
	TArray<FString> PatternNames;
	TArray<FString> SetsIDs;
	TArray<FString> AutosaveSetsIDs;
	FString CurrentSetTArrayText;
	FString LastListViewTArray;

	void LoadSet(FString SetID);
	void SaveSet(FString SetID);
	void RemoveSet(FString SetID);

	void SaveResultListFlush();
	void LoadResultList();

	void Load();
	void Save();
	void runAutosave();
	void OnAutosave();
	void ResetPatternNames();
	void SetParent(FAssetAssistantEdMode* EditMode);
	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent & PropertyChangedEvent) override;

private:
	class FAssetAssistantEdMode* ParentMode;

};
