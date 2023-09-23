#pragma once

#include "CoreMinimal.h"

#include "UnrealEd/Public/ThumbnailHelpers.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"

#include "BlastMeshThumbnailRenderer.generated.h"

class FCanvas;
class FRenderTarget;


class FBlastMeshThumbnailScene : public FThumbnailPreviewScene
{
public:
	/** Constructor */
	FBlastMeshThumbnailScene();

	void SetBlastMesh(class UBlastMesh* InBlastMesh);

protected:
	// FThumbnailPreviewScene implementation
	virtual void GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const override;

private:
	class UBlastMeshComponent* PreviewComponent;
};

UCLASS(MinimalAPI)
class UBlastMeshThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	// UThumbnailRenderer interface
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	// End of UThumbnailRenderer interface

	virtual void BeginDestroy() override;
private:
	TSharedPtr<FBlastMeshThumbnailScene> ThumbnailScene;
};
