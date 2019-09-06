// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEditorUISettingCustomization.h"
#include "AssetAssistantEditorUISetting.h"
#include "AssetRegistryModule.h"
#include "ARFilter.h"
#include "DetailLayoutBuilder.h"
#include "Slates/SAssistantTree.h"

#define LOCTEXT_NAMESPACE "AssetAssistant.UISetting"

FAssetAssistantEdMode* FAssetAssistantEditorUISettingCustomization::EditMode = nullptr;
FComboDirectories FAssetAssistantEditorUISettingCustomization::GetAllDirs()
{
	FComboDirectories ComboDirectories = FComboDirectories();
	const FString ThePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	FString RootFolderFullPath;
	FString Ext;

	RootFolderFullPath = "/";
	FPaths::NormalizeDirectoryName(RootFolderFullPath);
	IFileManager& FileManager = IFileManager::Get();
	RootFolderFullPath = ThePath;

	if (!Ext.Contains(TEXT("*")))
	{
		if (Ext == "")
		{
			Ext = "*.*";
		}
		else
		{
			Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
		}
	}

	FileManager.FindFilesRecursive(ComboDirectories.Directories, *RootFolderFullPath, *FString("*.*"), false, true, true);
	ComboDirectories.Names.Add("/");
	for (auto File : ComboDirectories.Directories)
	{
		FString Left = "", Right = "";
		File.Split("/Content/", &Left, &Right);
		ComboDirectories.Names.Add("/"+Right);
	}

	return ComboDirectories;
}


TSharedRef<FAssistantTreeItem> FAssetAssistantEditorUISettingCustomization::GetDirectoryTree()
{
	//Root Level
	TSharedRef<FAssistantTreeItem> RootDir = MakeShareable(new FAssistantTreeItem(NULL, "", FString("Content")));

	TMap<FString, TSharedRef<FAssistantTreeItem>> DirRefs;
	DirRefs.Add("", RootDir);

	TArray<FString> DirectoriesNames;
	IFileManager& FileManager = IFileManager::Get();
	const FString ThePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	FileManager.FindFilesRecursive(DirectoriesNames, *ThePath, *FString("*.*"), false, true, true);

	for (auto File : DirectoriesNames)
	{
		FString Left = "", Right = "", Path = "";
		Path = File.RightChop(ThePath.Len());
		Path.Split("/", &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (Right == "") Right = Path;
		if (Path.Contains("Developers") || Path.Contains("Collections")) continue; // skip developers and collections directories

		FAssistantTreeItemPtr ChildDir = MakeShareable(new FAssistantTreeItem(RootDir, Path, Right));
		FAssistantTreeItemPtr SubDir = *DirRefs.Find(Left);
		SubDir->AddSubDirectory(ChildDir);
		DirRefs.Add(Path, ChildDir.ToSharedRef());
	}

	return RootDir;
}

void FAssetAssistantEditorUISettingCustomization::GetAvaiableClasses(TArray<FString> &AvaiableClassesNames, TArray<int32> &AvaiableClassesCounts)
{
	TMap<FString, int32> classMap;

	FAssetRegistryModule* AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetData;
	FARFilter Filter = FARFilter();
	FString Directory = "/Game";
	Filter.PackagePaths.Add(FName(*Directory));
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	AssetRegistryModule->Get().GetAssets(Filter, AssetData);

	int32 TotalCount = 0;
	for (TArray<FAssetData>::TConstIterator PkgIter = AssetData.CreateConstIterator(); PkgIter; ++PkgIter)
	{
		int32 *mapRef = classMap.Find(*PkgIter->AssetClass.ToString());
		if (mapRef==nullptr)
		{
			classMap.Add(*PkgIter->AssetClass.ToString(), 1);
		}
		else
		{
			*mapRef = *mapRef+1;
		}


		TotalCount++;
	}

	classMap.Add("All Types", TotalCount);
	// sorting class map by key using predicate
	classMap.KeySort([](const FString& A, const FString& B) {return A < B;});

	for (auto& classElement : classMap)
	{
		AvaiableClassesNames.Add(classElement.Key);
		AvaiableClassesCounts.Add(classElement.Value);
	}
}

void FAssetAssistantEditorUISettingCustomization::HideCategoryByToolMode(IDetailLayoutBuilder& DetailBuilder, EToolMode ToolMode)
{
	TArray<FName> AllCategories;

	TArray<FName> FindCategories({ "Find" });
	AllCategories.Append(FindCategories);

	TArray<FName> ModifyCategories({ "Pattern" , "Modify"});
	AllCategories.Append(ModifyCategories);

	TArray<FName> MacroCategories({ "SavedAssetSets", "RecoveryActions", "AutosaveConfig", "Macro" });
	AllCategories.Append(MacroCategories);

	TArray<FName> ExtraCategories({ "Config", "Extra" });
	AllCategories.Append(ExtraCategories);
		
	TArray<FName> AboutCategories({ "About" });
	AllCategories.Append(AboutCategories);

	TArray<FName>* CurrentCategory = nullptr;
	switch (ToolMode)
	{
	case EToolMode::Find:
		CurrentCategory = &FindCategories;
		break;
	case EToolMode::Modify:
		CurrentCategory = &ModifyCategories;
		break;
	case EToolMode::Macro:
		CurrentCategory = &MacroCategories;
		break;
	case EToolMode::Extra:
		CurrentCategory = &ExtraCategories;
		break;
	case EToolMode::About:
		CurrentCategory = &AboutCategories;
		break;
	default:
		break;
	}

	if (CurrentCategory)
	{
		for (auto& Cat : AllCategories)
		{
			if (!CurrentCategory->Contains(Cat))
			{
				DetailBuilder.HideCategory(Cat);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE