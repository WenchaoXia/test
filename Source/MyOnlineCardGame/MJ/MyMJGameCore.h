// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Queue.h"

#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"
#include "MyMJGameCmd.h"

#include "MyMJGameAttender.h"

#include "MyMJGameResManager.h"


#include "MyMJGameData.h"

//#include "MyMJGameCore.generated.h"


//the resource group  used to prepare the core
//inqeueed elements must be allocated on heap
struct FMyMJGameIOGroupCpp
{
    //GENERATED_USTRUCT_BODY()

public:

    FMyMJGameIOGroupCpp() {
        reset();
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
        FMyMJGamePusherResultCpp *pPusherResult = NULL;
        while (m_cPusherResultQueue.Dequeue(pPusherResult)) {
            delete(pPusherResult);
        }
    };

    FMyMJGameIOGroupCpp m_aGroups[(uint8)MyMJGameRoleTypeCpp::Max];

    TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc> m_cPusherResultQueue;
};

//Base class ii used to ensure basic facility create/destory sequence
//Although we place some basic facilites here as bas class member, which ensured the dependence is OK in delete(), but for simple let's use smart pointer here
//Remember: sequence is 1 allocate all mem -> call member () -> call self ()
class FMyMJGameCoreBaseCpp
{
public:

    //Most are settings, so don't need reset() function
    FMyMJGameCoreBaseCpp()
    {
        m_pPusherIOFull = NULL;
        m_pCmdIO = NULL;

    };

    virtual ~FMyMJGameCoreBaseCpp()
    {

    };

    void initBase()
    {
        for (int i = 0; i < 4; i++) {
            MY_VERIFY(m_aAttendersAll[i].IsValid() == false);
            m_aAttendersAll[i] = MakeShareable<FMyMJGameAttenderCpp>(createAttender());
        }

        m_cDataAccessor.setupDataExt(&m_cData, true);

        resetDatasOwned();
    };


    virtual void resetDatasOwned()
    {
        m_cDataLogic.resetForNewGame();
    };

    TSharedPtr<FMyMJGameAttenderCpp> getAttenderByIdx(int32 idxAttender)
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
        TSharedPtr<FMyMJGameAttenderCpp> ret = m_aAttendersAll[idxAttender];

        return ret;
    };

    TSharedPtr<FMyMJGameAttenderCpp> getRealAttenderByIdx(int32 idxAttender, bool bVerifyIsReal = true)
    {
        TSharedPtr<FMyMJGameAttenderCpp> ret = getAttenderByIdx(idxAttender);

        bool bIsReal = ret->getIsRealAttender();
        if (bIsReal) {
            return ret;
        }
        else {
            if (bVerifyIsReal) {
                MY_VERIFY(false);
            }
            return NULL;
        }
    };

    inline const FMyMJDataAccessorCpp& getDataAccessorRefConst() const
    {
        return m_cDataAccessor;
    };

    inline
    const FMyMJCoreDataLogicOnlyCpp& getDataLogicRefConst() const
    {
        return m_cDataLogic;
    };

    inline
    const FMyMJCoreDataPublicCpp& getCoreDataPublicRefConst() const
    {
        return getDataAccessorRefConst().getCoreDataPublicRefConst();
    };

    inline const FMyMJCardInfoPackCpp& getCardInfoPackRefConst() const
    {
        return m_cDataAccessor.getCoreDataPublicRefConst().m_cCardInfoPack;
    };

    inline const FMyMJCardValuePackCpp& getCardValuePackOfSysKeeperRefConst() const
    {
        const FMyMJRoleDataPrivateCpp* pD = m_cDataAccessor.getRoleDataPrivateConst((uint8)MyMJGameRoleTypeCpp::SysKeeper);
        MY_VERIFY(pD);
        return pD->m_cCardValuePack;
    };


    //assert idxAttenderOfContainor is in place
    //assert idxAttenderBase is real attender, but it may now is not in game anymore
    int32 findIdxAttenderStillInGame(int32 idxAttenderBase, uint32 delta, bool bReverse);

    int32 genIdxAttenderStillInGameMaskOne(int32 idxAttender);
    int32 genIdxAttenderStillInGameMaskExceptOne(int32 idxAttender);
    int32 genIdxAttenderStillInGameMaskAll();



