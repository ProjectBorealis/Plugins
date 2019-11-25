/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/SMultiPackerMaterialNode.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "MultiPackerAssetEditor/MultiPackerColors.h"
#include "AssetThumbnail.h"
#include "AssetRegistryModule.h"
#include <GraphEditorSettings.h>
#include <SGraphPin.h>

void SMultiPackerMaterialNode::BuildThumbnailWidget()
{
	EdActorNode->AssetThumbnail = MakeShareable(new FAssetThumbnail(FAssetData(EdActorNode->MultiPackerMatNode->MaterialBaseInput), ThumbnailSize.X, ThumbnailSize.Y, FMultiPackerEditorThumbnail::Get()));
}
//////////////////////////////////////////////////////////////////////////

void SMultiPackerMaterialNode::Construct(const FArguments& InArgs, UMultiPackerMaterialNode* InNode)
{
	ThumbnailSize = FIntPoint(128, 128);
	GraphNode = InNode;
	EdActorNode = InNode;
	BuildThumbnailWidget();
	UpdateGraphNode();
}

void SMultiPackerMaterialNode::CreatePinWidgets()
{
	UMultiPackerMaterialNode* StateNode = CastChecked<UMultiPackerMaterialNode>(GraphNode);

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

void SMultiPackerMaterialNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

FSlateColor SMultiPackerMaterialNode::GetErrorColor() const
{
	return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

FText SMultiPackerMaterialNode::GetErrorMsgToolTip() const
{
	FString Error = FString(TEXT(""));
	UMultiPackerMaterialNode* MyNode = CastChecked<UMultiPackerMaterialNode>(GraphNode);
	if (!MyNode->MultiPackerMatNode->MaterialBaseInput) {
		Error = FString(TEXT("Need Material"));
	}
	if (MyNode->MultiPackerMatNode->TilesVertical==0) {
		Error = Error == FString(TEXT("")) ? FString(TEXT("Set TilesVertical")) : Error.Append(FString(TEXT(" & Set TilesVertical")));
	}
	if (MyNode->MultiPackerMatNode->TilesHorizontal==0) {
		Error = Error == FString(TEXT("")) ? FString(TEXT("Set TilesHorizontal")) : Error.Append(FString(TEXT(" & Set TilesHorizontal")));
	}
	ErrorText->SetError(Error);
	return FText::FromString(Error);
}

FSlateColor SMultiPackerMaterialNode::GetBorderBackgroundColor() const
{
	UMultiPackerMaterialNode* MyNode = CastChecked<UMultiPackerMaterialNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : MultiPackerColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor SMultiPackerMaterialNode::GetBackgroundColor() const
{
	return MultiPackerColors::NodeBody::Default;
}

EVisibility SMultiPackerMaterialNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

FText SMultiPackerMaterialNode::GetDescription() const
{
	UMultiPackerMaterialNode* MyNode = CastChecked<UMultiPackerMaterialNode>(GraphNode);
	return MyNode ? MyNode->GetDescription() : FText::GetEmpty();
}

EVisibility SMultiPackerMaterialNode::GetDescriptionVisibility() const
{
	return EVisibility::Hidden;
}

const FSlateBrush* SMultiPackerMaterialNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

TSharedRef<SWidget> SMultiPackerMaterialNode::CreateNodeContentArea()
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
