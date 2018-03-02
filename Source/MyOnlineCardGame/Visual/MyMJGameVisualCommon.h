// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Utils/CommonUtils/MyCommonUtilsLibrary.h"

#include "Kismet/KismetStringLibrary.h"

#include "MyMJGameVisualCommon.generated.h"

UENUM(BlueprintType)
enum class MyMJGameDeskVisualElemTypeCpp : uint8
{
    Invalid = 0                        UMETA(DisplayName = "Invalid"),
    Card = 1                           UMETA(DisplayName = "Card"),
    Dice = 2                           UMETA(DisplayName = "Dice"),
    Attender = 3                       UMETA(DisplayName = "Attender"),
};

UENUM(BlueprintType)
enum class MyMJGameDeskVisualElemAttenderSubtypeCpp : uint8
{
    Invalid = 0                        UMETA(DisplayName = "Invalid"),
    OnDeskLocation = 1                 UMETA(DisplayName = "OnDeskLocation"),
    Max = 2                            UMETA(DisplayName = "Max"),
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
        m_iRowMax = 0;
        m_iExtra0 = 0;
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

    //actually it contains two aspect: card sequence and actor alignment, left = card seq left + actor align left, for simple we use one property here. Warn: mid = card seq left + actor align mid, and only valid for hand and just taken slot now!
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row alignment"))
        MyMJGameHorizontalAlignmentCpp m_eColInRowAlignment;

    //assumed card num, if exceed the position may be out of range but will not crash the program
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row max num"))
    //int32 m_iColInRowMaxNum;

    //must >= 0
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin abs"))
        float m_fColInRowExtraMarginAbs;

    //if row number exceed this, the actor should stack. If it is 0, means no limit
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row max"))
    int32 m_iRowMax;

    //in case some unstandard cfg is needed, use this 
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "extra 0"))
    int32 m_iExtra0;
};



USTRUCT(BlueprintType)
struct FMyMJDiceModelInfoExtraCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJDiceModelInfoExtraCpp()
    {
        m_aLocalRotatorForDiceValues.Reset();
        for (int32 i = 0; i < 7; i++) {
            m_aLocalRotatorForDiceValues.Emplace(FRotator::ZeroRotator);
        }
    };

    //input is expected as in 1 to 6
    inline const FRotator& getLocalRotatorForDiceValueRefConst(int32 value) const
    {
        MY_VERIFY(value > 0 && value < 7);
        MY_VERIFY(value < m_aLocalRotatorForDiceValues.Num());
        return m_aLocalRotatorForDiceValues[value];
    };

protected:

    //the array's index means the value, and the array size is always 7
    UPROPERTY(BlueprintReadWrite, EditAnywhere, EditFixedSize, meta = (DisplayName = "local rotator for dice values"))
        TArray<FRotator> m_aLocalRotatorForDiceValues;
};

USTRUCT(BlueprintType)
struct FMyMJDiceModelInfoBoxCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJDiceModelInfoBoxCpp()
    {
    };

    inline void reset()
    {
        m_cBasic.reset();
    };

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "basic"))
    FMyActorModelInfoBoxCpp m_cBasic;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "extra"))
    FMyMJDiceModelInfoExtraCpp m_cExtra;

};