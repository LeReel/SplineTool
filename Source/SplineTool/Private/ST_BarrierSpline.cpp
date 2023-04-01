#include "ST_BarrierSpline.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

//Stored here because interfere with Save in scene if set in a sub-level
UWorld* world = nullptr;

AST_BarrierSpline::AST_BarrierSpline()
{
}

void AST_BarrierSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!world)
	{
		world = GetWorld();
	}

	if (bHasDoors) PlaceDoors();
	if (bHasTail) PlaceElementAtIndex(tailMesh, pointsAmount - 1);
}

void AST_BarrierSpline::PlaceElementAtIndex(const FSplineMeshData _datas, const int _index)
{
	USplineMeshComponent* _tmpSmc = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
	UpdateSplineMeshSettings(*_tmpSmc, _datas);

	const FVector _loc = splineComponent->GetLocationAtSplinePoint(_index, ESplineCoordinateSpace::World);
	_tmpSmc->SetWorldLocation(_loc);

	// Gets lastPoint's forward and rotates the mesh according to lookAt
	if (_index > 0)
	{
		const FVector _splineMeshForward = _tmpSmc->GetForwardVector();
		const FVector _prevLoc = splineComponent->GetLocationAtSplinePoint(_index - 1, ESplineCoordinateSpace::World);
		const FVector _fwd = _loc - _prevLoc;
		const FRotator _lookAtFwd = UKismetMathLibrary::FindLookAtRotation(_splineMeshForward, _fwd);
		_tmpSmc->AddLocalRotation(_lookAtFwd);
	}
}

void AST_BarrierSpline::PlaceDoors()
{
	for (const int _doorIndex : doorIndexes)
	{
		if (_doorIndex < 0 || _doorIndex >= pointsAmount - 1) continue;

		//TODO: Correct cast type / populating method (not working with SMC anymore as it is InstancedMesh now)
		//Casts the mesh at splinePoint[index] as a splineMeshComponent
		//USplineMeshComponent* _sMC = Cast<USplineMeshComponent>(splineComponent->GetChildComponent(_doorIndex));
		//if (!_sMC || _sMC->GetStaticMesh() == doorMesh.mesh) continue; //Avoids placing a door on an existing one
		//UpdateSplineMeshSettings(*_sMC, doorMesh);
		//SetSplineMeshStartEnd_Free(*_sMC, doorMesh, _doorIndex);
	}
}
