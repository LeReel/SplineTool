#include "ST_SplineTool.h"

#if WITH_EDITOR
#include "EditorViewportClient.h"
#endif
#include "ST_Utils.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/ThreadManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogCustom);

//Stored here because interfere with Save in scene if set in a sub-level
UWorld* world = nullptr;

AST_SplineTool::AST_SplineTool()
{
	//Enables Tick for TextRender rotation update
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	//A bit of optimization
	SetActorEnableCollision(false);
	bAsyncPhysicsTickEnabled = true;
	bEnableAutoLODGeneration = false;

	//Creates component
	splineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if (splineComponent)
	{
		//Prevents UE_Warnings about missing RootComponent
		SetRootComponent(splineComponent);
		splineComponent->SetClosedLoop(bIsClosed);

		//Some optimization again
		splineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		splineComponent->SetEnableGravity(false);

		//Updates status
		splineComponent->bSplineHasBeenEdited = true;
		//Passes SplinePoints to Construction so they can be manipulated
		splineComponent->bInputSplinePointsToConstructionScript = true;
		splineComponent->bIsEditorOnly = false;

		totalLength = splineComponent->GetSplineLength();
	}

	//Creates InstancedStaticMesh that will olds instances
	instancedStaticMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>("Mesh");
	if (instancedStaticMesh)
	{
		//Attaches it to spline
		instancedStaticMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	//Creates Length text
	lengthTextRender = CreateDefaultSubobject<UTextRenderComponent>("LengthText");
	if (lengthTextRender)
	{
		AddOwnedComponent(lengthTextRender);
		lengthTextRender->SetVisibility(bShowLengths);
	}
}

void AST_SplineTool::BeginPlay()
{
	Super::BeginPlay();
}

void AST_SplineTool::Tick(float DeltaSeconds)
{
#if WITH_EDITOR
	if (!lengthTextRender) return;

	if (world && bShowLengths)
	{
		FText _lengthText;

		//=================================ROTATION====================================//
		//Retrieves last frames' view locations
		auto viewLocations = world->ViewLocationsRenderedLastFrame;
		if (viewLocations.Num() == 0)
			return;

		//We choose the last (first in array)
		const FVector _cameraLocation = viewLocations[0];
		//Gets rotation pointing to last view location
		FRotator _rot = UKismetMathLibrary::FindLookAtRotation(lengthTextRender->GetComponentLocation(),
		                                                       _cameraLocation);
		lengthTextRender->SetWorldRotation(_rot);

		if (bShowDistanceBetweenEveryPoint)
		{
			for (UTextRenderComponent* testRenderer : betweenPointsTextRenders)
			{
				testRenderer->SetWorldRotation(_rot);
			}
		}
		//==========================================================================//

		//================================LOCATION==================================//
		FVector _loc = FVector(0);
		if (bShowLengthOnActorLocation)
		{
			_loc = GetActorLocation();
		}
		else
		{
			_loc = splineComponent->
				GetLocationAtDistanceAlongSpline(totalLength / 2, ESplineCoordinateSpace::World);
		}
		lengthTextRender->SetWorldLocation(_loc + lengthTextOffset);
		//==========================================================================//

		//If shows total length
		if (bShowTotalLength)
		{
			_lengthText = FText::FromString(FString::SanitizeFloat(totalLength));

			lengthTextRender->SetText(_lengthText);
		}
		else if (bShowDirectDistance)
		{
			const float _directDistance = FVector::Dist(
				splineComponent->GetLocationAtSplinePoint(
					pointsAmount - 1,
					ESplineCoordinateSpace::World),
				splineComponent->GetLocationAtSplinePoint(
					0, ESplineCoordinateSpace::World));

			_lengthText = FText::FromString(FString::SanitizeFloat(_directDistance));
		}
	}
#endif
	Super::Tick(DeltaSeconds);
}

void AST_SplineTool::GenerateTextRendersBetweenPoints()
{
	UE_LOG(LogTemp, Warning, TEXT("%d generateBetweenRenders %d points"), betweenPointsTextRenders.Num(), pointsAmount)
	if (pointsAmount - 1 != betweenPointsTextRenders.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Array flushed"))
		for (UTextRenderComponent* _tR : betweenPointsTextRenders)
		{
			betweenPointsTextRenders.Remove(_tR);
			_tR->DestroyComponent();
		}
	}
	for (int i = 0; i < pointsAmount - 1; ++i)
	{
		//TODO: 
		UTextRenderComponent* _tmpTr = NewObject<UTextRenderComponent>();
		betweenPointsTextRenders.Add(_tmpTr);
		const FVector _tmpLoc = splineComponent->GetLocationAtTime(
				splineComponent->GetTimeAtDistanceAlongSpline(
					splineComponent->GetDistanceAlongSplineAtSplinePoint(i)),
				ESplineCoordinateSpace::World)
			+ splineComponent->GetLocationAtTime(
				splineComponent->GetTimeAtDistanceAlongSpline(
					splineComponent->GetDistanceAlongSplineAtSplinePoint(i + 1)),
				ESplineCoordinateSpace::Local);

		_tmpTr->SetWorldLocation(_tmpLoc);
	}
}

#if WITH_EDITOR
void AST_SplineTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//Get the name of the property that was changed  
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr)
		                           ? PropertyChangedEvent.Property->GetFName()
		                           : NAME_None;

	//IsClosed update
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AST_SplineTool, bIsClosed))
	{
		if (splineComponent)
		{
			splineComponent->SetClosedLoop(bIsClosed);
		}
	}
	//ShowLength update
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AST_SplineTool, bShowLengths))
	{
		if (lengthTextRender)
		{
			lengthTextRender->SetVisibility(bShowLengths);
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AST_SplineTool::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!world)
	{
		world = GetWorld();
	}

	totalLength = splineComponent->GetSplineLength();

	//If pointsAmount has changed
	//if (bShowDistanceBetweenEveryPoint && pointsAmount != splineComponent->GetNumberOfSplinePoints())
	//{
	//	//GenerateTextRendersBetweenPoints();
	//}

	UE_LOG(LogTemp, Warning, TEXT("%d generateBetweenRenders %d points"), betweenPointsTextRenders.Num(), pointsAmount)
	//TODO: pointsAmount always == 0
	pointsAmount = splineComponent->GetNumberOfSplinePoints();

#if WITH_EDITOR
	//InstancedMesh for optimization (since UE5)
	if (bIsEditMode)
	{
		PopulateSplineWithInstancedMesh();
	}
	//"Bakes" the spline
	else
	{
		GenerateSplineMeshes();
	}
#endif

	if (bHasDoors) PlaceDoors();
	if (bHasTail) PlaceElementAtIndex(tailMesh, pointsAmount - 1);
}

