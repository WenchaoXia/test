// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/MyMJUtils.h"

#include "MyMJCommonDefines.generated.h"

#define MY_MJ_GAME_CORE_FULL_LOOP_TIME_MS 1000  //resolution is *ms plus parent thread kick
#define My_MJ_GAME_IO_DRAIN_LOOP_TIME_MS 100
#define MY_MJ_GAME_CORE_MIRROR_LOOP_TIME_MS 100

typedef class FMyMJGameCoreCpp FMyMJGameCoreCpp;
typedef class FMyMJGameAttenderCpp FMyMJGameAttenderCpp;
typedef struct FMyMJGameIOGroupCpp FMyMJGameIOGroupCpp;
typedef class FMyMJGamePusherIOComponentFullCpp FMyMJGamePusherIOComponentFullCpp;

typedef struct FMyMJDataStructCpp FMyMJDataStructCpp;
typedef struct FMyMJDataDeltaCpp FMyMJDataDeltaCpp;
typedef struct FMyMJGamePusherResultCpp FMyMJGamePusherResultCpp;

//Warn: code use its uint8 value, don't modify it unless checked carefully
UENUM(BlueprintType)
enum class MyMJGameErrorCodeCpp : uint8
{
    None = 0                     UMETA(DisplayName = "None"),
    NotHandled = 1               UMETA(DisplayName = "NotHandled"),
    HaveNoAuthority = 10         UMETA(DisplayName = "HaveNoAuthority"),
    GameRuleTypeNotEqual = 20    UMETA(DisplayName = "GameRuleTypeNotEqual"),
    pusherIdNotEqual = 100       UMETA(DisplayName = "pusherIdNotEqual"),
    choicesEmpty = 120           UMETA(DisplayName = "choicesEmpty"),
    choiceAlreadyMade = 121      UMETA(DisplayName = "choiceAlreadyMade"),
    choiceOutOfRange = 122       UMETA(DisplayName = "choiceOutOfRange"),
    choiceSubSelectInvalid = 123 UMETA(DisplayName = "choiceSubSelectInvalid")
};

UENUM(BlueprintType)
enum class MyMJGameCoreWorkModeCpp : uint8
{
    Invalid = 0  UMETA(DisplayName = "Invalid"),
    Full = 1     UMETA(DisplayName = "Full"), //Full Function mode
    Mirror = 2   UMETA(DisplayName = "Mirror"), //Mirror mode, doesn't produce any thing, just consume the pushers
                                                //MirrorAlone = 2      UMETA(DisplayName = "MirrorAlone")
};

UENUM(BlueprintType)
enum class MyMJGameRuleTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    GuoBiao = 1     UMETA(DisplayName = "GuoBiao"),
    LocalCS = 20    UMETA(DisplayName = "LocalCS")
};

//Warn: code use its uint8 value, don't modify it unless checked carefully
UENUM(BlueprintType)
enum class MyMJGameRoleTypeCpp : uint8
{
    Attender0 = 0 UMETA(DisplayName = "Attender0"),
    Attender1 = 1 UMETA(DisplayName = "Attender1"),
    Attender2 = 2 UMETA(DisplayName = "Attender2"),
    Attender3 = 3 UMETA(DisplayName = "Attender3"),
    SysKeeper = 4 UMETA(DisplayName = "SysKeeper"),
    Observer = 5  UMETA(DisplayName = "Observer"),
    Max = 6       UMETA(DisplayName = "Max")
};

UENUM(BlueprintType)
enum class MyMJGameStateCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    GameEnd = 2 UMETA(DisplayName = "GameEnd"), //real end, calc which should happen in game, should have been done

    CardsShuffled = 10 UMETA(DisplayName = "CardsShuffled"),
    CardsWaitingForDistribution = 11 UMETA(DisplayName = "CardsWaitingForDistribution"),
    CardsDistributedWaitingForBuHua = 12 UMETA(DisplayName = "CardsDistributedWaitingForBuHua"),
    JustStarted = 14 UMETA(DisplayName = "JustStarted"), //Just started game, zhuang attender need to take action
    JustHu = 20      UMETA(DisplayName = "JustHu"),

    WaitingForTakeCard = 100 UMETA(DisplayName = "WaitingForTakeCard"), //normal take card, not in gang path
    TakenCard = 105 UMETA(DisplayName = "TakenCard"), //normal take card, not in gang path
    GivenOutCard = 110 UMETA(DisplayName = "GivenOutCard"), //normal give out card, not in gang path

    WeavedNotGang = 130 UMETA(DisplayName = "WeavedNotGang"),
    WeavedGang = 140 UMETA(DisplayName = "WeavedGang"), //decided for gang
    WeavedGangQiangGangChecked = 141 UMETA(DisplayName = "WeavedGangQiangGangChecked"),
    WeavedGangTakenCards = 145 UMETA(DisplayName = "WeavedGangTakenCards"),
    WeavedGangGivenOutCards = 146 UMETA(DisplayName = "WeavedGangGivenOutCards"),

    CardsDistributedWaitingForLittleHuLocalCS = 150 UMETA(DisplayName = "CardsDistributedWaitingForLittleHuLocalCS"),

    WeavedGangSpBuZhangLocalCS = 152 UMETA(DisplayName = "WeavedGangSpBuZhangLocalCS"), //decided as buzhang, as GangYao code path we use weavedGang
    WeavedGangDicesThrownLocalCS = 153 UMETA(DisplayName = "WeavedGangDicesThrownLocalCS"),
};

