#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "InterrogationVoiceOutputComponent.generated.h"

class USoundWaveProcedural;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInterrogationSpeechPlaybackSignature, const FString&, SpokenText, bool, bSuccess, const FString&, ErrorMessage);

struct FInterrogationPcmWaveData
{
	int32 SampleRate = 0;
	int32 NumChannels = 0;
	float DurationSeconds = 0.0f;
	TArray<uint8> PcmBytes;
};

UCLASS(ClassGroup = (Interrogation), meta = (BlueprintSpawnableComponent))
class INTERROGATIONPROTOTYPE_API UInterrogationVoiceOutputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInterrogationVoiceOutputComponent();

	UFUNCTION(BlueprintCallable, Category = "Interrogation|Voice")
	void SpeakText(const FString& TextToSpeak);

	UPROPERTY(BlueprintAssignable, Category = "Interrogation|Voice")
	FInterrogationSpeechPlaybackSignature OnSpeechPlayback;

private:
	FString BuildSpeechRequestBody(const FString& TextToSpeak) const;
	void HandleSpeechResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	bool TryParsePcm16Wav(const TArray<uint8>& WavBytes, FInterrogationPcmWaveData& OutWaveData, FString& OutError) const;
	void PlayWaveData(const FInterrogationPcmWaveData& WaveData);
	void BroadcastFailure(const FString& ErrorMessage);

	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> ActiveSpeechWave;

	FString PendingSpeechText;
};
