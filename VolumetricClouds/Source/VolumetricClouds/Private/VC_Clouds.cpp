// Fill out your copyright notice in the Description page of Project Settings.

#include "VC_Clouds.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h"
#include "Components/LightComponent.h" 

#if WITH_EDITOR
#include "Runtime/Engine/Classes/Components/BillboardComponent.h"
#include "UnrealEd.h"
#include "Editor/UnrealEd/Public/EditorViewportClient.h"
#endif


#define CLOUD_GROUND_SHADOWS 0

// Sets default values
AVC_Clouds::AVC_Clouds(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Structure to hold one-time initialization.
	struct FConstructorStatics
	{
		//A helper class object we use to find target UTexture2D object in resource package.
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> VolumetricCloudsTextureObject;

		//Icon sprite category name.
		FName ID_VolumetricClouds; 

		//Icon sprite display name.
		FText NAME_VolumetricClouds;

		FConstructorStatics()
			//Use helper class object to find the texture.
			: VolumetricCloudsTextureObject(TEXT("Texture2D'/VolumetricClouds/Icons/I_VolumetricClouds.I_VolumetricClouds'"))
			, ID_VolumetricClouds(TEXT("VolumetricClouds"))
			, NAME_VolumetricClouds(NSLOCTEXT("SpriteCategory", "VolumetricClouds", "VolumetricClouds"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	//Create scene component.
	USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	RootComponent->Mobility = EComponentMobility::Static;



#if WITH_EDITORONLY_DATA
	SpriteComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->Sprite = ConstructorStatics.VolumetricCloudsTextureObject.Get();					//Get the sprite texture from helper class object.
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_VolumetricClouds;						//Assign sprite category name.
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_VolumetricClouds;					//Assign sprite display name.
		SpriteComponent->SetupAttachment(RootComponent);	//Attach sprite to scene component.
		SpriteComponent->Mobility = EComponentMobility::Static;
	}
#endif // WITH_EDITORONLY_DATA

	//Create a static mesh component.
	CloudsBoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CloudsBoxMesh"));
	CloudsBoxMesh->SetupAttachment(RootComponent);

	//Find a mesh for a static mesh component.
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/VolumetricClouds/Meshes/SM_CloudsBox.SM_CloudsBox'"));

	//If mesh exist apply it to a component.
	if (MeshAsset.Object != NULL)
	{
		CloudsBoxMesh->SetStaticMesh(MeshAsset.Object);
	}

	//Find material instance for clouds.
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant>CloudsMaterialInstanceAsset(TEXT("MaterialInstanceConstant'/VolumetricClouds/Materials/MI_VolumetricClouds.MI_VolumetricClouds'"));

	//If material exist apply it to cloud box mesh and store into actor variable.
	if (CloudsMaterialInstanceAsset.Object != NULL)
	{
		CloudsMaterialInstance = CloudsMaterialInstanceAsset.Object;
		CloudsBoxMesh->SetMaterial(0, CloudsMaterialInstance);
	}
	
	CloudsBoxMesh->bCastDynamicShadow = false;
	CloudsBoxMesh->bCastStaticShadow = false;
	CloudsBoxMesh->CastShadow = false;

	//Set default actor location and scale.
	SetActorLocation(FVector(0.0f, 0.0f, 125000.0f));
	SetActorScale3D(FVector(40000.0f, 40000.0f, 2000.0f));

	//Set tht actor can tick.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

#if CLOUD_GROUND_SHADOWS
	//Find material instance for render cloud shdows.
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant>ShadowsMaterialInstanceAsset(TEXT("MaterialInstanceConstant'/VolumetricClouds/Materials/MI_GroundShadowRender.MI_GroundShadowRender'"));

	//If material exist store it into actor variable.
	if (ShadowsMaterialInstanceAsset.Object != NULL)
	{
		ShadowRenderMaterialInstance = ShadowsMaterialInstanceAsset.Object;
	}

	//Find Render target texture.
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D>ShadowsRenderTargetAsset(TEXT("TextureRenderTarget2D'/VolumetricClouds/Textures/RT_GroundShadows.RT_GroundShadows'"));

	//If texture exist store it into actor variable.
	if (ShadowsRenderTargetAsset.Object != NULL)
	{
		ShadowRenderTarget = ShadowsRenderTargetAsset.Object;
	}

	//Find material instance for directional light light function.
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant>LightFunctionMaterialInstanceAsset(TEXT("MaterialInstanceConstant'/VolumetricClouds/Materials/LFMI_GroundShadows.LFMI_GroundShadows'"));

	//If material exist store it into actor variable.
	if (LightFunctionMaterialInstanceAsset.Object != NULL)
	{
		LightFunctionMaterialInstance = LightFunctionMaterialInstanceAsset.Object;
	}
#endif

	//Find find and load weather map texture.
	static ConstructorHelpers::FObjectFinder<UTexture2D>WeatherMapTexureAsset(TEXT("Texture2D'/VolumetricClouds/Textures/T_Weather_MA.T_Weather_MA'"));

	//If rexture exist store it into actor variable.
	if (WeatherMapTexureAsset.Object != NULL)
	{
		WeatherMapTexure = WeatherMapTexureAsset.Object;
	}

}



/** Event that starts when level loaded. */
void AVC_Clouds::PostLoad()
{
	Super::PostLoad();

	//Create dynamic materials if needed. And load material parameters.
	CreateDynamicMaterials();
	LoadMaterialParameters();
}

/** Event that starts when actor spawned. */
void AVC_Clouds::PostActorCreated()
{
	Super::PostActorCreated();

	//Create dynamic materials if needed. And load material parameters.
	CreateDynamicMaterials();
	LoadMaterialParameters();
}

/** Create dynamic material instances. */
void AVC_Clouds::CreateDynamicMaterials()
{
	if (GetWorld() != NULL)
	{
		//If first time loaded world in game create dynamic material instances is needed.
		if (IsDynamicInGame() && GetWorld()->IsGameWorld())
		{
			if (CloudsMaterialInstance != NULL)
			{
				CloudsMaterialInstanceDynamic = UMaterialInstanceDynamic::Create(CloudsMaterialInstance, this);
				CloudsBoxMesh->SetMaterial(0, CloudsMaterialInstanceDynamic);
			}

			if (ShadowRenderMaterialInstance != NULL)
			{
				ShadowRenderMaterialInstanceDynamic = UMaterialInstanceDynamic::Create(ShadowRenderMaterialInstance, this);
			}

			if (LightFunctionMaterialInstance != NULL)
			{
				LightFunctionMaterialInstanceDynamic = UMaterialInstanceDynamic::Create(LightFunctionMaterialInstance, this);
				if (DirectionalLightActor != NULL)
				{
					DirectionalLightActor->SetLightFunctionMaterial(LightFunctionMaterialInstanceDynamic);
				}
			}
		}
	}
}

/** Set all material variables to materials. */
void AVC_Clouds::LoadMaterialParameters()
{
	if (GetWorld() != NULL)
	{
		for (TFieldIterator<UProperty>PropIt(GetClass()); PropIt; ++PropIt)
		{
			UProperty* Property = *PropIt;
			FName PropertyName = FName(*Property->GetName());

			if (ArrayMaterialsParameterNames.Contains(PropertyName))
			{
				UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property);

				if (FloatProperty != nullptr)
				{
					float Value = FloatProperty->GetPropertyValue_InContainer(this);
					SetMaterialScalar(PropertyName, Value);
				}


				UStructProperty* StructProperty = Cast<UStructProperty>(Property);

				if (StructProperty != nullptr)
				{
					const FLinearColor* ColorPtr = Property->ContainerPtrToValuePtr<FLinearColor>(this);
					if (ColorPtr != nullptr)
					{
						FLinearColor Value = *ColorPtr;
						SetMaterialColor(PropertyName, Value);
					}
				}
			}
		}

		//Setup a few more functions that updates transform and light direction.
		UpdateLightDirection();
		UpdateTransform();

		//Setup tesutes.
		SetMaterialTexture(FName("WeatherMapTexure"), WeatherMapTexure);
	}
}

/** Setup material instance scalar value.
@Name - material parameter name.
@Value - material parameter Value.
*/
void AVC_Clouds::SetMaterialScalar(FName Name, float Value)
{
	if (GetWorld()->IsGameWorld())
	{
		if (IsDynamicInGame())
		{
			if (CloudsMaterialInstanceDynamic != NULL)
			{
				CloudsMaterialInstanceDynamic->SetScalarParameterValue(Name, Value);
			}
			if (ShadowRenderMaterialInstanceDynamic != NULL && LightFunctionMaterialInstanceDynamic != NULL)
			{
				ShadowRenderMaterialInstanceDynamic->SetScalarParameterValue(Name, Value);
				LightFunctionMaterialInstanceDynamic->SetScalarParameterValue(Name, Value);
			}
		}
	}
#if WITH_EDITOR
	else
	{
		if (CloudsMaterialInstance != NULL)
		{
			CloudsMaterialInstance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
		}
		if (ShadowRenderMaterialInstance != NULL && LightFunctionMaterialInstance != NULL)
		{
			ShadowRenderMaterialInstance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
			LightFunctionMaterialInstance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
		}
	}
#endif
}

/** Setup material instance color vector value.
@Name - material parameter name.
@Value - material parameter Value.
*/
void AVC_Clouds::SetMaterialColor(FName Name, FLinearColor Value)
{
	if (GetWorld()->IsGameWorld())
	{
		if (IsDynamicInGame())
		{
			if (CloudsMaterialInstanceDynamic != NULL)
			{
				CloudsMaterialInstanceDynamic->SetVectorParameterValue(Name, Value);
			}
			if (ShadowRenderMaterialInstanceDynamic != NULL && LightFunctionMaterialInstanceDynamic != NULL)
			{
				ShadowRenderMaterialInstanceDynamic->SetVectorParameterValue(Name, Value);
				LightFunctionMaterialInstanceDynamic->SetVectorParameterValue(Name, Value);
			}
		}
	}
#if WITH_EDITOR
	else
	{
		if (CloudsMaterialInstance != NULL)
		{
			CloudsMaterialInstance->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
		}
		if (ShadowRenderMaterialInstance != NULL && LightFunctionMaterialInstance != NULL)
		{
			
			ShadowRenderMaterialInstance->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
			LightFunctionMaterialInstance->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
		}
	}
#endif
}

/** Setup material instance color vector value.
@Name - material parameter name.
@Value - material parameter Value.
*/
void AVC_Clouds::SetMaterialTexture(FName Name, UTexture2D* Value)
{
	if (GetWorld()->IsGameWorld())
	{
		if (IsDynamicInGame())
		{
			if (CloudsMaterialInstanceDynamic != NULL)
			{
				CloudsMaterialInstanceDynamic->SetTextureParameterValue(Name, Value);
			}
			if (ShadowRenderMaterialInstanceDynamic != NULL)
			{
				ShadowRenderMaterialInstanceDynamic->SetTextureParameterValue(Name, Value);
			}
		}
	}
#if WITH_EDITOR
	else
	{
		if (CloudsMaterialInstance != NULL)
		{
			CloudsMaterialInstance->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
		}
		if (ShadowRenderMaterialInstance != NULL)
		{
			ShadowRenderMaterialInstance->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(Name), Value);
		}
	}
#endif
}

