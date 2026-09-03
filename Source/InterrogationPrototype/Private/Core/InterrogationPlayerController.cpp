#include "Core/InterrogationPlayerController.h"

#include "Components/InputComponent.h"
#include "Core/InterrogationCameraRig.h"
#include "Core/InterrogationConversationDirector.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

AInterrogationPlayerController::AInterrogationPlayerController()
{
	CameraBlendSeconds = 0.35f;
	ActiveRoomCameraView = EInterrogationCameraView::WideRoom;
	bUsingRoomCamera = false;
}

void AInterrogationPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	FindCameraRig();
	FindConversationDirector();
}

void AInterrogationPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("CycleCamera"), IE_Pressed, this, &AInterrogationPlayerController::CycleRoomCamera);
	InputComponent->BindAction(TEXT("UsePlayerCamera"), IE_Pressed, this, &AInterrogationPlayerController::UsePlayerCamera);
	InputComponent->BindAction(TEXT("AskSuspect"), IE_Pressed, this, &AInterrogationPlayerController::StartVoiceQuestion);
	InputComponent->BindAction(TEXT("AskSuspect"), IE_Released, this, &AInterrogationPlayerController::StopVoiceQuestion);
}

void AInterrogationPlayerController::CycleRoomCamera()
{
	if (!ActiveCameraRig)
	{
		FindCameraRig();
	}

	if (!ActiveCameraRig)
	{
		return;
	}

	ActiveRoomCameraView = bUsingRoomCamera ? GetNextCameraView(ActiveRoomCameraView) : EInterrogationCameraView::WideRoom;
	ActiveCameraRig->ActivateCamera(ActiveRoomCameraView);

	SetViewTargetWithBlend(ActiveCameraRig, CameraBlendSeconds);
	bUsingRoomCamera = true;
}

void AInterrogationPlayerController::UsePlayerCamera()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		SetViewTargetWithBlend(ControlledPawn, CameraBlendSeconds);
	}

	bUsingRoomCamera = false;
}

void AInterrogationPlayerController::FindCameraRig()
{
	ActiveCameraRig = nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AInterrogationCameraRig> CameraRigIt(World); CameraRigIt; ++CameraRigIt)
	{
		ActiveCameraRig = *CameraRigIt;
		return;
	}
}

void AInterrogationPlayerController::FindConversationDirector()
{
	ActiveConversationDirector = nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AInterrogationConversationDirector> DirectorIt(World); DirectorIt; ++DirectorIt)
	{
		ActiveConversationDirector = *DirectorIt;
		return;
	}
}

EInterrogationCameraView AInterrogationPlayerController::GetNextCameraView(EInterrogationCameraView CurrentView) const
{
	switch (CurrentView)
	{
	case EInterrogationCameraView::WideRoom:
		return EInterrogationCameraView::DetectiveSeat;
	case EInterrogationCameraView::DetectiveSeat:
		return EInterrogationCameraView::SuspectCloseup;
	case EInterrogationCameraView::SuspectCloseup:
		return EInterrogationCameraView::EvidenceTable;
	case EInterrogationCameraView::EvidenceTable:
	default:
		return EInterrogationCameraView::WideRoom;
	}
}

void AInterrogationPlayerController::StartVoiceQuestion()
{
	if (!ActiveConversationDirector)
	{
		FindConversationDirector();
	}

	if (ActiveConversationDirector)
	{
		ActiveConversationDirector->StartPlayerVoiceQuestion();
	}
}

void AInterrogationPlayerController::StopVoiceQuestion()
{
	if (!ActiveConversationDirector)
	{
		FindConversationDirector();
	}

	if (ActiveConversationDirector)
	{
		ActiveConversationDirector->StopPlayerVoiceQuestion();
	}
}