UENUM()
enum class MyMJActionLoopStateCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    WaitingToGenAction = 1 UMETA(DisplayName = "WaitingToGenAction"),
    ActionGened = 2 UMETA(DisplayName = "ActionGened"),
    ActionCollected = 3 UMETA(DisplayName = "ActionCollected")
};


USTRUCT()
struct FMyMJGameCardPackCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardPackCfgCpp()
    {
        reset();
    };

    void reset()
    {
        m_bHaveWordCards = false;
        m_bHaveHuaCards = false;
        m_bHaveZhongCards = false;
    };

    UPROPERTY()
    bool m_bHaveWordCards;

    UPROPERTY()
    bool m_bHaveHuaCards;

    UPROPERTY()
    bool m_bHaveZhongCards;
};

USTRUCT()
struct FMyMJGameTrivialCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameTrivialCfgCpp()
    {
        reset();
    };

    void reset()
    {
        m_iGameAttenderNum = 4;
        m_iCardNumPerStackInUntakenSlot = 2;
        m_iStackNumKeptFromTail = 0;
        m_bGangAnFlipUpCards = false;
    };

    UPROPERTY()
    int32 m_iGameAttenderNum; //[2, 4]

    UPROPERTY()
    int32 m_iCardNumPerStackInUntakenSlot;

    UPROPERTY()
    int32 m_iStackNumKeptFromTail;

    UPROPERTY()
    bool m_bGangAnFlipUpCards;

};

//For simple, define sub rule type cfg here
USTRUCT()
struct FMyMJGameSubGuoBiaoCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameSubGuoBiaoCfgCpp()
    {
        reset();
    };

    void reset()
    {
        m_bReqMenQingForZuHeLong = false;
    };


    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Req MenQing for ZuHeLong"))
    bool m_bReqMenQingForZuHeLong;
};

USTRUCT()
struct FMyMJGameSubLocalCSCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameSubLocalCSCfgCpp()
    {
        reset();
    };

    void reset()
    {
        m_bHuBornAllowMultiple = false;
        m_bHuBornShowAllCards = false;
        m_mHuBornScoreAttrs.Reset();

        m_i7Dui258DuiReq = 0;

        m_bHuAllowMultiple = true;
        m_bHuAttednerAsZhuangScore = false;
        m_bResetAttenderPaoHuBanStateAfterWeave = false;

        m_iGangYaoCount = 2;
        m_bBuZhangFromHead = false;

        m_iZhaNiaoCount = 2;

        m_bAllowGangYaoAfterGangYao = false;
    };

    UPROPERTY()
    bool m_bHuBornAllowMultiple; //Whether it allow one attender have more than 1 born hu at start

    UPROPERTY()
    bool m_bHuBornShowAllCards;

    UPROPERTY()
    TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> m_mHuBornScoreAttrs;

    UPROPERTY()
    int32 m_i7Dui258DuiReq;

    UPROPERTY()
    bool m_bHuAllowMultiple;

    UPROPERTY()
    bool m_bHuAttednerAsZhuangScore;

    UPROPERTY()
    bool m_bResetAttenderPaoHuBanStateAfterWeave;


    UPROPERTY()
    int32 m_iGangYaoCount;

    UPROPERTY()
    bool m_bBuZhangFromHead;

    UPROPERTY()
    int32 m_iZhaNiaoCount;

    UPROPERTY()
    bool m_bAllowGangYaoAfterGangYao;


};

USTRUCT()
struct FMyMJGameCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCfgCpp()
    {
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
    };

    virtual ~FMyMJGameCfgCpp()
    {

    };

    void reset()
    {
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
        m_cCardPackCfg.reset();
        m_cTrivialCfg.reset();
        m_cHuCfg.reset();
        m_cSubGuoBiaoCfg.reset();
        m_cSubLocalCSCfg.reset();
    };

    UPROPERTY()
    MyMJGameRuleTypeCpp m_eRuleType;

    UPROPERTY()
    FMyMJGameCardPackCfgCpp m_cCardPackCfg;

    UPROPERTY()
    FMyMJGameTrivialCfgCpp m_cTrivialCfg;

    UPROPERTY()
    FMyMJHuCfgCpp m_cHuCfg;

    //sub cfg, all optional, determined by @m_eRuleType
    UPROPERTY()
    FMyMJGameSubGuoBiaoCfgCpp m_cSubGuoBiaoCfg;

    UPROPERTY()
    FMyMJGameSubLocalCSCfgCpp m_cSubLocalCSCfg;
};

USTRUCT(BlueprintType)
struct FMyMJGameRunDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameRunDataCpp()
    {
        reset();
    };

    void reset()
    {
        m_iIdxAttenderQuanFeng = 0;
        m_iIdxAttenderMenFeng = 0;
        m_bZhuangTrue = false;
    };

    UPROPERTY()
    int32 m_iIdxAttenderQuanFeng;

    UPROPERTY()
    int32 m_iIdxAttenderMenFeng; //Same As Zhuang

    UPROPERTY()
    bool m_bZhuangTrue;
};