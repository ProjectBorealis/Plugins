/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/SMultiPackerOutputNode.h"
#include "CoreMinimal.h"
#include "AssetThumbnail.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "SGraphPin.h"
#include "GraphEditor.h"
#include <GraphEditorSettings.h>
#include <Widgets/SBoxPanel.h>
#include <Widgets/Layout/SWrapBox.h>

//////////////////////////////////////////////////////////////////////////
void SMultiPackerOutputNode::Construct(const FArguments& InArgs, UMultiPackerOutputNode* InNode)
{
	GraphNode = InNode;
	EdActorNode = InNode;
	EdActorNode->ProcessArrayThumbnail();
	UpdateGraphNode();
}

void SMultiPackerOutputNode::CreatePinWidgets()
{
	for (int32 PinIdx = 0; PinIdx < GraphNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = GraphNode->Pins[PinIdx];
		TSharedPtr<SGraphPin> NewPin = CreatePinWidget(MyPin); 
		AddPin(NewPin.ToSharedRef());
	}
}

void SMultiPackerOutputNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		FMargin Padding = Settings->GetInputPinPadding();
		Padding.Left *= 0.5f;
		Padding.Right = 0.0f;

		LeftNodeBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(Padding)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
}

EVisibility SMultiPackerOutputNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

FText SMultiPackerOutputNode::GetDescription() const
{
	return  FText::FromString("Texture");
}

EVisibility SMultiPackerOutputNode::GetDescriptionVisibility() const
{
	return EVisibility::Hidden;
}

const FSlateBrush* SMultiPackerOutputNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

TSharedRef<SWidget> SMultiPackerOutputNode::CreateNodeContentArea()
{
	// NODE CONTENT AREA
	return SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				// LEFT
				SAssignNew(LeftNodeBox, SVerticalBox)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				// RIGHT
				SAssignNew(RightNodeBox, SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					CreateThumbnailContentArea()
				]
			]
		];
}

TSharedRef<SWrapBox> SMultiPackerOutputNode::CreateThumbnailContentArea()
{
	//Get The Size of the Matrix to set Thumbnails
	int ArrayTexturesNum = EdActorNode->GetNumTexturesArray();
	int MatrixSize = FMath::CeilToInt(FMath::Sqrt(ArrayTexturesNum));//get the number of columns and rows
	TSharedRef<SWrapBox> WrapSlot = SNew(SWrapBox)
		+ SWrapBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top);
	WrapSlot->SetWrapWidth((128 + 5) * MatrixSize);
	for (int Thumb = 0; Thumb < ArrayTexturesNum; ++Thumb) 
	{
		WrapSlot->AddSlot()
			[
				SNew(SBox)
				.Padding(5.0f)
				.MaxDesiredWidth(128.0f)
				.MaxDesiredHeight(EdActorNode->IsThumbRectangled(Thumb) ? 64.0f : 128.0f)
				.VAlign(VAlign_Center)
				[
					EdActorNode->GetThumbnailByNum(Thumb)
				]
			];
	}
	return WrapSlot;
}
