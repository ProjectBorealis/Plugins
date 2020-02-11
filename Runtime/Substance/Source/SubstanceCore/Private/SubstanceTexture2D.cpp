// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceTexture2D.cpp

#include "SubstanceTexture2D.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceSettings.h"
#include "SubstanceTexture2DDynamicResource.h"
#include "substance/framework/output.h"

#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "Engine/TextureLODSettings.h"
#include "Engine/Texture2D.h"
#include "Runtime/RenderCore/Public/RenderUtils.h"

#if WITH_EDITOR
#include "Interfaces/ITargetPlatform.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "IContentBrowserSingleton.h"
#endif //WITH_EDITOR

#if PLATFORM_PS4 || (SUBSTANCE_HAS_PS4_SDK && WITH_EDITOR)
#include "SubstanceCorePS4Utils.h"
#endif

USubstanceTexture2D::USubstanceTexture2D(class FObjectInitializer const& PCIP) : Super(PCIP)
{
	bLinkLegacy = false;
}

FString USubstanceTexture2D::GetDesc()
{
	return FString::Printf(TEXT("%dx%d[%s]"), SizeX, SizeY, GPixelFormats[Format].Name);
}

void USubstanceTexture2D::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	//If we aren't loading, always use most up to date serialization method
	if (!Ar.IsLoading())
	{
		Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);
		SerializeCurrent(Ar);
		return;
	}

	//Check the version to see if we need to serialize legacy
	bool ShouldSerializeLegacy = false;
	if (Ar.CustomVer(FSubstanceCoreCustomVersion::GUID) < FSubstanceCoreCustomVersion::FrameworkRefactor)
	{
		//Handle Legacy Loading here
		bLinkLegacy = true;
		ShouldSerializeLegacy = true;
	}

	//Register the new version of the engine.
	Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);

	//Call serialize based on version
	(ShouldSerializeLegacy == true) ? SerializeLegacy(Ar) : SerializeCurrent(Ar);
}

void USubstanceTexture2D::SerializeCurrent(FArchive& Ar)
{
	//TODO:: Remove filler before release
	int32 Filler;

	Ar << Format;
	Ar << SizeX;
	Ar << SizeY;
	Ar << NumMips;
	Ar << mUid;
	Ar << ParentInstance;
	Ar << Filler;
	Ar << mUserData.CacheGuid;

	int32 FirstMipToSerialize = 0;

	bCooked = Ar.IsCooking();
	Ar << bCooked;

	if (bCooked && !this->IsDefaultSubobject())
	{
		ESubstanceGenerationMode ParentPackageGenerationMode = ESubstanceGenerationMode::SGM_PlatformDefault;

		if (ParentInstance && ParentInstance->ParentFactory && ParentInstance->Instance && ParentInstance->ParentFactory->SubstancePackage)
		{
			ParentPackageGenerationMode = ParentInstance->ParentFactory->GetGenerationMode();
			if (ParentPackageGenerationMode == ESubstanceGenerationMode::SGM_PlatformDefault)
			{
				ParentPackageGenerationMode = GetDefault<USubstanceSettings>()->DefaultGenerationMode;
			}
		}
		else
		{
			ParentPackageGenerationMode = ESubstanceGenerationMode::SGM_Baked;
		}

		if (Ar.IsSaving() || Ar.IsCooking())
		{
			//The number of mipmap levels which will not be saved / regenerated during loading
			int32 MipsToRegen = 0;

			switch (ParentPackageGenerationMode)
			{
			//Baked will not regenerate any of the mips.
			case ESubstanceGenerationMode::SGM_Baked:
				{
					MipsToRegen = 0;
					break;
				}

			//Load Sync will regenerate all of the maps at load time. This is a blocking call and to keep load times down, recommended that
			//this is used sparsely and elements that will have run time input changes or are core game meshes (characters)
			case ESubstanceGenerationMode::SGM_OnLoadSync:
			case ESubstanceGenerationMode::SGM_OnLoadSyncAndCache:
				{
					MipsToRegen = NumMips - 1;
					break;
				}

			//Load Async will generate all mip levels based on project settings. This allows for substances to load small data amounts
			//when many load operations are happening and after a few frames, regenerate so all mips. Used to optimize load times.
			case ESubstanceGenerationMode::SGM_OnLoadAsync:
			case ESubstanceGenerationMode::SGM_OnLoadAsyncAndCache:
				{
					MipsToRegen = FMath::Clamp(GetDefault<USubstanceSettings>()->AsyncLoadMipClip, 0, NumMips - 1);
					break;
				}

			//Default to regenerating all maps.
			default:
				{
					MipsToRegen = NumMips - 1;
					break;
				}
			}

			FirstMipToSerialize += FMath::Max(0, MipsToRegen);
		}

		Ar << FirstMipToSerialize;
	}

	//Special case for the ps4
	if (Ar.IsCooking() && !this->IsDefaultSubobject())
	{
#if WITH_EDITOR
		if (Ar.CookingTarget()->PlatformName() == FString(TEXT("PS4")))
			CookTextureForPS4(Ar, FirstMipToSerialize);
		else
			CookTexture2D(Ar, FirstMipToSerialize);
#endif
	}
	else if (Ar.IsSaving() && !Ar.IsTransacting() && !this->IsDefaultSubobject())
	{
		CookTexture2D(Ar, FirstMipToSerialize);
	}

	if (Ar.IsLoading())
	{
		NumMips -= FirstMipToSerialize;
		Mips.Empty(NumMips);

		for (int32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
		{
			Mips.Add(new FTexture2DMipMap());
			SerializeTexture2DMip(Mips[MipIndex], Ar, this, MipIndex);
		}
	}

	//Associate this asset with the current plugin version
	Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);
	Ar.SetCustomVersion(FSubstanceCoreCustomVersion::GUID, FSubstanceCoreCustomVersion::LatestVersion, FName("LegacyUpdated"));
}

