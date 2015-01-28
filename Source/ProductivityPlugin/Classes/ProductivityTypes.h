#pragma once

#include "ProductivityTypes.generated.h"

USTRUCT()
struct FMeshInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
	UStaticMesh* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
	TArray<UMaterialInterface*> Materials;

	bool operator==(const FMeshInfo& rightInfo) const
	{
		return StaticMesh == rightInfo.StaticMesh && Materials == rightInfo.Materials;
	}
};