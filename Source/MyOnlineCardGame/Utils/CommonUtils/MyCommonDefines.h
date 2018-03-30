// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyCommonDefines.generated.h"

//Some code have relationship with the implemention in UE4's source, when upgrade, search code with "UE4 upgrade" and check

UENUM(BlueprintType)
enum class MyErrorCodeCommonPartCpp : uint8
{
    NoError = 0                     UMETA(DisplayName = "NoError"),

    //common
    NotProcessed = 10                                     UMETA(DisplayName = "NotProcessed"),

    //Role and Authority
    HaveNoAuthority = 40                                  UMETA(DisplayName = "HaveNoAuthority"),

    //Setting and asset
    PropertyIncorrect = 60                                UMETA(DisplayName = "PropertyIncorrect"),
    AssetLoadFail = 61                                    UMETA(DisplayName = "AssetLoadFail"),
    ParamInvalid = 62                                     UMETA(DisplayName = "ParamInvalid"),

    //Comon object and C++
    ObjectNull = 81                                       UMETA(DisplayName = "ObjectNull"),
    ObjectCastFail = 82                                   UMETA(DisplayName = "ObjectCastFail"), //fail since RTTI mismatch, for example, UObject have RTTI

    //Uobject and Actor
    RuntimeCDONotPrepared = 100                           UMETA(DisplayName = "RuntimeCDONotPrepared"),
    UObjectNotExist = 101                                 UMETA(DisplayName = "UObjectNotExist"),
    UComponentNotExist = 120                              UMETA(DisplayName = "UComponentNotExist"),
    AActorNotExist = 140                                  UMETA(DisplayName = "AActorNotExist"), //not specified or configed

    //Render: Mesh, material, etc
    ModelSizeIncorrect = 160                              UMETA(DisplayName = "ModelSizeIncorrect"), //model can be box and sphere for mesh, or FVector for material and texture

    //interfaces
    InterfaceFunctionNotImplementedByChildClass = 200                UMETA(DisplayName = "InterfaceFunctionNotImplementedByChildClass"),
    InterfaceFunctionNotImplementedByBlueprint = 201                 UMETA(DisplayName = "InterfaceFunctionNotImplementedByBlueprint"),
    InterfaceFunctionNotImplementedOnPurPose = 202                   UMETA(DisplayName = "InterfaceFunctionNotImplementedOnPurPose"), //caller should not call it now, not implemented it yet

};

//always remember first error
#define MyErrorCodePartJoin(SourcePart, NewPart) \
if ((int32)SourcePart == 0) { \
    SourcePart = (NewPart); \
}

//following code combine different errorcode parts together
//Warning: errocode can't exceed MyErroCodeMax
#define MyErrorCodePartMaxValue (10000) //define to make it human readable which help debug

#define MyErrorCodeGetCommonPart(MyErrorCode)                   (MyErrorCode % MyErrorCodePartMaxValue)
#define MyErrorCodeInternalGenerateCommonPartValue(commonPart)  ((int32)commonPart % MyErrorCodePartMaxValue)

#define MyErrorCodeGetSubPart(MyErrorCode)                      ((MyErrorCode / MyErrorCodePartMaxValue) % MyErrorCodePartMaxValue)
#define MyErrorCodeInternalGenerateSubPartValue(subPart)        (((int32)subPart % MyErrorCodePartMaxValue) * MyErrorCodePartMaxValue)

#define MyErrorCodeBuild(commonPart, subPart) (MyErrorCodeInternalGenerateCommonPartValue(commonPart) + MyErrorCodeInternalGenerateSubPartValue(subPart))
#define MyErrorCodeAddCommonPart(MyErrorCode, commonPart) MyErrorCodeBuild(commonPart, MyErrorCodeGetSubPart(MyErrorCode))
#define MyErrorCodeAddSubPart(MyErrorCode, subPart)       MyErrorCodeBuild(MyErrorCodeGetCommonPart(MyErrorCode), subPart)