bool AST_SplineTool::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AST_SplineTool::PopulateSplineWithInstancedMesh()
{
	if (defaultMeshes.IsEmpty()) return;

	//Clears instances
	if (instancedStaticMesh->GetInstanceCount() > 0)
	{
		instancedStaticMesh->ClearInstances();
	}
	//Gets a random meshData
	FSplineMeshData _meshData = GetRandomDefaultMeshData();
	//Sets its mesh to the instancedMesh
	instancedStaticMesh->SetStaticMesh(_meshData.mesh);

	//InstancedMesh is a set of Transform at which it will be repeated
	FTransform _meshTransform = FTransform::Identity;
	FVector _meshLocation = FVector(0);
	FQuat _meshRotation = FQuat::Identity;

	const float _meshSpacing = bIsCustomSpacing
		                           ? customMeshLength
		                           //Nullity test (i.e. when no new meshData has been added to list yet)
		                           : _meshData.mesh
		                           ? _meshData.mesh->GetBounds().BoxExtent.X
		                           : customMeshLength;

	const float _splineLength = totalLength;
	if (_meshSpacing == 0) return;

	const float _meshesAmount = _splineLength / _meshSpacing;

	for (int i = 0; i < _meshesAmount; ++i)
	{
		//===============LOCATION==================//
		_meshLocation = splineComponent->GetLocationAtDistanceAlongSpline(
			i * _meshSpacing, ESplineCoordinateSpace::Local);
		_meshTransform.SetLocation(_meshLocation);
		//========================================//

		//==============ROTATION===================//
		//Rotates instance toward next point
		FRotator _lookAt = FRotator::ZeroRotator;
		const float nextDist = (i + 1) * _meshSpacing;
		FVector nextPoint = splineComponent->GetLocationAtDistanceAlongSpline(
			nextDist, ESplineCoordinateSpace::Local);
		_lookAt = UKismetMathLibrary::FindLookAtRotation(_meshLocation, nextPoint);
		_meshRotation = _lookAt.Quaternion();
		_meshTransform.SetRotation(_meshRotation);
		//=========================================//

		instancedStaticMesh->AddInstance(_meshTransform);
	}
}

void AST_SplineTool::GenerateSplineMeshes()
{
	if (instancedStaticMesh)
	{
		//Clears instances
		if (instancedStaticMesh->GetInstanceCount() >= 0)
		{
			instancedStaticMesh->ClearInstances();
		}
	}

	PopulateSplineWithSplineMeshComponent();
}

void AST_SplineTool::PopulateSplineWithSplineMeshComponent(bool _bUpdateMesh)
{
	const FSplineMeshData _meshData = GetRandomDefaultMeshData();

	const float _splineLength = totalLength;

	const float _meshSpacing = bIsCustomSpacing
		                           ? customMeshLength
		                           //Nullity test (i.e. when new meshData has been added to list)
		                           : _meshData.mesh
		                           ? _meshData.mesh->GetBounds().BoxExtent.X
		                           : customMeshLength;
	const int _meshesAmount = _splineLength / _meshSpacing;

	for (int i = 0; i <= _meshesAmount; ++i)
	{
		//Creates a SplineMeshComponent
		USplineMeshComponent* _tmpSmc = NewObject<USplineMeshComponent>(this);
		if (!_tmpSmc) break;
		UpdateSplineMeshSettings(*_tmpSmc, _meshData, _bUpdateMesh);
		SetSplineMeshStartEnd_Locked(*_tmpSmc, i, _meshSpacing);
	}
}

