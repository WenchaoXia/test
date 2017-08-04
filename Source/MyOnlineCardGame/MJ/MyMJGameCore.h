// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Queue.h"

#include "Utils/MyMJUtils.h"
#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"
#include "MyMJGameCmd.h"

#include "MyMJGameAttender.h"

#include "MyMJGameResManager.h"


//#include "GameFramework/Actor.h"

#include "MyMJGameCore.generated.h"


UENUM()
enum class MyMJActionLoopStateCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    WaitingToGenAction = 1 UMETA(DisplayName = "WaitingToGenAction"),
    ActionGened = 2 UMETA(DisplayName = "ActionGened"),
    ActionCollected = 3 UMETA(DisplayName = "ActionCollected")
};


//the resource group  used to prepare the core
//inqeueed elements must be allocated on heap
struct FMyMJGameIOGroupCpp
{
    //GENERATED_USTRUCT_BODY()

public:

    FMyMJGameIOGroupCpp() {

    };

    virtual ~FMyMJGameIOGroupCpp() {

        reset();
    };

    void reset() {
        FMyMJGameCmdBaseCpp *pCmd;
        while (m_cCmdInputQueue.Dequeue(pCmd)) {
            delete(pCmd);
        }
        while (m_cCmdOutputQueue.Dequeue(pCmd)) {
            delete(pCmd);
        }

    };

    inline TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc>& getCmdInputQueue()
    {
        return m_cCmdInputQueue;
    };

    inline TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc>& getCmdOutputQueue()
    {
        return m_cCmdOutputQueue;
    };

protected:


    //foloowing is the bridges, the model all like : produce£º allocate on heap, prducer: take ownership
    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> m_cCmdInputQueue;
    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> m_cCmdOutputQueue;

    //FMyMJGamePusherPointersCpp m_cPusherBuffer;

};

struct FMyMJGameIOGroupAllCpp
{
public:
    FMyMJGameIOGroupAllCpp()
    {

    };

    virtual ~FMyMJGameIOGroupAllCpp()
    {
        FMyMJGamePusherBaseCpp *pPusher = NULL;
        while (m_cPusherQueue.Dequeue(pPusher)) {
            delete(pPusher);
        }
    };

    FMyMJGameIOGroupCpp m_aGroups[(uint8)MyMJGameRoleTypeCpp::Max];

    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> m_cPusherQueue;
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
        m_iIdxUntakenSlotLengthAtStart = 0;

        m_iUntakenSlotCardsLeftNumTotal = 0;
        m_iUntakenSlotCardsLeftNumKeptFromTail = 0;
        m_iUntakenSlotCardsLeftNumNormalFromHead = 0;
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
    int32 m_iIdxUntakenSlotHeadNow;
    int32 m_iIdxUntakenSlotTailNow;
    int32 m_iIdxUntakenSlotTailAtStart;
    int32 m_iIdxUntakenSlotLengthAtStart; //Note this never change in game
    int32 m_iUntakenSlotCardsLeftNumTotal; //simply reflect how many cards present on dest as untaken, doesn't mean how many you can take it which should be decided by rule
    int32 m_iUntakenSlotCardsLeftNumKeptFromTail;
    int32 m_iUntakenSlotCardsLeftNumNormalFromHead;

};