/** Updates light direction for a sun light */
void AVC_Clouds::UpdateLightDirection()
{
	//If actor exist.
	if (DirectionalLightActor != NULL)
	{
		//Find it's light direction.
		FVector LightDirection = DirectionalLightActor->GetActorForwardVector() * -1;

		//Setup light direction parameter.
		SetMaterialColor(FName("LightDirection"), FLinearColor(LightDirection));

		//If we are in game set dynamic light function to a light.
		if (GetWorld()->IsGameWorld())
		{
			if (IsDynamicInGame())
			{
				if (DirectionalLightActor->GetLightComponent()->LightFunctionMaterial != LightFunctionMaterialInstanceDynamic && LightFunctionMaterialInstanceDynamic != NULL)
				{
					DirectionalLightActor->SetLightFunctionMaterial(LightFunctionMaterialInstanceDynamic);
				}
			}
		}
		else //If we are not game set static light function to a light.
		{
			if (DirectionalLightActor->GetLightComponent()->LightFunctionMaterial != LightFunctionMaterialInstance && LightFunctionMaterialInstance != NULL)
			{
				DirectionalLightActor->SetLightFunctionMaterial(LightFunctionMaterialInstance);
			}
		}
	}
	else
	{

	}
}

/** Updates actor transform to a material parameters. */
void AVC_Clouds::UpdateTransform()
{
	//Get transform of actor.
	FVector Location = this->GetTransform().GetLocation();
	FVector Scale = this->GetActorScale3D();

	SetMaterialColor(FName("ActorPosition"), FLinearColor(Location));
	SetMaterialColor(FName("ActorScale"), FLinearColor(Scale));
}

