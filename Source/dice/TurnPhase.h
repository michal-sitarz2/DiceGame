
#pragma once

#include "TurnPhase.generated.h"

UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
    Waiting UMETA(DisplayName = "Waiting"),
    Rolling UMETA(DisplayName = "Rolling"),
    Playing UMETA(DisplayName = "Playing"),
    Finished UMETA(DisplayName = "Finished")
};