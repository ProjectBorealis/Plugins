/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include <Engine/Texture2D.h>
#include "MultiPackerBaseEnums.h"
#include <Engine/Texture.h>
#include <Engine/TextureDefines.h>
#include "MultiPackerDetailPanel.generated.h"

UENUM(BlueprintType)
enum class EMPChannelMaskParameterColor : uint8
{
	Red,
	Green,
	Blue,
	Alpha,
};

UENUM(BlueprintType)
enum class EMPChannelPackingSizeFromTexture : uint8
{
	UserDefined,
	BiggestSize,
	SmallestSize,
};

UCLASS(NonTransient, autoExpandCategories = ("InputTextures|Red", "InputTextures|Green", "InputTextures|Blue", "InputTextures|Alpha"))
class UMultiPackerDetailPanel : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "InputTextures|Red")
		UTexture2D* TextureRed;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Red")
		EMPChannelMaskParameterColor MaskTextureRed = EMPChannelMaskParameterColor::Red;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Red")
		bool InvertRed;

	UPROPERTY(EditAnywhere, Category = "InputTextures|Green")
		UTexture2D* TextureGreen;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Green")
		EMPChannelMaskParameterColor MaskTextureGreen = EMPChannelMaskParameterColor::Green;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Green")
		bool InvertGreen;

	UPROPERTY(EditAnywhere, Category = "InputTextures|Blue")
		UTexture2D* TextureBlue;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Blue")
		EMPChannelMaskParameterColor MaskTextureBlue = EMPChannelMaskParameterColor::Blue;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Blue")
		bool InvertBlue;

	UPROPERTY(EditAnywhere, Category = "InputTextures|Alpha")
		bool AlphaChannel;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Alpha", meta = (EditCondition = AlphaChannel))
		UTexture2D* TextureAlpha;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Alpha", meta = (EditCondition = AlphaChannel))
		EMPChannelMaskParameterColor MaskTextureAlpha = EMPChannelMaskParameterColor::Alpha;
	UPROPERTY(EditAnywhere, Category = "InputTextures|Alpha")
		bool InvertAlpha;
	
	UPROPERTY(EditAnywhere, Category = "OutputTexture")
		UTexture2D* Texture;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Name&Location", meta = (RelativeToGameContentDir, ContentDir))
		FDirectoryPath TargetDirectory;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Name&Location")
		FString TextureName = "MultiPacker_CP";

	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		EMPChannelPackingSizeFromTexture OutputSizeMethod = EMPChannelPackingSizeFromTexture::UserDefined;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings", meta = (EditCondition = CanEditSizes))
		ETextureSizeOutputPersonal SizeVertical = ETextureSizeOutputPersonal::EMCE_Option5;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings", meta = (EditCondition = CanEditSizes))
		ETextureSizeOutputPersonal SizeHorizontal = ETextureSizeOutputPersonal::EMCE_Option5;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		TEnumAsByte<enum TextureCompressionSettings> CompressionSettings = TextureCompressionSettings::TC_Default;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		TEnumAsByte<enum TextureFilter> TextureFilter = TF_Bilinear;
	//Defines the if the Texture will be saved as a SRGB
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		bool sRGB = false;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		TEnumAsByte<enum TextureAddress> AddressX = TA_Clamp;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		TEnumAsByte<enum TextureAddress> AddressY = TA_Clamp;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		TEnumAsByte<enum ETexturePowerOfTwoSetting::Type> PowerOfTwoMode;
	UPROPERTY(EditAnywhere, Category = "OutputTexture|Settings")
		TEnumAsByte<enum TextureMipGenSettings> MipGenSettings;
	
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "OutputTexture|Settings" )
		bool CanEditSizes = true;

private:

#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
	{
		switch (OutputSizeMethod)
		{
		case EMPChannelPackingSizeFromTexture::UserDefined:
			CanEditSizes = true;
			break;
		case EMPChannelPackingSizeFromTexture::BiggestSize:
			SetSizeByTextures(true);
			CanEditSizes = false;
			break;
		case EMPChannelPackingSizeFromTexture::SmallestSize:
			SetSizeByTextures(false);
			CanEditSizes = false;
			break;
		}
	}
#endif

	void SetSizeByTextures(bool Biggest)
	{
		uint8 SizeTextureHorizontal = 0;
		uint8 SizeTextureVertical = 0;
		if (TextureRed)
		{
			SizeTextureHorizontal = GetSizeAsByte(TextureRed->GetSurfaceWidth(), SizeTextureHorizontal, Biggest);
			SizeTextureVertical = GetSizeAsByte(TextureRed->GetSurfaceHeight(), SizeTextureVertical, Biggest);
		}
		if (TextureGreen)
		{
			SizeTextureHorizontal = GetSizeAsByte(TextureGreen->GetSurfaceWidth(), SizeTextureHorizontal, Biggest);
			SizeTextureVertical = GetSizeAsByte(TextureGreen->GetSurfaceHeight(), SizeTextureVertical, Biggest);
		}
		if (TextureBlue)
		{
			SizeTextureHorizontal = GetSizeAsByte(TextureBlue->GetSurfaceWidth(), SizeTextureHorizontal, Biggest);
			SizeTextureVertical = GetSizeAsByte(TextureBlue->GetSurfaceHeight(), SizeTextureVertical, Biggest);
		}
		if (AlphaChannel = true && TextureAlpha)
		{
			SizeTextureHorizontal = GetSizeAsByte(TextureAlpha->GetSurfaceWidth(), SizeTextureHorizontal, Biggest);
			SizeTextureVertical = GetSizeAsByte(TextureAlpha->GetSurfaceHeight(), SizeTextureVertical, Biggest);
		}
		SizeHorizontal = (ETextureSizeOutputPersonal)SizeTextureHorizontal;
		SizeVertical = (ETextureSizeOutputPersonal)SizeTextureVertical;
	}

	uint16 GetSizeAsByte(float InSize, uint8 OldSizeByte, bool Biggest)
	{
		//easy way to round up the Texture Size to get a properly texture with power of two
		uint8 SizeByte = (FMath::CeilToInt(InSize/32) - 1);
		//by default the byte its zero, this sets a default size via the fist texture processed
		uint8 NewSizeByte = OldSizeByte == 0 ? SizeByte : OldSizeByte;
		//sets the size by the method selected
		SizeByte = Biggest ? FMath::Max(SizeByte, NewSizeByte) : FMath::Min(SizeByte, NewSizeByte);
		return SizeByte;
	}
};
