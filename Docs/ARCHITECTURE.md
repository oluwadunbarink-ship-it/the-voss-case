# Interrogation Prototype Architecture

This project is a UE5 prototype scaffold, not a finished game. The architecture is designed to let each system be added and tested in isolation before combining them into a playable interrogation loop.

## Prototype Pillars

1. **Single interrogation room**: A focused, controllable space for testing tone, camera, lighting, and interaction pacing.
2. **Single AI suspect**: One believable character is enough to validate the dialogue, memory, and emotion systems.
3. **Voice input**: The player asks questions through a microphone.
4. **Voice output**: The suspect answers through synthesized speech.
5. **Hidden emotional state**: Trust, stress, and suspicion change quietly behind the scenes and influence responses.
6. **Conversation memory**: Previous questions and answers are preserved so the suspect can refer back to them naturally.
7. **Replaceable LLM service**: LLM calls are isolated behind a service layer so provider details do not leak into gameplay code.

## Root Files

| Path | Purpose |
| --- | --- |
| `InterrogationPrototype.uproject` | Unreal project descriptor. It defines the runtime module and enables plugins needed later, including Enhanced Input and Audio Capture. |
| `.gitignore` | Keeps generated Unreal, IDE, build, and log files out of source control. |
| `.editorconfig` | Establishes shared whitespace and newline rules for code and config files. |
| `README.md` | Human entry point describing the prototype goal and current status. |

## Config

| Path | Purpose |
| --- | --- |
| `Config/DefaultEngine.ini` | Engine-level settings. The default map will be assigned after the playable room exists. |
| `Config/DefaultGame.ini` | Project metadata shown by Unreal. |
| `Config/DefaultInput.ini` | Reserved for project input settings. Enhanced Input assets will be added later under `Content/Input`. |

## Source Module

| Path | Purpose |
| --- | --- |
| `Source/InterrogationPrototype.Target.cs` | Build target for the standalone game. |
| `Source/InterrogationPrototypeEditor.Target.cs` | Build target for the Unreal Editor. |
| `Source/InterrogationPrototype/InterrogationPrototype.Build.cs` | Module dependency list. It already reserves dependencies for input, AI, audio capture, HTTP, JSON, and UI work. |
| `Source/InterrogationPrototype/Public/InterrogationPrototype.h` | Shared public module header. |
| `Source/InterrogationPrototype/Private/InterrogationPrototype.cpp` | Primary module implementation used by Unreal to load the game module. |

## Source Folders

| Folder | Responsibility |
| --- | --- |
| `Source/InterrogationPrototype/Public/Core` and `Private/Core` | Project-wide gameplay classes such as game mode, game instance, player controller, and subsystem coordination. |
| `Source/InterrogationPrototype/Public/Characters` and `Private/Characters` | Player and suspect actor classes, animation hooks, gaze control, and interaction components. |
| `Source/InterrogationPrototype/Public/Dialogue` and `Private/Dialogue` | Conversation turns, prompt assembly, dialogue history, response parsing, and transcript management. |
| `Source/InterrogationPrototype/Public/AI` and `Private/AI` | Suspect decision logic, emotional-state evaluation, behavior rules, and response conditioning. |
| `Source/InterrogationPrototype/Public/Audio` and `Private/Audio` | Microphone capture, speech-to-text integration, text-to-speech playback, and audio routing. |
| `Source/InterrogationPrototype/Public/Investigation` and `Private/Investigation` | Case facts, evidence records, contradiction flags, and knowledge the suspect may reveal or hide. |
| `Source/InterrogationPrototype/Public/UI` and `Private/UI` | Debug panels, subtitles, transcript display, state visualization, and interaction prompts. |
| `Source/InterrogationPrototype/Tests` | Automation tests for deterministic systems such as emotional-state updates and dialogue memory. |

## Content Folders

| Folder | Responsibility |
| --- | --- |
| `Content/Maps` | The interrogation room map and any test maps. |
| `Content/Blueprints/Characters` | Blueprint children for the suspect and player-facing character logic. |
| `Content/Blueprints/Controllers` | Player controller and input mapping Blueprints if needed. |
| `Content/Blueprints/UI` | Widget Blueprints for transcript, subtitles, recording state, and debug state display. |
| `Content/AI/Suspects` | Data assets defining suspect persona, known facts, secrets, initial emotional state, and response style. |
| `Content/Audio/Dialogue` | Temporary or generated dialogue audio used during local testing. |
| `Content/Materials` | Room, prop, and character materials. |
| `Content/Props/InterrogationRoom` | Meshes and Blueprint props for table, chairs, lamp, recorder, door, mirror, and evidence objects. |
| `Content/UI` | Textures, fonts, and UI styling assets. |

## Planned Runtime Flow

1. Player holds the talk control and asks a question.
2. Audio capture sends speech to a speech-to-text adapter.
3. The recognized question is appended to the dialogue memory.
4. Emotional-state logic updates trust, stress, and suspicion.
5. The dialogue service builds an LLM prompt using persona, case facts, emotional state, and memory.
6. The LLM returns a suspect response plus optional structured metadata.
7. The response is added to memory, shown as subtitle text, and sent to text-to-speech.
8. The suspect speaks the generated audio in the interrogation room.

## Hidden Emotional State

The initial prototype state should be simple and testable:

| State | Meaning | Example Influence |
| --- | --- | --- |
| Trust | How safe the suspect feels with the detective. | Higher trust makes answers more cooperative and personal. |
| Stress | How pressured or overwhelmed the suspect feels. | Higher stress can make answers shorter, evasive, or inconsistent. |
| Suspicion | How much the suspect thinks the detective is manipulating or cornering them. | Higher suspicion can make the suspect defensive or hostile. |

Each value should start as a bounded numeric value, likely `0-100`, with later tuning handled through data assets rather than hard-coded constants.

## Approval-Gated Feature Sequence

Feature 0 is the architecture scaffold.

Feature 1 is the interrogation room shell. It adds a procedural greybox room, table, two chairs, fixed camera viewpoints, a first-person player character, and basic movement controls.

Feature 2 adds voice recognition. The player holds `V`, speaks naturally, releases `V`, and the microphone recording is sent to a configurable speech-to-text endpoint.

Feature 3 connects the transcript to an LLM and returns spoken suspect responses. The conversation director sends recognized detective text to the LLM client, stores short-term dialogue memory, sends the suspect answer to text-to-speech, and plays returned WAV audio.

Next proposed features:

1. **AI suspect actor**: Add the suspect actor structure, basic animation placeholders, and interaction focus point.
2. **Hidden emotional state**: Implement deterministic trust, stress, and suspicion values with tests.
3. **Debug UI**: Add transcript, recording state, service errors, and hidden-state inspection for development builds.
4. **Interrogation polish pass**: Replace greybox assets, tune lighting, add subtitles, and improve room staging.

Work should stop after each feature until approval is given for the next one.
