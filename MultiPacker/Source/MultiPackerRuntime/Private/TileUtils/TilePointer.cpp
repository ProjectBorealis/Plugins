/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "TileUtils/TilePointer.h"
#include "Engine/TextureRenderTarget2D.h"
#include <ImageUtils.h>
#include <AssetRegistryModule.h>
#include <Misc/PackageName.h>
#include "Kismet/KismetRenderingLibrary.h"
#include <CanvasItem.h>
#include <CanvasTypes.h>
#include "Engine/Canvas.h"
#include <Async/ParallelFor.h>

#define LOCTEXT_NAMESPACE "UTilePointer"

void UTilePointer::GenerateFromRT(UTextureRenderTarget2D* InTileRT, uint16 InTileWidth, uint16 InTileHeight)
{
	NameUTile = "GeneratedFromRT";
	TileRT = InTileRT;
	GenerateDefaultVars(InTileWidth, InTileHeight);
	TileTexture = InTileRT->ConstructTexture2D(InTileRT->GetOuter(), InTileRT->GetName().Append("_T"), InTileRT->GetMaskedFlags(), CTF_Default | CTF_Compress | CTF_DeferCompression, NULL);
	TileTexture->UpdateResource();
	TileTexture->Modify();
	GetPointerFromRT();
}

void UTilePointer::GetPointerFromRT()
{
	FRenderTarget* RenderTarget = TileRT->GameThread_GetRenderTargetResource();
	GenerateDefaultVars(TileRT->GetSurfaceWidth(), TileRT->GetSurfaceHeight());
	RenderTarget->ReadPixels(TileData);
}

void UTilePointer::InitializeSDFFloatArray()
{
	TileDataFloat.Empty(TileDimension);
	for (uint32 pixels = 0; pixels < TileDimension; ++pixels)
	{
		TileDataFloat.Add(TileData[pixels].R > 18 ? (TileData[pixels].R / 255.0f) : 0.0f);
	}
}

void UTilePointer::setPixelColor(FColor pointer, uint8 red, uint8 green, uint8 blue, uint8 alpha)
{
	pointer = FColor(red, green, blue, alpha);
}

void UTilePointer::GenerateTextureCanvas(const uint16 Width, const uint16 Height)
{
	TileTexture = UTexture2D::CreateTransient(Width, Height);
#if WITH_EDITORONLY_DATA
	TileTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	TileTexture->CompressionNoAlpha = false;
#endif
	TileTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	TileTexture->SRGB = 0;
	TileTexture->AddToRoot();
	TileTexture->Filter = TextureFilter::TF_Nearest;
#if WITH_EDITORONLY_DATA
	TileTexture->Source.Init(Width, Height, 1, 1, TSF_BGRA8);
#endif
	TileTexture->UpdateResource();

	echoUpdateTextureRegion = TUniquePtr<FUpdateTextureRegion2D>(new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height));
	GenerateDefaultVars(Width, Height);
	RegenerateTileData();
	ClearTextureCanvas();
}

void UTilePointer::GenerateDefaultVars(const uint16 Width, const uint16 Height)
{
	TileWidth = Width;
	TileHeight = Height;
	TileDimension = TileWidth * TileHeight;
	TileData.Reset();
	TileData.AddUninitialized(TileDimension);
}

uint8 UTilePointer::GetPixelCombinedRGB(uint32 pixel)
{
	//uint8 Output = (TileData[pixel].R / 3) + (TileData[pixel].G / 3) + (TileData[pixel].B / 3);
	return TileData[pixel].R;
}

void UTilePointer::GenerateFromMaterial(UObject *InWorldContextObject, UTextureRenderTarget2D* RTMaterial, UMaterialInterface* Material, uint16 new_width, uint16 new_height)
{
	NameUTile = "GeneratedFromMaterial";
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(InWorldContextObject, RTMaterial, Material);
	GenerateFromRT(RTMaterial, new_width, new_height);
	FromRTtoTexture(new_width, new_height);
}

