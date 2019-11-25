/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/MultiPackerMaterialNode.h"
#include "Graph/MultiPackerMatNode.h"
#include "MultiPackerEditorTypes.h"
#include <EdGraph/EdGraphPin.h>
#include "MultiPackerAssetEditor/MultiPackerEdGraph.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TileUtils/TilePointer.h"
#include <Editor.h>
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "MultiPackerMaterialNode"

void UMultiPackerMaterialNode::AllocateDefaultPins()
{
	FEdGraphPinType newPin = FEdGraphPinType();
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	newPin.PinCategory = UMultiPackerEditorTypes::SPinMaterialNode;
	CreatePin(EGPD_Output, newPin, FString("Out"), 0);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::SPinMaterialNode);
	CreatePin(EGPD_Output, newPin, FName("Out"), 0);
#endif
}

void UMultiPackerMaterialNode::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
}

FText UMultiPackerMaterialNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (MultiPackerMatNode == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		MultiPackerMatNode->ChangeBackground(!MultiPackerMatNode->CanProcess());
		return FText::FromString(MultiPackerMatNode->MaterialBaseInput ? MultiPackerMatNode->MaterialBaseInput->GetName() : "Empty");
	}
}

void UMultiPackerMaterialNode::SetGenericGraphNode(UMultiPackerMatNode* InNode)
{
	MultiPackerMatNode = InNode;
}

FString UMultiPackerMaterialNode::GetNodeTitle()
{
	return (MultiPackerMatNode->MaterialBaseInput ? MultiPackerMatNode->MaterialBaseInput->GetName() : "Empty");
}

bool UMultiPackerMaterialNode::SetThumbnail()
{
	return true;
}

void UMultiPackerMaterialNode::SetMaterialInput(UMaterialInterface* Material)
{
	MultiPackerMatNode->MaterialBaseInput = Material;
	ChangeEnumMaterial(true);
}

UObject* UMultiPackerMaterialNode::GetThumbnailAssetObject()
{
	return MultiPackerMatNode->MaterialBaseInput;
}

UObject* UMultiPackerMaterialNode::GetNodeAssetObject(UObject* Outer)
{
	return  MultiPackerMatNode->MaterialBaseInput;
}

FLinearColor UMultiPackerMaterialNode::GetBackgroundColor() const
{
	return MultiPackerMatNode->GetBackground();
}

void UMultiPackerMaterialNode::GenerateTileDataBase()
{
	int GridTile = MultiPackerMatNode->Atlas ? (MultiPackerMatNode->TilesVertical * MultiPackerMatNode->TilesHorizontal) : 1;
	GridTile *= MaterialInstance ? NumPrints : 1;
	if (GridTile != AMatTileData.Num())
	{
		if (GridTile < AMatTileData.Num())//resize
		{
			TArray<FTileThumbDatabase> NewTileData = AMatTileData;
			AMatTileData.Reset();
			for (int grid = 0; grid < (NewTileData.Num() - 1); ++grid)
			{
				AMatTileData.Add(NewTileData[grid]);
			}
		}
		else//GridTile is bigger  //filler
		{
			for (int grid = AMatTileData.Num(); grid < GridTile; ++grid)
			{
				FTileThumbDatabase newTile;
				newTile.TileName = FName("None");
				newTile.TileTexture = nullptr;
				AMatTileData.Add(newTile);
			}
		}
	}
}

