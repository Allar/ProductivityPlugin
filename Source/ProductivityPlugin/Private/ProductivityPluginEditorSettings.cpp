#include "ProductivityPluginModulePCH.h"
#include "ProductivityPluginEditorSettings.h"

UProductivityPluginEditorSettings::UProductivityPluginEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GroupStaticToInstancedResults = false;
}