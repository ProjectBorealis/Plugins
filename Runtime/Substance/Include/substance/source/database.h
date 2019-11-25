//! @file database.h
//! @brief Substance Source Database Object
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Manages the Substance Source Database
#ifndef _SUBSTANCE_SOURCE_DATABASE_H_
#define _SUBSTANCE_SOURCE_DATABASE_H_
#pragma once

#include <substance/source/asset.h>

namespace Alg
{
namespace Source
{

typedef SharedPtr<class Database>		DatabasePtr;
typedef std::pair<String, AssetVector>	CategoryAssetPair;
typedef Vector<CategoryAssetPair>		CategoryAssetVector;

/**/
class Database
{
public:
	/** SearchWeight is used to boost aspects of an asset for search purposes */
	enum class SearchWeight
	{
		Author		= 1,
		Tags		= 5,
		Title		= 10,
	};

	static const char* sCategoryAllAssets;
	static const char* sCategoryNewAssets;
	static const char* sCategoryFreeAssets;
	static const char* sUE4DefaultFreeAssets;
	static const char* sUE4StudioFreeAssets;

public:
	Database(AssetVector&& assets);

	/** Retrieve Asset by ID */
	AssetPtr getAssetById(const String& id) const;

	/** For a given search phrase, return a vector of Assets that conform to the search terms */
	AssetVector searchAssets(const String& phrase) const;

	/** Get Category Asset Vector, which contains a category name and a list of assets */
	inline const CategoryAssetVector& getCategoryAssetVector() const;

private:
	/** Index an asset for search */
	void indexAsset(AssetPtr asset);

	/** Index a string for an asset with a weighted score */
	void indexSearchPhrase(const String& phrase, AssetPtr asset, SearchWeight weight);

private:

	/** List of all assets in database */
	AssetVector												mAssets;

	/** Assets organized by category */
	CategoryAssetVector										mCategoryAssets;

	/** Assets organized by search phrases and weight */
	typedef std::pair<AssetPtr, SearchWeight>				WeightedAssetPtr;
	typedef Set<WeightedAssetPtr>							WeightedAssetSet;
	typedef Map<String, WeightedAssetSet>					WeightedSearchAssetMap;
	WeightedSearchAssetMap									mWeightedSearchAssetMap;
};

/**/
inline const CategoryAssetVector& Database::getCategoryAssetVector() const
{
	return mCategoryAssets;
}

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_DATABASE_H_
