﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "ST_SplineTool.generated.h"

class UTextRenderComponent;
USTRUCT(BlueprintType)
struct FSplineMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mesh = nullptr;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ESplineMeshAxis::Type> forwardAxis = ESplineMeshAxis::X;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionEnabled::Type> collisionType = ECollisionEnabled::NoCollision;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EComponentMobility::Type> mobility = EComponentMobility::Movable;
};

UCLASS()
class SPLINETOOL_API AST_SplineTool : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	int pointsAmount = 0;

	UPROPERTY()
	UInstancedStaticMeshComponent* instancedStaticMesh = nullptr;

#pragma region MeasurementTool

	UPROPERTY(BlueprintReadWrite)
	UTextRenderComponent* lengthTextRender = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TArray<UTextRenderComponent*> betweenPointsTextRenders;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Debug")
	bool bShowLengths = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Debug",
		meta=(EditCondition="bShowLengths", EditConditionHides))
	FVector lengthTextOffset = FVector(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Debug",
		meta=(EditCondition="bShowLengths"))
	bool bShowLengthOnActorLocation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Debug",
		meta=(EditCondition="bShowLengths", EditConditionHides))
	bool bShowTotalLength = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Debug",
		meta=(EditCondition="bShowLengths", EditConditionHides,
			ToolTip="Shows direct distance between Start and End points"))
	bool bShowDirectDistance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Debug",
		meta=(EditCondition="bShowLengths", EditConditionHides))
	bool bShowDistancesBetweenPoints = false;

	UPROPERTY(BlueprintReadWrite)
	float totalLength = 0;

#pragma endregion MeasurementTool

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite,
		Category="SplineTool")
	USplineComponent* splineComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "SplineTool|Spline settings|Meshes")
	TArray<FSplineMeshData> defaultMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings")
	bool bIsEditMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		meta=(InlineEditConditionToggle),
		Category="SplineTool|Spline settings")
	bool bIsCustomSpacing = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		meta=(EditCondition="bIsCustomSpacing",
			ClampMin=1, UIMin=1),
		Category="SplineTool|Spline settings")
	float customMeshLength = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		meta=(EditCondition="bIsEditMode", EditConditionHides),
		Category="SplineTool|Spline settings")
	bool bIsClosed = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category="SplineTool|Spline settings",
		meta=(EditCondition="!bIsClosed", EditConditionHides))
	bool bHasTail = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "SplineTool|Spline settings|Meshes",
		meta=(EditCondition="bHasTail", EditConditionHides))
	FSplineMeshData tailMesh;

	UPROPERTY(EditAnywhere,
		Category="SplineTool",
		AdvancedDisplay)
	TArray<TEnumAsByte<EObjectTypeQuery>> clippingObjectTypes;

public:
	AST_SplineTool();

	void OnConstruction(const FTransform& Transform) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void GenerateTextRendersBetweenPoints();
	
	UFUNCTION()
	virtual void PopulateSplineWithInstancedMesh();

	void PopulateSplineWithSplineMeshComponent(bool _bUpdateMesh = true);
	void GenerateSplineMeshes();

	void UpdateSplineMeshSettings(USplineMeshComponent& _sMC,
	                              const FSplineMeshData _datas,
	                              bool _bUpdateMesh = true) const;

	void SetSplineMeshStartEnd_Free(USplineMeshComponent& _sMC,
	                                const int _pointIndex,
	                                bool _bUpdateMesh = true) const;

	void SetSplineMeshStartEnd_Locked(USplineMeshComponent& _sMC,
	                                  const int _index,
	                                  const float _spacing,
	                                  bool _bUpdateMesh = true) const;

	FSplineMeshData GetRandomDefaultMeshData() const
	{
		const int _meshesAmount = defaultMeshes.Num() - 1;
		if (_meshesAmount < 0) return FSplineMeshData();
		const int _meshIndex = FMath::RandRange(0, _meshesAmount);
		const FSplineMeshData _currentDatas = defaultMeshes[_meshIndex];

		return _currentDatas;
	}

	
	void PlaceElementAtIndex(const FSplineMeshData _datas, const int _index);

public:
	UFUNCTION(CallInEditor, Category="SplineTool")
	void ClipToGround();
};
