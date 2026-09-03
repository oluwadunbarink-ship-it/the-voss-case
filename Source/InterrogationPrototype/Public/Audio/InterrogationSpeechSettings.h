#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InterrogationSpeechSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Interrogation Speech Services"))
class INTERROGATIONPROTOTYPE_API UInterrogationSpeechSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UInterrogationSpeechSettings();

	UFUNCTION(BlueprintPure, Category = "Interrogation|Speech")
	FString ResolveApiKey() const;

#if WITH_EDITOR
	virtual FName GetCategoryName() const override;
#endif

	UPROPERTY(Config, EditAnywhere, Category = "Credentials")
	FString ApiKeyEnvironmentVariable;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Recognition")
	bool bEnableVoiceRecognition;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Recognition")
	FString TranscriptionEndpointUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Recognition")
	FString TranscriptionModel;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Recognition", meta = (ClampMin = "0.5", UIMin = "0.5"))
	float MaxRecordingSeconds;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Recognition", meta = (ClampMin = "256", UIMin = "256"))
	int32 DesiredCaptureFrames;

	UPROPERTY(Config, EditAnywhere, Category = "LLM")
	FString ChatEndpointUrl;

	UPROPERTY(Config, EditAnywhere, Category = "LLM")
	FString ChatModel;

	UPROPERTY(Config, EditAnywhere, Category = "LLM", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float ChatTemperature;

	UPROPERTY(Config, EditAnywhere, Category = "LLM", meta = (ClampMin = "1", UIMin = "1"))
	int32 ChatMaxTokens;

	UPROPERTY(Config, EditAnywhere, Category = "LLM", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxDialogueMemoryTurns;

	UPROPERTY(Config, EditAnywhere, Category = "LLM", meta = (MultiLine = "true"))
	FString SuspectSystemPrompt;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Output")
	bool bEnableSpokenResponses;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Output")
	FString TextToSpeechEndpointUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Output")
	FString TextToSpeechModel;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Output")
	FString TextToSpeechVoice;

	UPROPERTY(Config, EditAnywhere, Category = "Voice Output")
	FString TextToSpeechResponseFormat;
};
