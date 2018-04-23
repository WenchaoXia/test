// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Utils/CardGameUtils/MyCardGameElems.h"

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
    FString ToString() const
    {
        return FString::Printf(TEXT("[visible %d, trans %s]"), m_bVisible, *UKismetStringLibrary::Conv_TransformToString(m_cTransform));
    };

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor transform updated, id %d, trans %s."), idCard, *UKismetStringLibrary::Conv_TransformToString(pCardActor->GetActorTransform()));

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "visible"))
    bool m_bVisible;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "transform"))
    FTransform m_cTransform;

};



USTRUCT()
struct FMyCardGameUIBorderCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyCardGameUIBorderCfgCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_cAreaAlignToBorder.reset();
        m_cCommonEventShowPoint.reset();

        m_cCommonEventShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.2;
    };


    UPROPERTY(EditAnywhere, meta = (DisplayName = "area align to border"))
    FMyAreaAlignToBorderCfgOneDimCpp m_cAreaAlignToBorder;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "common event show point"))
    FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp m_cCommonEventShowPoint;
};

USTRUCT(BlueprintType)
struct FMyMJGameInRoomUIMainWidgetCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameInRoomUIMainWidgetCfgCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_aBorders.Reset();
        m_aBorders.AddDefaulted(4);
    }

    //idx is positionInBoxWidget2D
    UPROPERTY(EditAnywhere, EditFixedSize, meta = (DisplayName = "borders"))
    TArray<FMyCardGameUIBorderCfgCpp> m_aBorders;
};
