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
#include "MyMJGameCmd.h"
#include "MyMJGameAttender.h"

#include "MyMJGameResManager.h"

#include "MyMJGameCore.generated.h"


UENUM()
enum class MyMJActionLoopStateCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    WaitingToGenAction = 1 UMETA(DisplayName = "WaitingToGenAction"),
    ActionGened = 2 UMETA(DisplayName = "ActionGened"),
    ActionCollected = 3 UMETA(DisplayName = "ActionCollected")
};

UENUM(BlueprintType)
enum class MyMJGameCoreWorkModeCpp : uint8
{
    Full = 0     UMETA(DisplayName = "Full"), //Full Function mode
    Mirror = 1   UMETA(DisplayName = "Mirror"), //Mirror mode, doesn't produce any thing, just consume the pushers
    //MirrorAlone = 2      UMETA(DisplayName = "MirrorAlone")
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
        FMyMJGamePusherBaseCpp *pPusher;
        while (m_cCmdInputQueue.Dequeue(pCmd)) {
            delete(pCmd);
        }
        while (m_cCmdOutputQueue.Dequeue(pCmd)) {
            delete(pCmd);
        }
        while (m_cPusherOutputQueue.Dequeue(pPusher)) {
            delete(pPusher);
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

    inline TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>& getPusherOutputQueue()
    {
        return m_cPusherOutputQueue;
    };

    /*
    inline FMyMJGamePusherPointersCpp& getPusherBuffer()
    {
        return m_cPusherBuffer;
    };
    */

protected:


    //foloowing is the bridges, the model all like : produce£º allocate on heap, prducer: take ownership
    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> m_cCmdInputQueue;
    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> m_cCmdOutputQueue;

    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> m_cPusherOutputQueue;

    //FMyMJGamePusherPointersCpp m_cPusherBuffer;

};

struct FMyMJGameIOGroupAllCpp
{
public:
    FMyMJGameIOGroupCpp m_aGroups[(uint8)MyMJGameRoleTypeCpp::Max];
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

//Base class ii used to ensure basic facility create/destory sequence
//Although we place some basic facilites here as bas class member, which ensured the dependence is OK in delete(), but for simple let's use smart pointer here
//Remember: sequence is 1 allocate all mem -> call member () -> call self ()
class FMyMJGameCoreBaseCpp
{
public:
    FMyMJGameCoreBaseCpp()
    {
        m_pPusherIO = NULL;
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
        if (!ret->getISRealAttender()) {
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
    TSharedPtr<FMyMJGameAttenderCpp> m_aAttendersAll[4]; //always 4, note this should be a fixed structure, means don't change it after init()
    TSharedPtr<FMyMJGamePusherIOComponentCpp>  m_pPusherIO; //IO resource orgnize for pusher
    TSharedPtr<FMyMJGameCmdIOComponentCpp> m_pCmdIO;

    //FMyMJGameIOGroupAllCpp *m_pExtIOGroupAll; //This is the fundermental IO resource, for simple, directly use it to process cmd
};

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
        m_iActionGroupId = 0;
        m_iGameId = 0;
        m_iPusherId = 0;
        //m_iTurnId = 0;
        m_eGameState = MyMJGameStateCpp::Invalid;
        m_eActionLoopState = MyMJActionLoopStateCpp::Invalid;
        m_aHelperLastCardsGivenOutOrWeave.Reset();
        m_iMsLast = 0;

        m_iDiceNumberNow0 = -1;
        m_iDiceNumberNow1 = -1;


        m_pResManager = MakeShareable<FMyMJGameResManager>(new FMyMJGameResManager(iSeed));
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
        m_eWorkMode = eWorkMode;
    };

    virtual ~FMyMJGameCoreCpp()
    {};

    inline MyMJGameRuleTypeCpp getRuleType() const
    {
        if (m_pGameCfg.IsValid()) {
            MY_VERIFY(m_pGameCfg->m_eRuleType == m_eRuleType);
        }
        return m_eRuleType;
    };

    inline
    FMyMJCardPackCpp *getpCardPack()
    {
        return &m_cCardPack;
    };

    inline int32 getGameId() const
    {
        return m_iGameId;
    };

    inline int32 getPusherId() const
    {
        return m_iPusherId;
    };

    inline
    int32 getActionGroupId() const
    {
        return m_iActionGroupId;
    };

    inline
    MyMJGameStateCpp* getpGameState()
    {
        return &m_eGameState;
    };

    inline
    TSharedPtr<FMyMJGameResManager> getpResManager()
    {
        return m_pResManager;
    };

    const FMyMJGameUntakenSlotInfoCpp& getUntakenSlotInfoRef() const
    {
        return m_cUntakenSlotInfo;
    };


    inline TSharedPtr<FMyMJGameCfgCpp> getpGameCfg()
    {
        return m_pGameCfg;
    };

    inline TSharedPtr<FMyMJGameRunDataCpp> getpGameRunData()
    {
        return m_pGameRunData;
    };


    inline bool isInGameState()
    {
        return (!(m_eGameState == MyMJGameStateCpp::Invalid || m_eGameState == MyMJGameStateCpp::GameEnd));
    };

    inline const TArray<FMyIdValuePair>& getHelperLastCardsGivenOutOrWeaveRef() const
    {
        return m_aHelperLastCardsGivenOutOrWeave;
    };

    inline const FMyIdValuePair& getHelperLastCardTakenInGameRef() const
    {
        return m_cHelperLastCardTakenInGame;
    };