protected:

    inline FMyMJDataAccessorCpp& getDataAccessorRef()
    {
        return const_cast<FMyMJDataAccessorCpp &>(getDataAccessorRefConst());
    };

    //must allocate one attender on heap and return it, must be overwrite
    virtual FMyMJGameAttenderCpp* createAttender() = NULL;

    //Basic facilities
    TSharedPtr<FMyMJGameAttenderCpp> m_aAttendersAll[4]; //always 4, note this should be a fixed structure, means don't change it after init()
    TSharedPtr<FMyMJGamePusherIOComponentFullCpp>  m_pPusherIOFull; //only used in full mode
    TSharedPtr<FMyMJGameCmdIOComponentCpp> m_pCmdIO;

    //data
    FMyMJCoreDataLogicOnlyCpp m_cDataLogic;
    FMyMJDataStructCpp m_cData;
    FMyMJDataAccessorCpp m_cDataAccessor;

};

class FMyMJGameCoreActionLoopHelperDataCpp
{
public:

    FMyMJGameCoreActionLoopHelperDataCpp()
    {
        m_pPrevAction = NULL;
        m_pPostAction = NULL;
        m_iIdxAttenderMask = 0;
        m_bAllowSamePriAction = false;
        m_iIdxAttenderHavePriMax = 0;
        m_bHaveSetupData = false;
    };

    virtual ~FMyMJGameCoreActionLoopHelperDataCpp()
    {
        clear();
    };

    void clear()
    {
        if (m_pPrevAction) {
            delete(m_pPrevAction);
            m_pPrevAction = NULL;
        }
        if (m_pPostAction) {
            delete(m_pPostAction);
            m_pPostAction = NULL;
        }

        m_iIdxAttenderMask = 0;
        m_bAllowSamePriAction = false;
        m_iIdxAttenderHavePriMax = 0;
        m_bHaveSetupData = false;
    };

    bool getHaveSetupDataConst() const
    {
        return m_bHaveSetupData;
    };

    //Note @pPrevAction and @pPostAction will be taken ownership
    void setupDataForNextActionLoop(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, int32 iIdxAttenderMask, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax)
    {
        clear();
        m_pPrevAction = pPrevAction;
        m_pPostAction = pPostAction;
        m_iIdxAttenderMask = iIdxAttenderMask;
        m_bAllowSamePriAction = bAllowSamePriAction;
        m_iIdxAttenderHavePriMax = iIdxAttenderHavePriMax;
        m_bHaveSetupData = true;
    };

    FMyMJGameActionBaseCpp* takePrevAction()
    {
        FMyMJGameActionBaseCpp* ret = m_pPrevAction;
        m_pPrevAction = NULL;
        return ret;
    };

    FMyMJGameActionBaseCpp* takePostAction()
    {
        FMyMJGameActionBaseCpp* ret = m_pPostAction;
        m_pPostAction = NULL;
        return ret;
    };

    int32 m_iIdxAttenderMask;
    bool m_bAllowSamePriAction;
    int32 m_iIdxAttenderHavePriMax;

protected:
    FMyMJGameActionBaseCpp* m_pPrevAction;
    FMyMJGameActionBaseCpp* m_pPostAction;
    bool m_bHaveSetupData;
};


#define MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero 0x01
#define MyMJGameCoreTrivalConfigMaskShowPusherLog 0x02

//class FMyMJGameCoreCpp : public FMyMJGameCoreBaseCpp, public TSharedFromThis<FMyMJGameCoreCpp> //to save trouble, not use multiple inheritance

/*
 * The core have two mdes: full and mirror. Full mode have all data members, while mirror mode only have the data representing the state without logic only members.
 * this class is used for the full mode only. 
 * The dsign is: code is stateless, only pusher can change state
 * It works as one source generating data changes which can used to rebuild many mirrors. and it is not in thread safe(but works as produce->consume mode)
 * Should always be created on heap with SharedPtr created
 * Warn, since TArray use memcpy(), so don't use it with TSharedPtr to avoid operator =, the best way is to subclass TArray and write one support pointer,
 * but we don't have time and it may have issues with serilizetion, so let's leave this problem by not using UStruct which have = operator, but plain C++ class
 */
class FMyMJGameCoreCpp : public FMyMJGameCoreBaseCpp
{
public:

    FMyMJGameCoreCpp(int32 iSeed) : FMyMJGameCoreBaseCpp()
    {
        m_iTrivalConfigMask = 0;

        m_pActionCollector = NULL;
        m_pExtIOGroupAll = NULL;

        m_pResManager = MakeShareable<FMyMJGameResManager>(new FMyMJGameResManager(iSeed));

        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("[%s] inited with seed: %d"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameElemWorkModeCpp"), (uint8)eWorkMode), iSeed);
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("full core inited with seed: %d"), iSeed);
    };

