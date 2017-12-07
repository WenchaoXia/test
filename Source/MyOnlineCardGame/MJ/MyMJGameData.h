// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyMJCommonDefines.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"

#include "MyMJGameData.generated.h"

//Todo: use UE4's generated delta instead of my own

USTRUCT(BlueprintType)
struct FMyMJGameUntakenSlotSubSegmentInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameUntakenSlotSubSegmentInfoCpp()
    {
        reset();
    };

    virtual ~FMyMJGameUntakenSlotSubSegmentInfoCpp()
    {

    };

    void reset()
    {
        m_iIdxStart = -1;
        m_iLength = 0;
    };

    UPROPERTY()
        int32 m_iIdxStart;

    UPROPERTY()
        int32 m_iLength;

};

USTRUCT(BlueprintType)
struct FMyMJGameUntakenSlotInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameUntakenSlotInfoCpp()
    {
        reset();
    };

    virtual ~FMyMJGameUntakenSlotInfoCpp()
    {

    };

    void reset()
    {
        m_iIdxUntakenSlotHeadNow = -1;
        m_iIdxUntakenSlotTailNow = -1;
        m_iIdxUntakenSlotTailAtStart = -1;
        m_iUntakenSlotLengthAtStart = -1;

        m_iUntakenSlotCardsLeftNumTotal = -1;
        m_iUntakenSlotCardsLeftNumKeptFromTail = -1;
        m_iUntakenSlotCardsLeftNumNormalFromHead = -1;

        m_iCfgStackNumKeptFromTail = -1;
    };

    //Fill in basic info when game reseted, but not thrown dices yet
    void initWhenCardsFilledInUntakenSlot(int32 stackCount, int32 cardCount, int32 CfgStackNumKeptFromTail)
    {
        m_iUntakenSlotLengthAtStart = stackCount;

        m_iUntakenSlotCardsLeftNumTotal = cardCount;

        m_iCfgStackNumKeptFromTail = CfgStackNumKeptFromTail;
    };

    void setupWhenDicesThrownForCardDistAtStart(const TArray<FMyIdCollectionCpp>& aUntakenCardStacks, int32 iIdxUntakenSlotHeadNow)
    {
        MY_VERIFY(m_iUntakenSlotLengthAtStart > 0);

        m_iIdxUntakenSlotHeadNow = iIdxUntakenSlotHeadNow;
        m_iIdxUntakenSlotTailNow = (m_iIdxUntakenSlotHeadNow - 1 + m_iUntakenSlotLengthAtStart) % m_iUntakenSlotLengthAtStart;
        m_iIdxUntakenSlotTailAtStart = m_iIdxUntakenSlotTailNow;


        //the cfg is only say how many stack kept, now dice throwed, we can resolve how many cards kept
        int32 cardNumKept = calcUntakenSlotCardsLeftNumKeptFromTailConst(aUntakenCardStacks);

        m_iUntakenSlotCardsLeftNumKeptFromTail = cardNumKept;
        m_iUntakenSlotCardsLeftNumNormalFromHead = m_iUntakenSlotCardsLeftNumTotal - m_iUntakenSlotCardsLeftNumKeptFromTail;
    };

    bool isIdxUntakenSlotInKeptFromTailSegment(int32 idx) const
    {
        const FMyMJGameUntakenSlotInfoCpp *pInfo = this;

        int32 keptCount = pInfo->m_iCfgStackNumKeptFromTail;
        int32 idxTail = pInfo->m_iUntakenSlotLengthAtStart;
        int32 totalL = pInfo->m_iUntakenSlotLengthAtStart;

        if (keptCount <= 0) {
            return false;
        }

        MY_VERIFY(totalL > 0);
        MY_VERIFY(idxTail >= 0);

        int32 idxEnd = (idxTail + 1) % totalL; // use < 
        int32 idxStart = (idxEnd - keptCount + totalL) % totalL;

        int32 idxStartFix0, idxStartFix1, idxEndFix0, idxEndFix1;
        if (idxStart < idxEnd) {
            idxStartFix0 = idxStart;
            idxEndFix0 = idxEnd;
            idxStartFix1 = 0;
            idxEndFix1 = 0;
        }
        else {
            MY_VERIFY(idxStart != idxEnd);
            idxStartFix0 = idxStart;
            idxEndFix0 = totalL;
            idxStartFix1 = 0;
            idxEndFix1 = idxEnd;
        }

        return (idxStartFix0 <= idx && idx < idxEndFix0) || (idxStartFix1 <= idx && idx < idxEndFix1);
    }

    //Before calling, verify cfg data have been set.
    int32 calcUntakenSlotCardsLeftNumKeptFromTailConst(const TArray<FMyIdCollectionCpp> &aUntakenCardStacks) const
    {
        MY_VERIFY(m_iCfgStackNumKeptFromTail >= 0);
        MY_VERIFY(m_iIdxUntakenSlotTailAtStart >= 0);
        MY_VERIFY(m_iUntakenSlotCardsLeftNumTotal >= 0);

        int32 keptCount = m_iCfgStackNumKeptFromTail;
        int32 idxTail = m_iIdxUntakenSlotTailAtStart;
        int32 totalL = m_iUntakenSlotCardsLeftNumTotal;

        int32 idxWorking = idxTail;
        int32 stack2check = keptCount;

        int32 ret = 0;

        MY_VERIFY(totalL > 0);
        MY_VERIFY(keptCount >= 0);
        MY_VERIFY(idxTail >= 0);

        while (stack2check > 0) {

            ret += aUntakenCardStacks[idxWorking].m_aIds.Num();


            idxWorking = (idxWorking - 1 + totalL) % totalL;
            stack2check--;
        }


        return ret;
    }

    void updateUntakenInfoHeadOrTail(const TArray<FMyIdCollectionCpp>& aUntakenCardStacks, bool bUpdateHead, bool bUpdateTail)
    {

        if (m_iUntakenSlotCardsLeftNumTotal <= 0) {
            return;
        }

        MY_VERIFY(m_iIdxUntakenSlotHeadNow >= 0);
        MY_VERIFY(m_iIdxUntakenSlotTailNow >= 0);

        if (bUpdateHead) {
            int32 idx = UMyMJUtilsLibrary::getIdxOfUntakenSlotHavingCard(aUntakenCardStacks, m_iIdxUntakenSlotHeadNow, 0, false);
            MY_VERIFY(idx >= 0);
            m_iIdxUntakenSlotHeadNow = idx;
        }
        if (bUpdateTail) {
            int32 idx = UMyMJUtilsLibrary::getIdxOfUntakenSlotHavingCard(aUntakenCardStacks, m_iIdxUntakenSlotTailNow, 0, true);
            MY_VERIFY(idx >= 0);
            m_iIdxUntakenSlotTailNow = idx;
        }
    };

    inline
        int32 getCardNumCanBeTakenNormally() const
    {
        return m_iUntakenSlotCardsLeftNumNormalFromHead;
    };

    inline
        int32 getCardNumCanBeTakenAll() const
    {
        return m_iUntakenSlotCardsLeftNumTotal;
    };



    //reflect the untaken state on desktop, not how mnay/or where attender can take card

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx untaken slot head now"))
    int32 m_iIdxUntakenSlotHeadNow;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx untaken slot tail now"))
    int32 m_iIdxUntakenSlotTailNow;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx untaken slot tail at start"))
    int32 m_iIdxUntakenSlotTailAtStart;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot length at start"))
    int32 m_iUntakenSlotLengthAtStart; //Note this never change in game

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot cards left num total"))
    int32 m_iUntakenSlotCardsLeftNumTotal; //simply reflect how many cards present on dest as untaken, doesn't mean how many you can take it which should be decided by rule

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot cards left num kept from tail"))
    int32 m_iUntakenSlotCardsLeftNumKeptFromTail;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot cards left num normal from head"))
    int32 m_iUntakenSlotCardsLeftNumNormalFromHead;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "cfg stack num kept from tail"))
    int32 m_iCfgStackNumKeptFromTail; //Todo: set it when reset game
};

