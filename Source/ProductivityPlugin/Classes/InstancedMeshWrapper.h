#pragma once

#include "GameFramework/Actor.h"
#include "InstancedMeshWrapper.generated.h"

/**
*
*/
UCLASS()
class PRODUCTIVITYPLUGIN_API AInstancedMeshWrapper : public AActor
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Meshes")
	UInstancedStaticMeshComponent* InstancedMeshes;
};
