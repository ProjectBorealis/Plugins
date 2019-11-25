/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "EdGraph/EdGraph.h" 
#include "MultiPackerRuntimeEdGraph.generated.h"

UCLASS()
class UMultiPackerRuntimeEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UMultiPackerRuntimeEdGraph();
	virtual ~UMultiPackerRuntimeEdGraph();

	virtual void RebuildGenericGraph();
	bool MultiPackerNode = false;
	bool IsRebuilding = false;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
};
