/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerChannelTab.h"
#include "MultiPackerDetailPanel.h"
#include "MultiPackerSettings.h"
#include "TileUtils/TilePointer.h"
#include <PropertyEditorModule.h>
#include <IDetailsView.h>
#include <Framework/Docking/TabManager.h>
#include <AssetToolsModule.h>
#include <AssetRegistryModule.h>
#include <ImageUtils.h>
#include <Engine/TextureRenderTarget2D.h>
#include <Editor.h>
#include <EditorFontGlyphs.h>
#include "IAssetTools.h"
#include <AssetToolsModule.h>
#include <Brushes/SlateImageBrush.h>
#include <Widgets/Input/SButton.h>
#include <Widgets/Docking/SDockTab.h>
#include <Widgets/Layout/SBox.h>
#include <Widgets/Text/STextBlock.h>
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "FMultiPackerChannelTab"

const FName MPChannelTabsEditor = FName(TEXT("MPChannelTabsEditor"));

struct FMultiPackerChannelTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName ButtonsID;
};

const FName FMultiPackerChannelTabs::DetailsID(TEXT("Details"));
const FName FMultiPackerChannelTabs::ViewportID(TEXT("Viewport"));
const FName FMultiPackerChannelTabs::ButtonsID(TEXT("Buttons"));

FMultiPackerChannelTab::FMultiPackerChannelTab() {}

FMultiPackerChannelTab::~FMultiPackerChannelTab() {}

void FMultiPackerChannelTab::InitGenericGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost)
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bAllowSearch = false;

	const UMultiPackerSettings* Settings = GetDefault<UMultiPackerSettings>();
	PropertyMPChannel = NewObject<UMultiPackerDetailPanel>(UMultiPackerDetailPanel::StaticClass());
	PropertyMPChannel->TargetDirectory = Settings->GetTargetDirectory();
	PropertyMPChannel->TextureName = Settings->GetTextureChannelName();
	PropertyMPChannel->AddToRoot();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args); 
	PropertyWidget->SetObject(PropertyMPChannel, true);
	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("MultiPackerChannelTabs_Editor")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.10f)
				->SetHideTabWell(true)
				->AddTab(FMultiPackerChannelTabs::ButtonsID, ETabState::OpenedTab)
			)
			->Split				//Buttons
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.90f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->AddTab(FMultiPackerChannelTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.37f)
					->SetHideTabWell(true)
					->AddTab(FMultiPackerChannelTabs::DetailsID, ETabState::OpenedTab)
				)
			)
		);
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = false;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, MPChannelTabsEditor, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, PropertyMPChannel, false);
}

void FMultiPackerChannelTab::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MultiPackerChannelTab", "ChannelTabEditor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FMultiPackerChannelTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FMultiPackerChannelTab::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FMultiPackerChannelTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FMultiPackerChannelTab::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FMultiPackerChannelTabs::ButtonsID, FOnSpawnTab::CreateSP(this, &FMultiPackerChannelTab::SpawnTab_Buttons))
		.SetDisplayName(LOCTEXT("ButtonsTab", "Buttons"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.Details"));
}

void FMultiPackerChannelTab::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FMultiPackerChannelTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FMultiPackerChannelTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FMultiPackerChannelTabs::ButtonsID);
}

FName FMultiPackerChannelTab::GetToolkitFName() const
{
	return FName("FMultiPackerChannelTab");
}

FText FMultiPackerChannelTab::GetBaseToolkitName() const
{
	return LOCTEXT("MultiPackerChannelTabLabel", "MultiPackerChannelTabEditor");
}

FString FMultiPackerChannelTab::GetWorldCentricTabPrefix() const
{
	return TEXT("MPChannelTab");
}

FLinearColor FMultiPackerChannelTab::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FMultiPackerChannelTab::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PropertyMPChannel);
}

