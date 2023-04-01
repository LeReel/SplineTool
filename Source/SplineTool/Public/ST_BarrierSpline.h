#pragma once

#include "CoreMinimal.h"
#include "ST_SplineTool.h"
#include "ST_BarrierSpline.generated.h"

/**
 * 
 */
UCLASS()
class SPLINETOOL_API ST_BarrierSpline : public ST_SplineTool
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings|Barrier settings",
		meta=(EditCondition="!bIsLooped", EditConditionHides))
	bool bHasTail = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "SplineTool|Spline settings|Meshes",
		meta=(EditCondition="bHasTail", EditConditionHides))
	FSplineMeshData tailMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings|Barrier settings")
	bool bHasDoor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings|Barrier settings",
		meta=(EditCondition="bHasDoor", EditConditionHides))
	TArray<int32> doorIndexes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "SplineTool|Spline settings|Meshes",
		meta=(EditCondition="bHasDoor", EditConditionHides))
	FSplineMeshData doorMesh;

public:
	ADA_BarrierSpline();

	void OnConstruction(const FTransform& Transform) override;
	//virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	void PlaceElementAtIndex(UWorld* _w, const FSplineMeshData _datas, const int _index);
	void PlaceDoor(UWorld* _w, const int _pointAmnt);
};