    inline const FMyMJValueIdMapCpp& getHelperShowedOut2AllCardsRef() const
    {
        return m_cHelperShowedOut2AllCards;
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
        MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Full);
        initBase(pSelf);

        TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> *ppPusherOutputQueues[(uint8)MyMJGameRoleTypeCpp::Max];
        TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> *ppCmdInputQueues[(uint8)MyMJGameRoleTypeCpp::Max], *ppCmdOutputQueues[(uint8)MyMJGameRoleTypeCpp::Max];

        for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            ppPusherOutputQueues[i] = &pIOGroupAll->m_aGroups[i].getPusherOutputQueue();
            ppCmdInputQueues[i] = &pIOGroupAll->m_aGroups[i].getCmdInputQueue();
            ppCmdOutputQueues[i] = &pIOGroupAll->m_aGroups[i].getCmdOutputQueue();
        }

        TSharedPtr<FMyMJGamePusherIOComponentFullCpp> pPusherIO = MakeShareable(new FMyMJGamePusherIOComponentFullCpp());

        //setup pusher output path, which will link to @ppPusherOutputQueues as remote outputs
        pPusherIO->init(ppPusherOutputQueues, (uint8)MyMJGameRoleTypeCpp::Max);
        m_pPusherIO = pPusherIO;


        m_pActionCollector = MakeShareable(new FMyMJGameActionCollectorCpp(pSelf));
        m_pActionCollector->init(pPusherIO);

        //setup cmd in and out path
        m_pCmdIO = MakeShareable(new FMyMJGameCmdIOComponentCpp());
        m_pCmdIO->init(ppCmdInputQueues, ppCmdOutputQueues, (uint8)MyMJGameRoleTypeCpp::Max);
    };


    virtual void initMirrorMode(TWeakPtr<FMyMJGameCoreCpp> pSelf)
    {
        MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror);
        initBase(pSelf);

        m_pActionCollector = MakeShareable(new FMyMJGameActionCollectorCpp(pSelf));
        m_pActionCollector->initInMirrorMode();
    };

    //call this only in full mode
    void tryProgressInFullMode();

    //call this both in mirror mode, and let caller handle the cycle
    inline
    void makeProgressByPusher(FMyMJGamePusherBaseCpp *pPusher)
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("[%s:%d:%d]: Applying: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)m_eWorkMode), m_iActionGroupId, m_iPusherId, *pPusher->genDebugString());
        applyPusher(pPusher);
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

protected:

    //return whether game have progress
    bool actionLoop();
    bool findAndApplyPushers();
    bool findAndHandleCmd();


    //following should be implemented by child class
    //start

    //must allocate one attender on heap and return it
    virtual FMyMJGameAttenderCpp* createAndInitAttender(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx) = NULL;
    virtual void applyPusher(FMyMJGamePusherBaseCpp *pPusher) = NULL;
    virtual void handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp *pCmd) = NULL;

    //end


    int32 calcUntakenSlotCardsLeftNumKeptFromTail();

    bool isIdxUntakenSlotInKeptFromTailSegment(int32 idx);


    inline
    void initBase(TWeakPtr<FMyMJGameCoreCpp> pSelf)
    {
        MY_VERIFY(m_pGameCfg.IsValid() == false);

        m_pGameCfg = MakeShared<FMyMJGameCfgCpp>();
        m_pGameRunData = MakeShared<FMyMJGameRunDataCpp>();

        for (int i = 0; i < 4; i++) {
            MY_VERIFY(m_aAttendersAll[i].IsValid() == false);
            //m_aAttendersAll[i] = MakeShareable<FMyMJGameAttenderCpp>(new FMyMJGameAttenderCpp());
            m_aAttendersAll[i] = MakeShareable<FMyMJGameAttenderCpp>(createAndInitAttender(pSelf, i));

        }
    };


    //Anything may change in subclass, should be defined as pointer, otherwise direct a member. we don't use pointer for only reason about destruction sequence
    //Basic facilities

    TSharedPtr<FMyMJGameActionCollectorCpp>    m_pActionCollector;
    FMyMJGameIOGroupAllCpp *m_pExtIOGroupAll; //not owned by this class, also some member is used by m_pActionCollector

    TArray<FMyIdCollectionCpp> m_aUntakenCardStacks; //Always start from attender 0 to 3
    FMyMJCardPackCpp m_cCardPack;

    //Cfg
    TSharedPtr<FMyMJGameCfgCpp> m_pGameCfg;
    TSharedPtr<FMyMJGameRunDataCpp> m_pGameRunData;

    //Running state
    //Model is pusher->action->turn
    int32 m_iGameId;
    int32 m_iPusherId;
    int32 m_iActionGroupId;
    //int32 m_iTurnId;
    MyMJGameStateCpp m_eGameState;
    MyMJActionLoopStateCpp m_eActionLoopState;
    int64 m_iMsLast;

    int32 m_iDiceNumberNow0;
    int32 m_iDiceNumberNow1;
    FMyMJGameUntakenSlotInfoCpp m_cUntakenSlotInfo;

    TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave
    FMyIdValuePair m_cHelperLastCardTakenInGame;
    FMyMJValueIdMapCpp m_cHelperShowedOut2AllCards; //used to calculate how many cards left possible hu


    TSharedPtr<FMyMJGameResManager> m_pResManager;
    MyMJGameRuleTypeCpp m_eRuleType;//also distinguish sub type
    MyMJGameCoreWorkModeCpp m_eWorkMode;
};
