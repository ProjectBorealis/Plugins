/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/

#include "ToolbagMaterialImportFactory.h"
#include "ToolbagImporterPrivatePCH.h"
#include "AssetRegistryModule.h"
#include "ObjectTools.h"
#include "PackageTools.h"
//#include "BlueprintEditorUtils.h"


//Marmoset includes
#include "ToolbagSceneObject.h"
#include "ToolbagSceneObjectMesh.h"
#include "ToolbagMaterialResource.h"
#include "ToolbagSceneObjectLight.h"
#include "UI/ToolbagImportUI.h"
#include "UI/ToolbagMaterialImportOptionsWindow.h"
//#include <IMainFrameModule.h>
#include <AssetSelection.h>
#include "ToolbagUtils.h"

#include "ToolbagUtils/Structs.h"

#define LOCTEXT_NAMESPACE "ToolbagImportFactory"

UToolbagMaterialImportFactory::UToolbagMaterialImportFactory( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	bShowDialog = true;
	bCanceled = false;
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UMaterial::StaticClass();
	bEditorImport = true;
	bText = false;
	Formats.Add( TEXT( "tbmat;ToolbagMaterial" ) );
	ToolbagImporterModule = (FToolbagImporterModule*)FModuleManager::Get().GetModule(FName("ToolbagImporter"));
}

FText UToolbagMaterialImportFactory::GetDisplayName() const
{
	return LOCTEXT( "ToolbagImportFactoryDescription", "Toolbag Material Import" );
}

void UToolbagMaterialImportFactory::PostInitProperties()
{
	Super::PostInitProperties();
	bEditorImport = true;
	bText = false;

	ImportUI = NewObject<UToolbagImportUI>(this, NAME_None, RF_NoFlags);
}



UObject* UToolbagMaterialImportFactory::FactoryCreateBinary( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled )
{
	if( !FToolbagImporterModule::IsDllLoaded() )
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ToolbagImport_ToolbagUtilsNotLoaded", "ToolbagUtils.dll could not be located, make sure the Plugin is properly installed"));
		bOutOperationCanceled = true;
		return NULL;
	}
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPreImport.Broadcast( this, InClass, InParent, InName, Type );
	FString TargetPath = InParent->GetPathName();
	int32 index;
	if(TargetPath.FindLastChar('/', index))
	{
		TargetPath.RemoveAt( index, TargetPath.Len() - index );
	}
	TargetPath = PackageTools::SanitizePackageName( TargetPath );
	if(bShowDialog)
	{
		ShowImportOptions( ImportUI, GetCurrentFilename(), TargetPath);
	}
	if (bCanceled)
	{
		bOutOperationCanceled = true;
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, NULL);
		return NULL;
	}
	ImportUI->SlowTask = new FScopedSlowTask( 100, FText::Format(LOCTEXT( "Toolbag_Importing_Material", "Importing Material {0}" ), FText::FromName(InName) ) );

	ToolbagUtils::Material* dllMaterial = ToolbagImporterModule->DLLReadMaterial(StringCast<ANSICHAR>(*UFactory::GetCurrentFilename()).Get(), Buffer, BufferEnd - Buffer);
	// make static mesh object

	UMaterial* Material;
	{
		ImportUI->SlowTask->EnterProgressFrame( 0.5f );
		FString Key;
		Material = ToolbagMaterialResource::Read( dllMaterial, TargetPath, ImportUI, Key );
		//Create Material Instance
		FString name = Material->GetName() + "_Instance";
		UPackage* MaterialInstancePackage = ToolbagUtils::CreatePackageForAsset<UMaterialInstance>( TargetPath, name );
		UMaterialInstance* MaterialInstance = FindObject<UMaterialInstanceConstant>(MaterialInstancePackage, *name);
		if(MaterialInstance == NULL)
		{
			UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			Factory->InitialParent = Material;
			MaterialInstance = Cast<UMaterialInstance>(Factory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), MaterialInstancePackage, FName(*name), RF_Standalone | RF_Public, NULL, GWarn ));
			FAssetRegistryModule::AssetCreated( MaterialInstance );
		}
		MaterialInstance->MarkPackageDirty();
	}
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast( this, Material );
	delete(ImportUI->SlowTask);
	ToolbagImporterModule->DLLDestroyMaterial(dllMaterial);
	return Material;
}


void UToolbagMaterialImportFactory::ShowImportOptions(UToolbagImportUI* IUI, const FString& SourcePath, const FString& TargetPath)
{
	TSharedPtr<SWindow> ParentWindow;

	if( FModuleManager::Get().IsModuleLoaded( "MainFrame" ) )
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>( "MainFrame" );
		ParentWindow = MainFrame.GetParentWindow();
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("Toolbag_ImportWindowTitle", "Toolbag Material Import Options"))
		.SizingRule( ESizingRule::Autosized );

	TSharedPtr<SToolbagMaterialImportOptionsWindow> ToolbagWindow;
	Window->SetContent
	(
		SAssignNew(ToolbagWindow, SToolbagMaterialImportOptionsWindow)
		.ImportUI(IUI)
		.WidgetWindow(Window)
		.SourcePath(FText::FromString(SourcePath))
		.TargetPath(FText::FromString(TargetPath))
	);

	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);
	bCanceled = !ToolbagWindow->ShouldImport();
	bShowDialog = !ToolbagWindow->ShouldImportAll();
	if(!bCanceled)
	{
		IUI->WriteConfig();
	}
	IUI->SaveConfig();
}


#undef LOCTEXT_NAMESPACE