//Only Used for logic, common for any rule type, not for visualization, 
USTRUCT()
struct FMyMJCoreDataLogicOnlyCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJCoreDataLogicOnlyCpp()
    {
        resetForNewGame();
        m_iMsLast = 0;
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
        //m_eWorkMode = MyMJGameElemWorkModeCpp::Invalid;
    };

    void resetForNewGame()
    {
        m_eActionLoopState = MyMJActionLoopStateCpp::Invalid;
        m_cHelperShowedOut2AllCards.clear();
    };

public:

    int64 m_iMsLast;

    //actually representing the core class type, even before game config arrive
    MyMJGameRuleTypeCpp m_eRuleType;
    //MyMJGameElemWorkModeCpp m_eWorkMode;

    UPROPERTY( meta = (DisplayName = "Action Loop State"))
    MyMJActionLoopStateCpp m_eActionLoopState;

    //used to calculate how many cards left possible hu
    UPROPERTY( meta = (DisplayName = "Cards Showed Out to All"))
        FMyMJValueIdMapCpp m_cHelperShowedOut2AllCards;
};

USTRUCT(BlueprintType)
struct FMyMJCoreDataHelperCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreDataHelperCpp()
    {
        reset();
    };

    void reset()
    {
        m_aIdHelperLastCardsGivenOut.Reset();
        m_aHelperLastWeaves.Reset();
        m_aIdHelperLastCardsTakenInWholeGame.Reset();
        m_eHelperGameStateJustBeforeLastCardsTakenInWholeGame = MyMJGameStateCpp::Invalid;
    };

    //It takes all cards and number may > 1 in case of giving out cards just after taken card for gang
    //union with @m_aIdHelperLastWeave
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut"))
        TArray<int32> m_aIdHelperLastCardsGivenOut;

    //When weave, it takes all cards, and we can assert only one weave's card in. When give out cards, it takes all cards and number may > 1 in case of give out cards just ganged
    //union with @m_aIdHelperLastCardsGivenOut
    //Num() may > 1, but in most case it is 0 or 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Weaves"))
        TArray<FMyMJWeaveCpp> m_aHelperLastWeaves;

    //Note it only record the last cards taken in reltated pusher, and it is not equal to Haidi, some rule have extra condition such as Not BuZhang in local CS
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards Taken In Whole Game"))
    TArray<int32> m_aIdHelperLastCardsTakenInWholeGame;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State Just Before Last Cards Taken In Whole Game"))
    MyMJGameStateCpp m_eHelperGameStateJustBeforeLastCardsTakenInWholeGame;

};

