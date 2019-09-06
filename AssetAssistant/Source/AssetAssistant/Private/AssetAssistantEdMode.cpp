// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEdMode.h"
#include "AssetAssistantEdModeToolkit.h"
#include "AssetAssistantEditorUISetting.h"
#include "Toolkits/ToolkitManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "AssetAssistantEdMode"

const FEditorModeID FAssetAssistantEdMode::EM_AssetAssistantEdModeId = TEXT("EM_AssetAssistantEdMode");

FAssetAssistantEdMode::FAssetAssistantEdMode()
	: FEdMode()
	, CurrentToolMode(EToolMode::Find)
{
	UICommandList = MakeShareable(new FUICommandList);
	UISetting = NewObject<UAssetAssistantEditorUISetting>(GetTransientPackage(), TEXT("UISetting"));
	UISetting->ClearFlags(RF_Transactional);
	UISetting->SetParent(this);
}

void FAssetAssistantEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdMode::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(UISetting);
}

void FAssetAssistantEdMode::Enter()
{
	FEdMode::Enter();
	UISetting->Load();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FAssetAssistantEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FAssetAssistantEdMode::Exit()
{
	GEditor->OnObjectsReplaced().RemoveAll(this);
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	UISetting->Save();
	FEdMode::Exit();
}

bool FAssetAssistantEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return false;
}

bool FAssetAssistantEdMode::UsesToolkits() const
{
	return true;
}

void FAssetAssistantEdMode::SetCurrentToolMode(EToolMode InMode)
{
	CurrentToolMode = InMode;
}

EToolMode FAssetAssistantEdMode::GetCurrentToolMode() const
{
	return CurrentToolMode;
}

void FAssetAssistantEdMode::NotifyMessage(const FText& Message)
{
	if (!Message.IsEmpty())
	{
		FNotificationInfo NotificationInfo(Message);
		NotificationInfo.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
}

void FAssetAssistantEdMode::SaveResultList()
{
	UISetting->LastListViewTArray = ViewResult->GetTArrayToCopy().Replace(TEXT("\r\n"), TEXT(""));
	UISetting->SaveResultListFlush();
}

void FAssetAssistantEdMode::LoadResultList()
{
	UISetting->LoadResultList();
	ViewResult->UpdateResultFromList(UISetting->LastListViewTArray, false, false);
}

#undef LOCTEXT_NAMESPACE