void USubstanceTexture2D::SerializeLegacy(FArchive& Ar)
{
	Ar << Format;
	Ar << SizeX;
	Ar << SizeY;
	Ar << NumMips;
	Ar << mUserData.CacheGuid;
	Ar << ParentInstance;

	int32 FirstMipToSerialize = 0;

	bCooked = Ar.IsCooking();
	Ar << bCooked;

	if (bCooked && !this->IsDefaultSubobject())
	{
		ESubstanceGenerationMode ParentPackageGenerationMode = ESubstanceGenerationMode::SGM_PlatformDefault;

		if (ParentInstance && ParentInstance->ParentFactory)
		{
			ParentPackageGenerationMode = ParentInstance->ParentFactory->GetGenerationMode();
			if (ParentPackageGenerationMode == ESubstanceGenerationMode::SGM_PlatformDefault)
			{
				ParentPackageGenerationMode = GetDefault<USubstanceSettings>()->DefaultGenerationMode;
			}
		}

		Ar << FirstMipToSerialize;
	}

	NumMips -= FirstMipToSerialize;
	Mips.Empty(NumMips);

	for (int32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
	{
		Mips.Add(new FTexture2DMipMap());
		SerializeTexture2DMip(Mips[MipIndex], Ar, this, MipIndex);
	}

	//Clear the archive to be updated with the new serialization on save.
	Ar.FlushCache();

	//Forcing package dirty
	UPackage* Package = GetOutermost();
	Package->SetDirtyFlag(true);
}

void USubstanceTexture2D::CookTextureForPS4(FArchive& Ar, int32 FirstMip)
{
#if PLATFORM_PS4 || (SUBSTANCE_HAS_PS4_SDK && WITH_EDITOR)
	if (!PlatformMips.Num())
	{
		int32 LargestMipSize = Mips[FirstMip].BulkData.GetBulkDataSize();
		void* TempPtr = FMemory::Malloc(LargestMipSize);

		FTexture2DMipMap* MipMap;

		for (int32 MipIndex = 0; MipIndex < NumMips - FirstMip; ++MipIndex)
		{
			MipMap = new FTexture2DMipMap();
			PlatformMips.Add(MipMap);
			FTexture2DMipMap& MipReference = Mips[MipIndex + FirstMip];

			MipReference.BulkData.GetCopy(&TempPtr, true);
			MipMap->SizeX = MipReference.SizeX;
			MipMap->SizeY = MipReference.SizeY;

			Substance::Helpers::TileMipForPS4(TempPtr, MipMap, Format);
		}
		FMemory::Free(TempPtr);
	}

	for (int32 MipIndex = 0; MipIndex < PlatformMips.Num(); ++MipIndex)
	{
		SerializeTexture2DMip(PlatformMips[MipIndex], Ar, this, MipIndex);
	}
#endif
}

void USubstanceTexture2D::CookTexture2D(FArchive& Ar, int32 FirstMip)
{
	//Shouldn't hit this right now
	for (int32 MipIndex = 0; MipIndex < NumMips - FirstMip; ++MipIndex)
	{
		SerializeTexture2DMip(Mips[MipIndex + FirstMip], Ar, this, MipIndex);
	}
}

void USubstanceTexture2D::SerializeTexture2DMip(FTexture2DMipMap& mip, FArchive& Ar, UObject* Owner, int32 MipIdx)
{
	bool bMipCooked = Ar.IsCooking();
	Ar << bMipCooked;

	mip.BulkData.Serialize(Ar, Owner, MipIdx);
	Ar << mip.SizeX;
	Ar << mip.SizeY;

#if WITH_EDITORONLY_DATA
	if (!bMipCooked)
	{
		Ar << mip.DerivedDataKey;
	}
#endif // #if WITH_EDITORONLY_DATA
}

void USubstanceTexture2D::BeginDestroy()
{
	//Route BeginDestroy.
	Super::BeginDestroy();

	if (OutputCopy)
	{
		//Reset output user data
		OutputCopy->mUserData = 0;

		//Nullify the pointer to this texture's address so that others see it has been destroyed
		mUserData.Texture.Reset();

		//Disable the output in the parent instance
		if (ParentInstance && ParentInstance->Instance)
		{
			Substance::Helpers::ClearFromRender(ParentInstance);

			auto ItOut = ParentInstance->Instance->getOutputs().begin();
			for (; ItOut != ParentInstance->Instance->getOutputs().end(); ++ItOut)
			{
				if ((*ItOut)->mDesc.mUid == mUid)
				{
					(*ItOut)->mEnabled = false;
					break;
				}
			}
		}
	}
}

ESubChannelType USubstanceTexture2D::GetChannel()
{
	switch (OutputCopy->mDesc.defaultChannelUse())
	{
	case SubstanceAir::Channel_BaseColor:
		return ESubChannelType::Channel_BaseColor;
	case SubstanceAir::Channel_Metallic:
		return ESubChannelType::Channel_Metallic;
	case SubstanceAir::Channel_Roughness:
		return ESubChannelType::Channel_Roughness;
	case SubstanceAir::Channel_Emissive:
		return ESubChannelType::Channel_Emissive;
	case SubstanceAir::Channel_Normal:
		return ESubChannelType::Channel_Normal;
	case SubstanceAir::Channel_Mask:
		return ESubChannelType::Channel_Mask;
	case SubstanceAir::Channel_Opacity:
		return ESubChannelType::Channel_Opacity;
	case SubstanceAir::Channel_Refraction:
		return ESubChannelType::Channel_Refraction;
	case SubstanceAir::Channel_AmbientOcclusion:
		return ESubChannelType::Channel_AmbientOcclusion;
	case SubstanceAir::Channel_Displacement:
		return ESubChannelType::Channel_Displacement;
	case SubstanceAir::Channel_Ambient:
		return ESubChannelType::Channel_Ambient;
	case SubstanceAir::Channel_Glossiness:
		return ESubChannelType::Channel_Glossiness;
	case SubstanceAir::Channel_Reflection:
		return ESubChannelType::Channel_Reflection;
	case SubstanceAir::Channel_Diffuse:
		return ESubChannelType::Channel_Diffuse;
	case SubstanceAir::Channel_Height:
		return ESubChannelType::Channel_Height;

	default:
		return ESubChannelType::Channel_Invalid;
	};
}

#if WITH_EDITOR
bool USubstanceTexture2D::CanEditChange(const UProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);

	if (bIsEditable && InProperty != nullptr)
	{
		bIsEditable = false;

		if (InProperty->GetFName() == TEXT("AddressX") ||
		        InProperty->GetFName() == TEXT("AddressY") ||
		        InProperty->GetFName() == TEXT("UnpackMin") ||
		        InProperty->GetFName() == TEXT("UnpackMax") ||
		        InProperty->GetFName() == TEXT("Filter") ||
		        InProperty->GetFName() == TEXT("LODBias") ||
		        InProperty->GetFName() == TEXT("sRGB") ||
		        InProperty->GetFName() == TEXT("LODGroup"))
		{
			bIsEditable = true;
		}
	}

	return bIsEditable;
}
#endif