//#define FMyMJCoreDataPublicDirectMask0_IncreaseActionGroupId (1 << 0)

//#define FMyMJCoreDataPublicDirectMask0_UpdateGameState (1 << 3)

//#define FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave (1 << 4)

#define FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart (0)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen (4)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart (4)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen (4)

#define FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart    (8)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen          (4)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_Invalid         (0)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GameStart       (1)
#define FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GangYaoLocalCS  (2)

//#define FMyMJCoreDataPublicDirectMask0_UpdateHelperLastCardsGivenOutOrWeave (1 << 5)
//#define FMyMJCoreDataPublicDirectMask0_UpdateHelperLastCardTakenInGame (1 << 7) //not needed. automatically set when removing card from untaken slot

//Both used for logic and visualize
//What the fuck is, UE3 network always send all structor data in one packet even some members in it haven't change, thanks to UE4 this changed, and if Atomic specified, it goes UE3's way
USTRUCT(BlueprintType)
struct FMyMJCoreDataPublicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreDataPublicCpp()
    {
        reset();
    };

    virtual ~FMyMJCoreDataPublicCpp()
    {

    };

    void reset()
    {
        //some data does not need to reset since reset pusher will overwrite them all
        m_aUntakenCardStacks.Reset();
        m_cCardInfoPack.reset(0);

        m_cGameCfg.reset();
        m_cGameRunData.reset();

        m_iGameId = MyIntIdDefaultInvalidValue;
        m_iPusherIdLast = MyIntIdDefaultInvalidValue;
        m_iActionGroupId = MyIntIdDefaultInvalidValue;
        m_eGameState = MyMJGameStateCpp::Invalid;

        m_iDiceNumberNowMask = 0;

        m_cUntakenSlotInfo.reset();
        m_cHelper.reset();
    };

    inline bool isInGameState() const
    {
        return (!(m_eGameState == MyMJGameStateCpp::Invalid || m_eGameState == MyMJGameStateCpp::GameEnd));
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UnTaken Card Stacks"))
        TArray<FMyIdCollectionCpp> m_aUntakenCardStacks; //Always start from attender 0 to 3

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card info pack"))
        FMyMJCardInfoPackCpp  m_cCardInfoPack;

    //FMyMJCardValuePackCpp m_cCardValuePack;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Cfg"))
        FMyMJGameCfgCpp m_cGameCfg;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game RunData"))
        FMyMJGameRunDataCpp m_cGameRunData;

    // >= 0 when valid
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Id"))
        int32 m_iGameId;

    // >= 0 when valid, the last pusher id we got
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Pusher Id Last"))
        int32 m_iPusherIdLast;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Action Group Id"))
        int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
     MyMJGameStateCpp m_eGameState;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Untaken Slot Info"))
    FMyMJGameUntakenSlotInfoCpp m_cUntakenSlotInfo;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Helper Data"))
    FMyMJCoreDataHelperCpp m_cHelper;


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "DiceNumberNowMask"))
    int32 m_iDiceNumberNowMask;

};

//if not specified in member name, they are the target state
USTRUCT(BlueprintType)
struct FMyMJCoreDataDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCoreDataDeltaCpp()
    {
        m_eGameState = MyMJGameStateCpp::Invalid;
        m_iDiceNumberNowMask = 0;
        m_bIncreaseActionGroupId = m_bUpdateGameState = m_bResetHelperLastCardsGivenOutOrWeave = false;
    };

    //if not empty, update them
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card infos"))
    TArray<FMyMJCardInfoCpp> m_aCardInfos2Update;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
    MyMJGameStateCpp m_eGameState;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    //TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "DiceNumberNowMask"))
    int32 m_iDiceNumberNowMask;


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "IncreaseActionGroupId"))
    uint32 m_bIncreaseActionGroupId : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UpdateGameState"))
    uint32 m_bUpdateGameState : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "ResetHelperLastCardsGivenOutOrWeave"))
        uint32 m_bResetHelperLastCardsGivenOutOrWeave : 1;

};


//Only Used for logic, common for any rule type, not for visualization, 
USTRUCT()
struct FMyMJAttenderDataLogicOnlyCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJAttenderDataLogicOnlyCpp()
    {
        setup(-1);
    };


    void setup(int32 idxAttender)
    {
        m_iIdx = idxAttender;
        m_cActionContainor.setup(idxAttender);
        m_cActionContainor.reinit(false);
        resetForNewGame();
    };

    void resetForNewGame()
    {
        m_iTurn = 0;
        m_cHandCards.clear();
        m_cActionContainor.resetForNewLoop();
    };

    //can be cast to MyMJGameRoleTypeCpp
    int32 m_iIdx;

    UPROPERTY( NotReplicated, meta = (DisplayName = "turn"))
        int32 m_iTurn;

    //not need to duplicate, it is only used in core logic, not graphic
    UPROPERTY( meta = (DisplayName = "hand card Map"))
        FMyMJValueIdMapCpp m_cHandCards;

    UPROPERTY( meta = (DisplayName = "action containor for choices"))
        FMyMJGameActionContainorCpp m_cActionContainor;

};

