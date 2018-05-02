// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/DataAsset.h"

#include "Utils/CardGameUtils/MyCardGameUtilsLibrary.h"

#include "MyMJGameRoomUI.h"
#include "MyMJGameVisualElems.h"

#include "MyMJGameVisualCfg.generated.h"

/* affairs:
 * event: |--pusher(action is a subtype of pusher)
 *        |--trival
 *
 * incident:
 */

USTRUCT(BlueprintType)
struct FMyCardGameCameraStaticDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
    
    FMyCardGameCameraStaticDataCpp()
    {
        m_fMoveTime = 1;
        m_pMoveCurve = NULL;
    };

    inline void reset()
    {
        FTransform t;
        m_cCenterPoint = t;

        m_fMoveTime = 1;
        m_pMoveCurve = NULL;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point"))
    FTransform m_cCenterPoint;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "move time"))
    float m_fMoveTime;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "move curve"))
    UCurveVector* m_pMoveCurve;
};


USTRUCT(BlueprintType)
struct FMyCardGameCameraDynamicDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyCardGameCameraDynamicDataCpp()
    {
        m_fFOV = 90;
    };

    inline void reset()
    {
        m_cMyTransformOfZRotation.reset();
        m_fFOV = 90;
    };

    inline static void interp(const FMyCardGameCameraDynamicDataCpp& start, const FMyCardGameCameraDynamicDataCpp& end, float percent, FMyCardGameCameraDynamicDataCpp& result)
    {
        FMyTransformOfZRotationAroundPointCoordinateCpp::interp(start.m_cMyTransformOfZRotation, end.m_cMyTransformOfZRotation, percent, result.m_cMyTransformOfZRotation);
        
        float percentFixed = FMath::Clamp<float>(percent, 0, 1);
        result.m_fFOV = start.m_fFOV + (end.m_fFOV - start.m_fFOV) * percentFixed;
    };

    inline static bool equals(const FMyCardGameCameraDynamicDataCpp& a, const FMyCardGameCameraDynamicDataCpp& b)
    {
        if (!FMyTransformOfZRotationAroundPointCoordinateCpp::equals(a.m_cMyTransformOfZRotation, b.m_cMyTransformOfZRotation, FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min))
        {
            return false;
        }

        if (!FMath::IsNearlyEqual(a.m_fFOV, b.m_fFOV, FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min))
        {
            return false;
        }

        return true;
    };

    inline FString ToString() const
    {
        return m_cMyTransformOfZRotation.ToString() + FString::Printf(TEXT(", fov %f"), m_fFOV);
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "My Transform of Z Rotation Around Point"))
    FMyTransformOfZRotationAroundPointCoordinateCpp m_cMyTransformOfZRotation;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "field of view"))
    float m_fFOV;
};

USTRUCT(BlueprintType)
struct FMyCardGameCameraDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyCardGameCameraDataCpp()
    {

    };

    virtual ~FMyCardGameCameraDataCpp()
    {

    };

    inline void reset()
    {
        m_cStaticData.reset();
        m_cDynamicData.reset();
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "static data"))
    FMyCardGameCameraStaticDataCpp m_cStaticData;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "dynamic data"))
    FMyCardGameCameraDynamicDataCpp m_cDynamicData;
};

USTRUCT()
struct FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp()
    {
        reset();
    };

    virtual ~FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp()
    {

    };

    inline void reset()
    {
        m_cCardShowPoint.reset();
        m_cDiceShowPoint.reset();
        m_cCommonActionActorShowPoint.reset();

        m_bAttenderPointOnScreenOverride = false;
        m_cAttenderPointOnScreenPercentOverride = FVector2D::ZeroVector;

    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "card show point"))
    FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp m_cCardShowPoint;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dice show point"))
    FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp m_cDiceShowPoint;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "common action actor show point"))
    FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp m_cCommonActionActorShowPoint;

    //By default, attender point on screen is calculated at runtime by projected point from desktop area actor's attender point. If enabled, it will use overrided value directly to helper combine umg
    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender point on screen override"))
    bool m_bAttenderPointOnScreenOverride;

    //it should be the pointer on border, like (0.5, 0), unit is percent
    UPROPERTY(EditAnywhere, meta = (EditCondition = "m_bAttenderPointOnScreenOverride", DisplayName = "attender point on screen percent override as"))
    FVector2D m_cAttenderPointOnScreenPercentOverride;

};

