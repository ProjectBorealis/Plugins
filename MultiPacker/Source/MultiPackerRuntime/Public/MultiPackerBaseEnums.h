/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StreamableManager.h"
#include "MultiPackerBaseEnums.generated.h"

class UTexture;

UENUM(BlueprintType)
enum class EMultiPackerImageLayer : uint8
{
	EMCE_Option1        UMETA(DisplayName = "1_Layer"),
	EMCE_Option2        UMETA(DisplayName = "2_Layers"),
	EMCE_Option3        UMETA(DisplayName = "3_Layers")
};

UENUM(BlueprintType)
enum class ETextureSizeOutputPersonal : uint8
{
	EMCE_Option1 = 0    UMETA(DisplayName = "32"),//The enums has a value -1 on the normal state to can manage the last enum good
	EMCE_Option2 = 1    UMETA(DisplayName = "64"),
	EMCE_Option3 = 3    UMETA(DisplayName = "128"),
	EMCE_Option4 = 7    UMETA(DisplayName = "256"),
	EMCE_Option5 = 15   UMETA(DisplayName = "512"),
	EMCE_Option6 = 31   UMETA(DisplayName = "1024"),
	EMCE_Option7 = 63   UMETA(DisplayName = "2048"),
	EMCE_Option8 = 127  UMETA(DisplayName = "4096"),
	EMCE_Option9 = 255  UMETA(DisplayName = "8192")
};

UENUM(BlueprintType)
enum class EApplyMaterial : uint8
{
	EMCE_Option1        UMETA(DisplayName = "Material Updated"),
	EMCE_Option2        UMETA(DisplayName = "Dirty"),
	EMCE_Option3        UMETA(DisplayName = "Apply Changes")
};

UENUM(BlueprintType)
enum class EChannelSelectionInput : uint8
{
	CSI_Blue = 0      UMETA(DisplayName = "Blue"),
	CSI_Green = 1       UMETA(DisplayName = "Green"),
	CSI_Red = 2      UMETA(DisplayName = "Red"),
	CSI_Alpha = 3      UMETA(DisplayName = "Alpha"),
	CSI_RGB = 4      UMETA(DisplayName = "RGB"),
	CSI_RGBA = 5      UMETA(DisplayName = "RGBA")
};

UENUM()
enum class ESizeTexture : uint8
{
	EMCE_Option1        UMETA(DisplayName = "SelectToGetSize"),
	EMCE_Option2        UMETA(DisplayName = "GetTextureSize")
};

UENUM()
enum class ENameTexture : uint8
{
	EMCE_Option1        UMETA(DisplayName = "SelectToGetName"),
	EMCE_Option2        UMETA(DisplayName = "GetNameFromTexture"),
	EMCE_Option3        UMETA(DisplayName = "GetNameFromTextureWithAlphaSufix")
};

UENUM()
enum class ENameMaterial : uint8
{
	EMCE_Option1        UMETA(DisplayName = "SelectToGetName"),
	EMCE_Option2        UMETA(DisplayName = "GetNameFromMaterial"),
	EMCE_Option3        UMETA(DisplayName = "GetNameFromMaterialWithAlphaSufix")
};

UENUM(BlueprintType)
enum class ETextureSizeOutput : uint8
{
	EMCE_Option1 = 1    UMETA(DisplayName = "32"),
	EMCE_Option2 = 2    UMETA(DisplayName = "64"),
	EMCE_Option3 = 4   UMETA(DisplayName = "128"),
	EMCE_Option4 = 8   UMETA(DisplayName = "256"),
	EMCE_Option5 = 16   UMETA(DisplayName = "512"),
	EMCE_Option6 = 32  UMETA(DisplayName = "1024")
};

UENUM(BlueprintType)
enum class EStateButton : uint8
{
	EMCE_Option1        UMETA(DisplayName = "Normal"),
	EMCE_Option2        UMETA(DisplayName = "Press"),
	EMCE_Option3        UMETA(DisplayName = "Indicator"),
	EMCE_Option4        UMETA(DisplayName = "Single")
};

UENUM(BlueprintType)
enum class EChannelTextureSave : uint8
{
	CS_Multiple        UMETA(DisplayName = "Multiple"),
	CS_One        UMETA(DisplayName = "One"),
	CS_Atlas        UMETA(DisplayName = "Atlas"),
	CS_OneSdf        UMETA(DisplayName = "One_SDF")
};

