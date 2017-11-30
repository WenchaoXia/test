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

/*
struct FMyMJGamePusherAndResultCpp
{

public:
    FMyMJGamePusherAndResultCpp()
    {
        m_pPusher = NULL;
    };

    virtual ~FMyMJGamePusherAndResultCpp()
    {
        if (m_pPusher) {
            delete(m_pPusher);
        }
    };

    //this struct owns them
    FMyMJGamePusherBaseCpp *m_pPusher;
    FMyMJGamePusherResultCpp m_cResult;
};
*/

//both producer and consumer, consume itself, and produce to external unit
class FMyMJGamePusherIOComponentFullCpp
{

public:
    FMyMJGamePusherIOComponentFullCpp()
    {
        init(NULL);
    };

    virtual ~FMyMJGamePusherIOComponentFullCpp()
    {
        reset();
        m_pQueueRemote = NULL;
    };

    void reset()
    {
        m_iEnqueuePusherCount = 0;

        //clear what it owns, here it is the local queue that can work in produce mode
        FMyMJGamePusherBaseCpp *pRet = NULL;
        while (m_cQueueLocal.Dequeue(pRet)) {
            delete(pRet);
        };

    };

    void init(TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc> *pQueueRemote)
    {
        m_pQueueRemote = pQueueRemote;
        reset();
    };

    TSharedPtr<FMyMJGamePusherBaseCpp> tryPullPusherFromLocal();

    //@pPusher must be allocated on heap, and ownership will be taken
    void GivePusher(FMyMJGamePusherBaseCpp* pPusher, void** ppPusher)
    {
        MY_VERIFY(pPusher);
        MY_VERIFY(ppPusher);
        MY_VERIFY(pPusher == *ppPusher);

        if (pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            m_iEnqueuePusherCount = 0;
        }

        pPusher->m_iId = m_iEnqueuePusherCount;

        //FMyMJGamePusherAndResultCpp *pNew = new FMyMJGamePusherAndResultCpp();

        //pNew->m_pPusher = pusher.cloneDeep();
        //if (pNew->m_pPusher == NULL) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to clone pusher, pusher type %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pusher.getType()));
            //MY_VERIFY(false);
        //}

        //pNew->m_pPusher->tryGenResultAsSysKeeper(pNew->m_cResult);
        //pNew->m_cResult.checkHaveValidResult();

        //if (m_pQueueRemote) {
         //   FMyMJGamePusherResultCpp *pResultNew = pNew->m_cResult.cloneDeep();
         //   if (pResultNew) {
         //       m_pQueueRemote->Enqueue(pResultNew);
         //   }
         //   else {
         //       UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to clone pusher result, pusher type %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pusher.getType()));
         //       MY_VERIFY(false);
         //   }
        //}

        m_cQueueLocal.Enqueue(pPusher);
        //pPusher = NULL;

        *ppPusher = NULL;

        m_iEnqueuePusherCount++;
    };

    //@pPusherResult must be allocated on heap, and ownership will be taken
    bool GivePusherResult(FMyMJGamePusherResultCpp*& pPusherResult);


protected:
    //todo: make it as a cycle buffer

    //owned by this class, can only be used in one thread, so it is safe to clear by either producer or consumer
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc> m_cQueueLocal;

    //owned by other, never managed by this class, don't drain it in this class!
    //Note, only consume thread can clear it, and this is always created by main thread, destroy by main thread. destroy only happen after sub thread stopped/killed, after then, parent thread can clear() also
    TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc> *m_pQueueRemote;

    int32 m_iEnqueuePusherCount;

};

#define ActionCollectOK 0
#define ActionCollectEmpty 1
#define ActionCollectWaiting 2

#define MyMJGameActionContainorCpp_RandomMask_DoRandomSelect 0x01
#define MyMJGameActionContainorCpp_RandomMask_HighPriActionFirst 0x02

USTRUCT()
struct FMyMJGameActionContainorCpp
{
    GENERATED_USTRUCT_BODY()

public:

    friend struct FMyMJGameActionContainorForBPCpp;

    FMyMJGameActionContainorCpp()
    {
        FMyMJGameActionContainorCpp(-1);
    };

