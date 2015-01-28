// Some copyright should be here...
#pragma once

#include "SlateBasics.h"
#include "ProductivityPluginStyle.h"

class FProductivityPluginCommands : public TCommands<FProductivityPluginCommands>
{
public:

	FProductivityPluginCommands();

	static void BindGlobalStaticToInstancedActions();
	static TSharedRef< SWidget > GenerateStaticToInstancedMenuContent(TSharedRef<FUICommandList> InCommandList);

#if WITH_EDITOR
	virtual void RegisterCommands() override;
#endif

	TSharedPtr< FUICommandInfo > StaticToInstanced;
	TSharedPtr< FUICommandInfo > StaticToInstancedIsResultGrouped;

	PRODUCTIVITYPLUGIN_API static TSharedPtr<FUICommandList> GlobalStaticToInstancedActions;
};

class PRODUCTIVITYPLUGIN_API FProductivityPluginCommandCallbacks
{
public:
	static void OnToggleStaticToInstancedResultGrouped();
	static bool OnToggleStaticToInstancedResultGroupedEnabled();
};