TSharedRef<SDockTab> FMultiPackerChannelTab::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerChannelTabs::ViewportID);

	PreviewBrush = MakeShareable(new FSlateImageBrush(PropertyMPChannel->TextureRed, FVector2D(250, 250)));
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		.OnCanCloseTab(false)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(3.0f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Text(LOCTEXT("Output", "Output Texture"))
			]
			+ SVerticalBox::Slot()
			.Padding(3.0f)
			[
				SAssignNew(PreviewWidget, SBorder)
				.HAlign(HAlign_Fill)//HAlign_Fill HAlign_Center
				.VAlign(VAlign_Fill)
				.BorderImage(FEditorStyle::GetBrush("BlackBrush"))
				[
					SNew(SBorder)
					.HAlign(HAlign_Center)//HAlign_Fill HAlign_Center
					.VAlign(VAlign_Center)
					[
						SAssignNew(ImageWidget, SImage)
						.Image(PreviewBrush.Get())
					]
				]
			]
		];
}

TSharedRef<SDockTab> FMultiPackerChannelTab::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerChannelTabs::DetailsID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Details"))
		.ShouldAutosize(true)
		.OnCanCloseTab(false)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(3.0f)
			[
				PropertyWidget.ToSharedRef()
			]
		];
}

TSharedRef<SDockTab> FMultiPackerChannelTab::SpawnTab_Buttons(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerChannelTabs::ButtonsID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Buttons_Title", "Buttons"))
		.ShouldAutosize(true)
		.OnCanCloseTab(false)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(3.f)
			.MaxHeight(30.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					CreateAndSetButton(FOnClicked::CreateSP(this, &FMultiPackerChannelTab::OnClickButton),
					FEditorFontGlyphs::Floppy_O,
					LOCTEXT("ProcessTooltip", "Process TextureMake a new Channel Texture"),
					LOCTEXT("ProcessCS", "Process Texture"))
				]
				+ SHorizontalBox::Slot()
				[
					CreateAndSetButton(FOnClicked::CreateSP(this, &FMultiPackerChannelTab::PreviewRedTexture),
					FEditorFontGlyphs::File_Image_O,
					LOCTEXT("PreviewRT", "Preview Red Texture"),
					LOCTEXT("PreviewR", "Preview Red"))
				]
				+ SHorizontalBox::Slot()
				[
					CreateAndSetButton(FOnClicked::CreateSP(this, &FMultiPackerChannelTab::PreviewGreenTexture),
					FEditorFontGlyphs::File_Image_O,
					LOCTEXT("PreviewGT", "Preview Green Texture"),
					LOCTEXT("PreviewG", "Preview Green"))
				]
				+ SHorizontalBox::Slot()
				[
					CreateAndSetButton(FOnClicked::CreateSP(this, &FMultiPackerChannelTab::PreviewBlueTexture),
					FEditorFontGlyphs::File_Image_O,
					LOCTEXT("PreviewBT", "Preview Blue Texture"),
					LOCTEXT("PreviewB", "Preview Blue"))
				]
				+ SHorizontalBox::Slot()
				[
					CreateAndSetButton(FOnClicked::CreateSP(this, &FMultiPackerChannelTab::PreviewAlphaTexture),
					FEditorFontGlyphs::File_Image_O,
					LOCTEXT("PreviewAT", "Preview Alpha Texture"),
					LOCTEXT("PreviewA", "Preview Alpha"))
				]
			]
		];
}

