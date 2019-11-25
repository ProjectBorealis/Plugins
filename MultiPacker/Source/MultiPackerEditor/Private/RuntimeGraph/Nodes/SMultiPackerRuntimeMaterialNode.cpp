/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/Nodes/SMultiPackerRuntimeMaterialNode.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "MultiPackerAssetEditor/MultiPackerColors.h"
#include "AssetThumbnail.h"
#include "AssetRegistryModule.h"
#include <GraphEditorSettings.h>
#include <SGraphPin.h>

void SMultiPackerRuntimeMaterialNode::BuildThumbnailWidget()
{
	EdActorNode->AssetThumbnail = MakeShareable(new FAssetThumbnail(FAssetData(EdActorNode->MultiPackerRuntimeMatNode->MaterialBaseInput), ThumbnailSize.X, ThumbnailSize.Y, FMultiPackerEditorThumbnail::Get()));
}
//////////////////////////////////////////////////////////////////////////
void SMultiPackerRuntimeMaterialNode::Construct(const FArguments& InArgs, UMultiPackerRuntimeMaterialNode* InNode)
{
	ThumbnailSize = FIntPoint(128, 128);
	GraphNode = InNode;
	EdActorNode = InNode;
	BuildThumbnailWidget();
	UpdateGraphNode();
}

void SMultiPackerRuntimeMaterialNode::CreatePinWidgets()
{
	UMultiPackerRuntimeMaterialNode* StateNode = CastChecked<UMultiPackerRuntimeMaterialNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = CreatePinWidget(MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SMultiPackerRuntimeMaterialNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Output)
	{

		FMargin Padding = Settings->GetInputPinPadding();
		Padding.Left *= 0.5f;
		Padding.Right = 0.0f;

		RightNodeBox->AddSlot()
			.HAlign(HAlign_Right)
			.AutoHeight()
			.Padding(Padding)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
}

FSlateColor SMultiPackerRuntimeMaterialNode::GetErrorColor() const
{
	return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

FText SMultiPackerRuntimeMaterialNode::GetErrorMsgToolTip() const
{
	FString Error = FString(TEXT(""));
	UMultiPackerRuntimeMaterialNode* MyNode = CastChecked<UMultiPackerRuntimeMaterialNode>(GraphNode);
	if (!MyNode->MultiPackerRuntimeMatNode->MaterialBaseInput) {
		Error = FString(TEXT("Need Material"));
	}
	ErrorText->SetError(Error);
	return FText::FromString(Error);
}

FSlateColor SMultiPackerRuntimeMaterialNode::GetBorderBackgroundColor() const
{
	UMultiPackerRuntimeMaterialNode* MyNode = CastChecked<UMultiPackerRuntimeMaterialNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : MultiPackerColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor SMultiPackerRuntimeMaterialNode::GetBackgroundColor() const
{
	return MultiPackerColors::NodeBody::Default;
}

EVisibility SMultiPackerRuntimeMaterialNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

FText SMultiPackerRuntimeMaterialNode::GetDescription() const
{
	UMultiPackerRuntimeMaterialNode* MyNode = CastChecked<UMultiPackerRuntimeMaterialNode>(GraphNode);
	return MyNode ? MyNode->GetDescription() : FText::GetEmpty();
}

EVisibility SMultiPackerRuntimeMaterialNode::GetDescriptionVisibility() const
{
	return EVisibility::Hidden;
}

const FSlateBrush* SMultiPackerRuntimeMaterialNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

TSharedRef<SWidget> SMultiPackerRuntimeMaterialNode::CreateNodeContentArea()
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
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				// RIGHT
				SAssignNew(RightNodeBox, SVerticalBox)

			]
		];
}
