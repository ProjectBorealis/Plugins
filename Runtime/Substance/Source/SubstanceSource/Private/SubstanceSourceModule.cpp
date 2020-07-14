// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceModule.cpp

#include "SubstanceSourceModule.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceFactory.h"
#include "SubstanceSourceStyle.h"
#include "SubstanceSourceAsset.h"
#include "SourceCallbacks.h"
#include "SubstanceSourceDetails.h"
#include "SubstanceSourceWindow.h"

#include "Toolkits/AssetEditorToolkit.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "EdGraphUtilities.h"
#include "AssetToolsModule.h"
#include "EditorStyleSet.h"
#include "AutomatedAssetImportData.h"
#include "Algo/Reverse.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "IPluginWardenModule.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/Commands.h"

#include <substance/source/attachment.h>
#include <substance/source/databasefactory.h>
#include <substance/source/userfactory.h>
#include <substance/source/asset.h>
#include <substance/source/ihttpresponse.h>
#include <substance/source/source.h>

#define LOCTEXT_NAMESPACE "SubstanceSourceModule"

namespace SubstanceSourceModule
{
const FName SubstanceSourceAppIdentifier = FName(TEXT("SubstanceSourceApp"));
}


class FSubstanceSourceModuleCommands : public TCommands<FSubstanceSourceModuleCommands>
{
public:
	/** Constructor */
	FSubstanceSourceModuleCommands()
		: TCommands<FSubstanceSourceModuleCommands>(
		      "SubstanceSourceModuleCommands",
		      NSLOCTEXT("Contexts", "SubstanceSource", "SubstanceSource"),
		      NAME_None,
		      FEditorStyle::GetStyleSetName()) // Icon Style Set
	{
	}

	TSharedPtr<FUICommandInfo> LaunchSubstanceSourceWindow;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};

void FSubstanceSourceModuleCommands::RegisterCommands()
{
	UI_COMMAND(LaunchSubstanceSourceWindow, "Substance Source", "Run Substance Source", EUserInterfaceActionType::Button, FInputGesture());
}

class FSubstanceSourceModule : public ISubstanceSourceModule
{
public:
	/** Constructor, set up console commands and variables **/
	FSubstanceSourceModule()
		: mDatabase(nullptr)
		, mDatabaseLoadingStatus(ESourceDatabaseLoadingStatus::NotLoaded)
		, ContentBrowserPath("/Game")
	{
	}

