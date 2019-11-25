// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceStyle.cpp

#include "SubstanceSourceStyle.h"
#include "SubstanceSourcePrivatePCH.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#if WITH_EDITOR
#include "ClassIconFinder.h"
#include "EditorStyleSet.h"
#endif

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FSubstanceSourceStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define BOX_PLUGIN_BRUSH( RelativePath, ... ) FSlateBoxBrush( FSubstanceSourceStyle::InContent(RelativePath, ".png" ), __VA_ARGS__)
#define TTF_PLUGIN_FONT( RelativePath, ... ) FSlateFontInfo(FSubstanceSourceStyle::InContent(RelativePath, ".ttf" ), __VA_ARGS__)
#define OTF_PLUGIN_FONT( RelativePath, ... ) FSlateFontInfo(FSubstanceSourceStyle::InContent(RelativePath, ".otf" ), __VA_ARGS__)

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo(StyleSet->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo(StyleSet->RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

TSharedPtr<FSlateStyleSet> FSubstanceSourceStyle::StyleSet = nullptr;

void FSubstanceSourceStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon24x24(24.0f, 24.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	const FSlateColor SubstanceSelectionColor(MakeShareable(new FLinearColor(0.0f, 0.50f, 0.885f)));
	const FSlateColor SubstanceSelectionColor_Inactive(MakeShareable(new FLinearColor(0.25f, 0.25f, 0.25f)));
	const FSlateColor SubstanceSelectorColor(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)));
	const FSlateColor SubstanceDefaultForeground(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)));
	const FSlateColor SubstanceInvertedForeground(MakeShareable(new FLinearColor(0.0f, 0.0f, 0.0f)));

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	//Create style set
	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	//Set up image brushes
	StyleSet->Set("SourcePanelBackground", new IMAGE_PLUGIN_BRUSH("SourceUI/BaseBackground", FVector2D(4096.0f, 4096.0f)));
	StyleSet->Set("SubstanceHeaderbarBackground", new IMAGE_PLUGIN_BRUSH("SourceUI/SearchBar", FVector2D(2000.0f, 60.0f)));
	StyleSet->Set("SubstanceAvailableDownloads", new IMAGE_PLUGIN_BRUSH("SourceUI/AvailableDownloads", FVector2D(173.0f, 60.0f)));
	StyleSet->Set("SourceLoginIcon", new IMAGE_PLUGIN_BRUSH("SourceUI/logo-substance-login", FVector2D(1024.0f, 711.0f)));
	StyleSet->Set("SourceNewTileIconOverlay", new IMAGE_PLUGIN_BRUSH("SourceUI/NewIconTileOverlay", FVector2D(50.0f, 50.0f)));
	StyleSet->Set("SourceOwnedTileIconOverlay", new IMAGE_PLUGIN_BRUSH("SourceUI/OwnedTileOverlay", FVector2D(105.0f, 45.0f)));
	StyleSet->Set("SourceFreeAssetTileIconOverlay", new IMAGE_PLUGIN_BRUSH("SourceUI/FreeAssetIconTileOverlay", FVector2D(64.0f, 64.0f)));
	StyleSet->Set("SourceDetailsBackground", new IMAGE_PLUGIN_BRUSH("SourceUI/DetailsBackground", FVector2D(32.0f, 32.0f)));
	StyleSet->Set("SourceDownloadIconOverlay", new IMAGE_PLUGIN_BRUSH("SourceUI/Download_Icon_Overlay", FVector2D(126.0f, 126.0f)));

	StyleSet->Set("SubstanceSourceButtonIcon", new IMAGE_PLUGIN_BRUSH("SourceUI/Source_icon", Icon40x40));
	StyleSet->Set("SubstanceSourceButtonIcon.Small", new IMAGE_PLUGIN_BRUSH("SourceUI/Source_icon", Icon24x24));
	StyleSet->Set("SubstanceSourceButtonIcon.Medium", new IMAGE_PLUGIN_BRUSH("SourceUI/Source_icon", Icon40x40));
	StyleSet->Set("SubstanceSourceButtonIcon.Large", new IMAGE_PLUGIN_BRUSH("SourceUI/Source_icon", Icon64x64));
	StyleSet->Set("SubstanceSourceButtonIcon.XLarge", new IMAGE_PLUGIN_BRUSH("SourceUI/Source_icon", Icon128x128));

	//Set up loading images
	for (int i = 1; i <= ANIMLOADING_IMAGE_COUNT; i++)
	{
		FString Identifier = FString("SourceAnimLoading") + FString::FromInt(i);

		FSlateImageBrush* Brush = new IMAGE_PLUGIN_BRUSH(FString("SourceUI/AnimLoading"), FVector2D(300.0f, 300.0f));

		//Compute new UV region from sprite sheet
		int X = ((i - 1) % ANIMLOADING_SPRITESHEET_COLUMNS) * ANIMLOADING_SPRITESHEET_CELL_DIMENSION;
		int Y = ((i - 1) / ANIMLOADING_SPRITESHEET_COLUMNS) * ANIMLOADING_SPRITESHEET_CELL_DIMENSION;

		FVector2D BoxMin((float)X / (float)ANIMLOADING_SPRITESHEET_STRIDE, (float)Y / (float)ANIMLOADING_SPRITESHEET_STRIDE);
		FVector2D BoxMax(BoxMin.X + ((float)ANIMLOADING_SPRITESHEET_CELL_DIMENSION / (float)ANIMLOADING_SPRITESHEET_STRIDE),
		                 BoxMin.Y + ((float)ANIMLOADING_SPRITESHEET_CELL_DIMENSION / (float)ANIMLOADING_SPRITESHEET_STRIDE));
		Brush->SetUVRegion(FBox2D(BoxMin, BoxMax));

		StyleSet->Set(FName(*Identifier), Brush);
	}

	//Search Bar Text Box
	const FEditableTextBoxStyle SourceSearch =
	    FEditableTextBoxStyle()
	    .SetBackgroundImageNormal(BOX_PLUGIN_BRUSH("SourceUI/SearchBar", FMargin(0.05f)))
	    .SetBackgroundImageHovered(BOX_PLUGIN_BRUSH("SourceUI/SearchBar", FMargin(0.05f)))
	    .SetBackgroundImageFocused(BOX_PLUGIN_BRUSH("SourceUI/SearchBarSel", FMargin(0.05f)))
	    .SetForegroundColor(FSlateColor(FLinearColor(0.65f, 0.65f, 0.65f, 1.0f)))
	    .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 9.0f));
	{
		StyleSet->Set("SourceSearch", SourceSearch);
	}

	//Search Login Input Text Box
	const FEditableTextBoxStyle LoginInput =
	    FEditableTextBoxStyle()
	    .SetBackgroundImageNormal(BOX_PLUGIN_BRUSH("SourceUI/LoginTextInput", FMargin(0.05f)))
	    .SetBackgroundImageHovered(BOX_PLUGIN_BRUSH("SourceUI/LoginTextInput", FMargin(0.05f)))
	    .SetBackgroundImageFocused(BOX_PLUGIN_BRUSH("SourceUI/LoginTextInputSel", FMargin(0.05f)))
	    .SetForegroundColor(FSlateColor(FLinearColor(0.25f, 0.25f, 0.25f, 1.0f)))
	    .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 9.0f));
	{
		StyleSet->Set("SourceLoginInput", LoginInput);
	}

	//Substance Home Button
	const FButtonStyle SH_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/Source_Home_Button", FVector2D(152.0f, 60.0f), 0.05f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/Source_Home_Button", FVector2D(152.0f, 60.0f), 0.05f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/Source_Home_Button", FVector2D(152.0f, 60.0f), 0.05f))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("SubstanceHomeButton", SH_Button);
	}

	//Substance Login Button
	const FButtonStyle SLI_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/Login_button", FVector2D(124.0f, 60.0f), 0.05f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/Login_button", FVector2D(124.0f, 60.0f), 0.05f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/Login_button", FVector2D(124.0f, 60.0f), 0.05f))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("SubstanceLoginButton", SLI_Button);
	}

	//Substance Logout Button
	const FButtonStyle SLO_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/LogoutButton", FVector2D(138.0f, 60.0f), 0.05f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/LogoutButton", FVector2D(138.0f, 60.0f), 0.05f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/LogoutButton", FVector2D(138.0f, 60.0f), 0.05f))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("SubstanceLogoutButton", SLO_Button);
	}

	//Substance MyAssets Button
	const FButtonStyle SMA_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/MyAssetsButton", FVector2D(111.0f, 60.0f), 0.05f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/MyAssetsButton", FVector2D(111.0f, 60.0f), 0.05f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/MyAssetsButton", FVector2D(111.0f, 60.0f), 0.05f))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("SubstanceMyAssetsButton", SMA_Button);
	}

	const FButtonStyle TagSearch_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/TagButton", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/SearchBar", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/SearchBar", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetNormalPadding(FMargin(1.0f))
	    .SetPressedPadding(FMargin(1.0f, 2.0f, 1.0f, 0.0f));
	{
		StyleSet->Set("TagSearchButton", TagSearch_Button);
	}

	//Substance Category Button
	const FButtonStyle SC_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/Cat_Button", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/Cat_Button_Hovered", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/Cat_Button_Selected", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetNormalPadding(FMargin(1.0f))
	    .SetPressedPadding(FMargin(1.0f, 2.0f, 1.0f, 0.0f));
	{
		StyleSet->Set("SubstanceCategoryButton", SC_Button);
	}

	//Substance Detail Tag Button
	const FButtonStyle DT_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/TagButton", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/TagButton", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/TagButton", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetNormalPadding(FMargin(1.0f))
	    .SetPressedPadding(FMargin(1.0f, 2.0f, 1.0f, 0.0f));
	{
		StyleSet->Set("SubstanceDetailTagButton", DT_Button);
	}

	//Substance Category Selected Button
	const FButtonStyle SCS_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/Cat_Button_Selected", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/Cat_Button_Selected", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/Cat_Button_Selected", FVector2D(32, 32), 16.0f / 32.0f))
	    .SetNormalPadding(FMargin(1.0f))
	    .SetPressedPadding(FMargin(1.0f, 2.0f, 1.0f, 0.0f));
	{
		StyleSet->Set("SubstanceCategorySelectedButton", SCS_Button);
	}

	//Substance Login Submit Button
	const FButtonStyle SLS_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/LoginButton", FVector2D(430.0f, 40.0f), 0.05f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/LoginButton", FVector2D(430.0f, 40.0f), 0.05f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/LoginButton", FVector2D(430.0f, 40.0f), 0.05f))
	    .SetNormalPadding(FMargin(1.0f))
	    .SetPressedPadding(FMargin(1.0f, 2.0f, 1.0f, 0.0f));
	{
		StyleSet->Set("SubstanceLoginSubmitButton", SLS_Button);
	}

	//Substance Download Button
	const FButtonStyle SD_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/DownloadButton", FVector2D(160.0f, 41.0f), 0.05f))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/DownloadButton", FVector2D(160.0f, 41.0f), 0.05f))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/DownloadButton", FVector2D(160.0f, 41.0f), 0.05f))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("SubstanceDownloadButton", SD_Button);
	}

	FVector2D ArrowSize(180.0f, 240.0f);
	FMargin ArrowMargin(0.5f);

	//Right Details Arrow Button
	const FButtonStyle AR_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/ArrowRight", ArrowSize, ArrowMargin))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/ArrowRightHovered", ArrowSize, ArrowMargin))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/ArrowRight", ArrowSize, ArrowMargin))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("DetailsArrowRight", AR_Button);
	}

	//Left Details Arrow Button
	const FButtonStyle AL_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/ArrowLeft", ArrowSize, ArrowMargin))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/ArrowLeftHovered", ArrowSize, ArrowMargin))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/ArrowLeft", ArrowSize, ArrowMargin))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("DetailsArrowLeft", AL_Button);
	}

	FVector2D CloseDetailsButtonImageSize(20.0f, 20.0f);
	FMargin CloseDetailsButtonMargin(0.5f);

	//Close Details Panel Button
	const FButtonStyle CDP_Button =
	    FButtonStyle()
	    .SetNormal(BOX_PLUGIN_BRUSH("SourceUI/CloseDetailsPanel", CloseDetailsButtonImageSize, CloseDetailsButtonMargin))
	    .SetHovered(BOX_PLUGIN_BRUSH("SourceUI/CloseDetailsPanelHovered", CloseDetailsButtonImageSize, CloseDetailsButtonMargin))
	    .SetPressed(BOX_PLUGIN_BRUSH("SourceUI/CloseDetailsPanelHovered", CloseDetailsButtonImageSize, CloseDetailsButtonMargin))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("CloseDetailsPanelButton", CDP_Button);
	}

	const FLinearColor LoginLinkButtonNormal(FColor(0X00333333));
	const FLinearColor LoginLinkButtonHovered(0.1f, 0.1f, 0.1f);

	//Substance Footer Login Button
	const FButtonStyle SFLB_Button =
	    FButtonStyle()
	    .SetNormal(FSlateColorBrush(LoginLinkButtonNormal))
	    .SetHovered(FSlateColorBrush(LoginLinkButtonNormal))
	    .SetPressed(FSlateColorBrush(LoginLinkButtonNormal))
	    .SetNormalPadding(FMargin(0.0f))
	    .SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	{
		StyleSet->Set("SubstanceFooterLoginButton", SFLB_Button);
	}

	//Normal Text
	const FTextBlockStyle NormalText =
	    FTextBlockStyle()
	    .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 9.0f))
	    .SetColorAndOpacity(FSlateColor::UseForeground())
	    .SetShadowOffset(FVector2D::ZeroVector)
	    .SetShadowColorAndOpacity(FLinearColor::Black)
	    .SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f))
	    .SetHighlightShape(BOX_PLUGIN_BRUSH("SourceUI/TextBlockHighlightShape", FMargin(3.0f / 8.0f)));

	StyleSet->Set("TextTitle14", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 14.0f))
	              .SetColorAndOpacity(FLinearColor::White));

	StyleSet->Set("CategoryText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(0.35f, 0.35f, 0.35f, 1.0f)));

	StyleSet->Set("CategoryCountText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 7.0f))
	              .SetColorAndOpacity(FLinearColor(0.35f, 0.35f, 0.35f, 1.0f)));

	StyleSet->Set("CategoryTextHovered", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f)));

	StyleSet->Set("CategoryCountTextHovered", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 7.0f))
	              .SetColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f)));

	StyleSet->Set("CategoryTextSelected", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f)));

	StyleSet->Set("CategoryCountTextSelected", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 7.0f))
	              .SetColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f)));

	StyleSet->Set("SourceTileText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 11.0f))
	              .SetColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f)));

	StyleSet->Set("LoginHeader", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 20.0f))
	              .SetColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f)));

	StyleSet->Set("LoginNormalText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 12.0f))
	              .SetColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f)));

	StyleSet->Set("LoginHyperlinkText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 12.0f))
	              .SetColorAndOpacity(FLinearColor(0.0f, 0.75f, 0.95f, 1.0f)));

	StyleSet->Set("DetailsHeaderTop", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 22.0f))
	              .SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

	StyleSet->Set("DetailsHeaderMiddle", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 12.0f))
	              .SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

	StyleSet->Set("DetailsHeaderTags", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

	StyleSet->Set("StatusBarText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

	StyleSet->Set("TagSearchText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

	StyleSet->Set("UpdateLabelText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 7.0f))
	              .SetColorAndOpacity(FLinearColor(FColor(12, 159, 202))));

	const FTableRowStyle SubstanceTableRowStyle = FTableRowStyle()
	        .SetEvenRowBackgroundBrush(FSlateNoResource())
	        .SetEvenRowBackgroundHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)))
	        .SetOddRowBackgroundBrush(FSlateNoResource())
	        .SetOddRowBackgroundHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)))
	        .SetSelectorFocusedBrush(BORDER_BRUSH("Common/Selector", FMargin(4.0f / 16.0f), SubstanceSelectorColor))
	        .SetActiveBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SubstanceSelectionColor))
	        .SetActiveHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SubstanceSelectionColor))
	        .SetInactiveBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SubstanceSelectionColor_Inactive))
	        .SetInactiveHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SubstanceSelectionColor_Inactive))
	        .SetTextColor(SubstanceDefaultForeground)
	        .SetSelectedTextColor(SubstanceInvertedForeground)
	        .SetDropIndicator_Above(BOX_BRUSH("Common/DropZoneIndicator_Above", FMargin(10.0f / 16.0f, 10.0f / 16.0f, 0, 0), SubstanceSelectionColor))
	        .SetDropIndicator_Onto(BOX_BRUSH("Common/DropZoneIndicator_Onto", FMargin(4.0f / 16.0f), SubstanceSelectionColor))
	        .SetDropIndicator_Below(BOX_BRUSH("Common/DropZoneIndicator_Below", FMargin(10.0f / 16.0f, 0, 0, 10.0f / 16.0f), SubstanceSelectionColor));

	StyleSet->Set("SubstanceTableViewRow", SubstanceTableRowStyle);

	// ComboBox:
	StyleSet->Set("SourceComboBox.ToolbarButtonText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f)));

	StyleSet->Set("SourceComboBox.OptionButtonsText", FTextBlockStyle(NormalText)
	              .SetFont(DEFAULT_FONT("Fonts/Roboto-Regular", 10.0f))
	              .SetColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f)));

	const FLinearColor LightGrey(FColor(0xffcccccc));
	const FLinearColor DarkGrey(FColor(0xff777777));
	const FLinearColor DarkerGrey(FColor(0xff666666));

	const FButtonStyle ToolbarButton = FButtonStyle()
	                                   .SetNormal(BOX_BRUSH("common/FlatColorSquare", FVector2D(1.0f, 1.0f), FMargin(1), DarkGrey))
	                                   .SetHovered(BOX_BRUSH("common/FlatColorSquare", FVector2D(1.0f, 1.0f), FMargin(1), DarkerGrey))
	                                   .SetPressed(BOX_BRUSH("common/FlatColorSquare", FVector2D(1.0f, 1.0f), FMargin(1), DarkerGrey))
	                                   .SetNormalPadding(FMargin(2, 2, 2, 2))
	                                   .SetPressedPadding(FMargin(2, 3, 2, 1));

	const FComboButtonStyle OptionButtonsStyle = FComboButtonStyle()
	        .SetDownArrowImage(IMAGE_BRUSH("Common/ComboArrow", Icon8x8))
	        .SetMenuBorderBrush(BOX_BRUSH("common/FlatColorSquare", FVector2D(1.0f, 1.0f), FMargin(1), LightGrey))
	        .SetMenuBorderPadding(FMargin(0.0f));

	const FComboButtonStyle ComboBoxButton = FComboButtonStyle(OptionButtonsStyle)
	        .SetButtonStyle(ToolbarButton)
	        .SetMenuBorderPadding(FMargin(1.0));

	StyleSet->Set("SourceComboBox", FComboBoxStyle()
	              .SetComboButtonStyle(ComboBoxButton)
	             );

#if WITH_EDITOR
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
#endif
}

void FSubstanceSourceStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
#if WITH_EDITOR
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
#endif
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

FName FSubstanceSourceStyle::GetStyleSetName()
{
	static FName SubstanceStyleName(TEXT("SubstanceSourceStyle"));
	return SubstanceStyleName;
}

FString FSubstanceSourceStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Substance"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

#undef IMAGE_PLUGIN_BRUSH
