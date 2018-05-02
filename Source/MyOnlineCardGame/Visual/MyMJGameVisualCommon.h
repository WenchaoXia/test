// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Utils/CardGameUtils/MyCardGameElems.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

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


USTRUCT()
struct FMyInRoomViewRoleEventStyleSettingsCpp
{
    GENERATED_USTRUCT_BODY()
public:

    FMyInRoomViewRoleEventStyleSettingsCpp()
    {
        reset();
    };

    inline void reset()
    {
         m_cWeaveChiWidget = m_cWeavePengWidget = m_cWeaveGangWidget = m_cWeaveBuWidget = m_cTingWidget = m_cHuWidget = NULL;
    };

    inline bool checkSettings() const
    {
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeaveChiWidget, TEXT("weaveChiWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeavePengWidget, TEXT("weavePengWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeaveGangWidget, TEXT("weaveGangWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeaveBuWidget, TEXT("weaveBuWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cTingWidget, TEXT("tingWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cHuWidget, TEXT("huWidget")))
        {
            return false;
        }
        return true;
    };


    inline TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> getWeaveWidgetByType(MyMJGameEventVisualTypeCpp type) const
    {
        if (type == MyMJGameEventVisualTypeCpp::WeaveChi) {
            return m_cWeaveChiWidget;
        }
        else if (type == MyMJGameEventVisualTypeCpp::WeavePeng) {
            return m_cWeavePengWidget;
        }
        else if (type == MyMJGameEventVisualTypeCpp::WeaveGang) {
            return m_cWeavePengWidget;
        }
        else if (type == MyMJGameEventVisualTypeCpp::WeaveBu) {
            return m_cWeavePengWidget;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave chi Widget"))
        TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeaveChiWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave peng Widget"))
        TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeavePengWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave gang Widget"))
        TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeaveGangWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave bu Widget"))
        TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeaveBuWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "ting Widget"))
        TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cTingWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "hu Widget"))
        TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cHuWidget;
};


USTRUCT()
struct FMyInRoomViewRoleIncidentStyleSettingsCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyInRoomViewRoleIncidentStyleSettingsCpp()
    {
        reset();
    };

    inline void reset()
    {

    };

    inline bool checkSettings() const
    {
        return true;
    };
};


USTRUCT()
struct FMyInRoomViewRoleStyleSettingsCpp
{
    GENERATED_USTRUCT_BODY()
public:

    FMyInRoomViewRoleStyleSettingsCpp()
    {

    };

    inline void reset()
    {
        m_cEvent.reset();
        m_cIncident.reset();
    };

    inline bool checkSettings() const
    {
        if (!m_cEvent.checkSettings()) {
            return false;
        }

        if (!m_cIncident.checkSettings()) {
            return false;
        }

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "event"))
        FMyInRoomViewRoleEventStyleSettingsCpp m_cEvent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "incident"))
        FMyInRoomViewRoleIncidentStyleSettingsCpp m_cIncident;
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

        m_cDefaultInRoomViewRoleStyle.reset();
    }

    //idx is positionInBoxWidget2D
    UPROPERTY(EditAnywhere, EditFixedSize, meta = (DisplayName = "borders"))
    TArray<FMyCardGameUIBorderCfgCpp> m_aBorders;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "default In Room View Role Style"))
        FMyInRoomViewRoleStyleSettingsCpp m_cDefaultInRoomViewRoleStyle;
};