void UMultiPackerMaterialNode::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	GenerateTileDataBase();//this is a call to make a check to the DataArray


	// We test using GET_MEMBER_NAME_CHECKED so that if someone changes the property name  
	// in the future this will fail to compile and we can update it.  
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerMaterialNode, MaterialInstance)))
	{
		VectorParameter.Reset();
		TextureParameter.Reset();
		ScalarParameter.Reset();
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerMaterialNode, MaterialButton)))
	{
		switch (MaterialButton)
		{
		case EApplyMaterial::EMCE_Option1://MaterialUpdated
			if (MultiPackerMatNode->DirtyMaterial)
			{
				MaterialButton = EApplyMaterial::EMCE_Option2;
			}
			else
			{
				MaterialButton = EApplyMaterial::EMCE_Option1;
				MultiPackerMatNode->DirtyMaterial = false;
			}
			break;
		case EApplyMaterial::EMCE_Option2://MaterialDirty
			MultiPackerMatNode->DirtyMaterial = true;
			MultiPackerMatNode->ChangeBackground(true);
			break;
		case EApplyMaterial::EMCE_Option3://Apply
			ProcessParameters();
			MultiPackerMatNode->DirtyMaterial = false;
			MaterialButton = EApplyMaterial::EMCE_Option1;
			MultiPackerMatNode->ChangeBackground(false);
			break;
		default:
			break;
		}
	}
	else
	{
		if (MaterialInstance)
		{
			MaterialButton = EApplyMaterial::EMCE_Option2;
			MultiPackerMatNode->DirtyMaterial = true;
			MultiPackerMatNode->ChangeBackground(true);
		}
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerMaterialNode, AutoNameFill)))
	{
		FString Name;
		switch (AutoNameFill)
		{
		case ENameMaterial::EMCE_Option1:
			break;
		case ENameMaterial::EMCE_Option2:
			AutoNameFill = ENameMaterial::EMCE_Option1;
			Name = MultiPackerMatNode->MaterialBaseInput->GetName().Left(16);
			for (int data = 0; data < AMatTileData.Num(); ++data)
			{
				FString numName = Name;//initialization
				if (AMatTileData.Num() > 1)
				{
					numName = MultiPackerMatNode->MaterialBaseInput->GetName().Left(14).Append("_" + FString::FromInt(data));
				}
				AMatTileData[data].TileName = FName(*numName);
			}
			break;
		case ENameMaterial::EMCE_Option3:
			AutoNameFill = ENameMaterial::EMCE_Option1;
			Name = MultiPackerMatNode->MaterialBaseInput->GetName().Left(10).Append("_Alpha");
			for (int data = 0; data < AMatTileData.Num(); ++data)
			{
				FString numName = Name;//initialization
				if (AMatTileData.Num() > 1)
				{
					numName = MultiPackerMatNode->MaterialBaseInput->GetName().Left(8).Append("_" + FString::FromInt(data) + "_Alpha");
				}
				AMatTileData[data].TileName = FName(*numName);
			}
			break;
		default:
			AutoNameFill = ENameMaterial::EMCE_Option1;
			break;
		}
	}
	for (int tile = 0; tile < AMatTileData.Num(); ++tile)
	{
		AMatTileData[tile].TileName = FName(*(AMatTileData[tile].TileName.ToString().Left(16)));
	}
	OnChangeValueValidity();

	MultiPackerMatNode->NumTiles = (MultiPackerMatNode->TilesVertical * MultiPackerMatNode->TilesHorizontal) * NumPrints;
}

