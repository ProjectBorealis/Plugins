// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BlastMeshEditorDialogs.h"
#include "BlastFractureSettings.h"
#include "BlastFracture.h"
#include "BlastMeshFactory.h"
#include "BlastMesh.h"

#include "HAL/PlatformFilemanager.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorReimportHandler.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "PropertyEditorModule.h"
#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

#include "BlastMeshExporter.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtSerialization.h"
#include "NvBlastExtLlSerialization.h"


//////////////////////////////////////////////////////////////////////////
// SSelectStaticMeshDialog
//////////////////////////////////////////////////////////////////////////

// Constructs this widget with InArgs
void SSelectStaticMeshDialog::Construct(const FArguments& InArgs)
{
	FDetailsViewArgs Args;
	Args.bLockable = false;
	Args.bHideSelectionTip = true;
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	StaticMeshHolder = NewObject<UBlastStaticMeshHolder>();
	StaticMeshHolder->OnStaticMeshSelected.BindSP(this, &SSelectStaticMeshDialog::MeshSelected);
	MeshView = PropertyModule.CreateDetailView(Args);
	MeshView->SetObject(StaticMeshHolder);
	ChildSlot
	[
		SNew(SBorder)
		.Padding(FMargin(0.0f, 3.0f, 1.0f, 0.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(2.0f)
			.AutoHeight()
			[
				MeshView->AsShared()
			]

			+ SVerticalBox::Slot()
			.Padding(2.0f)
			.HAlign(HAlign_Right)
			.AutoHeight()
			[
				SNew(SUniformGridPanel)
				.SlotPadding(2)
				+ SUniformGridPanel::Slot(0, 0)
				[
					SAssignNew(LoadButton, SButton)
					.Text(FText::FromString("Load"))
					.IsEnabled(false)
					.OnClicked(this, &SSelectStaticMeshDialog::LoadClicked)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.Text(FText::FromString("Cancel"))
					.OnClicked(this, &SSelectStaticMeshDialog::CancelClicked)
				]
			]
		]
	];
}

void SSelectStaticMeshDialog::MeshSelected()
{
	LoadButton->SetEnabled(StaticMeshHolder->StaticMesh != nullptr);
}

FReply SSelectStaticMeshDialog::LoadClicked()
{
	IsLoad = true;
	CloseContainingWindow();
	return FReply::Handled();
}

FReply SSelectStaticMeshDialog::CancelClicked()
{
	CloseContainingWindow();
	return FReply::Handled();
}

void SSelectStaticMeshDialog::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

UStaticMesh* SSelectStaticMeshDialog::ShowWindow()
{
	const FText TitleText = NSLOCTEXT("BlastMeshEditor", "BlastMeshEditor_SelectStaticMesh", "Select static mesh");
	// Create the window to pick the class
	TSharedRef<SWindow> SelectStaticMeshWindow = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::Autosized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SSelectStaticMeshDialog> SelectStaticMeshDialog = SNew(SSelectStaticMeshDialog);
	SelectStaticMeshWindow->SetContent(SelectStaticMeshDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddModalWindow(SelectStaticMeshWindow, RootWindow.ToSharedRef());
	}
	else
	{
		//assert here?
	}

	return SelectStaticMeshDialog->IsLoad ? SelectStaticMeshDialog->StaticMeshHolder->StaticMesh : nullptr;
}

//////////////////////////////////////////////////////////////////////////
// SFixChunkHierarchyDialog
//////////////////////////////////////////////////////////////////////////

// Constructs this widget with InArgs
void SFixChunkHierarchyDialog::Construct(const FArguments& InArgs)
{
	FDetailsViewArgs Args;
	Args.bLockable = false;
	Args.bHideSelectionTip = true;
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	Properties = NewObject<UBlastFixChunkHierarchyProperties>();
	PropertyView = PropertyModule.CreateDetailView(Args);
	PropertyView->SetObject(Properties);
	ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(0.0f, 3.0f, 1.0f, 0.0f))
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.AutoHeight()
				[
					PropertyView->AsShared()
				]

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.HAlign(HAlign_Right)
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(2)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(FText::FromString("Fix"))
						.OnClicked(this, &SFixChunkHierarchyDialog::OnClicked, true)
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(FText::FromString("Cancel"))
						.OnClicked(this, &SFixChunkHierarchyDialog::OnClicked, false)
					]
				]
			]
		];
}

