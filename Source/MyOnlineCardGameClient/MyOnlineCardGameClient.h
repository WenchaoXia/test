// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CoreUObject.h"

#include "MyOnlineCardGameClient.generated.h"


USTRUCT()
struct FMyOnlineCardGameClientUnusedStruct
{
    GENERATED_USTRUCT_BODY()

        UPROPERTY()
        int32 m_iId; // >= 0 means valid
};


/*
#include "MyOnlineCardGame.generated.h"

//Just for the bug that UBT sometime require generated file of this
USTRUCT()
struct FMyTrashCpp
{
    GENERATED_USTRUCT_BODY()
};
*/