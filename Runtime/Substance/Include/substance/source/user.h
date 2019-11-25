//! @file user.h
//! @brief Substance Source User Object
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Represents a logged in User
#ifndef _SUBSTANCE_SOURCE_USER_H_
#define _SUBSTANCE_SOURCE_USER_H_
#pragma once

namespace Alg
{
namespace Source
{

/**/
typedef SharedPtr<class User> UserPtr;

/**/
typedef std::function<void(bool)> UserRefreshCallback;

/**/
class User
{
public:
	User(const String& id, const String& name, const String& refreshToken,
	     const String& accessToken, const time_t expireTime, AssetVector&& ownedAssets, int assetPoints);

	/** Get user ID */
	inline const String& getID() const;

	/** Get name */
	inline const String& getName() const;

	/** Get accessToken */
	inline const String& getAccessToken() const;

	/** Set accessToken */
	inline void setAccessToken(const String& token);

	/** Get list of owned assets */
	inline const AssetVector& getOwnedAssets() const;

	/** Does user own a specific asset? */
	bool hasOwnedAsset(AssetPtr asset) const;

	/** Returns the number of assets the user can download */
	inline int getAssetPoints() const;

	/** Add an owned asset */
	inline void addOwnedAsset(AssetPtr asset);

	/** Returns the users token expire time */
	inline time_t getExpireTime() const;

	/** Set expireTime*/
	inline void setExpireTime(const time_t time);

	/** Returns the users refresh token*/
	inline const String& getRefreshToken() const;

private:
	String			mId;
	String			mName;
	String			mRefreshToken;
	String			mAccessToken;
	time_t			mExpireTime;
	int				mAssetPoints;

	AssetVector		mOwnedAssets;
};

inline const String& User::getID() const
{
	return mId;
}

inline const String& User::getName() const
{
	return mName;
}

inline const String& User::getAccessToken() const
{
	return mAccessToken;
}

inline void User::setAccessToken(const String& token)
{
	mAccessToken = token;
}

inline const AssetVector& User::getOwnedAssets() const
{
	return mOwnedAssets;
}

inline int User::getAssetPoints() const
{
	return mAssetPoints;
}

inline void User::addOwnedAsset(AssetPtr asset)
{
	mOwnedAssets.push_back(asset);
	if (asset->getCost() != 0 && !asset->getIsPromotionallyFree() && mAssetPoints > 0)
		mAssetPoints--;
}

inline time_t User::getExpireTime() const
{
	return mExpireTime;
}

inline void User::setExpireTime(const time_t time)
{
	mExpireTime = time;
}

inline const String& User::getRefreshToken() const
{
	return mRefreshToken;
}
} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_USER_H_
