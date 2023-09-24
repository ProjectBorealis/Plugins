#include "BlastMeshThumbnailRenderer.h"
#include "BlastMesh.h"
#include "BlastMeshComponent.h"
#include "BlastExtendedSupport.h"

#include "ShowFlags.h"
#include "SceneView.h"
#include "RendererInterface.h"
#include "EngineModule.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

#include "LegacyScreenPercentageDriver.h"

bool UBlastMeshThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	//Don't try with UBlastMeshExtendedSupport since there is no render data
	if (!Object->IsA<UBlastMeshExtendedSupport>())
	{
		return false;
	}
	//Timing issue during import/build
	UBlastMesh* BlastMesh = Cast<UBlastMesh>(Object);
	if (!BlastMesh->Mesh->GetResourceForRendering())
	{
		return false;
	}
	return true;
}

void UBlastMeshThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	//Just delegate to the skeletal mesh one
	UBlastMesh* BlastMesh = Cast<UBlastMesh>(Object);
	if (BlastMesh != nullptr)
	{
		if (!ThumbnailScene.IsValid())
		{
			ThumbnailScene = TSharedPtr<FBlastMeshThumbnailScene>(new FBlastMeshThumbnailScene());
		}

		ThumbnailScene->SetBlastMesh(BlastMesh);
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(Viewport, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetTime(FGameTime::CreateUndilated(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime())));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
			ViewFamily, /* GlobalResolutionFraction = */ 1.0f, /* AllowPostProcessSettingsScreenPercentage = */ false));

		RenderViewFamily(Canvas, &ViewFamily, ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height));
		ThumbnailScene->SetBlastMesh(nullptr);
	}
}

void UBlastMeshThumbnailRenderer::BeginDestroy()
{
	Super::BeginDestroy();
	//Thise needs to be done before the real destructor
	ThumbnailScene.Reset();
}

FBlastMeshThumbnailScene::FBlastMeshThumbnailScene()
{
	bForceAllUsedMipsResident = false;
	// Create preview actor
	// checked
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bNoFail = true;
	SpawnInfo.ObjectFlags = RF_Transient;
	AActor* PreviewActor = GetWorld()->SpawnActor<AActor>(SpawnInfo);

	PreviewActor->SetActorEnableCollision(false);

	PreviewComponent = NewObject<UBlastMeshComponent>(PreviewActor);
	//Make sure we are rendering in that one frame we have a chance to
	PreviewComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	PreviewActor->SetRootComponent(PreviewComponent);
	PreviewComponent->RegisterComponent();
}

void FBlastMeshThumbnailScene::SetBlastMesh(class UBlastMesh* InBlastMesh)
{
	PreviewComponent->SetBlastMesh(InBlastMesh);
	PreviewComponent->RefreshBoneTransforms();
	PreviewComponent->UpdateBounds();
}

void FBlastMeshThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const
{
	check(PreviewComponent->GetBlastMesh());

	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;
	// No need to add extra size to view slightly outside of the sphere to compensate for perspective since skeletal meshes already buffer bounds.
	const float HalfMeshSize = PreviewComponent->Bounds.SphereRadius;
	const float BoundsZOffset = GetBoundsZOffset(PreviewComponent->Bounds);
	const float TargetDistance = HalfMeshSize / FMath::Tan(HalfFOVRadians);

	//Use the thumbnail info on the inner skeletal mesh since there is already one there.
	USceneThumbnailInfo* ThumbnailInfo = nullptr;
	if (PreviewComponent->GetBlastMesh()->Mesh != nullptr)
	{
		ThumbnailInfo = Cast<USceneThumbnailInfo>(PreviewComponent->GetBlastMesh()->Mesh->GetThumbnailInfo());
		if (ThumbnailInfo)
		{
			if (TargetDistance + ThumbnailInfo->OrbitZoom < 0)
			{
				ThumbnailInfo->OrbitZoom = -TargetDistance;
			}
		}
	}
	if (ThumbnailInfo == nullptr)
	{
		ThumbnailInfo = USceneThumbnailInfo::StaticClass()->GetDefaultObject<USceneThumbnailInfo>();
	}

	OutOrigin = FVector(0, 0, -BoundsZOffset);
	OutOrbitPitch = ThumbnailInfo->OrbitPitch;
	OutOrbitYaw = ThumbnailInfo->OrbitYaw;
	OutOrbitZoom = TargetDistance + ThumbnailInfo->OrbitZoom;
}
