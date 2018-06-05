// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyCommonDefines.generated.h"

#define UE_MY_LOG(CategoryName, Verbosity, Format, ...) \
        UE_LOG(CategoryName, Verbosity, TEXT("%s:%d: ") Format, *myGetFileNameFromFullPath(TEXT(__FILE__)), __LINE__, ##__VA_ARGS__)

//Fatal cause log not written to disk but core dump, so we don't use Fatal anywhere
#define MY_VERIFY(cond) \
       if (!(cond)) { UE_MY_LOG(LogMyUtilsI, Error, _TEXT("my verify false: (" #cond ")")); UE_MY_LOG(LogMyUtilsI, Fatal, _TEXT("core dump now")); verify(false); }

//     if (!(cond)) {UE_MY_LOG(LogMyUtilsI, Error, _TEXT("my verify false")); UE_MY_LOG(LogMyUtilsI, Fatal, _TEXT("core dump now")); verify(false);}

//#define UE_MY_LOG_UOBJECT(CategoryName, Verbosity, Format, ...) \
        //UE_MY_LOG(CategoryName, Verbosity, TEXT("%s: ") Format, *GetName(), ##__VA_ARGS__)

//Some code have relationship with the implemention in UE4's source, when upgrade, search code with "UE4 upgrade" and check
#define MySelectionUnknown (-1)
#define MyIDUnknown (-1)
#define MyIDFake (-2)

#define MyValueUnknown (-1)

#define MyTypeUnknown (-1)

#define MyIdxInvalid (-1)

#define MyCommonMinDelta (0.0001f)
#define MyDeNominatorAsZeroTolerance MyCommonMinDelta


UENUM(BlueprintType)
enum class MyErrorCodeCommonPartCpp : uint8
{
    Invalid = 0                     UMETA(DisplayName = "Invalid"),
    NoError = 1                     UMETA(DisplayName = "NoError"),
    InternalError = 5               UMETA(DisplayName = "InternalError"),

    //common
    NotProcessed = 10                                     UMETA(DisplayName = "NotProcessed"),
    TypeUnexpected = 11                                   UMETA(DisplayName = "TypeUnexpected"),
    NotSupported = 12                                     UMETA(DisplayName = "NotSupported"),

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
    AControllerNotExist = 150                             UMETA(DisplayName = "AControllerNotExist"),
    AControllerTypeUnexpected = 151                       UMETA(DisplayName = "AControllerTypeUnexpected"),


    //Render: Mesh, material, etc
    ModelSizeIncorrect = 160                              UMETA(DisplayName = "ModelSizeIncorrect"), //model can be box and sphere for mesh, or FVector for material and texture
    ModelInfoIncorrect = 161                              UMETA(DisplayName = "ModelInfoIncorrect"),
    UIPanelSlotTypeUnexpected = 170                       UMETA(DisplayName = "UIPanelSlotTypeUnexpected"),
    UIPanelSlotDataUnexpected = 171                       UMETA(DisplayName = "UIPanelSlotDataUnexpected"),

    RenderDataUnexpected = 176                             UMETA(DisplayName = "RenderDataUnexpected"),

    //interfaces
    InterfaceFunctionNotImplementedByChildClass = 200     UMETA(DisplayName = "InterfaceFunctionNotImplementedByChildClass"),
    InterfaceFunctionNotImplementedByBlueprint = 201      UMETA(DisplayName = "InterfaceFunctionNotImplementedByBlueprint"),
    InterfaceFunctionNotImplementedOnPurpose = 202        UMETA(DisplayName = "InterfaceFunctionNotImplementedOnPurpose"), //caller should not call it now, not implemented it yet

    //interactions
    InputTimePassed = 220                                 UMETA(DisplayName = "InputTimePassed"),
};

//always remember first error
#define MyErrorCodeCommonPartJoin(SourcePart, NewPart) \
if (SourcePart == MyErrorCodeCommonPartCpp::NoError) { \
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


UENUM()
enum class MyAxisTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    X = 1           UMETA(DisplayName = "X"),
    Y = 2           UMETA(DisplayName = "Y"),
    Z = 3           UMETA(DisplayName = "Z")
};

UENUM()
enum class MyPlaneTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    XY = 1           UMETA(DisplayName = "XY"),
    YZ = 2           UMETA(DisplayName = "YZ"),
    XZ = 3           UMETA(DisplayName = "XZ")
};

UENUM()
enum class MyOrderTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    ASC = 1    UMETA(DisplayName = "ASC"),
    DESC = 2    UMETA(DisplayName = "DESC")
};

#define SetValidValue_MyOrderTypeCpp(dest, source) \
dest = source; \
if (dest == MyOrderTypeCpp::Invalid) { \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT(#dest" invalid, using default value now.")); \
    dest = MyOrderTypeCpp::ASC; \
}

UENUM()
enum class MyAxisAlignmentTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Negative = 1    UMETA(DisplayName = "Negative"),
    Mid = 2         UMETA(DisplayName = "Mid"),
    Positive = 3    UMETA(DisplayName = "Positive")
};

#define SetValidValue_MyAxisAlignmentTypeCpp(dest, source) \
dest = source; \
if (dest == MyAxisAlignmentTypeCpp::Invalid) { \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT(#dest" invalid, using default value now.")); \
    dest = MyAxisAlignmentTypeCpp::Negative; \
}

UENUM()
enum class MyContinuousAlignmentTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Prev = 1    UMETA(DisplayName = "Prev"),
    Mid = 2         UMETA(DisplayName = "Mid"),
    Next = 3    UMETA(DisplayName = "Next")
};

//same as pitch
UENUM()
enum class MyBoxFlipStateCpp : uint8
{
    Invalid = 0                    UMETA(DisplayName = "Invalid"),

    Down = 1                       UMETA(DisplayName = "Down"),
    Stand = 2                      UMETA(DisplayName = "Stand"),
    Up = 3                         UMETA(DisplayName = "Up")

};

//UE4's coordinate are X positve as front. for any rotate axis, look toward its positive way, rotate as clockwise for positive rotate value
UENUM()
enum class MyRotateState90DCpp : uint8
{
    Invalid = 0                    UMETA(DisplayName = "Invalid"),

    Zero = 1                       UMETA(DisplayName = "Zero"),
    Positive90D = 10               UMETA(DisplayName = "Positive90D"),
    Negative90D = 20               UMETA(DisplayName = "Negative90D")
};
