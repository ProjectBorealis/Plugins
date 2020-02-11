/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/

#include "ToolbagImportFactory.h"
#include "ToolbagImporterPrivatePCH.h"
#include "AssetRegistryModule.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include <Kismet2/BlueprintEditorUtils.h>


//Marmoset includes
#include "ToolbagSceneObject.h"
#include "ToolbagSceneObjectMesh.h"
#include "ToolbagMaterialResource.h"
#include "ToolbagSceneObjectLight.h"
#include "UI/ToolbagImportUI.h"
#include "UI/ToolbagImportOptionsWindow.h"
//#include <IMainFrameModule.h>
#include <MainFrame.h>
#include <AssetSelection.h>
#include "ToolbagUtils.h"

#include "ToolbagUtils/Structs.h"

#define LOCTEXT_NAMESPACE "ToolbagImportFactory"

UToolbagImportFactory::UToolbagImportFactory( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	bShowDialog = true;
	bCanceled = false;
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UBlueprint::StaticClass();
	bEditorImport = true;
	bText = false;
	Formats.Add( TEXT( "tbscene;ToolbagScene" ) );
	ToolbagImporterModule = (FToolbagImporterModule*)FModuleManager::Get().GetModule(FName("ToolbagImporter"));
}

FText UToolbagImportFactory::GetDisplayName() const
{
	return LOCTEXT( "ToolbagImportFactoryDescription", "Toolbag Import" );
}

void UToolbagImportFactory::PostInitProperties()
{
	Super::PostInitProperties();
	bEditorImport = true;
	bText = false;
	ImportUI = NewObject<UToolbagImportUI>(this, NAME_None, RF_NoFlags);
}



UObject* UToolbagImportFactory::FactoryCreateBinary( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled )
{
	if( !FToolbagImporterModule::IsDllLoaded() )
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ToolbagImport_ToolbagUtilsNotLoaded", "ToolbagUtils.dll could not be located, make sure the Plugin is properly installed"));
		bOutOperationCanceled = true;
		return NULL;
	}
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPreImport.Broadcast( this, InClass, InParent, InName, Type );
	//FEditorDelegates::OnAssetPreImport.Broadcast( this, InClass, InParent, InName, Type );
	if(bShowDialog)
	{
		FString TargetPath = InParent->GetPathName();
		int32 index;
		if(TargetPath.FindLastChar('/', index))
		{
			TargetPath.RemoveAt( index, TargetPath.Len() - index );
		}
		TargetPath = PackageTools::SanitizePackageName( TargetPath );
		ShowImportOptions( ImportUI, GetCurrentFilename(), TargetPath);
	}
	if (bCanceled)
	{
		bOutOperationCanceled = true;
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast(this, NULL);
		return NULL;
	}
	ImportUI->SlowTask = new FScopedSlowTask( 100, FText::Format(LOCTEXT( "Toolbag_Importing", "Importing {0}" ), FText::FromName(InName) ) );

	ToolbagUtils::Scene* dllScene = ToolbagImporterModule->DLLReadScene(StringCast<ANSICHAR>(*FPaths::GetPath(UFactory::GetCurrentFilename())).Get(), Buffer, BufferEnd - Buffer);

	ImportUI->TBScene = dllScene;

	ImportUI->SceneScaleFactor = CalculateScaleFactor( dllScene->unitScene );
	ImportUI->ImportScaleFactor = CalculateScaleFactor( dllScene->unitImport ) * dllScene->sceneScale;
	
	FString SceneName = ObjectTools::SanitizeObjectName( InName.ToString() );

	FString NewPackageName = FPackageName::GetLongPackagePath( InParent->GetOutermost()->GetName() ) + TEXT( "/" ) + SceneName;
	NewPackageName = PackageTools::SanitizePackageName( NewPackageName );
	ImportUI->BlueprintPackage = CreatePackage( NULL, *NewPackageName );
	USCS_Node* Node = NULL;
	UObject* NewAsset;
	if(ImportUI->ImportMode == BLUEPRINT)
	{
		bool CreatedNew = false;
		UBlueprintFactory* BlueprintFactory = NewObject<UBlueprintFactory>();
		BlueprintFactory->ParentClass = AActor::StaticClass();
		UBlueprint* Blueprint = FindObject<UBlueprint> (ImportUI->BlueprintPackage, *InName.ToString());
		if(Blueprint == NULL)
		{
			Blueprint = Cast<UBlueprint>( BlueprintFactory->FactoryCreateNew( UBlueprint::StaticClass(), ImportUI->BlueprintPackage, InName, RF_Public | RF_Standalone, NULL, GWarn ) );
			CreatedNew = true;
		}

		Blueprint->Modify( true );
		auto RootNodes = Blueprint->SimpleConstructionScript->GetRootNodes();
		//Find the toolbag root, so only that one will be replaced
		for(int i = 0; i < RootNodes.Num(); ++i)
		{
			if(RootNodes[i]->GetName().Equals("Toolbag Root"))
			{
				Node = RootNodes[i];
				//Clear toolbag root
				while( Node->GetChildNodes().Num() > 0)
					Node->RemoveChildNodeAt(0);
				break;
			}
		}
		//Create new Toolbag root node
		if(Node == NULL)
		{
			Node = Blueprint->SimpleConstructionScript->CreateNode( UStaticMeshComponent::StaticClass(), FName( TEXT( "Toolbag Root" ) ) );
			Node->GetSCS()->AddNode( Node );
		}
		ReadBlueprint( dllScene, Node );
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified( Blueprint );
		if(CreatedNew)
			FAssetRegistryModule::AssetCreated( Blueprint );
		Blueprint->MarkPackageDirty();
		NewAsset = Blueprint;
	}
	else if(ImportUI->ImportMode == INTO_SCENE)
	{
		ReadScene( dllScene, GWorld );
		NewAsset = GWorld;
	}
	else if(ImportUI->ImportMode == ONLY_MATERIALS)
	{
		TMap<FString, UMaterialInterface*> Materials;
		ReadMaterials(dllScene, &Materials);
		TArray<FString> Keys;
		Materials.GetKeys(Keys);
		if(Keys.Num() == 0)
		{
			NewAsset = NULL;
		}
		else
		{
			NewAsset = Materials[Keys[0]];
		}
	}
	else
	{
		NewAsset = NULL;
	}

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Broadcast( this, NewAsset );
	delete(ImportUI->SlowTask);
	ToolbagImporterModule->DLLDestroyScene(dllScene);
	return NewAsset;
}



