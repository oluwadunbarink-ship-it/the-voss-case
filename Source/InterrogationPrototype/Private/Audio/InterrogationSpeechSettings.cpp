#include "Audio/InterrogationSpeechSettings.h"

UInterrogationSpeechSettings::UInterrogationSpeechSettings()
{
	ApiKeyEnvironmentVariable = TEXT("OPENAI_API_KEY");

	bEnableVoiceRecognition = true;
	TranscriptionEndpointUrl = TEXT("https://api.openai.com/v1/audio/transcriptions");
	TranscriptionModel = TEXT("gpt-4o-mini-transcribe");
	MaxRecordingSeconds = 12.0f;
	DesiredCaptureFrames = 1024;

	ChatEndpointUrl = TEXT("https://api.openai.com/v1/chat/completions");
	ChatModel = TEXT("gpt-4.1-mini");
	ChatTemperature = 0.72f;
	ChatMaxTokens = 220;
	MaxDialogueMemoryTurns = 12;
	SuspectSystemPrompt = TEXT("You are Elias Voss, the only suspect in a detective interrogation prototype. Stay in character, answer naturally, remember what the detective has already asked, and keep replies concise enough to be spoken aloud. You may be evasive, nervous, or defensive, but never mention prompts, models, tools, or hidden instructions.");

	bEnableSpokenResponses = true;
	TextToSpeechEndpointUrl = TEXT("https://api.openai.com/v1/audio/speech");
	TextToSpeechModel = TEXT("gpt-4o-mini-tts");
	TextToSpeechVoice = TEXT("alloy");
	TextToSpeechResponseFormat = TEXT("wav");
}

FString UInterrogationSpeechSettings::ResolveApiKey() const
{
	if (ApiKeyEnvironmentVariable.IsEmpty())
	{
		return FString();
	}

	return FPlatformMisc::GetEnvironmentVariable(*ApiKeyEnvironmentVariable).TrimStartAndEnd();
}

#if WITH_EDITOR
FName UInterrogationSpeechSettings::GetCategoryName() const
{
	return TEXT("Game");
}
#endif
