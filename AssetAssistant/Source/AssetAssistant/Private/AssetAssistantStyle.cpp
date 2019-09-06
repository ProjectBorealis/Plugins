// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantStyle.h"
#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

FString FAssetAssistantStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("AssetAssistant"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr<FSlateStyleSet> FAssetAssistantStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FAssetAssistantStyle::Get() { return StyleSet; }

FName FAssetAssistantStyle::GetStyleSetName()
{
	static FName AssetAssistantStyleName(TEXT("AssetAssistantStyle"));
	return AssetAssistantStyleName;
}

void FAssetAssistantStyle::Initialize()
{
	// Icon sizes
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin("AssetAssistant")->GetBaseDir() / TEXT("Resources"));
	
	// Icon
	{
		StyleSet->Set("AssetAssistant.AssetAssistantMode", new IMAGE_BRUSH("Icons/icon_Mode_AssetAssistant_40x", Icon40x40));
		StyleSet->Set("AssetAssistant.AssetAssistantMode.Small", new IMAGE_BRUSH("Icons/icon_Mode_AssetAssistant_40x", Icon20x20));
	}

	{
		StyleSet->Set("AssetAssistant.ToolbarBackground", new IMAGE_BRUSH("Icons/ToolbarBackground_128x", Icon128x128, FLinearColor::White, ESlateBrushTileType::Both));
		
		StyleSet->Set("AssetAssistant.FindMode", new IMAGE_BRUSH("Icons/icon_Find_40x", Icon40x40));
		StyleSet->Set("AssetAssistant.FindMode.Small", new IMAGE_BRUSH("Icons/icon_Find_20x", Icon20x20));

		StyleSet->Set("AssetAssistant.ModifyMode", new IMAGE_BRUSH("Icons/icon_Modify_40x", Icon40x40));
		StyleSet->Set("AssetAssistant.ModifyMode.Small", new IMAGE_BRUSH("Icons/icon_Modify_20x", Icon20x20));

		StyleSet->Set("AssetAssistant.MacroMode", new IMAGE_BRUSH("Icons/icon_Macro_40x", Icon40x40));
		StyleSet->Set("AssetAssistant.MacroMode.Small", new IMAGE_BRUSH("Icons/icon_Macro_20x", Icon20x20));

		StyleSet->Set("AssetAssistant.ExtraMode", new IMAGE_BRUSH("Icons/icon_Extra_40x", Icon40x40));
		StyleSet->Set("AssetAssistant.ExtraMode.Small", new IMAGE_BRUSH("Icons/icon_Extra_20x", Icon20x20));
	
		StyleSet->Set("AssetAssistant.AboutMode", new IMAGE_BRUSH("Icons/icon_AboutMode_40x", Icon40x40));
		StyleSet->Set("AssetAssistant.AboutMode.Small", new IMAGE_BRUSH("Icons/icon_AboutMode_20x", Icon20x20));

		StyleSet->Set("AssetAssistant.ButSearch", new IMAGE_BRUSH("Icons/butIcon_search", Icon16x16));

		StyleSet->Set("AssetAssistant.ButtonStyle", FButtonStyle()
			.SetNormal(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, FLinearColor(0.10616, 0.48777, 0.10616)))
			.SetHovered(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, FLinearColor(0.10616, 0.48777, 0.10616)))
			.SetPressed(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, FLinearColor(0.10616, 0.48777, 0.10616))));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FAssetAssistantStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}