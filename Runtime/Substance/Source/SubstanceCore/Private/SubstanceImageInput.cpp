// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceImageInput.cpp

#include "SubstanceImageInput.h"
#include "SubstanceCorePrivatePCH.h"
#include "substance/framework/graph.h"
#include "SubstanceCoreHelpers.h"

#if WITH_EDITOR
#include "Factories.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#endif

USubstanceImageInput::USubstanceImageInput(class FObjectInitializer const& PCIP) : Super(PCIP)
{
	mUserData.ImageUObjectSource = this;
}

//TODO:: Currently there is no support for altering the JPEG compression settings! If we need to support this - add checks here
#if WITH_EDITOR
bool USubstanceImageInput::CanEditChange(const UProperty* InProperty) const
{
	if (SourceFilePath.EndsWith(".tga"))
	{
		if (InProperty->GetNameCPP() == TEXT("CompressionAlpha"))
		{
			if (ImageA.GetBulkDataSize() == 0)
			{
				return false;
			}
		}

		//Check the image input is still available before allowing modification of compression level
		TUniquePtr<FArchive> FileReader(IFileManager::Get().CreateFileReader(*SourceFilePath));
		if (!FileReader)
		{
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

/** Callback for whenever a property of an input image has be changed */
void USubstanceImageInput::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//Change in the consumers array can come from the deletion of a graph instance
	//in this case we need to remove invalid entries
	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.MemberProperty->GetNameCPP() == TEXT("Consumers"))
	{
		for (auto itConsumer = Consumers.CreateIterator(); itConsumer; ++itConsumer)
		{
			if ((*itConsumer) == nullptr)
			{
				Consumers.RemoveAt(itConsumer.GetIndex());
				itConsumer.Reset();
			}
		}
	}

	if (PropertyChangedEvent.MemberProperty && SourceFilePath.EndsWith(".tga"))
	{
		// start by reloading the image input
		TUniquePtr<FArchive> FileReader(IFileManager::Get().CreateFileReader(*SourceFilePath));

		// which could be missing
		if (!FileReader)
		{
			return;
		}

		uint32	BufferSize = FileReader->TotalSize();
		void* Buffer = FMemory::Malloc(BufferSize);
#if SUBSTANCE_MEMORY_STAT
		INC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, BufferSize);
#endif

		FileReader->Serialize(Buffer, BufferSize);

		const FTGAFileHeader* TGA = (FTGAFileHeader*)Buffer;
		SizeX = TGA->Width;
		SizeY = TGA->Height;
		int32 TextureDataSizeRGBA = SizeX * SizeY * 4;
		uint32* DecompressedImageRGBA = (uint32*)FMemory::Malloc(TextureDataSizeRGBA);
#if SUBSTANCE_MEMORY_STAT
		INC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, TextureDataSizeRGBA);
#endif

		DecompressTGA_helper(TGA, DecompressedImageRGBA, TextureDataSizeRGBA, nullptr);
#if SUBSTANCE_MEMORY_STAT
		DEC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, FMemory::GetAllocSize(Buffer));
#endif
		FMemory::Free(Buffer);
		Buffer = nullptr;

		// if the user modified the color channel compression level
		if (PropertyChangedEvent.MemberProperty->GetNameCPP() == TEXT("CompressionRGB"))
		{
			// replace the eventually compressed color channel by the file's content
			ImageRGB.RemoveBulkData();
			ImageRGB.Lock(LOCK_READ_WRITE);

			void* ImagePtr = ImageRGB.Realloc(TextureDataSizeRGBA);

			FMemory::Memcpy(ImagePtr, DecompressedImageRGBA, TextureDataSizeRGBA);

			// perform compression if not 0
			if (CompressionLevelRGB != 0 && ImageRGB.GetBulkDataSize())
			{
				TArray<uint8> CompressedImageRGB;

				// for retro compatibility, 1 = default compression quality, otherwise, its the desired compression level
				int32 Quality = FMath::Clamp(CompressionLevelRGB == 1 ? 85 : CompressionLevelRGB, 0, 100);

				Substance::Helpers::CompressJpeg(ImagePtr, ImageRGB.GetBulkDataSize(), SizeX, SizeY, 4, CompressedImageRGB, Quality);

				ImagePtr = ImageRGB.Realloc(CompressedImageRGB.Num());
				FMemory::Memcpy(ImagePtr, CompressedImageRGB.GetData(), CompressedImageRGB.Num());
			}

			ImageRGB.Unlock();
		}
		else if (PropertyChangedEvent.MemberProperty->GetNameCPP() == TEXT("CompressionAlpha") && ImageA.GetBulkDataSize())
		{
			int32 TextureDataSizeA = SizeX * SizeY;
			uint32* DecompressedImageA = (uint32*)FMemory::Malloc(TextureDataSizeA);
#if SUBSTANCE_MEMORY_STAT
			INC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, TextureDataSizeA);