void USubstanceTexture2D::LinkOutputInstance()
{
	//Find the output using the UID serialized
	OutputCopy = nullptr;
	for (const auto& OutIt : ParentInstance->Instance->getOutputs())
	{
		//Check to verify the substance output instance has a valid ID.
		if (OutIt->mDesc.mUid == 0)
		{
			UE_LOG(LogSubstanceCore, Error, TEXT("(%s) Output instance has an invalid UID"), OutIt->mDesc.mLabel.c_str());
		}

		if (OutIt->mDesc.mUid == mUid)
		{
			OutputCopy = const_cast<SubstanceAir::OutputInstance*>(OutIt);
			break;
		}
	}
}

void USubstanceTexture2D::LinkLegacyOutputInstance()
{
	//Find the output using the UID serialized
	OutputCopy = nullptr;
	for (const auto& OutIt : ParentInstance->Instance->getOutputs())
	{
		//Search the map for the GUID using the mUid as the key
		if (ParentInstance->OutputTextureLinkData[OutIt->mDesc.mUid] == mUserData.CacheGuid)
		{
			OutputCopy = const_cast<SubstanceAir::OutputInstance*>(OutIt);
			mUid = OutputCopy->mDesc.mUid;
			break;
		}
	}
}

void USubstanceTexture2D::PostLoad()
{
	//Before we early return, make sure we set no tile!
#if PLATFORM_PS4
	bNoTiling = false;
#endif

	//Make sure our parent is valid
	if (nullptr == ParentInstance)
	{
		UE_LOG(LogSubstanceCore, Log, TEXT("No parent instance found for this SubstanceTexture2D (%s). Defaulting to baked texture."), *GetFullName());
		Super::PostLoad();
		return;
	}

	//Make sure the parent instance is loaded
	ParentInstance->ConditionalPostLoad();

	//Make sure we have a valid parent factory or the instance will always be invalid / thrashed
	if (ParentInstance->ParentFactory == nullptr)
	{
		UE_LOG(LogSubstanceCore, Log, TEXT("Parent Graph Instance Factory of (%s) is Invalid - Instance Frozen - Defaulting to baked texture."), *GetFullName());
		Super::PostLoad();
		return;
	}

	//Make sure the graph instance is valid for non baked assets
	if (nullptr == ParentInstance->Instance && ParentInstance->ParentFactory->GenerationMode != SGM_Baked)
	{
		UE_LOG(LogSubstanceCore, Log, TEXT("Framework graph instance of (%s) is invalid - Defaulting to baked texture."), *GetFullName());
		Super::PostLoad();
		return;
	}

	//Don't run the setup for baked assets
	if (ParentInstance->ParentFactory->GenerationMode == SGM_Baked && bCooked)
	{
		Super::PostLoad();
		return;
	}

	//Finds our output instance and stores a reference to class member
	(bLinkLegacy == true) ? LinkLegacyOutputInstance() : LinkOutputInstance();

	if (!OutputCopy)
	{
		//The opposite situation is possible, no texture but an OutputInstance,
		//in this case the OutputInstance is disabled, but when the texture is
		//alive the Output must exist.
		UE_LOG(LogSubstanceCore, Error, TEXT("No matching output found for this SubstanceTexture2D (%s). You need to delete the texture and its parent instance."), *GetFullName());
		Super::PostLoad();
		return;
	}
	else
	{
		mUserData.Texture = this;
		mUserData.ParentInstance = ParentInstance;
		OutputCopy->mUserData = (size_t)&mUserData;
		OutputCopy->mEnabled = true;

		if (ParentInstance->ParentFactory->GetGenerationMode() != ESubstanceGenerationMode::SGM_Baked)
		{
			OutputCopy->invalidate();
		}

		//Revalidate format in the case things have changes since this asset was created
		if (ParentInstance && ParentInstance->ParentFactory)
			Substance::Helpers::ValidateFormat(this, OutputCopy);
	}

	Super::PostLoad();
}

