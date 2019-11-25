/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "IPropertyUtilities.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FTileCopyCustomizationLayout"

//Class forwarding
class UMultiPackerLayerDatabase;

/**
* Customizes TileCopy to have a button
*/
class FTileCopyCustomizationLayout : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	FReply OnButtonMCPressed();

	FReply OnButtonLCPressed();

	void GetNodesOnText(bool bMaterialCollection);

	FString GetNodesMaterialCollection();

	FString GetNodesLinearColor();

	UMultiPackerLayerDatabase* LayerDatabase;

	UPROPERTY(EditInstanceOnly, meta = (MultiLine = true))
		FString OutputString;

};

#undef LOCTEXT_NAMESPACE