UENUM(BlueprintType)
enum class EChannelOutput : uint8
{
	Channel_RGB        UMETA(DisplayName = "RGB"),
	Channel_Red        UMETA(DisplayName = "Red"),
	Channel_Green        UMETA(DisplayName = "Green"),
	Channel_Blue        UMETA(DisplayName = "Blue"),
	Channel_Alpha        UMETA(DisplayName = "Alpha"),
	Channel_RGBA        UMETA(DisplayName = "RGBA"),
	Channel_Red_1        UMETA(DisplayName = "Red_1"),
	Channel_Red_2        UMETA(DisplayName = "Red_2"),
	Channel_Red_3        UMETA(DisplayName = "Red_3"),
	Channel_Green_1       UMETA(DisplayName = "Green_1"),
	Channel_Green_2       UMETA(DisplayName = "Green_2"),
	Channel_Green_3       UMETA(DisplayName = "Green_3"),
	Channel_Blue_1       UMETA(DisplayName = "Blue_1"),
	Channel_Blue_2       UMETA(DisplayName = "Blue_2"),
	Channel_Blue_3       UMETA(DisplayName = "Blue_3"),
	Channel_Alpha_1       UMETA(DisplayName = "Alpha_1"),
	Channel_Alpha_2       UMETA(DisplayName = "Alpha_2"),
	Channel_Alpha_3       UMETA(DisplayName = "Alpha_3")
};

/// Specifies the different heuristic rules that can be used when deciding where to place a new rectangle.
UENUM(BlueprintType)
enum class EFreeRectChoiceHeuristic : uint8
{
	EMCE_Option1        UMETA(DisplayName = "RectBestShortSideFit"),///< BestShortSideFit: Positions the rectangle against the short side of a free rectangle into which it fits the best.
	EMCE_Option2        UMETA(DisplayName = "RectBestLongSideFit"),///< BestLongSideFit: Positions the rectangle against the long side of a free rectangle into which it fits the best.
	EMCE_Option3        UMETA(DisplayName = "RectBestAreaFit"),///< BestAreaFit: Positions the rectangle into the smallest free rect into which it fits.
	EMCE_Option4        UMETA(DisplayName = "RectBottomLeftRule"),///< BottomLeftRule: Does the Tetris placement.
	EMCE_Option5        UMETA(DisplayName = "RectContactPointRule")///< ContactPointRule: Choosest the placement where the rectangle touches other rects as much as possible.
};

USTRUCT(BlueprintType)
struct FRectSize
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Size")
		uint16 width = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Size")
		uint16 height = 0;
};

USTRUCT(BlueprintType)
struct FRectSizePadding
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Padding")
		uint16 x = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Padding")
		uint16 y = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Size")
		uint16 width = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Size")
		uint16 height = 0;

	void Initialize(uint16 InX, uint16 InY, uint16 InWidth, uint16 InHeight)
	{
		x = InX;
		y = InY;
		width = InWidth;
		height = InHeight;
	}
};

USTRUCT(BlueprintType)
struct FTileThumbDatabase
{
	GENERATED_BODY()
public:
	//This is a preview Image to recognize the Tile
	UPROPERTY(VisibleAnywhere, Category = "Default")
		UTexture* TileTexture;
	//Here you can set the Name for every Tile The number of characters will be chopped to 16 to get a stable dropdown UI 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
		FName TileName;
};