FReply FMultiPackerChannelTab::OnClickButton()
{
	if (PropertyMPChannel->TextureRed != nullptr && PropertyMPChannel->TextureGreen != nullptr && PropertyMPChannel->TextureBlue != nullptr && (PropertyMPChannel->AlphaChannel ? PropertyMPChannel->TextureAlpha != nullptr : true) )
	{
		const int32 SizeVertical = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(PropertyMPChannel->SizeVertical);
		const int32 SizeHorizontal = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(PropertyMPChannel->SizeHorizontal);
		UTilePointer* TileRed = ProcessTextureChannel(PropertyMPChannel->TextureRed, SizeVertical, SizeHorizontal, PropertyMPChannel->MaskTextureRed, PropertyMPChannel->InvertRed);
		UTilePointer* TileGreen = ProcessTextureChannel(PropertyMPChannel->TextureGreen, SizeVertical, SizeHorizontal, PropertyMPChannel->MaskTextureGreen, PropertyMPChannel->InvertGreen);
		UTilePointer* TileBlue = ProcessTextureChannel(PropertyMPChannel->TextureBlue, SizeVertical, SizeHorizontal, PropertyMPChannel->MaskTextureBlue, PropertyMPChannel->InvertBlue);
		UTilePointer* TileAlpha = ProcessTextureChannel(PropertyMPChannel->TextureAlpha, SizeVertical, SizeHorizontal, PropertyMPChannel->MaskTextureAlpha, PropertyMPChannel->InvertAlpha);
		
		//Texture Output
		TArray<UTilePointer*> OutArray = UTilePointer::DoFinalTextures({ TileRed, TileGreen, TileBlue, TileAlpha }, 4, PropertyMPChannel->AlphaChannel);
		
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		TArray<FString> NewPackageName = TexturePackageName(AssetToolsModule);
		UPackage* package = CreatePackage(NULL, *NewPackageName[1]);
		package->FullyLoad();
		package->Modify(); 
		UTexture2D* NewTexture = OutArray[0]->FillTextureOutput(package, *NewPackageName[0], RF_Public | RF_Standalone);
		//NewTexture->PostEditChange();
		if (NewTexture)
		{
			// package needs saving
			NewTexture->MarkPackageDirty();
			NewTexture->Filter = PropertyMPChannel->TextureFilter;
			NewTexture->SRGB = PropertyMPChannel->sRGB;
			NewTexture->AddressX = PropertyMPChannel->AddressX;
			NewTexture->AddressY = PropertyMPChannel->AddressY;
			NewTexture->PowerOfTwoMode = PropertyMPChannel->PowerOfTwoMode;
			NewTexture->CompressionNoAlpha = !PropertyMPChannel->AlphaChannel;
			NewTexture->CompressionSettings = PropertyMPChannel->CompressionSettings;
			NewTexture->PostEditChange();
			FAssetRegistryModule::AssetCreated(NewTexture);
		}
		PropertyMPChannel->Texture = NewTexture;
		SetPreviewTexture(NewTexture);
	}
	return FReply::Handled();
}

EChannelSelectionInput FMultiPackerChannelTab::GetChannelEnum(EMPChannelMaskParameterColor InEnum)
{
	switch (InEnum)
	{
	case EMPChannelMaskParameterColor::Red:
		return EChannelSelectionInput::CSI_Red;
		break;
	case EMPChannelMaskParameterColor::Green:
		return EChannelSelectionInput::CSI_Green;
		break;
	case EMPChannelMaskParameterColor::Blue:
		return EChannelSelectionInput::CSI_Blue;
		break;
	case EMPChannelMaskParameterColor::Alpha:
		return EChannelSelectionInput::CSI_Alpha;
		break;
	default:
		return EChannelSelectionInput::CSI_Red;
		break;
	}
	return EChannelSelectionInput::CSI_Red;
}

UTilePointer* FMultiPackerChannelTab::ProcessTextureChannel(UTexture2D* InTexture, const int32 InSizeVertical, const int32 InSizeHorizontal, EMPChannelMaskParameterColor InChannel, bool InvertColors)
{
	EChannelSelectionInput InEnum = GetChannelEnum(InChannel);
	//set the data from the Node UTexture
	UTexture2D* Texture = Cast<UTexture2D>(InTexture);
	UTilePointer* InTile = NewObject<UTilePointer>(UTilePointer::StaticClass());
	UTilePointer* Resized = NewObject<UTilePointer>(UTilePointer::StaticClass());
	if (Texture == nullptr)
	{
		InTile->GenerateTextureCanvas(InSizeHorizontal, InSizeVertical);
	}
	else
	{
		InTile->GenerateFromTexture(Texture, Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());
		InTile->FromChannelToTexture(InEnum);
		Resized->ChangeResolution(InSizeHorizontal, InSizeVertical, InTile);
		if (InvertColors)
			Resized->InvertAllChannels();
	}
	return Resized;
}