//#define FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender       (1 << 0)
//#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsRealAttender (1 << 1)
//#define FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame        (1 << 2)
//#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsStillInGame  (1 << 3)


//#define FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS        (1 << 16)
//#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateGangYaoedLocalCS  (1 << 17)


USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPublicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPublicCpp()
    {
        setup(-1);
    };

    virtual ~FMyMJRoleDataAttenderPublicCpp()
    {

    };

    void setup(int32 idxAttender)
    {
        m_iIdxAttender = idxAttender;
        reset();
    };

    void reset()
    {
        m_aIdHandCards.Reset();
        m_aIdJustTakenCards.Reset();
        m_aIdGivenOutCards.Reset();
        m_aShowedOutWeaves.Reset();
        m_aIdWinSymbolCards.Reset();
        m_aIdShownOnDesktopCards.Reset();

        m_cUntakenSlotSubSegmentInfo.reset();
        m_aHuScoreResultFinalGroups.Reset();

        m_bIsRealAttender = m_bIsStillInGame = m_bGangYaoedLocalCS = false;

    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx Attender"))
    int32 m_iIdxAttender; //it is the position

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards in hand"))
    TArray<int32> m_aIdHandCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards just taken"))
    TArray<int32> m_aIdJustTakenCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards just given out"))
    TArray<int32> m_aIdGivenOutCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "weaves"))
    TArray<FMyMJWeaveCpp> m_aShowedOutWeaves; //weaves showed out

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards win symbol"))
    TArray<int32> m_aIdWinSymbolCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards shown on desktop"))
    TArray<int32> m_aIdShownOnDesktopCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards win symbol"))
    FMyMJGameUntakenSlotSubSegmentInfoCpp m_cUntakenSlotSubSegmentInfo;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final Groups"))
    TArray<FMyMJHuScoreResultFinalGroupCpp> m_aHuScoreResultFinalGroups;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "IsRealAttender"))
        uint32 m_bIsRealAttender : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "IsStillInGame"))
        uint32 m_bIsStillInGame : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "GangYaoedLocalCS"))
        uint32 m_bGangYaoedLocalCS : 1;

};

USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPublicDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPublicDeltaCpp()
    {
        m_bIsRealAttender = m_bIsStillInGame = m_bGangYaoedLocalCS = false;
        m_bUpdateIsRealAttender = m_bUpdateIsStillInGame = m_bUpdateGangYaoedLocalCS = false;
    };

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "weave to add"))
    TArray<FMyMJWeaveCpp> m_aWeave2Add;

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final Group to Add"))
    TArray<FMyMJHuScoreResultFinalGroupCpp> m_aHuScoreResultFinalGroup2Add;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "IsRealAttender"))
        uint32 m_bIsRealAttender : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "IsStillInGame"))
        uint32 m_bIsStillInGame : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "GangYaoedLocalCS"))
        uint32 m_bGangYaoedLocalCS : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UpdateIsRealAttender"))
        uint32 m_bUpdateIsRealAttender : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UpdateIsStillInGame"))
        uint32 m_bUpdateIsStillInGame : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UpdateGangYaoedLocalCS"))
        uint32 m_bUpdateGangYaoedLocalCS : 1;
};

//#define FMyMJRoleDataAttenderPrivateCpp_Mask0_BanPaoHuLocalCS         (1 << 0)
//#define FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS   (1 << 1)

USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPrivateCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPrivateCpp()
    {
        reset();
    };

    virtual ~FMyMJRoleDataAttenderPrivateCpp()
    {

    };

    void reset()
    {
        m_cActionContainor.resetForNewActionLoop();
        m_cHuScoreResultTingGroup.reset();
        m_bBanPaoHuLocalCS = false;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor"))
    FMyMJGameActionContainorForBPCpp m_cActionContainor;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    FMyMJHuScoreResultTingGroupCpp  m_cHuScoreResultTingGroup;


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "BanPaoHuLocalCS"))
    uint32 m_bBanPaoHuLocalCS : 1;

};


USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPrivateDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPrivateDeltaCpp()
    {
        m_bBanPaoHuLocalCS = m_bUpdateBanPaoHuLocalCS = false;
    };

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor"))
    TArray<FMyMJGameActionContainorForBPCpp> m_aActionContainor;

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    TArray<FMyMJHuScoreResultTingGroupCpp>  m_aHuScoreResultTingGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "BanPaoHuLocalCS"))
    uint32 m_bBanPaoHuLocalCS : 1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UpdateBanPaoHuLocalCS"))
    uint32 m_bUpdateBanPaoHuLocalCS : 1;
};

