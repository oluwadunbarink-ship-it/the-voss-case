#include "Audio/InterrogationVoiceOutputComponent.h"

#include "Audio/InterrogationSpeechSettings.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Sound/SoundWaveProcedural.h"

DEFINE_LOG_CATEGORY_STATIC(LogInterrogationVoiceOutput, Log, All);

namespace
{
	bool HasFourCc(const TArray<uint8>& Bytes, int32 Offset, const ANSICHAR* FourCc)
	{
		return Bytes.IsValidIndex(Offset + 3)
			&& Bytes[Offset] == static_cast<uint8>(FourCc[0])
			&& Bytes[Offset + 1] == static_cast<uint8>(FourCc[1])
			&& Bytes[Offset + 2] == static_cast<uint8>(FourCc[2])
			&& Bytes[Offset + 3] == static_cast<uint8>(FourCc[3]);
	}

	uint16 ReadUInt16LE(const TArray<uint8>& Bytes, int32 Offset)
	{
		return static_cast<uint16>(static_cast<uint16>(Bytes[Offset]) | (static_cast<uint16>(Bytes[Offset + 1]) << 8));
	}

	uint32 ReadUInt32LE(const TArray<uint8>& Bytes, int32 Offset)
	{
		return static_cast<uint32>(Bytes[Offset])
			| (static_cast<uint32>(Bytes[Offset + 1]) << 8)
			| (static_cast<uint32>(Bytes[Offset + 2]) << 16)
			| (static_cast<uint32>(Bytes[Offset + 3]) << 24);
	}
}

UInterrogationVoiceOutputComponent::UInterrogationVoiceOutputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInterrogationVoiceOutputComponent::SpeakText(const FString& TextToSpeak)
{
	const FString CleanText = TextToSpeak.TrimStartAndEnd();
	if (CleanText.IsEmpty())
	{
		BroadcastFailure(TEXT("Cannot speak an empty suspect response."));
		return;
	}

	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();
	if (!Settings)
	{
		BroadcastFailure(TEXT("Interrogation Speech settings are unavailable."));
		return;
	}

	if (!Settings->bEnableSpokenResponses)
	{
		BroadcastFailure(TEXT("Spoken responses are disabled in Interrogation Speech settings."));
		return;
	}

	if (Settings->TextToSpeechEndpointUrl.IsEmpty() || Settings->TextToSpeechModel.IsEmpty())
	{
		BroadcastFailure(TEXT("Text-to-speech endpoint or model is not configured."));
		return;
	}

	const FString ApiKey = Settings->ResolveApiKey();
	if (ApiKey.IsEmpty())
	{
		BroadcastFailure(FString::Printf(TEXT("No API key found. Set the %s environment variable before using text-to-speech."), *Settings->ApiKeyEnvironmentVariable));
		return;
	}

	PendingSpeechText = CleanText;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->TextToSpeechEndpointUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("audio/wav"));
	Request->SetContentAsString(BuildSpeechRequestBody(CleanText));
	Request->OnProcessRequestComplete().BindUObject(this, &UInterrogationVoiceOutputComponent::HandleSpeechResponse);

	if (!Request->ProcessRequest())
	{
		PendingSpeechText.Reset();
		BroadcastFailure(TEXT("The text-to-speech HTTP request could not be started."));
	}
}

FString UInterrogationVoiceOutputComponent::BuildSpeechRequestBody(const FString& TextToSpeak) const
{
	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("model"), Settings ? Settings->TextToSpeechModel : FString());
	RootObject->SetStringField(TEXT("voice"), Settings ? Settings->TextToSpeechVoice : FString());
	RootObject->SetStringField(TEXT("input"), TextToSpeak);
	RootObject->SetStringField(TEXT("response_format"), Settings ? Settings->TextToSpeechResponseFormat : TEXT("wav"));

	FString RequestBody;
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter);
	return RequestBody;
}

void UInterrogationVoiceOutputComponent::HandleSpeechResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		PendingSpeechText.Reset();
		BroadcastFailure(TEXT("The text-to-speech service did not return a valid response."));
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		PendingSpeechText.Reset();
		BroadcastFailure(FString::Printf(TEXT("Text-to-speech failed with HTTP %d: %s"), ResponseCode, *Response->GetContentAsString()));
		return;
	}

	FInterrogationPcmWaveData WaveData;
	FString ParseError;
	if (!TryParsePcm16Wav(Response->GetContent(), WaveData, ParseError))
	{
		PendingSpeechText.Reset();
		BroadcastFailure(ParseError);
		return;
	}

	PlayWaveData(WaveData);
	OnSpeechPlayback.Broadcast(PendingSpeechText, true, FString());
	PendingSpeechText.Reset();
}