TArray<FString> FMultiPackerChannelTab::TexturePackageName(FAssetToolsModule& AssetToolsModule)
{
	FString importDirectory = PropertyMPChannel->TargetDirectory.Path;
	importDirectory = importDirectory == "" ? "Textures/" : importDirectory;
	FString ObjectName = PropertyMPChannel->TextureName;
	// last minute sanitizing, just in case we missed one
	ObjectName = ObjectName.Replace(TEXT("*"), TEXT("X"));
	ObjectName = ObjectName.Replace(TEXT("?"), TEXT("Q"));
	ObjectName = ObjectName.Replace(TEXT("!"), TEXT("I"));
	ObjectName = ObjectName.Replace(TEXT("."), TEXT("-"));
	ObjectName = ObjectName.Replace(TEXT("&"), TEXT("_"));
	ObjectName = ObjectName.Replace(TEXT(" "), TEXT("_"));
	FString TextureName = ObjectName;
	if (ObjectName == "None")
		TextureName = "MultiPacker";
	if (TextureName.Contains("/"))
		TextureName = TextureName.Mid(TextureName.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);
	FString NewPackageName = importDirectory.EndsWith( TEXT("/"), ESearchCase::IgnoreCase) ? importDirectory : importDirectory + "/";
	NewPackageName = NewPackageName + TextureName;
	if (!NewPackageName.StartsWith(TEXT("/Game/"), ESearchCase::IgnoreCase))
	{
		NewPackageName = TEXT("/Game/") + NewPackageName;
	}
	FString Name;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(NewPackageName, TextureName, PackageName, Name);
	TArray<FString> NamesOut;
	NamesOut.Add(TextureName);
	NamesOut.Add(PackageName);
	return NamesOut;
}

TSharedRef<class SButton> FMultiPackerChannelTab::CreateAndSetButton(FOnClicked InOnClicked, const TAttribute<FText>& InIcon, const TAttribute<FText>& InTooltip, const TAttribute<FText>& InButton)
{
	return SNew(SButton)
		.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(InTooltip)
		.ContentPadding(FMargin(6, 2))
		.OnClicked(InOnClicked)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
				.Text(InIcon)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Text(InButton)
			]
		];
}

FReply FMultiPackerChannelTab::PreviewRedTexture()
{
	if (PropertyMPChannel && PropertyMPChannel->TextureRed)
	{
		PreviewTexture(PropertyMPChannel->MaskTextureRed, PropertyMPChannel->TextureRed, PropertyMPChannel->InvertRed);
	}
	return FReply::Handled();
}

FReply FMultiPackerChannelTab::PreviewGreenTexture()
{
	if (PropertyMPChannel && PropertyMPChannel->TextureGreen)
	{
		PreviewTexture(PropertyMPChannel->MaskTextureGreen, PropertyMPChannel->TextureGreen, PropertyMPChannel->InvertGreen);
	}
	return FReply::Handled();
}

FReply FMultiPackerChannelTab::PreviewBlueTexture()
{
	if (PropertyMPChannel && PropertyMPChannel->TextureBlue)
	{
		PreviewTexture(PropertyMPChannel->MaskTextureBlue, PropertyMPChannel->TextureBlue, PropertyMPChannel->InvertBlue);
	}
	return FReply::Handled();
}

FReply FMultiPackerChannelTab::PreviewAlphaTexture()
{
	if (PropertyMPChannel && PropertyMPChannel->TextureAlpha)
	{
		PreviewTexture(PropertyMPChannel->MaskTextureAlpha, PropertyMPChannel->TextureAlpha, PropertyMPChannel->InvertAlpha);
	}
	return FReply::Handled();
}

void FMultiPackerChannelTab::PreviewTexture(EMPChannelMaskParameterColor InEnum, UTexture2D* InTexture, bool InvertColors)
{
	EChannelSelectionInput InChannel = GetChannelEnum(InEnum);
	UTexture2D* Texture = Cast<UTexture2D>(InTexture);
	UTilePointer* InTile = NewObject<UTilePointer>(UTilePointer::StaticClass()); 
	InTile->GenerateFromTexture(Texture, Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());
	InTile->FromChannelToTexture(InChannel);
	if (InvertColors)
		InTile->InvertAllChannels();
	SetPreviewTexture(InTile->TileTexture);
}

void FMultiPackerChannelTab::SetPreviewTexture(UTexture2D* InPreviewTexture)
{
	const FGeometry GeoWidget = PreviewWidget->GetCachedGeometry();
	const float SizeSquared = FMath::Min(GeoWidget.GetLocalSize().X, GeoWidget.GetLocalSize().Y);
	PreviewBrush = MakeShareable(new FSlateImageBrush(InPreviewTexture, FVector2D(SizeSquared, SizeSquared)));
	ImageWidget->SetImage(PreviewBrush.Get());
}

#undef LOCTEXT_NAMESPACE