void USubstanceTexture2D::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		//Set the parent instance to null to disconnect the duplicate from the graph instance
		ParentInstance = nullptr;

#if WITH_EDITOR
		if (GIsEditor)
		{
			TArray<UObject*> AssetList;
			AssetList.AddUnique(this);

			FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
		}
#endif
	}
}

void USubstanceTexture2D::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	for (auto it = Mips.CreateConstIterator(); it; ++it)
	{
		CumulativeResourceSize.AddDedicatedSystemMemoryBytes(it->BulkData.GetBulkDataSize());
	}
}

FTextureResource* USubstanceTexture2D::CreateResource()
{
	if (Mips.Num())
	{
		return new FSubstanceTexture2DDynamicResource(this);
	}

	return nullptr;
}

void USubstanceTexture2D::UpdateResource()
{
	Super::UpdateResource();

	if (Resource)
	{
		struct FUpdateSubstanceTexture
		{
			FTexture2DDynamicResource* Resource;
			USubstanceTexture2D* Owner;
			TArray<const void*>	MipData;
		};

		FUpdateSubstanceTexture* SubstanceData = new FUpdateSubstanceTexture;

		SubstanceData->Resource = (FTexture2DDynamicResource*)Resource;
		SubstanceData->Owner = this;

		for (int32 MipIndex = 0; MipIndex < SubstanceData->Owner->Mips.Num(); MipIndex++)
		{
			FTexture2DMipMap& MipMap = SubstanceData->Owner->Mips[MipIndex];

			//This gets unlocked in render command
			const void* MipData = MipMap.BulkData.LockReadOnly();
			SubstanceData->MipData.Add(MipData);
		}

		ENQUEUE_RENDER_COMMAND(UpdateSubstanceTexture)(
		   [SubstanceData](FRHICommandList& RHICmdList)
		{
			//Read the resident mip-levels into the RHI texture.
			for (int32 MipIndex = 0; MipIndex < SubstanceData->Owner->Mips.Num(); MipIndex++)
			{
				uint32 DestPitch;
				void* TheMipData = RHILockTexture2D(SubstanceData->Resource->GetTexture2DRHI(), MipIndex, RLM_WriteOnly, DestPitch, false);

				FTexture2DMipMap& MipMap = SubstanceData->Owner->Mips[MipIndex];
				const void* MipData = SubstanceData->MipData[MipIndex];

				//For platforms that returned 0 pitch from Lock, we need to just use the bulk data directly, never do
				//runtime block size checking, conversion, or the like
				if (DestPitch == 0)
				{
					FMemory::Memcpy(TheMipData, MipData, MipMap.BulkData.GetBulkDataSize());
				}
				else
				{
					EPixelFormat PixelFormat = SubstanceData->Owner->Format;
					const uint32 BlockSizeX = GPixelFormats[PixelFormat].BlockSizeX;	//Block width in pixels
					const uint32 BlockSizeY = GPixelFormats[PixelFormat].BlockSizeY;	//Block height in pixels
					const uint32 BlockBytes = GPixelFormats[PixelFormat].BlockBytes;
					uint32 NumColumns = (MipMap.SizeX + BlockSizeX - 1) / BlockSizeX;	//Num-of columns in the source data (in blocks)
					uint32 NumRows = (MipMap.SizeY + BlockSizeY - 1) / BlockSizeY;	    //Num-of rows in the source data (in blocks)
					if (PixelFormat == PF_PVRTC2 || PixelFormat == PF_PVRTC4)
					{
						//PVRTC has minimum 2 blocks width and height
						NumColumns = FMath::Max<uint32>(NumColumns, 2);
						NumRows = FMath::Max<uint32>(NumRows, 2);
					}
					const uint32 SrcPitch = NumColumns * BlockBytes;					//Num-of bytes per row in the source data
					const uint32 EffectiveSize = BlockBytes * NumColumns * NumRows;

					//Copy the texture data.
					CopyTextureData2D(MipData, TheMipData, MipMap.SizeY, PixelFormat, SrcPitch, DestPitch);
				}

				MipMap.BulkData.Unlock();

				RHIUnlockTexture2D(SubstanceData->Resource->GetTexture2DRHI(), MipIndex, false);
			}

			delete SubstanceData;
		});
	}
}

