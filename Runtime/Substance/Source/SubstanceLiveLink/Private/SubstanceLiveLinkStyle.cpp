// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkStyle.cpp
#include "SubstanceLiveLinkStyle.h"
#include "SubstanceLiveLinkPrivatePCH.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Templates/SharedPointer.h"
#include "Interfaces/IPluginManager.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FSubstanceLiveLinkStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FSubstanceLiveLinkStyle::StyleSet = nullptr;

void FSubstanceLiveLinkStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon16x16(16.0f, 16.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	//Create style set
	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	//Set up image brushes
	StyleSet->Set("Icon.SubstancePainter16px", new IMAGE_PLUGIN_BRUSH("LiveLinkUI/PainterIcon16", Icon16x16));

#if WITH_EDITOR
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
#endif
}

void FSubstanceLiveLinkStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
#if WITH_EDITOR
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
#endif
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

FName FSubstanceLiveLinkStyle::GetStyleSetName()
{
	static FName SubstanceStyleName(TEXT("SubstanceLiveLinkStyle"));
	return SubstanceStyleName;
}

FString FSubstanceLiveLinkStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Substance"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

#undef IMAGE_PLUGIN_BRUSH
