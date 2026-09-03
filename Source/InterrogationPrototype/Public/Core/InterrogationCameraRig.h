#pragma once

#include "CoreMinimal.h"
#include "Core/InterrogationCameraView.h"
#include "GameFramework/Actor.h"
#include "InterrogationCameraRig.generated.h"

class UCameraComponent;
class USceneComponent;

UCLASS()
class INTERROGATIONPROTOTYPE_API AInterrogationCameraRig : public AActor
{
	GENERATED_BODY()

public:
	AInterrogationCameraRig();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Interrogation Camera")
	void ActivateCamera(EInterrogationCameraView NewView);

	UFUNCTION(BlueprintPure, Category = "Interrogation Camera")
	EInterrogationCameraView GetActiveView() const;

	UFUNCTION(BlueprintPure, Category = "Interrogation Camera")
	UCameraComponent* GetActiveCameraComponent() const;

private:
	void ConfigureCamera(UCameraComponent* Camera, const FVector& RelativeLocation, const FVector& LookAtPoint, float FieldOfView) const;
	UCameraComponent* GetCameraForView(EInterrogationCameraView View) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RigRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> WideRoomCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> DetectiveSeatCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> SuspectCloseupCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> EvidenceTableCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	FVector FocusPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	EInterrogationCameraView ActiveView;
};
