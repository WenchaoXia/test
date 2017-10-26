// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/MyMJUtils.h"

#include "MyMJCommonDefines.generated.h"

#define MY_MJ_GAME_CORE_FULL_SUB_THREAD_LOOP_TIME_MS 1000  //resolution is *ms plus parent thread kick
#define MY_MJ_GAME_CORE_MIRROR_TO_CORE_FULL_LOOP_TIME_MS 50
#define MY_MJ_GAME_CORE_MIRROR_FOR_VISUAL_LOOP_TIME_MS 50

#define MyMJGameDup8BitMaskForSingleAttenderTo32BitMaskForAll(iMaskSingle) ( ((iMaskSingle & 0xff) << 24) | ((iMaskSingle & 0xff) << 16) | ((iMaskSingle & 0xff) << 8) | ((iMaskSingle & 0xff) << 0) )

typedef class FMyMJGameCoreCpp FMyMJGameCoreCpp;
typedef class FMyMJGameAttenderCpp FMyMJGameAttenderCpp;
typedef struct FMyMJGameIOGroupCpp FMyMJGameIOGroupCpp;
typedef class FMyMJGamePusherIOComponentFullCpp FMyMJGamePusherIOComponentFullCpp;

typedef struct FMyMJDataStructCpp FMyMJDataStructCpp;
typedef struct FMyMJDataDeltaCpp FMyMJDataDeltaCpp;
typedef struct FMyMJGamePusherResultCpp FMyMJGamePusherResultCpp;

//Warn: code use its uint8 value, don't modify it unless checked carefully
UENUM()
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

UENUM()
enum class MyMJGameElemWorkModeCpp : uint8
{
    Invalid = 0  UMETA(DisplayName = "Invalid"),
    Full = 1     UMETA(DisplayName = "Full"), //Full Function mode
    Mirror = 2   UMETA(DisplayName = "Mirror"), //Mirror mode, doesn't produce any thing, just consume the pushers
    Temp = 3     UMETA(DisplayName = "Temp"),
};

UENUM()
enum class MyMJGameRuleTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    GuoBiao = 1     UMETA(DisplayName = "GuoBiao"),
    LocalCS = 20    UMETA(DisplayName = "LocalCS")
};

//Warn: code use its uint8 value, don't modify it unless checked carefully
UENUM()
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

UENUM()
enum class MyMJGameStateCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    GameReseted = 1 UMETA(DisplayName = "GameReseted"), //just started
    GameStarted = 2 UMETA(DisplayName = "GameStarted"), //just started
    GameEnd = 3 UMETA(DisplayName = "GameEnd"), //real end, calc which should happen in game, should have been done

    CardsWaitingForThrowDicesToDistributeCards = 10 UMETA(DisplayName = "CardsWaitingForThrowDicesToDistributeCards"),
    CardsWaitingForDistribution = 11 UMETA(DisplayName = "CardsWaitingForDistribution"),
    CardsDistributed = 12 UMETA(DisplayName = "CardsDistributed"),

    JustStarted = 15 UMETA(DisplayName = "JustStarted"), //Just started game in nornal process after card dist and special hu, zhuang attender need to take action, Note: for simple subclass core must use action update state to enter just started state
    JustHu = 20      UMETA(DisplayName = "JustHu"),

    WaitingForTakeCard = 100 UMETA(DisplayName = "WaitingForTakeCard"), //normal take card, not in gang path
    TakenCard = 105 UMETA(DisplayName = "TakenCard"), //normal take card, not in gang path
    GivenOutCard = 110 UMETA(DisplayName = "GivenOutCard"), //normal give out card, not in gang path

    WeavedNotGang = 130 UMETA(DisplayName = "WeavedNotGang"),
    WeavedGang = 140 UMETA(DisplayName = "WeavedGang"), //decided for gang
    WeavedGangQiangGangChecked = 141 UMETA(DisplayName = "WeavedGangQiangGangChecked"),
    WeavedGangTakenCards = 145 UMETA(DisplayName = "WeavedGangTakenCards"),
    WeavedGangGivenOutCards = 146 UMETA(DisplayName = "WeavedGangGivenOutCards"),


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


USTRUCT(BlueprintInternalUseOnly)
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

    inline
    int32 getSupposedCardNum() const
    {
        int32 ret = 9 * 3 * 4;

        if (m_bHaveWordCards) {
            ret += 7 * 4;
        }

        if (m_bHaveHuaCards) {
            ret += 8;
        }

        if (m_bHaveZhongCards && !m_bHaveWordCards) {
            ret += 1 * 4;
        }

        return ret;
    };
            
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "have word cards"))
    bool m_bHaveWordCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "have hua cards"))
    bool m_bHaveHuaCards;

    //some rule only have zhong, but now word cards
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "have zhong cards"))
    bool m_bHaveZhongCards;
};

