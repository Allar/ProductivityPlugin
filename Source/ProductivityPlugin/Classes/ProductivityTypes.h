#pragma once

#include "ProductivityTypes.generated.h"

USTRUCT(BlueprintType)
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

USTRUCT(BlueprintType)
struct FBatchPlaceMeshInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BatchPlacing)
	FString ImportNameSubstring;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BatchPlacing)
	FMeshInfo MeshInfo;
};

enum class EProductivityMessageType
{
	ADD_STATICMESH = 1,
};

struct FAddStaticMeshPayload
{
	FString OriginalSceneName;
	FString LocationX;
	FString LocationY;
	FString LocationZ;
	FString RotationX;
	FString RotationY;
	FString RotationZ;
	FString ScaleX;
	FString ScaleY;
	FString ScaleZ;


	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FAddStaticMeshPayload& Payload)
	{
		return Ar << Payload.OriginalSceneName << Payload.LocationX << Payload.LocationY << Payload.LocationZ << Payload.RotationX << Payload.RotationY << Payload.RotationZ << Payload.ScaleX << Payload.ScaleY << Payload.ScaleZ;
	}
};

struct FProductivityNetworkMessage
{
	TEnumAsByte<EProductivityMessageType> Type;
	FAddStaticMeshPayload Payload;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FProductivityNetworkMessage& NetworkMessage)
	{
		return Ar << NetworkMessage.Type << NetworkMessage.Payload;
	}
};