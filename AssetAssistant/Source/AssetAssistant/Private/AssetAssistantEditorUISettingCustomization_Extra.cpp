// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEditorUISettingCustomization_Extra.h"
#include "AssetAssistantEditorUISetting.h"
#include "AssetAssistantEditorUIView.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "ObjectTools.h"
#include "Factories/BlueprintFactory.h"
#include "FileHelpers.h"
#include "ConsolidateWindow.h"
#include "Kismet/KismetMathLibrary.h"
#include "Toolkits/AssetEditorManager.h"
#include "EditorStyleSet.h"
#include "EditorFontGlyphs.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "Slates/SItemButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "AssetAssistant.UISetting"

TSharedRef<IDetailCustomization> FAssetAssistantEditorUISettingCustomization_Extra::MakeInstance(class FAssetAssistantEdMode* InEditMode)
{
	FAssetAssistantEditorUISettingCustomization_Extra *AssetExtra = new FAssetAssistantEditorUISettingCustomization_Extra();
	auto Instance = MakeShareable(AssetExtra);
	EditMode = InEditMode;
	EditMode->AssetExtra = AssetExtra;
	return Instance;
}

void FAssetAssistantEditorUISettingCustomization_Extra::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Extra);
}

#undef LOCTEXT_NAMESPACE