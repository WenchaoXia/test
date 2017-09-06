// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyMJCommonDefines.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"

#include "MyMJGameData.generated.h"

//Todo: use UE4's generated delta instead of my own

USTRUCT()
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

USTRUCT()
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Loop State"))
    MyMJActionLoopStateCpp m_eActionLoopState;

    //used to calculate how many cards left possible hu
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Cards Showed Out to All"))
        FMyMJValueIdMapCpp m_cHelperShowedOut2AllCards;
};

USTRUCT()
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

#define FMyMJCoreDataPublicDirectMask0_IncreaseActionGroupId (1 << 0)

#define FMyMJCoreDataPublicDirectMask0_UpdateGameState (1 << 3)

#define FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave (1 << 4)

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
USTRUCT()
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
        m_aIdShownOnDesktopCards.Reset();
        m_cCardInfoPack.reset(0);

        m_cGameCfg.reset();
        m_cGameRunData.reset();

        m_iGameId = -1;
        m_iPusherIdLast = -1;
        m_iActionGroupId = -1;
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards shown on desktop"))
    TArray<int32> m_aIdShownOnDesktopCards;

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
    UPROPERTY(BlueprintReadWrite, NotReplicated, meta = (DisplayName = "Pusher Id Last"))
        int32 m_iPusherIdLast;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Action Group Id"))
        int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
     MyMJGameStateCpp m_eGameState;

    //see mask define
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number Now Mask"))
    int32 m_iDiceNumberNowMask;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Untaken Slot Info"))
    FMyMJGameUntakenSlotInfoCpp m_cUntakenSlotInfo;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Helper Data"))
    FMyMJCoreDataHelperCpp m_cHelper;

};

//if not specified in member name, they are the target state
USTRUCT()
struct FMyMJCoreDataDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCoreDataDeltaCpp()
    {
        m_eGameState = MyMJGameStateCpp::Invalid;
        m_iDiceNumberNowMask = 0;
        m_iMask0 = 0;
    };

    //if not empty, update them
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card infos"))
    TArray<FMyMJCardInfoCpp> m_aCardInfos2Update;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
    MyMJGameStateCpp m_eGameState;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    //TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Numbers Now Mask"))
    int32 m_iDiceNumberNowMask;

    //used for bit bool and bit tip updating as delta
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
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

    UPROPERTY(BlueprintReadWrite, NotReplicated, meta = (DisplayName = "turn"))
        int32 m_iTurn;

    //not need to duplicate, it is only used in core logic, not graphic
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hand card Map"))
        FMyMJValueIdMapCpp m_cHandCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor for choices"))
        FMyMJGameActionContainorCpp m_cActionContainor;

};

#define FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender       (1 << 0)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsRealAttender (1 << 1)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame        (1 << 2)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsStillInGame  (1 << 3)


#define FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS        (1 << 16)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateGangYaoedLocalCS  (1 << 17)


USTRUCT()
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
        m_cUntakenSlotSubSegmentInfo.reset();
        m_aHuScoreResultFinalGroups.Reset();

        m_iMask0 = 0;
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards win symbol"))
    FMyMJGameUntakenSlotSubSegmentInfoCpp m_cUntakenSlotSubSegmentInfo;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final Groups"))
    TArray<FMyMJHuScoreResultFinalGroupCpp> m_aHuScoreResultFinalGroups;

    //used for bit bool and bit tip updating as delta
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
};

USTRUCT()
struct FMyMJRoleDataAttenderPublicDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPublicDeltaCpp()
    {
        m_iMask0 = 0;
    };

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "weave to add"))
    TArray<FMyMJWeaveCpp> m_aWeave2Add;

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final Group to Add"))
    TArray<FMyMJHuScoreResultFinalGroupCpp> m_aHuScoreResultFinalGroup2Add;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
};

#define FMyMJRoleDataAttenderPrivateCpp_Mask0_BanPaoHuLocalCS         (1 << 0)
#define FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS   (1 << 1)

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
        m_iMask0 = 0;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor"))
    FMyMJGameActionContainorForBPCpp m_cActionContainor;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    FMyMJHuScoreResultTingGroupCpp  m_cHuScoreResultTingGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;

};


USTRUCT()
struct FMyMJRoleDataAttenderPrivateDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPrivateDeltaCpp()
    {
        m_iMask0 = 0;
    };

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor"))
    TArray<FMyMJGameActionContainorForBPCpp> m_aActionContainor;

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    TArray<FMyMJHuScoreResultTingGroupCpp>  m_aHuScoreResultTingGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
};

USTRUCT()
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
USTRUCT()
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

USTRUCT()
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

    FMyMJRoleDataAttenderPublicCpp  m_cDataAttenderPublic;
    FMyMJRoleDataAttenderPrivateCpp m_cDataAttenderPrivate;
    FMyMJRoleDataPrivateCpp         m_cDataPrivate;
};