    FMyMJGameActionContainorCpp(int32 idxAttender)
    {
        setup(idxAttender);
    };

    virtual ~FMyMJGameActionContainorCpp()
    {};

    void setup(int32 idxAttender)
    {
        m_iIdxAttedner = idxAttender;
        reinit(0);
    };

    void reinit(int8 iRandomMask)
    {
        m_iRandomMask = iRandomMask;
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

        //m_iActionGroupId = -1;
        m_iPriorityMax = -1;
        m_bAlwaysCheckDistWhenCalcPri =  false;

        m_pSelected = NULL;

        m_bNeed2Collect = false;

    };

    inline
    int32 getIdxAttender() const
    {
        return m_iIdxAttedner;
    };

    inline
    bool &getNeed2CollectRef()
    {
        return m_bNeed2Collect;
    };

    inline
    bool getNeed2CollectConst() const
    {
        return m_bNeed2Collect;
    };

    int32 getActionChoiceRealCount() const
    {
        int32 totalCount = 0;
        int32 l = m_aActionChoices.Num();
        for (int32 i = 0; i < l; i++) {
            totalCount += m_aActionChoices[i]->getRealCountOfSelection();
        }

        return totalCount;
    };

    inline
    int32 getActionChoiceUnitCount() const
    {
        return m_aActionChoices.Num();
    };

    void fillInNewChoices(int32 iActionGroupId, TArray<TSharedPtr<FMyMJGameActionBaseCpp>> &actionChoices)
    {
        resetForNewLoop();
        //m_iActionGroupId = iActionGroupId;
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
    MyMJGameErrorCodeCpp makeSelection(int32 iSelection)
    {
        TArray<int32> t;
        return makeSelection(iSelection, t);
    };

    //called only in "full" core, don't call dump in the code path in case of reject invalid request
    MyMJGameErrorCodeCpp makeSelection(int32 iSelection, TArray<int32> &subSelections)
    {
        //if (m_iActionGroupId != iActionGroupId) {
            //return MyMJGameErrorCodeCpp::pusherIdNotEqual;
        //}

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

    void makeRandomSelection(FRandomStream &RS);

    //this code path core dump if met any invalid request
    void showSelectionOnNotify(int32 iSelection, const TArray<int32> &subSelections)
    {
        //if (m_iActionGroupId != iActionGroupId) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("action group id not equal: m_iActionGroupId %d, iActionGroupId %d."), m_iActionGroupId, iActionGroupId);
            //MY_VERIFY(false);
        //}

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

    int32 collectAction(int32 iTimePassedMs, int32 &outPriorityMax, bool &outAlwaysCheckDistWhenCalcPri, TSharedPtr<FMyMJGameActionBaseCpp> &outPSelected, int32 &outSelection, TArray<int32> &outSubSelections, FRandomStream &RS);

    inline
    int32 getPriorityMax() {
        return m_iPriorityMax;
    };

    inline
    bool getAlwaysCheckDistWhenCalcPri() {
        return m_bAlwaysCheckDistWhenCalcPri;
    };

    //inline
    //int32 getActionGroupId() const
    //{
    //    return m_iActionGroupId;
    //};

protected:
    //following is used to tell BP what we have
    //< 0 means not selected yet
    int32 m_iSelectionPushed;

    TArray<int32> m_aSubSelectionsPushed;

    bool m_bNeed2CollectPushed; //can be used to tip whether waiting to choose in UI

    TArray<TSharedPtr<FMyMJGameActionBaseCpp>> m_aActionChoices; //Todo: War

    int32 m_iSelectionInputed; //It is the easiest way to save it like this, kick the core and let the core pick it
    TArray<int32> m_aSubSelectionsInputed;

    //int32 m_iActionGroupId;
    int32 m_iPriorityMax; //the max pri action's property
    bool  m_bAlwaysCheckDistWhenCalcPri; //the max pri action's property

    TSharedPtr<FMyMJGameActionBaseCpp> m_pSelected;
    bool m_bNeed2Collect;

    int8 m_iRandomMask; //Mainly used for test and one stupid AI

    int32 m_iIdxAttedner;

};

//USTRUCT()
//only exist in full mode
struct FMyMJGameActionCollectorCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionCollectorCpp(TWeakPtr<FMyMJGameCoreCpp> pCore)
    {
        m_pCore = pCore;
        m_pPusherIO = NULL;
        m_pPrevAction = NULL;
        m_pPostAction = NULL;

        resetForNewLoopForFullMode(NULL, NULL, false, 0, -1);
        //setActionGroupId(0);
    };

    virtual ~FMyMJGameActionCollectorCpp()
    {
        clear();
    };

    void init(TSharedPtr<FMyMJGamePusherIOComponentFullCpp> &pPusherIO)
    {
        m_pPusherIO = pPusherIO;
    };

    void clear()
    {
        m_pPrevAction = NULL;
        m_pPostAction = NULL;
        m_aActionCollected.Reset();
    };

    //reinit means can be called multiple times, unlike init() which is disigned to be called only one time in its life time
    void reinit(TArray<FMyMJGameActionContainorCpp *> &aActionContainors, int32 iRandomSelectMask);

    //Warn: @pPreAction, @pPostAction must allocated on heap and this function will take ownership
    void resetForNewLoopForFullMode(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax, int32 iExpectedContainorDebug);

    //inline
    //void setActionGroupId(int32 iActionGroupId)
    //{
    //    m_iActionGroupId = iActionGroupId;
    //};

    //inline
    //int32 getActionGroupId() const
    //{
        //return m_iActionGroupId;
    //};

    inline
    TSharedPtr<FMyMJGamePusherIOComponentFullCpp> getpPusherIO()
    {
        return m_pPusherIO;
    };

    //return if all collected
    bool collectAction(int32 iActionGroupId, int32 iTimePassedMs, bool &outHaveProgress, FRandomStream &RS);

protected:

    //we can't use raw pointers since the action may exist both collector and attender's containor, which require you write ref count by you own
    TArray<TSharedPtr<FMyMJGameActionBaseCpp>> m_aActionCollected;

    TSharedPtr<FMyMJGameActionBaseCpp> m_pPrevAction;

    TSharedPtr<FMyMJGameActionBaseCpp> m_pPostAction;

    int32 m_iCalcActionCollectedPriMax;

    bool m_bAllowSamePriAction;

    int32 m_iIdxAttenderHavePriMax; //When  m_bAllowSamePriAction = false, this decide who's choice will be pick when same priority action exist
    int32 m_iIdxContainorSearchStart;

    //int32 m_iActionGroupId;

    bool m_bEnQueueDone;


    TArray<FMyMJGameActionContainorCpp *> m_aActionContainors; //should be equal to real attender num, and we record it here directly since it make things simple when calculate distance

    TSharedPtr<FMyMJGamePusherIOComponentFullCpp> m_pPusherIO;

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;
};

USTRUCT(BlueprintType)
struct FMyMJGameActionContainorForBPCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameActionContainorForBPCpp()
    {
        resetForNewActionLoop();
    };

    void resetForNewActionLoop()
    {
        m_iChoiceSelected = -1;
        m_aSubDataChoiceSelected.Reset();
        m_aActionChoices.Reset();
    };

    bool equal(const FMyMJGameActionContainorCpp &other) const
    {

        if (m_iChoiceSelected != other.m_iSelectionPushed) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("action choice selection not equal, logic %d, data %d."), other.m_iSelectionPushed, m_iChoiceSelected);
            return false;
        }

        if (m_aSubDataChoiceSelected != other.m_aSubSelectionsPushed) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("action choice sub data not equal, logic num %d, data num %d."), other.m_aSubSelectionsPushed.Num(), m_aSubDataChoiceSelected.Num());
            return false;
        }

        int32 l0, l1;
        l0 = other.m_aActionChoices.Num();
        l1 = m_aActionChoices.Num();

        if (l0 != l1) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("action choice num not equal, logic %d, data %d."), l0, l1);
            return false;
        }

        return true;
    };

    //following is used to tell BP what we have
    //< 0 means not selected yet
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx of choice selected"))
    int32 m_iChoiceSelected;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "sub data of choices selected"))
    TArray<int32> m_aSubDataChoiceSelected;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action choices"))
    TArray<FMyMJGameActionUnfiedForBPCpp> m_aActionChoices; //Todo: War


};