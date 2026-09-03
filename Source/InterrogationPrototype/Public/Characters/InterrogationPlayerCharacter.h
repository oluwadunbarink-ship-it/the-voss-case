#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InterrogationPlayerCharacter.generated.h"

class UCameraComponent;

UCLASS()
class INTERROGATIONPROTOTYPE_API AInterrogationPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AInterrogationPlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
};
