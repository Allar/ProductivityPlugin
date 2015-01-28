#include "ProductivityPluginModulePCH.h"
#include "InstancedMeshWrapper.h"

AInstancedMeshWrapper::AInstancedMeshWrapper(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	InstancedMeshes = PCIP.CreateDefaultSubobject<UInstancedStaticMeshComponent>(this, TEXT("InstancedMeshes_0"));
}