USTRUCT(BlueprintType)
struct FMyMJRoleDataPrivateCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataPrivateCpp()
    {
        reset();
    };

    void reset()
    {
        m_cCardValuePack.reset(0);
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Card Value Pack"))
        FMyMJCardValuePackCpp m_cCardValuePack;
};

#define MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All               (0x0f | (1 << (uint8)MyMJGameRoleTypeCpp::Observer))
#define MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_One(idxAttender)  (1 << idxAttender)

//this is mutable for different role of seeing the game, some role may be even empty
USTRUCT(BlueprintType)
struct FMyMJRoleDataPrivateDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataPrivateDeltaCpp()
    {
        m_eRoleType = MyMJGameRoleTypeCpp::Max;
    };

    void reset()
    {
        m_aIdValuePairs2Reveal.Reset();
        m_eRoleType = MyMJGameRoleTypeCpp::Max;
        m_iRoleMaskForDataPrivateClone = 0;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card id values to reveal"))
    TArray<FMyIdValuePair> m_aIdValuePairs2Reveal;

    //mute for different role of viewer, not actually used but for debug purpose
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role type it belong to"))
    MyMJGameRoleTypeCpp m_eRoleType;

    //Controls which role should have @m_aDataPrivate when clone/mute for different roles
    //when a role test return false, @m_aDataPrivate will be removed afer clone for that role
    int32 m_iRoleMaskForDataPrivateClone;

};

USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderDeltaCpp()
    {
        m_iIdxAttender = -1;
    };

    //Num > 0 means valid, and num must equal 1 in that case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data public"))
    TArray<FMyMJRoleDataAttenderPublicDeltaCpp> m_aDataPublic;

    //Num > 0 means valid, and num must equal 1 in that case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data private"))
    TArray<FMyMJRoleDataAttenderPrivateDeltaCpp> m_aDataPrivate;

    //equal to idx role, MyMJRoleTypeCpp
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx attender"))
    int32 m_iIdxAttender;
};


USTRUCT()
struct FMyMJRoleDataAttenderCpp
{
    GENERATED_USTRUCT_BODY()

public:

    friend struct FMyMJDataStructCpp;

    void resetup(uint8 idxRole)
    {
        m_cDataAttenderPublic.setup(idxRole);
        reset();
    };

    void reset()
    {
        m_cDataAttenderPublic.reset();
        m_cDataAttenderPrivate.reset();
        m_cDataPrivate.reset();
    };

protected:

    UPROPERTY()
    FMyMJRoleDataAttenderPublicCpp  m_cDataAttenderPublic;
    
    UPROPERTY()
    FMyMJRoleDataAttenderPrivateCpp m_cDataAttenderPrivate;
    
    UPROPERTY()
    FMyMJRoleDataPrivateCpp         m_cDataPrivate;
};


USTRUCT(BlueprintType)
struct FMyMJDataDeltaCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataDeltaCpp() : Super()
    {
        m_iIdxAttenderActionInitiator = -1;
        m_iGameId = -1;
        //m_uiStateServerWorldTimeStamp_10ms = 0;
        //m_uiTimeWaitForAnimation_10Ms = 0;
    };

    virtual FString genDebugString() const
    {
        FString ret = Super::genDebugString();

        if (m_aCoreData.Num() > 0) {
            if (m_aCoreData[0].m_bUpdateGameState) {
                ret += FString::Printf(TEXT("gameState %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameStateCpp"), (uint8)m_aCoreData[0].m_eGameState));
            }
        }

        return ret;
    };

    inline
    int32 getIdxAttenderActionInitiator(bool bVerifyValid = true) const
    {
        if (bVerifyValid) {
            MY_VERIFY(m_iIdxAttenderActionInitiator >= 0 && m_iIdxAttenderActionInitiator < 4);
        }
        return m_iIdxAttenderActionInitiator;
    };

    //self must be syskeeper's role
    void copyWithRoleFromSysKeeperRole(MyMJGameRoleTypeCpp eTargetRole, FMyMJDataDeltaCpp& cTargetDelta) const
    {
        MY_VERIFY(eTargetRole != MyMJGameRoleTypeCpp::SysKeeper)

        *(StaticCast<FMyMJGamePusherBaseCpp *>(&cTargetDelta)) = *this;

        cTargetDelta.m_aCoreData = m_aCoreData;
        cTargetDelta.m_aRoleDataAttender = m_aRoleDataAttender;
        cTargetDelta.m_aRoleDataPrivate = m_aRoleDataPrivate;
        cTargetDelta.m_iIdxAttenderActionInitiator = m_iIdxAttenderActionInitiator;
        cTargetDelta.m_iGameId = m_iGameId;


        int32 l = cTargetDelta.m_aRoleDataAttender.Num();
        MY_VERIFY(l <= 1);
        for (int32 i = 0; i < l; i++) {
            if ((uint8)eTargetRole != i) {
                cTargetDelta.m_aRoleDataAttender[i].m_aDataPrivate.Reset();
            }
        }

        l = cTargetDelta.m_aRoleDataPrivate.Num();
        MY_VERIFY(l <= 1);
        if (l > 0) {
            bool bShouldKeepInfo = (cTargetDelta.m_aRoleDataPrivate[0].m_iRoleMaskForDataPrivateClone & (uint8)eTargetRole) > 0;
            if (bShouldKeepInfo) {
                cTargetDelta.m_aRoleDataPrivate[0].m_eRoleType = eTargetRole;
            }
            else {
                cTargetDelta.m_aRoleDataPrivate.Reset();
            }
        }
    };

    //Num > 0 means valid, and num must equal 1 in that case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "core data"))
    TArray<FMyMJCoreDataDeltaCpp> m_aCoreData;

    //Num > 0 means valid, and num must equal 1 in that case
    //represent whe attender's update, what he does to make game progress, private data is hidden to other
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role data attender"))
    TArray<FMyMJRoleDataAttenderDeltaCpp> m_aRoleDataAttender;

    //Num > 0 means valid, and num must equal 1 in that case
    //represent the role's update, how he knows the game,  it is different for different role
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role data private"))
    TArray<FMyMJRoleDataPrivateDeltaCpp> m_aRoleDataPrivate;

    //most action, have a Initiator
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx Attender Action Initiator"))
    int32 m_iIdxAttenderActionInitiator;

    //>= 0 means valid, in a idea env, game id and pusher id is not neccessary, we put it here just to detect if data goes wrong in network case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "game id"))
    int32 m_iGameId;
};