FReply SFixChunkHierarchyDialog::OnClicked(bool isFix)
{
	IsFix = isFix;
	CloseContainingWindow();
	return FReply::Handled();
}

void SFixChunkHierarchyDialog::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

bool SFixChunkHierarchyDialog::ShowWindow(TSharedPtr<FBlastFracture> Fracturer, UBlastFractureSettings* FractureSettings, TSet<int32>& SelectedChunkIndices)
{
	const FText TitleText = NSLOCTEXT("BlastMeshEditor", "BlastMeshEditor_FixChunkHierarchy", "Fix chunk hierarchy");
	// Create the window to pick the class
	TSharedRef<SWindow> FixChunkHierarchyWindow = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::Autosized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SFixChunkHierarchyDialog> FixChunkHierarchyDialog = SNew(SFixChunkHierarchyDialog);
	FixChunkHierarchyWindow->SetContent(FixChunkHierarchyDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddModalWindow(FixChunkHierarchyWindow, RootWindow.ToSharedRef());
	}
	else
	{
		//assert here?
	}
	if (FixChunkHierarchyDialog->IsFix)
	{
		Fracturer->BuildChunkHierarchy(FractureSettings, SelectedChunkIndices, FixChunkHierarchyDialog->Properties->Threshold,
			FixChunkHierarchyDialog->Properties->TargetedClusterSize, FixChunkHierarchyDialog->Properties->RemoveMergedOriginalChunks);
	}
	return FixChunkHierarchyDialog->IsFix;
}

