#include "DA_SplineTool.h"

#if WITH_EDITOR
#include "EditorViewportClient.h"
#endif
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/ThreadManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//Stored here because interfere with Save in scene if set in a sub-level
UWorld* world = nullptr;

ADA_SplineTool::ADA_SplineTool()
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
	}
}

void ADA_SplineTool::BeginPlay()
{
	Super::BeginPlay();

	if (!world)
	{
		world = GetWorld();
	}
}

void ADA_SplineTool::Tick(float DeltaSeconds)
{
	if (!world)
	{
		world = GetWorld();
	}

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

		//TODO: Correct placement
		if (bShowDistanceBetweenEveryPoint)
		{
			for (int i = 1; i < pointsAmount; ++i)
			{
				const FVector _tmpLoc = splineComponent->GetLocationAtDistanceAlongSpline(
					splineComponent->GetDistanceAlongSplineAtSplinePoint(i) / 2,
					ESplineCoordinateSpace::World);

				DrawDebugSphere(world,
				                _tmpLoc + lengthTextOffset,
				                50,
				                10,
				                FColor::Green,
				                false,
				                -1,
				                0,
				                1);
			}
		}

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

#if WITH_EDITOR
void ADA_SplineTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//Get the name of the property that was changed  
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr)
		                           ? PropertyChangedEvent.Property->GetFName()
		                           : NAME_None;

	//IsClosed update
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ADA_SplineTool, bIsClosed))
	{
		if (splineComponent)
		{
			splineComponent->SetClosedLoop(bIsClosed);
		}
	}
	//ShowLength update
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ADA_SplineTool, bShowLengths))
	{
		if (lengthTextRender)
		{
			lengthTextRender->SetVisibility(bShowLengths);
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ADA_SplineTool::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!world)
	{
		world = GetWorld();
	}

	totalLength = splineComponent->GetSplineLength();
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
	//TODO: See if this #if cancels mesh generation on Runtime
#endif
}

bool ADA_SplineTool::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ADA_SplineTool::PopulateSplineWithInstancedMesh()
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

	const float _meshSpacing = bIsCustomMeshLength
		                           ? customMeshLength
		                           //Nullity test (i.e. when new meshData has been added to list)
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

void ADA_SplineTool::GenerateSplineMeshes()
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

void ADA_SplineTool::PopulateSplineWithSplineMeshComponent(bool _bUpdateMesh)
{
	const FSplineMeshData _meshData = GetRandomDefaultMeshData();

	const float _splineLength = totalLength;

	const float _meshSpacing = bIsCustomMeshLength
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
		SetSplineMeshStartEnd_Locked(*_tmpSmc, _meshData, i, _meshSpacing);
	}
}

void ADA_SplineTool::UpdateSplineMeshSettings(USplineMeshComponent& _sMC,
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

void ADA_SplineTool::SetSplineMeshStartEnd_Locked(USplineMeshComponent& _sMC,
                                                  const FSplineMeshData _datas,
                                                  const int _index,
                                                  const float _meshSize,
                                                  bool _bUpdateMesh) const
{
	const int _endIndex = _index + 1;

	if (_index * _meshSize >= totalLength)
	{
		return;
	}

	const FVector _startPos = splineComponent->GetLocationAtDistanceAlongSpline(
		_index * _meshSize, ESplineCoordinateSpace::Local);
	const FVector _startTan = splineComponent->GetTangentAtDistanceAlongSpline(
		_index * _meshSize, ESplineCoordinateSpace::Local);
	const FVector _cStartTan = _startTan.GetClampedToMaxSize(_meshSize);

	const FVector _endPos = splineComponent->GetLocationAtDistanceAlongSpline(
		_endIndex * _meshSize, ESplineCoordinateSpace::Local);
	const FVector _endTan = splineComponent->GetTangentAtDistanceAlongSpline(
		_endIndex * _meshSize, ESplineCoordinateSpace::Local);
	const FVector _cEndTan = _endTan.GetClampedToMaxSize(_meshSize);

	_sMC.SetStartAndEnd(_startPos, _cStartTan, _endPos, _cEndTan, _bUpdateMesh);
}

void ADA_SplineTool::SetSplineMeshStartEnd_Free(USplineMeshComponent& _sMC,
                                                const FSplineMeshData _datas,
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

void ADA_SplineTool::ClipToGround()
{
	//Too perf-consuming at the moment
	if (!bIsEditMode) return;

	if (!world)
	{
		world = GetWorld();
	}
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