#define RoleDataAttenderNum (4)

USTRUCT(BlueprintType)
struct FMyMJDataStructCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataStructCpp();
    virtual ~FMyMJDataStructCpp();

    void reset()
    {
        m_cCoreDataPublic.reset();
        int32 l = m_aRoleDataAttenders.Num();
        for (int i = 0; i < l; i++) {
            m_aRoleDataAttenders[i].reset();
        }
        m_cRoleDataPrivateSysKeeper.reset();
        m_cRoleDataPrivateObserver.reset();
    };

    inline const FMyMJCoreDataPublicCpp& getCoreDataPublicRefConst() const
    {
        return m_cCoreDataPublic;
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRefConst(int32 idxAttender) const
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < RoleDataAttenderNum);

        MY_VERIFY(m_aRoleDataAttenders.Num() == RoleDataAttenderNum);
        return m_aRoleDataAttenders[idxAttender].m_cDataAttenderPublic;
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataAttenderPrivateCpp& getRoleDataAttenderPrivateRefConst(int32 idxAttender) const
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < RoleDataAttenderNum);

        MY_VERIFY(m_aRoleDataAttenders.Num() == RoleDataAttenderNum);
        return m_aRoleDataAttenders[idxAttender].m_cDataAttenderPrivate;
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataPrivateCpp& getRoleDataPrivateRefConst(int32 idxRole) const
    {
        MY_VERIFY(idxRole >= 0 && idxRole < (uint8)MyMJGameRoleTypeCpp::Max);

        MY_VERIFY(m_aRoleDataAttenders.Num() == RoleDataAttenderNum);

        if (idxRole < RoleDataAttenderNum)
        {
            return m_aRoleDataAttenders[idxRole].m_cDataPrivate;
        }
        else if (idxRole == (uint8)MyMJGameRoleTypeCpp::SysKeeper) {
            return m_cRoleDataPrivateSysKeeper;
        }
        else if (idxRole == (uint8)MyMJGameRoleTypeCpp::Observer) {
            return m_cRoleDataPrivateObserver;
        }
        else {
            MY_VERIFY(false);
            return m_aRoleDataAttenders[idxRole].m_cDataPrivate;
        }

    };

    //self must be syskeeper's role
    void copyWithRoleFromSysKeeperRole(MyMJGameRoleTypeCpp eTargetRole, FMyMJDataStructCpp& cTargetData) const
    {
        MY_VERIFY(eTargetRole != MyMJGameRoleTypeCpp::SysKeeper);
        MY_VERIFY(getRole() == MyMJGameRoleTypeCpp::SysKeeper);

        cTargetData = *this;
        cTargetData.setRole(eTargetRole);

        //remove unused infos
        cTargetData.m_cRoleDataPrivateSysKeeper.reset();

        int32 l = cTargetData.m_aRoleDataAttenders.Num();
        for (int32 i = 0; i < l; i++) {
            if ((uint8)eTargetRole != i) {
                cTargetData.m_aRoleDataAttenders[i].m_cDataAttenderPrivate.reset();
                cTargetData.m_aRoleDataAttenders[i].m_cDataPrivate.reset();
            }
        }

        if (eTargetRole != MyMJGameRoleTypeCpp::Observer) {
            cTargetData.m_cRoleDataPrivateObserver.reset();
        }
    };

    inline MyMJGameRoleTypeCpp getRole() const
    {
        return m_eRole;
    };

    inline void setRole(MyMJGameRoleTypeCpp eRole)
    {
        m_eRole = eRole;
    };