USTRUCT()
struct FMyMJGameCameraCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameCameraCfgCpp()
    {
        reset();
    };

    inline void reset()
    {
        FTransform t;
        m_cAttenderCameraRelativeTransformAsAttender0 = t;
        m_fAttenderCameraFOV = 90;
        m_fAttenderCameraMoveTime = 1;
        m_cAttenderCameraMoveCurve.reset();
    };

    bool checkSettings() const;
    void fillDefaultData();

    //the relative position to desk center, it will apply it's loc, then it's rotation according to attender idx, then desk center's transform, a typical value will be (-1250, 0, 1030), (0, -40, 0), (1, 1, 1) 
    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender camera relative transform as attender 0"))
    FTransform m_cAttenderCameraRelativeTransformAsAttender0;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender camera FOV"))
    float m_fAttenderCameraFOV;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender camera move time"))
    float m_fAttenderCameraMoveTime;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender camera move curve"))
    FMyCurveVectorSettingsCpp m_cAttenderCameraMoveCurve;
};

USTRUCT()
struct FMyMJGameInGamePlayerScreenCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameInGamePlayerScreenCfgCpp()
    {
        m_aAttenderAreas.Reset();
        m_aAttenderAreas.AddDefaulted(4);
    };

    virtual ~FMyMJGameInGamePlayerScreenCfgCpp()
    {

    };

    inline void reset()
    {
        for (int32 i = 0; i < 4; i++) {
            m_aAttenderAreas[i].reset();
        }
    };

    bool checkSettings() const;

    //mainly for test
    void fillDefaultData();

    //actually it is based on view position on player screen, idx is 0-3, meaning is same with idxScreenPosition
    UPROPERTY(EditAnywhere, EditFixedSize, meta = (DisplayName = "Attender Areas"))
    TArray<FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp> m_aAttenderAreas;
};

//MyMJGameCoreRelatedEventMainTypeCpp

USTRUCT()
struct FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyGameEventVisualDataBaseCpp()
    {
        m_fTotalTime = 1.f;
    }

    //unit is second
    UPROPERTY(EditAnywhere, meta = (DisplayName = "total time"))
    float m_fTotalTime;
};

USTRUCT()
struct FMyGameClientCommonUpdateVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        m_cMoveCurve.reset();
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "move curve"))
    FMyCurveVectorSettingsCpp m_cMoveCurve;
};

USTRUCT()
struct FMyMJGameEventPusherFullBaseResetVisualDataBaseCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
    };
};

USTRUCT()
struct FMyMJGameEventPusherGameStartVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    
};

USTRUCT()
struct FMyMJGameEventPusherThrowDicesVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        m_fTotalTime = 1.f;
        m_aDiceSteps.Reset();
    }

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dice steps"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aDiceSteps;
};

USTRUCT()
struct FMyMJGameEventCardsRelatedVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventCardsRelatedVisualDataCpp() : Super()
    {
        reset();
    };

    inline void reset()
    {
        m_fTotalTime = 1.f;
        m_aCardsFocusedSteps.Reset();
        m_aCardsOtherSteps.Reset();
        m_fDelayTimeForCardsUnfocused = 0;
        m_fTotalTimeOverridedForCardsUnfocused = 0;
    };

    //the cards focused, like given out cards in give out action. If empty it will use the default moving action.
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards focused steps"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aCardsFocusedSteps;

    //other cards, like the cards still moving but not the given out ones in give out action. If empty it will use the default moving action
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards Other steps"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aCardsOtherSteps;

    //if it > 0.01 and steps for other(or subclass's step for specified cards), a wait sequence will be inserted at head for those cards
    UPROPERTY(EditAnywhere, meta = (DisplayName = "delay time for cards unfocused"))
    float m_fDelayTimeForCardsUnfocused;

    //if it > 0.01, will override the total time set for this event, for cards unfocused
    UPROPERTY(EditAnywhere, meta = (DisplayName = "total time overrided for cards unfocused"))
    float m_fTotalTimeOverridedForCardsUnfocused;
};

