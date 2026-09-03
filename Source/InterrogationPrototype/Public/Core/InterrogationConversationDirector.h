#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterrogationConversationDirector.generated.h"

class UInterrogationLLMClientComponent;
class UInterrogationVoiceInputComponent;
class UInterrogationVoiceOutputComponent;

UCLASS()
class INTERROGATIONPROTOTYPE_API AInterrogationConversationDirector : public AActor
{
	GENERATED_BODY()

public:
	AInterrogationConversationDirector();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Interrogation|Conversation")
	void StartPlayerVoiceQuestion();

	UFUNCTION(BlueprintCallable, Category = "Interrogation|Conversation")
	void StopPlayerVoiceQuestion();

private:
	UFUNCTION()
	void HandleTranscriptReceived(const FString& Transcript, bool bSuccess, const FString& ErrorMessage);

	UFUNCTION()
	void HandleSuspectResponseReceived(const FString& ResponseText, bool bSuccess, const FString& ErrorMessage);

	UFUNCTION()
	void HandleSpeechPlayback(const FString& SpokenText, bool bSuccess, const FString& ErrorMessage);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Conversation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInterrogationVoiceInputComponent> VoiceInputComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Conversation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInterrogationLLMClientComponent> LLMClientComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Conversation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInterrogationVoiceOutputComponent> VoiceOutputComponent;
};
