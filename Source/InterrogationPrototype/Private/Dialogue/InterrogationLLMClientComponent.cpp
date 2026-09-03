#include "Dialogue/InterrogationLLMClientComponent.h"

#include "Audio/InterrogationSpeechSettings.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogInterrogationLLM, Log, All);

UInterrogationLLMClientComponent::UInterrogationLLMClientComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bRequestInFlight = false;
}

void UInterrogationLLMClientComponent::RequestSuspectResponse(const FString& PlayerTranscript)
{
	const FString CleanTranscript = PlayerTranscript.TrimStartAndEnd();
	if (CleanTranscript.IsEmpty())
	{
		BroadcastFailure(TEXT("Cannot request an LLM response for an empty transcript."));
		return;
	}

	if (bRequestInFlight)
	{
		BroadcastFailure(TEXT("A suspect response request is already in progress."));
		return;
	}

	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();
	if (!Settings)
	{
		BroadcastFailure(TEXT("Interrogation Speech settings are unavailable."));
		return;
	}

	if (Settings->ChatEndpointUrl.IsEmpty() || Settings->ChatModel.IsEmpty())
	{
		BroadcastFailure(TEXT("Chat endpoint or model is not configured."));
		return;
	}

	const FString ApiKey = Settings->ResolveApiKey();
	if (ApiKey.IsEmpty())
	{
		BroadcastFailure(FString::Printf(TEXT("No API key found. Set the %s environment variable before using the LLM."), *Settings->ApiKeyEnvironmentVariable));
		return;
	}

	PendingPlayerTranscript = CleanTranscript;
	bRequestInFlight = true;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ChatEndpointUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContentAsString(BuildChatRequestBody(CleanTranscript));
	Request->OnProcessRequestComplete().BindUObject(this, &UInterrogationLLMClientComponent::HandleChatResponse);

	if (!Request->ProcessRequest())
	{
		bRequestInFlight = false;
		PendingPlayerTranscript.Reset();
		BroadcastFailure(TEXT("The LLM HTTP request could not be started."));
	}
}

void UInterrogationLLMClientComponent::ResetDialogueMemory()
{
	DialogueMemory.Reset();
}

TArray<FInterrogationDialogueTurn> UInterrogationLLMClientComponent::GetDialogueMemory() const
{
	return DialogueMemory;
}

FString UInterrogationLLMClientComponent::BuildChatRequestBody(const FString& PlayerTranscript) const
{
	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();

	TArray<TSharedPtr<FJsonValue>> Messages;

	auto AddMessage = [&Messages](const FString& Role, const FString& Text)
	{
		TSharedPtr<FJsonObject> MessageObject = MakeShared<FJsonObject>();
		MessageObject->SetStringField(TEXT("role"), Role);
		MessageObject->SetStringField(TEXT("content"), Text);
		Messages.Add(MakeShared<FJsonValueObject>(MessageObject));
	};

	if (Settings && !Settings->SuspectSystemPrompt.IsEmpty())
	{
		AddMessage(TEXT("system"), Settings->SuspectSystemPrompt);
	}

	for (const FInterrogationDialogueTurn& Turn : DialogueMemory)
	{
		if (!Turn.Role.IsEmpty() && !Turn.Text.IsEmpty())
		{
			AddMessage(Turn.Role, Turn.Text);
		}
	}

	AddMessage(TEXT("user"), PlayerTranscript);

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("model"), Settings ? Settings->ChatModel : FString());
	RootObject->SetArrayField(TEXT("messages"), Messages);
	RootObject->SetNumberField(TEXT("temperature"), Settings ? Settings->ChatTemperature : 0.7f);
	RootObject->SetNumberField(TEXT("max_tokens"), Settings ? Settings->ChatMaxTokens : 220);

	FString RequestBody;
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter);
	return RequestBody;
}

