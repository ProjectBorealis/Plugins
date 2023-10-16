#include "BlastEditorModule.h"

#include "Interfaces/IPluginManager.h"
#include "LevelEditor.h"
#include "EditorBuildUtils.h"
#include "DrawDebugHelpers.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "PropertyEditorModule.h"
#include "AssetToolsModule.h"
#include "EngineUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "UObject/UObjectIterator.h"
#include "ComponentReregisterContext.h"
#include "SkeletalMeshTypes.h"
#include "Animation/Skeleton.h"
#include "Engine/SkinnedAssetCommon.h"

#include "NvBlastExtAssetUtils.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastGlobals.h"
#include "NvBlast.h"

#include "BlastGlobals.h"
#include "BlastGlueVolume.h"
#include "BlastExtendedSupport.h"
#include "BlastUICommands.h"
#include "BlastMeshThumbnailRenderer.h"
#include "BlastMeshComponentDetails.h"
#include "BlastMeshComponent.h"
#include "BlastMeshFactory.h"
#include "AssetTypeActions_BlastMesh.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Editor.h"
#include "BlastMeshActor.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/PackageName.h"
#include "Widgets/Input/SCheckBox.h"
#include "GPUSkinVertexFactory.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshModel.h"

#include "PhysXPublic.h"

IMPLEMENT_MODULE(FBlastEditorModule, BlastEditor);
DEFINE_LOG_CATEGORY(LogBlastEditor);

#define LOCTEXT_NAMESPACE "Blast"

FBlastEditorModule::FBlastEditorModule()
{

}

void FBlastEditorModule::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	BlastMeshAssetTypeActions = MakeShareable(new FAssetTypeActions_BlastMesh);
	AssetTools.RegisterAssetTypeActions(BlastMeshAssetTypeActions.ToSharedRef());

	OnScreenMessageHandle = FCoreDelegates::OnGetOnScreenMessages.AddRaw(this, &FBlastEditorModule::HandleGetOnScreenMessages);
	RefreshPhysicsAssetHandle = UPhysicsAsset::OnRefreshPhysicsAssetChange.AddRaw(this, &FBlastEditorModule::HandleRefreshPhysicsAssetChange);

	FEditorBuildUtils::RegisterCustomBuildType(BlastBuildStepId, FDoEditorBuildDelegate::CreateRaw(this, &FBlastEditorModule::DoBlastBuild), FBuildOptions::BuildGeometry);

	BindCommands();

	BuildMenuExtender = FLevelEditorModule::FLevelEditorMenuExtender::CreateLambda([this](const TSharedRef<FUICommandList> EditorCommandList)
		{
			TSharedRef<FExtender> Ret = MakeShared<FExtender>();
			Ret->AddMenuExtension("LevelEditorGeometry", EExtensionHook::Before, CommandList, FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& InMenuBuilder)
				{
					InMenuBuilder.BeginSection("Blast", LOCTEXT("BuildBlast", "Build Blast"));
					InMenuBuilder.AddMenuEntry(FBlastUICommands::Get().BuildBlast);
					InMenuBuilder.EndSection();
				}));
			return Ret;
		});

	ActorMenuExtender = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateLambda([this](const TSharedRef<FUICommandList> EditorCommandList, const TArray<AActor*>& Actors)
		{
			TSharedRef<FExtender> Ret = MakeShared<FExtender>();
			TArray<AActor*> FilteredActors = GetActorsWithBlastComponents(Actors);
			if (FilteredActors.Num() > 0)
			{
				Ret->AddMenuExtension("ActorAsset", EExtensionHook::Before, CommandList, FMenuExtensionDelegate::CreateLambda([this, FilteredActors](FMenuBuilder& InMenuBuilder)
					{
						InMenuBuilder.BeginSection("Blast", LOCTEXT("Blast", "Blast"));
						PopulateBlastMenuForActors(InMenuBuilder, FilteredActors);
						InMenuBuilder.EndSection();
					}));
			}
			return Ret;
		});

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Add(BuildMenuExtender);
	LevelEditorModule.GetAllLevelViewportContextMenuExtenders().Add(ActorMenuExtender);

	UThumbnailManager::Get().RegisterCustomRenderer(UBlastMesh::StaticClass(), UBlastMeshThumbnailRenderer::StaticClass());

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UBlastMeshComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FBlastMeshComponentDetails::MakeInstance));

}

void FBlastEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		if (BlastMeshAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(BlastMeshAssetTypeActions.ToSharedRef());
			BlastMeshAssetTypeActions.Reset();
		}
	}

	if (OnScreenMessageHandle.IsValid())
	{
		FCoreDelegates::OnGetOnScreenMessages.Remove(OnScreenMessageHandle);
		OnScreenMessageHandle.Reset();
	}

	if (RefreshPhysicsAssetHandle.IsValid())
	{
		UPhysicsAsset::OnRefreshPhysicsAssetChange.Remove(RefreshPhysicsAssetHandle);
		RefreshPhysicsAssetHandle.Reset();
	}

	FEditorBuildUtils::UnregisterCustomBuildType(BlastBuildStepId);

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().RemoveAll([this](const FLevelEditorModule::FLevelEditorMenuExtender& Extender) { return Extender.GetHandle() == BuildMenuExtender.GetHandle(); });
	BuildMenuExtender.Unbind();

	LevelEditorModule.GetAllLevelViewportContextMenuExtenders().RemoveAll([this](const FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors& Extender) { return Extender.GetHandle() == ActorMenuExtender.GetHandle(); });
	ActorMenuExtender.Unbind();

	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UBlastMesh::StaticClass());
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UBlastMeshComponent::StaticClass()->GetFName());
	}
}

void FBlastEditorModule::BindCommands()
{
	FBlastUICommands::Register();
	CommandList = MakeShareable(new FUICommandList());
	CommandList->MapAction(FBlastUICommands::Get().BuildBlast, FExecuteAction::CreateLambda([]
		{
			FEditorBuildUtils::EditorBuild(GEditor->GetEditorWorldContext().World(), FBlastEditorModule::BlastBuildStepId);
		}));
}

void FBlastEditorModule::HandleGetOnScreenMessages(FCoreDelegates::FSeverityMessageMap& OutMessages)
{
	UBlastGlueWorldTag* Tag = UBlastGlueWorldTag::GetForWorld(GEditor->GetEditorWorldContext().World());
	if (Tag && Tag->bIsDirty)
	{
		OutMessages.Add(FCoreDelegates::EOnScreenMessageSeverity::Warning, FText::FromString("Blast Build needed!"));
	}
}

void FBlastEditorModule::HandleRefreshPhysicsAssetChange(const class UPhysicsAsset* Asset)
{
	for (TObjectIterator<UBlastMesh> Iter; Iter; ++Iter)
	{
		if (Iter->PhysicsAsset == Asset)
		{
			Iter->RebuildCookedBodySetupsIfRequired(true);
		}
	}

	TArray<UActorComponent*> ComponentsUsing;
	for (TObjectIterator<UBlastMeshComponent> Iter; Iter; ++Iter)
	{
		UBlastMesh* Mesh = Iter->GetBlastMesh();
		if (Mesh && Mesh->PhysicsAsset == Asset)
		{
			ComponentsUsing.Add(*Iter);
		}
	}

	FMultiComponentReregisterContext RegregContext(ComponentsUsing);
}

TArray<AActor*> FBlastEditorModule::GetActorsWithBlastComponents(const TArray<AActor*>& Actors)
{
	TArray<AActor*> Ret;
	for (AActor* Actor : Actors)
	{
		if (Actor && Actor->FindComponentByClass<UBlastMeshComponent>())
		{
			Ret.Add(Actor);
		}
	}
	return Ret;
}

namespace
{
	struct FTempCollisionHull : public Nv::Blast::CollisionHull
	{
		TArray<NvcVec3> Points;
		TArray<uint32> Indices;
		TArray<Nv::Blast::HullPolygon> Polygons;

		void SetPointers()
		{
			pointsCount = Points.Num();
			points = Points.GetData();

			indicesCount = Indices.Num();
			indices = Indices.GetData();

			polygonDataCount = Polygons.Num();
			polygonData = Polygons.GetData();
		}

		void release()
		{ //do nothing
		}
	};
}

class AssetUnionTool
{
public:

