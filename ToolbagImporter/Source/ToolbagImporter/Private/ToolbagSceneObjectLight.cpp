/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagSceneObjectLight.h"
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagUtils/Structs.h"
#include "UI/ToolbagImportUI.h"


void ToolbagSceneObjectLight::ParseComponent( UToolbagImportUI* ImportUI, USceneComponent* Component, FString Name )
{
	ToolbagUtils::LightSceneObject* TSOL = (ToolbagUtils::LightSceneObject*) TSO;
	UClass* C = 0;

	ULightComponent* LightComponent = CastChecked<ULightComponent>( Component );
	LightComponent->SetLightColor( FLinearColor(TSOL->color[0],TSOL->color[1],TSOL->color[2]) );
	LightComponent->SetIntensity( TSOL->brightness * (TSOL->lightType == ToolbagUtils::LightSceneObject::LightType::LIGHT_DIRECTIONAL ? 2 : PI ) );
	FMatrix Mat = LightComponent->GetRelativeTransform().ToMatrixWithScale();
	FRotator newRotator = FRotator(90, 0, 0);
	Mat = FRotationMatrix(newRotator) * Mat;
	FTransform transform;
	transform.SetFromMatrix( Mat );
	LightComponent->SetRelativeTransform(transform);
	LightComponent->CastShadows = TSOL->castShadows;
	LightComponent->CastStaticShadows = TSOL->castShadows;


	if(TSOL->lightType == ToolbagUtils::LightSceneObject::LightType::LIGHT_OMNI || TSOL->lightType == ToolbagUtils::LightSceneObject::LightType::LIGHT_SPOT)
	{
		UPointLightComponent* PointLight = CastChecked<UPointLightComponent>( LightComponent );
		PointLight->SetLightFalloffExponent( FMath::Pow( TSOL->attenuation, 1.5f ) * 7 + 1 );
		PointLight->SetAttenuationRadius( TSOL->radius * ImportUI->SceneScaleFactor );
		PointLight->bUseInverseSquaredFalloff = false;
	}

	if(TSOL->lightType == ToolbagUtils::LightSceneObject::LightType::LIGHT_SPOT)
	{
		USpotLightComponent* SpotLight = CastChecked<USpotLightComponent>( LightComponent );
		SpotLight->SetInnerConeAngle( TSOL->spotAngle/2 - TSOL->spotSharpness*TSOL->spotAngle/200 );
		SpotLight->SetOuterConeAngle( TSOL->spotAngle/2 );
	}
}

UClass* ToolbagSceneObjectLight::GetComponentClass()
{
	switch (((ToolbagUtils::LightSceneObject*)TSO)->lightType)
	{
	case ToolbagUtils::LightSceneObject::LightType::LIGHT_DIRECTIONAL:
		return UDirectionalLightComponent::StaticClass();
		break;
	case ToolbagUtils::LightSceneObject::LightType::LIGHT_OMNI:
		return UPointLightComponent::StaticClass();
		break;
	case ToolbagUtils::LightSceneObject::LightType::LIGHT_SPOT:
		return USpotLightComponent::StaticClass();
		break;
	}
	return UPointLightComponent::StaticClass();
}