protected:



    UPROPERTY()
    FMyMJCoreDataPublicCpp m_cCoreDataPublic;
    
    UPROPERTY()
    TArray<FMyMJRoleDataAttenderCpp> m_aRoleDataAttenders;

    //To reduce size, syskeeper and observer have no attender data and we list them here seperately
    UPROPERTY()
    FMyMJRoleDataPrivateCpp m_cRoleDataPrivateSysKeeper;
    
    UPROPERTY()
    FMyMJRoleDataPrivateCpp m_cRoleDataPrivateObserver;

    UPROPERTY()
    MyMJGameRoleTypeCpp m_eRole;

    //UPROPERTY()
    //TArray<int32> m_aTestArray;

    //UPROPERTY()
    //TArray<FMyMJRoleDataPrivateCpp> m_aTestPrivArray;
};

USTRUCT()
struct FMyMJGamePusherResultCpp
{
    GENERATED_USTRUCT_BODY()
public:
    FMyMJGamePusherResultCpp()
    {
        //m_pResultBase = NULL;
        //m_pResultDelta = NULL;
    };

    virtual ~FMyMJGamePusherResultCpp()
    {
        reset();
    };

    void reset()
    {
        m_aResultBase.Reset();
        m_aResultDelta.Reset();
    }

    bool checkHaveValidResult()
    {
        int32 l0 = m_aResultBase.Num();
        int32 l1 = m_aResultDelta.Num();

        if ((l0 + l1) == 1) {
            return true;
        }

        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Invalid pusher result: base count %d, delta count %d."), l0, l1);

        return false;
    };

    void copyDeep(const FMyMJGamePusherResultCpp &other)
    {
        reset();

        m_aResultBase = other.m_aResultBase;
        m_aResultDelta = other.m_aResultDelta;
    }

    //returned one is allocated on heap
    inline FMyMJGamePusherResultCpp* cloneDeep() const
    {
        FMyMJGamePusherResultCpp* ret = new FMyMJGamePusherResultCpp();
        ret->copyDeep(*this);
        return ret;
    };

    void copyWithRoleFromSysKeeperRole(MyMJGameRoleTypeCpp eTargetRole, FMyMJGamePusherResultCpp& cTargetData) const
    {
        cTargetData.m_aResultBase.Reset();
        cTargetData.m_aResultDelta.Reset();

        int32 l;

        l = m_aResultBase.Num();
        MY_VERIFY(l <= 1);
        if (l > 0) {
            cTargetData.m_aResultBase.Emplace();
            m_aResultBase[0].copyWithRoleFromSysKeeperRole(eTargetRole, cTargetData.m_aResultBase[0]);
        }

        l = m_aResultDelta.Num();
        MY_VERIFY(l <= 1);
        if (l > 0) {
            cTargetData.m_aResultDelta.Emplace();
            m_aResultDelta[0].copyWithRoleFromSysKeeperRole(eTargetRole, cTargetData.m_aResultDelta[0]);
        }
    };

    inline void verifyValid() const
    {
        MY_VERIFY((m_aResultBase.Num() + m_aResultDelta.Num()) == 1);
    };

    //return always >= 0, means valid
    inline void getGameIdAndPusherId(int32 *pOutGameId, int32* pOutPusherId) const
    {
        verifyValid();

        int32 iGameId, iPusherId;

        if (m_aResultBase.Num() > 0) {
            iGameId = m_aResultBase[0].getCoreDataPublicRefConst().m_iGameId;
            iPusherId = m_aResultBase[0].getCoreDataPublicRefConst().m_iPusherIdLast;
        }
        else if (m_aResultDelta.Num() > 0) {
            iGameId = m_aResultDelta[0].m_iGameId;
            iPusherId = m_aResultDelta[0].getId();
        }
        else {
            MY_VERIFY(false);
            iGameId = iPusherId = -1;
        }

        MY_VERIFY(iGameId >= 0);
        MY_VERIFY(iPusherId >= 0);

        if (pOutGameId) {
            *pOutGameId = iGameId;
        }
        if (pOutPusherId) {
            *pOutPusherId = iPusherId;
        }
    };

    bool isGameEndDelta() const
    {
        if (m_aResultDelta.Num() > 0 && m_aResultDelta[0].getType() == MyMJGamePusherTypeCpp::ActionStateUpdate &&
            m_aResultDelta[0].m_aCoreData.Num() > 0 && m_aResultDelta[0].m_aCoreData[0].m_eGameState == MyMJGameStateCpp::GameEnd)
        {
            return true;
        }

        if (m_aResultBase.Num() > 0 && m_aResultBase[0].getCoreDataPublicRefConst().m_eGameState == MyMJGameStateCpp::GameEnd) {
            MY_VERIFY(false);
        }

        return false;
    }

    FString genDebugMsg() const
    {
        if (m_aResultBase.Num() > 0) {
            return TEXT("Base");
        }
        else {
            MY_VERIFY(m_aResultDelta.Num() == 1)
            return m_aResultDelta[0].genDebugString();
        }
    };

    UPROPERTY()
    TArray<FMyMJDataStructCpp> m_aResultBase;

    UPROPERTY()
    TArray<FMyMJDataDeltaCpp>  m_aResultDelta;
};

UENUM(BlueprintType)
enum class MyMJGameCoreDataDirtyMainTypeCpp : uint8
{
    Invalid = 0                        UMETA(DisplayName = "Invalid"),
    Card = 1                           UMETA(DisplayName = "Card"),
    Dice = 2                           UMETA(DisplayName = "Dice"),
};