    virtual ~FMyMJGameCoreCpp()
    {};

    inline MyMJGameRuleTypeCpp getRuleType() const
    {
        MyMJGameRuleTypeCpp eRuleType = m_cDataLogic.m_eRuleType;

        MyMJGameRuleTypeCpp eRuleType2 = m_cDataAccessor.getCoreDataPublicRefConst().m_cGameCfg.m_eRuleType;
        if (eRuleType2 != MyMJGameRuleTypeCpp::Invalid) {
            MY_VERIFY(eRuleType2 == eRuleType); //If set, they must equal
        }

        return eRuleType;
    };


    inline
    FMyMJGameResManager& getResManagerRef() 
    {
        MY_VERIFY(m_pResManager.IsValid());
        return *m_pResManager.Get();
    };

    inline FMyMJGamePusherIOComponentFullCpp& getPusherIOFullRef()
    {
        MY_VERIFY(m_pActionCollector.IsValid());
        TSharedPtr<FMyMJGamePusherIOComponentFullCpp> p = m_pActionCollector->getpPusherIO();
        MY_VERIFY(p.IsValid());
        return *p.Get();
    };

    //@pIOGroupAll owned by external, don't manage their lifecycle inside
    virtual void initFullMode(TWeakPtr<FMyMJGameCoreCpp> pSelf, FMyMJGameIOGroupAllCpp *pIOGroupAll)
    {
        initBase();
        
        int32 l;
        l = MY_GET_ARRAY_LEN(m_aAttendersAll);
        MY_VERIFY(l == 4);

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
        pPusherIOFull->init(&pIOGroupAll->m_cPusherResultQueue);
        m_pPusherIOFull = pPusherIOFull;

        m_pActionCollector = MakeShareable(new FMyMJGameActionCollectorCpp(pSelf));
        m_pActionCollector->init(pPusherIOFull);

        //setup cmd in and out path
        m_pCmdIO = MakeShareable(new FMyMJGameCmdIOComponentCpp());
        m_pCmdIO->init(ppCmdInputQueues, ppCmdOutputQueues, (uint8)MyMJGameRoleTypeCpp::Max);
    };

    void tryProgressInFullMode();
  

    //@idxBase can be a empty slot, will return the first one found valid, return < 0 means no one found
    int32 getIdxOfUntakenSlotHavingCard(int32 idxBase, uint32 delta, bool bReverse);

    //This function will assert if not enough cards collected
    void collectCardsFromUntakenSlot(int32 idxBase, uint32 len, bool bReverse, TArray<int32> &outIds);

    //This function will not assert if not enough cards collected
    void tryCollectCardsFromUntakenSlot(int32 idxBase, uint32 len, bool bReverse, TArray<int32> &outIds);

    void moveCardFromOldPosi(int32 id);
    void moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst);


    //config
    int32 m_iTrivalConfigMask;

protected:

    //return whether game have progress
    bool actionLoop();
    bool findAndApplyPushers();
    bool findAndHandleCmd();
    void resetForNewActionLoop();

    //following should be implemented by child class
    //start

    
    //optional implement

    //gen action choices
    virtual void genActionChoices();
    //return whether we need to verify unformation, it checks the input, and here we may apply some thing to logic for common pushers, such as moving cards
    virtual bool prevApplyPusherResult(const FMyMJGamePusherResultCpp &pusherResult);
    //called when both result and pusher itself applied
    virtual bool verifyDataUniformationAfterPusherAndResultApplied();


    //must implement

    //returned one should be allocated on heap
    virtual FMyMJGamePusherResultCpp* genPusherResultAsSysKeeper(const FMyMJGamePusherBaseCpp &pusher) = NULL;
    virtual void applyPusher(const FMyMJGamePusherBaseCpp &pusher) = NULL;
    virtual void handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp &cmd) = NULL;

    //end

    //Anything may change in subclass, should be defined as pointer, otherwise direct a member. we don't use pointer for only reason about destruction sequence
    //Basic facilities

    TSharedPtr<FMyMJGameActionCollectorCpp>    m_pActionCollector;
    FMyMJGameIOGroupAllCpp *m_pExtIOGroupAll; //not owned by this class, also some member is used by m_pActionCollector
    TSharedPtr<FMyMJGameResManager> m_pResManager;

    //helperActionLoopData
    FMyMJGameCoreActionLoopHelperDataCpp m_cActionLoopHelperData;
};
