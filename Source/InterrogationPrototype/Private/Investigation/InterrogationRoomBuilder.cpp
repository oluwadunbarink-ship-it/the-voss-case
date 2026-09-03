#include "Investigation/InterrogationRoomBuilder.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AInterrogationRoomBuilder::AInterrogationRoomBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRoot"));
	SetRootComponent(RoomRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		CubeMesh = CubeMeshAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterialAsset.Succeeded())
	{
		StructuralMaterial = BasicMaterialAsset.Object;
		FurnitureMaterial = BasicMaterialAsset.Object;
		AccentMaterial = BasicMaterialAsset.Object;
	}

	CreateStructuralBox(TEXT("Floor"), FVector(0.0f, 0.0f, -5.0f), FVector(12.0f, 9.0f, 0.1f));
	CreateStructuralBox(TEXT("Ceiling"), FVector(0.0f, 0.0f, 355.0f), FVector(12.0f, 9.0f, 0.1f));
	CreateStructuralBox(TEXT("NorthWall"), FVector(0.0f, 455.0f, 175.0f), FVector(12.2f, 0.1f, 3.5f));
	CreateStructuralBox(TEXT("SouthWall"), FVector(0.0f, -455.0f, 175.0f), FVector(12.2f, 0.1f, 3.5f));
	CreateStructuralBox(TEXT("EastWall"), FVector(605.0f, 0.0f, 175.0f), FVector(0.1f, 9.0f, 3.5f));
	CreateStructuralBox(TEXT("WestWall"), FVector(-605.0f, 0.0f, 175.0f), FVector(0.1f, 9.0f, 3.5f));

	CreateFurnitureBox(TEXT("TableTop"), FVector(0.0f, 0.0f, 76.0f), FVector(2.4f, 1.2f, 0.12f));
	CreateFurnitureBox(TEXT("TableLegFrontLeft"), FVector(-100.0f, -45.0f, 36.0f), FVector(0.08f, 0.08f, 0.72f));
	CreateFurnitureBox(TEXT("TableLegFrontRight"), FVector(100.0f, -45.0f, 36.0f), FVector(0.08f, 0.08f, 0.72f));
	CreateFurnitureBox(TEXT("TableLegBackLeft"), FVector(-100.0f, 45.0f, 36.0f), FVector(0.08f, 0.08f, 0.72f));
	CreateFurnitureBox(TEXT("TableLegBackRight"), FVector(100.0f, 45.0f, 36.0f), FVector(0.08f, 0.08f, 0.72f));

	CreateChair(TEXT("DetectiveChair"), FVector(-260.0f, 0.0f, 45.0f), -38.0f);
	CreateChair(TEXT("SuspectChair"), FVector(260.0f, 0.0f, 45.0f), 38.0f);

	CreateAccentBox(TEXT("ObservationMirror"), FVector(0.0f, -461.0f, 180.0f), FVector(2.8f, 0.025f, 1.0f));
	CreateAccentBox(TEXT("DoorPanel"), FVector(606.0f, 260.0f, 105.0f), FVector(0.025f, 1.0f, 2.1f));
	CreateAccentBox(TEXT("WallCameraMount"), FVector(-120.0f, -462.0f, 285.0f), FVector(0.22f, 0.08f, 0.18f));
	CreateAccentBox(TEXT("InterrogationLamp"), FVector(0.0f, 0.0f, 130.0f), FVector(0.28f, 0.28f, 0.08f));
}

void AInterrogationRoomBuilder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyMaterial(StructuralParts, StructuralMaterial);
	ApplyMaterial(FurnitureParts, FurnitureMaterial);
	ApplyMaterial(AccentParts, AccentMaterial);
}

UStaticMeshComponent* AInterrogationRoomBuilder::CreateStructuralBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale)
{
	return CreateBox(PartName, RelativeLocation, RelativeScale, StructuralMaterial, StructuralParts);
}

UStaticMeshComponent* AInterrogationRoomBuilder::CreateFurnitureBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale)
{
	return CreateBox(PartName, RelativeLocation, RelativeScale, FurnitureMaterial, FurnitureParts);
}

UStaticMeshComponent* AInterrogationRoomBuilder::CreateAccentBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale)
{
	return CreateBox(PartName, RelativeLocation, RelativeScale, AccentMaterial, AccentParts);
}

void AInterrogationRoomBuilder::CreateChair(const FString& Prefix, const FVector& SeatLocation, float BackOffsetX)
{
	CreateFurnitureBox(FName(*(Prefix + TEXT("_Seat"))), SeatLocation, FVector(0.62f, 0.62f, 0.1f));
	CreateFurnitureBox(FName(*(Prefix + TEXT("_Back"))), SeatLocation + FVector(BackOffsetX, 0.0f, 42.0f), FVector(0.1f, 0.68f, 0.78f));

	const FVector LegScale(0.08f, 0.08f, 0.45f);
	CreateFurnitureBox(FName(*(Prefix + TEXT("_FrontLeftLeg"))), SeatLocation + FVector(-20.0f, -22.0f, -22.0f), LegScale);
	CreateFurnitureBox(FName(*(Prefix + TEXT("_FrontRightLeg"))), SeatLocation + FVector(20.0f, -22.0f, -22.0f), LegScale);
	CreateFurnitureBox(FName(*(Prefix + TEXT("_BackLeftLeg"))), SeatLocation + FVector(-20.0f, 22.0f, -22.0f), LegScale);
	CreateFurnitureBox(FName(*(Prefix + TEXT("_BackRightLeg"))), SeatLocation + FVector(20.0f, 22.0f, -22.0f), LegScale);
}

UStaticMeshComponent* AInterrogationRoomBuilder::CreateBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale, UMaterialInterface* Material, TArray<TObjectPtr<UStaticMeshComponent>>& TargetCollection)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(PartName);
	Part->SetupAttachment(RoomRoot);
	Part->SetRelativeLocation(RelativeLocation);
	Part->SetRelativeScale3D(RelativeScale);
	Part->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Part->SetMobility(EComponentMobility::Static);

	if (CubeMesh)
	{
		Part->SetStaticMesh(CubeMesh);
	}

	if (Material)
	{
		Part->SetMaterial(0, Material);
	}

	TargetCollection.Add(Part);
	return Part;
}

void AInterrogationRoomBuilder::ApplyMaterial(const TArray<TObjectPtr<UStaticMeshComponent>>& Parts, UMaterialInterface* Material) const
{
	if (!Material)
	{
		return;
	}

	for (UStaticMeshComponent* Part : Parts)
	{
		if (Part)
		{
			Part->SetMaterial(0, Material);
		}
	}
}