USTRUCT()
struct FMyMJDataAccessorCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJDataAccessorCpp()
    {
        m_pDataExt = NULL;
        m_bDebugIsFullMode = false;
        m_bShowApplyInfo = false;
    };

    virtual ~FMyMJDataAccessorCpp()
    {
        //setHelperAttenderSlotDirtyMasksEnabled(false);
    };

    void setShowApplyInfo(bool bShowApplyInfo)
    {
        m_bShowApplyInfo = bShowApplyInfo;
    };

    inline
    MyMJGameRoleTypeCpp getAccessRoleType(bool bVerify = true) const
    {
        MyMJGameRoleTypeCpp ret = MyMJGameRoleTypeCpp::Max;
 
        if (m_pDataExt) {
            ret = m_pDataExt->getRole();
        }

        if (bVerify && (uint8)ret >= (uint8)MyMJGameRoleTypeCpp::Max) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid data role: ret %d, m_pDataExt %p."), (uint8)ret, m_pDataExt);
            MY_VERIFY(false);
        }

        return ret;
    };

    void setupDataExt(FMyMJDataStructCpp* pDataExt, bool bDebugIsFullMode = false)
    {
        //MY_VERIFY(m_eWorkMode == MyMJGameElemWorkModeCpp::Invalid); //it is OK to change multiple times
        MY_VERIFY(pDataExt);

        m_pDataExt = pDataExt;
        m_bDebugIsFullMode = bDebugIsFullMode;
    };

    inline
    bool isSetupped() const
    {
        return m_pDataExt != NULL;
    };

    //don't store the return, it can be invalid anytime
    const FMyMJCoreDataPublicCpp& getCoreDataPublicRefConst() const
    {
        MY_VERIFY(m_pDataExt);
        return m_pDataExt->getCoreDataPublicRefConst();
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime
    const FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRefConst(int32 idxAttender) const
    {
        MY_VERIFY(m_pDataExt);
        return m_pDataExt->getRoleDataAttenderPublicRefConst(idxAttender);
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime, can return NULL
    const FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivateConst(int32 idxAttender) const
    {
        MY_VERIFY(m_pDataExt);
        return &m_pDataExt->getRoleDataAttenderPrivateRefConst(idxAttender);
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataPrivateCpp* getRoleDataPrivateConst(int32 idxRole) const
    {
        MY_VERIFY(m_pDataExt);
        return &m_pDataExt->getRoleDataPrivateRefConst(idxRole);
    };

    //Note all data will be overwritten, include default values, usually used in full mode, but also possible for mirror as one way to implement replay  
    void applyBase(const FMyMJDataStructCpp &base, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord);

    //the delta myst be applied by two step, and after step 0 it can be visualized, can be used by both full and mirror mode
    //after step 0 we have a chance for visualize, before calling next
    void applyDeltaStep0(const FMyMJDataDeltaCpp &delta, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord);
    void applyDeltaStep1(const FMyMJDataDeltaCpp &delta, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord);

    //direct to apply, don't leave a chance for visualize
    void applyDelta(const FMyMJDataDeltaCpp &delta, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord);


    void applyPusherResult(const FMyMJGamePusherResultCpp& result, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
    {
        if (result.m_aResultBase.Num() > 0) {
            applyBase(result.m_aResultBase[0], pDirtyRecord);
        }

        if (result.m_aResultDelta.Num() > 0) {
            applyDelta(result.m_aResultDelta[0], pDirtyRecord);
        }
    };

    void resetForNewActionLoop();

protected:

    inline FMyMJCoreDataPublicCpp& getCoreDataRef()
    {
        return const_cast<FMyMJCoreDataPublicCpp &>(getCoreDataPublicRefConst());
    };

    inline FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRef(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPublicCpp &>(getRoleDataAttenderPublicRefConst(idxAttender));
    };

    inline FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivate(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPrivateCpp *>(getRoleDataAttenderPrivateConst(idxAttender));
    };

    inline FMyMJRoleDataPrivateCpp* getRoleDataPrivate(int32 idxRole)
    {
        return const_cast<FMyMJRoleDataPrivateCpp *>(getRoleDataPrivateConst(idxRole));
    };

    FMyMJCardValuePackCpp& getCardValuePackRef()
    {
        int32 iAccessRoleType = (uint8)getAccessRoleType();

        FMyMJRoleDataPrivateCpp* pDRolePriv = getRoleDataPrivate(iAccessRoleType);

        MY_VERIFY(pDRolePriv);

        return pDRolePriv->m_cCardValuePack;
    };

    FString genDebugStateString() const
    {
        return FString::Printf(TEXT("[%s]"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRoleTypeCpp"), (uint8)getAccessRoleType(false)));
    };


    void moveCardFromOldPosi(int32 id, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord);
    //@idxAttender can < 0
    void moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord);

    FMyMJDataStructCpp* m_pDataExt; //not owning it
    bool m_bDebugIsFullMode;

    bool m_bShowApplyInfo;
};