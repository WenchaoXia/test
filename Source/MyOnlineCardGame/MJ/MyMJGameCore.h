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
        FMyMJGamePusherBaseCpp *pPusher = NULL;
        while (m_cPusherQueue.Dequeue(pPusher)) {
            delete(pPusher);
        }
    };

    FMyMJGameIOGroupCpp m_aGroups[(uint8)MyMJGameRoleTypeCpp::Max];

    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> m_cPusherQueue;
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

        m_cDataLogic.m_eWorkMode = eWorkMode;

        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("[%s] inited with seed: %d"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)eWorkMode), iSeed);
    };

    virtual ~FMyMJGameCoreCpp()
    {};

    inline MyMJGameCoreWorkModeCpp getWorkMode() const
    {
        return m_cDataLogic.m_eWorkMode;
    };

    inline MyMJGameRuleTypeCpp getRuleType() const
    {
        MyMJGameRuleTypeCpp eRuleType = m_cDataLogic.m_eRuleType;
        MyMJGameCoreWorkModeCpp eWorkMode = getWorkMode();

        if (eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataForMirrorMode.IsValid()) {
                MyMJGameRuleTypeCpp eRuleType2 = m_pDataForMirrorMode->m_cDataPubicDirect.m_cGameCfg.m_eRuleType;
                if (eRuleType2 != MyMJGameRuleTypeCpp::Invalid) {
                    MY_VERIFY(eRuleType2 == eRuleType); //If set, they must equal
                }
            }
        }
        else if (eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            if (m_pDataForFullMode.IsValid()) {
                MyMJGameRuleTypeCpp eRuleType2 = m_pDataForFullMode->m_cDataPubicDirect.m_cGameCfg.m_eRuleType;
                if (eRuleType2 != MyMJGameRuleTypeCpp::Invalid) {
                    MY_VERIFY(eRuleType2 == eRuleType); //If set, they must equal
                }
                //MY_VERIFY(m_pDataForFullMode->m_cDataPubicDirect.m_eRuleType == m_eRuleType);
            }
        }

        return eRuleType;
    };

    const FMyMJCoreDataLogicOnlyCpp& getDataLogicRef() const
    {
        return m_cDataLogic;
    };

    FMyMJCoreDataPublicDirectCpp* getDataPublicDirect()
    {
        MyMJGameCoreWorkModeCpp eWorkMode = getWorkMode();
        if (eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataForMirrorMode.IsValid()) {
                return &m_pDataForMirrorMode->m_cDataPubicDirect;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataForMirrorMode Invalid!"));  
                return NULL;
            }
        }
        else if (eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            MY_VERIFY(m_pDataForFullMode.IsValid());
            return &m_pDataForFullMode->m_cDataPubicDirect;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
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


    //@pIOGroupAll owned by external, don't manage their lifecycle inside
    virtual void initFullMode(TWeakPtr<FMyMJGameCoreCpp> pSelf, FMyMJGameIOGroupAllCpp *pIOGroupAll)
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("initFullMode."));

        MY_VERIFY(getWorkMode() == MyMJGameCoreWorkModeCpp::Full);
        initBase(pSelf);

        //setup data members
        MY_VERIFY(!m_pDataForFullMode.IsValid());
        //MY_VERIFY(m_pDataForMirrorMode == NULL); //don't do this, we may across thread

        m_pDataForFullMode = MakeShareable<FMyMJCoreDataForFullModeCpp>(new FMyMJCoreDataForFullModeCpp());
        m_pDataForFullMode->m_cDataPubicDirect.reinit(getRuleType());
 
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


    virtual void initMirrorMode(TWeakPtr<FMyMJGameCoreCpp> pSelf, UMyMJDataForMirrorModeCpp *pMJData)
    {

        MY_VERIFY(getWorkMode() == MyMJGameCoreWorkModeCpp::Mirror);
        
        initBase(pSelf);

        //MY_VERIFY(!m_pDataForFullMode.IsValid());
        MY_VERIFY(m_pDataForMirrorMode == NULL);

        m_pDataForMirrorMode = pMJData->m_pCoreData;
        MY_VERIFY(m_pDataForMirrorMode.IsValid());
        //m_pDataForMirrorMode->m_cDataPubicDirect.m_cGameCfg.m_eRuleType
        m_pDataForMirrorMode->m_cDataPubicDirect.reinit(getRuleType());

        int32 l;
        l = MY_GET_ARRAY_LEN(m_aAttendersAll);
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);

        MY_VERIFY(l == pMJData->m_aAttenderDatas.Num());

        for (int32 i = 0; i < l; i++) {
            m_aAttendersAll[i]->initMirrorMode(pSelf, i, pMJData->m_aAttenderDatas[i]->m_pDataPublic, pMJData->m_aAttenderDatas[i]->m_pDataPrivate);
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
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("[%s:%d:%d]: Applying: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)getWorkMode()), pCoreData->m_iActionGroupId, m_cDataLogic.m_iPusherIdLast, *pPusher->genDebugString());
        }
        applyPusher(pPusher);

        if (pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            //all data reseted when applyPusher(), we don't bother about it here
        }
        else {
            m_cDataLogic.m_iPusherIdLast++;
        }

        if (!(m_cDataLogic.m_iPusherIdLast == pPusher->getId())) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s, pusher [%d] id not equal: %d, %d."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCoreWorkModeCpp"), (uint8)getWorkMode()), (uint8)pPusher->getType(), m_cDataLogic.m_iPusherIdLast, pPusher->getId());
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
            iGameId = m_cDataLogic.m_iGameId;
            iPusherIdLast = m_cDataLogic.m_iPusherIdLast;
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
            m_aAttendersAll[i] = MakeShareable<FMyMJGameAttenderCpp>(createAttender(getWorkMode()));
        }
    };

    //data
    TSharedPtr<FMyMJCoreDataForFullModeCpp> m_pDataForFullMode;

    TWeakObjectPtr<UMyMJCoreDataForMirrorModeCpp> m_pDataForMirrorMode;

    FMyMJCoreDataLogicOnlyCpp m_cDataLogic;

    //Anything may change in subclass, should be defined as pointer, otherwise direct a member. we don't use pointer for only reason about destruction sequence
    //Basic facilities

    TSharedPtr<FMyMJGameActionCollectorCpp>    m_pActionCollector;
    FMyMJGameIOGroupAllCpp *m_pExtIOGroupAll; //not owned by this class, also some member is used by m_pActionCollector
    TSharedPtr<FMyMJGameResManager> m_pResManager;

};