/** Event that called when actor construction changed.
@Transform - new transform.
*/
void AVC_Clouds::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
	UpdateTransform();
}

/* Render clouds shadows into texture. */
void AVC_Clouds::RenderShadows()
{
	//If shadow render target exist.
	if (GetWorld() != NULL && ShadowRenderTarget != NULL && ShadowIntensity != 0.0f)
	{
		//If in game set dynamic material instance and render it.
		if (GetWorld()->IsGameWorld())
		{
			if (IsDynamicInGame())
			{
				if (ShadowRenderMaterialInstanceDynamic != NULL)
				{
					UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), ShadowRenderTarget, ShadowRenderMaterialInstanceDynamic);
				}
			}
		}
		else //If in editor set static material instance and render it.
		{
			if (ShadowRenderMaterialInstance != NULL)
			{
				UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), ShadowRenderTarget, ShadowRenderMaterialInstance);
			}
		}
	}
	else
	{

	}
}

// Called every frame
void AVC_Clouds::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//Custom time parameter.
	Time += DeltaTime;

	//Update material instance time.
	SetMaterialScalar(FName("Time"), Time);

	//Update light direction every frame.
	//TODO: Terrible solution. FIX it later.
	UpdateLightDirection();

	//Render shadows.
	RenderShadows();
	
}