bool UToolbagImportFactory::ReadObject( ToolbagUtils::SceneObject* ToolbagObject, USCS_Node* Parent, TMap<FString, UMaterialInterface*>& Materials )
{
	USCS_Node* Node;
	ToolbagSceneObject* SO;
	if (ToolbagObject->type == ToolbagUtils::SceneObject::SO_MESH)
	{
		SO = new ToolbagSceneObjectMesh((ToolbagUtils::MeshSceneObject*)ToolbagObject, Materials);
	}
	else if (ToolbagObject->type == ToolbagUtils::SceneObject::SO_EXTERNAL)
	{
		//if(!ImportUI->bSuppressWarnings)
		//{
		//	FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ToolbagImport_AnimationNotSupported", "Animated models are currently unsupported"));
		//}
		SO = new ToolbagSceneObject(ToolbagObject);
	}
	else if(ToolbagObject->type == ToolbagUtils::SceneObject::SO_LIGHT && ImportUI->bImportLights)
	{
		SO = new ToolbagSceneObjectLight((ToolbagUtils::LightSceneObject*)ToolbagObject);
	}
	else
	{
		SO = new ToolbagSceneObject(ToolbagObject);
	}
	Node = SO->Read(Parent, ImportUI);
	for (unsigned i = 0; i < ToolbagObject->childCount; ++i)
	{
		ReadObject( ToolbagObject->children[i], Node, Materials );
	}
	return true;
}


bool UToolbagImportFactory::ReadObject( ToolbagUtils::SceneObject* ToolbagObject, AActor* Parent, TMap<FString, UMaterialInterface*>& Materials )
{
	//read object header
	AActor* Actor;
	ToolbagSceneObject* SO;
	// MESH 1297303631
	// SUBMESH 1397573711
	// LIGHT 1279874127
	// SKYBOX 1397446991
	if (ToolbagObject->type == ToolbagUtils::SceneObject::SO_MESH)
	{
		SO = new ToolbagSceneObjectMesh(ToolbagObject, Materials);
	}
	else if(ToolbagObject->type == ToolbagUtils::SceneObject::SO_LIGHT && ImportUI->bImportLights)
	{
		SO = new ToolbagSceneObjectLight(ToolbagObject);
	}
	else
	{
		SO = new ToolbagSceneObject(ToolbagObject);
	}
	Actor = SO->Read( Parent, ImportUI );
	for (unsigned i = 0; i < ToolbagObject->childCount; ++i)
	{
		ReadObject( ToolbagObject->children[i], Actor, Materials );
	}
	return true;
}


