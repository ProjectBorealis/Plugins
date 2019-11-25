/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/SMultiPackerTextureEdNode.h"
#include "AssetThumbnail.h"
#include "AssetRegistryModule.h"
#include "MultiPackerEditorTypes.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include <SGraphPin.h>

void SMultiPackerTextureEdNode::BuildThumbnailWidget()
{
	UTexture* C = EdActorNode->GenericGraphNode->TextureInput;
	FAssetData ValueAsset = FAssetData(C);
	EdActorNode->AssetThumbnail = MakeShareable(new FAssetThumbnail(FAssetData(C), ThumbnailSize.X, ThumbnailSize.Y, FMultiPackerEditorThumbnail::Get()));
}
//////////////////////////////////////////////////////////////////////////
void SMultiPackerTextureEdNode::Construct(const FArguments& InArgs, UMultiPackerTextureEdNode* InNode)
{
	ThumbnailSize = FIntPoint(128, 128);
	GraphNode = InNode;
	EdActorNode = InNode;
	BuildThumbnailWidget();
	UpdateGraphNode();
}

void SMultiPackerTextureEdNode::CreatePinWidgets()
{
	UMultiPackerTextureEdNode* StateNode = CastChecked<UMultiPackerTextureEdNode>(GraphNode);
	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); ++PinIdx)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		TSharedPtr<SGraphPin> NewPin = SNew(SGraphPin, MyPin).UsePinColorForText(false);
		AddPin(NewPin.ToSharedRef());
	}
}

void SMultiPackerTextureEdNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

FSlateColor SMultiPackerTextureEdNode::GetErrorColor() const
{
	return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

FText SMultiPackerTextureEdNode::GetErrorMsgToolTip() const
{
	FString Error = FString(TEXT(""));
	UMultiPackerTextureEdNode* MyNode = CastChecked<UMultiPackerTextureEdNode>(GraphNode);
	if (!MyNode->GenericGraphNode->TextureInput) {
		Error = FString(TEXT("Need Texture"));
	}
	if (MyNode->GenericGraphNode->TilesVertical == 0) {
		Error = Error== FString(TEXT("")) ? FString(TEXT("Set TilesVertical")): Error.Append(FString(TEXT(" & Set TilesVertical")));
	}
	if (MyNode->GenericGraphNode->TilesHorizontal == 0) {
		Error = Error == FString(TEXT("")) ? FString(TEXT("Set TilesHorizontal")) : Error.Append(FString(TEXT(" & Set TilesHorizontal")));
	}
	ErrorText->SetError(Error);
	return FText::FromString(Error);
}

FSlateColor SMultiPackerTextureEdNode::GetBorderBackgroundColor() const
{
	UMultiPackerTextureEdNode* MyNode = CastChecked<UMultiPackerTextureEdNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : MultiPackerColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor SMultiPackerTextureEdNode::GetBackgroundColor() const
{
	return MultiPackerColors::NodeBody::Default;
}

EVisibility SMultiPackerTextureEdNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

FText SMultiPackerTextureEdNode::GetDescription() const
{
	UMultiPackerTextureEdNode* MyNode = CastChecked<UMultiPackerTextureEdNode>(GraphNode);
	return MyNode ? MyNode->GetDescription() : FText::GetEmpty();
}

EVisibility SMultiPackerTextureEdNode::GetDescriptionVisibility() const
{
	return EVisibility::Hidden;
}

const FSlateBrush* SMultiPackerTextureEdNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

TSharedRef<SWidget> SMultiPackerTextureEdNode::CreateNodeContentArea()
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