bool SExportAssetToFileDialog::ShowWindow(TSharedPtr<FBlastFracture> Fracturer, UBlastFractureSettings* FractureSettings)
{
	IDesktopPlatform* platform = FDesktopPlatformModule::Get();
	if (platform)
	{
		const FText TitleText = NSLOCTEXT("BlastMeshEditor", "BlastMeshEditor_ExportAsset", "Export asset to a file");
		TArray<FString> path;
		if (platform->SaveFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), TitleText.ToString(), TEXT(""), TEXT("asset.obj"), "Wavefront OBJ|*.obj|Autodesk FBX|*.fbx", 0, path))
		{
			int32 dotPs1;
			int32 dotPs2;
			path[0].FindLastChar('/', dotPs1);
			path[0].FindLastChar('\\', dotPs2);
			dotPs1 = FMath::Max(dotPs1, dotPs2);
			path[0].FindLastChar('.', dotPs2);
			

			FString folderPath = path[0].Left(dotPs1);
			FString extension = path[0].RightChop(dotPs2);
			FString name = path[0].Mid(dotPs1 + 1, dotPs2 - dotPs1 - 1);
			FString blastFile = folderPath;
			blastFile.Append("/").Append(name).Append(".blast");
			auto* bmesh = FractureSettings->FractureSession->BlastMesh;
			
			TArray<const char*> matNames;

			for (int32 i = 0; i < bmesh->Mesh->GetMaterials().Num(); ++i)
			{
				int32_t elemCount = bmesh->Mesh->GetMaterials()[i].MaterialSlotName.ToString().Len() + 1;
				char* data = new char[elemCount];
				memcpy(data, TCHAR_TO_UTF8(bmesh->Mesh->GetMaterials()[i].MaterialSlotName.ToString().GetCharArray().GetData()), sizeof(char) * (elemCount - 1));
				data[elemCount - 1] = 0; // set terminating character
				matNames.Add(data);
			}


			FractureSettings->FractureSession->FractureData->materialCount = matNames.Num();
			FractureSettings->FractureSession->FractureData->materialNames = matNames.GetData();
			FString assetName = FractureSettings->FractureSession->BlastMesh->GetName();

			if (extension == TEXT(".fbx"))
			{

				Nv::Blast::IMeshFileWriter* writer = NvBlastExtExporterCreateFbxFileWriter();
				writer->appendMesh(*FractureSettings->FractureSession->FractureData.Get(), TCHAR_TO_UTF8(assetName.GetCharArray().GetData()));
				writer->saveToFile(TCHAR_TO_UTF8(name.GetCharArray().GetData()), TCHAR_TO_UTF8(folderPath.GetCharArray().GetData()));
			}
			if (extension == TEXT(".obj"))
			{
				Nv::Blast::IMeshFileWriter* writer = NvBlastExtExporterCreateObjFileWriter();
				writer->appendMesh(*FractureSettings->FractureSession->FractureData.Get(), TCHAR_TO_UTF8(assetName.GetCharArray().GetData()));
				writer->saveToFile(TCHAR_TO_UTF8(name.GetCharArray().GetData()), TCHAR_TO_UTF8(folderPath.GetCharArray().GetData()));
			}


			{
				Nv::Blast::ICollisionExporter* writer = NvBlastExtExporterCreateJsonFileWriter();
				FString jsonFile = folderPath;
				jsonFile.Append("/").Append(name).Append(".json");
				const Nv::Blast::AuthoringResult* Auth = FractureSettings->FractureSession->FractureData.Get();
				writer->writeCollision(TCHAR_TO_UTF8(jsonFile.GetCharArray().GetData()), Auth->chunkCount, Auth->collisionHullOffset, Auth->collisionHull);
			}


			UBlastMeshFactory::TransformBlastAssetFromUE4ToBlastCoordinateSystem(FractureSettings->FractureSession->FractureData->asset, nullptr);


			for (int32_t i = 0; i < matNames.Num(); ++i)
			{
				delete[] matNames[i];
			}

			void* buffer;
			Nv::Blast::ExtSerialization* serializer = NvBlastExtSerializationCreate();
			serializer->setSerializationEncoding(Nv::Blast::ExtSerialization::EncodingID::CapnProtoBinary);
			uint64_t bsize = serializer->serializeIntoBuffer(buffer, FractureSettings->FractureSession->FractureData->asset, Nv::Blast::LlObjectTypeID::Asset);
			if (bsize)
			{
				IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
				IFileHandle* file = platformFile.OpenWrite(*blastFile);
				if (file)
				{
					file->Write((uint8*)buffer, bsize);
				}
				delete file;
			}
			UBlastMeshFactory::TransformBlastAssetToUE4CoordinateSystem(FractureSettings->FractureSession->FractureData->asset, nullptr);
			NVBLAST_FREE(buffer);
			buffer = nullptr;
			serializer->release();
		}
	}
	return false;
}




//////////////////////////////////////////////////////////////////////////
// SUVCoordinatesDialog
//////////////////////////////////////////////////////////////////////////

void SFitUvCoordinatesDialog::Construct(const FArguments& InArgs)
{
	mSquareSize = 1.f;
	isOnlySelectedToggle = ECheckBoxState::Unchecked;
	ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(0.0f, 3.0f, 1.0f, 0.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.AutoHeight()

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.HAlign(HAlign_Left)
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(2)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(STextBlock).Text(FText::FromString("Square size")).Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SNumericEntryBox<float>).MinValue(0).OnValueChanged(this, &SFitUvCoordinatesDialog::OnSquareSizeChanged).Value(this, &SFitUvCoordinatesDialog::getSquareSize)
					]
				]

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.HAlign(HAlign_Left)
				.AutoHeight()
				[
					SNew(SCheckBox).OnCheckStateChanged(this, &SFitUvCoordinatesDialog::OnIsSelectedToggleChanged).IsChecked(this, &SFitUvCoordinatesDialog::getIsOnlySelectedToggle).ToolTipText(NSLOCTEXT("BlastMeshEditor", "UVFITTOOL_ONLYSELC", "Fit only selected chunks"))
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("BlastMeshEditor", "OnlySelLabel", "Fit UV for only selected chunks."))
					]
				]

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.HAlign(HAlign_Right)
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(2)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(FText::FromString("Fit UV"))
						.OnClicked(this, &SFitUvCoordinatesDialog::OnClicked, true)
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(FText::FromString("Cancel"))
						.OnClicked(this, &SFitUvCoordinatesDialog::OnClicked, false)
					]
				]
			]
		];
}