USTRUCT()
struct FMyMJGameEventPusherDistCardsVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventPusherDistCardsVisualDataCpp() : Super()
    {
        reset();
    };

    inline void reset()
    {
        Super::reset();

        m_fTotalTimeOverrideForLast = 1.f;
        m_aCardsFocusedStepsOverrideForLast.Reset();
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "total time override for last"))
    float m_fTotalTimeOverrideForLast;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards focused steps override for last"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aCardsFocusedStepsOverrideForLast;
};

USTRUCT()
struct FMyMJGameEventPusherTakeCardsVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventPusherTakeCardsVisualDataCpp() : Super()
    {
        m_bOverrideCardsFocusedStepsForAttenderAsViewer = false;
    };

    //whether act differently for current viewer, that is the attender near camera
    UPROPERTY(EditAnywhere, meta = (DisplayName = "override cards focused steps for attender as viewer"))
    bool m_bOverrideCardsFocusedStepsForAttenderAsViewer;

    //for cards belong to the attender near camera
    UPROPERTY(EditAnywhere, meta = (DisplayName = "overrided cards focused steps for attender as viewer", EditCondition = "m_bOverrideCardsFocusedStepsForAttenderAsViewer"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aOverridedCardsFocusedStepsForAttenderAsViewer;
};

USTRUCT()
struct FMyMJGameEventPusherGiveOutCardsVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        Super::reset();
        m_aCardsInsertedToHandSlotSteps.Reset();
    };

    //If empty it will use the default moving action
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards inserted to hand slot steps"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aCardsInsertedToHandSlotSteps;
};

USTRUCT()
struct FMyMJGameEventPusherWeaveVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        Super::reset();
        m_aDancingActor0Steps.Reset();
    }

    inline void copyExceptClass(const FMyMJGameEventPusherWeaveVisualDataCpp& other)
    {
        if (this != &other) {
            TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> oldClass = m_cDancingActor0Class;
            *this = other;
            m_cDancingActor0Class = oldClass;
        }
    };


    bool checkSettings(FString debugStr) const
    {
        const TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> &cClass = m_cDancingActor0Class;
        if (IsValid(cClass)) {
            if (cClass == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: class must be a child class of AMyMJGameTrivalDancingActorBaseCpp!"), *debugStr);
                return false;
            }
        }

        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class is %s."), *cClass->GetName());

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dancing actor0 class"))
    TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> m_cDancingActor0Class;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dancing actor0 steps"))
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> m_aDancingActor0Steps;
};

USTRUCT()
struct FMyMJGameEventPusherHuVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventPusherGameEndVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventPusherLocalCSHuBornVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventPusherLocalCSZhaNiaoVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};


USTRUCT()
struct FMyMJGameEventPusherCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventPusherCfgCpp()
    {
    };

    void fillDefaultData();

    bool checkSettings() const
    {
        if (!m_cWeaveChi.checkSettings(TEXT("event pusher cfg weave chi")))
        {
            return false;
        }

        if (!m_cWeavePeng.checkSettings(TEXT("event pusher cfg weave peng")))
        {
            return false;
        }

        if (!m_cWeaveGang.checkSettings(TEXT("event pusher cfg weave gang")))
        {
            return false;
        }

        if (!m_cWeaveBu.checkSettings(TEXT("event pusher cfg weave bu")))
        {
            return false;
        }

        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class is %s."), *cClass->GetName());

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "client common update"))
    FMyGameClientCommonUpdateVisualDataCpp m_cClientCommonUpdate;

    //unlike "resync unexpected in game", this event happen both on server and client
    UPROPERTY(EditAnywhere, meta = (DisplayName = "full base reset at start"))
    FMyMJGameEventPusherFullBaseResetVisualDataBaseCpp m_cFullBaseResetAtStart;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "game start"))
    FMyMJGameEventPusherGameStartVisualDataCpp m_cGameStart;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "throw dices"))
    FMyMJGameEventPusherThrowDicesVisualDataCpp m_cThrowDices;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dist cards"))
    FMyMJGameEventPusherDistCardsVisualDataCpp m_cDistCards;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "take cards"))
    FMyMJGameEventPusherTakeCardsVisualDataCpp m_cTakeCards;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "give out cards"))
    FMyMJGameEventPusherGiveOutCardsVisualDataCpp m_cGiveOutCards;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave chi"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeaveChi;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave peng"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeavePeng;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave gang"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeaveGang;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave bu"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeaveBu;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "hu"))
    FMyMJGameEventPusherHuVisualDataCpp m_cHu;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "game end"))
    FMyMJGameEventPusherGameEndVisualDataCpp m_cGameEnd;

    UPROPERTY(EditAnywhere, Category = "local cs", meta = (DisplayName = "local CS hu born"))
    FMyMJGameEventPusherLocalCSHuBornVisualDataCpp  m_cLocalCSHuBorn;

    UPROPERTY(EditAnywhere, Category = "local cs", meta = (DisplayName = "local CS zha niao"))
    FMyMJGameEventPusherLocalCSZhaNiaoVisualDataCpp m_cLocalCSZhaNiao;
};

