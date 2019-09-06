// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "AssetAssistantEditorUISetting.h"
#include "AssetAssistantEditorUISettingCustomization_Macro.h"
#include "AssetAssistantEdMode.h"

UAssetAssistantEditorUISetting::UAssetAssistantEditorUISetting(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ParentMode(nullptr)
{
	ResetPatternNames();
}

void UAssetAssistantEditorUISetting::LoadSet(FString SetID)
{
	GConfig->GetString(TEXT("AssetAssistant"), *SetID, CurrentSetTArrayText, GEditorPerProjectIni);
}

void UAssetAssistantEditorUISetting::SaveSet(FString SetID)
{
	GConfig->SetString(TEXT("AssetAssistant"), *SetID, *CurrentSetTArrayText, GEditorPerProjectIni);
}

void UAssetAssistantEditorUISetting::RemoveSet(FString SetID)
{
	GConfig->RemoveKey(TEXT("AssetAssistant"), *SetID, GEditorPerProjectIni);
}

void UAssetAssistantEditorUISetting::SaveResultListFlush()
{
	GConfig->SetString(TEXT("AssetAssistant"), TEXT("LastListViewTArray"), *LastListViewTArray, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

void UAssetAssistantEditorUISetting::LoadResultList()
{
	GConfig->GetString(TEXT("AssetAssistant"), TEXT("LastListViewTArray"), LastListViewTArray, GEditorPerProjectIni);
}

void UAssetAssistantEditorUISetting::Load()
{
	bool bConfigExist = false;
	GConfig->GetBool(TEXT("AssetAssistant"), TEXT("bConfigExist"), bConfigExist, GEditorPerProjectIni);
	
	if (bConfigExist)
	{
		GConfig->GetInt(TEXT("AssetAssistant"), TEXT("LastSetNR"), LastSetNR, GEditorPerProjectIni);
		GConfig->GetInt(TEXT("AssetAssistant"), TEXT("LastAutosaveSetNR"), LastAutosaveSetNR, GEditorPerProjectIni);
		GConfig->GetArray(TEXT("AssetAssistant"), TEXT("PatternNames"), PatternNames, GEditorPerProjectIni);
		GConfig->GetArray(TEXT("AssetAssistant"), TEXT("SetsIDs"), SetsIDs, GEditorPerProjectIni);
		GConfig->GetArray(TEXT("AssetAssistant"), TEXT("AutosaveSetsIDs"), AutosaveSetsIDs, GEditorPerProjectIni);

		int32 aRecoverType;
		GConfig->GetInt(TEXT("AssetAssistant"), TEXT("AutosaveRecoverType"), aRecoverType, GEditorPerProjectIni);
		AutosaveRecoverType = (EAutosaveRecoverType)aRecoverType;

		GConfig->GetFloat(TEXT("AssetAssistant"), TEXT("AutosaveRepeatRate"), AutosaveRepeatRate, GEditorPerProjectIni);
		
		GConfig->GetInt(TEXT("AssetAssistant"), TEXT("PluginHotkey"), PluginHotkey, GEditorPerProjectIni);
	}
}

void UAssetAssistantEditorUISetting::Save()
{
	GConfig->SetArray(TEXT("AssetAssistant"), TEXT("PatternNames"), PatternNames, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("AssetAssistant"), TEXT("LastSetNR"), LastSetNR, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("AssetAssistant"), TEXT("LastAutosaveSetNR"), LastAutosaveSetNR, GEditorPerProjectIni);
	GConfig->SetArray(TEXT("AssetAssistant"), TEXT("SetsIDs"), SetsIDs, GEditorPerProjectIni);
	GConfig->SetArray(TEXT("AssetAssistant"), TEXT("AutosaveSetsIDs"), AutosaveSetsIDs, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("AssetAssistant"), TEXT("AutosaveRecoverType"), (uint8)AutosaveRecoverType, GEditorPerProjectIni);
	GConfig->SetFloat(TEXT("AssetAssistant"), TEXT("AutosaveRepeatRate"), AutosaveRepeatRate, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("AssetAssistant"), TEXT("PluginHotkey"), PluginHotkey, GEditorPerProjectIni);

	// Set flag that config exist
	GConfig->SetBool(TEXT("AssetAssistant"), TEXT("bConfigExist"), true, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}


void UAssetAssistantEditorUISetting::ResetPatternNames()
{
	// Default pattern names
	PatternNames.Empty();
	PatternNames.Add("{Arr1}");
	PatternNames.Add("{Name}_{Nr}");
	PatternNames.Add("Renamed_{Nr+1}");
	PatternNames.Add("{Date}");
	PatternNames.Add("{Time}");
	PatternNames.Add("{Arr1}{Name}{Arr2}");
}

void UAssetAssistantEditorUISetting::SetParent(FAssetAssistantEdMode* EditMode)
{
	ParentMode = EditMode;
}

void UAssetAssistantEditorUISetting::PostEditChangeProperty(struct FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// save if Auto-save properties were changed
	FString propertyName = *PropertyChangedEvent.Property->GetName();
	if (propertyName == "AutosaveRecoverType" ||
		propertyName == "AutosaveRepeatRate" ||
		propertyName == "PluginHotkey" ||
		propertyName == "UnregisterModules")
	{
		Save();
	}
}

void UAssetAssistantEditorUISetting::runAutosave()
{
	GEditor->GetTimerManager()->ClearTimer(AutosaveHandle);
	GEditor->GetTimerManager()->SetTimer(AutosaveHandle, this, &UAssetAssistantEditorUISetting::OnAutosave, AutosaveRepeatRate, true, -1.0f);
}

void UAssetAssistantEditorUISetting::OnAutosave()
{
	ParentMode->AssetMacro->OnCreateAutosaveClicked();
}
