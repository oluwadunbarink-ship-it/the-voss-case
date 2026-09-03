#include "Core/InterrogationCameraRig.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

AInterrogationCameraRig::AInterrogationCameraRig()
{
	PrimaryActorTick.bCanEverTick = false;

	RigRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RigRoot"));
	SetRootComponent(RigRoot);

	WideRoomCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("WideRoomCamera"));
	WideRoomCamera->SetupAttachment(RigRoot);

	DetectiveSeatCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DetectiveSeatCamera"));
	DetectiveSeatCamera->SetupAttachment(RigRoot);

	SuspectCloseupCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SuspectCloseupCamera"));
	SuspectCloseupCamera->SetupAttachment(RigRoot);

	EvidenceTableCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("EvidenceTableCamera"));
	EvidenceTableCamera->SetupAttachment(RigRoot);

	FocusPoint = FVector(0.0f, 0.0f, 90.0f);
	ActiveView = EInterrogationCameraView::WideRoom;
}

void AInterrogationCameraRig::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConfigureCamera(WideRoomCamera, FVector(-470.0f, -345.0f, 235.0f), FocusPoint, 72.0f);
	ConfigureCamera(DetectiveSeatCamera, FVector(-330.0f, 85.0f, 145.0f), FVector(70.0f, 0.0f, 105.0f), 58.0f);
	ConfigureCamera(SuspectCloseupCamera, FVector(-85.0f, -60.0f, 135.0f), FVector(260.0f, 0.0f, 115.0f), 42.0f);
	ConfigureCamera(EvidenceTableCamera, FVector(-120.0f, 170.0f, 190.0f), FVector(0.0f, 0.0f, 76.0f), 48.0f);

	ActivateCamera(ActiveView);
}

void AInterrogationCameraRig::ActivateCamera(EInterrogationCameraView NewView)
{
	WideRoomCamera->SetActive(false);
	DetectiveSeatCamera->SetActive(false);
	SuspectCloseupCamera->SetActive(false);
	EvidenceTableCamera->SetActive(false);

	if (UCameraComponent* SelectedCamera = GetCameraForView(NewView))
	{
		SelectedCamera->SetActive(true);
		ActiveView = NewView;
	}
}

EInterrogationCameraView AInterrogationCameraRig::GetActiveView() const
{
	return ActiveView;
}

UCameraComponent* AInterrogationCameraRig::GetActiveCameraComponent() const
{
	return GetCameraForView(ActiveView);
}

void AInterrogationCameraRig::ConfigureCamera(UCameraComponent* Camera, const FVector& RelativeLocation, const FVector& LookAtPoint, float FieldOfView) const
{
	if (!Camera)
	{
		return;
	}

	Camera->SetRelativeLocation(RelativeLocation);
	Camera->SetRelativeRotation((LookAtPoint - RelativeLocation).Rotation());
	Camera->SetFieldOfView(FieldOfView);
}

UCameraComponent* AInterrogationCameraRig::GetCameraForView(EInterrogationCameraView View) const
{
	switch (View)
	{
	case EInterrogationCameraView::WideRoom:
		return WideRoomCamera;
	case EInterrogationCameraView::DetectiveSeat:
		return DetectiveSeatCamera;
	case EInterrogationCameraView::SuspectCloseup:
		return SuspectCloseupCamera;
	case EInterrogationCameraView::EvidenceTable:
		return EvidenceTableCamera;
	default:
		return WideRoomCamera;
	}
}