void UMultiPackerMaterialNode::ProcessTiles()
{
	TArray<UTilePointer*> ArrayTiles;
	//material instance had numprints
	if (MaterialInstance)
	{
		PreProcessPrints();
		for (uint8 Prints = 0; Prints < GetNumberOfMaterialPrints(); ++Prints)
		{
			ProcessPrintByNumber(Prints);
			UTilePointer* NewTile = NewObject<UTilePointer>(UTilePointer::StaticClass());
			NewTile->GenerateFromMaterial(GEditor->GetEditorWorldContext().World(), UMultiPackerBaseEnums::GenerateRenderTarget(GetTileSize().X, GetTileSize().Y, false), GetMaterialInstanceDynamic(Prints), GetTileSize().X, GetTileSize().Y);
			ArrayTiles.Add(NewTile);
		}
	}
	else
	{
		UTilePointer* NewTile = NewObject<UTilePointer>(UTilePointer::StaticClass());
		NewTile->GenerateFromMaterial(GEditor->GetEditorWorldContext().World(), UMultiPackerBaseEnums::GenerateRenderTarget(GetTileSize().X, GetTileSize().Y, false), MultiPackerMatNode->MaterialBaseInput, GetTileSize().X, GetTileSize().Y);
		ArrayTiles.Add(NewTile);
	}
	//split stage
	if ((MultiPackerMatNode->TilesVertical * MultiPackerMatNode->TilesHorizontal) > 1)
	{
		TArray<UTilePointer*> SplitArrayTiles;
		for (UTilePointer* Tile : ArrayTiles)
		{
			SplitArrayTiles.Append(Tile->SplitTile(MultiPackerMatNode->TilesVertical, MultiPackerMatNode->TilesHorizontal) );
		}
		ArrayTiles = SplitArrayTiles;
	}
	//sdf stage
	if (IsTilesSDF())
	{
		for (UTilePointer* Tile : ArrayTiles)
		{
			//Tile->ChangeResolution(128, 128, Tile);
			Tile->SDFGenParallel(MultiPackerMatNode->SDF_Radius, GetTileSize().Y, GetTileSize().X);
		}
	}
	//Tile Size Final
	for (UTilePointer* Tile : ArrayTiles)
	{
		if (GetSplitTileSize() != FVector2D(Tile->TileWidth, Tile->TileHeight))
		{
			Tile->ChangeResolution(GetSplitTileSize().X, GetSplitTileSize().Y, Tile);
		}
	}
	Tiles = ArrayTiles;
	//Thumbnail generation
	GenerateTileDataBase();//this is a call to make a check to the DataArray
	SetDatabase();//Info database from the tiles to the final process
	//thumbnails = Tiles
	SetTileThumbs(ArrayTiles);//info thumbnail
}

FVector2D UMultiPackerMaterialNode::GetTileSize()
{
	if (MultiPackerMatNode->RectangleSize)
	{
		return FVector2D((uint16)(MultiPackerMatNode->SizeHorizontal ), (uint16)(MultiPackerMatNode->SizeVertical ));
	}
	return FVector2D(SizeTile * MultiPackerMatNode->TilesHorizontal, SizeTile * MultiPackerMatNode->TilesVertical);
}

FVector2D UMultiPackerMaterialNode::GetSplitTileSize()
{
	if (MultiPackerMatNode->RectangleSize)
	{
		return FVector2D((uint16)(MultiPackerMatNode->SizeHorizontal / MultiPackerMatNode->TilesHorizontal), (uint16)(MultiPackerMatNode->SizeVertical / MultiPackerMatNode->TilesVertical));
	}
	return FVector2D(SizeTile , SizeTile);
}

bool UMultiPackerMaterialNode::PinConnected()
{
	for (UEdGraphPin* Pin : Pins)
	{
		return Pin->LinkedTo.Num() > 0;
	}
	return false;
}

bool UMultiPackerMaterialNode::IsObjectValid()
{
	return (MultiPackerMatNode->MaterialBaseInput != nullptr);
}

bool UMultiPackerMaterialNode::IsNodeSelectedSdf()
{
	return MultiPackerMatNode->SDF;
}

bool UMultiPackerMaterialNode::GetMsdf()
{
	return false;
}

FTileThumbDatabase UMultiPackerMaterialNode::GetTileThumbDatabase(uint16 num)
{
	return AMatTileData[num];
}

uint16 UMultiPackerMaterialNode::GetNumberTiles()
{
	return MultiPackerMatNode->TilesHorizontal * MultiPackerMatNode->TilesVertical * NumPrints;
}

void UMultiPackerMaterialNode::ChangeEnumMaterial(bool Material)
{
	MaterialInstance = !Material;
}

