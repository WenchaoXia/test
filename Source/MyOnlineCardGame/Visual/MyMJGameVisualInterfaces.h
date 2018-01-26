// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "MyMJGameVisualInterfaces.generated.h"


UINTERFACE(Blueprintable)
class UMyMJGameInRoomUIMainInterface : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomUIMainInterface
{
    GENERATED_BODY()

public:

    //return error code, 0 means OK
    UFUNCTION(BlueprintImplementableEvent, meta = (CallableWithoutWorldContext))
    int32 showWeave();

protected:

};