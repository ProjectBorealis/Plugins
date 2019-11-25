/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "MultiPackerRuntimeGraph.h"
#include "MultiPackerRuntimeMatNode.h"
#include "Kismet/KismetRenderingLibrary.h"
#include <CanvasItem.h>
#include "Engine/Canvas.h"
#include "RenderUtils.h"
#include "BinPack/MultiPackerRuntimeBinPack.h"
#include <GeneratedCodeHelpers.h>
#include <Materials/MaterialInstanceDynamic.h>
#include "TileUtils/TilePointer.h"

#define LOCTEXT_NAMESPACE "MultiPackerRuntimeGraph"

UMultiPackerRuntimeGraph::UMultiPackerRuntimeGraph()
{
	MatNodeType = UMultiPackerRuntimeMatNode::StaticClass();
#if WITH_EDITORONLY_DATA
	EdGraph = nullptr;
#endif
}

UMultiPackerRuntimeGraph::~UMultiPackerRuntimeGraph()
{
}

void UMultiPackerRuntimeGraph::RuntimePreProcess(UObject* WorldContextObject)
{
	RuntimeData = NewObject<UMultiPackerRuntimeBinPack>();
	RenderTargetPages = RuntimeData->RuntimePreProcess(WorldContextObject, MaterialMap, Pages, Size, Alpha);
}

void UMultiPackerRuntimeGraph::RetrieveMaterialInfo(UObject* WorldContextObject, UMaterialInterface* Material, /*out*/ UTextureRenderTarget2D*& RT_Output, /*out*/ FMaterialDataBinPack& Database)
{
	ValidationOnRuntime(WorldContextObject);
	FMaterialDataBinPack BinPack_value{};
	FCustomThunkTemplates::Map_Find(MaterialMap, FName(*Material->GetName()), /*out*/ BinPack_value);
	Database = BinPack_value;
	RT_Output = RenderTargetPages[BinPack_value.NumPage];
}

void UMultiPackerRuntimeGraph::SetTextureRuntimeOnMaterial(UObject* WorldContextObject, UMaterialInterface* MaterialToGet, UMaterialInstanceDynamic* MaterialToSet, FName Texture, FName Booleans, FName SizePadding)
{
	UTextureRenderTarget2D* RT_Output;
	FMaterialDataBinPack Database;
	RetrieveMaterialInfo(WorldContextObject, MaterialToGet, RT_Output, Database);
	MaterialToSet->SetTextureParameterValue(Texture, RT_Output);
	MaterialToSet->SetVectorParameterValue(Booleans, FLinearColor(1.0f, 0.0f, 0.0f, 0.0f));
	float Red = (float) Database.MaterialRect.x / (float) Database.MaterialRect.width;
	float Green = (float) Database.MaterialRect.y / (float) Database.MaterialRect.height;
	float Blue = (float)Size / (float) Database.MaterialRect.width;
	float Alpha_f = (float)Size / (float) Database.MaterialRect.height;
	Blue = Blue == INFINITY ? 0.0f : Blue;
	Alpha_f = Alpha_f == INFINITY ? 0.0f : Alpha_f;
	Red = Red == INFINITY ? 0.0f : Red;
	Green = Green == INFINITY ? 0.0f : Green;
	MaterialToSet->SetVectorParameterValue(SizePadding, FLinearColor(Red, Green, Blue, Alpha_f));
}

UTexture2D* UMultiPackerRuntimeGraph::GetTileTexture(UObject* WorldContextObject, UMaterialInterface* MaterialToGet)
{
	UTextureRenderTarget2D* RT_Output;
	FMaterialDataBinPack Database;
	RetrieveMaterialInfo(WorldContextObject, MaterialToGet, RT_Output, Database);
	UTilePointer* NewTile = NewObject<UTilePointer>(UTilePointer::StaticClass());
	NewTile->GenerateFromRT(RT_Output, Size, Size);
	UTilePointer* TexTile = NewTile->GetSplitTile(Database.MaterialRect);
	return TexTile->TileTexture;
}

void UMultiPackerRuntimeGraph::ValidationOnRuntime(UObject* WorldContextObject)
{
	if (RenderTargetPages.Num() > 0)
	{
		for (UTextureRenderTarget2D* RT_Transitional : RenderTargetPages)
		{
			if (!::IsValid(RT_Transitional))
			{
				RuntimePreProcess(WorldContextObject);//RT array GC
			}
		}
	}
	else
	{
		RuntimePreProcess(WorldContextObject);//RT array empty
	}
}

void UMultiPackerRuntimeGraph::ClearGraph()
{
	for (int i = 0; i < MatNodes.Num(); ++i)
	{
		UMultiPackerRuntimeMatNode* Node = MatNodes[i];

		Node->ParentNode = nullptr;
	}
	MatNodes.Reset();
}

 #if WITH_EDITOR
 void UMultiPackerRuntimeGraph::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
 {
 	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
 	Super::PostEditChangeProperty(PropertyChangedEvent);
 }

 void UMultiPackerRuntimeGraph::ProcessNodes(UObject* WorldContextObject)
 {
	 RuntimeData = NewObject<UMultiPackerRuntimeBinPack>();
	 Size = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(RenderTargetSizeOutput);
	 RuntimeData->Init(WorldContextObject, Size, Alpha);
	 for (UMultiPackerRuntimeMatNode* Node : MatNodes)
	 {
		 RuntimeData->SetMaterial(Node->MaterialBaseInput, Node->Width, Node->Height);
	 }
	 Pages = RuntimeData->BinPackPages.Num();
	 MaterialMap = RuntimeData->MaterialMap;
	 MarkPackageDirty();
 }

#endif

#undef LOCTEXT_NAMESPACE