void UMultiPackerMaterialNode::ProcessPrintByNumber(uint8 number)
{
	UMaterialInstanceDynamic* MID_new = CreateMaterialDynamic();
	ProcessMaterialInstance(MID_new);
	if (TimeParameter)
	{
		float newTime = Period;
		if (!SpecificTime)
		{
			newTime = Period / GetNumberOfMaterialPrints();
			newTime *= number;
		}
		MID_new->SetScalarParameterValue("Time", newTime);
	}
	for (int v = 0; v < VectorParameter.Num(); ++v)
	{
		if (VectorParameter[v].ValueChangesOnTime)
		{
			MID_new->SetVectorParameterValue(VectorParameter[v].ParameterName, GetActualVector(VectorParameter[v].DefaultVectorValue, VectorParameter[v].MaxVectorValue, number));
		}
	}
	for (int s = 0; s < ScalarParameter.Num(); ++s)
	{
		if (ScalarParameter[s].ValueChangesOnTime)
		{
			MID_new->SetScalarParameterValue(ScalarParameter[s].ParameterName, GetFloatDifference(ScalarParameter[s].DefaultValue, ScalarParameter[s].MaxValue, number));
		}
	}
	MID_Iteration.Add(MID_new);
}

uint8 UMultiPackerMaterialNode::GetNumberOfMaterialPrints()
{
	uint8 prints = 1;
	if (MaterialInstance)
	{
		prints = NumPrints > 1 ? NumPrints : prints;
	}
	return prints;
}

UMaterialInstanceDynamic* UMultiPackerMaterialNode::GetMaterialInstanceDynamic(uint8 number) const
{
	return MID_Iteration[number];
}

void UMultiPackerMaterialNode::PreProcessPrints()
{
	MID_Iteration.Reset(0);
}

void UMultiPackerMaterialNode::SetTileThumbs(TArray<UTilePointer*> TileThumbs)
{
	Thumbnails = TileThumbs;
	for (int tile = 0; tile < AMatTileData.Num(); ++tile)
	{
		AMatTileData[tile].TileTexture = TileThumbs[tile]->TileTexture;
	}
}

FName UMultiPackerMaterialNode::GetTileName(int Tile)
{
	return AMatTileData[Tile].TileName;
}

void UMultiPackerMaterialNode::OnChangeValueValidity()
{
	bool color = MultiPackerMatNode->GetBackground() == FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
	bool InstanceDirty = MaterialInstance ? MultiPackerMatNode->DirtyMaterial : false;
	MultiPackerMatNode->bMatAsset = true;

	bool MatTiles = (MultiPackerMatNode->TilesHorizontal > 0) && (MultiPackerMatNode->TilesVertical > 0);
	if (MultiPackerMatNode->bDebug_MatNode)
	{
		UE_LOG(LogTemp, Log, TEXT("UMultiPackerMaterialNode::OnChangeValueValidity(): color %s"), color ? TEXT("true") : TEXT("false"));   // %s strings  %d int  %f float   %s fvector
		UE_LOG(LogTemp, Log, TEXT("UMultiPackerMaterialNode::OnChangeValueValidity(): InstanceDirty %s"), InstanceDirty ? TEXT("true") : TEXT("false"));   // %s strings  %d int  %f float   %s fvector
		UE_LOG(LogTemp, Log, TEXT("UMultiPackerMaterialNode::OnChangeValueValidity(): bMatAsset %s"), MultiPackerMatNode->bMatAsset ? TEXT("true") : TEXT("false"));   // %s strings  %d int  %f float   %s fvector
		UE_LOG(LogTemp, Log, TEXT("UMultiPackerMaterialNode::OnChangeValueValidity(): MatTiles %s"), MatTiles ? TEXT("true") : TEXT("false"));   // %s strings  %d int  %f float   %s fvector
		UE_LOG(LogTemp, Log, TEXT("UMultiPackerMaterialNode::OnChangeValueValidity(): ErrorSize %s"), MultiPackerMatNode->ErrorSize ? TEXT("true") : TEXT("false"));   // %s strings  %d int  %f float   %s fvector
		UE_LOG(LogTemp, Log, TEXT("UMultiPackerMaterialNode::OnChangeValueValidity(): ErrorTiles %s"), MultiPackerMatNode->ErrorTiles ? TEXT("true") : TEXT("false"));   // %s strings  %d int  %f float   %s fvector
	}
	MultiPackerMatNode->bCanProcess = color && !InstanceDirty && MultiPackerMatNode->bMatAsset && MatTiles && !MultiPackerMatNode->ErrorSize && !MultiPackerMatNode->ErrorTiles;
	MultiPackerMatNode->ChangeBackground(MultiPackerMatNode->bCanProcess);
}