	static void CreateInPackageAsset(const TArray<AActor*>& Actors, float distance, bool uniteMaterials)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FScopedSlowTask SlowTask(1, LOCTEXT("CreateUnionAsset", "Creating united asset"));
		TArray<UObject*> ObjectsToSync;

		SlowTask.EnterProgressFrame();
		FString Name;
		FString PackageName;

		const FString DefaultSuffix = TEXT("_UNION");


		UBlastMeshComponent* bMesh0 = static_cast<UBlastMeshComponent*> (Actors[0]->GetRootComponent());
		if (bMesh0 == nullptr) return;

		AssetToolsModule.Get().CreateUniqueAssetName(bMesh0->GetBlastMesh()->GetOutermost()->GetName(), DefaultSuffix, /*out*/ PackageName, /*out*/ Name);
		const FString PackagePath = FPackageName::GetLongPackagePath(FString("/") + PackageName);
		FVector centroid;

		if (UBlastMesh* NewAsset = Cast<UBlastMesh>(AssetToolsModule.Get().CreateAsset(Name, PackagePath, UBlastMesh::StaticClass(), nullptr)))
		{
			FillNewAsset(NewAsset, Actors, distance, centroid, uniteMaterials);

			ObjectsToSync.Add(NewAsset);

			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
			UWorld* world = Actors[0]->GetWorld();
			FActorSpawnParameters spawnPm;
			spawnPm.bNoFail = true;
			ABlastMeshActor* actor = world->SpawnActor<ABlastMeshActor>(centroid, FRotator::ZeroRotator, spawnPm);
			actor->GetBlastMeshComponent()->SetBlastMesh(NewAsset);
			for (auto& act : Actors)
			{
				world->DestroyActor(act);
			}
		}

	}

	static void FillNewAsset(UBlastMesh* nasset, const TArray<AActor*>& Actors, float distance, FVector& centroid, bool uniteMaterials)
	{
		struct ChMapping
		{
			int32 cmp, ch;
		};

		FVector essCentroid(0.f, 0.f, 0.f);
		TArray<UBlastMeshComponent*> meshes;
		{
			FVector minBox;
			FVector maxBox;
			for (int32 i = 0; i < Actors.Num(); ++i)
			{
				UBlastMeshComponent* comp = static_cast<UBlastMeshComponent*>(Actors[i]->GetRootComponent());
				if (i == 0)
				{
					minBox = comp->Bounds.GetBox().Min;
					maxBox = comp->Bounds.GetBox().Max;
				}
				else
				{
					minBox = minBox.ComponentMin(comp->Bounds.GetBox().Min);
					maxBox = maxBox.ComponentMax(comp->Bounds.GetBox().Max);
				}
				meshes.Add(comp);
			}
			essCentroid = (minBox + maxBox) * 0.5f;
		}
		centroid = essCentroid;

		TArray<NvcQuat> AssetRotations;
		TArray<NvcVec3> AssetLocations;
		TArray<NvcVec3> AssetScales;
		TArray<FTransform3f> ComponentTransforms;

		ComponentTransforms.SetNumUninitialized(meshes.Num());

		TArray<const NvBlastAsset*> AssetList;
		TArray<TArray<uint32> > PerComponentHullRanges;
		TArray<TArray<FTempCollisionHull> > PerComponentHullLists;
		TArray<TArray<const Nv::Blast::CollisionHull*> > PerComponentHullPtrLists;


		AssetRotations.SetNumUninitialized(meshes.Num());
		AssetLocations.SetNumUninitialized(meshes.Num());
		AssetScales.SetNumUninitialized(meshes.Num());
		AssetList.SetNumUninitialized(meshes.Num());
		PerComponentHullRanges.SetNum(meshes.Num());
		PerComponentHullLists.SetNum(meshes.Num());
		PerComponentHullPtrLists.SetNum(meshes.Num());


		TArray<TArray<FBlastCollisionHull>> NewCombinedHulls;
		uint32 CurChunkCount = 0;

		for (int32 I = 0; I < meshes.Num(); I++)
		{
			UBlastMeshComponent* ParticipatingComponent = meshes[I];

			FTransform3f cTransform = FTransform3f(ParticipatingComponent->GetComponentTransform());
			cTransform.AddToTranslation(-FVector3f(essCentroid));
			UBlastAsset* ComponentAsset = ParticipatingComponent->GetBlastAsset();
			AssetList[I] = ComponentAsset->GetLoadedAsset();
			AssetRotations[I] = ToNvQuat(cTransform.GetRotation());
			AssetLocations[I] = ToNvVector(cTransform.GetTranslation());
			AssetScales[I] = ToNvVector(cTransform.GetScale3D());
			FMatrix44f TransformAtMergeMat = cTransform.ToMatrixWithScale();
			ComponentTransforms[I] = cTransform;

			const TArray<FBlastCookedChunkData>& CookedChunkData = ParticipatingComponent->GetBlastMesh()->GetCookedChunkData();

			PerComponentHullRanges[I].Add(0);

			//Transform convex hulls to world space also
			for (int32 Chunk = 0; Chunk < CookedChunkData.Num(); Chunk++)
			{
				NewCombinedHulls.Emplace();
				TArray<FBlastCollisionHull>& NewUEHulls = NewCombinedHulls.Last();

				UBodySetup* TempBodySetup = NewObject<UBodySetup>();
				TempBodySetup->AggGeom = CookedChunkData[Chunk].CookedBodySetup->AggGeom;

				auto& ConvexList = TempBodySetup->AggGeom.ConvexElems;
				//Convert boxes to convex
				for (auto& Box : TempBodySetup->AggGeom.BoxElems)
				{
					ConvexList.Emplace();
					ConvexList.Last().ConvexFromBoxElem(Box);
				}
				TempBodySetup->AggGeom.BoxElems.Empty();

				for (auto& Convex : ConvexList)
				{
					Convex.BakeTransformToVerts();
				}
				if (!TempBodySetup->AggGeom.SphereElems.IsEmpty() ||
					!TempBodySetup->AggGeom.SphylElems.IsEmpty())
				{
					UE_LOG(LogBlastEditor, Warning, TEXT("Collision contains unsupported elements"));
				}
				TempBodySetup->CreatePhysicsMeshes();
				for (const auto& C : ConvexList)
				{
					PerComponentHullLists[I].Emplace();
					FTempCollisionHull& NewHull = PerComponentHullLists[I].Last();

					NewUEHulls.Emplace();
					//NewUEHull is transformed with the TransformAtMerge, but NewHull is not
					FBlastCollisionHull& NewUEHull = NewUEHulls.Last();

					int32 NumIndicies = 0;
#if BLAST_USE_PHYSX
					physx::PxConvexMesh* pxMesh = C.GetConvexMesh();
					NewHull.Polygons.SetNum(pxMesh->getNbPolygons());
					NewUEHull.PolygonData.SetNum(pxMesh->getNbPolygons());
					NewHull.Points.SetNumUninitialized(pxMesh->getNbVertices());
					NewUEHull.Points.SetNumUninitialized(pxMesh->getNbVertices());
					const NvcVec3* OrigVerts = pxMesh->getVertices();
					FMemory::Memcpy(NewHull.Points.GetData(), OrigVerts, NewHull.Points.Num() * sizeof(FVector3f));
					for (int32 P = 0; P < NewHull.Points.Num(); P++)
					{
						NewUEHull.Points[P] = cTransform.TransformPosition(FromNvVector(OrigVerts[P]));
					}
#else
#if ENGINE_MINOR_VERSION < 4
					const Chaos::FConvex* chaosMesh = C.GetChaosConvexMesh().Get();
#else
					const Chaos::FConvex* chaosMesh = C.GetChaosConvexMesh().GetReference();
#endif
					NewHull.Polygons.SetNum(chaosMesh->NumPlanes());
					NewUEHull.PolygonData.SetNum(chaosMesh->NumPlanes());
					NewHull.Points.SetNumUninitialized(chaosMesh->NumVertices());
					NewUEHull.Points.SetNumUninitialized(chaosMesh->NumVertices());
					const TArray<Chaos::FVec3f>& OrigVerts = chaosMesh->GetVertices();
					FMemory::Memcpy(NewHull.Points.GetData(), OrigVerts.GetData(), NewHull.Points.Num() * sizeof(FVector3f));
					for (int32 P = 0; P < NewHull.Points.Num(); P++)
					{
						NewUEHull.Points[P] = cTransform.TransformPosition(OrigVerts[P]);
					}
#endif

					for (int32 P = 0; P < NewHull.Polygons.Num(); P++)
					{
#if BLAST_USE_PHYSX
						Nv::Blast::HullPolygon HullPoly;
						pxMesh->getPolygonData(P, HullPoly);
						NumIndicies = FMath::Max(NumIndicies, HullPoly.IndexBase + HullPoly.NbVerts);
#else
						Nv::Blast::HullPolygon& outPoly = NewHull.Polygons[P];
						const Chaos::FConvex::FPlaneType& ChaosPlane = chaosMesh->GetPlaneRaw(P);
						ToNvPlane4(FPlane4f(ChaosPlane.X(), ChaosPlane.Normal()), outPoly.plane);
						outPoly.vertexCount = chaosMesh->NumPlaneVertices(P);
						outPoly.indexBase = NumIndicies;
						NumIndicies += outPoly.vertexCount;
#endif

						FPlane4f Plane = FromNvPlane4f(outPoly.plane);
						//This flips the normal automatically if required
						Plane = Plane.TransformBy(TransformAtMergeMat);

						NewUEHull.PolygonData[P].indexBase = outPoly.indexBase;
						NewUEHull.PolygonData[P].vertexCount = outPoly.vertexCount;
						ToNvPlane4(Plane, NewUEHull.PolygonData[P].plane);
					}

					NewHull.Indices.SetNumUninitialized(NumIndicies);
					NewUEHull.Indices.SetNumUninitialized(NumIndicies);
					const uint32* IndiciesToCopy;
#if BLAST_USE_PHYSX
					IndiciesToCopy = pxMesh->getIndexBuffer();
#else
					IndiciesToCopy = reinterpret_cast<const uint32*>(C.IndexData.GetData());
#endif
					FMemory::Memcpy(NewHull.Indices.GetData(), IndiciesToCopy, sizeof(uint32) * NumIndicies);
					FMemory::Memcpy(NewUEHull.Indices.GetData(), IndiciesToCopy, sizeof(uint32) * NumIndicies);
				}
				PerComponentHullRanges[I].Add(PerComponentHullRanges[I].Last() + ConvexList.Num());
			}

			//Now the the list wont be resized populate the pointers
			PerComponentHullPtrLists[I].SetNumZeroed(PerComponentHullLists[I].Num());
			for (int32 CH = 0; CH < PerComponentHullLists[I].Num(); CH++)
			{
				PerComponentHullLists[I][CH].SetPointers();
				PerComponentHullPtrLists[I][CH] = &PerComponentHullLists[I][CH];
			}
			CurChunkCount += ComponentAsset->GetChunkCount();
		}

		//Final buffers to pass to API call
		TArray<const uint32*> ConvexHullOffsets;
		TArray<const Nv::Blast::CollisionHull**> ConvexHulls;

		ConvexHullOffsets.SetNumUninitialized(Actors.Num());
		ConvexHulls.SetNumUninitialized(Actors.Num());
		for (int32 I = 0; I < Actors.Num(); I++)
		{
			ConvexHullOffsets[I] = PerComponentHullRanges[I].GetData();
			ConvexHulls[I] = PerComponentHullPtrLists[I].GetData();
		}

		NvBlastExtAssetUtilsBondDesc* NewBonds = nullptr;
		int32 NewBondsCount = NvBlastExtAuthoringFindAssetConnectingBonds(AssetList.GetData(),
			AssetScales.GetData(), AssetRotations.GetData(), AssetLocations.GetData(),
			ConvexHullOffsets.GetData(), ConvexHulls.GetData(), Actors.Num(), NewBonds, distance);

		TArray<uint32> CombinedChunkReorderMap;
		CombinedChunkReorderMap.SetNumUninitialized(CurChunkCount);

		TArray<uint32> ResultingChunkIndexOffsets;
		ResultingChunkIndexOffsets.SetNumUninitialized(Actors.Num());

		NvBlastAssetDesc MergedAssetDesc = NvBlastExtAssetUtilsMergeAssets(AssetList.GetData(),
			AssetScales.GetData(),
			AssetRotations.GetData(),
			AssetLocations.GetData(),
			Actors.Num(), NewBonds, NewBondsCount, ResultingChunkIndexOffsets.GetData(), CombinedChunkReorderMap.GetData(), CombinedChunkReorderMap.Num());

		TArray<uint8> MergedAsset, MergedScratch;
		MergedAsset.SetNumUninitialized(NvBlastGetAssetMemorySize(&MergedAssetDesc, Nv::Blast::logLL));
		MergedScratch.SetNumUninitialized(NvBlastGetRequiredScratchForCreateAsset(&MergedAssetDesc, Nv::Blast::logLL));

		NvBlastAsset* MergedLLAsset = NvBlastCreateAsset(MergedAsset.GetData(), &MergedAssetDesc, MergedScratch.GetData(), &Nv::Blast::logLL);

		// TODO: TkAssetDesc BondFlags::BondJointed, addJointDesc

		nasset->PhysicsAsset = NewObject<UPhysicsAsset>(nasset, *nasset->GetName().Append(TEXT("_PhysicsAsset")), RF_NoFlags);
		nasset->Mesh = NewObject<USkeletalMesh>(nasset, *nasset->GetName().Append(TEXT("_SkelMesh")), RF_Public);
		nasset->Skeleton = NewObject<USkeleton>(nasset, *nasset->GetName().Append(TEXT("_Skeleton")));
		nasset->Mesh->SetSkeleton(nasset->Skeleton);

		USkeletalMesh* SkeletalMesh = nasset->Mesh;
		SkeletalMesh->PreEditChange(NULL);

		FTransform RootTransform(FTransform::Identity);
		if (SkeletalMesh->GetRefSkeleton().GetRefBonePose().Num())
		{
			RootTransform = SkeletalMesh->GetRefSkeleton().GetRefBonePose()[0];
		}
		SkeletalMesh->GetRefSkeleton().Empty();

		TArray<TArray<int32>> perComponentBoneToMerged;
		perComponentBoneToMerged.SetNum(meshes.Num());

		for (int32 cmp = 0; cmp < meshes.Num(); ++cmp)
		{
			int32 count = meshes[cmp]->GetBlastMesh()->GetChunkCount();
			for (int32 chunk = 0; chunk < count; ++chunk)
			{
				int32 newIndex = CombinedChunkReorderMap[ResultingChunkIndexOffsets[cmp] + chunk];
				perComponentBoneToMerged[cmp].Add(newIndex);
			}
		}

		{
			FReferenceSkeletonModifier RefSkelModifier(SkeletalMesh->GetRefSkeleton(), SkeletalMesh->GetSkeleton());
			RefSkelModifier.Add(FMeshBoneInfo(FName(TEXT("root"), FNAME_Add), TEXT("root"), INDEX_NONE), RootTransform);

			for (int32 NewChunk = 0; NewChunk < int32(MergedAssetDesc.chunkCount); NewChunk++)
			{
				uint32 ParentChunk = MergedAssetDesc.chunkDescs[NewChunk].parentChunkDescIndex;
				FName BoneName = UBlastMesh::GetDefaultChunkBoneNameFromIndex(NewChunk);
				//+1 to skip root
				int32 ParentBone = (ParentChunk != INDEX_NONE) ? (ParentChunk + 1) : 0;
				const FMeshBoneInfo BoneInfo(BoneName, BoneName.ToString(), ParentBone);
				RefSkelModifier.Add(BoneInfo, FTransform::Identity);
			}
		}

		FSkeletalMeshModel* ImportedResource = SkeletalMesh->GetImportedModel();
		ImportedResource->LODModels.Empty();
		ImportedResource->LODModels.Add(new FSkeletalMeshLODModel());

		FSkeletalMeshLODModel& LODModel = ImportedResource->LODModels[0];

		LODModel.MeshToImportVertexMap.Empty();
		LODModel.MaxImportVertex = 0;
		LODModel.GetRawPointIndices().Empty();
		LODModel.ActiveBoneIndices.Reset();
		LODModel.NumTexCoords = 1;

		TArray<TArray<uint32> > oldIndices;

		TMap<UMaterialInterface*, uint32> materialToIndex;

		TArray<TArray<uint32> > materialMapping;
		materialMapping.SetNum(meshes.Num());
		for (int32 i = 0; i < meshes.Num(); ++i)
		{
			const auto& inmats = meshes[i]->GetMaterials();
			materialMapping[i].SetNum(inmats.Num());

			const auto SlotsNames = meshes[i]->GetMaterialSlotNames();

			for (int32 mat = 0; mat < inmats.Num(); ++mat)
			{
				if (uniteMaterials == false)
				{
					SkeletalMesh->GetMaterials().Add(meshes[i]->GetSkinnedAsset()->GetMaterials()[mat]);
					SkeletalMesh->GetMaterials().Last().MaterialInterface = inmats[mat];
					SkeletalMesh->GetMaterials().Last().ImportedMaterialSlotName = SlotsNames[mat];
					SkeletalMesh->GetMaterials().Last().MaterialSlotName = SlotsNames[mat];
					materialMapping[i][mat] = SkeletalMesh->GetMaterials().Num() - 1;
				}
				else
				{
					uint32* pntr = materialToIndex.Find(inmats[mat]);
					if (pntr == nullptr)
					{
						SkeletalMesh->GetMaterials().Add(meshes[i]->GetSkinnedAsset()->GetMaterials()[mat]);
						SkeletalMesh->GetMaterials().Last().MaterialInterface = inmats[mat];
						SkeletalMesh->GetMaterials().Last().ImportedMaterialSlotName = SlotsNames[mat];
						SkeletalMesh->GetMaterials().Last().MaterialSlotName = SlotsNames[mat];
						materialMapping[i][mat] = SkeletalMesh->GetMaterials().Num() - 1;
						materialToIndex.Add(inmats[mat], materialMapping[i][mat]);
					}
					else
					{
						materialMapping[i][mat] = *pntr;
					}
				}
			}
		}


		FBox MeshBounds;
		MeshBounds.Init();
		TArray< TArray<int32_t> > perSectionParents;
		TArray< TArray<int32_t> > sectionToPerParentSection;

		int32 maxBonesPerSection = FGPUBaseSkinVertexFactory::GetMaxGPUSkinBones();

		for (int32 i = 0; i < meshes.Num(); ++i)
		{
			const auto& rresource = meshes[i]->GetSkinnedAsset()->GetResourceForRendering();
			const auto& rmodel = meshes[i]->GetSkinnedAsset()->GetImportedModel()->LODModels[0];
			const FSkeletalMeshLODRenderData& LODRenderData = rresource->LODRenderData[0];

			oldIndices.Emplace();
			LODRenderData.MultiSizeIndexContainer.GetIndexBuffer(oldIndices.Last());
			int32 numTriangles = 0;
			int32 sectNumber = 0;
			for (auto& sect : rmodel.Sections)
			{
				int32 sct = materialMapping[i][sect.MaterialIndex];
				for (; sct < LODModel.Sections.Num() && LODModel.Sections[sct].BoneMap.Num() + sect.BoneMap.Num() > maxBonesPerSection; ++sct);

				if (sct >= LODModel.Sections.Num())
				{
					LODModel.Sections.Add(sect);
					LODModel.Sections.Last().MaterialIndex = materialMapping[i][sect.MaterialIndex];
					auto& lsect = LODModel.Sections.Last();
					lsect.SoftVertices.Empty();
					lsect.BoneMap.Empty();
					lsect.NumTriangles = 0;
					lsect.NumVertices = 0;
					perSectionParents.Add(TArray<int32>());
					sectionToPerParentSection.Add(TArray<int32>());
				}
				auto& lsect = LODModel.Sections[sct];
				perSectionParents[sct].Add(i);
				sectionToPerParentSection[sct].Add(sectNumber);

				lsect.NumTriangles += sect.NumTriangles;
				lsect.NumVertices += sect.SoftVertices.Num();
				for (int32 vrt = 0; vrt < sect.SoftVertices.Num(); ++vrt)
				{
					lsect.SoftVertices.Add(sect.SoftVertices[vrt]);
					lsect.SoftVertices.Last().Position = ComponentTransforms[i].TransformPosition(lsect.SoftVertices.Last().Position);
					lsect.SoftVertices.Last().TangentX = ComponentTransforms[i].TransformVectorNoScale(lsect.SoftVertices.Last().TangentX);
					lsect.SoftVertices.Last().TangentY = ComponentTransforms[i].TransformVectorNoScale(lsect.SoftVertices.Last().TangentY);
					lsect.SoftVertices.Last().TangentZ = ComponentTransforms[i].TransformVectorNoScale(lsect.SoftVertices.Last().TangentZ);
					lsect.SoftVertices.Last().InfluenceBones[0] += lsect.BoneMap.Num();

					MeshBounds += FVector(lsect.SoftVertices.Last().Position);
				}
				for (int32 bid = 0; bid < sect.BoneMap.Num(); ++bid)
				{
					int32_t oldBone = sect.BoneMap[bid];
					int32_t chunk = meshes[i]->GetBlastMesh()->ChunkIndexToBoneIndex.IndexOfByKey(oldBone);
					lsect.BoneMap.Add(perComponentBoneToMerged[i][chunk] + 1);
					LODModel.ActiveBoneIndices.AddUnique(lsect.BoneMap.Last());
				}
				sectNumber += 1;
			}
		}
		SkeletalMesh->GetRefSkeleton().EnsureParentsExistAndSort(LODModel.ActiveBoneIndices);
		LODModel.NumVertices = LODModel.GetNumNonClothingVertices();

		// Finish building the sections.
		int32 vertexIndexOffset = 0;
		for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
		{
			FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
			Section.BaseIndex = LODModel.IndexBuffer.Num();
			Section.BaseVertexIndex = vertexIndexOffset;


			for (int32 parentMesh = 0; parentMesh < perSectionParents[SectionIndex].Num(); ++parentMesh)
			{
				int32 cpar = perSectionParents[SectionIndex][parentMesh];
				int32 psec = sectionToPerParentSection[SectionIndex][parentMesh];

				const auto& rresource = meshes[cpar]->GetSkinnedAsset()->GetImportedModel()->LODModels[0];
				const auto& csec = rresource.Sections[psec];

				for (uint32 Index = 0; Index < rresource.Sections[psec].NumTriangles * 3; Index++)
				{
					int32 realIndex = oldIndices[cpar][Index + csec.BaseIndex] - csec.BaseVertexIndex;
					LODModel.IndexBuffer.Add(realIndex + vertexIndexOffset);
				}
				vertexIndexOffset += csec.NumVertices;
			}
		}

		// Compute the required bones for this model.
		USkeletalMesh::CalculateRequiredBones(LODModel, SkeletalMesh->GetRefSkeleton(), NULL);

		SkeletalMesh->SetImportedBounds(FBoxSphereBounds(MeshBounds));

		SkeletalMesh->ResetLODInfo();
		SkeletalMesh->AddLODInfo();
		TArray<FSkeletalMeshLODInfo>& lodInfo = SkeletalMesh->GetLODInfoArray();

		lodInfo[0].LODHysteresis = 0.02f;
		FSkeletalMeshOptimizationSettings Settings;
		lodInfo[0].ReductionSettings = Settings;

		SkeletalMesh->CalculateInvRefMatrices();
		SkeletalMesh->PostEditChange();
		SkeletalMesh->MarkPackageDirty();

		SkeletalMesh->GetSkeleton()->MergeAllBonesToBoneTree(SkeletalMesh);

		TArray<TArray<FBlastCollisionHull> > combinedHullsRemapped;
		combinedHullsRemapped.SetNum(NewCombinedHulls.Num());

		TMap<FName, TArray<FBlastCollisionHull>> NewCombinedHullsMap;
		{
			int32 hullsOffset = 0;
			for (int32 cmp = 0; cmp < meshes.Num(); ++cmp)
			{
				int32 count = meshes[cmp]->GetBlastMesh()->GetChunkCount();
				for (int32 chunk = 0; chunk < count; ++chunk)
				{
					int32 newIndex = CombinedChunkReorderMap[ResultingChunkIndexOffsets[cmp] + chunk];
					combinedHullsRemapped[newIndex] = MoveTemp(NewCombinedHulls[chunk + hullsOffset]);
				}
				hullsOffset += count;
			}
		}


		for (int32 C = 0; C < NewCombinedHulls.Num(); C++)
		{
			NewCombinedHullsMap.Emplace(UBlastMesh::GetDefaultChunkBoneNameFromIndex(C), MoveTemp(combinedHullsRemapped[C]));
		}

		UBlastMeshFactory::RebuildPhysicsAsset(nasset, NewCombinedHullsMap);

		nasset->CopyFromLoadedAsset(MergedLLAsset);
		nasset->PostLoad();

		NVBLAST_FREE(NewBonds);
		NVBLAST_FREE((void*)MergedAssetDesc.bondDescs);
		NVBLAST_FREE((void*)MergedAssetDesc.chunkDescs);
	}


	static void UniteAssets(const TArray<AActor*>& Actors, float distance, bool uniteMaterials)
	{
		if (Actors.Num() < 2)
		{
			return;
		}
		CreateInPackageAsset(Actors, distance, uniteMaterials);
	}
};