USTRUCT(BlueprintInternalUseOnly)
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "attender num"))
    int32 m_iGameAttenderNum; //[2, 4]

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card num per stack in untaken slot"))
    int32 m_iCardNumPerStackInUntakenSlot;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "stack num kept from untaken slot tail"))
    int32 m_iStackNumKeptFromTail;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "gang an flip up cards"))
    bool m_bGangAnFlipUpCards;

};

//For simple, define sub rule type cfg here
USTRUCT(BlueprintInternalUseOnly)
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


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Req MenQing for ZuHeLong"))
    bool m_bReqMenQingForZuHeLong;
};

USTRUCT(BlueprintInternalUseOnly)
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
        m_aHuBornScoreAttrs.Reset();

        m_i7Dui258DuiReq = 0;

        m_bHuAllowMultiple = true;
        m_bHuAttenderAsZhuangForScore = false;
        m_bResetAttenderPaoHuBanStateAfterWeave = false;

        m_iGangYaoCount = 2;
        m_bBuZhangFromHead = false;

        m_iZhaNiaoCount = 2;

        m_bAllowGangYaoAfterGangYao = false;

        m_mHuBornScoreAttrs.Reset();
    };

    inline
    void prepareForUse()
    {
        UMyMJUtilsLibrary::array2MapForHuScoreAttr(m_aHuBornScoreAttrs, m_mHuBornScoreAttrs);
    };

    inline
    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp>& getHuBornScoreAttrsRefConst() const
    {
        MY_VERIFY(UMyMJUtilsLibrary::checkUniformOfArrayAndMapForHuScoreAttr(m_aHuBornScoreAttrs, m_mHuBornScoreAttrs, false));
        return m_mHuBornScoreAttrs;
    };

    TArray<FMyMJHuScoreAttrCpp>& getHuBornScoreAttrsRef()
    {
        return m_aHuBornScoreAttrs;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hu born allow multiple"))
    bool m_bHuBornAllowMultiple; //Whether it allow one attender have more than 1 born hu at start

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hu born show all cards"))
    bool m_bHuBornShowAllCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu 7 DUi 258 Dui Required"))
    int32 m_i7Dui258DuiReq;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hu allow multiple"))
    bool m_bHuAllowMultiple;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hu attender as zhuang for score"))
    bool m_bHuAttenderAsZhuangForScore;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "reset attender pao hu ban state after weave"))
    bool m_bResetAttenderPaoHuBanStateAfterWeave;


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "gang yao count"))
    int32 m_iGangYaoCount;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "bu zhang from head"))
    bool m_bBuZhangFromHead;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "zha niao count"))
    int32 m_iZhaNiaoCount;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "allow gang yao after gang yao"))
    bool m_bAllowGangYaoAfterGangYao;

protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hu born score attrs"))
    TArray<FMyMJHuScoreAttrCpp> m_aHuBornScoreAttrs;

    TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> m_mHuBornScoreAttrs;
};

USTRUCT(BlueprintType)
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
        //m_cSubGuoBiaoCfg.reset();
        //m_cSubLocalCSCfg.reset();
        m_aSubGuoBiaoCfg.Reset();
        m_aSubLocalCSCfg.Reset();
    };

    inline
    void prepareForUse()
    {
        m_cHuCfg.prepareForUse();
        int32 l = m_aSubLocalCSCfg.Num();
        for (int32 i = 0; i < l; i++) {
            m_aSubLocalCSCfg[i].prepareForUse();
        }
    };

    inline const FMyMJGameSubGuoBiaoCfgCpp& getSubGuoBiaoCfgRefConst() const
    {
        MY_VERIFY(m_aSubGuoBiaoCfg.Num() == 1);
        return m_aSubGuoBiaoCfg[0];
    };

    inline const FMyMJGameSubLocalCSCfgCpp& getSubLocalCSCfgRefConst() const
    {
        MY_VERIFY(m_aSubLocalCSCfg.Num() == 1);
        return m_aSubLocalCSCfg[0];
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "rule type"))
    MyMJGameRuleTypeCpp m_eRuleType;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card pack cfg"))
    FMyMJGameCardPackCfgCpp m_cCardPackCfg;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "trivial cfg"))
    FMyMJGameTrivialCfgCpp m_cTrivialCfg;

    UPROPERTY(meta = (DisplayName = "hu cfg"))
    FMyMJHuCfgCpp m_cHuCfg;

    //sub cfg, all optional, determined by @m_eRuleType
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "sub guo biao cfg"))
    TArray<FMyMJGameSubGuoBiaoCfgCpp> m_aSubGuoBiaoCfg;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "sub local CS cfg"))
    TArray<FMyMJGameSubLocalCSCfgCpp> m_aSubLocalCSCfg;
};

USTRUCT(BlueprintInternalUseOnly)
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx attender quan feng"))
    int32 m_iIdxAttenderQuanFeng;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx attender men feng"))
    int32 m_iIdxAttenderMenFeng; //Same As Zhuang

    //if true, use as socre calc, else only used in game logic such as card distribution sequence
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "zhuang true"))
    bool m_bZhuangTrue;
};