//Note - The flags at the bottom might be able to be altered due to engine updates? Look into this
/** Create RHI sampler states. */
void FSubstanceTexture2DDynamicResource::CreateSamplerStates(float MipMapBias)
{
	//Create the sampler state RHI resource.
	FSamplerStateInitializerRHI SamplerStateInitializer
	(
	    (ESamplerFilter)UDeviceProfileManager::Get().GetActiveProfile()->GetTextureLODSettings()->GetSamplerFilter(SubstanceOwner),
	    SubstanceOwner->AddressX == TA_Wrap ? AM_Wrap : (SubstanceOwner->AddressX == TA_Clamp ? AM_Clamp : AM_Mirror),
	    SubstanceOwner->AddressY == TA_Wrap ? AM_Wrap : (SubstanceOwner->AddressY == TA_Clamp ? AM_Clamp : AM_Mirror),
	    AM_Wrap,
	    MipMapBias
	);
	SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);

	//Create a custom sampler state for using this texture in a deferred pass, where ddx / ddy are discontinuous
	FSamplerStateInitializerRHI DeferredPassSamplerStateInitializer
	(
	    (ESamplerFilter)UDeviceProfileManager::Get().GetActiveProfile()->GetTextureLODSettings()->GetSamplerFilter(SubstanceOwner),
	    SubstanceOwner->AddressX == TA_Wrap ? AM_Wrap : (SubstanceOwner->AddressX == TA_Clamp ? AM_Clamp : AM_Mirror),
	    SubstanceOwner->AddressY == TA_Wrap ? AM_Wrap : (SubstanceOwner->AddressY == TA_Clamp ? AM_Clamp : AM_Mirror),
	    AM_Wrap,
	    MipMapBias,
	    //Disable anisotropic filtering, since aniso doesn't respect MaxLOD
	    1,
	    0,
	    //Prevent the less detailed mip levels from being used, which hides artifacts on silhouettes due to ddx / ddy being very large
	    //This has the side effect that it increases minification aliasing on light functions
	    2
	);

	DeferredPassSamplerStateRHI = RHICreateSamplerState(DeferredPassSamplerStateInitializer);
}

