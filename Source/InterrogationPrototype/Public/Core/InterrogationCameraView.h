#pragma once

#include "CoreMinimal.h"
#include "InterrogationCameraView.generated.h"

UENUM(BlueprintType)
enum class EInterrogationCameraView : uint8
{
	WideRoom UMETA(DisplayName = "Wide Room"),
	DetectiveSeat UMETA(DisplayName = "Detective Seat"),
	SuspectCloseup UMETA(DisplayName = "Suspect Closeup"),
	EvidenceTable UMETA(DisplayName = "Evidence Table")
};
