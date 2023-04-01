#include "DA_BarrierSpline.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

//Stored here because interfere with Save in scene if set in a sub-level
UWorld* world = nullptr;

ADA_BarrierSpline::ADA_BarrierSpline()
{
}

void ADA_BarrierSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!world)
	{
		world = GetWorld();
	}

	if (bHasDoor) PlaceDoor(world, pointsAmount);
	if (bHasTail) PlaceElementAtIndex(world, tailMesh, pointsAmount - 1);
}

void ADA_BarrierSpline::PlaceElementAtIndex(UWorld* _w, const FSplineMeshData _datas, const int _index)
{
	USplineMeshComponent* _tmpSmc = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
	UpdateSplineMeshSettings(*_tmpSmc, _datas, _w);

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

void ADA_BarrierSpline::PlaceDoor(UWorld* _w, const int _pointAmnt)
{
	for (const int _doorIndex : doorIndexes)
	{
		if (_doorIndex < 0 || _doorIndex >= _pointAmnt - 1) continue;

		//TODO: Correct cast type / populating method (not working with SMC anymore as it is InstancedMesh now)
		//Casts the mesh at splinePoint[index] as a splineMeshComponent
		//USplineMeshComponent* _sMC = Cast<USplineMeshComponent>(splineComponent->GetChildComponent(_doorIndex));
		//if (!_sMC || _sMC->GetStaticMesh() == doorMesh.mesh) continue; //Avoids placing a door on an existing one
		//UpdateSplineMeshSettings(*_sMC, doorMesh, _w);
		//SetSplineMeshStartEnd_Free(*_sMC, doorMesh, _doorIndex);
	}
}