	/** Declare event callbacks for when an event occurs from the framework */
	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FUserLoggedInEvent, FUserLoggedInEvent)
	virtual FUserLoggedInEvent& OnLoggedIn() override
	{
		return LoggedInEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FUserLoggedOutEvent, FUserLoggedOutEvent)
	virtual FUserLoggedOutEvent& OnLoggedOut() override
	{
		return LoggedOutEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FUserLoginFailedEvent, FUserLoginFailedEvent)
	virtual FUserLoginFailedEvent& OnLoginFailed() override
	{
		return LoginFailedEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FDatabaseFailedEvent, FDatabaseFailedEvent)
	virtual FDatabaseFailedEvent& OnDatabaseError() override
	{
		return LoadDatabaseFailed;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FDatabaseLoaded, FDatabaseLoaded)
	virtual FDatabaseLoaded& OnDatabaseLoaded() override
	{
		return DatabaseLoadedEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FNewSearchedAssetsSet, FNewSearchedAssetsSet)
	virtual FNewSearchedAssetsSet& OnSearchCompleted() override
	{
		return SearchCompletedEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FDisplayMyAssets, FDisplayMyAssets)
	virtual FDisplayMyAssets& OnDisplayUsersAssets() override
	{
		return DisplayUserAssetsEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FDisplayCategory, FDisplayCategory)
	virtual FDisplayCategory& OnDisplayCategory() override
	{
		return DisplayCategoryEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FSubstancePurchased, FSubstancePurchased)
	virtual FSubstancePurchased& OnSubstancePurchased() override
	{
		return SubstancePurchasedEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FModuleShutdown, FModuleShutdown)
	virtual FModuleShutdown& OnModuleShutdown() override
	{
		return ModuleShutdownEvent;
	}

	DECLARE_DERIVED_EVENT(FSubstanceSourceModule, ISubstanceSourceModule::FCategoryChanged, FCategoryChanged)
	virtual FCategoryChanged& OnCategoryChanged() override
	{
		return CategoryChanged;
	}

	/** Called right after the module DLL has been loaded and the module object has been created */
	virtual void StartupModule() override
	{
		Alg::Source::Callbacks::setInstance(&mSourceCallbacks);

		//Creates the button above the scene viewport to launch source window
		CreateToolbarButton();

		mDatabaseLoadingStatus = ESourceDatabaseLoadingStatus::NotLoaded;
		mUserLoginStatus = ESourceUserLoginStatus::LoggedOut;

		FailedDownloads = 0;
		SuccessfulDownloads = 0;

		//Create extensibility managers
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

		FSubstanceSourceStyle::Initialize();

		//Register delegate to get updated path from the content browser
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FContentBrowserModule::FOnAssetPathChanged& PathChangedDelegate = ContentBrowserModule.GetOnAssetPathChanged();
		PathChangedDelegate.AddRaw(this, &FSubstanceSourceModule::OnContentBrowserPathSwitch);
	}

	virtual bool IsEntitled() override
	{
		bool bIsEntitled = false;

		const FString ItemId = TEXT("ebbba723e5cd42d0be496506ead4172f");
		const FString OfferId = TEXT("");

		IPluginWardenModule::Get().CheckEntitlementForPlugin(
		    LOCTEXT("SubstancePlugin", "Substance Plugin"),
		    ItemId,
		    OfferId,
		    FText(),
		    IPluginWardenModule::EUnauthorizedErrorHandling::Silent,
		    [&]()
		{
			bIsEntitled = true;
		});

		return bIsEntitled;
	}

	/** Called before the module is unloaded, right before the module object is destroyed */
	virtual void ShutdownModule() override
	{
		//#NOTE:: Any slate classes holding on to smart pointer of framework objects (Including callback bindings)
		//needs to register as a listener to this event and clean up this memory before the framework is shutdown
		//Notify listeners of impending shutdown
		ModuleShutdownEvent.Broadcast();

		//Clear all of our callbacks!
		ClearAllCallbacks();

		FSubstanceSourceStyle::Shutdown();

		mCurrentSearchedAssetSet.clear();
		mCurrentSearchedAssetSet.shrink_to_fit();

		UserLogout();

		CurrentDownloads.Empty();

		mDatabase.reset();
		mDatabaseLoadingStatus = ESourceDatabaseLoadingStatus::NotLoaded;

		Alg::Source::Callbacks::setInstance(nullptr);
	}

	/** SourceModuleInterface - Gets database */
	Alg::Source::Database* GetDatabase() const override
	{
		return mDatabase.get();
	}

	/** SourceModuleInterface - Load Source Database */
	void LoadDatabase() override
	{
		//only allow loading the database if it hasn't been loaded yet
		if (mDatabaseLoadingStatus != ESourceDatabaseLoadingStatus::NotLoaded && mDatabaseLoadingStatus != ESourceDatabaseLoadingStatus::Error)
		{
			return;
		}

		mDatabaseLoadingStatus = ESourceDatabaseLoadingStatus::Loading;

		UE_LOG(LogSubstanceSource, Log, TEXT("Loading Substance Source Database"));

		mDatabaseFactory.createInstance([&](Alg::Source::SharedPtr<Alg::Source::Database> database)
		{
			mDatabase = database;

			if (database == nullptr)
			{
				UE_LOG(LogSubstanceSource, Warning, TEXT("Unable to instance Source database due to error"));
				LoadDatabaseFailed.Broadcast();
				mDatabaseLoadingStatus = ESourceDatabaseLoadingStatus::Error;
			}
			else
			{
				DatabaseLoadedEvent.Broadcast();
				mDatabaseLoadingStatus = ESourceDatabaseLoadingStatus::Loaded;

				if (mSubstanceSourceToolkit.IsValid())
				{
					mSubstanceSourceToolkit->EnableLoginButton();
				}
			}
		});
	}

	//Download a substance
	void LoadSubstance(Alg::Source::SharedPtr<Alg::Source::Asset> SourceAsset) override
	{
		//Make sure we're logged in:
		if (mUser == nullptr)
		{
			FText DialogTitle = LOCTEXT("PluginCreatedTitle", "Plugin Substance Source");
			FText DialogText = LOCTEXT("PluginCreatedText", "Please login to Substance Source.");
			FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
			return;
		}

		//Prevent double clicks or other oddities where we duplicate an asset download
		auto containsPred = [&SourceAsset](const auto & cmp)
		{
			return cmp.Asset == SourceAsset;
		};

		if (CurrentDownloads.ContainsByPredicate(containsPred))
		{
			return;
		}

		if (Alg::Source::DownloadAttachment* Download = SourceAsset->getDownloadAttachmentByMimeType(SUBSTANCE_SOURCE_MIMETYPE_SBSAR))
		{
			CurrentDownloads.Add({ SourceAsset, nullptr });

			//Craft the final path to either load from or download to if not present
			FString destinationFolder = FString(FPaths::ProjectDir() + "Substance/");
			FString finalSbsarPath = FString(destinationFolder + Download->getFilename().c_str());

			//Before starting the download, check to see if the .sbsar has already been downloaded to the download location. If so, load that.
			if (FPaths::FileExists(finalSbsarPath))
			{
				//Load that substance from disk
				FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

				USubstanceFactory::SuppressImportOverwriteDialog();

				//Add the name of the file to import
				TArray<FString> SourcePaths;
				SourcePaths.Add(finalSbsarPath);

				//Import settings
				UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();

				//Set up the data needed to import
				ImportData->bReplaceExisting = true;
				ImportData->Filenames = SourcePaths;
				ImportData->DestinationPath = ContentBrowserPath;
				ImportData->bSkipReadOnly = true;

				AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

				//notify listeners when an asset completes import and remove from set
				SuccessfulDownloads++;
				CurrentDownloads.RemoveAll([SourceAsset](const auto & Info)
				{
					return Info.Asset == SourceAsset;
				});

				return;
			}

			Alg::Source::DownloadAttachment::HandlePtr DownloadHandle = Download->loadAttachmentData(mUser, [this, finalSbsarPath, SourceAsset](Alg::Source::DownloadAttachment::DownloadAttachmentResult result, const Alg::Source::BinaryData & payload)
			{
				switch (result)
				{
				case Alg::Source::DownloadAttachment::DownloadAttachmentResult::Success:
					{
						//Convert payload to TArray
						TArray<uint8> RawData;
						RawData.Insert(&payload[0], payload.size(), 0);

						FFileHelper::SaveArrayToFile(RawData, *finalSbsarPath);

						//Import the file
						FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

						USubstanceFactory::SuppressImportOverwriteDialog();

						//Add the name of the file to import
						TArray<FString> SourcePaths;
						SourcePaths.Add(finalSbsarPath);

						//Import settings
						UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();

						//Set up the data needed to import
						ImportData->bReplaceExisting = true;
						ImportData->Filenames = SourcePaths;
						ImportData->DestinationPath = ContentBrowserPath;
						ImportData->bSkipReadOnly = true;

						AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

						//notify listeners when an asset completes import and remove from set
						SuccessfulDownloads++;
						CurrentDownloads.RemoveAll([SourceAsset](const auto & Info)
						{
							return Info.Asset == SourceAsset;
						});

						break;
					}
				case Alg::Source::DownloadAttachment::DownloadAttachmentResult::Purchased:
					{
						//Refresh the user to acquire new download count
						RefreshLogin(mUser->getRefreshToken());


						SubstancePurchasedEvent.Broadcast(SourceAsset);

						break;
					}
				case Alg::Source::DownloadAttachment::DownloadAttachmentResult::Failed:
				default:
					{
						if (mUser->getAssetPoints() <= 0)
						{
							ErrorPopup(" You do not have any downloads left.\n Subscribe to Substance to download 30 assets per month.\n If you are subscribed, wait for your renewal period to get new downloads.",
							           Alg::Source::HTTPStatus::NotFound, payload);
						}
						else
						{
							ErrorPopup("Substance download error!", Alg::Source::HTTPStatus::NotFound, payload);
						}

						//notify listeners a download failed and remove from set
						FailedDownloads++;
						CurrentDownloads.RemoveAll([SourceAsset](const auto & Info)
						{
							return Info.Asset == SourceAsset;
						});

						break;
					}
				}
			});

			//add download to list
			for (auto& Info : CurrentDownloads)
			{
				if (Info.Asset == SourceAsset)
				{
					Info.DownloadHandle = DownloadHandle;
					break;
				}
			}
		}
	}

	/** Constructs a UE4 Slate brush for an image from a given asset - Handles thumbnail, and details image brush creation */
	TSharedPtr<FSlateBrush> ConstructImageBrush(const char* AttachmentName, Alg::Source::AssetPtr asset) override
	{
		TSharedPtr<FSlateBrush> Brush;
		if (Alg::Source::ImageAttachment* image = asset->getImageAttachmentByLabel(AttachmentName))
		{
			const Alg::Source::BinaryData& imageData = image->getImageData();

			//Create a texture 2d out of the raw image data!
			UTexture2D* Texture = nullptr;

			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(&imageData.front(), imageData.size()))
			{
				TArray<uint8> UncompressedBGRA;
				if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
				{
					Texture = nullptr;

					TArray<FColor> ColorArr;
					ColorArr.AddUninitialized(UncompressedBGRA.Num() / 4);
					FMemory::Memcpy(&ColorArr[0], UncompressedBGRA.GetData(), UncompressedBGRA.Num());

					CacheImage(ColorArr, image->getDesiredImageWidth(), image->getDesiredImageHeight(), FString(asset->getTitle().c_str()) + "_" + FString(AttachmentName));

					FCreateTexture2DParameters ParamSet;
					ParamSet.CompressionSettings = TC_Default;
					ParamSet.bSRGB = true;
					ParamSet.bDeferCompression = true;
					Texture = FImageUtils::CreateTexture2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), ColorArr, GetTransientPackage(), FString(), RF_Standalone, ParamSet);
				}
			}

			//Create the slice brush
			Brush = MakeShareable(new FSlateBrush());

			//Create a slate brush from the texture
			Brush->SetResourceObject(Texture);
			Brush->ImageSize.X = image->getDesiredImageWidth();
			Brush->ImageSize.Y = image->getDesiredImageHeight();
			Brush->DrawAs = ESlateBrushDrawType::Image;
		}
		return Brush;
	}

	/** Returns the complete path to an image cache file given the expected files name */
	FString GetImageCachePath(const FString& AssetPath) const override
	{
		return FString::Printf(TEXT("%sRuntime/Substance/Cache/%s.cache"), *FPaths::EnginePluginsDir(), *(AssetPath));
	}

	/** Writes an image to cache */
	void CacheImage(TArray<FColor>& colorData, int32 width, int32 height, const FString& AssetName) const
	{
		FString filename = GetImageCachePath(AssetName);
		FArchive* Archive = IFileManager::Get().CreateFileWriter(*filename, 0);

		if (Archive)
		{
			int32 colorSize = colorData.Num();
			FArchive& Ar = *Archive;
			Ar << width;
			Ar << height;
			Ar << colorData;

			Ar.Close();

			//Cleanup archive
			delete Archive;
		}
	}

	virtual TSharedPtr<FSlateBrush> LoadImageFromCache(const FString& AssetName) override
	{
		TSharedPtr<FSlateBrush> Brush;
		FString filename = GetImageCachePath(AssetName);
		FArchive* Archive = IFileManager::Get().CreateFileReader(*filename, 0);

		if (Archive)
		{
			Brush = MakeShareable(new FSlateBrush());
			FArchive& Ar = *Archive;
			TArray<FColor> colorData;
			int32 width = 0;
			int32 height = 0;

			Ar << width;
			Ar << height;
			Ar << colorData;

			FCreateTexture2DParameters ParamSet;
			ParamSet.CompressionSettings = TC_Default;
			ParamSet.bSRGB = true;
			ParamSet.bDeferCompression = true;

			UTexture2D* TextureResource = FImageUtils::CreateTexture2D(width, height, colorData, GetTransientPackage(), FString(), RF_Standalone, ParamSet);

			Brush->SetResourceObject(TextureResource);
			Brush->ImageSize.X = width;
			Brush->ImageSize.Y = height;
			Brush->DrawAs = ESlateBrushDrawType::Image;

			//TODO:: Make sure to regenerate within the SubstanceSourceAsset
			//Regenerates now that the thumbnail brush is valid
			//RegenerateTileLayout();

			Ar.Close();

			//Cleanup archive
			delete Archive;
		}

		return Brush;
	}

	/** SourceModuleInterface - Query the image to load to store in the alg asset */
	void LoadThumbnail(TSharedRef<SWidget> Asset) override
	{
		TSharedRef<SSubstanceSourceAsset> SourceAsset = StaticCastSharedRef<SSubstanceSourceAsset>(Asset);
		Alg::Source::AssetPtr asset = SourceAsset->GetAsset();

		if (Alg::Source::ImageAttachment* image = asset->getImageAttachmentByLabel(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL))
		{
			SourceAsset->SetThumbnailLoaded(true);

			image->loadImageData([SourceAsset](Alg::Source::ImageAttachment::LoadImageResult result)
			{
				if (result != Alg::Source::ImageAttachment::LoadImageResult::Failed)
				{
					SourceAsset.Get().ConstructThumbnail();
				}
				else
				{
					SourceAsset->SetThumbnailLoaded(false);
				}
			});
		}
	}

	/** SourceModuleInterface - Query the image to load to store in the alg asset */
	void LoadDetailsSliced(TSharedRef<SWidget> Asset, const FVector2D& DesiredSize) override
	{
		TSharedRef<SSubstanceSourceDetails> SourceAsset = StaticCastSharedRef<SSubstanceSourceDetails>(Asset);
		Alg::Source::AssetPtr asset = SourceAsset->GetAsset();

		if (Alg::Source::ImageAttachment* image = asset->getImageAttachmentByLabel(SUBSTANCE_SOURCE_PREVIEW_IMAGE_SLICES))
		{
			image->setDesiredImageWidth((int)DesiredSize.X);
			image->setDesiredImageHeight((int)DesiredSize.Y);

			image->loadImageData([SourceAsset](Alg::Source::ImageAttachment::LoadImageResult result)
			{
				if (result != Alg::Source::ImageAttachment::LoadImageResult::Failed)
				{
					SourceAsset.Get().ConstructDetailsSlicesImage();
				}
			});
		}
	}

	void LoadDetailsMaps(TSharedRef<SWidget> Asset, const FVector2D& DesiredSize) override
	{
		TSharedRef<SSubstanceSourceDetails> SourceAsset = StaticCastSharedRef<SSubstanceSourceDetails>(Asset);
		Alg::Source::AssetPtr asset = SourceAsset->GetAsset();

		if (Alg::Source::ImageAttachment* image = asset->getImageAttachmentByLabel(SUBSTANCE_SOURCE_PREVIEW_IMAGE_OUTPUTS))
		{
			image->setDesiredImageWidth((int)DesiredSize.X);
			image->setDesiredImageHeight((int)DesiredSize.Y);

			image->loadImageData([SourceAsset](Alg::Source::ImageAttachment::LoadImageResult result)
			{
				if (result != Alg::Source::ImageAttachment::LoadImageResult::Failed)
				{
					SourceAsset.Get().ConstructDetailsMapsImage();
				}
			});
		}
	}

	/** SourceModuleInterface - Query the status of the Database Load */
	ESourceDatabaseLoadingStatus GetDatabaseLoadingStatus() const override
	{
		return mDatabaseLoadingStatus;
	}

	/** Initiate User Login */
	virtual void UserLogin(const FString& Username, const FString& Password) override
	{
		if (mUserLoginStatus == ESourceUserLoginStatus::LoggingIn)
		{
			UE_LOG(LogSubstanceSource, Error, TEXT("UserLogin called while prior request still in flight. Ignoring subsequent request"));
			return;
		}

		if (mDatabase == nullptr)
		{
			UE_LOG(LogSubstanceSource, Error, TEXT("UserLogin cannot be invoked before Database has been loaded"));
			return;
		}

		//log out previous user
		UserLogout();

		mUserLoginStatus = ESourceUserLoginStatus::LoggingIn;

		mUserFactory.createInstance(TCHAR_TO_UTF8(*Username), TCHAR_TO_UTF8(*Password), mDatabase,
		                            [this](Alg::Source::SharedPtr<Alg::Source::User> user, const Alg::Source::HTTPResponse & response)
		{
			if (user != nullptr)
			{
				mUser = user;
				mUserLoginStatus = ESourceUserLoginStatus::LoggedIn;
				mUserLoginTimer = 0.0f;
				LoggedInEvent.Broadcast();
				UE_LOG(LogSubstanceSource, Log, TEXT("User Logged In Successfully"));
			}
			else
			{
				mUserLoginStatus = ESourceUserLoginStatus::Failed;
				LoginFailedEvent.Broadcast();
				UE_LOG(LogSubstanceSource, Log, TEXT("User Login Failed"));

				ErrorPopup("User Login Failed", response.getResponseCode());
			}
		});
	}

	virtual void RefreshLogin(const Alg::Source::String& RefreshToken) override
	{
		mUserFactory.createInstance(RefreshToken, mDatabase,
		                            [this](Alg::Source::SharedPtr<Alg::Source::User> user,
		                                   const Alg::Source::HTTPResponse & response)
		{
			if (user != nullptr)
			{
				mUser = user;
				mUserLoginStatus = ESourceUserLoginStatus::LoggedIn;
				mUserLoginTimer = 0.0f;
				UE_LOG(LogSubstanceSource, Log, TEXT("Refresh Login Successful"));
			}
			else
			{
				mUserLoginStatus = ESourceUserLoginStatus::Failed;
				LoginFailedEvent.Broadcast();
				UE_LOG(LogSubstanceSource, Log, TEXT("Refresh Login Failed"));

				ErrorPopup("User Refresh Login Failed", response.getResponseCode());
			}
		});
	}

	virtual void ManageLoginTimer(const double InCurrentTime) override // 'InCurrentTime' is in seconds
	{
		if (mUserLoginStatus == ESourceUserLoginStatus::LoggedIn)
		{
			bool bUserLoginTimer = false;

			if (mUserLoginTimer == 0.0f)
			{
				// User just logged in, or login refresh just occurred
				bUserLoginTimer = true;
			}
			else if (InCurrentTime > mUserLoginTimer)
			{
				RefreshLogin(mUser->getRefreshToken());
				bUserLoginTimer = true;
			}

			if (bUserLoginTimer)
			{
				// Reset "timeout" delay
				mUserLoginTimer = InCurrentTime + (27.0f * 60.0f);
			}
		}
	}

	virtual void UserLogout() override
	{
		mUser.reset();
		mUserLoginStatus = ESourceUserLoginStatus::LoggedOut;
		LoggedOutEvent.Broadcast();
	}

	virtual ESourceUserLoginStatus GetUserLoginStatus() const override
	{
		return mUserLoginStatus;
	}

	virtual int32 GetUserAvailableDownloads() override
	{
		if (!mUser.get())
		{
			return 0;
		}

		return mUser->getAssetPoints();
	}

	/** Checks to see if an asset is owned by the user */
	virtual bool IsAssetOwned(Alg::Source::AssetPtr Asset) const override
	{
		if (nullptr == mUser)
		{
			return false;
		}

		return mUser->hasOwnedAsset(Asset);
	}

	/** Returns number of successful downloads */
	virtual int GetNumSuccessfulDownloads() const override
	{
		return SuccessfulDownloads;
	}

	/** Returns number of failed downloads */
	virtual int GetNumFailedDownloads() const override
	{
		return FailedDownloads;
	}

	/** Get information on the currently active download (if applicable) */
	virtual bool GetCurrentDownloadInformation(int& NumDownloads, Alg::Source::String& AssetTitle, float& PercentDownloaded) override
	{
		NumDownloads = CurrentDownloads.Num();

		if (CurrentDownloads.Num() > 0)
		{
			AssetTitle = CurrentDownloads[0].Asset->getTitle();
			Alg::Source::DownloadAttachment::HandlePtr DownloadHandle = CurrentDownloads[0].DownloadHandle;

			if (DownloadHandle->isValid())
			{
				if (DownloadHandle->getPayloadLength() == 0)
				{
					PercentDownloaded = 1.0f;
				}
				else
				{
					PercentDownloaded = (float)DownloadHandle->getProgressHandle()->getBytesReceived() / (float)DownloadHandle->getPayloadLength();
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	/** Create access point to Substance Source utility (in LevelEditor toolbar) */
	TSharedPtr<FUICommandList> CommandList;

	/** Callback for when the substance source button is pressed */
	void OnLaunchSubstanceSourceWindow()
	{
		//Check Entitlement
		if (IsEntitled())
		{
			Alg::Source::Platform::GetInstance().SetSourceIntegrationID("Jy3fpB8TaPmb2v8hyL9J4tYbP2FsGgE8");
		}
		else
		{
			Alg::Source::Platform::GetInstance().SetSourceIntegrationID("Ch4Mu7sZ5BA2HPqdRUEyrHqacYhqEczy");
		}

		if (!mSubstanceSourceToolkit.IsValid())
		{
			//Clear all of our callbacks in case there are any still bound
			ClearAllCallbacks();
			mSubstanceSourceToolkit = CreateSubstanceSourceWindow(TSharedPtr<IToolkitHost>());
		}
		else
		{
			mSubstanceSourceToolkit->BringToolkitToFront();
			mSubstanceSourceToolkit->FocusWindow();
		}
	}

	void CreateToolbarButton()
	{
		FSubstanceSourceModuleCommands::Register(); //

		// Bind command
		CommandList = MakeShareable(new FUICommandList);

		struct Local
		{
			static void AddToolbarButton(FToolBarBuilder& ToolbarBuilder)
			{
				//NOTE:: Leave spacing in names for auto padding - Might be benificial to alter this later to lock in a solid size
				//and to remove string size
				ToolbarBuilder.AddToolBarButton(FSubstanceSourceModuleCommands::Get().LaunchSubstanceSourceWindow,
				                                NAME_None,
				                                FText::FromString("   Source   "),
				                                FText::FromString("Substance Source"),
				                                FSlateIcon("SubstanceSourceStyle", "SubstanceSourceButtonIcon")
				                               );
			}
		};

		const FSubstanceSourceModuleCommands& Commands = FSubstanceSourceModuleCommands::Get();

		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

		ToolbarExtender->AddToolBarExtension(
		    "Content",
		    EExtensionHook::After,
		    CommandList,
		    FToolBarExtensionDelegate::CreateStatic(&Local::AddToolbarButton)
		);

		CommandList->MapAction(
		    Commands.LaunchSubstanceSourceWindow,
		    FExecuteAction::CreateRaw(this, &FSubstanceSourceModule::OnLaunchSubstanceSourceWindow),
		    FCanExecuteAction()
		);

		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	// Message (modal) popup utility function:
	void ErrorPopup(const Alg::Source::String& ErrorMessage, Alg::Source::HTTPStatus ResponseCode = Alg::Source::HTTPStatus::Unknown, const Alg::Source::BinaryData& ResponseData = Alg::Source::BinaryData())
	{
		FString text = ErrorMessage.c_str();
		bool bShowMoreInfo = false; // false: user friendly error message, true: with more HTTP info

		if (bShowMoreInfo)
		{
			if (ResponseCode != Alg::Source::HTTPStatus::Unknown)
			{
				text += "\n\nHTTP return code is: " + FString::FromInt((int)ResponseCode);
				int minLen = 2, maxLen = 40;

				if ((ResponseData.size() >= minLen) &&
				        (ResponseData.size() <= maxLen))
				{
					//Create a string from a byte array!
					Alg::Source::String str(ResponseData.begin(), ResponseData.end());

					text += "\n";
					text += str.c_str();
				}
			}
		}

		FText DialogTitle = FText::FromString("Plugin Substance Source");
		FText DialogText = FText::FromString(text);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
	}

	/** Listener call back for when the content browser switches paths */
	void OnContentBrowserPathSwitch(const FString& Path)
	{
		ContentBrowserPath = Path;
	}

	/** Called from the toolkit when the window has closed */
	virtual void OnSubstanceSourceWindowClosed() override
	{
		ClearThumbnailCacheQueue();
		mSubstanceSourceToolkit->CloseWindow();
		mSubstanceSourceToolkit.Reset();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}

	/** Called with an updated string to search for to get assets from */
	virtual void SearchStringChanged(const FString& SearchedString) override
	{
		mCurrentSearchedAssetSet.clear();

		//Search database with the new and current user entered search string if the search string is valid
		if (!SearchedString.IsEmpty())
		{
			mCurrentSearchedAssetSet = mDatabase->searchAssets(TCHAR_TO_ANSI(*SearchedString));
		}

		//Let the UI know it needs to be updated with the new asset vector
		SearchCompletedEvent.Broadcast();
	}

	/** Returns the current searched for asset vector */
	virtual Alg::Source::AssetVector GetSearchResults() override
	{
		return mCurrentSearchedAssetSet;
	}

	/** Returns a list of all assets currently purchased by the user */
	virtual const Alg::Source::AssetVector& GetUserOwnedAssets() override
	{
		if (!mUser)
		{
			UE_LOG(LogSubstanceSource, Error, TEXT("Attempted to get owned assets from an invalid user account!"))
			static Alg::Source::AssetVector __emptyAssets;
			return __emptyAssets;
		}

		return mUser->getOwnedAssets();
	}

	/** Lets the front-end know it needs to display user's owned assets */
	virtual void DisplayUsersOwnedAssets() override
	{
		DisplayUserAssetsEvent.Broadcast();
	}

	/** Notify handlers that an asset category change was requested */
	virtual void DisplayAssetCategory(const FString& Category) override
	{
		DisplayCategoryEvent.Broadcast(Category);
	}

	/** Notify front end that the category has changed */
	virtual void OnCategorySwitched() override
	{
		CategoryChanged.Broadcast();
	}

private:
	/** Events to fire when framework events occur */
	FUserLoggedInEvent LoggedInEvent;
	FUserLoggedOutEvent LoggedOutEvent;
	FUserLoginFailedEvent LoginFailedEvent;
	FDatabaseFailedEvent LoadDatabaseFailed;
	FDatabaseLoaded DatabaseLoadedEvent;
	FNewSearchedAssetsSet SearchCompletedEvent;
	FDisplayMyAssets DisplayUserAssetsEvent;
	FDisplayCategory DisplayCategoryEvent;
	FSubstancePurchased SubstancePurchasedEvent;
	FModuleShutdown ModuleShutdownEvent;
	FCategoryChanged CategoryChanged;

	/** Source Framework Elements */
	Alg::Source::SharedPtr<Alg::Source::Database>	mDatabase;
	Alg::Source::SharedPtr<Alg::Source::User>		mUser;
	Alg::Source::DatabaseFactory					mDatabaseFactory;
	Alg::Source::UserFactory						mUserFactory;
	Alg::Source::AssetVector						mCurrentSearchedAssetSet;
	SourceCallbacks									mSourceCallbacks;
	ESourceDatabaseLoadingStatus					mDatabaseLoadingStatus;
	ESourceUserLoginStatus							mUserLoginStatus;
	TSharedPtr<FSubstanceToolkit>					mSubstanceSourceToolkit;

	double mUserLoginTimer;

	/** Gets the extensibility managers for outside entities to extend static mesh editor's menus */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager()
	{
		return MenuExtensibilityManager;
	}

	/** Gets the extensibility managers for outside entities to extend static mesh editor's tool bars */
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager()
	{
		return ToolBarExtensibilityManager;
	}

	/** Creates an instance of the source window */
	virtual TSharedRef<FSubstanceToolkit> CreateSubstanceSourceWindow(const TSharedPtr<IToolkitHost>& InitToolkitHost) override
	{
		TSharedRef<FSubstanceSourceWindow> NewSourceWindow(new FSubstanceSourceWindow());
		NewSourceWindow->InitSubstanceSourceWindow(InitToolkitHost);
		return NewSourceWindow;
	}

	/** Add asset to load from cache queue */
	virtual void QueueLoadThumbnailCache(TSharedRef<SWidget> Asset) override
	{
		ThumbnailCacheQueue.AddHead(Asset);

		TSharedRef<SSubstanceSourceAsset> SourceAsset = StaticCastSharedRef<SSubstanceSourceAsset>(Asset);
		SourceAsset->SetThumbnailLoaded(true);
	}

	/** Called to move the assets within paint to the front of the queue */
	virtual void PrioritizeThumbnailCacheLoading(TSharedRef<SWidget> Asset) override
	{
		ThumbnailPriorityTransferArray.Add(Asset);

		TSharedRef<SSubstanceSourceAsset> SourceAsset = StaticCastSharedRef<SSubstanceSourceAsset>(Asset);
		SourceAsset->SetPrioritizedCache();
	}

	/** Removes an asset from load queue as it has been destroyed and no longer needs loaded */
	virtual void ClearThumbnailCacheQueue() override
	{
		ThumbnailCacheQueue.Empty();
	}

	/** Polling function that processes thumbnail cache queue */
	virtual void ProcessThumbnailCacheQueue() override
	{
		if (ThumbnailPriorityTransferArray.Num())
		{
			//Reverse the order
			Algo::Reverse(ThumbnailPriorityTransferArray);
			for (const auto& QueueItr : ThumbnailPriorityTransferArray)
			{
				ThumbnailCacheQueue.RemoveNode(QueueItr);
				ThumbnailCacheQueue.AddTail(QueueItr);
			}
		}

		//Clear the transfer as we won't need to shift these again
		ThumbnailPriorityTransferArray.Empty();

		//Process main queue
		if (ThumbnailCacheQueue.Num())
		{
			TSharedRef<SWidget> BaseWidget = ThumbnailCacheQueue.GetTail()->GetValue();
			TSharedRef<SSubstanceSourceAsset> SourceAsset = StaticCastSharedRef<SSubstanceSourceAsset>(BaseWidget);
			SourceAsset->OnLoadCachedThumbnail().Broadcast();
			ThumbnailCacheQueue.RemoveNode(BaseWidget);
		}
	}

private:

	/** Unbinds all of the delegates from all of our events */
	void ClearAllCallbacks()
	{
		LoggedInEvent.Clear();
		LoggedOutEvent.Clear();
		LoginFailedEvent.Clear();
		LoadDatabaseFailed.Clear();
		DatabaseLoadedEvent.Clear();
		SearchCompletedEvent.Clear();
		DisplayUserAssetsEvent.Clear();
		DisplayCategoryEvent.Clear();
		SubstancePurchasedEvent.Clear();
		ModuleShutdownEvent.Clear();
		CategoryChanged.Clear();
	}

	/** Managers used to be able to extend to different UI elements for this module */
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	/** The current string associated with the content browser */
	FString ContentBrowserPath;

	/** Download Information */
	struct CurrentDownloadInfo
	{
		Alg::Source::AssetPtr						Asset;
		Alg::Source::DownloadAttachment::HandlePtr	DownloadHandle;
	};

	TDoubleLinkedList<TSharedRef<SWidget>> ThumbnailCacheQueue;
	TArray<TSharedRef<SWidget>> ThumbnailPriorityTransferArray;
	TArray<CurrentDownloadInfo> CurrentDownloads;
	int32 SuccessfulDownloads;
	int32 FailedDownloads;
};

/** Accessor for the module */
ISubstanceSourceModule* ISubstanceSourceModule::Get()
{
	return &FModuleManager::LoadModuleChecked<FSubstanceSourceModule>("SubstanceSource");
}

//==================================================
IMPLEMENT_MODULE(FSubstanceSourceModule, SubstanceSource);

#undef LOCTEXT_NAMESPACE
