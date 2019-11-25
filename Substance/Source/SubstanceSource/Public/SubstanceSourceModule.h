// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceModule.h
#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "SubstanceToolkit.h"
#include "ISubstanceSource.h"

#include <substance/source/attachment.h>
#include <substance/source/database.h>


namespace SubstanceSourceModule
{
extern const FName SubstanceSourceAppIdentifier;
}

enum class ESourceDatabaseLoadingStatus
{
	NotLoaded,		//LoadDatabase has not been called yet
	Loading,		//Database is still loading
	Loaded,			//Database has been loaded successfully
	Error,			//Database loading had an error
};

enum class ESourceUserLoginStatus
{
	LoggedOut,		//User is logged out
	LoggingIn,		//User is logging in, wait for response
	LoggedIn,		//User is logged in
	Failed,			//User failed to log in on the last attempt
};

/**/

class ISubstanceSourceModule : public IModuleInterface
{
public:
	/** Generic Accessor to be able to access to module from other classes */
	static ISubstanceSourceModule* Get();

	/** Retrieve Source Database Object, can be NULL */
	virtual Alg::Source::Database* GetDatabase() const = 0;

	/** Call this when you want to begin loading the Source database */
	virtual void LoadDatabase() = 0;

	/** Query the status of the Database Load */
	virtual ESourceDatabaseLoadingStatus GetDatabaseLoadingStatus() const = 0;

	/** Polling function that processes thumbnail cache queue */
	virtual void ProcessThumbnailCacheQueue() = 0;

	/** Initiate user login */
	virtual void UserLogin(const FString& Username, const FString& Password) = 0;
	virtual void RefreshLogin(const Alg::Source::String& RefreshToken) = 0;
	virtual void ManageLoginTimer(const double InCurrentTime) = 0;

	/** Logout current user */
	virtual void UserLogout() = 0;

	/** Get Substance data */
	virtual void LoadSubstance(Alg::Source::SharedPtr<Alg::Source::Asset> Asset) = 0;

	/** Get asset thumbnail data */
	virtual void LoadThumbnail(TSharedRef<SWidget> Asset) = 0;

	/** Add asset to load from cache queue */
	virtual void QueueLoadThumbnailCache(TSharedRef<SWidget> Asset) = 0;

	/** Called to move the assets within paint to the front of the queue */
	virtual void PrioritizeThumbnailCacheLoading(TSharedRef<SWidget> Asset) = 0;

	/** Removes an asset from load queue as it has been destroyed and no longer needs loaded */
	virtual void ClearThumbnailCacheQueue() = 0;

	/** Pulls the texture that shows the final sbsar results */
	virtual void LoadDetailsSliced(TSharedRef<SWidget> Asset, const FVector2D& DesiredSize) = 0;

	/** Pulls the texture that contains the various map examples */
	virtual void LoadDetailsMaps(TSharedRef<SWidget> Asset, const FVector2D& DesiredSize) = 0;

	/** Resets our reference to the window when the window is no longer active */
	virtual void OnSubstanceSourceWindowClosed() = 0;

	/** Called when a category has been changed. Fires the event letting other areas of the front end know */
	virtual void OnCategorySwitched() = 0;

	/** Called with an updated string to search for to get assets from */
	virtual void SearchStringChanged(const FString& SearchedString) = 0;

	/** Returns the complete path to an image cache file given the expected files name */
	virtual FString GetImageCachePath(const FString& AssetPath) const = 0;

	/** Query the status of the user login */
	virtual ESourceUserLoginStatus GetUserLoginStatus() const = 0;

	/** Returns the amount of asset tokens the current user has available with their account */
	virtual int32 GetUserAvailableDownloads() = 0;

	/** Creates the source window */
	virtual TSharedRef<FSubstanceToolkit> CreateSubstanceSourceWindow(const TSharedPtr<IToolkitHost>& InitToolkitHost) = 0;

	/** Returns the current searched for asset vector */
	virtual Alg::Source::AssetVector GetSearchResults() = 0;

