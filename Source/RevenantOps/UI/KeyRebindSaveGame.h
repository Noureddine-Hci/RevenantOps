// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "InputCoreTypes.h"
#include "KeyRebindSaveGame.generated.h"

UCLASS()
class REVENANTOPS_API UKeyRebindSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    /** MappingName -> touche remappée */
    UPROPERTY() TMap<FName, FKey> MappedKeys;

    static const FString SlotName;
    static const int32   UserIndex;
};