FReply SFitUvCoordinatesDialog::OnClicked(bool isFix)
{
	shouldFix = isFix;
	CloseContainingWindow();
	return FReply::Handled();
}

void SFitUvCoordinatesDialog::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

bool SFitUvCoordinatesDialog::ShowWindow(TSharedPtr<FBlastFracture> Fracturer, UBlastFractureSettings* FractureSettings, TSet<int32>& ChunkIndices)
{
	const FText TitleText = NSLOCTEXT("FitUVDialog", "FitUVDialog", "Fit UV");
	// Create the window to pick the class
	TSharedRef<SWindow> FitUV = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::Autosized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SFitUvCoordinatesDialog> FitUVDialog = SNew(SFitUvCoordinatesDialog);
	FitUV->SetContent(FitUVDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddModalWindow(FitUV, RootWindow.ToSharedRef());
	}
	else
	{
		//assert here?
	}

	if (FitUVDialog->shouldFix)
	{
		Fracturer->FitUvs(FractureSettings, FitUVDialog->mSquareSize, FitUVDialog->isOnlySelectedToggle == ECheckBoxState::Checked, ChunkIndices);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// SRebuildCollisionMeshDialog
//////////////////////////////////////////////////////////////////////////

// Constructs this widget with InArgs
void SRebuildCollisionMeshDialog::Construct(const FArguments& InArgs)
{
	FDetailsViewArgs Args;
	Args.bLockable = false;
	Args.bHideSelectionTip = true;
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	Properties = NewObject<UBlastRebuildCollisionMeshProperties>();
	PropertyView = PropertyModule.CreateDetailView(Args);
	PropertyView->SetObject(Properties);
	ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(0.0f, 3.0f, 1.0f, 0.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.AutoHeight()
				[
					PropertyView->AsShared()
				]

				+ SVerticalBox::Slot()
				.Padding(2.0f)
				.HAlign(HAlign_Right)
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(2)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(FText::FromString("Build"))
						.OnClicked(this, &SRebuildCollisionMeshDialog::OnClicked, true)
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(FText::FromString("Cancel"))
						.OnClicked(this, &SRebuildCollisionMeshDialog::OnClicked, false)
					]
				]
			]
		];
}

FReply SRebuildCollisionMeshDialog::OnClicked(bool InIsRebuild)
{
	IsRebuild = InIsRebuild;
	CloseContainingWindow();
	return FReply::Handled();
}

void SRebuildCollisionMeshDialog::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

bool SRebuildCollisionMeshDialog::ShowWindow(TSharedPtr<FBlastFracture> Fracturer, UBlastFractureSettings* FractureSettings, TSet<int32>& ChunkIndices)
{
	const FText TitleText = NSLOCTEXT("BlastMeshEditor", "BlastMeshEditor_RebuildCollisionMesh", "Rebuild collision mesh");
	// Create the window to pick the class
	TSharedRef<SWindow> RebuildCollisionMeshWindow = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::Autosized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SRebuildCollisionMeshDialog> RebuildCollisionMeshDialog = SNew(SRebuildCollisionMeshDialog);
	RebuildCollisionMeshWindow->SetContent(RebuildCollisionMeshDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddModalWindow(RebuildCollisionMeshWindow, RootWindow.ToSharedRef());
	}
	else
	{
		//assert here?
	}
	if (RebuildCollisionMeshDialog->IsRebuild)
	{
		Fracturer->RebuildCollisionMesh(FractureSettings, RebuildCollisionMeshDialog->Properties->MaximumNumberOfHulls,
			RebuildCollisionMeshDialog->Properties->VoxelGridResolution, 
			RebuildCollisionMeshDialog->Properties->Concavity,
			RebuildCollisionMeshDialog->Properties->IsOnlyForSelectedChunks ? ChunkIndices : TSet<int32>());
	}
	return RebuildCollisionMeshDialog->IsRebuild;
}
