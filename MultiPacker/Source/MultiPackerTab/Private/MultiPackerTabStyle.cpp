/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerTabStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FMultiPackerTabStyle::StyleInstance = NULL;

void FMultiPackerTabStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMultiPackerTabStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FMultiPackerTabStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MultiPacker"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FMultiPackerTabStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("MultiPacker"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("MultiPacker")->GetBaseDir() / TEXT("Resources"));

	Style->Set("MultiPacker.OpenPluginWindow", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("MultiPacker.OpenPluginWindow.Small", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH

void FMultiPackerTabStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FMultiPackerTabStyle::Get()
{
	return *StyleInstance;
}