class SUniteAssetsDialog : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SUniteAssetsDialog) {}
	SLATE_END_ARGS()

	// Constructs this widget with InArgs
	void Construct(const FArguments& InArgs)
	{

		mDistThreshold = 0.f;
		mMaterialUnionToggle = ECheckBoxState::Unchecked;
		ChildSlot
			[
				SNew(SBorder)
					.Padding(FMargin(0.0f, 3.0f, 1.0f, 0.0f))
					[
						SNew(SVerticalBox)

							+ SVerticalBox::Slot()
							.Padding(2.0f)
							.AutoHeight()

							+ SVerticalBox::Slot()
							.Padding(2.0f)
							.HAlign(HAlign_Right)
							.AutoHeight()
							[
								SNew(SUniformGridPanel)
									.SlotPadding(2)
									+ SUniformGridPanel::Slot(0, 1)
									[
										SNew(SButton)
											.Text(FText::FromString("Generate united asset"))
											.OnClicked(this, &SUniteAssetsDialog::OnClicked, true)
									]
									+ SUniformGridPanel::Slot(1, 1)
									[
										SNew(SButton)
											.Text(FText::FromString("Cancel"))
											.OnClicked(this, &SUniteAssetsDialog::OnClicked, false)
									]
									+ SUniformGridPanel::Slot(0, 0)
									[
										SNew(SNumericEntryBox<float>).MinValue(0).OnValueChanged(this, &SUniteAssetsDialog::OnDistanceThrChanged).Value(this, &SUniteAssetsDialog::getDistanceValue)
									]
									+ SUniformGridPanel::Slot(1, 0)
									[
										SNew(STextBlock).Text(FText::FromString("Distance threshold")).Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
									]
									+ SUniformGridPanel::Slot(2, 0)
									[
										SNew(SCheckBox).OnCheckStateChanged(this, &SUniteAssetsDialog::OnCheckBoxChanged).IsChecked(this, &SUniteAssetsDialog::getMaterialBoxValue).ToolTipText(LOCTEXT("UniteTool_MatUnionTT", "Unite materials"))
											[
												SNew(STextBlock)
													.Text(LOCTEXT("UniteMaterialLabelT", "Unite material slots with same material"))
											]
									]

							]
					]
			];
	}
	void OnDistanceThrChanged(float value)
	{
		mDistThreshold = value;
	}
	TOptional<float> getDistanceValue() const
	{
		return mDistThreshold;
	}

	void OnCheckBoxChanged(ECheckBoxState vl)
	{
		mMaterialUnionToggle = vl;
	}
	ECheckBoxState getMaterialBoxValue() const
	{
		return mMaterialUnionToggle;
	}


	FReply OnClicked(bool isGenerate)
	{
		shouldGenerate = isGenerate;
		CloseContainingWindow();
		return FReply::Handled();
	}

	static bool ShowWindow(const TArray<AActor*>& Actors);

	void CloseContainingWindow()
	{
		TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
		if (ContainingWindow.IsValid())
		{
			ContainingWindow->RequestDestroyWindow();
		}
	}

	float mDistThreshold;
	bool shouldGenerate;
	ECheckBoxState mMaterialUnionToggle;
};



