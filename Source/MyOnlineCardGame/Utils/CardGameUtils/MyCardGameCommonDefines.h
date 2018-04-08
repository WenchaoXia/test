// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CommonUtils/MyCommonDefines.h"

//#include "MyCardGameCommonDefines.generated.h"

//value > it means valid
//Warning: if it is not zero, you need to check all code, since some API use 0 as default value and it is faster at runtime
#define MyCardGameValueUnknown MyValueUnknown

UENUM()
enum class MyCardGameBoxLikeElemFlipStateCpp : uint8
{
    Invalid = 0                    UMETA(DisplayName = "Invalid"),

    Down = 1                       UMETA(DisplayName = "Down"),
    Stand = 2                      UMETA(DisplayName = "Stand"),
    Up = 3                        UMETA(DisplayName = "Up")

};