USTRUCT(BlueprintType)
struct FMyMJCoreDataPublicDirectCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreDataPublicDirectCpp()
    {
        reinit(MyMJGameRuleTypeCpp::Invalid);
    };

    virtual ~FMyMJCoreDataPublicDirectCpp()
    {

    };

    void reinit(MyMJGameRuleTypeCpp eRuleType)
    {
        m_eRuleType = eRuleType;
        reset();
    };

    void reset()
    {
        //some data does not need to reset since reset pusher will overwrite them all
        m_aUntakenCardStacks.Reset();
        m_cCardInfoPack.reset(0);

        m_cGameCfg.reset();
        m_cGameRunData.reset();

        m_iGameId = -1;
        m_iPusherIdLast = -1;
        m_iActionGroupId = -1;
        m_eGameState = MyMJGameStateCpp::Invalid;
        m_eActionLoopState = MyMJActionLoopStateCpp::Invalid;

        m_iDiceNumberNow0 = -1;
        m_iDiceNumberNow1 = -1;

        m_cUntakenSlotInfo.reset();

        m_aHelperLastCardsGivenOutOrWeave.Reset();
        m_cHelperLastCardTakenInGame.reset(true);
        m_cHelperShowedOut2AllCards.clear();

    };

    inline bool isInGameState()
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Id"))
    int32 m_iGameId;

    //the last pusher id we got
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Pusher Id Last"))
    int32 m_iPusherIdLast;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Group Id"))
    int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
    MyMJGameStateCpp m_eGameState;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Loop State"))
    MyMJActionLoopStateCpp m_eActionLoopState;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number 0"))
    int32 m_iDiceNumberNow0;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number 1"))
    int32 m_iDiceNumberNow1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Untaken Slot Info"))
    FMyMJGameUntakenSlotInfoCpp m_cUntakenSlotInfo;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hai Di Card Id"))
    FMyIdValuePair m_cHelperLastCardTakenInGame; //hai di card if id >= 0

    //used to calculate how many cards left possible hu
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Cards Showed Out to All"))
    FMyMJValueIdMapCpp m_cHelperShowedOut2AllCards;

    //used to tell what rule this core is fixed to, not game cfg's type can be invalid, which means not started yet
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Rule Type"))
    MyMJGameRuleTypeCpp m_eRuleType;

};

USTRUCT()
struct FMyMJCoreDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCoreDataPublicDirectCpp m_cDataDirectPubic;
};


UCLASS(NotBlueprintType, NotBlueprintable)
class UMyMJCoreDataForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:
    FMyMJCoreDataPublicDirectCpp m_cDataDirectPubic;
};

#define MY_GET_ARRAY_LEN(A) ( sizeof(A) / sizeof(A[0]))

//Base class ii used to ensure basic facility create/destory sequence
//Although we place some basic facilites here as bas class member, which ensured the dependence is OK in delete(), but for simple let's use smart pointer here
//Remember: sequence is 1 allocate all mem -> call member () -> call self ()
class FMyMJGameCoreBaseCpp
{
public:
    FMyMJGameCoreBaseCpp()
    {
        m_pPusherIOFull = NULL;
        m_pCmdIO = NULL;
        //m_pExtIOGroupAll = NULL;
    };

    virtual ~FMyMJGameCoreBaseCpp()
    {

    };

    TSharedPtr<FMyMJGameAttenderCpp> getAttenderByIdx(int32 idxAttender)
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
        TSharedPtr<FMyMJGameAttenderCpp> ret = m_aAttendersAll[idxAttender];

        return ret;
    };

    TSharedPtr<FMyMJGameAttenderCpp> getRealAttenderByIdx(int32 idxAttender)
    {
        TSharedPtr<FMyMJGameAttenderCpp> ret = getAttenderByIdx(idxAttender);
        if (!ret->getIsRealAttenderRef()) {
            MY_VERIFY(false);
        }

        return ret;
    };

    //assert idxAttenderOfContainor is in place
    //assert idxAttenderBase is real attender, but it may now is not in game anymore
    int32 findIdxAttenderStillInGame(int32 idxAttenderBase, uint32 delta, bool bReverse);

    int32 genIdxAttenderStillInGameMaskOne(int32 idxAttender);
    int32 genIdxAttenderStillInGameMaskExceptOne(int32 idxAttender);
    int32 genIdxAttenderStillInGameMaskAll();


protected:

    //Basic facilities
    TSharedPtr<FMyMJGameAttenderCpp> m_aAttendersAll[(uint8)MyMJGameRoleTypeCpp::Max]; //always 4, note this should be a fixed structure, means don't change it after init()
    TSharedPtr<FMyMJGamePusherIOComponentFullCpp>  m_pPusherIOFull; //only used in full mode
    TSharedPtr<FMyMJGameCmdIOComponentCpp> m_pCmdIO;

    //FMyMJGameIOGroupAllCpp *m_pExtIOGroupAll; //This is the fundermental IO resource, for simple, directly use it to process cmd
};