bool SUniteAssetsDialog::ShowWindow(const TArray<AActor*>& Actors)
{
	const FText TitleText = NSLOCTEXT("UniteAssetsDialog", "UniteAssetsDialog", "Unite assets");
	// Create the window to pick the class
	TSharedRef<SWindow> CreateExtendedStructure = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::Autosized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SUniteAssetsDialog> CreateExtendedStructureDialog = SNew(SUniteAssetsDialog);
	CreateExtendedStructure->SetContent(CreateExtendedStructureDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddModalWindow(CreateExtendedStructure, RootWindow.ToSharedRef());
	}
	else
	{
		//assert here?
	}

	if (CreateExtendedStructureDialog->shouldGenerate)
	{
		AssetUnionTool::UniteAssets(Actors, CreateExtendedStructureDialog->mDistThreshold, CreateExtendedStructureDialog->mMaterialUnionToggle == ECheckBoxState::Checked);
	}


	return true;
}


void FBlastEditorModule::PopulateBlastMenuForActors(FMenuBuilder& InMenuBuilder, const TArray<AActor*>& Actors)
{
	TArray<UBlastMeshComponent*> BlastComponents;
	bool bAnyInSupportGraphAlready = false;
	for (AActor* Actor : Actors)
	{
		Actor->GetComponents(BlastComponents);
		for (UBlastMeshComponent* MC : BlastComponents)
		{
			if (MC->GetOwningSupportStructure() != nullptr)
			{
				bAnyInSupportGraphAlready = true;
				break;
			}
		}
		if (bAnyInSupportGraphAlready)
		{
			break;
		}
	}

	InMenuBuilder.AddMenuEntry(LOCTEXT("UniteAssets", "Unite assets"), FText(), FSlateIcon(), FExecuteAction::CreateLambda([Actors]
		{
			if (Actors.Num() < 2)
			{
				return;
			}
			SUniteAssetsDialog::ShowWindow(Actors);
		}));


	if (!bAnyInSupportGraphAlready)
	{
		InMenuBuilder.AddMenuEntry(LOCTEXT("AddToNewExtSupport", "Add New Extended Support Group"), FText(), FSlateIcon(), FExecuteAction::CreateLambda([Actors]
			{
				if (Actors.Num() == 0)
				{
					return;
				}

				FBox BoundsBox(ForceInit);
				for (AActor* Actor : Actors)
				{
					BoundsBox += Actor->GetComponentsBoundingBox();
				}

				FActorSpawnParameters SpawnParams;
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				ABlastExtendedSupportStructure* SupportStructure = Actors[0]->GetWorld()->SpawnActor<ABlastExtendedSupportStructure>(BoundsBox.GetCenter(), FRotator::ZeroRotator, SpawnParams);

				FName CurrentFolder;
				for (AActor* Actor : Actors)
				{
					if (CurrentFolder.IsNone())
					{
						CurrentFolder = Actor->GetFolderPath();
					}
					SupportStructure->AddStructureActor(Actor);
				}
				//Clear the old associations of the actors if any and re-assign them
				SupportStructure->ResetActorAssociations();

				if (!CurrentFolder.IsNone())
				{
					SupportStructure->SetFolderPath(CurrentFolder);
				}

				GEditor->SelectNone(true, true);
				GEditor->SelectActor(SupportStructure, true, true);
			}));
	}

	if (bAnyInSupportGraphAlready)
	{
		InMenuBuilder.AddMenuEntry(LOCTEXT("RemoveFromExtSupport", "Remove Extended Support Groups"), FText(), FSlateIcon(), FExecuteAction::CreateLambda([Actors]
			{
				TArray<UBlastMeshComponent*> AllBlastComponents;
				ABlastExtendedSupportStructure::GetStructureComponents(Actors, AllBlastComponents);
				for (UBlastMeshComponent* MC : AllBlastComponents)
				{
					ABlastExtendedSupportStructure* CurrentStructure = MC->GetOwningSupportStructure();
					if (CurrentStructure)
					{
						MC->SetOwningSuppportStructure(nullptr, INDEX_NONE);
						CurrentStructure->RemoveStructureActor(MC->GetOwner());
						if (CurrentStructure->GetStructureActors().Num() == 0)
						{
							//remove this one
							CurrentStructure->Destroy();
						}
					}
				}
			}));
	}
}

