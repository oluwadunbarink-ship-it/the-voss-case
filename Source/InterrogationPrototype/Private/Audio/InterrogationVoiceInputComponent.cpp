#include "Audio/InterrogationVoiceInputComponent.h"

#include "Audio/InterrogationSpeechSettings.h"
#include "AudioCaptureCore.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Misc/Guid.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogInterrogationVoiceInput, Log, All);

struct FInterrogationCaptureRuntime
{
	Audio::FAudioCapture Capture;
	FCriticalSection BufferLock;
	TArray<int16> PcmSamples;
	int32 SampleRate = 0;
	int32 NumChannels = 0;
	float MaxRecordingSeconds = 12.0f;
	bool bRecording = false;
};

UInterrogationVoiceInputComponent::UInterrogationVoiceInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bListening = false;
	LastRecordingSeconds = 0.0f;
	CaptureRuntime = MakeUnique<FInterrogationCaptureRuntime>();
}

UInterrogationVoiceInputComponent::~UInterrogationVoiceInputComponent() = default;

void UInterrogationVoiceInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CaptureRuntime)
	{
		{
			FScopeLock Lock(&CaptureRuntime->BufferLock);
			CaptureRuntime->bRecording = false;
		}

		if (CaptureRuntime->Capture.IsCapturing())
		{
			CaptureRuntime->Capture.StopStream();
		}

		if (CaptureRuntime->Capture.IsStreamOpen())
		{
			CaptureRuntime->Capture.CloseStream();
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UInterrogationVoiceInputComponent::StartListening()
{
	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();
	if (!Settings || !Settings->bEnableVoiceRecognition)
	{
		BroadcastFailure(TEXT("Voice recognition is disabled in Interrogation Speech settings."));
		return false;
	}

	if (bListening)
	{
		return true;
	}

	if (!CaptureRuntime)
	{
		CaptureRuntime = MakeUnique<FInterrogationCaptureRuntime>();
	}

	{
		FScopeLock Lock(&CaptureRuntime->BufferLock);
		CaptureRuntime->PcmSamples.Reset();
		CaptureRuntime->SampleRate = 0;
		CaptureRuntime->NumChannels = 0;
		CaptureRuntime->MaxRecordingSeconds = FMath::Max(0.5f, Settings->MaxRecordingSeconds);
		CaptureRuntime->bRecording = true;
	}

	if (!OpenCaptureStream())
	{
		FScopeLock Lock(&CaptureRuntime->BufferLock);
		CaptureRuntime->bRecording = false;
		bListening = false;
		return false;
	}

	if (!CaptureRuntime->Capture.StartStream())
	{
		BroadcastFailure(TEXT("The microphone capture stream could not be started."));
		FScopeLock Lock(&CaptureRuntime->BufferLock);
		CaptureRuntime->bRecording = false;
		bListening = false;
		return false;
	}

	bListening = true;
	UE_LOG(LogInterrogationVoiceInput, Log, TEXT("Started interrogation voice recording."));
	return true;
}

void UInterrogationVoiceInputComponent::StopListening()
{
	if (!bListening || !CaptureRuntime)
	{
		return;
	}

	TArray<int16> RecordedSamples;
	int32 RecordedSampleRate = 0;
	int32 RecordedNumChannels = 0;

	{
		FScopeLock Lock(&CaptureRuntime->BufferLock);
		CaptureRuntime->bRecording = false;
		RecordedSamples = CaptureRuntime->PcmSamples;
		RecordedSampleRate = CaptureRuntime->SampleRate;
		RecordedNumChannels = CaptureRuntime->NumChannels;
	}

	if (CaptureRuntime->Capture.IsCapturing())
	{
		CaptureRuntime->Capture.StopStream();
	}

	bListening = false;

	if (RecordedSamples.Num() == 0 || RecordedSampleRate <= 0 || RecordedNumChannels <= 0)
	{
		BroadcastFailure(TEXT("No microphone audio was captured."));
		return;
	}

	LastRecordingSeconds = static_cast<float>(RecordedSamples.Num()) / static_cast<float>(RecordedSampleRate * RecordedNumChannels);
	UE_LOG(LogInterrogationVoiceInput, Log, TEXT("Stopped voice recording after %.2f seconds."), LastRecordingSeconds);

	const TArray<uint8> WavBytes = BuildWavFile(RecordedSamples, RecordedSampleRate, RecordedNumChannels);
	SubmitTranscriptionRequest(WavBytes);
}

bool UInterrogationVoiceInputComponent::IsListening() const
{
	return bListening;
}

bool UInterrogationVoiceInputComponent::OpenCaptureStream()
{
	if (!CaptureRuntime)
	{
		return false;
	}

	if (CaptureRuntime->Capture.IsStreamOpen())
	{
		return true;
	}

	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();
	const uint32 DesiredFrames = Settings ? static_cast<uint32>(FMath::Max(256, Settings->DesiredCaptureFrames)) : 1024;

	Audio::FAudioCaptureDeviceParams CaptureParams;
	const bool bOpened = CaptureRuntime->Capture.OpenCaptureStream(
		CaptureParams,
		[this](const void* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverflow)
		{
			HandleCapturedAudio(AudioData, NumFrames, NumChannels, SampleRate, StreamTime, bOverflow);
		},
		DesiredFrames);

	if (!bOpened)
	{
		BroadcastFailure(TEXT("The microphone capture stream could not be opened. Check OS microphone permission and Audio Capture plugin support."));
	}

	return bOpened;
}

void UInterrogationVoiceInputComponent::HandleCapturedAudio(const void* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverflow)
{
	if (!AudioData || NumFrames <= 0 || NumChannels <= 0 || SampleRate <= 0 || !CaptureRuntime)
	{
		return;
	}

	if (bOverflow)
	{
		UE_LOG(LogInterrogationVoiceInput, Warning, TEXT("Microphone capture overflow occurred during interrogation recording."));
	}

	const int32 NumSamples = NumFrames * NumChannels;
	const float* FloatSamples = static_cast<const float*>(AudioData);

	FScopeLock Lock(&CaptureRuntime->BufferLock);
	if (!CaptureRuntime->bRecording)
	{
		return;
	}

	CaptureRuntime->SampleRate = SampleRate;
	CaptureRuntime->NumChannels = NumChannels;

	const int32 MaxSamples = FMath::Max(1, FMath::RoundToInt(CaptureRuntime->MaxRecordingSeconds * SampleRate * NumChannels));
	const int32 RemainingSampleBudget = FMath::Max(0, MaxSamples - CaptureRuntime->PcmSamples.Num());
	const int32 SamplesToCopy = FMath::Min(NumSamples, RemainingSampleBudget);

	CaptureRuntime->PcmSamples.Reserve(CaptureRuntime->PcmSamples.Num() + SamplesToCopy);
	for (int32 SampleIndex = 0; SampleIndex < SamplesToCopy; ++SampleIndex)
	{
		const float ClampedSample = FMath::Clamp(FloatSamples[SampleIndex], -1.0f, 1.0f);
		CaptureRuntime->PcmSamples.Add(static_cast<int16>(ClampedSample * 32767.0f));
	}
}

void UInterrogationVoiceInputComponent::SubmitTranscriptionRequest(const TArray<uint8>& WavBytes)
{
	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();
	if (!Settings)
	{
		BroadcastFailure(TEXT("Interrogation Speech settings are unavailable."));
		return;
	}

	if (Settings->TranscriptionEndpointUrl.IsEmpty() || Settings->TranscriptionModel.IsEmpty())
	{
		BroadcastFailure(TEXT("Transcription endpoint or model is not configured."));
		return;
	}

	const FString ApiKey = Settings->ResolveApiKey();
	if (ApiKey.IsEmpty())
	{
		BroadcastFailure(FString::Printf(TEXT("No API key found. Set the %s environment variable before using voice recognition."), *Settings->ApiKeyEnvironmentVariable));
		return;
	}

	const FString Boundary = BuildMultipartBoundary();
	const TArray<uint8> RequestBody = BuildMultipartRequestBody(Boundary, WavBytes, Settings->TranscriptionModel);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->TranscriptionEndpointUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	Request->SetContent(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UInterrogationVoiceInputComponent::HandleTranscriptionResponse);

	if (!Request->ProcessRequest())
	{
		BroadcastFailure(TEXT("The transcription HTTP request could not be started."));
	}
}

void UInterrogationVoiceInputComponent::HandleTranscriptionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		BroadcastFailure(TEXT("The transcription service did not return a valid response."));
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		BroadcastFailure(FString::Printf(TEXT("Transcription failed with HTTP %d: %s"), ResponseCode, *Response->GetContentAsString()));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		BroadcastFailure(TEXT("Transcription response was not valid JSON."));
		return;
	}

	FString Transcript;
	if (!JsonObject->TryGetStringField(TEXT("text"), Transcript))
	{
		BroadcastFailure(TEXT("Transcription response did not contain a text field."));
		return;
	}

	Transcript = Transcript.TrimStartAndEnd();
	if (Transcript.IsEmpty())
	{
		BroadcastFailure(TEXT("Transcription response was empty."));
		return;
	}

	UE_LOG(LogInterrogationVoiceInput, Log, TEXT("Transcript: %s"), *Transcript);
	OnTranscriptReceived.Broadcast(Transcript, true, FString());
}

