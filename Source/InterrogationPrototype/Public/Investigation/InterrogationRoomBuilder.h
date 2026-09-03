#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterrogationRoomBuilder.generated.h"

class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class INTERROGATIONPROTOTYPE_API AInterrogationRoomBuilder : public AActor
{
	GENERATED_BODY()

public:
	AInterrogationRoomBuilder();

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UStaticMeshComponent* CreateStructuralBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale);
	UStaticMeshComponent* CreateFurnitureBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale);
	UStaticMeshComponent* CreateAccentBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale);
	void CreateChair(const FString& Prefix, const FVector& SeatLocation, float BackOffsetX);
	UStaticMeshComponent* CreateBox(const FName PartName, const FVector& RelativeLocation, const FVector& RelativeScale, UMaterialInterface* Material, TArray<TObjectPtr<UStaticMeshComponent>>& TargetCollection);
	void ApplyMaterial(const TArray<TObjectPtr<UStaticMeshComponent>>& Parts, UMaterialInterface* Material) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RoomRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UStaticMeshComponent>> StructuralParts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UStaticMeshComponent>> FurnitureParts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UStaticMeshComponent>> AccentParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> StructuralMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> FurnitureMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> AccentMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CubeMesh;
};