USTRUCT(BlueprintType)
struct FTileDatabase
{
	GENERATED_BODY()

public:
	//Preview of the Tile Texture 
	UPROPERTY(VisibleAnywhere, Transient, Category = "Default")
		UTexture* TileTexture;
	//This Name will be AutoFilled with the name settled on the Node,
	//every Tile needs a Different Name for Avoiding Duplications on Name to select later
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FName TileName;
	//Number of the Tile on the Atlas
	UPROPERTY(BlueprintReadOnly, Category = "Default")
		int32 Frame;
	//Information about the channel where the Tile is
	UPROPERTY(BlueprintReadOnly, Category = "Default")
		EChannelOutput ChannelTexture;
	//Information about the TextureSize and the Padding on the TextureOutput: This information its only for the Rectangle Method
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FRectSizePadding SizeAndPadding;
	UPROPERTY(BlueprintReadOnly, Category = "Default")
		bool SDFNode;
	UPROPERTY(BlueprintReadOnly, Category = "Default")
		bool MSDFNode;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FLinearColor Channel_vec;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FLinearColor Layer_vec;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FLinearColor SizePadding_vec;

public:
	void SetFromPointer(FTileDatabase* Tile)
	{
		TileTexture = Tile->TileTexture;
		TileName = Tile->TileName;
		Frame = Tile->Frame;
		ChannelTexture = Tile->ChannelTexture;
		SizeAndPadding = Tile->SizeAndPadding;
		SDFNode = Tile->SDFNode;
		Channel_vec = Tile->Channel_vec;
		Layer_vec = Tile->Layer_vec;
	}
	void SetFromObject(FTileDatabase Tile)
	{
		TileTexture = Tile.TileTexture;
		TileName = Tile.TileName;
		Frame = Tile.Frame;
		ChannelTexture = Tile.ChannelTexture;
		SizeAndPadding = Tile.SizeAndPadding;
		SDFNode = Tile.SDFNode;
		Channel_vec = Tile.Channel_vec;
		Layer_vec = Tile.Layer_vec;
	}
	void SetFromThumbDatabase(FTileThumbDatabase Tile, bool SDF, bool MSDF)
	{
		TileTexture = Tile.TileTexture;
		TileName = Tile.TileName;
		SDFNode = SDF;
		MSDFNode = MSDF;
	}
};

USTRUCT(BlueprintType)
struct FVectorMaterial
{
	GENERATED_BODY()
public:
	//Name of the Vector Parameter (AutoFilled)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FName ParameterName;

	//Vector
	//The Vector Parameter will be updated with this Value
	//When ValueChanges is True this is the MinValue
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		FLinearColor DefaultVectorValue;

	//Needs NumPrints be bigger than 1 to work
	//Set true if need to change the values on Print
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		bool ValueChangesOnTime = false;

	//Vector
	//From the DefaultVectorValue every Print the value will increase linearly 
	//When ValueChanges is True this is the MaxValue
	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (EditCondition = ValueChangesOnTime))
		FLinearColor MaxVectorValue;
};

USTRUCT(BlueprintType)
struct FTextureMaterial
{
	GENERATED_BODY()
public:
	//Name of the Texture Input node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FName TileName;

	//Texture To Set on the Material
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		UTexture* TileTexture;
};

USTRUCT(BlueprintType)
struct FTextureScalar
{
	GENERATED_BODY()
public:

	//Name of the Scalar Parameter (AutoFilled)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FName ParameterName;

	//Float
	//The Scalar Parameter will be updated with this Value
	//When ValueChanges is True this is the MinValue
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		float DefaultValue;

	//Needs NumPrints be bigger than 1 to work
	//Set true if need to change the values on Print
	UPROPERTY(EditDefaultsOnly, Category = "Default")
		bool ValueChangesOnTime = false;

	//Float
	//From the DefaultValue every Print the value will increase linearly 
	//When ValueChanges is True this is the MaxValue
	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (EditCondition = ValueChangesOnTime))
		float MaxValue;
};

UCLASS()
class UMultiPackerBaseEnums : public UObject
{
	GENERATED_BODY()
public:
	static int32 GetTextureSizeOutputTile(ETextureSizeOutput In)
	{
		return (uint16)In * 32;
	};

	static int32 GetTextureSizeOutputEnum(ETextureSizeOutputPersonal In)
	{
		return ((uint16)In + 1) * 32;
	};

	static UTextureRenderTarget2D* GenerateRenderTarget(uint16 Width, uint16 Height, bool Alpha)
	{
		UTextureRenderTarget2D* NewRT = NewObject<UTextureRenderTarget2D>();
		NewRT->bNeedsTwoCopies = false;
		NewRT->InitCustomFormat(Width, Height, PF_B8G8R8A8, true);
		NewRT->Filter = TextureFilter::TF_Nearest;
		NewRT->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		NewRT->SRGB = 0;
		NewRT->TargetGamma = 1.04f;
#if WITH_EDITORONLY_DATA
		NewRT->CompressionNoAlpha = !Alpha;
#endif
		NewRT->UpdateResourceImmediate(true);
		return NewRT;
	};

	static UObject* LoadAssetFromContent(FString Path)
	{
		FStreamableManager assetLoader;
		FStringAssetReference asset;
		asset.SetPath(Path);
		return assetLoader.LoadSynchronous(asset, false, nullptr);
	};
};