#endif

			Substance::Helpers::Split_RGBA_8bpp(
			    SizeX, SizeY,
			    (uint8*)DecompressedImageRGBA, TextureDataSizeRGBA,
			    (uint8*)DecompressedImageA, TextureDataSizeA);

			ImageA.RemoveBulkData();
			ImageA.Lock(LOCK_READ_WRITE);

			void* AlphaPtr = ImageA.Realloc(TextureDataSizeA);

			FMemory::Memcpy(AlphaPtr, DecompressedImageA, TextureDataSizeA);
#if SUBSTANCE_MEMORY_STAT
			DEC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, FMemory::GetAllocSize(DecompressedImageA));
#endif
			FMemory::Free(DecompressedImageA);
			DecompressedImageA = nullptr;

			if (CompressionLevelAlpha != 0)
			{
				TArray<uint8> CompressedImageA;

				// for retro compatibility, 1 = default compression quality, otherwise, its the desired compression level
				int32 Quality = FMath::Clamp(CompressionLevelAlpha == 1 ? 85 : CompressionLevelAlpha, 0, 100);

				Substance::Helpers::CompressJpeg(AlphaPtr, ImageA.GetBulkDataSize(), SizeX,	SizeY, 1, CompressedImageA,	Quality);

				AlphaPtr = ImageA.Realloc(CompressedImageA.Num());
				FMemory::Memcpy(AlphaPtr, CompressedImageA.GetData(), CompressedImageA.Num());
			}

			ImageA.Unlock();
		}
#if SUBSTANCE_MEMORY_STAT
		DEC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, FMemory::GetAllocSize(DecompressedImageRGBA));
#endif
		FMemory::Free(DecompressedImageRGBA);
		DecompressedImageRGBA = nullptr;
	}
	else
	{
		CompressionLevelRGB = 1;
	}

	// update eventual Substance using this image input
	for (const auto& ConsumerItr : Consumers)
	{
		//Handling 0 as UID for reimported images
		Substance::Helpers::UpdateInput(ConsumerItr->Instance, nullptr, this);
		Substance::Helpers::RenderAsync(ConsumerItr->Instance);
	}
}
#endif

void USubstanceImageInput::BeginDestroy()
{
	//Set the input of the consumer using this object to clear user data
	for (const auto& ConsumerItr : Consumers)
	{
		if (!ConsumerItr || !ConsumerItr->Instance)
			continue;

		for (const auto& InputItr : ConsumerItr->Instance->getInputs())
		{
			if (InputItr->mDesc.isImage())
			{
				SubstanceAir::InputInstanceImage* imageInstance = (SubstanceAir::InputInstanceImage*)InputItr;

				if (imageInstance->getImage().get() == nullptr)
				{
					continue;
				}

				UObject* imageWrapper = reinterpret_cast<InputImageData*>(imageInstance->getImage()->mUserData)->ImageUObjectSource;

				if (imageWrapper == this)
				{
					Substance::Helpers::UpdateInput(ConsumerItr->Instance, InputItr, nullptr);
				}
			}
		}
	}

	//Route BeginDestroy
	Super::BeginDestroy();
}

void USubstanceImageInput::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);

	//#TODO:: If all consumers are frozen and nothing will be using this image, do not save out the texture data
	if (Ar.IsSaving())
	{
		ImageRGB.StoreCompressedOnDisk(0 == CompressionLevelRGB ? NAME_Zlib : NAME_None);
		ImageA.StoreCompressedOnDisk(0 == CompressionLevelAlpha ? NAME_Zlib : NAME_None);
	}

	ImageRGB.Serialize(Ar, this);
	ImageA.Serialize(Ar, this);

	//Image inputs can be used multiple times
	ImageRGB.ClearBulkDataFlags(BULKDATA_SingleUse);
	ImageA.ClearBulkDataFlags(BULKDATA_SingleUse);

	Ar << CompressionLevelRGB;
	Ar << CompressionLevelAlpha;

	if (Ar.IsCooking())
	{
		SourceFilePath = FString();
		SourceFileTimestamp = FString();
	}
}

FString USubstanceImageInput::GetDesc()
{
	return FString::Printf(TEXT("%dx%d (%d kB)"), SizeX, SizeY, GetResourceSizeBytes(EResourceSizeMode::Exclusive) / 1024);
}

void USubstanceImageInput::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	CumulativeResourceSize.AddDedicatedSystemMemoryBytes(ImageRGB.GetBulkDataSize());
	CumulativeResourceSize.AddDedicatedSystemMemoryBytes(ImageA.GetBulkDataSize());
}