//Called when the resource is initialized. This is only called by the rendering thread.
void FSubstanceTexture2DDynamicResource::InitRHI()
{
	float MipMapBias =
	    FMath::Clamp<float>(
	        UTexture2D::GetGlobalMipMapLODBias() + ((SubstanceOwner->LODGroup == TEXTUREGROUP_UI) ?
	                -SubstanceOwner->Mips.Num() :
	                SubstanceOwner->LODBias),
	        0,
	        SubstanceOwner->Mips.Num());

	//Create the sampler state RHI resource.
	CreateSamplerStates(MipMapBias);

	uint32 Flags = 0;
	if (SubstanceOwner->bIsResolveTarget)
	{
		Flags |= TexCreate_ResolveTargetable;
		bIgnoreGammaConversions = true;		//Note, we're ignoring Owner->SRGB (it should be false).
	}

	if (SubstanceOwner->SRGB)
	{
		Flags |= TexCreate_SRGB;
	}
	else
	{
		bIgnoreGammaConversions = true;
		bSRGB = false;
	}

	if (SubstanceOwner->bNoTiling)
	{
		Flags |= TexCreate_NoTiling;
	}

#if PLATFORM_PS4
	Flags |= TexCreate_OfflineProcessed;
#endif

	FRHIResourceCreateInfo CreateInfo;
	Texture2DRHI = RHICreateTexture2D(GetSizeX(), GetSizeY(), SubstanceOwner->Format, SubstanceOwner->NumMips, 1, Flags, CreateInfo);
	TextureRHI = Texture2DRHI;
	RHIUpdateTextureReference(SubstanceOwner->TextureReference.TextureReferenceRHI, TextureRHI);
}

//Called when the resource is released. This is only called by the rendering thread.
void FSubstanceTexture2DDynamicResource::ReleaseRHI()
{
	RHIUpdateTextureReference(SubstanceOwner->TextureReference.TextureReferenceRHI, TextureRHI);
	FTextureResource::ReleaseRHI();
	Texture2DRHI.SafeRelease();
}

//Returns the Texture2DRHI, which can be used for locking/unlocking the mips.
FTexture2DRHIRef FSubstanceTexture2DDynamicResource::GetTexture2DRHI()
{
	return Texture2DRHI;
}
