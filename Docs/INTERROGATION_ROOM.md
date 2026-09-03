# Interrogation Room Shell

This feature created the physical room shell and player/camera foundation. Voice recognition, LLM calls, and spoken responses now live in separate components documented in `Docs/VOICE_LLM_PIPELINE.md`.

## What Exists Now

| Component | Path | Role |
| --- | --- | --- |
| Interrogation game mode | `Source/InterrogationPrototype/Public/Core/InterrogationGameModeBase.h` | Owns the room-only startup flow. It selects the player character and controller, places the player at a fixed starting position, and spawns the room shell if the map does not already contain one. |
| Room builder | `Source/InterrogationPrototype/Public/Investigation/InterrogationRoomBuilder.h` | Builds the interrogation room from scaled cube meshes at runtime. This gives us a greybox room without needing a hand-authored binary map asset yet. |
| Player character | `Source/InterrogationPrototype/Public/Characters/InterrogationPlayerCharacter.h` | Provides first-person walking and mouse look. Movement is deliberately slow and grounded to fit an interrogation room instead of an action game. |
| Player controller | `Source/InterrogationPrototype/Public/Core/InterrogationPlayerController.h` | Owns camera switching. It can move the view from the player camera to fixed room cameras and back. |
| Camera rig | `Source/InterrogationPrototype/Public/Core/InterrogationCameraRig.h` | Defines the fixed room viewpoints used for cinematic inspection and later interrogation staging. |
| Camera view enum | `Source/InterrogationPrototype/Public/Core/InterrogationCameraView.h` | Keeps the available fixed camera views named and type-safe. |
| Input config | `Config/DefaultInput.ini` | Binds WASD movement, mouse look, `C` for cycling room cameras, and `R` for returning to the player camera. |
| Game mode config | `Config/DefaultEngine.ini` | Sets the default game mode to the interrogation shell game mode. |

## Room Layout

The current room is a procedural greybox using Unreal's built-in cube mesh:

| Object | Description |
| --- | --- |
| Floor and ceiling | A closed room volume sized for a compact interrogation space. |
| Four walls | Blocking static meshes so the player remains inside the test room. |
| Table | One central interrogation table with a tabletop and four legs. |
| Detective chair | A simple cube-built chair on the player's side of the table. |
| Suspect chair | A matching chair opposite the detective chair. |
| Observation mirror | A wall-mounted accent object for interrogation-room staging. |
| Door panel | A simple wall accent marking the room entrance. |
| Wall camera mount | A placeholder prop suggesting recorded surveillance. |
| Interrogation lamp | A placeholder table lamp volume, not a real light source yet. |

## Camera System

The camera rig currently has four fixed views:

| View | Purpose |
| --- | --- |
| Wide Room | Shows the whole interrogation space for layout inspection. |
| Detective Seat | Frames the detective side of the table and future question position. |
| Suspect Closeup | Frames where the suspect will sit once the suspect actor exists. |
| Evidence Table | Looks down at the table for future evidence and transcript staging. |

Controls:

- `C`: Cycle through fixed room cameras.
- `R`: Return to the first-person player camera.
- `V`: Hold to ask a spoken question. Release to submit the recording.

## Movement

Controls:

- `W`: Move forward.
- `S`: Move backward.
- `A`: Strafe left.
- `D`: Strafe right.
- Mouse X: Turn.
- Mouse Y: Look up and down.

The player starts behind the detective-side chair and faces the suspect side of the table. This keeps the first playable moment aligned with the interrogation fantasy without adding any interrogation mechanics yet.

## Why The Room Is Procedural

Unreal maps are binary `.umap` assets that are normally authored or saved by the Unreal Editor. Since this step is being created from code, the room is implemented as an actor that can spawn into any empty test map. Later, once the layout feels right, the room can be converted into an authored level or Blueprint prefab.

## What This Room Feature Does Not Include

- No suspect actor.
- No visible character performance or animation.
- No emotional state.
- No interrogation UI.
