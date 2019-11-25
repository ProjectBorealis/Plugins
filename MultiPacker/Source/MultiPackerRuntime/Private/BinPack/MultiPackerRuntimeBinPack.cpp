/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "BinPack/MultiPackerRuntimeBinPack.h"
#include <Materials/MaterialInterface.h>
#include "BinPack/MaxRectsBinPack.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include <RenderUtils.h>
#include <RenderResource.h>
#include <Engine/Texture.h>

#define LOCTEXT_NAMESPACE "MultiPackerRuntimeBinPack"

void UMultiPackerRuntimeBinPack::Init(UObject* WorldContextObject, int InSize, bool InAlpha)
{
	Size = InSize;
	Alpha = InAlpha;
	WorldContext = WorldContextObject;
	NumPages.Add(MakeNewPage());
}

void UMultiPackerRuntimeBinPack::SetMaterial(UMaterialInterface* Material, int32 Width, int32 Height)
{
	FMaterialDataBinPack NewMat = AddMaterialToMap(Material, Width, Height);
	MaterialMap.Add( FName( *(Material->GetName() ) ), NewMat);
	SetRTOnCanvas(RenderTargetPages[NewMat.NumPage], NewMat.RT_Material, NewMat.MaterialRect, Size, Size);// NewMat.Width, NewMat.Height);
}

FMaterialDataBinPack* UMultiPackerRuntimeBinPack::GetMaterialData(FName Material)
{
	if (MaterialMap.Contains(Material))
	{
		return MaterialMap.Find(Material);
	}
	return nullptr;
}

FMaterialDataBinPack UMultiPackerRuntimeBinPack::AddMaterialToMap(UMaterialInterface* Material, int32 Width, int32 Height)
{
	FMaterialDataBinPack NewMat = FMaterialDataBinPack();
	NewMat.Material = Material;
	NewMat.Width = Size;
	NewMat.Height = Size;
	UTextureRenderTarget2D* RT_new = UMultiPackerBaseEnums::GenerateRenderTarget(Height, Width, Alpha);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContext, RT_new, Material);

	NewMat.RT_Material = RT_new;
	for (int p = 0; p < NumPages.Num(); p++)
	{
		NewMat.MaterialRect = BinPackPages[NumPages[p]]->Insert(Width, Height, EFreeRectChoiceHeuristic::EMCE_Option3);
		if (NewMat.MaterialRect.height == 0 && p == (NumPages.Num() - 1))
		{
			int newPage = MakeNewPage();
			NumPages.Add(newPage);
			NewMat.MaterialRect = BinPackPages[newPage]->Insert(Width, Height, EFreeRectChoiceHeuristic::EMCE_Option3);
			NewMat.NumPage = newPage;
		}
		else
		{
			NewMat.NumPage = NumPages[p];
		}
	}
	return NewMat;
}

int UMultiPackerRuntimeBinPack::MakeNewPage()
{
	UMaxRectsBinPack* BinPack = NewObject<UMaxRectsBinPack>();
	BinPack->Init(Size, Size);
	RenderTargetPages.Add(UMultiPackerBaseEnums::GenerateRenderTarget(Size, Size, Alpha));
	return BinPackPages.Add(BinPack);
}

TArray<UTextureRenderTarget2D*> UMultiPackerRuntimeBinPack::RuntimePreProcess(UObject* WorldContextObject, TMap<FName, FMaterialDataBinPack> InMaterialMap, int InPages, int InSize, bool InAlpha)
{
	WorldContext = WorldContextObject;
	TArray<UTextureRenderTarget2D*> RT_New;
	for (int page = 0; page < InPages; page++) 
	{
		RT_New.Add(UMultiPackerBaseEnums::GenerateRenderTarget(InSize, InSize, InAlpha));
	}
	for (auto& Elem : InMaterialMap)
	{
		UTextureRenderTarget2D* RT_Mat = UMultiPackerBaseEnums::GenerateRenderTarget(Elem.Value.Height, Elem.Value.Width, InAlpha);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContextObject, RT_Mat, Elem.Value.Material);
		SetRTOnCanvas(RT_New[Elem.Value.NumPage], RT_Mat, Elem.Value.MaterialRect, InSize, InSize);
	}
	return RT_New;
}

UTextureRenderTarget2D* UMultiPackerRuntimeBinPack::SetRTOnCanvas(UTextureRenderTarget2D* OutputTexture, UTextureRenderTarget2D* RT_Array_New, FRectSizePadding Rect, uint16 Width, uint16 Height)
{
	UCanvas* Canvas;
	FDrawToRenderTargetContext Context = FDrawToRenderTargetContext();
	FVector2D VectorSize = FVector2D(Width, Height);
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(WorldContext, OutputTexture, Canvas, VectorSize, Context);

	FTexture* RenderTextureResource = (RT_Array_New) ? RT_Array_New->Resource : GWhiteTexture;
	FCanvasTileItem TileItem(FVector2D(Rect.x, Rect.y), RenderTextureResource, FVector2D(Rect.width, Rect.height), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f) + FVector2D::UnitVector, FLinearColor::White);
	TileItem.Rotation = FRotator(0, 0.f, 0);
	TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
	TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_AlphaComposite);
	Canvas->DrawItem(TileItem);

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(WorldContext, Context);
	return OutputTexture;
}

#undef LOCTEXT_NAMESPACE