void UInterrogationVoiceInputComponent::BroadcastFailure(const FString& ErrorMessage)
{
	UE_LOG(LogInterrogationVoiceInput, Warning, TEXT("%s"), *ErrorMessage);
	OnTranscriptReceived.Broadcast(FString(), false, ErrorMessage);
}

TArray<uint8> UInterrogationVoiceInputComponent::BuildWavFile(const TArray<int16>& Samples, int32 SampleRate, int32 NumChannels) const
{
	TArray<uint8> WavBytes;
	const uint16 BitsPerSample = 16;
	const uint16 BlockAlign = static_cast<uint16>(NumChannels * sizeof(int16));
	const uint32 ByteRate = static_cast<uint32>(SampleRate * BlockAlign);
	const uint32 PcmDataBytes = static_cast<uint32>(Samples.Num() * sizeof(int16));

	AppendUtf8String(WavBytes, TEXT("RIFF"));
	AppendUInt32LE(WavBytes, 36 + PcmDataBytes);
	AppendUtf8String(WavBytes, TEXT("WAVE"));

	AppendUtf8String(WavBytes, TEXT("fmt "));
	AppendUInt32LE(WavBytes, 16);
	AppendUInt16LE(WavBytes, 1);
	AppendUInt16LE(WavBytes, static_cast<uint16>(NumChannels));
	AppendUInt32LE(WavBytes, static_cast<uint32>(SampleRate));
	AppendUInt32LE(WavBytes, ByteRate);
	AppendUInt16LE(WavBytes, BlockAlign);
	AppendUInt16LE(WavBytes, BitsPerSample);

	AppendUtf8String(WavBytes, TEXT("data"));
	AppendUInt32LE(WavBytes, PcmDataBytes);

	const uint8* SampleBytes = reinterpret_cast<const uint8*>(Samples.GetData());
	WavBytes.Append(SampleBytes, static_cast<int32>(PcmDataBytes));
	return WavBytes;
}