void UTilePointer::GenerateFromTexture(UTexture2D* Texture, const int InTileWidth, const int InTileHeight)
{
	NameUTile = "GeneratedFromTexture";
	GenerateTextureCanvas(InTileWidth, InTileHeight);
	//TextureSettings
#if (ENGINE_MINOR_VERSION >= 23)
	if (Texture->IsCurrentlyVirtualTextured())
	{
		UObject* UseOuter = GetTransientPackage();
		FObjectDuplicationParameters Parameters(Texture, UseOuter);
		Parameters.ApplyFlags |= RF_Transient;
		Parameters.FlagMask &= ~RF_Transactional;
		UTexture2D* Tex = CastChecked<UTexture2D>(StaticDuplicateObjectEx(Parameters));
		Tex->VirtualTextureStreaming = false;
		Tex->Modify();
#if WITH_EDITOR
		Tex->PostEditChange();
#endif
		Texture = Tex;
	}
#endif
	TextureCompressionSettings OldCompressionSettings = Texture->CompressionSettings;
#if WITH_EDITORONLY_DATA
	TextureMipGenSettings OldMipGenSettings = Texture->MipGenSettings;
#endif
	uint32 OldSRGB = Texture->SRGB;
	//SetNewSettings
	Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	Texture->SRGB = 0;
	//Process
	Texture->UpdateResource();
	RegenerateTileData(Texture);
	UpdateTextureCanvas();
	//SetOldSettings
	Texture->CompressionSettings = OldCompressionSettings;
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = OldMipGenSettings;
#endif
	Texture->SRGB = OldSRGB;
#if WITH_EDITOR
	Texture->UpdateResource();
#endif
}

