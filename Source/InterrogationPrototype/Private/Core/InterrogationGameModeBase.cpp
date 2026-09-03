#include "Core/InterrogationGameModeBase.h"

#include "Characters/InterrogationPlayerCharacter.h"
#include "Core/InterrogationCameraRig.h"
#include "Core/InterrogationConversationDirector.h"
#include "Core/InterrogationPlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "Investigation/InterrogationRoomBuilder.h"

AInterrogationGameModeBase::AInterrogationGameModeBase()
{
	DefaultPawnClass = AInterrogationPlayerCharacter::StaticClass();
	PlayerControllerClass = AInterrogationPlayerController::StaticClass();

	RoomBuilderClass = AInterrogationRoomBuilder::StaticClass();
	CameraRigClass = AInterrogationCameraRig::StaticClass();
	ConversationDirectorClass = AInterrogationConversationDirector::StaticClass();

	PlayerSpawnLocation = FVector(-420.0f, 0.0f, 96.0f);
	PlayerSpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
}

void AInterrogationGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnRoomShell();
}

void AInterrogationGameModeBase::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	const FTransform SpawnTransform(PlayerSpawnRotation, PlayerSpawnLocation);
	APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);
	if (!NewPawn)
	{
		return;
	}

	NewPlayer->Possess(NewPawn);
	SetPlayerDefaults(NewPawn);
}

bool AInterrogationGameModeBase::DoesWorldContainRoomBuilder() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<AInterrogationRoomBuilder> ActorIt(World); ActorIt; ++ActorIt)
	{
		return true;
	}

	return false;
}

bool AInterrogationGameModeBase::DoesWorldContainCameraRig() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<AInterrogationCameraRig> ActorIt(World); ActorIt; ++ActorIt)
	{
		return true;
	}

	return false;
}

bool AInterrogationGameModeBase::DoesWorldContainConversationDirector() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<AInterrogationConversationDirector> ActorIt(World); ActorIt; ++ActorIt)
	{
		return true;
	}

	return false;
}

void AInterrogationGameModeBase::SpawnRoomShell()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (RoomBuilderClass && !DoesWorldContainRoomBuilder())
	{
		World->SpawnActor<AInterrogationRoomBuilder>(RoomBuilderClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	}

	if (CameraRigClass && !DoesWorldContainCameraRig())
	{
		World->SpawnActor<AInterrogationCameraRig>(CameraRigClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	}

	if (ConversationDirectorClass && !DoesWorldContainConversationDirector())
	{
		World->SpawnActor<AInterrogationConversationDirector>(ConversationDirectorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	}
}