//Called by PostEditChangeProperty
//GetMaterial and set vector scalar and texture arrays parameters
void UMultiPackerMaterialNode::ProcessParameters()
{
	GenerateTileDataBase();
	UMaterial* BaseMaterial = CreateMaterialDynamic()->GetBaseMaterial();
	TArray<FGuid> GUIDs;
	//Scalar
	TArray<FName> ScalarParametersName;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	BaseMaterial->GetAllScalarParameterNames(ScalarParametersName, GUIDs);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	TArray<FMaterialParameterInfo> ScalarMaterialParameterInfo;
	BaseMaterial->GetAllScalarParameterInfo(ScalarMaterialParameterInfo, GUIDs);
	for (FMaterialParameterInfo Parameter : ScalarMaterialParameterInfo)
	{
		ScalarParametersName.Add(Parameter.Name);
	}
#endif
	for (int scalar = 0; scalar < ScalarParametersName.Num(); scalar++)
	{
		bool NameRepeat = false;
		for (int sc_param = 0; sc_param < ScalarParameter.Num(); sc_param++)
		{
			if (ScalarParameter[sc_param].ParameterName == ScalarParametersName[scalar])
			{
				NameRepeat = true;
			}
		}
		if (!NameRepeat && ScalarParametersName[scalar] != "None" && ScalarParametersName[scalar] != "Time")//create a new scalar parameter by name
		{
			FTextureScalar NewScalarParameter = FTextureScalar();
			NewScalarParameter.ParameterName = ScalarParametersName[scalar];
			ScalarParameter.Add(NewScalarParameter);
		}
	}
	//Vector
	TArray<FName> VectorParametersName;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	BaseMaterial->GetAllVectorParameterNames(VectorParametersName, GUIDs);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	TArray<FMaterialParameterInfo> VectorMaterialParameterInfo;
	BaseMaterial->GetAllVectorParameterInfo(VectorMaterialParameterInfo, GUIDs);
	for (FMaterialParameterInfo Parameter : VectorMaterialParameterInfo)
	{
		VectorParametersName.Add(Parameter.Name);
	}
#endif
	for (int vector = 0; vector < VectorParametersName.Num(); vector++)
	{
		bool NameRepeat = false;
		for (int sc_param = 0; sc_param < VectorParameter.Num(); sc_param++)
		{
			if (VectorParameter[sc_param].ParameterName == VectorParametersName[vector])
			{
				NameRepeat = true;
			}
		}
		if (!NameRepeat && VectorParametersName[vector] != "None")//create a new scalar parameter by name
		{
			FVectorMaterial NewVectorParameter = FVectorMaterial();
			NewVectorParameter.ParameterName = VectorParametersName[vector];
			VectorParameter.Add(NewVectorParameter);
		}
	}
	//Texture
	TArray<FName> TextureParametersName;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	BaseMaterial->GetAllTextureParameterNames(TextureParametersName, GUIDs);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	TArray<FMaterialParameterInfo> TextureMaterialParameterInfo;
	BaseMaterial->GetAllTextureParameterInfo(TextureMaterialParameterInfo, GUIDs);
	for (FMaterialParameterInfo Parameter : TextureMaterialParameterInfo)
	{
		TextureParametersName.Add(Parameter.Name);
	}
#endif
	for (int texture = 0; texture < TextureParametersName.Num(); texture++)
	{
		bool NameRepeat = false;
		for (int sc_param = 0; sc_param < TextureParameter.Num(); sc_param++)
		{
			if (TextureParameter[sc_param].TileName == TextureParametersName[texture])
			{
				NameRepeat = true;
			}
		}
		if (!NameRepeat && TextureParametersName[texture] != "None")//create a new scalar parameter by name
		{
			FTextureMaterial NewTextureParameter = FTextureMaterial();
			NewTextureParameter.TileName = TextureParametersName[texture];
			UTexture* TextureSrc = NULL;
			BaseMaterial->GetTextureParameterValue(TextureParametersName[texture], TextureSrc);
			NewTextureParameter.TileTexture = TextureSrc;
			TextureParameter.Add(NewTextureParameter);
		}
	}
}

