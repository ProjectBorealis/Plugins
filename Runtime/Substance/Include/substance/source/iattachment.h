//! @file iattachment.h
//! @brief Substance Source Attachment Interface
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Interface for asset attachments
#ifndef _SUBSTANCE_SOURCE_IATTACHMENT_H_
#define _SUBSTANCE_SOURCE_IATTACHMENT_H_
#pragma once

namespace Alg
{
namespace Source
{

class Asset;

typedef SharedPtr<class IAttachment>	IAttachmentPtr;
typedef Vector<IAttachmentPtr>			IAttachmentVector;

/**/
class IAttachment : public std::enable_shared_from_this<IAttachment>
{
public:
	virtual ~IAttachment() {}

	/** Called by asset when ownership has taken place */
	virtual void onAcquire(Asset* asset) = 0;

	/** Pointer to parent Asset */
	virtual Asset* getAsset() const = 0;

	/** String value used for RTTI */
	virtual const char* getType() const = 0;

	/** Asset Label */
	virtual const String& getLabel() const = 0;

	/** Retrieve Attachment URL */
	virtual const String& getURL() const = 0;

	/** Safely cast to derived classes */
	template<class T> static inline T safeCast(IAttachmentPtr attachment)
	{
		//normally string pointer equality compares are bad,
		//but this is valid because we are comparing static strings
		//as an easy way to determine object type without RTTI compiled in
		if (std::remove_pointer<T>::type::getTypeStatic() == attachment->getType())
		{
			return static_cast<T>(attachment.get());
		}

		return nullptr;
	}
};

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_IATTACHMENT_H_
