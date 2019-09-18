// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Runtime/Engine/Classes/Materials/MaterialInstanceConstant.h"
#include "Runtime/Engine/Classes/Engine/DirectionalLight.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget.h"

#include "VC_Clouds.generated.h"

UCLASS()
class VOLUMETRICCLOUDS_API AVC_Clouds : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Sets default values for this actor's properties. */
	AVC_Clouds(const FObjectInitializer& ObjectInitializer);
	
	/** Event that starts when level loaded. */
	virtual void PostLoad() override;

	/** Event that starts when actor spawned. */
	virtual void PostActorCreated() override;

private:
	/** Create dynamic material instances. */
	void CreateDynamicMaterials();

	/** Set all material variables to materials. */
	void LoadMaterialParameters();


	/** Setup material instance scalar value.
	@Name - material parameter name.
	@Value - material parameter Value.
	*/
	void SetMaterialScalar(FName Name, float Value);

	/** Setup material instance color vector value.
	@Name - material parameter name.
	@Value - material parameter Value.
	*/
	void SetMaterialColor(FName Name, FLinearColor Value);

	/** Setup material instance color vector value.
	@Name - material parameter name.
	@Value - material parameter Value.
	*/
	void SetMaterialTexture(FName Name, UTexture2D* Value);

	/** Updates light direction for a sun light */
	//Temporal: Update light position every frame. as a private function instead of public blueprintable.
	//UFUNCTION(CallInEditor, Category = "Clouds", DisplayName = "Update Light Direction")
	void UpdateLightDirection();

	/** Updates actor transform to a material parameters. */
	void UpdateTransform();

	/** Event that called when actor construction changed.
	@Transform - new transform.
	*/
	virtual void OnConstruction(const FTransform & Transform) override;

	/** Render clouds shadows material into texture. */
	void RenderShadows();

public:
	/** Is this actor uses dynamic materials in game. */
	bool IsDynamicInGame() { return bUseDynamicMaterials; };

	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

private:
#if WITH_EDITORONLY_DATA
	/** A UBillboardComponent to hold Icon sprite. */
	UPROPERTY()
	UBillboardComponent* SpriteComponent;
#endif

	/** Icon sprite testure. */
	UTexture2D* SpriteTexture;

	/* Static mesh component for a clouds box mesh. */
	UPROPERTY()
	UStaticMeshComponent* CloudsBoxMesh;
	/** Shadow render target texture. */
	UPROPERTY()
	UTextureRenderTarget2D* ShadowRenderTarget;

	/** 
	Editor material instances.
	*/
	/** Material instance for clouds box mesh. */
	UPROPERTY()
	UMaterialInstanceConstant* CloudsMaterialInstance;
	/** Material instance for clouds shadow render target. */
	UPROPERTY()
	UMaterialInstanceConstant* ShadowRenderMaterialInstance;
	/** Light function material instance. */
	UPROPERTY()
	UMaterialInstanceConstant* LightFunctionMaterialInstance;

	/**
	Game material instances.
	*/
	/** Material instance for clouds box mesh. */
	UPROPERTY()
	UMaterialInstanceDynamic* CloudsMaterialInstanceDynamic;
	/** Material instance for clouds shadow render target. */
	UPROPERTY()
	UMaterialInstanceDynamic* ShadowRenderMaterialInstanceDynamic;
	/** Light function material instance. */
	UPROPERTY()
	UMaterialInstanceDynamic* LightFunctionMaterialInstanceDynamic;

	/** Array of avalable clouds material parameter names. //38 */
	const TArray<FName> ArrayMaterialsParameterNames
	{	
		FName("AtmosphereBlendDistance"), 
		FName("AtmosphereBlendIntensity"),
		FName("AttenClampIntensity"),
		FName("BeerLawDensity"),
		FName("CloudOutScatterAmbient"),
		FName("Coverage"),
		FName("Density"),
		FName("InOutScatterLerp"),
		FName("InScatter"),
		FName("InScatterIntensity"),
		FName("LightColor"),
		FName("LightColorIntensity"),
		FName("LightIntensity"),
		FName("LightPow"),
		FName("LightStepScale"),
		FName("MipMapScaleDistance"),
		FName("NoiseTile"),
		FName("SmallNoiseTile"),
		FName("OutScatter"),
		FName("RayMaxSteps"),
		FName("ShadowColor"),
		FName("ShadowMaxSteps"),
		FName("SilverLightExp"),
		FName("SilverLightIntensity"),
		FName("StepScaleDistance"),
		FName("WeatherMapTile"),
		FName("WindDirectionX"),
		FName("WindDirectionY"),
		FName("WindDirectionZ"),
		FName("WindSpeed"),
		FName("ActorPosition"),
		FName("ActorScale"),
		FName("LightDirection"),
		FName("Time"),
		FName("ShadowBlur"),
		FName("ShadowDensity"),
		FName("ShadowIntensity")
		//LightDirection
		//ActorPos
		//ActorScale
	};

