/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/Nodes/SMultiPackerRuntimeOutputNode.h"
#include "MultiPackerAssetEditor/MultiPackerColors.h"
#include "CoreMinimal.h"
#include "AssetThumbnail.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "SGraphPin.h"
#include "GraphEditor.h"
#include <GraphEditorSettings.h>

//////////////////////////////////////////////////////////////////////////
void SMultiPackerRuntimeOutputNode::Construct(const FArguments& InArgs, UMultiPackerRuntimeOutputNode* InNode)
{
	GraphNode = InNode;
	EdActorNode = InNode;
	EdActorNode->SetThumnailOutput(nullptr);
	UpdateGraphNode();
}

void SMultiPackerRuntimeOutputNode::CreatePinWidgets()
{
	for (int32 PinIdx = 0; PinIdx < GraphNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = GraphNode->Pins[PinIdx];
		TSharedPtr<SGraphPin> NewPin = CreatePinWidget(MyPin);
		AddPin(NewPin.ToSharedRef());
	}
}

void SMultiPackerRuntimeOutputNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

FSlateColor SMultiPackerRuntimeOutputNode::GetBackgroundColor() const
{
	return MultiPackerColors::NodeBody::Default;
}

EVisibility SMultiPackerRuntimeOutputNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

FText SMultiPackerRuntimeOutputNode::GetDescription() const
{
	return  FText::FromString("Material");
}

EVisibility SMultiPackerRuntimeOutputNode::GetDescriptionVisibility() const
{
	return EVisibility::Hidden;
}

const FSlateBrush* SMultiPackerRuntimeOutputNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

TSharedRef<SWidget> SMultiPackerRuntimeOutputNode::CreateNodeContentArea()
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
			.FillWidth(1.0f)
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
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(128)
					[
						EdActorNode->AssetThumbnail->MakeThumbnailWidget()
					]
				]
			]
		];
}