void UTilePointer::GenerateAndCombineTexturesOnChannels(uint16 new_width, uint16 new_height, UTilePointer* TileRed, UTilePointer* TileGreen, UTilePointer* TileBlue, UTilePointer* TileAlpha)
{
	GenerateTextureCanvas(new_width, new_height);
	SetChannels(TileRed, EChannelOutput::Channel_Red);
	SetChannels(TileGreen, EChannelOutput::Channel_Green);
	SetChannels(TileBlue, EChannelOutput::Channel_Blue);
	SetChannels(TileAlpha, EChannelOutput::Channel_Alpha);
	if (TileRed)
		TileRed->RegenerateTileData();
	if (TileGreen)
		TileGreen->RegenerateTileData();
	if (TileBlue)
		TileBlue->RegenerateTileData();
	if (TileAlpha)
		TileAlpha->RegenerateTileData();

	FColor* MipData = static_cast<FColor*>(TileTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	for (uint32 D = 0; D < TileDimension; ++D)
	{
		FColor& CurColor = MipData[D];
		uint8 TileRedPixelPtr = TileRed == nullptr ? 255 : TileRed->GetPixelCombinedRGB(D);
		uint8 TileGreenPixelPtr = TileGreen == nullptr ? 255 : TileGreen->GetPixelCombinedRGB(D);
		uint8 TileBluePixelPtr = TileBlue == nullptr ? 255 : TileBlue->GetPixelCombinedRGB(D);
		uint8 TileAlphaPixelPtr = TileAlpha == nullptr ? 255 : TileAlpha->GetPixelCombinedRGB(D);
		TileData[D] = CurColor = FColor(TileRedPixelPtr, TileGreenPixelPtr, TileBluePixelPtr, TileAlphaPixelPtr);
	}
	TileTexture->PlatformData->Mips[0].BulkData.Unlock();
	TileTexture->UpdateResource();
}

void UTilePointer::GenerateAndSetArrayTilesOnRenderTarget(UObject* InWorldContextObject, UTextureRenderTarget2D* OutputTexture, TArray<UTilePointer*> TileArray, uint16 Width, uint16 Height)
{
	TileRT = OutputTexture;
	TileBinPack = TileArray;
	GenerateTextureCanvas(Width, Height);
	UKismetRenderingLibrary::ClearRenderTarget2D(InWorldContextObject, OutputTexture);
	UCanvas* Canvas;
	FDrawToRenderTargetContext Context = FDrawToRenderTargetContext();
	FVector2D Size = FVector2D(Width, Height);
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(InWorldContextObject, OutputTexture, Canvas, Size, Context);
	for (uint16 number = 0; number < TileArray.Num(); ++number)
	{
		FRectSizePadding Transitional = TileArray[number]->TileDatabase.SizeAndPadding;
		FTexture* RenderTextureResource = TileArray[number] != nullptr ? TileArray[number]->GetResource() : GWhiteTexture;
		FCanvasTileItem TileItem(FVector2D(Transitional.x, Transitional.y), RenderTextureResource, FVector2D(Transitional.width, Transitional.height), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f) + FVector2D::UnitVector, FLinearColor::White);
		TileItem.Rotation = FRotator(0, 0.f, 0);
		TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(InWorldContextObject, Context);
	GenerateFromRT(OutputTexture, Width, Height);
#if WITH_EDITORONLY_DATA
	TileTexture->Source.Init(TileWidth, TileHeight, 1, 1, TSF_BGRA8, TileTexture->Source.LockMip(0));
	TileTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	TileTexture->CompressionNoAlpha = false;
#endif
	TileTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	TileTexture->SRGB = 0;
	TileTexture->Filter = TextureFilter::TF_Nearest;
	TileTexture->UpdateResource();
	UpdateTextureCanvas();
}

void UTilePointer::GenerateAndCombineTexturesOnLayers(uint16 new_width, uint16 new_height, UTilePointer* TileRed_A, UTilePointer* TileRed_B, UTilePointer* TileRed_C, UTilePointer* TileGreen_A, UTilePointer* TileGreen_B, UTilePointer* TileGreen_C, UTilePointer* TileBlue_A, UTilePointer* TileBlue_B, UTilePointer* TileBlue_C, UTilePointer* TileAlpha_A, UTilePointer* TileAlpha_B, UTilePointer* TileAlpha_C)
{
	GenerateTextureCanvas(new_width, new_height);
	SetChannels(TileRed_A, EChannelOutput::Channel_Red_1);
	SetChannels(TileRed_B, EChannelOutput::Channel_Red_2);
	SetChannels(TileRed_C, EChannelOutput::Channel_Red_3);
	SetChannels(TileGreen_A, EChannelOutput::Channel_Green_1);
	SetChannels(TileGreen_B, EChannelOutput::Channel_Green_2);
	SetChannels(TileGreen_C, EChannelOutput::Channel_Green_3);
	SetChannels(TileBlue_A, EChannelOutput::Channel_Blue_1);
	SetChannels(TileBlue_B, EChannelOutput::Channel_Blue_2);
	SetChannels(TileBlue_C, EChannelOutput::Channel_Blue_3);
	SetChannels(TileAlpha_A, EChannelOutput::Channel_Alpha_1);
	SetChannels(TileAlpha_B, EChannelOutput::Channel_Alpha_2);
	SetChannels(TileAlpha_C, EChannelOutput::Channel_Alpha_3);
	for (uint32 i = 0; i < TileDimension; ++i)
	{
		uint8 Red_A = TileRed_A == nullptr ? GetPixelCombinedRGB(i) : TileRed_A->GetPixelCombinedRGB(i);
		uint8 Red_B = TileRed_B == nullptr ? GetPixelCombinedRGB(i) : TileRed_B->GetPixelCombinedRGB(i);
		uint8 Red_C = TileRed_C == nullptr ? GetPixelCombinedRGB(i) : TileRed_C->GetPixelCombinedRGB(i);
		uint8 Green_A = TileGreen_A == nullptr ? GetPixelCombinedRGB(i) : TileGreen_A->GetPixelCombinedRGB(i);
		uint8 Green_B = TileGreen_B == nullptr ? GetPixelCombinedRGB(i) : TileGreen_B->GetPixelCombinedRGB(i);
		uint8 Green_C = TileGreen_C == nullptr ? GetPixelCombinedRGB(i) : TileGreen_C->GetPixelCombinedRGB(i);
		uint8 Blue_A = TileBlue_A == nullptr ? GetPixelCombinedRGB(i) : TileBlue_A->GetPixelCombinedRGB(i);
		uint8 Blue_B = TileBlue_B == nullptr ? GetPixelCombinedRGB(i) : TileBlue_B->GetPixelCombinedRGB(i);
		uint8 Blue_C = TileBlue_C == nullptr ? GetPixelCombinedRGB(i) : TileBlue_C->GetPixelCombinedRGB(i);
		uint8 Alpha_A = TileAlpha_A == nullptr ? GetPixelCombinedRGB(i) : TileAlpha_A->GetPixelCombinedRGB(i);
		uint8 Alpha_B = TileAlpha_B == nullptr ? GetPixelCombinedRGB(i) : TileAlpha_B->GetPixelCombinedRGB(i);
		uint8 Alpha_C = TileAlpha_C == nullptr ? GetPixelCombinedRGB(i) : TileAlpha_C->GetPixelCombinedRGB(i);
		uint8 RedPixel = GenerateChannelFromLayer(Red_A, Red_B, Red_C);
		uint8 GreenPixel = GenerateChannelFromLayer(Green_A, Green_B, Green_C);
		uint8 BluePixel = GenerateChannelFromLayer(Blue_A, Blue_B, Blue_C);
		uint8 AlphaPixel = GenerateChannelFromLayer(Alpha_A, Alpha_B, Alpha_C);
		setPixelColor(TileData[i], RedPixel, GreenPixel, BluePixel, AlphaPixel);
	}
	UpdateTextureCanvas();
}

uint8 UTilePointer::GenerateChannelFromLayer(uint8 Tile_A, uint8 Tile_B, uint8 Tile_C)
{
	uint8 pixel = 0;
	bool Layer_A = Tile_A > 190 ? true : false;
	bool Layer_B = Tile_B > 190 ? true : false;
	bool Layer_C = Tile_C > 190 ? true : false;
	pixel += Layer_A ? 32 : 0;
	pixel += Layer_B ? 80 : 0;
	pixel += Layer_C ? 143 : 0;
	return pixel;
}

void UTilePointer::GenerateThumbnailFromTile(UTilePointer* Tile)
{
	GenerateTextureCanvas(64, 64);/// Tile->TileWidth, Tile->TileHeight);
	ChangeResolution(64, 64, Tile);
}

TArray<FColor> UTilePointer::GetColorArray()
{
	TArray<FColor> OutColorData;
	OutColorData.AddUninitialized(TileDimension);
	const FColor* FormatedImageData = static_cast<const FColor*>(TileTexture->PlatformData->Mips[0].BulkData.LockReadOnly());
	FMemory::Memcpy(OutColorData.GetData(), FormatedImageData, TileDimension * sizeof(FColor));
	TileTexture->PlatformData->Mips[0].BulkData.Unlock();
	return OutColorData;
}

UTexture2D* UTilePointer::FillTextureOutput(UObject* Outer, const FString Name, EObjectFlags Flags)
{
	FCreateTexture2DParameters FCT;
	FCT.bUseAlpha = true;
	FCT.bSRGB = false;
	FCT.CompressionSettings = TextureCompressionSettings::TC_BC7;
	UTexture2D* Output = FImageUtils::CreateTexture2D(TileWidth, TileHeight, TileData, Outer, Name, Flags, FCT);
	//Output->CompressionSettings = TextureCompressionSettings::TC_BC7;//TC_VectorDisplacementmap
#if WITH_EDITORONLY_DATA
	//Output->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	//Output->SRGB = 0;
//	Output->UpdateResource();
	return Output;
}

void UTilePointer::ClearTextureCanvas()
{
	TileData.Reset();
	TileData.AddDefaulted(TileDimension);
	UpdateTextureCanvas();
}

void UTilePointer::UpdateTextureCanvas()
{
	FColor* MipData = static_cast<FColor*>(TileTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	FMemory::Memcpy(MipData, TileData.GetData(), TileDimension * sizeof(FColor));
	TileTexture->PlatformData->Mips[0].BulkData.Unlock();
	TileTexture->UpdateResource();
}

void UTilePointer::SDFGenParallel(uint16 Radius, uint16 V_size, uint16 H_size)
{
	//variables to set: MaxRadius VSize HSize
	InitializeSDFFloatArray();
	ClearTextureCanvas();

	ParallelFor(H_size, [&TileData = TileData, &TileDataFloat = TileDataFloat, Radius = Radius, V_size = V_size, H_size = H_size](int32 px)
	{

		const uint8 MaxRadius = Radius;
		const float FMaxRadius = MaxRadius * 1.0f;
		float mindist = FMaxRadius;
		uint16 startX;
		uint16 endX;
		uint16 startY;
		uint16 endY;
		float startSample;
		float FcurSample;
		float curdist = 0.0f;
		float output = 0.0f;
		for (int32 py = 0; py < V_size; ++py)
		{
			mindist = FMaxRadius;
			startX = (px - MaxRadius) < 0 ? px : MaxRadius;
			endX = (px + MaxRadius) < H_size ? MaxRadius : (H_size - px);
			startY = (py - MaxRadius) < 0 ? py : MaxRadius;
			endY = (py + MaxRadius) < V_size ? MaxRadius : (V_size - py);
			startSample = TileDataFloat[px + (py * H_size)];
			for (int32 sdfX = -startX; sdfX < endX; ++sdfX)
			{
				const int32 PixelXFloat = sdfX + px;
				for (int32 sdfY = -startY; sdfY < endY; ++sdfY)
				{
					FcurSample = TileDataFloat[PixelXFloat + ((py + sdfY) * H_size)];
					if (FcurSample != startSample)
					{
						curdist = sqrtf( FVector2D::DotProduct( FVector2D(sdfX, sdfY), FVector2D(sdfX, sdfY) ) );
						mindist = FMath::Min(mindist, curdist);
						if (mindist == 0.0f)
						{
							sdfY = endY;
							sdfX = endX;
						}
					}
				}
			}
			output = (mindist - 0.5f) / (MaxRadius - 0.5f);
			//output *= startSample == 0.0f ? -1.0f : 1.0f;
			output *= startSample < 0.1f ? -1.0f : 1.0f;
			output = (output + 1.0f) * 0.5f;
			FMath::Clamp(output, 0.0f, 1.0f);
			const uint32 number = (px + py * H_size);
			TileData[number].R = TileData[number].G = TileData[number].B = TileData[number].A = uint8(output * 255.0f);
		}
	});

	UpdateTextureCanvas();
}

void UTilePointer::ChangeResolution(uint16 new_width, uint16 new_height, UTilePointer* InTile)
{
	NameUTile = "ChangeResolution";
	TileBinPack.Add(InTile);//add a reference from the old TilePointer to Debug
	GenerateDefaultVars(new_width, new_height);
	if ( ((new_width - InTile->TileWidth) == 0) && ((new_width == InTile->TileHeight) == 0) )//fix for the blur on texture output
	{
		TileData = InTile->GetColorArray();
	}
	else
	{
		TArray<FColor> OldTextureData = InTile->GetColorArray();
		FImageUtils::ImageResize(InTile->TileWidth, InTile->TileHeight, OldTextureData, new_width, new_height, TileData, true);
	}
	TileTexture = UTexture2D::CreateTransient(new_width, new_height, PF_B8G8R8A8);
	UpdateTextureCanvas();
}

void UTilePointer::SetColorArray(uint16 width, uint16 height, const TArray<FColor>& colorArray)
{
	GenerateDefaultVars(width, height);
	FCreateTexture2DParameters FCT;
	FCT.bUseAlpha = true;
	TileTexture = FImageUtils::CreateTexture2D(width, height, colorArray, NULL, *TileTexture->GetName(), RF_Transient, FCT);
#if WITH_EDITORONLY_DATA
	TileTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	TileTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	TileTexture->SRGB = 0;
	TileTexture->AddToRoot();
	TileTexture->Filter = TextureFilter::TF_Nearest;
	TileTexture->UpdateResource();

	echoUpdateTextureRegion = TUniquePtr<FUpdateTextureRegion2D>(new FUpdateTextureRegion2D(0, 0, 0, 0, TileWidth, TileHeight));

	TileData = colorArray;
	UpdateTextureCanvas();
}

void UTilePointer::FromRTtoTexture(uint16 new_width, uint16 new_height)
{
	ChangeResolution(new_width, new_height, this);
}

void UTilePointer::FromChannelToTexture(EChannelSelectionInput Channel)
{
	if (Channel != EChannelSelectionInput::CSI_RGBA)
	{
		for (uint32 i = 0; i < TileDimension; ++i)
		{
			uint8 color = 0;
			switch (Channel)
			{
			case EChannelSelectionInput::CSI_RGB:
				TileData[i].A = 0;
				break;
			case EChannelSelectionInput::CSI_RGBA:
			default:
				break;
			case EChannelSelectionInput::CSI_Blue:
				color = TileData[i].B;
				break;
			case EChannelSelectionInput::CSI_Green:
				color = TileData[i].G;
				break;
			case EChannelSelectionInput::CSI_Red:
				color = TileData[i].R;
				break;
			case EChannelSelectionInput::CSI_Alpha:
				color = TileData[i].A;
				break;
			}
			//Fill the FColor with one Channel
			if (color > 0)
			{
				TileData[i] = FColor(color, color, color, color);
			}
		}
		UpdateTextureCanvas();
	}
}

void UTilePointer::InvertAllChannels()
{
	for (uint32 i = 0; i < TileDimension; ++i)
	{
		TileData[i] = FColor(255 - TileData[i].R, 255 - TileData[i].G, 255 - TileData[i].B, 255 - TileData[i].A);
	}
	UpdateTextureCanvas();
}

TArray<UTilePointer*> UTilePointer::SplitTile(uint16 VerticalSlices, uint16 HorizontalSlices)
{
	TArray<UTilePointer*> Tiles;
	uint16 newWidth = TileWidth / HorizontalSlices;
	uint16 newHeight = TileHeight / VerticalSlices;
	for (uint16 HorizontalNum = 0; HorizontalNum < HorizontalSlices; ++HorizontalNum)
	{
		for (uint16 VerticalNum = 0; VerticalNum < VerticalSlices; ++VerticalNum)
		{
			UTilePointer* NewTile = NewObject<UTilePointer>(UTilePointer::StaticClass());
			NewTile->GenerateTextureCanvas(newWidth, newHeight);
			const uint16 C_Width = HorizontalNum * newWidth;
			const uint16 R_Height = (VerticalNum * newHeight) * TileHeight;
			for (uint16 W = 0; W < newWidth; ++W)
			{
				for (uint16 H = 0; H < newHeight; ++H)
				{
					const uint32 TilePixelPtr = (W + C_Width + (H * TileWidth) + R_Height);
					NewTile->TileData[W + (H * newHeight)] = TileData[TilePixelPtr];
				}
			}
			NewTile->UpdateTextureCanvas();
			Tiles.Add(NewTile);
		}
	}
	return Tiles;
}

UTilePointer* UTilePointer::GetSplitTile(FRectSizePadding SizePadding)
{
	UTilePointer* Tile = NewObject<UTilePointer>(UTilePointer::StaticClass());
	Tile->GenerateTextureCanvas(SizePadding.width, SizePadding.height);
	for (uint16 H = 0; H < SizePadding.height; ++H)
	{
		for (uint16 W = 0; W < SizePadding.width; ++W)
		{
			const uint32 TilePixelPtr = (W + SizePadding.x + ( (H + SizePadding.y) * TileWidth) );
			Tile->TileData[W + (H * SizePadding.height)] = TileData[TilePixelPtr];
		}
	}
	Tile->UpdateTextureCanvas();
	return Tile;
}

TArray<UTilePointer*> UTilePointer::TileAtlas(TArray<UTilePointer*> InTileBinPack, bool Alpha)
{
	for (UTilePointer* Tile : InTileBinPack)
	{
		Tile->SetChannelsBinPack(Tile->TileBinPack, Alpha ? EChannelOutput::Channel_RGBA : EChannelOutput::Channel_RGB);
	}
	return InTileBinPack;
}

TArray<UTilePointer*> UTilePointer::TileChannels(TArray<UTilePointer*> InTileBinPack, uint16 InMasks, bool Alpha)
{
	TArray<UTilePointer*> TileTextures;
	uint16 FinalTextures = InTileBinPack.Num() / InMasks;
	FinalTextures += (FinalTextures * InMasks) == InTileBinPack.Num() ? 0 : 1;
	for (uint16 TextureProcessActual = 0; TextureProcessActual < FinalTextures; ++TextureProcessActual)
	{
		TArray<UTilePointer*> TileLayers;
		for (uint16 MaskNumber = 0; MaskNumber < InMasks; ++MaskNumber)
		{
			uint16 actualBase = TextureProcessActual * InMasks;
			TileLayers.Add(actualBase + MaskNumber < InTileBinPack.Num() ? InTileBinPack[actualBase + MaskNumber] : nullptr);
		}
		if (InMasks <= 4)
		{
			TileTextures.Add(TileRGBA(TileLayers, Alpha));
		}
		else
		{
			TileTextures.Add(TileMultipleRGBA(TileLayers, Alpha));
		}
	}
	return TileTextures;
}

TArray<UTilePointer*> UTilePointer::DoFinalTextures(TArray<UTilePointer*> InTileBinPack, uint16 InMasks, bool Alpha)
{
	if (InMasks == 1)
	{
		return TileAtlas(InTileBinPack, Alpha);
	}
	else
	{
		return TileChannels(InTileBinPack, InMasks, Alpha);
	}
}

UTilePointer* UTilePointer::TileRGBA(TArray<UTilePointer*> InTileBinPack, bool Alpha)
{
	UTilePointer* TileRGB = NewObject<UTilePointer>();
	if (Alpha)
	{
		TileRGB->GenerateAndCombineTexturesOnChannels(InTileBinPack[0]->TileWidth, InTileBinPack[0]->TileHeight, InTileBinPack[0], InTileBinPack[1], InTileBinPack[2], InTileBinPack[3]);
	}
	else
	{
		TileRGB->GenerateAndCombineTexturesOnChannels(InTileBinPack[0]->TileWidth, InTileBinPack[0]->TileHeight, InTileBinPack[0], InTileBinPack[1], InTileBinPack[2], nullptr);
	}
	return TileRGB;
}

UTilePointer* UTilePointer::TileMultipleRGBA(TArray<UTilePointer*> InTileBinPack, bool Alpha)
{
	UTilePointer* TileRGB = NewObject<UTilePointer>();
	if (Alpha)
	{
		TileRGB->GenerateAndCombineTexturesOnLayers(InTileBinPack[0]->TileWidth, InTileBinPack[0]->TileHeight, InTileBinPack[0], InTileBinPack[1], InTileBinPack[2], InTileBinPack[3], InTileBinPack[4], InTileBinPack[5], InTileBinPack[6], InTileBinPack[7], InTileBinPack[8], InTileBinPack[9], InTileBinPack[10], InTileBinPack[11]);
	}
	else
	{
		TileRGB->GenerateAndCombineTexturesOnLayers(InTileBinPack[0]->TileWidth, InTileBinPack[0]->TileHeight, InTileBinPack[0], InTileBinPack[1], InTileBinPack[2], InTileBinPack[3], InTileBinPack[4], InTileBinPack[5], InTileBinPack[6], InTileBinPack[7], InTileBinPack[8], nullptr, nullptr, nullptr);
	}
	return TileRGB;
}

void UTilePointer::SetChannelsBinPack(TArray<UTilePointer*> InTileBinPack, EChannelOutput ChannelTexture)
{
	for (UTilePointer* Tile : InTileBinPack)
	{
		Tile->TileDatabase.ChannelTexture = ChannelTexture;
	}
}

void UTilePointer::SetChannels(UTilePointer* InTileChannel, EChannelOutput ChannelTexture)
{
	if (InTileChannel != nullptr)
	{
		SetChannelsBinPack(InTileChannel->TileBinPack, ChannelTexture);
	}
}

void UTilePointer::RegenerateTileData(UTexture2D* Texture)
{
	if (Texture == nullptr)
		Texture = TileTexture;
	const FColor* FormatedImageData = static_cast<const FColor*>(Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
	FMemory::Memcpy(TileData.GetData(), FormatedImageData, TileDimension * sizeof(FColor));
	Texture->PlatformData->Mips[0].BulkData.Unlock();
}

FTexture* UTilePointer::GetResource()
{
	if (TileTexture != nullptr)
	{
		return TileTexture->Resource;
	}
	else if (TileRT != nullptr)
	{
		return TileRT->GameThread_GetRenderTargetResource();
	}
	else
	{
		return nullptr;
	}
}

TArray<UTilePointer*> UTilePointer::SortArrayTiles(TArray<UTilePointer*> InTiles)
{
	TArray<UTilePointer*> Output;
	Output.Reserve(InTiles.Num());
	for (UTilePointer* Tile : InTiles)
	{
		if (Output.Num() == 0)
		{
			Output.Add(Tile);
		}
		else
		{
			for (uint16 Num = 0; Num < Output.Num(); ++Num)
			{
				if (Tile->TileDimension > Output[Num]->TileDimension)
				{
					Output.Insert(Tile, Num);
					break;
				}
				else if (Tile->TileDimension == Output[0]->TileDimension)// this case its when material instances do a array of prints the order isnt affected
				{
					Output.Add(Tile);
					break;
				}
				else if (Num + 1 == Output.Num())
				{
					Output.Add(Tile);
					break;
				}
			}
		}
	}
	return Output;
}

#undef LOCTEXT_NAMESPACE