#define MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero 0x01
#define MyMJGameCoreTrivalConfigMaskShowPusherLog 0x02

/*
 * It works in two mode: full or mirror mode
 * The dsign is: code is stateless, only pusher can change state
 * It works as one source plus mutiple mirrors, not in thread safe mode(but works as produce->consume mode)
 * Should always be created on heap with SharedPtr created
 * Warn, since TArray use memcpy(), so don't use it with TSharedPtr to avoid operator =, the best way is to subclass TArray and write one support pointer,
 *but we don't have time and it may have issues with serilizetion, so let's leave this problem by not using UStruct, but plain C++ class
 */
//class FMyMJGameCoreCpp : public FMyMJGameCoreBaseCpp, public TSharedFromThis<FMyMJGameCoreCpp> //to save trouble, not use multiple inheritance
class FMyMJGameCoreCpp : public FMyMJGameCoreBaseCpp
{
public:

    FMyMJGameCoreCpp(MyMJGameCoreWorkModeCpp eWorkMode, int32 iSeed) : FMyMJGameCoreBaseCpp()
    {
        m_iTrivalConfigMask = 0;

        m_pDataForFullMode = NULL;
        m_pDataForMirrorMode = NULL;

        m_pActionCollector = NULL;
        m_pExtIOGroupAll = NULL;

        m_pResManager = MakeShareable<FMyMJGameResManager>(new FMyMJGameResManager(iSeed));

        m_iMsLast = 0;

        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
        m_eWorkMode = eWorkMode;

        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("[%s] inited with seed: %d"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)m_eWorkMode), iSeed);
    };

    virtual ~FMyMJGameCoreCpp()
    {};

    FMyMJCoreDataPublicDirectCpp* getDataPublicDirect()
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataForMirrorMode.IsValid()) {
                return &m_pDataForMirrorMode->m_cDataDirectPubic;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataForMirrorMode Invalid!"));  
                return NULL;
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            MY_VERIFY(m_pDataForFullMode.IsValid());
            return &m_pDataForFullMode->m_cDataDirectPubic;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    inline MyMJGameRuleTypeCpp getRuleType() const
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataForMirrorMode.IsValid()) {
                MY_VERIFY(m_pDataForMirrorMode->m_cDataDirectPubic.m_eRuleType == m_eRuleType);
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            if (m_pDataForFullMode.IsValid()) {
                MY_VERIFY(m_pDataForFullMode->m_cDataDirectPubic.m_eRuleType == m_eRuleType);
            }
        }

        return m_eRuleType;
    };

    inline
    FMyMJCardInfoPackCpp& getCardInfoPack()
    {
        FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
        MY_VERIFY(pD);
        return pD->m_cCardInfoPack;
    };

    inline
    FMyMJCardValuePackCpp& getCardValuePackOfSys()
    {
        TSharedPtr<FMyMJGameAttenderCpp> &pAttender = m_aAttendersAll[(uint8)MyMJGameRoleTypeCpp::SysKeeper];
        MY_VERIFY(pAttender.IsValid());
        FMyMJAttenderDataPrivateDirectForBPCpp *pDPriD = pAttender->getDataPrivateDirect();
        MY_VERIFY(pDPriD);
        return pDPriD->m_cCardValuePack;
    }


    inline
    TSharedPtr<FMyMJGameResManager> getpResManager()
    {
        return m_pResManager;
    };


    inline TSharedPtr<FMyMJGamePusherIOComponentFullCpp> getpPusherIOFull()
    {
        MY_VERIFY(m_pActionCollector.IsValid());
        return m_pActionCollector->getpPusherIO();
    };

    inline MyMJGameCoreWorkModeCpp getWorkMode()
    {
        return m_eWorkMode;
    }

    //@pIOGroupAll owned by external, don't manage their lifecycle inside
    virtual void initFullMode(TWeakPtr<FMyMJGameCoreCpp> pSelf, FMyMJGameIOGroupAllCpp *pIOGroupAll)
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("initFullMode."));

        MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Full);
        initBase(pSelf);

        //setup data members
        MY_VERIFY(!m_pDataForFullMode.IsValid());
        //MY_VERIFY(m_pDataForMirrorMode == NULL); //don't do this, we may across thread

        m_pDataForFullMode = MakeShareable<FMyMJCoreDataForFullModeCpp>(new FMyMJCoreDataForFullModeCpp());
        m_pDataForFullMode->m_cDataDirectPubic.reinit(m_eRuleType);
 
        int32 l;
        l = MY_GET_ARRAY_LEN(m_aAttendersAll);
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);

        for (int32 i = 0; i < l; i++) {
            m_aAttendersAll[i]->initFullMode(pSelf, i);
        }


        TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> *ppCmdInputQueues[(uint8)MyMJGameRoleTypeCpp::Max], *ppCmdOutputQueues[(uint8)MyMJGameRoleTypeCpp::Max];

        for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            ppCmdInputQueues[i] = &pIOGroupAll->m_aGroups[i].getCmdInputQueue();
            ppCmdOutputQueues[i] = &pIOGroupAll->m_aGroups[i].getCmdOutputQueue();
        }

        TSharedPtr<FMyMJGamePusherIOComponentFullCpp>  pPusherIOFull = MakeShareable<FMyMJGamePusherIOComponentFullCpp>(new FMyMJGamePusherIOComponentFullCpp());

        //setup pusher output path, which will link to @ppPusherOutputQueues as remote outputs
        pPusherIOFull->init(&pIOGroupAll->m_cPusherQueue);
        m_pPusherIOFull = pPusherIOFull;


        m_pActionCollector = MakeShareable(new FMyMJGameActionCollectorCpp(pSelf));
        m_pActionCollector->init(pPusherIOFull);

        //setup cmd in and out path
        m_pCmdIO = MakeShareable(new FMyMJGameCmdIOComponentCpp());
        m_pCmdIO->init(ppCmdInputQueues, ppCmdOutputQueues, (uint8)MyMJGameRoleTypeCpp::Max);
    };


    virtual void initMirrorMode(TWeakPtr<FMyMJGameCoreCpp> pSelf, UMyMJCoreDataForMirrorModeCpp* pCoreData, TArray<UMyMJAttenderDataPublicForMirrorModeCpp *> &apAttenderDataPublic, TArray<UMyMJAttenderDataPrivateForMirrorModeCpp *> &apAttenderDataPrivate)
    {

        MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror);
        
        initBase(pSelf);

        //MY_VERIFY(!m_pDataForFullMode.IsValid());
        MY_VERIFY(m_pDataForMirrorMode == NULL);

        m_pDataForMirrorMode = pCoreData;
        m_pDataForMirrorMode->m_cDataDirectPubic.reinit(m_eRuleType);

        int32 l;
        l = MY_GET_ARRAY_LEN(m_aAttendersAll);
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);

        MY_VERIFY(l == apAttenderDataPublic.Num());
        MY_VERIFY(l == apAttenderDataPrivate.Num());

        for (int32 i = 0; i < l; i++) {
            m_aAttendersAll[i]->initMirrorMode(pSelf, i, apAttenderDataPublic[i], apAttenderDataPrivate[i]);
        }
    };

    //call this only in full mode
    void tryProgressInFullMode();

    //call this both in mirror mode, and let caller handle the cycle
    inline
    void makeProgressByPusher(FMyMJGamePusherBaseCpp *pPusher)
    {
        FMyMJCoreDataPublicDirectCpp* pCoreData = getDataPublicDirect();
        if (pCoreData == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCoreData is invalid!"));
            return;
        }

        /* this should be done by apply pusher
        if (pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            pCoreData->m_iGameId = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher)->m_iGameId;
            pCoreData->m_iPusherIdLast = -1;
        }
        */

        if ((m_iTrivalConfigMask & MyMJGameCoreTrivalConfigMaskShowPusherLog) > 0) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("[%s:%d:%d]: Applying: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)m_eWorkMode), pCoreData->m_iActionGroupId, pCoreData->m_iPusherIdLast, *pPusher->genDebugString());
        }
        applyPusher(pPusher);

        if (pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            //all data reseted when applyPusher(), we don't bother about it here
        }
        else {
            pCoreData->m_iPusherIdLast++;
        }

        if (!(pCoreData->m_iPusherIdLast == pPusher->getId())) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s, pusher [%d] id not equal: %d, %d."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)m_eWorkMode), (uint8)pPusher->getType(), pCoreData->m_iPusherIdLast, pPusher->getId());
            MY_VERIFY(false);
        }
    };

    inline
    void getGameIdAndPusherIdLast(int32 *pOutGameId, int32 *pOutPusherIdLast)
    {
        int32 iGameId = -1, iPusherIdLast = -1;
        FMyMJCoreDataPublicDirectCpp* pCoreData = getDataPublicDirect();
        if (pCoreData == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCoreData is invalid!"));
        }
        else {
            iGameId = pCoreData->m_iGameId;
            iPusherIdLast = pCoreData->m_iPusherIdLast;
        }

        if (pOutGameId) {
            *pOutGameId = iGameId;
        }
        if (*pOutPusherIdLast) {
            *pOutPusherIdLast = iPusherIdLast;
        }
    };

    //@idxBase can be a empty slot, will return the first one found valid, return < 0 means no one found
    int32 getIdxOfUntakenSlotHavingCard(int32 idxBase, uint32 delta, bool bReverse);

    //This function will assert if not enough cards collected
    void collectCardsFromUntakenSlot(int32 idxBase, uint32 len, bool bReverse, TArray<int32> &outIds);

    //This function will not assert if not enough cards collected
    void tryCollectCardsFromUntakenSlot(int32 idxBase, uint32 len, bool bReverse, TArray<int32> &outIds);

    void moveCardFromOldPosi(int32 id);
    void moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst);
    void updateUntakenInfoHeadOrTail(bool bUpdateHead, bool bUpdateTail);


    //config
    int32 m_iTrivalConfigMask;