USTRUCT()
struct FMyMJGameEventTrivalCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
 
    inline void fillDefaultData()
    {

    };

    inline bool checkSettings() const
    {
        return true;
    };
};

//event is some thing time critical, important thing can't be missed
USTRUCT()
struct FMyMJGameInRoomEventCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void fillDefaultData()
    {
        m_cPusherCfg.fillDefaultData();
        m_cTrivalCfg.fillDefaultData();
    };

    inline bool checkSettings() const
    {
        if (!m_cPusherCfg.checkSettings())
        {
            return false;
        }

        if (!m_cTrivalCfg.checkSettings())
        {
            return false;
        }

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "pusher"))
    FMyMJGameEventPusherCfgCpp m_cPusherCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "trival"))
    FMyMJGameEventTrivalCfgCpp m_cTrivalCfg;
};

 //incident is some thing no time critical, not important
USTRUCT()
struct FMyMJGameInRoomIncidentCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

};

USTRUCT()
struct FMyMJGameInRoomMainActorClassCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    bool checkSettings() const;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "card class"))
    TSubclassOf<AMyMJGameCardActorBaseCpp> m_cCardClass;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dice class"))
    TSubclassOf<AMyMJGameDiceBaseCpp> m_cDiceClass;
};

USTRUCT()
struct FMyMJGameUICfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameUICfgCpp()
    {
        m_cInRoomUIMainWidgetClass = NULL;
    };

    void fillDefaultData();

    inline bool checkSettings() const
    {

        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyMJGameInRoomUIMainWidgetBaseCpp>(m_cInRoomUIMainWidgetClass, TEXT("in room ui main widget")))
        {
            return false;
        }

        //if (!m_cDefaultInRoomViewRoleStyle.checkSettings()) {
            //return false;
        //}

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "in room ui main widget class"))
    TSubclassOf<UMyMJGameInRoomUIMainWidgetBaseCpp> m_cInRoomUIMainWidgetClass;
};

UCLASS()
class MYONLINECARDGAME_API UMyMJGameInRoomVisualCfgCpp : public UDataAsset
{
    GENERATED_BODY()

public:

    void fillDefaultData();

    bool checkSettings() const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    static void helperMapToSimplifiedTimeCfg(const FMyMJGameInRoomEventCfgCpp& eventCfg, FMyMJGameEventTimeCfgCpp& outSimplifiedTimeCfg);


    UPROPERTY(EditAnywhere, meta = (DisplayName = "Player Screen Cfg"))
    FMyMJGameInGamePlayerScreenCfgCpp m_cPlayerScreenCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Event Cfg"))
    FMyMJGameInRoomEventCfgCpp m_cEventCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Incident Cfg"))
    FMyMJGameInRoomIncidentCfgCpp m_cIncidentCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Main actor class Cfg"))
    FMyMJGameInRoomMainActorClassCfgCpp m_cMainActorClassCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "camera Cfg"))
    FMyMJGameCameraCfgCpp m_cCameraCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "UI Cfg"))
    FMyMJGameUICfgCpp m_cUICfg;

    //If set to 10, all data will reset to default
    UPROPERTY(EditAnywhere, meta = (DisplayName = "fake reset default"))
    int32 m_iResetDefault;
};
