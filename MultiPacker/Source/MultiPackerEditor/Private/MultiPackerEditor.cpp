/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "EdGraphUtilities.h"
#include "Styling/SlateStyle.h"
#include "IMultiPackerEditor.h"
#include "MultiPackerAssetTypeActions.h"
#include "LayerDatabase/MultiPackerLayerDatabaseAssetTypeActions.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerTextureEdNode.h"
#include "MultiPackerAssetEditor/Nodes/SMultiPackerTextureEdNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"
#include "MultiPackerAssetEditor/Nodes/SMultiPackerOutputNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerMaterialNode.h"
#include "MultiPackerAssetEditor/Nodes/SMultiPackerMaterialNode.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include <PropertyEditorModule.h>
//#include <Modules/ModuleManager.h>
#include "DetailsProperty/FChannelDatabaseCustomization.h"
#include "DetailsProperty/FLayerDatabaseCustomization.h"
#include "DetailsProperty/FTileCopyCustomization.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeOutputNode.h"
#include "RuntimeGraph/Nodes/SMultiPackerRuntimeOutputNode.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeMaterialNode.h"
#include "RuntimeGraph/Nodes/SMultiPackerRuntimeMaterialNode.h"
#include "RuntimeGraph/MultiPackerRuntimeAssetTypeActions.h"
#include "Engine/StreamableManager.h"
#include <Materials/Material.h>
#include <Materials/MaterialInstance.h>

DEFINE_LOG_CATEGORY(MultiPackerEditor)

#define LOCTEXT_NAMESPACE "MultiPackerEditor"

class FGraphPanelNodeFactory_MultiPacker : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UMultiPackerTextureEdNode* TexEdNode = Cast<UMultiPackerTextureEdNode>(Node))
		{
			return SNew(SMultiPackerTextureEdNode, TexEdNode);
		}
		else {
			if (UMultiPackerOutputNode* OutEdNode = Cast<UMultiPackerOutputNode>(Node))
			{
				return SNew(SMultiPackerOutputNode, OutEdNode);
			}
			else {
				if (UMultiPackerMaterialNode* MatEdNode = Cast<UMultiPackerMaterialNode>(Node))
				{
					return SNew(SMultiPackerMaterialNode, MatEdNode);
				}
			}
		}
		return nullptr;
	}
};

class FGraphPanelNodeFactory_MultiPackerRuntime : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UMultiPackerRuntimeOutputNode* OutEdNode = Cast<UMultiPackerRuntimeOutputNode>(Node))
		{
			return SNew(SMultiPackerRuntimeOutputNode, OutEdNode);
		}
		else {
			if (UMultiPackerRuntimeMaterialNode* MatEdNode = Cast<UMultiPackerRuntimeMaterialNode>(Node))
			{
				return SNew(SMultiPackerRuntimeMaterialNode, MatEdNode);
			}
		}
		return nullptr;
	}
};

TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_MultiPacker;
TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_MultiPackerRuntime;

class FMultiPackerEditor : public IMultiPackerEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
public:
	TSharedPtr<FSlateStyleSet> StyleSet;

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
	void RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate);
	void CompileMaterials();
	UObject* LoadAssetFromContent(FString Path);
private:
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	EAssetTypeCategories::Type MultiPackerAssetCategoryBit;
	TSet< FName > RegisteredPropertyTypes;
};

IMPLEMENT_MODULE( FMultiPackerEditor, MultiPackerEditor )

TSharedPtr<FMultiPackerEditorThumbnail> FMultiPackerEditorThumbnail::Instance;