static TAutoConsoleVariable<int32> CVarBlastGlueDebugDrawing(TEXT("blast.GlueDebugDrawing"), 1, TEXT("Show debug lines during Blast building."));

EEditorBuildResult FBlastEditorModule::DoBlastBuild(UWorld* World, FName Step)
{
	const bool bDrawDebug = CVarBlastGlueDebugDrawing.GetValueOnGameThread() != 0;
	if (bDrawDebug)
	{
		//Just to avoid confusion
		FlushPersistentDebugLines(World);
	}

	UBlastGlueWorldTag* WorldTag = UBlastGlueWorldTag::GetForWorld(World);
	if (!WorldTag || !WorldTag->bIsDirty)
	{
		return EEditorBuildResult::Success;
	}

	UE_LOG(LogBlastEditor, Log, TEXT("Doing Blast Build"));

	TArray<UBlastMeshComponent*> BlastComponentsInScene;
	TArray<UBlastMeshComponent*> TempComponents;
	for (TActorIterator<AActor> PossibleBlastActorItr(World); PossibleBlastActorItr; ++PossibleBlastActorItr)
	{
		AActor* PossibleBlastActor = *PossibleBlastActorItr;
		PossibleBlastActor->GetComponents(TempComponents);

		for (UBlastMeshComponent* BlastComponent : TempComponents)
		{
			if (BlastComponent->GetBlastAsset() == nullptr)
			{
				if (BlastComponent->GetModifiedAsset() != nullptr)
				{
					BlastComponent->MarkPackageDirty();
				}
				BlastComponent->SetModifiedAsset(nullptr);
			}
			else
			{

				FBox ComponentBounds = BlastComponent->Bounds.GetBox();
				if (BlastComponent->bSupportedByWorld)
				{
					if (bDrawDebug)
					{
						DrawDebugBox(World, ComponentBounds.GetCenter(), ComponentBounds.GetExtent(), FQuat::Identity, FColor::Green, true, 5.0f, 0, 3.0f);
					}
					BlastComponentsInScene.Add(BlastComponent);
				}
				else
				{
					//Clear the modified asset
					if (BlastComponent->GetModifiedAsset() != nullptr)
					{
						BlastComponent->MarkPackageDirty();
					}
					BlastComponent->SetModifiedAsset(nullptr);
					if (bDrawDebug)
					{
						DrawDebugBox(World, ComponentBounds.GetCenter(), ComponentBounds.GetExtent(), FQuat::Identity, FColor::Blue, true, 5.0f, 0, 3.0f);
					}
				}
			}
		}
	}

	if (BlastComponentsInScene.Num() == 0)
	{
		//nothing to do
		return EEditorBuildResult::Success;
	}

	//We are going to loop over this many times, so cache the result
	TArray<ABlastGlueVolume*> GlueVolumes;
	for (TActorIterator<ABlastGlueVolume> ActorItr(World); ActorItr; ++ActorItr)
	{
		if (ActorItr->bEnabled)
		{
			GlueVolumes.Add(*ActorItr);
		}
	}

	TArray<ABlastExtendedSupportStructure*> ExtendedSupportActors;
	for (TActorIterator<ABlastExtendedSupportStructure> ActorItr(World); ActorItr; ++ActorItr)
	{
		if (ActorItr->bEnabled)
		{
			ExtendedSupportActors.Add(*ActorItr);
		}
	}

	if (GlueVolumes.Num() == 0 && ExtendedSupportActors.Num() == 0)
	{
		//nothing to do
		return EEditorBuildResult::Success;
	}

	TArray<uint32> OverlappingChunks;
	TArray<FVector> GlueVectors;
	TSet<ABlastGlueVolume*> OverlappingVolumes;
	for (UBlastMeshComponent* BlastComponent : BlastComponentsInScene)
	{
		if (BlastComponent->GetSupportChunksInVolumes(GlueVolumes, OverlappingChunks, GlueVectors, OverlappingVolumes, bDrawDebug))
		{
			check(OverlappingChunks.Num() == GlueVectors.Num());
			UE_LOG(LogBlastEditor, Log, TEXT("Found %d support chunks in volume"), OverlappingChunks.Num());

			const bool bAllowModifiedAsset = false;
			UBlastAsset* Asset = BlastComponent->GetBlastAsset(bAllowModifiedAsset);

			TArray<NvcVec3> BondVector;
			BondVector.SetNumUninitialized(GlueVectors.Num());
			for (int32 I = 0; I < GlueVectors.Num(); I++)
			{
				BondVector[I].x = GlueVectors[I].X;
				BondVector[I].y = GlueVectors[I].Y;
				BondVector[I].z = GlueVectors[I].Z;
			}

			//Add "ghost chunk" here and create a new UBlastAsset
			NvBlastAsset* LLModifiedAsset = NvBlastExtAssetUtilsAddWorldBonds(Asset->GetLoadedAsset(), OverlappingChunks.GetData(), static_cast<uint32>(OverlappingChunks.Num()), BondVector.GetData(), nullptr);
			check(LLModifiedAsset);

			UBlastAsset* NewModifiedAsset = NewObject<UBlastAsset>(BlastComponent);
			//Use the same GUID as our non-modified asset so we can tell if it changes later
			NewModifiedAsset->CopyFromLoadedAsset(LLModifiedAsset, Asset->GetAssetGUID());
			NVBLAST_FREE(LLModifiedAsset);

			BlastComponent->SetModifiedAsset(NewModifiedAsset);
			BlastComponent->MarkPackageDirty();

			for (ABlastGlueVolume* Volume : OverlappingVolumes)
			{
				Volume->GluedComponents.Add(BlastComponent);
			}
		}
		else
		{
			//Set it to the mesh to mark it as done
			BlastComponent->SetModifiedAsset(BlastComponent->GetBlastMesh());
			BlastComponent->MarkPackageDirty();
		}
	}

	for (ABlastExtendedSupportStructure* ExtendedSupport : ExtendedSupportActors)
	{
		if (!BuildExtendedSupport(ExtendedSupport))
		{
			return EEditorBuildResult::Skipped;
		}
	}

	WorldTag->bIsDirty = false;

	return EEditorBuildResult::Success;
}