	/** Returns a list of all assets currently purchased by the user */
	virtual const Alg::Source::AssetVector& GetUserOwnedAssets() = 0;

	/** Called when the display my assets button is pressed */
	virtual void DisplayUsersOwnedAssets() = 0;

	/** Called when you wish to select a new asset category */
	virtual void DisplayAssetCategory(const FString& Category) = 0;

	/** Checks to see if an asset is owned by the user */
	virtual bool IsAssetOwned(Alg::Source::AssetPtr Asset) const = 0;

	/** Returns number of successful downloads */
	virtual int GetNumSuccessfulDownloads() const = 0;

	/** Returns number of failed downloads */
	virtual int GetNumFailedDownloads() const = 0;

	/** Get information on the currently active download (if applicable) */
	virtual bool GetCurrentDownloadInformation(int& NumDownloads, Alg::Source::String& AssetTitle, float& PercentDownloaded) = 0;

	/** Checks plugin entitlement*/
	virtual bool IsEntitled() = 0;

	/** Constructs a UE4 Slate brush for an image from a given asset - Handles thumbnail, and details image brush creation */
	virtual TSharedPtr<FSlateBrush> ConstructImageBrush(const char* AttachmentName, Alg::Source::AssetPtr asset) = 0;

	/** Creates a slate brush for an image given an asset from caches data */
	virtual TSharedPtr<FSlateBrush> LoadImageFromCache(const FString& AssetName) = 0;

	/** Create the login callback that other classes will use to register function to call when event triggers */
	DECLARE_EVENT(FSubstanceSourceModule, FUserLoggedInEvent)
	virtual FUserLoggedInEvent& OnLoggedIn() = 0;

	/** Create the logout callback that other classes will use to register function to call when event triggers */
	DECLARE_EVENT(FSubstanceSourceModule, FUserLoggedOutEvent)
	virtual FUserLoggedOutEvent& OnLoggedOut() = 0;

	/** Create the login failed callback that other classes will use to register function to call when event triggers */
	DECLARE_EVENT(FSubstanceSourceModule, FUserLoginFailedEvent)
	virtual FUserLoginFailedEvent& OnLoginFailed() = 0;

	/** Create the load database error callback that other classes will use to register function to call when event triggers */
	DECLARE_EVENT(FSubstanceSourceModule, FDatabaseFailedEvent)
	virtual FDatabaseFailedEvent& OnDatabaseError() = 0;

	/** Create the load database error callback that other classes will use to register function to call when event triggers */
	DECLARE_EVENT(FSubstanceSourceModule, FDatabaseLoaded)
	virtual FDatabaseLoaded& OnDatabaseLoaded() = 0;

	/** Send the new asset vector to the front end to update the available assets search for */
	DECLARE_EVENT(FSubstanceSourceModule, FNewSearchedAssetsSet)
	virtual FNewSearchedAssetsSet& OnSearchCompleted() = 0;

	/** Event fired to tell the front-end to display all of the assets that the user currently owns */
	DECLARE_EVENT(FSubstanceSourceModule, FDisplayMyAssets)
	virtual FDisplayMyAssets& OnDisplayUsersAssets() = 0;

	/** Event fired to tell the front-end to display a new asset category */
	DECLARE_EVENT_OneParam(FSubstanceSourceModule, FDisplayCategory, const FString&)
	virtual FDisplayCategory& OnDisplayCategory() = 0;

	/** Event fired whenever an asset is purchased by the user */
	DECLARE_EVENT_OneParam(FSubstanceSourceModule, FSubstancePurchased, Alg::Source::AssetPtr)
	virtual FSubstancePurchased& OnSubstancePurchased() = 0;

	/** Event fired when the source module is preparing to shut down */
	DECLARE_EVENT(FSubstanceSourceModule, FModuleShutdown)
	virtual FModuleShutdown& OnModuleShutdown() = 0;

	/** Event fired when the selected category has changed*/
	DECLARE_EVENT(FSubstanceSourceModule, FCategoryChanged)
	virtual FCategoryChanged& OnCategoryChanged() = 0;
};
