#include "Core/InterrogationConversationDirector.h"

#include "Audio/InterrogationVoiceInputComponent.h"
#include "Audio/InterrogationVoiceOutputComponent.h"
#include "Dialogue/InterrogationLLMClientComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogInterrogationConversationDirector, Log, All);

AInterrogationConversationDirector::AInterrogationConversationDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	VoiceInputComponent = CreateDefaultSubobject<UInterrogationVoiceInputComponent>(TEXT("VoiceInputComponent"));
	LLMClientComponent = CreateDefaultSubobject<UInterrogationLLMClientComponent>(TEXT("LLMClientComponent"));
	VoiceOutputComponent = CreateDefaultSubobject<UInterrogationVoiceOutputComponent>(TEXT("VoiceOutputComponent"));
}

void AInterrogationConversationDirector::BeginPlay()
{
	Super::BeginPlay();

	if (VoiceInputComponent)
	{
		VoiceInputComponent->OnTranscriptReceived.AddDynamic(this, &AInterrogationConversationDirector::HandleTranscriptReceived);
	}

	if (LLMClientComponent)
	{
		LLMClientComponent->OnSuspectResponseReceived.AddDynamic(this, &AInterrogationConversationDirector::HandleSuspectResponseReceived);
	}

	if (VoiceOutputComponent)
	{
		VoiceOutputComponent->OnSpeechPlayback.AddDynamic(this, &AInterrogationConversationDirector::HandleSpeechPlayback);
	}
}

void AInterrogationConversationDirector::StartPlayerVoiceQuestion()
{
	if (VoiceInputComponent)
	{
		VoiceInputComponent->StartListening();
	}
}

void AInterrogationConversationDirector::StopPlayerVoiceQuestion()
{
	if (VoiceInputComponent)
	{
		VoiceInputComponent->StopListening();
	}
}

void AInterrogationConversationDirector::HandleTranscriptReceived(const FString& Transcript, bool bSuccess, const FString& ErrorMessage)
{
	if (!bSuccess)
	{
		UE_LOG(LogInterrogationConversationDirector, Warning, TEXT("Voice transcript failed: %s"), *ErrorMessage);
		return;
	}

	UE_LOG(LogInterrogationConversationDirector, Log, TEXT("Detective transcript: %s"), *Transcript);
	if (LLMClientComponent)
	{
		LLMClientComponent->RequestSuspectResponse(Transcript);
	}
}

void AInterrogationConversationDirector::HandleSuspectResponseReceived(const FString& ResponseText, bool bSuccess, const FString& ErrorMessage)
{
	if (!bSuccess)
	{
		UE_LOG(LogInterrogationConversationDirector, Warning, TEXT("Suspect response failed: %s"), *ErrorMessage);
		return;
	}

	UE_LOG(LogInterrogationConversationDirector, Log, TEXT("Suspect text: %s"), *ResponseText);
	if (VoiceOutputComponent)
	{
		VoiceOutputComponent->SpeakText(ResponseText);
	}
}

void AInterrogationConversationDirector::HandleSpeechPlayback(const FString& SpokenText, bool bSuccess, const FString& ErrorMessage)
{
	if (!bSuccess)
	{
		UE_LOG(LogInterrogationConversationDirector, Warning, TEXT("Suspect speech failed: %s"), *ErrorMessage);
		return;
	}

	UE_LOG(LogInterrogationConversationDirector, Log, TEXT("Suspect speech playback started."));
}
