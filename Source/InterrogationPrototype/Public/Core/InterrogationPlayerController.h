#pragma once

#include "CoreMinimal.h"
#include "Core/InterrogationCameraView.h"
#include "GameFramework/PlayerController.h"
#include "InterrogationPlayerController.generated.h"

class AInterrogationCameraRig;
class AInterrogationConversationDirector;

UCLASS()
class INTERROGATIONPROTOTYPE_API AInterrogationPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AInterrogationPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Interrogation Camera")
	void CycleRoomCamera();

	UFUNCTION(BlueprintCallable, Category = "Interrogation Camera")
	void UsePlayerCamera();

private:
	void FindCameraRig();
	void FindConversationDirector();
	EInterrogationCameraView GetNextCameraView(EInterrogationCameraView CurrentView) const;
	void StartVoiceQuestion();
	void StopVoiceQuestion();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	float CameraBlendSeconds;

	UPROPERTY(Transient)
	TObjectPtr<AInterrogationCameraRig> ActiveCameraRig;

	UPROPERTY(Transient)
	TObjectPtr<AInterrogationConversationDirector> ActiveConversationDirector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	EInterrogationCameraView ActiveRoomCameraView;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation Camera", meta = (AllowPrivateAccess = "true"))
	bool bUsingRoomCamera;
};