bool UInterrogationVoiceOutputComponent::TryParsePcm16Wav(const TArray<uint8>& WavBytes, FInterrogationPcmWaveData& OutWaveData, FString& OutError) const
{
	if (WavBytes.Num() < 44 || !HasFourCc(WavBytes, 0, "RIFF") || !HasFourCc(WavBytes, 8, "WAVE"))
	{
		OutError = TEXT("Text-to-speech response was not a valid WAV file.");
		return false;
	}

	bool bFoundFormat = false;
	bool bFoundData = false;
	uint16 AudioFormat = 0;
	uint16 NumChannels = 0;
	uint32 SampleRate = 0;
	uint16 BitsPerSample = 0;
	int32 DataOffset = 0;
	uint32 DataSize = 0;

	int32 ChunkOffset = 12;
	while (ChunkOffset + 8 <= WavBytes.Num())
	{
		const uint32 ChunkSize = ReadUInt32LE(WavBytes, ChunkOffset + 4);
		const int32 ChunkDataOffset = ChunkOffset + 8;
		if (ChunkDataOffset + static_cast<int32>(ChunkSize) > WavBytes.Num())
		{
			break;
		}

		if (HasFourCc(WavBytes, ChunkOffset, "fmt "))
		{
			if (ChunkSize < 16)
			{
				OutError = TEXT("WAV fmt chunk was too small.");
				return false;
			}

			AudioFormat = ReadUInt16LE(WavBytes, ChunkDataOffset);
			NumChannels = ReadUInt16LE(WavBytes, ChunkDataOffset + 2);
			SampleRate = ReadUInt32LE(WavBytes, ChunkDataOffset + 4);
			BitsPerSample = ReadUInt16LE(WavBytes, ChunkDataOffset + 14);
			bFoundFormat = true;
		}
		else if (HasFourCc(WavBytes, ChunkOffset, "data"))
		{
			DataOffset = ChunkDataOffset;
			DataSize = ChunkSize;
			bFoundData = true;
		}

		ChunkOffset = ChunkDataOffset + static_cast<int32>(ChunkSize) + static_cast<int32>(ChunkSize % 2);
	}

	if (!bFoundFormat || !bFoundData)
	{
		OutError = TEXT("WAV response was missing a fmt or data chunk.");
		return false;
	}

	if (AudioFormat != 1 || BitsPerSample != 16 || NumChannels == 0 || SampleRate == 0)
	{
		OutError = TEXT("Only 16-bit PCM WAV speech responses are supported by this prototype.");
		return false;
	}

	OutWaveData.SampleRate = static_cast<int32>(SampleRate);
	OutWaveData.NumChannels = static_cast<int32>(NumChannels);
	OutWaveData.PcmBytes.Append(&WavBytes[DataOffset], static_cast<int32>(DataSize));
	OutWaveData.DurationSeconds = static_cast<float>(DataSize) / static_cast<float>(SampleRate * NumChannels * sizeof(int16));
	return OutWaveData.PcmBytes.Num() > 0;
}

void UInterrogationVoiceOutputComponent::PlayWaveData(const FInterrogationPcmWaveData& WaveData)
{
	USoundWaveProcedural* SpeechWave = NewObject<USoundWaveProcedural>(this);
	SpeechWave->SetSampleRate(WaveData.SampleRate);
	SpeechWave->NumChannels = WaveData.NumChannels;
	SpeechWave->Duration = WaveData.DurationSeconds;
	SpeechWave->SoundGroup = SOUNDGROUP_Voice;
	SpeechWave->bLooping = false;
	SpeechWave->QueueAudio(WaveData.PcmBytes.GetData(), WaveData.PcmBytes.Num());

	ActiveSpeechWave = SpeechWave;
	UGameplayStatics::PlaySound2D(this, ActiveSpeechWave);
	UE_LOG(LogInterrogationVoiceOutput, Log, TEXT("Playing suspect speech for %.2f seconds."), WaveData.DurationSeconds);
}

void UInterrogationVoiceOutputComponent::BroadcastFailure(const FString& ErrorMessage)
{
	UE_LOG(LogInterrogationVoiceOutput, Warning, TEXT("%s"), *ErrorMessage);
	OnSpeechPlayback.Broadcast(FString(), false, ErrorMessage);
}
