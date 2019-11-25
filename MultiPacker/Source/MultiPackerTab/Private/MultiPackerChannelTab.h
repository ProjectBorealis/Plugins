/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "MultiPackerBaseEnums.h"
#include "MultiPackerDetailPanel.h"
#include <Framework/SlateDelegates.h>
#include "Toolkits/AssetEditorToolkit.h"
#include <UObject/GCObject.h>

class IDetailsView;
class FSpawnTabArgs;
class FAssetToolsModule;
class UTilePointer;
struct FSlateImageBrush;
class SImage;

class FMultiPackerChannelTab : public FAssetEditorToolkit, public FGCObject
{
public:
	FMultiPackerChannelTab();
	virtual ~FMultiPackerChannelTab();
	
	void InitGenericGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost);
	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	// End of IToolkit interface

	UPROPERTY()
		UMultiPackerDetailPanel* PropertyMPChannel;

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	// End of FAssetEditorToolkit

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface
private:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Buttons(const FSpawnTabArgs& Args);
	TSharedPtr<class IDetailsView> PropertyWidget;
	//Preview Variables
	TSharedPtr<FSlateImageBrush> PreviewBrush;
	TSharedPtr<SImage> ImageWidget;
	TSharedPtr<SBorder> PreviewWidget;

	FReply OnClickButton();

	EChannelSelectionInput GetChannelEnum(EMPChannelMaskParameterColor InEnum);
	UTilePointer* ProcessTextureChannel(UTexture2D* InTexture, const int32 InSizeVertical, const int32 InSizeHorizontal, EMPChannelMaskParameterColor InChannel, bool InvertColors);
	TArray<FString> TexturePackageName(FAssetToolsModule& AssetToolsModule);
	TSharedRef<class SButton> CreateAndSetButton(FOnClicked InOnClicked, const TAttribute<FText>& InIcon, const TAttribute<FText>& InTooltip, const TAttribute<FText>& InButton);
	FReply PreviewRedTexture();
	FReply PreviewGreenTexture();
	FReply PreviewBlueTexture();
	FReply PreviewAlphaTexture();
	void PreviewTexture(EMPChannelMaskParameterColor InEnum, UTexture2D* InTexture, bool InvertColors);
	void SetPreviewTexture(UTexture2D* InPreviewTexture);
};
