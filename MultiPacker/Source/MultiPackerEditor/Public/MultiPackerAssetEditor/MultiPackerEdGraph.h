/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "EdGraph/EdGraph.h" 
#include "MultiPackerEdGraph.generated.h"

UCLASS()
class UMultiPackerEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UMultiPackerEdGraph();
	virtual ~UMultiPackerEdGraph();

	virtual void RebuildGenericGraph();
	bool MultiPackerNode = false;
	bool IsRebuilding = false;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
};