bool UToolbagImportFactory::ReadScene( ToolbagUtils::Scene* ToolbagScene, UWorld* World)
{
	//materials
	ImportUI->SlowTask->EnterProgressFrame( 10 );
	TMap<FString, UMaterialInterface*> Materials;
	ReadMaterials( ToolbagScene, &Materials );
	ImportUI->SlowTask->EnterProgressFrame( 10, LOCTEXT("Toolbag_ImportingScene", "Importing SceneObjects...") );

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName( "ToolbagImport" );
	//Create an empty actor if the node is an empty attribute or the attribute is a mesh(static mesh or skeletal mesh) that was not export
	UActorFactory* Factory = GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
	FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
	//This is a group create an empty actor that just have a transform
	UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
	//Place an empty actor
	AActor * CoordinateConverterActor = Factory->CreateActor( EmptyActorAsset, World->GetLevel( 0 ), FTransform(), RF_Transactional, FName("ToolbagImport") );
	USceneComponent* RootComponent = NewObject<USceneComponent>(CoordinateConverterActor, USceneComponent::GetDefaultSceneRootVariableName());
	RootComponent->Mobility = EComponentMobility::Static;
	RootComponent->bVisualizeComponent = true;
	CoordinateConverterActor->SetRootComponent(RootComponent);
	CoordinateConverterActor->AddInstanceComponent(RootComponent);
	RootComponent->RegisterComponent();
	RootComponent->UpdateComponentToWorld();
	RootComponent->SetRelativeRotation( FRotator( 0, 0, 90 ) );
	//RootComponent->SetRelativeScale3D( FVector( -ImportUI->SceneScaleFactor, -ImportUI->SceneScaleFactor, -ImportUI->SceneScaleFactor ) );
	RootComponent->UpdateComponentToWorld();

	if (ReadObject( ToolbagScene->rootObject, CoordinateConverterActor, Materials ))
	{
		return true;
	}

	//Log::error( "could not read any objects out of scene file (invalid file?)" );
	return false;
}

bool UToolbagImportFactory::ReadBlueprint( ToolbagUtils::Scene* ToolbagScene, USCS_Node* Root )
{
	//materials
	ImportUI->SlowTask->EnterProgressFrame( 10 );
	TMap<FString, UMaterialInterface*> Materials;
	ReadMaterials( ToolbagScene, &Materials );
	ImportUI->SlowTask->EnterProgressFrame( 10, LOCTEXT("Toolbag_ImportingScene", "Importing SceneObjects...") );

	USCS_Node* CoordinateConverterNode = Root->GetSCS()->CreateNode( UStaticMeshComponent::StaticClass(), FName( "ToolbagObject" ) );
	Root->AddChildNode( CoordinateConverterNode );
	UStaticMeshComponent* CoordinateConverter = CastChecked<UStaticMeshComponent>( CoordinateConverterNode->ComponentTemplate );
	CoordinateConverter->SetRelativeRotation( FRotator( 0, 0, 90 ) );
	//CoordinateConverter->SetRelativeScale3D( FVector( -ImportUI->SceneScaleFactor, -ImportUI->SceneScaleFactor, -ImportUI->SceneScaleFactor ) );
	CoordinateConverter->UpdateComponentToWorld();

	//scene objects
	if (ReadObject( ToolbagScene->rootObject, CoordinateConverterNode, Materials ))
	{
		return true;
	}

	//Log::error( "could not read any objects out of scene file (invalid file?)" );
	return false;
}

void UToolbagImportFactory::ReadMaterials( ToolbagUtils::Scene* ToolbagScene, TMap<FString, UMaterialInterface*>* MaterialMap ) const
{
	for( unsigned i=0; i<ToolbagScene->materialsCount; ++i )
	{
		ImportUI->SlowTask->EnterProgressFrame( 70 / ToolbagScene->materialsCount );
		FString Key;
		UMaterial* Material = ToolbagMaterialResource::Read( &ToolbagScene->materials[i], ImportUI->AbsoluteMaterialsPath, ImportUI, Key );

		//Create Material Instance
		FString name = Material->GetName() + "_Instance";
		UPackage* MaterialInstancePackage = ToolbagUtils::CreatePackageForAsset<UMaterialInstance>( ImportUI->AbsoluteMaterialsPath, name );
		UMaterialInstance* MaterialInstance = FindObject<UMaterialInstanceConstant>(MaterialInstancePackage, *name);
		if(MaterialInstance == NULL)
		{
			UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			Factory->InitialParent = Material;
			MaterialInstance = Cast<UMaterialInstance>(Factory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), MaterialInstancePackage, FName(*name), RF_Standalone | RF_Public, NULL, GWarn ));
			FAssetRegistryModule::AssetCreated( MaterialInstance );
		}
		MaterialInstance->MarkPackageDirty();
		if(MaterialMap != NULL)
			MaterialMap->Add(Key, MaterialInstance);
	}
}

void UToolbagImportFactory::ShowImportOptions(UToolbagImportUI* IUI, const FString& SourcePath, const FString& TargetPath)
{
	TSharedPtr<SWindow> ParentWindow;

	if( FModuleManager::Get().IsModuleLoaded( "MainFrame" ) )
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>( "MainFrame" );
		ParentWindow = MainFrame.GetParentWindow();
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("Toolbag_ImportWindowTitle", "Toolbag Scene Import Options"))
		.SizingRule( ESizingRule::Autosized );

	TSharedPtr<SToolbagImportOptionsWindow> ToolbagWindow;
	Window->SetContent
		(
			SAssignNew(ToolbagWindow, SToolbagImportOptionsWindow)
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
