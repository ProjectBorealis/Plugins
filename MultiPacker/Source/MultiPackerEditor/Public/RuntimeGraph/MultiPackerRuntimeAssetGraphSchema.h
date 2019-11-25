/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "GraphEditor.h"
#include "EdGraph/EdGraphSchema.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MultiPackerRuntimeAssetGraphSchema.generated.h"

USTRUCT()
struct FMultiPackerRuntimeAssetSchemaAction_NewNodeMat : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FMultiPackerRuntimeAssetSchemaAction_NewNodeMat()
		: FEdGraphSchemaAction()
	{}

	FMultiPackerRuntimeAssetSchemaAction_NewNodeMat(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
	{}

	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};

//* Action to create new comment *
USTRUCT()
struct FMultiPackerRuntimeAssetSchemaAction_NewNodeComment : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FMultiPackerRuntimeAssetSchemaAction_NewNodeComment() : FEdGraphSchemaAction() {}
	FMultiPackerRuntimeAssetSchemaAction_NewNodeComment(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping) {}
	TSharedPtr<SGraphEditor> Graph = NULL;
	//~ Begin FEdGraphSchemaAction Interface
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};

UCLASS(MinimalAPI)
class UMultiPackerRuntimeAssetGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

	void GetBreakLinkToSubMenuActions(class FMenuBuilder& MenuBuilder, class UEdGraphPin* InGraphPin);

	//~ Begin EdGraphSchema Interface
 	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
 	virtual void GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, class FMenuBuilder* MenuBuilder, bool bIsDebugging) const override;
 	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
 	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
 	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) override;
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 19)
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const;
#endif
 //	virtual void DroppedAssetsOnGraph(const TArray<class FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const override;
// 	virtual void DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node)const override;
 //	virtual int32 GetNodeSelectionCount(const UEdGraph* Graph) const override;
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;
	virtual int32 GetCurrentVisualizationCacheID() const override;
	virtual void ForceVisualizationCacheClear() const override;
	//~ End EdGraphSchema Interface

private:
	static int32 CurrentCacheRefreshID;
};

