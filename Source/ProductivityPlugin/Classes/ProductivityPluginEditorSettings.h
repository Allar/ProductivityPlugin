#pragma once

#include "ProductivityPluginEditorSettings.generated.h"

/**
* Implements the Editor's play settings.
*/
UCLASS(config = EditorUserSettings)
class PRODUCTIVITYPLUGIN_API UProductivityPluginEditorSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(config, EditAnywhere, Category = ProductivityOptions)
	bool GroupStaticToInstancedResults;

	void SetGroupStaticToInstancedResults(const bool InGroupStaticToInstancedResults) { GroupStaticToInstancedResults = InGroupStaticToInstancedResults; SaveConfig(); }
	bool GetGroupStaticToInstancedResults() const { return GroupStaticToInstancedResults; }

};