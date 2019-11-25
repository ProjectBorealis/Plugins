//! @file asset.h
//! @brief Substance Source Asset Object
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Contains details on a Substance Source Asset
#ifndef _SUBSTANCE_SOURCE_ASSET_H_
#define _SUBSTANCE_SOURCE_ASSET_H_
#pragma once

#include <substance/source/iattachment.h>

namespace Alg
{
namespace Source
{

class DownloadAttachment;
class ImageAttachment;

typedef WeakPtr<class Asset>							AssetWeakPtr;
typedef SharedPtr<class Asset>							AssetPtr;
typedef Set<AssetPtr>									AssetSet;
typedef Vector<AssetPtr>								AssetVector;

/**/
class Asset : public std::enable_shared_from_this<Asset>
{
public:
	Asset(const String& id, const String& title, const String& author, const String& category, const std::tm& createdAt,
	      const std::tm& updatedAt, Vector<String>&& tags, Vector<String>&& groups, IAttachmentVector&& attachments, bool isNew, bool isRecentlyUpdated, int cost);

	/** Asset is non copyable */
	Asset(const Asset& cpy) = delete;

	/** Retrieve Asset ID value */
	inline const String& getID() const;

	/** Retrieve Asset Title */
	inline const String& getTitle() const;

	/** Retrieve Asset Author */
	inline const String& getAuthor() const;

	/** Retrieve Asset Category */
	inline const String& getCategory() const;

	/** Retrieve Asset "createdAt" */
	inline const std::tm& getCreatedAt() const;

	/** Retrieve Asset "updatedAt" */
	inline const std::tm& getUpdatedAt() const;

	/** Is this a new asset? */
	inline bool isNewAsset() const;

	/** Has this asset been recently updated? */
	inline bool isRecentlyUpdatedAsset() const;

	/** Retrieve Asset Cost */
	inline int getCost() const;

	/** Retrieve Asset "tags" */
	inline const Vector<String>& getTags() const;

	/** Retrieve Asset "groups" */
	inline const Vector<String>& getGroups() const;

	/** Is Asset free from promotion */
	bool getIsPromotionallyFree() const;

	/** Retrieve download attachment by mime-type */
	DownloadAttachment* getDownloadAttachmentByMimeType(const String& mimeType) const;

	/** Retrieve image attachment by label */
	ImageAttachment* getImageAttachmentByLabel(const String& label) const;

private:
	String						mID;
	String						mTitle;
	String						mAuthor;
	String						mCategory;
	std::tm						mCreatedAt;
	std::tm						mUpdatedAt;
	Vector<String>				mTags;
	Vector<String>				mGroups;
	IAttachmentVector			mAttachments;
	bool						mIsNew;
	bool						mIsRecentlyUpdated;
	int							mCost;

	bool						mPurchasedAsset;
};

/**/
inline const String& Asset::getID() const
{
	return mID;
}

inline const String& Asset::getTitle() const
{
	return mTitle;
}

inline const String& Asset::getAuthor() const
{
	return mAuthor;
}

inline const String& Asset::getCategory() const
{
	return mCategory;
}

inline const tm& Asset::getCreatedAt() const
{
	return mCreatedAt;
}

inline const tm& Asset::getUpdatedAt() const
{
	return mUpdatedAt;
}

inline bool Asset::isNewAsset() const
{
	return mIsNew;
}

inline bool Asset::isRecentlyUpdatedAsset() const
{
	return mIsRecentlyUpdated;
}

inline int Asset::getCost() const
{
	return mCost;
}

inline const Vector<String>& Asset::getTags() const
{
	return mTags;
}

inline const Vector<String>& Asset::getGroups() const
{
	return mGroups;
}

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_ASSET_H_
