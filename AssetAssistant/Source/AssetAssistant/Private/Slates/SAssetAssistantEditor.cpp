// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "Slates/SAssetAssistantEditor.h"
#include "AssetAssistantEditorUIView.h"
#include "AssetAssistantEditorUISetting.h"
#include "AssetAssistantEditorUISettingCustomization.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "AssetAssistantEditorUISettingCustomization_Find.h"
#include "AssetAssistantEditorUISettingCustomization_Modify.h"
#include "AssetAssistantEditorUISettingCustomization_Macro.h"
#include "AssetAssistantEditorUISettingCustomization_Extra.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "AssetAssistantEditor"

void SAssetAssistantEditor::Construct(const FArguments& InArgs, TSharedRef<FAssetAssistantEdModeToolkit> InParentToolkit)
{
	TSharedRef<FUICommandList> CommandList = InParentToolkit->GetToolkitCommands();
	FToolBarBuilder ModeToolBar(CommandList, FMultiBoxCustomization::None);
	{
		ModeToolBar.AddToolBarButton(FAssetAssistantEditorCommands::Get().FindMode, NAME_None,		LOCTEXT("Mode.Find",		"Find"), LOCTEXT("Mode.Select.Tooltip", "Find assets / blueprints"));
 		ModeToolBar.AddToolBarButton(FAssetAssistantEditorCommands::Get().ModifyMode, NAME_None,	LOCTEXT("Mode.Modify",		"Modify"), LOCTEXT("Mode.Modify.Tooltip", "Modify assets / blueprints"));
 		ModeToolBar.AddToolBarButton(FAssetAssistantEditorCommands::Get().MacroMode, NAME_None,		LOCTEXT("Mode.Macro",		"Macro"), LOCTEXT("Mode.Macro.Tooltip", "Remember sets of actions, assets, widows"));
		ModeToolBar.AddToolBarButton(FAssetAssistantEditorCommands::Get().ExtraMode, NAME_None,		LOCTEXT("Mode.Extra",		"Extra"), LOCTEXT("Mode.Extra.Tooltip", "Extra UE4 editor functions exposed"));
		ModeToolBar.AddSeparator();

		TSharedRef<SWidget>	widget = SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SCanvas)
				]
				+ SHorizontalBox::Slot()				
				.AutoWidth()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.ToolTipText(LOCTEXT("CreditsTooltip", "Info about the plugin, credits and autor."))
					.ContentPadding(FMargin(1, 0))
					.OnClicked(this, &SAssetAssistantEditor::CreditsButtonClicked)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("Icons.Info"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SCanvas)
				]
				+ SHorizontalBox::Slot()				
				.AutoWidth()
				[
					SNew(SButton)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.ToolTipText(LOCTEXT("RestoreDefault", "Restore default look of the plugin."))
					.ContentPadding(FMargin(1, 0))
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("CurveEd.VisibleHighlight"))
					]
				]
			];
		ModeToolBar.AddWidget(widget);
	}
	

	FPropertyEditorModule& PropetyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(
		/*InUpdateFromSelection=*/ false, 
		/*InLockable=*/ false, 
		/*InAllowSearch=*/false, 
		/*InHideSelectionTip=*/ FDetailsViewArgs::HideNameArea);

	FAssetAssistantEdMode* EditMode = GetEditorMode();

	SelectionDetailView = PropetyEditorModule.CreateDetailView(DetailsViewArgs);

	TSharedPtr<IDetailsView> ToolDetailView_Find = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Find->RegisterInstancedCustomPropertyLayout(UAssetAssistantEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAssetAssistantEditorUISettingCustomization_Find::MakeInstance, EditMode)
		);
	ToolDetailView_Find->SetObject(EditMode->UISetting);

	TSharedPtr<IDetailsView> ToolDetailView_Modify = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Modify->RegisterInstancedCustomPropertyLayout(UAssetAssistantEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAssetAssistantEditorUISettingCustomization_Modify::MakeInstance, EditMode)
	);
	ToolDetailView_Modify->SetObject(EditMode->UISetting);

	TSharedPtr<IDetailsView> ToolDetailView_Macro = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Macro->RegisterInstancedCustomPropertyLayout(UAssetAssistantEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAssetAssistantEditorUISettingCustomization_Macro::MakeInstance, EditMode)
	);
	ToolDetailView_Macro->SetObject(EditMode->UISetting);

	TSharedPtr<IDetailsView> ToolDetailView_Extra = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Extra->RegisterInstancedCustomPropertyLayout(UAssetAssistantEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAssetAssistantEditorUISettingCustomization_Extra::MakeInstance, EditMode)
	);
	ToolDetailView_Extra->SetObject(EditMode->UISetting);
	
	EditMode->ViewResult = new FAssetAssistantEditorUIView(EditMode);

	// Run autosave
	if (//(uint8)EditMode->UISetting->AutosaveRecoverType != 0 &&
		EditMode->UISetting->AutosaveRepeatRate > 0 &&
		!EditMode->UISetting->recoverFlag)
	{
		FString LastTarrayTemp = EditMode->UISetting->LastListViewTArray;
		EditMode->AssetMacro->OnRecover();
		EditMode->UISetting->recoverFlag = true;
		EditMode->UISetting->runAutosave();

		EditMode->UISetting->LastListViewTArray = LastTarrayTemp;
	}

	// Bring back last resultList from savegame
	EditMode->LoadResultList();
	ChildSlot
	[
		SNew(SVerticalBox)
        + SVerticalBox::Slot()
		. FillHeight(1)
		[
			// SPLITTERS
			SNew(SSplitter)
			.Orientation(Orient_Vertical)
			.ResizeMode( ESplitterResizeMode::Fill )

			// Results
			+SSplitter::Slot()
			.Value(0.35f)
			[
				SNew(SBorder)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						EditMode->ViewResult->GetWidget()
					]
				]
			]

			// Vertical menus
			+SSplitter::Slot()
			.Value(0.65f)
			[
				SNew(SBorder)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SAssignNew(ErrorText, SErrorText)
					]

					// Toolbar
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4, 0, 4, 0)
					[
						// Mode ToolBar
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FAssetAssistantStyle::Get()->GetBrush("AssetAssistant.ToolbarBackground"))
							.HAlign(HAlign_Left)
							.Padding(FMargin(5, 5, 0, 5))
							[
								ModeToolBar.MakeWidget()
							]
						]
					]
		
					// Main
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot().Padding(0, 5)
						[
							SNew(SVerticalBox)

							// Selection
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0)
							[
								SNew(SHorizontalBox)
								.Visibility_Static(&GetVisibility, (int32)(EToolMode::Find) | (int32)(EToolMode::Modify))
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.Padding(0)
								[
								SelectionDetailView.ToSharedRef()
								]
							]

							// Tools - Find
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0)
							[
								SNew(SHorizontalBox)
								.Visibility_Static(&GetVisibility, EToolMode::Find)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.Padding(0)
								[
									ToolDetailView_Find.ToSharedRef()
								]
							]

							// Tools - Modify
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0)
							[
								SNew(SHorizontalBox)
								.Visibility_Static(&GetVisibility, EToolMode::Modify)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.Padding(0)
								[
									ToolDetailView_Modify.ToSharedRef()
								]
							]

							// Tools - Macro
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0)
								[
									SNew(SHorizontalBox)
									.Visibility_Static(&GetVisibility, EToolMode::Macro)
									+ SHorizontalBox::Slot()
									.FillWidth(1.0f)
									.Padding(0)
								[
									ToolDetailView_Macro.ToSharedRef()
								]
							]

							// Tools - Extra
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0)
								[
									SNew(SHorizontalBox)
									.Visibility_Static(&GetVisibility, EToolMode::Extra)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.Padding(0)
								[
									ToolDetailView_Extra.ToSharedRef()
								]
							]
						]
					]
				]
			]	
		]
	];
}

FAssetAssistantEdMode* SAssetAssistantEditor::GetEditorMode()
{
	return (FAssetAssistantEdMode*)GLevelEditorModeTools().GetActiveMode(FAssetAssistantEdMode::EM_AssetAssistantEdModeId);
}

FReply SAssetAssistantEditor::CreditsButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(AssetAssistantTabName);
	return FReply::Handled();
}

bool SAssetAssistantEditor::GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	return true;
}

EVisibility SAssetAssistantEditor::GetVisibility(EToolMode InModes)
{
	FAssetAssistantEdMode* EditMode = GetEditorMode();
	if (EditMode && (EditMode->GetCurrentToolMode() == InModes))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SAssetAssistantEditor::GetVisibility(int32 InMode)
{
	FAssetAssistantEdMode* EditMode = GetEditorMode();
	if (EditMode && ((uint8)EditMode->GetCurrentToolMode() & InMode))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE