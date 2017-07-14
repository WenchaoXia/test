// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "MJ/MyMJCommonDefines.h"

#include "Utils/MyMJUtils.h"
#include "MyMJCardPack.h"

#include "MyMJGamePusher.h"

#include "MyMJGamePushersIO.generated.h"


//contains 1 local queue, 1 remote queue, and all can only have one consumer
USTRUCT()
struct FMyMJGamePushersIOCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePushersIOCpp()
    {
        m_pQueueLocal = NULL;
        m_pQueueRemote = NULL;
        m_eDataRoleType = MyMJGameRoleTypeCpp::Max;
    };

    virtual ~FMyMJGamePushersIOCpp()
    {
        reset();
        if (m_pQueueLocal) {
            delete(m_pQueueLocal);
            m_pQueueLocal = NULL;
        }

        if (m_pQueueRemote) {
            //not touching it
            m_pQueueRemote = NULL;
        }
    };

    void reset()
    {
        //clear what it owns, here it is the local queue that can work in produce mode
        if (m_pQueueLocal) {
            FMyMJGamePusherBaseCpp *pRet = NULL;
            while (m_pQueueLocal->Dequeue(pRet)) {
                delete(pRet);
            };
        }
    };

    void init(MyMJGameRoleTypeCpp eDataType, bool bEnableLocal, TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> *pQueueRemoteThread)
    {
        m_eDataRoleType = eDataType;

        if (bEnableLocal) {
            m_pQueueLocal = new TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>();
        }

        if (pQueueRemoteThread) {
            m_pQueueRemote = pQueueRemoteThread;
        }
    };

    //Can only be called in local thread, it copies data and enqueue
    inline
    void Enqueue(const FMyMJGamePusherBaseCpp &pusher)
    {
        FMyMJGamePusherBaseCpp *pNew;
        if (m_pQueueLocal) {
            pNew = pusher.cloneDeepWithRoleType(m_eDataRoleType);
            if (pNew) {
                m_pQueueLocal->Enqueue(pNew);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to clone, pusher type %s, roleType %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pusher.getType()), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRoleTypeCpp"), (uint8)m_eDataRoleType));
                MY_VERIFY(false);
            }
        }

        if (m_pQueueRemote) {
            pNew = pusher.cloneDeepWithRoleType(m_eDataRoleType);
            if (pNew) {
                m_pQueueRemote->Enqueue(pNew);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to clone, pusher type %s, roleType %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pusher.getType()), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRoleTypeCpp"), (uint8)m_eDataRoleType));
                MY_VERIFY(false);
            }
        }

    };

    inline
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* getQueueLocal()
    {
        return m_pQueueLocal;
    };


protected:

    MyMJGameRoleTypeCpp m_eDataRoleType;

    //owned by this class, can only be used in one thread, so it is safe to clear by either producer or consumer
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> *m_pQueueLocal;

    //owned by other, never managed by this class
    //Note, only consume thread can clear it, and this is always created by main thread, destroy by main thread. destroy only happen after sub thread stopped/killed, after then, parent thread can clear() also
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> *m_pQueueRemote;

};


USTRUCT()
struct FMyMJGamePushersIOOutputsCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePushersIOOutputsCpp()
    {

    };

    virtual ~FMyMJGamePushersIOOutputsCpp()
    {

    };

    void reset()
    {
        int32 l = m_aIOs.Num();
        for (int32 i = 0; i < l; i++) {
            m_aIOs[i].reset();
        }
    };

    void init(TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> **ppRemoteQueues, int32 iRemoteQueuesCount)
    {
        m_aIOs.Empty();

        for (uint8 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            int32 idx = m_aIOs.Emplace();
            MY_VERIFY(i == idx);
            FMyMJGamePushersIOCpp &IO = m_aIOs[i];

            MyMJGameRoleTypeCpp eDataType = (MyMJGameRoleTypeCpp)i;
            bool bEnableLocal = false;
            if (eDataType == MyMJGameRoleTypeCpp::SysKeeper) {
                bEnableLocal = true;
            }

            TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> *pRemoteQueue = NULL;
            if (i < iRemoteQueuesCount) {
                pRemoteQueue = ppRemoteQueues[i];
            }

            IO.init(eDataType, bEnableLocal, pRemoteQueue);
        }

    };

    inline
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* getSysKeeperQueueLocal()
    {
        TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> *pRet = m_aIOs[(uint8)MyMJGameRoleTypeCpp::SysKeeper].getQueueLocal();
        MY_VERIFY(pRet != NULL);
        return pRet;
    };

    inline
    void Enqueue(const FMyMJGamePusherBaseCpp &pusher)
    {
        int l = m_aIOs.Num();
        for (int32 i = 0; i < l; i++) {
            m_aIOs[i].Enqueue(pusher);
        }
    };

protected:
    TArray<FMyMJGamePushersIOCpp> m_aIOs;
};

//one input queue, multiple output queue, which will be part of the core
class FMyMJGamePusherIOComponentCpp
{
public:
    FMyMJGamePusherIOComponentCpp()
    {

    };

    virtual ~FMyMJGamePusherIOComponentCpp()
    {

    };

    //this may delete pushers, so don't call it when game reset
    //virtual void reset() = NULL;

    //warn: when caller destruct the shared_ptr, it is possible make data deleted, so keep one sharedPtr while using it!
    virtual TSharedPtr<FMyMJGamePusherBaseCpp> tryPullPusher(int32 iGameId, int32 iPusherId) = NULL;

    virtual void EnqueuePusher(FMyMJGamePusherBaseCpp &pusher) = NULL;
};

//both producer and consumer, consume itself, and produce to external unit
class FMyMJGamePusherIOComponentFullCpp : public FMyMJGamePusherIOComponentCpp
{

public:
    FMyMJGamePusherIOComponentFullCpp() : FMyMJGamePusherIOComponentCpp()
    {
        m_pInputQueueBridged = NULL;
        m_iPusherCount = 0;
    };

    virtual ~FMyMJGamePusherIOComponentFullCpp()
    {

    };

    void init(TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> **ppRemoteQueues, int32 iRemoteQueuesCount)
    {
        m_cOutputs.init(ppRemoteQueues, iRemoteQueuesCount);
        m_pInputQueueBridged = m_cOutputs.getSysKeeperQueueLocal();

    };

    //virtual void reset() override
    //{
       //m_cOutputs.reset();
       // m_iPusherCount = 0;
    //};

    virtual TSharedPtr<FMyMJGamePusherBaseCpp> tryPullPusher(int32 iGameId, int32 iPusherId) override;

    virtual void EnqueuePusher(FMyMJGamePusherBaseCpp &pusher) override
    {
        if (pusher.getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            m_iPusherCount = 0;
        }

        pusher.m_iId = m_iPusherCount;
        m_cOutputs.Enqueue(pusher);

        m_iPusherCount++;
    };

protected:

    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* m_pInputQueueBridged; //Don't create it, instead point it to another created one
    FMyMJGamePushersIOOutputsCpp m_cOutputs;

    int32 m_iPusherCount;

};

/*
typedef class FMyMJGamePusherIOComponentConsumerCpp FMyMJGamePusherIOComponentConsumerCpp;

//for safe we should build a link class, but for sumple we simpley do all the recusive check only in buffer class
class FMyMJGamePusherIOBuffer
{
public:
    FMyMJGamePusherIOBuffer()
    {
        m_pProducerPrev = NULL;
        m_pConsumerNext = NULL;
    };

    virtual ~FMyMJGamePusherIOBuffer()
    {
        MY_VERIFY(m_pProducerPrev == NULL);
        cleanUp();
    };

    void cleanUp()
    {
        checkAndChangeProducerPrev(NULL);
        checkAndChangeConsumerNext(NULL);
    };

    void checkAndChangeProducerPrev(FMyMJGameIOGroupCpp *pProducerPrev);

    void checkAndChangeConsumerNext(FMyMJGamePusherIOComponentConsumerCpp *pConsumerNext);

    void trySyncBufferFromPrev();

    TSharedPtr<FMyMJGamePusherBaseCpp> tryPullPusherFromBuffer(int32 iGameId, int32 iPusherId);

protected:

    FMyMJGameIOGroupCpp *m_pProducerPrev;
    FMyMJGamePusherIOComponentConsumerCpp *m_pConsumerNext;


    //following is the buffer
    TArray<TSharedPtr<FMyMJGamePusherBaseCpp>> m_aPusherBuffer;
};

//used to pull data from external source, may across thread
class FMyMJGamePusherIOComponentConsumerCpp : public FMyMJGamePusherIOComponentCpp
{

public:
    FMyMJGamePusherIOComponentConsumerCpp() : FMyMJGamePusherIOComponentCpp()
    {
        m_pProducerPrev = NULL;
    };

    virtual ~FMyMJGamePusherIOComponentConsumerCpp()
    {

    };

    //virtual void reset() override
    //{
    //
    //};

    virtual TSharedPtr<FMyMJGamePusherBaseCpp> tryPullPusher(int32 iGameId, int32 iPusherId) override
    {
        if (m_pProducerPrev == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("tryPullPusher(), m_pProducerPrev is NULL!"));
            return NULL;
        }
        m_pProducerPrev->trySyncBufferFromPrev();
        return m_pProducerPrev->tryPullPusherFromBuffer(iGameId, iPusherId);
    };

    virtual void EnqueuePusher(FMyMJGamePusherBaseCpp &pusher) override
    {
        MY_VERIFY(false);
    };




protected:

    friend void FMyMJGamePusherIOBuffer::checkAndChangeConsumerNext(FMyMJGamePusherIOComponentConsumerCpp *pConsumerNext);

    void changeProducerPrev(FMyMJGamePusherIOBuffer *pProducerPrev)
    {
        if (m_pProducerPrev == pProducerPrev) {
            return;
        }

        m_pProducerPrev = pProducerPrev;
    };


    FMyMJGamePusherIOBuffer* m_pProducerPrev;
    //TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* m_pInputQueue;
};
*/

#define ActionCollectOK 0
#define ActionCollectEmpty 1
#define ActionCollectWaiting 2


class FMyMJGameActionContainorCpp
{

public:
    FMyMJGameActionContainorCpp(FMyMJGameAttenderCpp *pParentAttender)
    {
        m_pParentAttender = pParentAttender;
        reinit(false);
    };

    virtual ~FMyMJGameActionContainorCpp()
    {};


    void reinit(bool bRandomSelect)
    {
        m_bRandomSelect = bRandomSelect;
        resetForNewLoop();
    };

    void resetForNewLoop()
    {
        m_aActionChoices.Reset();
        m_iSelectionPushed = -1;
        m_aSubSelectionsPushed.Reset();
        m_bNeed2CollectPushed = false;

        m_iSelectionInputed = -1;
        m_aSubSelectionsInputed.Reset();

        m_iActionGroupId = -1;
        m_iPriorityMax = -1;
        m_bAlwaysCheckDistWhenCalcPri =  false;

        m_pSelected = NULL;

        m_bNeed2Collect = false;

    };

    inline
    bool &getNeed2Collect()
    {
        return m_bNeed2Collect;
    };

    inline
    FMyMJGameAttenderCpp *getpParentAttender()
    {
        return m_pParentAttender;
    };


    int32 getActionChoiceCount()
    {
        int32 totalCount = 0;
        int32 l = m_aActionChoices.Num();
        for (int32 i = 0; i < l; i++) {
            totalCount += m_aActionChoices[i]->getRealCountOfSelection();
        }

        return totalCount;
    };

    void fillInNewChoices(int32 iActionGroupId, TArray<TSharedPtr<FMyMJGameActionBaseCpp>> &actionChoices)
    {
        resetForNewLoop();
        m_iActionGroupId = iActionGroupId;
        m_aActionChoices = actionChoices;

        int l = m_aActionChoices.Num();
        for (int i = 0; i < l; i++) {
            int32 p = m_aActionChoices[i]->getPriority();
            if (m_iPriorityMax < p) {
                m_iPriorityMax = p;
                m_bAlwaysCheckDistWhenCalcPri = m_aActionChoices[i]->isAlwaysCheckDistWhenCalcPri();
            }
        }

        m_bNeed2Collect = true;
        m_bNeed2CollectPushed = true;
    };

    //called only in "full" core
    inline
    MyMJGameErrorCodeCpp makeSelection(int32 iActionGroupId, int32 iSelection)
    {
        TArray<int32> t;
        return makeSelection(iActionGroupId, iSelection, t);
    };

    //called only in "full" core, don't call dump in the code path in case of reject invalid request
    MyMJGameErrorCodeCpp makeSelection(int32 iActionGroupId, int32 iSelection, TArray<int32> &subSelections)
    {
        if (m_iActionGroupId != iActionGroupId) {
            return MyMJGameErrorCodeCpp::pusherIdNotEqual;
        }

        if (m_pSelected.Get() != NULL) {
            return MyMJGameErrorCodeCpp::choiceAlreadyMade;
        }

        int l = m_aActionChoices.Num();
        if (l <= 0) {
            return MyMJGameErrorCodeCpp::choicesEmpty;
        }

        if (!(iSelection >= 0 && iSelection < l)) {
            return MyMJGameErrorCodeCpp::choiceOutOfRange;
        }

        TSharedPtr<FMyMJGameActionBaseCpp> &sel = m_aActionChoices[iSelection];
        if (sel->getRealCountOfSelection() > 1) {
            //only call it when need decision
            if (0 != sel->makeSubSelection(subSelections)) {
                return MyMJGameErrorCodeCpp::choiceSubSelectInvalid;
            }
        }

        m_iSelectionInputed = iSelection;
        m_aSubSelectionsInputed = subSelections;

        m_pSelected = sel;
        return MyMJGameErrorCodeCpp::None;
    };

    void makeRandomSelection(FRandomStream *pRandomStream);

    //this code path core dump if met any invalid request
    void showSelectionOnNotify(int32 iActionGroupId, int32 iSelection, TArray<int32> &subSelections)
    {
        MY_VERIFY(m_iActionGroupId == iActionGroupId);

        int l = m_aActionChoices.Num();
        MY_VERIFY(iSelection >= 0 && iSelection < l);


        TSharedPtr<FMyMJGameActionBaseCpp> &sel = m_aActionChoices[iSelection];
        if (sel->getRealCountOfSelection() > 1) {
            if (!m_pSelected.IsValid()) {
                //not translated before(make selection not called), maybe this is a replay system, let's do it
                MY_VERIFY(0 == sel->makeSubSelection(subSelections));
            }
        };

        m_iSelectionPushed = iSelection;
        m_aSubSelectionsPushed = subSelections;
        m_bNeed2CollectPushed = false;

        if (!m_pSelected.IsValid()) {
            m_pSelected = sel;
        }
    }

    int32 collectAction(int64 iTimePassedMs, int32 &outPriorityMax, bool &outAlwaysCheckDistWhenCalcPri, TSharedPtr<FMyMJGameActionBaseCpp> &outPSelected, int32 &outSelection, TArray<int32> &outSubSelections);

    inline
    int32 getPriorityMax() {
        return m_iPriorityMax;
    };

    inline
    bool getAlwaysCheckDistWhenCalcPri() {
        return m_bAlwaysCheckDistWhenCalcPri;
    };

protected:
    //Also used to tip the state in UI
    TArray<TSharedPtr<FMyMJGameActionBaseCpp>> m_aActionChoices; //Todo: War
    int32 m_iSelectionPushed;
    TArray<int32> m_aSubSelectionsPushed;
    bool m_bNeed2CollectPushed; //can be used to tip whether waiting to choose in UI

    int32 m_iSelectionInputed; //It is the easiest way to save it like this, kick the core and let the core pick it
    TArray<int32> m_aSubSelectionsInputed;

    int32 m_iActionGroupId;
    int32 m_iPriorityMax; //the max pri action's property
    bool  m_bAlwaysCheckDistWhenCalcPri; //the max pri action's property

    TSharedPtr<FMyMJGameActionBaseCpp> m_pSelected;

    bool m_bNeed2Collect;

    bool m_bRandomSelect; //Mainly used for test and one stupid AI

    FMyMJGameAttenderCpp *m_pParentAttender;

};

//USTRUCT()
struct FMyMJGameActionCollectorCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionCollectorCpp(TWeakPtr<FMyMJGameCoreCpp> pCore)
    {
        m_pCore = pCore;
        m_pPusherIO = NULL;

        resetForNewLoop(NULL, NULL, 0, false, 0);
        setActionGroupId(0);
    };

    virtual ~FMyMJGameActionCollectorCpp()
    {};

    void init(TSharedPtr<FMyMJGamePusherIOComponentFullCpp> &pPusherIO)
    {
        m_pPusherIO = pPusherIO;
    };

    void initInMirrorMode()
    {

    };

    //reinit means can be called multiple times, unlike init() which is disigned to be called only one time in its life time
    void reinit(TArray<FMyMJGameActionContainorCpp *> &aActionContainors, int32 iRandomSelectMask);

    //Warn: @pPreAction, @pPostAction must allocated on heap and this function will take ownership
    //Warn: m_iActionGroupId will not be set, you need call setActionGroupId() for that
    void resetForNewLoop(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, int32 iIdxAttenderMask, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax);

    inline
    void setActionGroupId(int32 iActionGroupId)
    {
        m_iActionGroupId = iActionGroupId;
    };

    inline
    TSharedPtr<FMyMJGamePusherIOComponentFullCpp> getpPusherIO()
    {
        return m_pPusherIO;
    };

    void genActionChoices();

    //return if all collected
    bool collectAction(int64 iTimePassedMs, bool &outHaveProgress);

protected:

    TArray<TSharedPtr<FMyMJGameActionBaseCpp>> m_aActionCollected;

    TSharedPtr<FMyMJGameActionBaseCpp> m_pPrevAction;

    TSharedPtr<FMyMJGameActionBaseCpp> m_pPostAction;

    int32 m_iCalcActionCollectedPriMax;

    bool m_bAllowSamePriAction;

    int32 m_iIdxAttenderHavePriMax; //When  m_bAllowSamePriAction = false, this decide who's choice will be pick when same priority action exist
    int32 m_iIdxContainorSearchStart;

    int32 m_iActionGroupId;

    bool m_bEnQueueDone;


    TArray<FMyMJGameActionContainorCpp *> m_aActionContainors; //should be equal to real attender num, observer

    TSharedPtr<FMyMJGamePusherIOComponentFullCpp> m_pPusherIO;

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;
};