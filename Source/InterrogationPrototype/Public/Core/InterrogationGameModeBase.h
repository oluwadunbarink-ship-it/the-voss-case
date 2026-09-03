#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InterrogationGameModeBase.generated.h"

class AInterrogationCameraRig;
class AInterrogationConversationDirector;
class AInterrogationRoomBuilder;

UCLASS()
class INTERROGATIONPROTOTYPE_API AInterrogationGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInterrogationGameModeBase();

	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	bool DoesWorldContainRoomBuilder() const;
	bool DoesWorldContainCameraRig() const;
	bool DoesWorldContainConversationDirector() const;
	void SpawnRoomShell();

	UPROPERTY(EditDefaultsOnly, Category = "Interrogation Room")
	TSubclassOf<AInterrogationRoomBuilder> RoomBuilderClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interrogation Room")
	TSubclassOf<AInterrogationCameraRig> CameraRigClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interrogation Room")
	TSubclassOf<AInterrogationConversationDirector> ConversationDirectorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interrogation Room")
	FVector PlayerSpawnLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Interrogation Room")
	FRotator PlayerSpawnRotation;
};
