#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Templates/UniquePtr.h"
#include "InterrogationVoiceInputComponent.generated.h"

struct FInterrogationCaptureRuntime;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInterrogationVoiceTranscriptSignature, const FString&, Transcript, bool, bSuccess, const FString&, ErrorMessage);

UCLASS(ClassGroup = (Interrogation), meta = (BlueprintSpawnableComponent))
class INTERROGATIONPROTOTYPE_API UInterrogationVoiceInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInterrogationVoiceInputComponent();
	virtual ~UInterrogationVoiceInputComponent() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Interrogation|Voice")
	bool StartListening();

	UFUNCTION(BlueprintCallable, Category = "Interrogation|Voice")
	void StopListening();

	UFUNCTION(BlueprintPure, Category = "Interrogation|Voice")
	bool IsListening() const;

	UPROPERTY(BlueprintAssignable, Category = "Interrogation|Voice")
	FInterrogationVoiceTranscriptSignature OnTranscriptReceived;

private:
	bool OpenCaptureStream();
	void HandleCapturedAudio(const void* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverflow);
	void SubmitTranscriptionRequest(const TArray<uint8>& WavBytes);
	void HandleTranscriptionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void BroadcastFailure(const FString& ErrorMessage);

	TArray<uint8> BuildWavFile(const TArray<int16>& Samples, int32 SampleRate, int32 NumChannels) const;
	TArray<uint8> BuildMultipartRequestBody(const FString& Boundary, const TArray<uint8>& WavBytes, const FString& ModelName) const;
	FString BuildMultipartBoundary() const;
	void AppendUtf8String(TArray<uint8>& OutBytes, const FString& Text) const;
	void AppendUInt16LE(TArray<uint8>& OutBytes, uint16 Value) const;
	void AppendUInt32LE(TArray<uint8>& OutBytes, uint32 Value) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Voice", meta = (AllowPrivateAccess = "true"))
	bool bListening;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Voice", meta = (AllowPrivateAccess = "true"))
	float LastRecordingSeconds;

	TUniquePtr<FInterrogationCaptureRuntime> CaptureRuntime;
};
