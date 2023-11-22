// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LogViewerStyle.h"

class FLogViewerCommands : public TCommands<FLogViewerCommands>
{
public:

	FLogViewerCommands()
		: TCommands<FLogViewerCommands>(TEXT("LogViewer"), NSLOCTEXT("LogViewer", "LogViewerPluginName", "LogViewer Plugin"), NAME_None, FLogViewerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};