protected:

    //return whether game have progress
    bool actionLoop();
    bool findAndApplyPushers();
    bool findAndHandleCmd();
    void genActionChoices();
    void resetForNewLoop(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, int32 iIdxAttenderMask, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax);

    //following should be implemented by child class
    //start

    //must allocate one attender on heap and return it
    virtual FMyMJGameAttenderCpp* createAttender(MyMJGameCoreWorkModeCpp eWorkMode) = NULL;
    virtual void applyPusher(FMyMJGamePusherBaseCpp *pPusher) = NULL;
    virtual void handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp *pCmd) = NULL;

    //end


    int32 calcUntakenSlotCardsLeftNumKeptFromTail();

    bool isIdxUntakenSlotInKeptFromTailSegment(int32 idx);


    inline
    void initBase(TWeakPtr<FMyMJGameCoreCpp> pSelf)
    {
        for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            MY_VERIFY(m_aAttendersAll[i].IsValid() == false);
            m_aAttendersAll[i] = MakeShareable<FMyMJGameAttenderCpp>(createAttender(m_eWorkMode));
        }
    };

    //data
    TSharedPtr<FMyMJCoreDataForFullModeCpp> m_pDataForFullMode;

    TWeakObjectPtr<UMyMJCoreDataForMirrorModeCpp> m_pDataForMirrorMode;


    //Anything may change in subclass, should be defined as pointer, otherwise direct a member. we don't use pointer for only reason about destruction sequence
    //Basic facilities

    TSharedPtr<FMyMJGameActionCollectorCpp>    m_pActionCollector;
    FMyMJGameIOGroupAllCpp *m_pExtIOGroupAll; //not owned by this class, also some member is used by m_pActionCollector
    TSharedPtr<FMyMJGameResManager> m_pResManager;

    int64 m_iMsLast;

    MyMJGameRuleTypeCpp m_eRuleType;//also distinguish sub type
    MyMJGameCoreWorkModeCpp m_eWorkMode;

};
