#pragma once

#include "CoreMinimal.h"
#include "InterrogationDialogueTypes.generated.h"

USTRUCT(BlueprintType)
struct INTERROGATIONPROTOTYPE_API FInterrogationDialogueTurn
{
	GENERATED_BODY()

public:
	FInterrogationDialogueTurn()
		: Role(TEXT("user"))
	{
	}

	FInterrogationDialogueTurn(const FString& InRole, const FString& InText)
		: Role(InRole)
		, Text(InText)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Dialogue")
	FString Role;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interrogation|Dialogue")
	FString Text;
};
