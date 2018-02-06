// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/DataAsset.h"

#include "Utils/CardGameUtils/MyCardGameUtilsLibrary.h"

#include "MyMJGameRoomUI.h"
#include "MyMJGameVisualElems.h"

#include "MyMJGameVisualCfg.generated.h"


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
        m_cCommonActionShowPoint.reset();

        m_bAttenderPointOnScreenOverride = false;
        m_cAttenderPointOnScreenPercentOverride = FVector2D::ZeroVector;

    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "card show point"))
    FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp m_cCardShowPoint;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "action show point"))
    FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp m_cCommonActionShowPoint;

    //By default, attender point on screen is calculated at runtime by project desktop point on player screen. If enabled, it will use overrided value directly to helper combine umg
    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender point on screen override"))
    bool m_bAttenderPointOnScreenOverride;

    //it should be the pointer on border, like (0.5, 0), unit is percent
    UPROPERTY(EditAnywhere, meta = (EditCondition = "m_bAttenderPointOnScreenOverride", DisplayName = "attender point on screen percent override as"))
    FVector2D m_cAttenderPointOnScreenPercentOverride;

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

        m_fAttenderCameraFOV = 90;
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

    UPROPERTY(EditAnywhere, EditFixedSize, meta = (DisplayName = "Attender Areas"))
    TArray<FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp> m_aAttenderAreas;

    //the relative position to desk center, it will apply it's loc, then it's rotation according to attender idx, then desk center's transform, a typical value will be (-1250, 0, 1030), (0, -40, 0), (1, 1, 1) 
    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender camera relative transform"))
    FTransform m_cAttenderCameraRelativeTransform;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender camera FOV"))
    float m_fAttenderCameraFOV;
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
struct FMyMJGameGameStartVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    
};

USTRUCT()
struct FMyMJGameThrowDicesVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameDistCardsDoneVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
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
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aCardsFocusedSteps;

    //other cards, like the cards still moving but not the given out ones in give out action. If empty it will use the default moving action
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards Other steps"))
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aCardsOtherSteps;

    //if it > 0.01 and steps for other(or subclass's step for specified cards), a wait sequence will be inserted at head for those cards
    UPROPERTY(EditAnywhere, meta = (DisplayName = "delay time for cards unfocused"))
    float m_fDelayTimeForCardsUnfocused;

    //if it > 0.01, will override the total time set for this event, for cards unfocused
    UPROPERTY(EditAnywhere, meta = (DisplayName = "total time overrided for cards unfocused"))
    float m_fTotalTimeOverridedForCardsUnfocused;
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
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aOverridedCardsFocusedStepsForAttenderAsViewer;
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
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aCardsInsertedToHandSlotSteps;
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
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aDancingActor0Steps;
};

USTRUCT()
struct FMyMJGameEventHuVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameGameEndVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventLocalCSHuBornVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventLocalCSZhaNiaoVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
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

    //when resync unexpected, the elems animation time. Note it would not occupy time segment on server, so this should be small as a pure client animation time 
    UPROPERTY(EditAnywhere, meta = (DisplayName = "resync unexpected in game"))
    FMyGameEventVisualDataBaseCpp m_cResyncUnexpectedIngame;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "resync normal at start"))
    FMyGameEventVisualDataBaseCpp m_cResyncNormalAtStart;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "game start"))
    FMyMJGameGameStartVisualDataCpp m_cGameStart;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "throw dices"))
    FMyMJGameThrowDicesVisualDataCpp m_cThrowDices;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dist cards done"))
    FMyMJGameDistCardsDoneVisualDataCpp m_cDistCardsDone;

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
    FMyMJGameEventHuVisualDataCpp m_cHu;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "game end"))
    FMyMJGameGameEndVisualDataCpp m_cGameEnd;

    UPROPERTY(EditAnywhere, Category = "local cs", meta = (DisplayName = "local CS hu born"))
    FMyMJGameEventLocalCSHuBornVisualDataCpp  m_cLocalCSHuBorn;

    UPROPERTY(EditAnywhere, Category = "local cs", meta = (DisplayName = "local CS zha niao"))
    FMyMJGameEventLocalCSZhaNiaoVisualDataCpp m_cLocalCSZhaNiao;
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
    TSubclassOf<AMyMJGameCardBaseCpp> m_cCardClass;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "in room ui main widget class"))
    TSubclassOf<UMyMJGameInRoomUIMainWidgetBaseCpp> m_cInRoomUIMainWidgetClass;
};


UCLASS()
class MYONLINECARDGAME_API UMyMJGameInRoomVisualCfgType : public UDataAsset
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

    //If set to 10, all data will reset to default
    UPROPERTY(EditAnywhere, meta = (DisplayName = "fake reset efault"))
    int32 m_iResetDefault;
};
