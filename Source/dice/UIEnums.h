#pragma once

#include "CoreMinimal.h"
#include "UIEnums.generated.h"

UENUM()
enum class EAnimState : uint8
{
	NotStarted,
	Running,
	Finished
};
