//! @file attachment.h
//! @brief Substance Source Attachment
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Attachment Implementations
#ifndef _SUBSTANCE_SOURCE_ATTACHMENT_H_
#define _SUBSTANCE_SOURCE_ATTACHMENT_H_
#pragma once

#include <substance/source/iattachment.h>
#include <substance/source/ihttprequest.h>
#include <substance/source/user.h>

namespace Alg
{
namespace Source
{

class Asset;

/** Base Attachment */
class BaseAttachment : public IAttachment
{
public:
	BaseAttachment(const String& label, const String& url);

	/** From IAttachment */
	virtual void onAcquire(Asset* asset) override;
	virtual Asset* getAsset() const override;
	virtual const String& getLabel() const override;
	virtual const String& getURL() const override;

private:
	Asset*				mAsset;

	String				mLabel;
	String				mURL;
};

/** Download Attachment, which require purchase by the user */
class DownloadAttachment : public BaseAttachment
{
public:
	/** Result code for ImageAttachment loading */
	enum class DownloadAttachmentResult
	{
		Success,
		Failed,
		Purchased,
	};

	/** Callback when image data is loaded */
	typedef std::function<void(DownloadAttachmentResult, const BinaryData&)> DownloadAttachmentCallback;

	/** Helper class used to track a download */
	class Handle
	{
		friend class DownloadAttachment;
	public:
		Handle();

		/** Query if this handle has been initialized properly */
		inline bool isValid() const;

		/** Get current HTTP Request for this download */
		inline IHTTPRequest::ProgressHandlePtr getProgressHandle() const;

		/** Get payload length of response */
		inline int getPayloadLength() const;

	private:
		void init(IHTTPRequest::ProgressHandlePtr progressHandle, int payloadLength);

	private:
		IHTTPRequest::ProgressHandlePtr		mProgressHandle;
		int									mPayloadLength;
	};

	typedef SharedPtr<Handle> HandlePtr;

public:
	DownloadAttachment(const String& label, const String& url, const String& filename, const String& mimeType);

	/** Get download filename */
	const String& getFilename() const;

	/** Get download mime-type */
	const String& getMimeType() const;

	/** Download asset data (latent function) */
	HandlePtr loadAttachmentData(UserPtr user, DownloadAttachmentCallback callback) const;

	/** From IAttachment */
	virtual const char* getType() const override;

	/** Get RTTI type */
	static const char* getTypeStatic();

private:
	static void downloadAttachment(UserPtr user, DownloadAttachmentCallback callback, HandlePtr handle, const String& url);

private:
	String			mFilename;
	String			mMimeType;
};

/** These macros define commonly used image names within source frontend */
#define SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL	"main"
#define SUBSTANCE_SOURCE_PREVIEW_IMAGE_SLICES		"slices"
#define SUBSTANCE_SOURCE_PREVIEW_IMAGE_OUTPUTS		"tri"

/**/
typedef SharedPtr<class ImageAttachment> ImageAttachmentPtr;

/** Preview Attachments, used for images, and free to download */
class ImageAttachment : public BaseAttachment
{
public:
	/** Result code for ImageAttachment loading */
	enum class LoadImageResult
	{
		Success,
		Failed,
		AlreadyLoaded,
	};

	/** Callback when image data is loaded */
	typedef std::function<void(LoadImageResult)> LoadImageCallback;

public:
	ImageAttachment(const String& label, const String& url);

	/** Get desired image width */
	inline int getDesiredImageWidth() const;

	/** Set desired image width */
	inline void setDesiredImageWidth(int width);

	/** Get desired image height */
	inline int getDesiredImageHeight() const;

	/** Set desired image height */
	inline void setDesiredImageHeight(int height);

	/** Get image data in JPEG format */
	inline const BinaryData& getImageData() const;

	/** Load image data from web (latent function) */
	void loadImageData(LoadImageCallback callback);

	/** From IAttachment */
	virtual const char* getType() const override;

	/** Get RTTI type */
	static const char* getTypeStatic();

private:
	/** Construct fully qualified URL using provided dimensions */
	String getCompleteURL() const;

private:
	BinaryData			mImageData;

	int					mDesiredWidth;
	int					mDesiredHeight;
};

/**/
inline bool DownloadAttachment::Handle::isValid() const
{
	return (mProgressHandle.use_count() > 0);
}

inline IHTTPRequest::ProgressHandlePtr DownloadAttachment::Handle::getProgressHandle() const
{
	return mProgressHandle;
}

inline int DownloadAttachment::Handle::getPayloadLength() const
{
	return mPayloadLength;
}

/**/
inline int ImageAttachment::getDesiredImageWidth() const
{
	return mDesiredWidth;
}

inline void ImageAttachment::setDesiredImageWidth(int width)
{
	mDesiredWidth = width;
}

inline int ImageAttachment::getDesiredImageHeight() const
{
	return mDesiredHeight;
}

inline void ImageAttachment::setDesiredImageHeight(int height)
{
	mDesiredHeight = height;
}

const BinaryData& ImageAttachment::getImageData() const
{
	return mImageData;
}

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_ATTACHMENT_H_