void UInterrogationLLMClientComponent::HandleChatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	bRequestInFlight = false;

	if (!bWasSuccessful || !Response.IsValid())
	{
		PendingPlayerTranscript.Reset();
		BroadcastFailure(TEXT("The LLM service did not return a valid response."));
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		PendingPlayerTranscript.Reset();
		BroadcastFailure(FString::Printf(TEXT("LLM request failed with HTTP %d: %s"), ResponseCode, *Response->GetContentAsString()));
		return;
	}

	FString AssistantText;
	if (!TryExtractAssistantText(Response->GetContentAsString(), AssistantText))
	{
		PendingPlayerTranscript.Reset();
		BroadcastFailure(TEXT("LLM response did not contain assistant text."));
		return;
	}

	AddDialogueTurn(TEXT("user"), PendingPlayerTranscript);
	AddDialogueTurn(TEXT("assistant"), AssistantText);
	PendingPlayerTranscript.Reset();

	UE_LOG(LogInterrogationLLM, Log, TEXT("Suspect response: %s"), *AssistantText);
	OnSuspectResponseReceived.Broadcast(AssistantText, true, FString());
}

void UInterrogationLLMClientComponent::BroadcastFailure(const FString& ErrorMessage)
{
	UE_LOG(LogInterrogationLLM, Warning, TEXT("%s"), *ErrorMessage);
	OnSuspectResponseReceived.Broadcast(FString(), false, ErrorMessage);
}

void UInterrogationLLMClientComponent::AddDialogueTurn(const FString& Role, const FString& Text)
{
	const FString CleanText = Text.TrimStartAndEnd();
	if (Role.IsEmpty() || CleanText.IsEmpty())
	{
		return;
	}

	DialogueMemory.Add(FInterrogationDialogueTurn(Role, CleanText));

	const UInterrogationSpeechSettings* Settings = GetDefault<UInterrogationSpeechSettings>();
	const int32 MaxEntries = Settings ? FMath::Max(1, Settings->MaxDialogueMemoryTurns) * 2 : 24;
	while (DialogueMemory.Num() > MaxEntries)
	{
		DialogueMemory.RemoveAt(0);
	}
}

bool UInterrogationLLMClientComponent::TryExtractAssistantText(const FString& ResponseBody, FString& OutAssistantText) const
{
	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ResponseBody);
	if (!FJsonSerializer::Deserialize(JsonReader, RootObject) || !RootObject.IsValid())
	{
		return false;
	}

	FString OutputText;
	if (RootObject->TryGetStringField(TEXT("output_text"), OutputText))
	{
		OutAssistantText = OutputText.TrimStartAndEnd();
		return !OutAssistantText.IsEmpty();
	}

	const TArray<TSharedPtr<FJsonValue>>* Choices = nullptr;
	if (RootObject->TryGetArrayField(TEXT("choices"), Choices) && Choices && Choices->Num() > 0)
	{
		const TSharedPtr<FJsonValue> FirstChoice = (*Choices)[0];
		const TSharedPtr<FJsonObject> ChoiceObject = FirstChoice.IsValid() ? FirstChoice->AsObject() : nullptr;
		if (ChoiceObject.IsValid())
		{
			const TSharedPtr<FJsonObject>* MessageObject = nullptr;
			if (ChoiceObject->TryGetObjectField(TEXT("message"), MessageObject) && MessageObject && MessageObject->IsValid())
			{
				FString Content;
				if ((*MessageObject)->TryGetStringField(TEXT("content"), Content))
				{
					OutAssistantText = Content.TrimStartAndEnd();
					return !OutAssistantText.IsEmpty();
				}
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* OutputArray = nullptr;
	if (RootObject->TryGetArrayField(TEXT("output"), OutputArray) && OutputArray)
	{
		for (const TSharedPtr<FJsonValue>& OutputValue : *OutputArray)
		{
			const TSharedPtr<FJsonObject> OutputObject = OutputValue.IsValid() ? OutputValue->AsObject() : nullptr;
			if (!OutputObject.IsValid())
			{
				continue;
			}

			const TArray<TSharedPtr<FJsonValue>>* ContentArray = nullptr;
			if (!OutputObject->TryGetArrayField(TEXT("content"), ContentArray) || !ContentArray)
			{
				continue;
			}

			for (const TSharedPtr<FJsonValue>& ContentValue : *ContentArray)
			{
				const TSharedPtr<FJsonObject> ContentObject = ContentValue.IsValid() ? ContentValue->AsObject() : nullptr;
				if (!ContentObject.IsValid())
				{
					continue;
				}

				FString Text;
				if (ContentObject->TryGetStringField(TEXT("text"), Text))
				{
					OutAssistantText = Text.TrimStartAndEnd();
					return !OutAssistantText.IsEmpty();
				}
			}
		}
	}

	return false;
}