TArray<uint8> UInterrogationVoiceInputComponent::BuildMultipartRequestBody(const FString& Boundary, const TArray<uint8>& WavBytes, const FString& ModelName) const
{
	TArray<uint8> Body;

	AppendUtf8String(Body, FString::Printf(TEXT("--%s\r\n"), *Boundary));
	AppendUtf8String(Body, TEXT("Content-Disposition: form-data; name=\"model\"\r\n\r\n"));
	AppendUtf8String(Body, ModelName);
	AppendUtf8String(Body, TEXT("\r\n"));

	AppendUtf8String(Body, FString::Printf(TEXT("--%s\r\n"), *Boundary));
	AppendUtf8String(Body, TEXT("Content-Disposition: form-data; name=\"file\"; filename=\"question.wav\"\r\n"));
	AppendUtf8String(Body, TEXT("Content-Type: audio/wav\r\n\r\n"));
	Body.Append(WavBytes);
	AppendUtf8String(Body, TEXT("\r\n"));

	AppendUtf8String(Body, FString::Printf(TEXT("--%s--\r\n"), *Boundary));
	return Body;
}

FString UInterrogationVoiceInputComponent::BuildMultipartBoundary() const
{
	return TEXT("----InterrogationBoundary") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
}

void UInterrogationVoiceInputComponent::AppendUtf8String(TArray<uint8>& OutBytes, const FString& Text) const
{
	FTCHARToUTF8 ConvertedText(*Text);
	OutBytes.Append(reinterpret_cast<const uint8*>(ConvertedText.Get()), ConvertedText.Length());
}

void UInterrogationVoiceInputComponent::AppendUInt16LE(TArray<uint8>& OutBytes, uint16 Value) const
{
	OutBytes.Add(static_cast<uint8>(Value & 0xFF));
	OutBytes.Add(static_cast<uint8>((Value >> 8) & 0xFF));
}

void UInterrogationVoiceInputComponent::AppendUInt32LE(TArray<uint8>& OutBytes, uint32 Value) const
{
	OutBytes.Add(static_cast<uint8>(Value & 0xFF));
	OutBytes.Add(static_cast<uint8>((Value >> 8) & 0xFF));
	OutBytes.Add(static_cast<uint8>((Value >> 16) & 0xFF));
	OutBytes.Add(static_cast<uint8>((Value >> 24) & 0xFF));
}