public:
	/** Directional light actor for a material light direction paramater. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds", DisplayName = "Directional Light Actor")
		ADirectionalLight* DirectionalLightActor;

	/** Cloud sun light color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds", DisplayName = "Light Color")
		FLinearColor LightColor = FLinearColor(1.14f, 1.22f, 1.31f, 1.0f);
	/** Cloud shadow color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds", DisplayName = "Shadow Color")
		FLinearColor ShadowColor = FLinearColor(0.60f, 0.63f, 0.7f, 1.0f);

	/** Distance to atmosophere blend. (in km)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Atmosphere", DisplayName = "Blend Disntace", meta = (ClampMin = "0.0", ClampMax = "20.0", UIMin = "0.0", UIMax = "20.0"))
		float AtmosphereBlendDistance = 5.0f;
	/** Intensity of the atmosphere fog blend. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Atmosphere", DisplayName = "Blend Intensity", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
		float AtmosphereBlendIntensity = 1.0f;

	/** Cloud shadow intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Shadow Density", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
		float BeerLawDensity = 0.85f;
	/** Cloud shadow clamp intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Attenusation Clamp", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
		float AttenClampIntensity = 0.6f;
	/** Cloud ambient ntensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Ambient Intensity", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float CloudOutScatterAmbient = 1.0f;
	/** Cloud inscattering exponent for a light. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "In Scatter", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float InScatter = 0.75f;
	/** Cloud inscattering intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "In Scatter Instensity", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float InScatterIntensity = 0.05f;
	/** Cloud silverlight exponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Silver Light Exp", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SilverLightExp = 1.0f;
	/** Cloud silverlight intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Silver Light Intensity", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SilverLightIntensity = 0.5f;
	/** Cloud outnscattering exponent for a light. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Out Scatter", meta = (ClampMin = "-1.0", ClampMax = "0.0", UIMin = "-1.0", UIMax = "0.0"))
		float OutScatter = -0.045f;
	/** Cloind interpolation between inscatter and outscatter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "InOutScatterLerp", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float InOutScatterLerp = 0.5;
	/** Cloind light color intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Light Color Intensity", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
		float LightColorIntensity = 1.25f;
	/** Cloind light intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Light Intensity", meta = (ClampMin = "0.0", ClampMax = "6.0", UIMin = "0.0", UIMax = "6.0"))
		float LightIntensity = 4.0f;
	/** Cloind light power. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Light", DisplayName = "Light Power", meta = (ClampMin = "0.0", ClampMax = "5.0", UIMin = "0.0", UIMax = "5.0"))
		float LightPow = 3.0f;

	/** Cloud coverage of the sky. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Weather", DisplayName = "Coverage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float Coverage = 0.65f;
	/** Cloud density. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Weather", DisplayName = "Density", meta = (ClampMin = "0.0", ClampMax = "20.0", UIMin = "0.0", UIMax = "20.0"))
		float Density = 14.0f;
	/** Tile for a weather map texture. (in UV) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Weather", DisplayName = "Map Tile", meta = (ClampMin = "0.0", ClampMax = "14.0", UIMin = "0.0", UIMax = "14.0"))
		float WeatherMapTile = 1.5f;
	/** Weather map texture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Weather", DisplayName = "Weather Map")
		UTexture2D* WeatherMapTexure;

	/** Maximum amout of ray steps for a raymarch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ray", DisplayName = "View Max Steps", meta = (ClampMin = "1.0", ClampMax = "4000.0", UIMin = "1.0", UIMax = "4000.0"))
		float RayMaxSteps = 500.0f;
	/** Distance when ray start make less steps. (in km) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ray", DisplayName = "View Step Scale Distance", meta = (ClampMin = "1.0", ClampMax = "4000.0", UIMin = "1.0", UIMax = "4000.0"))
		float StepScaleDistance = 2.0f;
	/** Max amount of shadow ray steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ray", DisplayName = "Shadow Max Steps", meta = (ClampMin = "1.0", ClampMax = "32.0", UIMin = "1.0", UIMax = "32.0"))
		float ShadowMaxSteps = 8.0f;
	/** Scale for a ray step offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ray", DisplayName = "Shadow Step Scale", meta = (ClampMin = "0.0", ClampMax = "4.0", UIMin = "0.0", UIMax = "4.0"))
		float LightStepScale = 2.0f;

	/** Wind direction X value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Wind", DisplayName = "X Direction", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
		float WindDirectionX = 0.25f;
	/** Wind direction Y value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Wind", DisplayName = "Y Direction", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
		float WindDirectionY = 0.0f;
	/** Wind direction Z value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Wind", DisplayName = "Z Direction", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
		float WindDirectionZ = -0.05f;
	/** Global wind speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Wind", DisplayName = "Speed", meta = (UIMin = "0.0", UIMax = "20.0"))
		float WindSpeed = 12.0f;

	/** Base noise tile value. (in UV by Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Noise", DisplayName = "Noise Tile", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
		float NoiseTile = 20.0f;
	/** Detail noise tile value. (in UV by Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Noise", DisplayName = "Detail Noise Tile", meta = (ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
		float SmallNoiseTile = 230.0f;

	/** Ground shadow blur intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ground Shadow", DisplayName = "Blur", meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
		float ShadowBlur = 0.25f;
	/** Ground shadow density and edge hardness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ground Shadow", DisplayName = "Density", meta = (ClampMin = "0.0", ClampMax = "4.0", UIMin = "0.0", UIMax = "4.0"))
		float ShadowDensity = 1.0f;
	/** Ground shadow global intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Ground Shadow", DisplayName = "Intensity", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float ShadowIntensity = 1.0f;

	/** Distance when sampler starts use lower mip maps for a noise textures. (in km) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Misc", DisplayName = "Mip Map Scale Disntace", meta = (ClampMin = "0.0", ClampMax = "20.0", UIMin = "0.0", UIMax = "20.0"))
		float MipMapScaleDistance = 5.0f;
	/** Is game should use dynamic material instances for game. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds|Misc", DisplayName = "Use Dynamic Materials")
		bool bUseDynamicMaterials = true;

	UPROPERTY(VisibleAnywhere, Category = "Clouds|Info")
		FString Desription = FString("Beta version of volumatric clouds.\nStill work in progress.\nHave major optimisation issues.\nPlease stay tuned for a future updates.");

private:
	/** Shader time.
	//TODO: Check is it real that each shader has separate time parameter. So it's impossible to synchronize two shaders.
	*/
	UPROPERTY()
	float Time = 0.0f;




#if WITH_EDITOR
private:
	/** Event that called when editor changes property. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	/** Should actor tick in editor. */
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif // WITH_EDITOR

	//Level load test

};