#if WITH_EDITOR
/** Event that called when editor changes property. */
void AVC_Clouds::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);
	 
	//Get edited property name.
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

	//When new directional light actor setups, automaticly update material light direction.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AVC_Clouds, DirectionalLightActor))
	{
		UpdateLightDirection();

		//Set default light function fade distance.
		if(DirectionalLightActor != NULL)
		{
			DirectionalLightActor->SetLightFunctionFadeDistance(GetActorScale().X * 50.0f);
		}
	}


	UFloatProperty* PropertyFloat = ((UFloatProperty*)e.Property);
	if (PropertyFloat)
	{
		//TODO: Convert basic struct property to an actual property.
		if (PropertyName == FName("R") || PropertyName == FName("G") || PropertyName == FName("B"))
		{
			SetMaterialColor(FName("LightColor"), FLinearColor(LightColor));
			SetMaterialColor(FName("ShadowColor"), FLinearColor(ShadowColor));
		}
		else
		{
			float Value = PropertyFloat->GetPropertyValue_InContainer(this);
			SetMaterialScalar(PropertyName, Value);
		}
	}

	UStructProperty* StructProperty = ((UStructProperty*)e.Property);
	if (StructProperty)
	{
		//TODO: Convert basic struct property to an actual property.
		SetMaterialColor(FName("LightColor"), FLinearColor(LightColor));
		SetMaterialColor(FName("ShadowColor"), FLinearColor(ShadowColor));
	}

	//When new directional light actor setups, automaticly update material light direction.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AVC_Clouds, WeatherMapTexure))
	{
		SetMaterialTexture(PropertyName, WeatherMapTexure);
	}
	
}

/** Should actor tick in editor. */
bool AVC_Clouds::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif // WITH_EDITOR

