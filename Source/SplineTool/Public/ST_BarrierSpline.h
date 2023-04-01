#pragma once

#include "CoreMinimal.h"
#include "ST_SplineTool.h"
#include "ST_BarrierSpline.generated.h"

/**
 * 
 */
UCLASS()
class SPLINETOOL_API AST_BarrierSpline : public AST_SplineTool
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings|Barrier settings",
		meta=(EditCondition="!bIsClosed", EditConditionHides))
	bool bHasTail = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "SplineTool|Spline settings|Meshes",
		meta=(EditCondition="bHasTail", EditConditionHides))
	FSplineMeshData tailMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings|Barrier settings")
	bool bHasDoors = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings|Barrier settings",
		meta=(EditCondition="bHasDoors", EditConditionHides))
	TArray<int32> doorIndexes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "SplineTool|Spline settings|Meshes",
		meta=(EditCondition="bHasDoors", EditConditionHides))
	FSplineMeshData doorMesh;

public:
	AST_BarrierSpline();

	void OnConstruction(const FTransform& Transform) override;
	//virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	void PlaceElementAtIndex(const FSplineMeshData _datas, const int _index);
	void PlaceDoors();
};