USTRUCT()
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
        for (int32 i = 0; i < l; i++) {
            bool bShouldKeepInfo = (cTargetDelta.m_aRoleDataPrivate[i].m_iRoleMaskForDataPrivateClone & (uint8)eTargetRole) > 0;
            if (!bShouldKeepInfo) {
                cTargetDelta.m_aRoleDataPrivate[i].reset();
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

USTRUCT()
struct FMyMJDataStructCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataStructCpp()
    {
        for (int i = 0; i < RoleDataAttenderNum; i++) {
            int32 idx = m_aRoleDataAttenders.Emplace();
            FMyMJRoleDataAttenderCpp *pD = &m_aRoleDataAttenders[idx];
            pD->resetup(i);
        }
    };

    void reset()
    {
        m_cCoreData.reset();
        int32 l = m_aRoleDataAttenders.Num();
        for (int i = 0; i < l; i++) {
            m_aRoleDataAttenders[i].reset();
        }
        m_cRoleDataPrivateSysKeeper.reset();
        m_cRoleDataPrivateObserver.reset();
    };

    inline const FMyMJCoreDataPublicCpp& getCoreDataRefConst() const
    {
        return m_cCoreData;
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
        MY_VERIFY(eTargetRole != MyMJGameRoleTypeCpp::SysKeeper)

            cTargetData.m_cCoreData = m_cCoreData;
        cTargetData.m_aRoleDataAttenders = m_aRoleDataAttenders;
        cTargetData.m_cRoleDataPrivateSysKeeper.reset();
        cTargetData.m_cRoleDataPrivateObserver = m_cRoleDataPrivateObserver;

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

protected:



    UPROPERTY()
    FMyMJCoreDataPublicCpp m_cCoreData;
    
    UPROPERTY()
    TArray<FMyMJRoleDataAttenderCpp> m_aRoleDataAttenders;

    //To reduce size, syskeeper and observer have no attender data and we list them here seperately
    UPROPERTY()
    FMyMJRoleDataPrivateCpp m_cRoleDataPrivateSysKeeper;
    
    UPROPERTY()
    FMyMJRoleDataPrivateCpp m_cRoleDataPrivateObserver;
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
    inline int32 getGameId() const
    {
        verifyValid();

        int32 ret = 0;
        if (m_aResultBase.Num() > 0) {
            ret = m_aResultBase[0].getCoreDataRefConst().m_iGameId;
            MY_VERIFY(ret >= 0);
        }
        else if (m_aResultDelta.Num() > 0) {
            ret = m_aResultDelta[0].m_iGameId;
            MY_VERIFY(ret >= 0);
        }
        else {
            MY_VERIFY(false);
        }

        MY_VERIFY(ret >= 0);
        return ret;
    };

    //return always >= 0, means valid
    inline int32 getPusherIdLast() const
    {
        verifyValid();

        int32 ret = 0;
        if (m_aResultBase.Num() > 0) {
            ret = m_aResultBase[0].getCoreDataRefConst().m_iPusherIdLast;
            //MY_VERIFY(ret >= 0);
            MY_VERIFY(ret == 0);
        }
        else if (m_aResultDelta.Num() > 0) {
            ret = m_aResultDelta[0].getId();
            MY_VERIFY(ret >= 0);
        }
        else {
            MY_VERIFY(false);
        }

        MY_VERIFY(ret >= 0);
        return ret;
    };

    UPROPERTY()
    TArray<FMyMJDataStructCpp> m_aResultBase;

    UPROPERTY()
    TArray<FMyMJDataDeltaCpp>  m_aResultDelta;
};


USTRUCT()
struct FMyMJDataAccessorCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJDataAccessorCpp()
    {
        m_pDataFullMode = NULL;

        m_pDataExtTempMode = NULL;
        m_eAccessRoleType = MyMJGameRoleTypeCpp::Observer;
        m_eWorkMode = MyMJGameElemWorkModeCpp::Invalid;
    };

    MyMJGameRoleTypeCpp getAccessRoleType() const
    {
        //todo: make it conditional
        return m_eAccessRoleType;
    };

    void setupFullMode()
    {
        MY_VERIFY(m_eWorkMode == MyMJGameElemWorkModeCpp::Invalid);
        m_eAccessRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        m_eWorkMode = MyMJGameElemWorkModeCpp::Full;
        m_pDataFullMode = MakeShareable<FMyMJDataStructCpp>(new FMyMJDataStructCpp());
    };

    void setupTempMode(FMyMJDataStructCpp* pDataExtTempMode)
    {
        MY_VERIFY(m_eWorkMode == MyMJGameElemWorkModeCpp::Invalid);
        m_eAccessRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        m_eWorkMode = MyMJGameElemWorkModeCpp::Temp;
        m_pDataExtTempMode = pDataExtTempMode;
    };

    //don't store the return, it can be invalid anytime
    const FMyMJCoreDataPublicCpp& getCoreDataRefConst() const
    {
        if (m_eWorkMode == MyMJGameElemWorkModeCpp::Full) {
            return m_pDataFullMode->getCoreDataRefConst();
        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Mirror) {

        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Temp) {

            MY_VERIFY(m_pDataExtTempMode);
            return m_pDataExtTempMode->getCoreDataRefConst();

        }
        else {
            MY_VERIFY(false);
        }

        return *(FMyMJCoreDataPublicCpp *)NULL;
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime
    const FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRefConst(int32 idxAttender) const
    {
        if (m_eWorkMode == MyMJGameElemWorkModeCpp::Full) {
            return m_pDataFullMode->getRoleDataAttenderPublicRefConst(idxAttender);
        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Mirror) {


        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Temp) {

            MY_VERIFY(m_pDataExtTempMode);
            return m_pDataExtTempMode->getRoleDataAttenderPublicRefConst(idxAttender);

        }
        else {
            MY_VERIFY(false);
        }

        return *(FMyMJRoleDataAttenderPublicCpp *)NULL;
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime, can return NULL
    const FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivateConst(int32 idxAttender) const
    {
        if (m_eWorkMode == MyMJGameElemWorkModeCpp::Full) {
            return &m_pDataFullMode->getRoleDataAttenderPrivateRefConst(idxAttender);
        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Mirror) {


        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Temp) {

            MY_VERIFY(m_pDataExtTempMode);
            return &m_pDataExtTempMode->getRoleDataAttenderPrivateRefConst(idxAttender);

        }
        else {
            MY_VERIFY(false);
        }

        return NULL;
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataPrivateCpp* getRoleDataPrivateConst(int32 idxRole) const
    {
        if (m_eWorkMode == MyMJGameElemWorkModeCpp::Full) {
            return &m_pDataFullMode->getRoleDataPrivateRefConst(idxRole);
        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Mirror) {


        }
        else if (m_eWorkMode == MyMJGameElemWorkModeCpp::Temp) {

            MY_VERIFY(m_pDataExtTempMode);
            return &m_pDataExtTempMode->getRoleDataPrivateRefConst(idxRole);

        }
        else {
            MY_VERIFY(false);
        }

        return NULL;
    };

    //Note all data will be overwritten, include default values, usually used in full mode, but also possible for mirror as one way to implement replay  
    void applyBase(const FMyMJDataStructCpp &base)
    {
        if (m_eWorkMode == MyMJGameElemWorkModeCpp::Mirror) {
            //now this is not supposed to happen unless low level replay method is used
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("apply base but mode is mirror, check it!"), );
        }

        getCoreDataRef() = base.getCoreDataRefConst();

        int32 l = (uint8)MyMJGameRoleTypeCpp::Max;
        for (int i = 0; i < l; i++) {

            if (i < 4)
            {
                getRoleDataAttenderPublicRef(i) = base.getRoleDataAttenderPublicRefConst(i);


                FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivSelf = getRoleDataAttenderPrivate(i);
                const FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivOther = &base.getRoleDataAttenderPrivateRefConst(i);
                if (pAttenderPrivSelf && pAttenderPrivOther) {
                    *pAttenderPrivSelf = *pAttenderPrivOther;
                }
                else {
                    //it is OK, but in a fine design this should be avoid
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's attender private data is NULL, self %p, dest %p."), i, pAttenderPrivSelf, pAttenderPrivOther);
                }

            }

            {
                FMyMJRoleDataPrivateCpp *pPrivSelf = getRoleDataPrivate(i);
                const FMyMJRoleDataPrivateCpp *pPrivOther = &base.getRoleDataPrivateRefConst(i);
                if (pPrivSelf && pPrivOther) {
                    *pPrivSelf = *pPrivOther;
                }
                else {
                    //it is OK, but in a fine design this should be avoid
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's private data is NULL, self %p, dest %p."), i, pPrivSelf, pPrivOther);
                }
            }
        }

        getCoreDataRef().m_cGameCfg.prepareForUse();
    }


    //the delta myst be applied by two step, and after step 0 it can be visualized, can be used by both full and mirror mode
    //after step 0 we have a chance for visualize, before calling next
    void applyDeltaStep0(const FMyMJDataDeltaCpp &delta);
    void applyDeltaStep1(const FMyMJDataDeltaCpp &delta);

    //direct to apply, don't leave a chance for visualize
    void applyDelta(const FMyMJDataDeltaCpp &delta);


    void applyPusherResult(const FMyMJGamePusherResultCpp& result)
    {
        if (result.m_aResultBase.Num() > 0) {
            applyBase(result.m_aResultBase[0]);
        }

        if (result.m_aResultDelta.Num() > 0) {
            applyDelta(result.m_aResultDelta[0]);
        }
    };

    void resetForNewActionLoop();

protected:

    inline FMyMJCoreDataPublicCpp& getCoreDataRef()
    {
        return const_cast<FMyMJCoreDataPublicCpp &>(getCoreDataRefConst());
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


    void moveCardFromOldPosi(int32 id);
    //@idxAttender can < 0
    void moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst);

    //the really storage we point to
    TSharedPtr<FMyMJDataStructCpp> m_pDataFullMode;

    FMyMJDataStructCpp* m_pDataExtTempMode; //not owning it

    MyMJGameRoleTypeCpp m_eAccessRoleType;
    MyMJGameElemWorkModeCpp m_eWorkMode;
};