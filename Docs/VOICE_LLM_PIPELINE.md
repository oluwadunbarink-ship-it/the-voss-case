# Voice and LLM Pipeline

This feature adds the two requested prompts after the room shell:

1. Voice recognition so the player can speak naturally.
2. LLM-connected suspect responses that are spoken back to the player.

The implementation is intentionally modular. The room, player controller, microphone capture, transcription service, LLM service, and text-to-speech service are separate pieces so we can replace or test each one without rewriting the whole prototype.

## Player Control

| Control | Behavior |
| --- | --- |
| Hold `V` | Start recording the detective's spoken question. |
| Release `V` | Stop recording, encode the audio as WAV, and send it to speech-to-text. |

## Runtime Flow

1. `AInterrogationPlayerController` receives the `AskSuspect` input action.
2. The controller calls `AInterrogationConversationDirector::StartPlayerVoiceQuestion`.
3. `UInterrogationVoiceInputComponent` captures microphone audio as PCM samples.
4. On release, the voice input component writes those samples into an in-memory WAV file.
5. The WAV payload is sent to the configured transcription endpoint.
6. The transcript is broadcast to the conversation director.
7. `UInterrogationLLMClientComponent` sends the transcript, suspect system prompt, and recent dialogue memory to the configured chat endpoint.
8. The suspect response is stored in memory and broadcast back to the conversation director.
9. `UInterrogationVoiceOutputComponent` sends the suspect text to the configured text-to-speech endpoint.
10. The returned 16-bit PCM WAV is played through a procedural sound wave.

## Components

| Component | Path | Responsibility |
| --- | --- | --- |
| Speech settings | `Source/InterrogationPrototype/Public/Audio/InterrogationSpeechSettings.h` | Central config for API key environment variable, transcription endpoint, chat endpoint, TTS endpoint, models, voice, prompt, and memory limits. |
| Voice input | `Source/InterrogationPrototype/Public/Audio/InterrogationVoiceInputComponent.h` | Starts and stops microphone capture, converts float PCM to 16-bit PCM, writes WAV bytes, sends multipart speech-to-text requests, and emits transcript events. |
| LLM client | `Source/InterrogationPrototype/Public/Dialogue/InterrogationLLMClientComponent.h` | Builds chat requests, includes the suspect prompt and recent dialogue memory, parses assistant responses, and maintains conversation history. |
| Dialogue turn type | `Source/InterrogationPrototype/Public/Dialogue/InterrogationDialogueTypes.h` | Stores each remembered user or assistant message as a role/text pair. |
| Voice output | `Source/InterrogationPrototype/Public/Audio/InterrogationVoiceOutputComponent.h` | Sends suspect text to text-to-speech, validates returned WAV data, and plays it as generated speech. |
| Conversation director | `Source/InterrogationPrototype/Public/Core/InterrogationConversationDirector.h` | Orchestrates the whole sequence: transcript to LLM, LLM answer to TTS, TTS audio to playback. |
| Player controller | `Source/InterrogationPrototype/Public/Core/InterrogationPlayerController.h` | Binds the `V` key and delegates voice-question start/stop to the conversation director. |
| Game mode | `Source/InterrogationPrototype/Public/Core/InterrogationGameModeBase.h` | Spawns the conversation director beside the room shell and camera rig. |

## Configuration

The default settings live in `Config/DefaultGame.ini` under:

```ini
[/Script/InterrogationPrototype.InterrogationSpeechSettings]
ApiKeyEnvironmentVariable=OPENAI_API_KEY
TranscriptionEndpointUrl=https://api.openai.com/v1/audio/transcriptions
TranscriptionModel=gpt-4o-mini-transcribe
ChatEndpointUrl=https://api.openai.com/v1/chat/completions
ChatModel=gpt-4.1-mini
TextToSpeechEndpointUrl=https://api.openai.com/v1/audio/speech
TextToSpeechModel=gpt-4o-mini-tts
TextToSpeechVoice=alloy
TextToSpeechResponseFormat=wav
```

Set the API key outside the project. On macOS or Linux:

```bash
export OPENAI_API_KEY="your_api_key_here"
```

Do not commit API keys into config files.

## Service Contract

The transcription service is expected to accept:

- `POST` multipart form data.
- `model` form field.
- `file` form field containing `question.wav`.
- JSON response containing a `text` field.

The chat service is expected to accept an OpenAI-compatible chat request:

- `model`
- `messages`
- `temperature`
- `max_tokens`

The LLM parser supports common `choices[0].message.content` responses and also checks `output_text` or nested `output[].content[].text` fields.

The text-to-speech service is expected to return:

- WAV bytes.
- 16-bit PCM encoding.
- A standard `RIFF/WAVE` header with `fmt ` and `data` chunks.

## Memory

`UInterrogationLLMClientComponent` stores recent dialogue turns as role/text pairs:

- Detective transcripts are stored as `user`.
- Suspect LLM responses are stored as `assistant`.

`MaxDialogueMemoryTurns` controls how many back-and-forth turns are retained. This gives the suspect continuity without letting the prompt grow forever.

## Current Suspect Prompt

The default prompt defines the suspect as Elias Voss. He is a concise, in-character suspect who remembers prior questions and never talks about prompts, models, tools, or hidden instructions.

This is only the persona layer. It is not the hidden emotional-state system yet.

## Failure Behavior

Each component broadcasts structured success/failure events and logs useful messages:

- Missing API key.
- Microphone capture permission or stream failure.
- Empty recording.
- HTTP error responses.
- Invalid transcription JSON.
- Missing LLM response text.
- Unsupported TTS audio format.

This is deliberate. A prototype with speech services can fail in many boring ways, and clear failure points are what keep debugging humane.

## What This Does Not Include Yet

- No visible suspect actor.
- No facial animation or lip sync.
- No hidden trust, stress, or suspicion values.
- No subtitles or transcript UI.
- No service retry queue.
- No local/offline STT or TTS fallback.