void UMultiPackerMaterialNode::ProcessMaterialInstance(UMaterialInstanceDynamic* MID_New) const
{
	for (int v = 0; v < VectorParameter.Num(); ++v)
	{
		if (!VectorParameter[v].ValueChangesOnTime)
		{
			MID_New->SetVectorParameterValue(VectorParameter[v].ParameterName, VectorParameter[v].DefaultVectorValue);
		}
	}
	for (int t = 0; t < TextureParameter.Num(); ++t)
	{
		MID_New->SetTextureParameterValue(TextureParameter[t].TileName, TextureParameter[t].TileTexture);
	}
	for (int s = 0; s < ScalarParameter.Num(); ++s)
	{
		if (!ScalarParameter[s].ValueChangesOnTime)
		{
			MID_New->SetScalarParameterValue(ScalarParameter[s].ParameterName, ScalarParameter[s].DefaultValue);
		}
	}
}

float UMultiPackerMaterialNode::GetFloatDifference(float f1, float f2, int numActualPrint)
{
	if (numActualPrint == 0)
	{
		return f1;
	}
	if (f1 == f2)
	{
		return f1;
	}
	bool f1_negative = f1 < 0;
	bool f2_negative = f2 < 0;
	bool f1Slowerf2 = f1 < f2;
	bool negativeMultiplier = false;
	float diff = 0.0;
	if (!f1_negative && !f2_negative)//++
	{
		diff = f1Slowerf2 ? f2 - f1 : fabs(f1 - f2);
		negativeMultiplier = f1Slowerf2 ? false : true;
	}
	if ((f1_negative == f2_negative) == false)//+- or -+
	{
		diff = f1Slowerf2 ? fabs(f1) + f2 : (f1 + fabs(f2));
		negativeMultiplier = f1Slowerf2 ? true : false;
	}
	if ((f1_negative == true) && (f2_negative == true))//--
	{
		diff = f1Slowerf2 ? fabs(f1) - f2 : (f1 + fabs(f2));
		negativeMultiplier = f1Slowerf2 ? false : true;
	}

	diff /= (GetNumberOfMaterialPrints() - 1);
	diff *= numActualPrint;

	if (negativeMultiplier)
		diff *= -1;

	diff = f1 + diff;
	return diff;
}

FLinearColor UMultiPackerMaterialNode::GetActualVector(FLinearColor v1, FLinearColor v2, int numActualPrint)
{
	return FLinearColor(GetFloatDifference(v1.R, v2.R, numActualPrint), GetFloatDifference(v1.G, v2.G, numActualPrint), GetFloatDifference(v1.B, v2.B, numActualPrint), GetFloatDifference(v1.A, v2.A, numActualPrint));
}

UMaterialInstanceDynamic* UMultiPackerMaterialNode::CreateMaterialDynamic()
{
	while (!MultiPackerMatNode->MaterialBaseInput->GetClass()->IsChildOf<UMaterial>())
	{
		MultiPackerMatNode->MaterialBaseInput = MultiPackerMatNode->MaterialBaseInput->GetBaseMaterial();
	}
	return UMaterialInstanceDynamic::Create(MultiPackerMatNode->MaterialBaseInput, this);
}

#undef LOCTEXT_NAMESPACE