void AST_SplineTool::UpdateSplineMeshSettings(USplineMeshComponent& _sMC,
                                              const FSplineMeshData _datas,
                                              bool _bUpdateMesh) const
{
	_sMC.CreationMethod = EComponentCreationMethod::UserConstructionScript;

	_sMC.SetStaticMesh(_datas.mesh);
	_sMC.SetMobility(_datas.mobility);
	_sMC.SetCollisionEnabled(_datas.collisionType);
	_sMC.SetForwardAxis(_datas.forwardAxis, _bUpdateMesh);

	if (!world)
	{
		world = GetWorld();
	}
	_sMC.RegisterComponentWithWorld(world);

	_sMC.AttachToComponent(splineComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

//Placement based on a given spacing (meshSize or custom) multiplied by _index
void AST_SplineTool::SetSplineMeshStartEnd_Locked(USplineMeshComponent& _sMC,
                                                  const int _index,
                                                  const float _spacing,
                                                  bool _bUpdateMesh) const
{
	const int _endIndex = _index + 1;

	if (_index * _spacing >= totalLength)
	{
		return;
	}

	const FVector _startPos = splineComponent->GetLocationAtDistanceAlongSpline(
		_index * _spacing, ESplineCoordinateSpace::Local);
	const FVector _startTan = splineComponent->GetTangentAtDistanceAlongSpline(
		_index * _spacing, ESplineCoordinateSpace::Local);
	const FVector _cStartTan = _startTan.GetClampedToMaxSize(_spacing);

	const FVector _endPos = splineComponent->GetLocationAtDistanceAlongSpline(
		_endIndex * _spacing, ESplineCoordinateSpace::Local);
	const FVector _endTan = splineComponent->GetTangentAtDistanceAlongSpline(
		_endIndex * _spacing, ESplineCoordinateSpace::Local);
	const FVector _cEndTan = _endTan.GetClampedToMaxSize(_spacing);

	_sMC.SetStartAndEnd(_startPos, _cStartTan, _endPos, _cEndTan, _bUpdateMesh);
}

//Placement based on splinePoints (meshStart = splinePoints[_index]; meshEnd = splinePoints[_index + 1]) 
void AST_SplineTool::SetSplineMeshStartEnd_Free(USplineMeshComponent& _sMC,
                                                const int _pointIndex,
                                                bool _bUpdateMesh) const
{
	const FVector _startPos = splineComponent->GetLocationAtSplinePoint(
		_pointIndex, ESplineCoordinateSpace::Local);
	const FVector _startTan = splineComponent->GetTangentAtSplinePoint(
		_pointIndex, ESplineCoordinateSpace::Local);

	int _endIndex = _pointIndex + 1;

	if (bIsClosed)
	{
		_endIndex = _pointIndex >= pointsAmount ? 0 : _endIndex;
	}
	else
	{
		if (_endIndex >= pointsAmount) return;
	}

	const FVector _endPos = splineComponent->GetLocationAtSplinePoint(
		_endIndex, ESplineCoordinateSpace::Local);
	const FVector _endTan = splineComponent->GetTangentAtSplinePoint(
		_endIndex, ESplineCoordinateSpace::Local);

	_sMC.SetStartAndEnd(_startPos, _startTan, _endPos, _endTan, _bUpdateMesh);
}


void AST_SplineTool::PlaceElementAtIndex(const FSplineMeshData _datas, const int _index)
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

void AST_SplineTool::PlaceDoors()
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

void AST_SplineTool::ClipToGround()
{
	//Too perf-consuming at the moment
	if (!bIsEditMode || !world) return;

	const FVector _upVector = splineComponent->GetUpVector();

	//Updates points locations through a lineTrace
	TArray<AActor*> _toIgnore;
	FHitResult _hitResult;
	for (int _pointCount = 0; _pointCount < pointsAmount; ++_pointCount)
	{
		const FTransform _transform = splineComponent->GetTransformAtSplinePoint(
			_pointCount, ESplineCoordinateSpace::World);

		const bool _hit = UKismetSystemLibrary::LineTraceSingleForObjects(
			world,
			_transform.GetLocation(),
			_transform.GetLocation() + _upVector * (0 - 10000),
			clippingObjectTypes,
			false,
			_toIgnore,
			EDrawDebugTrace::ForDuration,
			_hitResult,
			true
		);

		if (!_hit)continue;
		const FVector _newPointLocation = _hitResult.ImpactPoint;
		splineComponent->SetLocationAtSplinePoint(_pointCount, _newPointLocation, ESplineCoordinateSpace::World, true);
	}

	//Refresh meshes
	PopulateSplineWithInstancedMesh();
}
