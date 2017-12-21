// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/KismetStringLibrary.h"

#include "MyMJGameVisualCommon.generated.h"

UENUM(BlueprintType)
enum class MyMJGameDeskVisualElemTypeCpp : uint8
{
    Invalid = 0                        UMETA(DisplayName = "Invalid"),
    Card = 1                           UMETA(DisplayName = "Card"),
    Dice = 2                           UMETA(DisplayName = "Dice"),

};

UENUM()
enum class MyMJGameHorizontalAlignmentCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Left = 1     UMETA(DisplayName = "Left"),
    Mid = 2    UMETA(DisplayName = "Mid"),
    Right = 3    UMETA(DisplayName = "Right")
};

UENUM()
enum class MyMJGameVerticalAlignmentCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Bottom = 1     UMETA(DisplayName = "Bottom"),
    Mid = 2    UMETA(DisplayName = "Mid"),
    Top = 3    UMETA(DisplayName = "Top")
};


//model always and must facing x axis
USTRUCT(BlueprintType)
struct FMyMJGameActorModelInfoBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActorModelInfoBaseCpp()
    {
        m_cCenterPointRelativeLocation = FVector(0);
    };

    virtual ~FMyMJGameActorModelInfoBaseCpp()
    {

    };

    void reset()
    {
        m_cCenterPointRelativeLocation = FVector(0);
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
        FVector m_cCenterPointRelativeLocation;
};

USTRUCT(BlueprintType)
struct FMyMJGameActorModelInfoBoxCpp : public FMyMJGameActorModelInfoBaseCpp

{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActorModelInfoBoxCpp() : Super()
    {
        m_cBoxExtend = FVector(1);
    };

    void reset()
    {
        Super::reset();

        m_cBoxExtend = FVector(1);
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "box extend final"))
        FVector m_cBoxExtend;

};


USTRUCT(BlueprintType)
struct FMyMJGameActorVisualResultBaseCpp

{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameActorVisualResultBaseCpp()
    {
        reset();
    };

    void reset()
    {
        m_bVisible = false;
        m_cTransform = FTransform();
    };

    inline
    FString genDebugString() const
    {
        return FString::Printf(TEXT("[visible %d, trans %s]"), m_bVisible, *UKismetStringLibrary::Conv_TransformToString(m_cTransform));
    };

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor transform updated, id %d, trans %s."), idCard, *UKismetStringLibrary::Conv_TransformToString(pCardActor->GetActorTransform()));

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "visible"))
    bool m_bVisible;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "transform"))
    FTransform m_cTransform;

};

//Can change in runtime, and caller should invalidate cache after change
USTRUCT(BlueprintType)
struct FMyMJGameDeskVisualPointCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualPointCfgCpp() : m_cAreaBoxExtendFinal(0)
    {
        m_eRowAlignment = MyMJGameVerticalAlignmentCpp::Invalid;
        //m_iRowMaxNum = 0;
        m_eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Invalid;
        //m_iColInRowMaxNum = 0;

        m_fColInRowExtraMarginAbs = 0;
    };

    virtual ~FMyMJGameDeskVisualPointCfgCpp()
    {

    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point world transform"))
        FTransform m_cCenterPointWorldTransform;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "area box extend final"))
        FVector m_cAreaBoxExtendFinal;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row alignment"))
        MyMJGameVerticalAlignmentCpp m_eRowAlignment;

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row max num"))
    //int32 m_iRowMaxNum;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row alignment"))
        MyMJGameHorizontalAlignmentCpp m_eColInRowAlignment;

    //assumed card num, if exceed the position may be out of range but will not crash the program
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row max num"))
    //int32 m_iColInRowMaxNum;

    //must >= 0
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin abs"))
        float m_fColInRowExtraMarginAbs;

};
