// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ProductivityTypes.h"
#include "ProductivitySettings.generated.h"


/**
 * Implements the settings for the Slate Remote plug-in.
 */
UCLASS(config=Engine)
class UProductivitySettings
	: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Whether the Productivity Server is enabled. */
	UPROPERTY(config, EditAnywhere, Category = Productivity)
	bool EnableProductivityServer;

	/** Settings for batch placing. */
	UPROPERTY(config, EditAnywhere, Category = Productivity)
	TArray<FBatchPlaceMeshInfo> BatchPlaceSettings;
};
