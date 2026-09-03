#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/InterrogationDialogueTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "InterrogationLLMClientComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInterrogationLLMResponseSignature, const FString&, ResponseText, bool, bSuccess, const FString&, ErrorMessage);

UCLASS(ClassGroup = (Interrogation), meta = (BlueprintSpawnableComponent))
class INTERROGATIONPROTOTYPE_API UInterrogationLLMClientComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInterrogationLLMClientComponent();

	UFUNCTION(BlueprintCallable, Category = "Interrogation|LLM")
	void RequestSuspectResponse(const FString& PlayerTranscript);

	UFUNCTION(BlueprintCallable, Category = "Interrogation|LLM")
	void ResetDialogueMemory();

	UFUNCTION(BlueprintPure, Category = "Interrogation|LLM")
	TArray<FInterrogationDialogueTurn> GetDialogueMemory() const;

	UPROPERTY(BlueprintAssignable, Category = "Interrogation|LLM")
	FInterrogationLLMResponseSignature OnSuspectResponseReceived;

private:
	FString BuildChatRequestBody(const FString& PlayerTranscript) const;
	void HandleChatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void BroadcastFailure(const FString& ErrorMessage);
	void AddDialogueTurn(const FString& Role, const FString& Text);
	bool TryExtractAssistantText(const FString& ResponseBody, FString& OutAssistantText) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|LLM", meta = (AllowPrivateAccess = "true"))
	TArray<FInterrogationDialogueTurn> DialogueMemory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|LLM", meta = (AllowPrivateAccess = "true"))
	bool bRequestInFlight;

	FString PendingPlayerTranscript;
};
