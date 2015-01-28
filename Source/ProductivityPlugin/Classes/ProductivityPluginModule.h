// Some copyright should be here...

#pragma once

#include "ModuleManager.h"

class FProductivityPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command.*/
	void StaticToInstancedClicked();
	
private:

	void AddToolbarExtension(class FToolBarBuilder &);
	void AddMenuExtension(class FMenuBuilder &);

	TSharedPtr<class FUICommandList> PluginCommands;
};