void FMultiPackerEditor::StartupModule()
{

	StyleSet = MakeShareable(new FSlateStyleSet("MultiPackerStyle"));
	//Content path of this plugin
	FString ContentDir = IPluginManager::Get().FindPlugin("MultiPacker")->GetBaseDir();// MultiPackerEditor

	//The image we wish to load is located inside the Resources folder inside the Base Directory
	//so let's set the content dir to the base dir and manually switch to the Resources folder:
	StyleSet->SetContentRoot(ContentDir);

	//Create a brush from the icon
	FSlateImageBrush* ThumbnailBrush = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/Icon128"), TEXT(".png")), FVector2D(128.f, 128.f));
	FSlateImageBrush* ThumbnailBrushRuntime = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/IconRuntime128"), TEXT(".png")), FVector2D(128.f, 128.f));
	FSlateImageBrush* ThumbnailBrushStyle = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/SlateWidgetStyleAsset_64x"), TEXT(".png")), FVector2D(128.f, 128.f));
	FSlateImageBrush* ThumbnailBrushDatabase = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/ObjectLibrary_64x"), TEXT(".png")), FVector2D(128.f, 128.f));
	if (ThumbnailBrush)
	{
		//In order to bind the thumbnail to our class we need to type ClassThumbnail.X where X is the name of the C++ class of the asset
		StyleSet->Set("ClassThumbnail.MultiPacker", ThumbnailBrush);
		StyleSet->Set("ClassThumbnail.MultiPackerRuntimeGraph", ThumbnailBrushRuntime);
		StyleSet->Set("ClassThumbnail.MultiPackerLayerDatabase", ThumbnailBrushStyle);
		StyleSet->Set("ClassThumbnail.MultiPackerDataBase", ThumbnailBrushDatabase);
		//Register the created style
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}

	FMultiPackerEditorThumbnail::Create();
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	MultiPackerAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Texture Graph")), LOCTEXT("MultiPackerAssetCategory", "MultiPacker"));
	/////MultiPacker Graph
	GraphPanelNodeFactory_MultiPacker = MakeShareable(new FGraphPanelNodeFactory_MultiPacker());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_MultiPacker);
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FMultiPackerAssetTypeActions(MultiPackerAssetCategoryBit)));

	/////MultiPackerRuntime
	GraphPanelNodeFactory_MultiPackerRuntime = MakeShareable(new FGraphPanelNodeFactory_MultiPackerRuntime());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_MultiPackerRuntime);
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FMultiPackerRuntimeAssetTypeActions(MultiPackerAssetCategoryBit)));

	/////LayerDatabase
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FMultiPackerLayerDatabaseAssetTypeActions(MultiPackerAssetCategoryBit)));

	//Custom Property Details
	RegisterCustomPropertyTypeLayout("ChannelDatabase", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FChannelDatabaseCustomizationLayout::MakeInstance));
	RegisterCustomPropertyTypeLayout("LayerDatabase", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLayerDatabaseCustomizationLayout::MakeInstance));
	RegisterCustomPropertyTypeLayout("TileCopy", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FTileCopyCustomizationLayout::MakeInstance));
	CompileMaterials();
}


void FMultiPackerEditor::ShutdownModule()
{
	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}

	if (GraphPanelNodeFactory_MultiPacker.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_MultiPacker);
		GraphPanelNodeFactory_MultiPacker.Reset();
	}

	if (GraphPanelNodeFactory_MultiPackerRuntime.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_MultiPackerRuntime);
		GraphPanelNodeFactory_MultiPackerRuntime.Reset();
	}
}

void FMultiPackerEditor::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

void FMultiPackerEditor::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
	check(PropertyTypeName != NAME_None);

	RegisteredPropertyTypes.Add(PropertyTypeName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}

void FMultiPackerEditor::CompileMaterials()
{
	TArray<FString> MaterialsPath;
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_Tile.MB_Umg_Tile");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Icon.MB_Icon");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_1Image.MB_Umg_1Image");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_2Image.MB_Umg_2Image");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_3Image.MB_Umg_3Image");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_1Image_M.MB_Umg_1Image_M");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_2Image_M.MB_Umg_2Image_M");
	MaterialsPath.Add("/MultiPacker/Material/UMG/MB_Umg_3Image_M.MB_Umg_3Image_M");

	for (FString Path : MaterialsPath)
	{
		UMaterial* MaterialFinder = Cast<UMaterial>(LoadAssetFromContent(Path));
		if (MaterialFinder)
		{
			MaterialFinder->ForceRecompileForRendering();
		}
	}

	TArray<FString> MaterialsInstancePath;
	MaterialsInstancePath.Add("/MultiPacker/Material/UMG/MI_IconSelected.MI_IconSelected");
	for (FString Path : MaterialsPath)
	{
		UMaterialInstance* MaterialFinderInstance = Cast<UMaterialInstance>(LoadAssetFromContent(Path));
		if (MaterialFinderInstance)
		{
			MaterialFinderInstance->ForceRecompileForRendering();
		}
	}
}

UObject* FMultiPackerEditor::LoadAssetFromContent(FString Path)
{
	FStreamableManager assetLoader;
	FStringAssetReference asset;
	asset.SetPath(Path);
	return assetLoader.LoadSynchronous(asset, false, nullptr);
}
#undef LOCTEXT_NAMESPACE

