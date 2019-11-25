/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "MultiPackerAssetEditor/Nodes/MultiPackerBaseNode.h"
#include "MultiPackerMaterialNode.generated.h"

class UMultiPackerMatNode;
class UMaterialInstanceDynamic;

UCLASS(MinimalAPI)
class UMultiPackerMaterialNode : public UMultiPackerBaseNode
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, instanced, Category = "MaterialNode")
		UMultiPackerMatNode* MultiPackerMatNode;

	//Create a New MaterialInstance Based on the Material Father
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode")
		bool MaterialInstance = false;

	//Every time you modify the Material you need to recalculate this to set the data before process everything
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNodeParameters", meta = (EditCondition = MaterialInstance))
		EApplyMaterial MaterialButton;

	//Array of Vector Parameters on Material, the Parameter Name is filled automatically
	UPROPERTY(EditDefaultsOnly, EditFixedSize, Category = "MaterialNodeParameters|Parameters Arrays", meta = (EditCondition = MaterialInstance))
		TArray<FVectorMaterial> VectorParameter;

	//Array of Texture Parameters on Material, the Parameter Name is filled automatically
	UPROPERTY(EditDefaultsOnly, EditFixedSize, Category = "MaterialNodeParameters|Parameters Arrays", meta = (EditCondition = MaterialInstance))
		TArray<FTextureMaterial> TextureParameter;

													//Array of Scalar Parameters on Material, the Parameter Name is filled automatically
	UPROPERTY(EditDefaultsOnly, EditFixedSize, Category = "MaterialNodeParameters|Parameters Arrays", meta = (EditCondition = MaterialInstance))
		TArray<FTextureScalar> ScalarParameter;

	//This number indicates how many times the Material will be rendered,
	//every render is a Tile and if you select modification on the Parameters will be done by lineal change by print
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNodeParameters", meta = (ClampMin = "1", ClampMax = "500", UIMin = "1", UIMax = "500", EditCondition = MaterialInstance))
		float NumPrints = 1.0f;

	//This Scalar Parameter is Independently because its a strong modificator on the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNodeTimeParameters", meta = (EditCondition = MaterialInstance))
		bool TimeParameter = false;

	//The value of time will do a lineal change of the period starting from 0
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNodeTimeParameters", meta = (EditCondition = TimeParameter))
		float Period = 1.0f;

	//True to render every NumPrint with the same Time Value
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNodeTimeParameters", meta = (EditCondition = TimeParameter))
		bool SpecificTime = false;

	//Select to autoFill the names of the DataBase
	UPROPERTY(EditAnywhere, Category = "MaterialNodeTileDataBase")
		ENameMaterial AutoNameFill;

	//Array of Information to modify and set the name to select every individual Tile from the DataBase
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "MaterialNodeTileDataBase")
		TArray<FTileThumbDatabase> AMatTileData;

	void AllocateDefaultPins() override;
	void NodeConnectionListChanged() override;

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const;
	void SetGenericGraphNode(UMultiPackerMatNode* InNode);
		
	FString GetNodeTitle();
	
	bool SetThumbnail();

	void SetMaterialInput(UMaterialInterface* Material);
	
	UObject* GetNodeAssetObject(UObject* Outer);
	UObject* GetThumbnailAssetObject();

	FLinearColor GetBackgroundColor() const;

	void GenerateTileDataBase();
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
	//TilePointer Side class
	void ProcessTiles() override; //override function from UMultiPackerBaseNode
	FVector2D GetTileSize();
	FVector2D GetSplitTileSize();
#endif
//UMultiPackerBaseNode override functions part
	bool PinConnected() override;
	bool IsObjectValid() override;
	bool IsNodeSelectedSdf() override;
	bool GetMsdf() override;
	FTileThumbDatabase GetTileThumbDatabase(uint16 num) override;
	uint16 GetNumberTiles() override;

	void ChangeEnumMaterial(bool Material);
	void ProcessPrintByNumber(uint8 number);
	uint8 GetNumberOfMaterialPrints();
	UMaterialInstanceDynamic* GetMaterialInstanceDynamic(uint8 number) const;
	void PreProcessPrints();
	void SetTileThumbs(TArray<UTilePointer*> TileThumbs);
	FName GetTileName(int Tile);
private:
	TArray<UMaterialInstanceDynamic*> MID_Iteration;
	void OnChangeValueValidity();
	void ProcessParameters();
	void ProcessMaterialInstance(UMaterialInstanceDynamic* MID_New) const;
	float GetFloatDifference(float f1, float f2, int numActualPrint);
	FLinearColor GetActualVector(FLinearColor v1, FLinearColor v2, int numActualPrint);
	UMaterialInstanceDynamic* CreateMaterialDynamic();
	UPROPERTY(Transient)
		TArray<UTilePointer *> Thumbnails;
};
