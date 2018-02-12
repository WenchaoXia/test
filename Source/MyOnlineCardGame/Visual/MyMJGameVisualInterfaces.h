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
    //GENERATED_BODY()
    GENERATED_UINTERFACE_BODY()
};

class IMyMJGameInRoomUIMainInterface
{
    //GENERATED_BODY()
    GENERATED_IINTERFACE_BODY()

public:

    //return error code, 0 means OK
    UFUNCTION(BlueprintImplementableEvent)
    int32 showWeave();

    UFUNCTION(BlueprintImplementableEvent)
    int32 changeViewPosition(int32 idxAttender);

    //virtual void changeViewPosition_Implementation(int32 idxAttender)
    //{
    //
    //};

protected:

};

UINTERFACE()
class UMyPawnUIInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IMyPawnUIInterface
{
    GENERATED_IINTERFACE_BODY()

public:

    virtual void OnPossessedByLocalPlayerController(APlayerController* newController) = NULL;
    virtual void OnUnPossessedByLocalPlayerController(APlayerController* oldController) = NULL;

protected:

};