bool FBlastEditorModule::BuildExtendedSupport(ABlastExtendedSupportStructure* ExtSupportActor)
{
	TArray<UBlastMeshComponent*> ParticipatingComponents;
	ExtSupportActor->GetStructureComponents(ParticipatingComponents);

	if (ParticipatingComponents.Num() == 0) //Skip empty support actor
	{
		return true;
	}

	TArray<FBlastExtendedStructureComponent> StoredComponents;
	StoredComponents.SetNum(ParticipatingComponents.Num());

	TArray<TArray<FTempCollisionHull>> PerComponentHullLists;
	PerComponentHullLists.SetNum(ParticipatingComponents.Num());

	TArray<TArray<const Nv::Blast::CollisionHull*>> PerComponentHullPtrLists;
	PerComponentHullPtrLists.SetNum(ParticipatingComponents.Num());

	TArray<TArray<uint32>> PerComponentHullRanges;
	PerComponentHullRanges.SetNum(ParticipatingComponents.Num());

	TArray<TArray<FBlastCollisionHull>> NewCombinedHulls;
	TArray<FIntPoint> ChunkToOriginalChunkMap;

	TArray<const NvBlastAsset*> AssetList;
	TArray<NvcQuat> AssetRotations;
	TArray<NvcVec3> AssetLocations;
	TArray<NvcVec3> AssetScales;

	AssetList.SetNumUninitialized(ParticipatingComponents.Num());
	AssetRotations.SetNumUninitialized(ParticipatingComponents.Num());
	AssetLocations.SetNumUninitialized(ParticipatingComponents.Num());
	AssetScales.SetNumUninitialized(ParticipatingComponents.Num());

	//This is not required but it's used for validation our assumption about how the API orders chunks
	TArray<uint32> ChunkIndexOffsets;
	ChunkIndexOffsets.SetNumUninitialized(ParticipatingComponents.Num());

	int32 CurChunkCount = 0;
	for (int32 I = 0; I < ParticipatingComponents.Num(); I++)
	{
		UBlastMeshComponent* ParticipatingComponent = ParticipatingComponents[I];
		FBlastExtendedStructureComponent& Component = StoredComponents[I];

		ChunkIndexOffsets[I] = CurChunkCount;

		Component.MeshComponent = ParticipatingComponent;
		Component.TransformAtMerge = ParticipatingComponent->GetComponentTransform();

		UBlastAsset* ComponentAsset = ParticipatingComponent->GetBlastAsset();
		Component.GUIDAtMerge = ComponentAsset->GetAssetGUID();

		AssetList[I] = ComponentAsset->GetLoadedAsset();

		AssetRotations[I] = ToNvQuat(Component.TransformAtMerge.GetRotation());
		AssetLocations[I] = ToNvVector(Component.TransformAtMerge.GetTranslation());
		AssetScales[I] = ToNvVector(Component.TransformAtMerge.GetScale3D());

		FMatrix44f TransformAtMergeMat = FMatrix44f(Component.TransformAtMerge.ToMatrixWithScale());

		const TArray<FBlastCookedChunkData>& CookedChunkData = ParticipatingComponent->GetBlastMesh()->GetCookedChunkData();

		PerComponentHullRanges[I].Add(0);

		Component.ChunkIDs.Reserve(CookedChunkData.Num());
		//Transform convex hulls to world space also
		for (int32 Chunk = 0; Chunk < CookedChunkData.Num(); Chunk++)
		{
			Component.ChunkIDs.Add(Chunk + CurChunkCount);
			ChunkToOriginalChunkMap.Emplace(I, Chunk);

			NewCombinedHulls.Emplace();
			TArray<FBlastCollisionHull>& NewUEHulls = NewCombinedHulls.Last();

			UBodySetup* TempBodySetup = NewObject<UBodySetup>();
			TempBodySetup->AggGeom = CookedChunkData[Chunk].CookedBodySetup->AggGeom;

			auto& ConvexList = TempBodySetup->AggGeom.ConvexElems;
			//Convert boxes to convex
			for (auto& Box : TempBodySetup->AggGeom.BoxElems)
			{
				ConvexList.Emplace();
				ConvexList.Last().ConvexFromBoxElem(Box);
			}
			TempBodySetup->AggGeom.BoxElems.Empty();

			for (auto& Convex : ConvexList)
			{
				Convex.BakeTransformToVerts();
			}

			if (TempBodySetup->AggGeom.SphereElems.Num() > 0 ||
				TempBodySetup->AggGeom.SphylElems.Num() > 0)
			{
				UE_LOG(LogBlastEditor, Warning, TEXT("Collision contains unsupported elements"));
			}

			TempBodySetup->CreatePhysicsMeshes();

			for (const auto& C : ConvexList)
			{

				PerComponentHullLists[I].Emplace();
				FTempCollisionHull& NewHull = PerComponentHullLists[I].Last();

				NewUEHulls.Emplace();
				//NewUEHull is transformed with the TransformAtMerge, but NewHull is not
				FBlastCollisionHull& NewUEHull = NewUEHulls.Last();

				int32 NumIndicies = 0;
#if BLAST_USE_PHYSX
				physx::PxConvexMesh* pxMesh = C.GetConvexMesh();
				NewHull.Polygons.SetNum(pxMesh->getNbPolygons());
				NewUEHull.PolygonData.SetNum(pxMesh->getNbPolygons());
				NewHull.Points.SetNumUninitialized(pxMesh->getNbVertices());
				NewUEHull.Points.SetNumUninitialized(pxMesh->getNbVertices());
				const NvcVec3* OrigVerts = pxMesh->getVertices();
				FMemory::Memcpy(NewHull.Points.GetData(), OrigVerts, NewHull.Points.Num() * sizeof(FVector3f));
				for (int32 P = 0; P < NewHull.Points.Num(); P++)
				{
					NewUEHull.Points[P] = Component.TransformAtMerge.TransformPosition(FromNvVector(OrigVerts[P]));
				}
#else
#if ENGINE_MINOR_VERSION < 4
					const Chaos::FConvex* chaosMesh = C.GetChaosConvexMesh().Get();
#else
					const Chaos::FConvex* chaosMesh = C.GetChaosConvexMesh().GetReference();
#endif
				NewHull.Polygons.SetNum(chaosMesh->NumPlanes());
				NewUEHull.PolygonData.SetNum(chaosMesh->NumPlanes());
				NewHull.Points.SetNumUninitialized(chaosMesh->NumVertices());
				NewUEHull.Points.SetNumUninitialized(chaosMesh->NumVertices());
				const TArray<Chaos::FVec3f>& OrigVerts = chaosMesh->GetVertices();
				FMemory::Memcpy(NewHull.Points.GetData(), OrigVerts.GetData(), NewHull.Points.Num() * sizeof(FVector3f));
				for (int32 P = 0; P < NewHull.Points.Num(); P++)
				{
					NewUEHull.Points[P] = FVector3f(Component.TransformAtMerge.TransformPosition(FVector(OrigVerts[P])));
				}
#endif

				for (int32 P = 0; P < NewHull.Polygons.Num(); P++)
				{
#if BLAST_USE_PHYSX
					Nv::Blast::HullPolygon HullPoly;
					pxMesh->getPolygonData(P, HullPoly);
					NumIndicies = FMath::Max(NumIndicies, HullPoly.IndexBase + HullPoly.NbVerts);
#else
					Nv::Blast::HullPolygon& outPoly = NewHull.Polygons[P];
					const Chaos::FConvex::FPlaneType& ChaosPlane = chaosMesh->GetPlaneRaw(P);
					ToNvPlane4(FPlane4f(ChaosPlane.X(), ChaosPlane.Normal()), outPoly.plane);
					outPoly.vertexCount = chaosMesh->NumPlaneVertices(P);
					outPoly.indexBase = NumIndicies;
					NumIndicies += outPoly.vertexCount;
#endif

					FPlane4f Plane = FromNvPlane4f(outPoly.plane);
					Plane = Plane.TransformBy(TransformAtMergeMat);

					NewUEHull.PolygonData[P].indexBase = outPoly.indexBase;
					NewUEHull.PolygonData[P].vertexCount = outPoly.vertexCount;
					ToNvPlane4(Plane, NewUEHull.PolygonData[P].plane);
				}

				NewHull.Indices.SetNumUninitialized(NumIndicies);
				NewUEHull.Indices.SetNumUninitialized(NumIndicies);
				const uint32* IndiciesToCopy;
#if BLAST_USE_PHYSX
				IndiciesToCopy = pxMesh->getIndexBuffer();
#else
				IndiciesToCopy = reinterpret_cast<const uint32*>(C.IndexData.GetData());
#endif
				FMemory::Memcpy(NewHull.Indices.GetData(), IndiciesToCopy, sizeof(uint32) * NumIndicies);
				FMemory::Memcpy(NewUEHull.Indices.GetData(), IndiciesToCopy, sizeof(uint32) * NumIndicies);
			}
			PerComponentHullRanges[I].Add(PerComponentHullRanges[I].Last() + ConvexList.Num());
		}

		//Now the the list wont be resized populate the pointers
		PerComponentHullPtrLists[I].SetNumZeroed(PerComponentHullLists[I].Num());
		for (int32 CH = 0; CH < PerComponentHullLists[I].Num(); CH++)
		{
			PerComponentHullLists[I][CH].SetPointers();
			PerComponentHullPtrLists[I][CH] = &PerComponentHullLists[I][CH];
		}

		CurChunkCount += ComponentAsset->GetChunkCount();
	}

	//Final buffers to pass to API call
	TArray<const uint32*> ConvexHullOffsets;
	TArray<const Nv::Blast::CollisionHull**> ConvexHulls;

	ConvexHullOffsets.SetNumUninitialized(ParticipatingComponents.Num());
	ConvexHulls.SetNumUninitialized(ParticipatingComponents.Num());
	for (int32 I = 0; I < ParticipatingComponents.Num(); I++)
	{
		ConvexHullOffsets[I] = PerComponentHullRanges[I].GetData();
		ConvexHulls[I] = PerComponentHullPtrLists[I].GetData();
	}

	NvBlastExtAssetUtilsBondDesc* NewBonds = nullptr;
	int32 NewBondsCount = NvBlastExtAuthoringFindAssetConnectingBonds(AssetList.GetData(),
		AssetScales.GetData(), AssetRotations.GetData(), AssetLocations.GetData(),
		ConvexHullOffsets.GetData(), ConvexHulls.GetData(), ParticipatingComponents.Num(), NewBonds, ExtSupportActor->GetBondGenerationDistance());

	TArray<uint32> CombinedChunkReorderMap;
	CombinedChunkReorderMap.SetNumUninitialized(CurChunkCount);

	TArray<uint32> ResultingChunkIndexOffsets;
	ResultingChunkIndexOffsets.SetNumUninitialized(ParticipatingComponents.Num());

	NvBlastAssetDesc MergedAssetDesc = NvBlastExtAssetUtilsMergeAssets(AssetList.GetData(),
		AssetScales.GetData(),
		AssetRotations.GetData(),
		AssetLocations.GetData(),
		ParticipatingComponents.Num(), NewBonds, NewBondsCount, ResultingChunkIndexOffsets.GetData(), CombinedChunkReorderMap.GetData(), CombinedChunkReorderMap.Num());

	//Make sure the entries in CombinedChunkReorderMap are in the order we expect: the input assets in the order we passed.
	check(ChunkIndexOffsets == ResultingChunkIndexOffsets);

	{
		//Build a new to old chunk map also
		TArray<uint32> CombinedChunkReorderMapReverse;
		CombinedChunkReorderMapReverse.SetNum(MergedAssetDesc.chunkCount);

		for (int32 OrigChunk = 0; OrigChunk < CombinedChunkReorderMap.Num(); OrigChunk++)
		{
			CombinedChunkReorderMapReverse[CombinedChunkReorderMap[OrigChunk]] = OrigChunk;
		}

		//Remap combined to component/chunk pair map
		TArray<FIntPoint> ChunkToOriginalChunkMapNew;
		ChunkToOriginalChunkMapNew.SetNum(ChunkToOriginalChunkMap.Num());
		for (int32 NewChunk = 0; NewChunk < ChunkToOriginalChunkMapNew.Num(); NewChunk++)
		{
			ChunkToOriginalChunkMapNew[NewChunk] = MoveTemp(ChunkToOriginalChunkMap[CombinedChunkReorderMapReverse[NewChunk]]);
		}
		ChunkToOriginalChunkMap = MoveTemp(ChunkToOriginalChunkMapNew);

		TArray<TArray<FBlastCollisionHull>> NewCombinedHullsRemapped;
		NewCombinedHullsRemapped.SetNum(NewCombinedHulls.Num());
		for (int32 NewChunk = 0; NewChunk < NewCombinedHullsRemapped.Num(); NewChunk++)
		{
			NewCombinedHullsRemapped[NewChunk] = MoveTemp(NewCombinedHulls[CombinedChunkReorderMapReverse[NewChunk]]);
		}
		NewCombinedHulls = MoveTemp(NewCombinedHullsRemapped);

		//Remap component/chunk pair to combined map
		for (FBlastExtendedStructureComponent& C : StoredComponents)
		{
			for (int32& CI : C.ChunkIDs)
			{
				CI = CombinedChunkReorderMap[CI];
			}
		}
	}

	TArray<uint8> MergedAsset, MergedScratch;
	MergedAsset.SetNumUninitialized(NvBlastGetAssetMemorySize(&MergedAssetDesc, Nv::Blast::logLL));
	MergedScratch.SetNumUninitialized(NvBlastGetRequiredScratchForCreateAsset(&MergedAssetDesc, Nv::Blast::logLL));

	NvBlastAsset* MergedLLAsset = NvBlastCreateAsset(MergedAsset.GetData(), &MergedAssetDesc, MergedScratch.GetData(), &Nv::Blast::logLL);

	UBlastMeshExtendedSupport* BlastMesh = NewObject<UBlastMeshExtendedSupport>(ExtSupportActor->GetExtendedSupportMeshComponent());
	BlastMesh->PhysicsAsset = NewObject<UPhysicsAsset>(BlastMesh, *BlastMesh->GetName().Append(TEXT("_PhysicsAsset")), RF_NoFlags);
	BlastMesh->Mesh = NewObject<USkeletalMesh>(BlastMesh, *BlastMesh->GetName().Append(TEXT("_SkelMesh")), RF_Public);
	BlastMesh->Skeleton = NewObject<USkeleton>(BlastMesh, *BlastMesh->GetName().Append(TEXT("_Skeleton")));
	BlastMesh->Mesh->SetSkeleton(BlastMesh->Skeleton);

	USkeletalMesh* SkeletalMesh = BlastMesh->Mesh;
	SkeletalMesh->PreEditChange(NULL);

	FTransform RootTransform(FTransform::Identity);
	if (SkeletalMesh->GetRefSkeleton().GetRefBonePose().Num())
	{
		RootTransform = SkeletalMesh->GetRefSkeleton().GetRefBonePose()[0];
	}
	SkeletalMesh->GetRefSkeleton().Empty();

	{
		FReferenceSkeletonModifier RefSkelModifier(SkeletalMesh->GetRefSkeleton(), SkeletalMesh->GetSkeleton());
		RefSkelModifier.Add(FMeshBoneInfo(FName(TEXT("root"), FNAME_Add), TEXT("root"), INDEX_NONE), RootTransform);

		for (int32 NewChunk = 0; NewChunk < int32(MergedAssetDesc.chunkCount); NewChunk++)
		{
			uint32 ParentChunk = MergedAssetDesc.chunkDescs[NewChunk].parentChunkDescIndex;
			FName BoneName = UBlastMesh::GetDefaultChunkBoneNameFromIndex(NewChunk);
			//+1 to skip root
			int32 ParentBone = (ParentChunk != INDEX_NONE) ? (ParentChunk + 1) : 0;
			const FMeshBoneInfo BoneInfo(BoneName, BoneName.ToString(), ParentBone);
			RefSkelModifier.Add(BoneInfo, FTransform::Identity);
		}
	}

	FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
	ImportedModel->LODModels.Empty();
	ImportedModel->LODModels.Add(new FSkeletalMeshLODModel());

	FSkeletalMeshLODModel& LODModel = ImportedModel->LODModels[0];
	LODModel.NumTexCoords = 1;

	SkeletalMesh->ResetLODInfo();
	SkeletalMesh->AddLODInfo();
	TArray<FSkeletalMeshLODInfo>& lodInfo = SkeletalMesh->GetLODInfoArray();

	lodInfo[0].LODHysteresis = 0.02f;
	FSkeletalMeshOptimizationSettings Settings;
	lodInfo[0].ReductionSettings = Settings;

	SkeletalMesh->CalculateInvRefMatrices();
	SkeletalMesh->PostEditChange();
	SkeletalMesh->MarkPackageDirty();

	SkeletalMesh->GetSkeleton()->MergeAllBonesToBoneTree(SkeletalMesh);

	TMap<FName, TArray<FBlastCollisionHull>> NewCombinedHullsMap;
	for (int32 C = 0; C < NewCombinedHulls.Num(); C++)
	{
		NewCombinedHullsMap.Emplace(UBlastMesh::GetDefaultChunkBoneNameFromIndex(C), MoveTemp(NewCombinedHulls[C]));
	}

	UBlastMeshFactory::RebuildPhysicsAsset(BlastMesh, NewCombinedHullsMap);

	BlastMesh->CopyFromLoadedAsset(MergedLLAsset);
	BlastMesh->PostLoad();

	NVBLAST_FREE(NewBonds);
	NVBLAST_FREE((void*)MergedAssetDesc.bondDescs);
	NVBLAST_FREE((void*)MergedAssetDesc.chunkDescs);
	ExtSupportActor->StoreSavedComponents(StoredComponents, ChunkToOriginalChunkMap, BlastMesh);

	//Now that we are populated we can set this which rebuilds the components
	for (int32 I = 0; I < StoredComponents.Num(); I++)
	{
		StoredComponents[I].MeshComponent->SetOwningSuppportStructure(ExtSupportActor, I);
	}

	return true;
}

const FName FBlastEditorModule::BlastBuildStepId = FName("BlastBuild");

#undef LOCTEXT_NAMESPACE
