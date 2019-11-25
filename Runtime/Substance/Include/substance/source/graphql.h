//! @file graphql.h
//! @brief Substance Source GraphQL Builder
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Build GraphQL Queries
#ifndef _SUBSTANCE_SOURCE_GRAPHQL_H_
#define _SUBSTANCE_SOURCE_GRAPHQL_H_
#pragma once

namespace Alg
{
namespace Source
{

typedef SharedPtr<class GraphQLSelection> GraphQLSelectionPtr;

/**/
class GraphQLSelection
{
public:
	GraphQLSelection(const String& name = "");

	/** Add a field to the current selection level */
	inline GraphQLSelection& addField(const String& field);

	/** Add a selection */
	inline GraphQLSelection& addSelection(const String& name);

	/** Add a Implementation */
	inline GraphQLSelection& addImplementation(const String& name);

	/** Create JSON Request String */
	inline String asJSON(bool toplevel = true) const;

private:
	String								mName;

	Vector<String>						mFields;
	Vector<GraphQLSelectionPtr>			mImplementations;
	Vector<GraphQLSelectionPtr>			mSelections;
};

/**/
GraphQLSelection::GraphQLSelection(const String& name/* = ""*/)
	: mName(name)
{
}

inline GraphQLSelection& GraphQLSelection::addField(const String& field)
{
	mFields.push_back(field);
	return *this;
}

inline GraphQLSelection& GraphQLSelection::addSelection(const String& name)
{
	GraphQLSelectionPtr selection = MakeShared<GraphQLSelection>(name);
	mSelections.push_back(selection);
	return *(selection.get());
}

inline GraphQLSelection& GraphQLSelection::addImplementation(const String& name)
{
	GraphQLSelectionPtr implementation = MakeShared<GraphQLSelection>(name);
	mImplementations.push_back(implementation);
	return *(implementation.get());
}

inline String GraphQLSelection::asJSON(bool toplevel/* = true */) const
{
	Stringstream stream;

	if (toplevel)
		stream << "{\"query\": \"";

	if (mName.length() > 0)
	{
		stream << mName << " {\\r\\n";
	}
	else
	{
		stream << "{\\r\\n";
	}

	//write out fields
	for (const auto& field : mFields)
		stream << field << "\\r\\n";

	//write out implementations
	for (const auto& implementation : mImplementations)
	{
		implementation->mName = "... on " + implementation->mName;
		stream << implementation->asJSON(false);
	}

	//write out sub selections
	for (const auto& selection : mSelections)
		stream << selection->asJSON(false);

	stream << "}\\r\\n";

	if (toplevel)
		stream << "\"}";

	return stream.str();
}

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_GRAPHQL_H_
