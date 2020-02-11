/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagSceneObject.h"
#include "ToolbagImporterPrivatePCH.h"
#include <string>
#include "UI/ToolbagImportUI.h"
#include <AssetSelection.h>
#include "ToolbagUtils/Structs.h"

#define LOCTEXT_NAMESPACE "ToolbagImportFactory"
USCS_Node* ToolbagSceneObject::Read(  USCS_Node* Parent, UToolbagImportUI* ImportUI )
{
	FString SOName = FString( TSO->name );
	ImportUI->SlowTask->FrameMessage = FText::Format(LOCTEXT("Toolbag_ImportingScene", "Importing SceneObjects... {0}"), FText::FromString(SOName) );
	USCS_Node* Node = Parent->GetSCS()->CreateNode( GetComponentClass(), FName( *SOName ) );
	Parent->AddChildNode( Node );
	USceneComponent* SceneComponent = CastChecked<USceneComponent>( Node->ComponentTemplate );
	FMatrix matrix;
	for (int i = 0; i < 4; ++i)
	{
		matrix.M[0][i] = TSO->matrix[0  + i];
		matrix.M[1][i] = TSO->matrix[4  + i];
		matrix.M[2][i] = TSO->matrix[8  + i];
		matrix.M[3][i] = TSO->matrix[12 + i] * (i < 3 ? -ImportUI->SceneScaleFactor : 1);
	}
	//matrix.ScaleTranslation( matrix.Inverse().GetScaleVector() );
	//matrix.M[3][0] *= -1;
	FTransform transform;
	transform.SetFromMatrix( matrix );
	SceneComponent->SetRelativeTransform(transform);
	if(Parent != NULL)
	{
		USceneComponent* ParentSceneComponent = CastChecked<USceneComponent>( Parent->ComponentTemplate );
		SceneComponent->SetVisibility( ParentSceneComponent->IsVisible() && TSO->isVisible, true );
	}
	ParseComponent( ImportUI, SceneComponent, SOName);
	SceneComponent->UpdateComponentToWorld();
	return Node;
}

AActor* ToolbagSceneObject::Read(  AActor* Parent, UToolbagImportUI* ImportUI )
{
	FString SOName = FString( TSO->name );
	ImportUI->SlowTask->FrameMessage = FText::Format(LOCTEXT("Toolbag_ImportingScene", "Importing SceneObjects... {0}"), FText::FromString(SOName) );
	//Create an empty actor if the node is an empty attribute or the attribute is a mesh(static mesh or skeletal mesh) that was not export
	UActorFactory* Factory = GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
	FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
	//This is a group create an empty actor that just have a transform
	UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
	//Place an empty actor
	AActor * Actor = Factory->CreateActor( EmptyActorAsset, Parent->GetWorld()->GetLevel( 0 ), FTransform(), RF_Transactional, FName(*SOName) );
	USceneComponent* RootComponent = NewObject<USceneComponent>(Actor, USceneComponent::GetDefaultSceneRootVariableName());
	RootComponent->Mobility = EComponentMobility::Static;
	RootComponent->bVisualizeComponent = true;
	Actor->SetRootComponent(RootComponent);
	Actor->AddInstanceComponent(RootComponent);
	RootComponent->RegisterComponent();
	USceneComponent* Component = NewObject<USceneComponent>( Actor, GetComponentClass());
	Actor->SetActorLabel(SOName);
	if (Parent != nullptr)
	{
		USceneComponent* ParentRootComponent = Cast<USceneComponent>(Parent->GetRootComponent());
		if (ParentRootComponent)
		{
			if (GEditor->CanParentActors(Parent, Actor))
			{
				GEditor->ParentActors(Parent, Actor, NAME_None);
				RootComponent->SetVisibility( ParentRootComponent->IsVisible() && TSO->isVisible, true );
			}
		}
	}

	Component->RegisterComponent();        //You must ConstructObject with a valid Outer that has world, see above	 
	Component->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform); 
	FMatrix matrix;
	for (int i = 0; i < 4; ++i)
	{
		matrix.M[0][i] = TSO->matrix[0  + i];
		matrix.M[1][i] = TSO->matrix[4  + i];
		matrix.M[2][i] = TSO->matrix[8  + i];
		matrix.M[3][i] = TSO->matrix[12 + i] * (i < 3 ? -ImportUI->SceneScaleFactor : 1);
	}
	//matrix.ScaleTranslation( matrix.Inverse().GetScaleVector() );
	//matrix.M[3][0] *= -1;
	FTransform transform;
	transform.SetFromMatrix( matrix );
	RootComponent->SetRelativeTransform(transform);
	ParseComponent( ImportUI, Component, SOName);
	Component->UpdateComponentToWorld();
	return Actor;
}
#undef LOCTEXT_NAMESPACE