// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePushersIO.h"
#include "MyMJGameAttender.h"
#include "MyMJGameCore.h"


TSharedPtr<FMyMJGamePusherBaseCpp> FMyMJGamePusherIOComponentFullCpp::tryPullPusherFromLocal()
{

    FMyMJGamePusherBaseCpp *pTaken = NULL;

    if (m_cQueueLocal.Dequeue(pTaken)) {

        //Take onwership, for full mode IO Component, we directly done it here
        pTaken->onReachedConsumeThread();
        return MakeShareable<FMyMJGamePusherBaseCpp>(pTaken);
    }
    else {
        return NULL;
    }
};



int32
FMyMJGameActionContainorCpp::collectAction(int32 iTimePassedMs, int32 &outPriorityMax, bool &outAlwaysCheckDistWhenCalcPri, TSharedPtr<FMyMJGameActionBaseCpp> &outPSelected, int32 &outSelection, TArray<int32> &outSubSelections, FRandomStream &RS)
{
    outPriorityMax = m_iPriorityMax;
    outAlwaysCheckDistWhenCalcPri = m_bAlwaysCheckDistWhenCalcPri;

    int32 choiceCount = getActionChoiceCount();

    //have no choices
    if (choiceCount <= 0) {
        return ActionCollectEmpty;
    }

    //have choices, not choose yet, try auto choose
    if (m_pSelected.Get() == NULL) {
        if (choiceCount == 1) {
            //try auto select
            int32 &timeLeft = m_aActionChoices[0]->getTimeLeft2AutoChooseRef();
            //1st, try calc
            if (timeLeft > 0) {
                timeLeft -= iTimePassedMs;
                if (timeLeft < 0) {
                    timeLeft = 0;
                }
            }
            else {
                timeLeft = 0;
            }

            //2nd, try make choice
            if (timeLeft == 0) {
                MyMJGameErrorCodeCpp errorCode = makeSelection(m_iActionGroupId, 0);
                if (errorCode != MyMJGameErrorCodeCpp::None) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("errorCode: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameErrorCodeCpp"), (uint8)errorCode));
                    MY_VERIFY(false);
                }
            }

        }
    }

    //have choices, not choose yet, try random choose
    if (m_pSelected.Get() == NULL && m_bRandomSelect) {

        makeRandomSelection(RS);
    }

    //check again:
    if (m_pSelected.Get() == NULL) {
        return ActionCollectWaiting;
    }

    outPSelected = m_pSelected;
    outSelection = m_iSelectionInputed;
    outSubSelections = m_aSubSelectionsInputed;
    return ActionCollectOK;
};

void
FMyMJGameActionContainorCpp::makeRandomSelection(FRandomStream &RS)
{
    int32 l = m_aActionChoices.Num();
    MY_VERIFY(l > 0);
    int32 selection0 = RS.RandRange(0, l - 1);

    TSharedPtr<FMyMJGameActionBaseCpp> pAction = m_aActionChoices[selection0];

    if (pAction->getRealCountOfSelection() < 2) {
        MyMJGameErrorCodeCpp errorCode = makeSelection(m_iActionGroupId, selection0);
        if (errorCode != MyMJGameErrorCodeCpp::None) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("errorCode: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameErrorCodeCpp"), (uint8)errorCode));
            MY_VERIFY(false);
        }
        return;
    }
    
    TArray<int32> subSelections;
    MY_VERIFY(pAction->genRandomSubSelections(RS, subSelections) == 0);

    MyMJGameErrorCodeCpp errorCode = makeSelection(m_iActionGroupId, selection0, subSelections);
    if (errorCode != MyMJGameErrorCodeCpp::None) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("errorCode: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameErrorCodeCpp"), (uint8)errorCode));
        MY_VERIFY(false);
    }
    return;
}

void
FMyMJGameActionCollectorCpp::reinit(TArray<FMyMJGameActionContainorCpp *> &aActionContainors, int32 iRandomSelectMask)
{
    //raw pointer can be assigned directly, unlike TSharedPointer
    m_aActionContainors = aActionContainors;

    int32 l = m_aActionContainors.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJGameActionContainorCpp *pContainor = m_aActionContainors[i];
        int32 idxAttender = pContainor->getIdxAttender();
        if ((iRandomSelectMask & (1 << idxAttender)) > 0) {
            pContainor->reinit(true);
        }
        else {
            pContainor->reinit(false);
        }

    }

    //reinit always happend before game start
    setActionGroupId(0);
};

void
FMyMJGameActionCollectorCpp::resetForNewLoopForFullMode(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax, int32 iExpectedContainorDebug)
{
    m_aActionCollected.Reset();
    m_iCalcActionCollectedPriMax = 0;
    m_bAllowSamePriAction = bAllowSamePriAction;

    if (iIdxAttenderHavePriMax >= 0 && iIdxAttenderHavePriMax < 4) {
        m_iIdxAttenderHavePriMax = iIdxAttenderHavePriMax;
    }
    else {
        iIdxAttenderHavePriMax = 0;
    }
    m_iIdxContainorSearchStart = 0;

    //m_iActionGroupId = iActionGroupId;
    m_bEnQueueDone = false;

    if (pPrevAction) {
        m_pPrevAction = MakeShareable<FMyMJGameActionBaseCpp>(pPrevAction);
    }
    else {
        m_pPrevAction = NULL;
    }

    if (pPostAction) {
        m_pPostAction = MakeShareable<FMyMJGameActionBaseCpp>(pPostAction);
    }
    else {
        m_pPostAction = NULL;
    }

    FMyMJGameActionContainorCpp* pContainor;
    int l = m_aActionContainors.Num();
    if (iExpectedContainorDebug >= 0) {
        if (iExpectedContainorDebug != l) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("expectation not eqal, expected:%d, l:%d"), iExpectedContainorDebug, l);
            MY_VERIFY(false);
        }
    }

    for (int i = 0; i < l; i++) {
        pContainor = m_aActionContainors[i];
        
        if (pContainor->getIdxAttender() == m_iIdxAttenderHavePriMax) {
            m_iIdxContainorSearchStart = i;
        }
    }
};


bool
FMyMJGameActionCollectorCpp::collectAction(int32 iTimePassedMs, bool &outHaveProgress, FRandomStream &RS)
{
    MY_VERIFY(m_pPusherIO.IsValid());
    outHaveProgress = false;
    if (m_bEnQueueDone) {
        return true;
    }

    FMyMJGameActionContainorCpp* pContainor;
    int32 l = m_aActionContainors.Num();

    //int32 idxAttender;
    int32 calcPri;
    bool bAllCollected = true;

    TArray<int32> aSubSelections;

    int32 idxWorkingContainor;

    for (int i0 = 0; i0 < l; i0++) {

        idxWorkingContainor = (m_iIdxContainorSearchStart + i0) % l;
        pContainor = m_aActionContainors[idxWorkingContainor];

        bool &bNeed2Collect = pContainor->getNeed2Collect();
        if (!bNeed2Collect) {
            continue;
        }

        TSharedPtr<FMyMJGameActionBaseCpp> selected = NULL;
        int selectionsPriMax = 0;
        int32 selection;
        aSubSelections.Reset();

        int32 idxAttender = pContainor->getIdxAttender();
        MY_VERIFY(idxAttender >= 0);
        //if (m_bAllowSamePriAction) {

        //The calculation assume: all attender's setting is setting from 1 - 4, always the next number have higher priority
        int32 distancePri = 4 - ((idxAttender - m_iIdxAttenderHavePriMax + 4) % 4);


        //now collect all equal to m_iCalcActionCollectedPriMax
        bool bAlwaysCalcDist;
        int ret = pContainor->collectAction(iTimePassedMs, selectionsPriMax, bAlwaysCalcDist, selected, selection, aSubSelections, RS);

        if (ret == ActionCollectOK) {

            int32 calcPriSub;
            if (m_bAllowSamePriAction && !selected->isAlwaysCheckDistWhenCalcPri()) {
                calcPriSub = 0;
            }
            else {
                calcPriSub = distancePri;
            }
            calcPri = selected->getPriority() * 10 + calcPriSub;

            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("action collected, %lld ms. m_iCalcActionCollectedPriMax %d, calcPri %d"), iTimePassedMs, m_iCalcActionCollectedPriMax, calcPri);
            if (m_iCalcActionCollectedPriMax <= calcPri) {
                m_iCalcActionCollectedPriMax = calcPri;
                m_aActionCollected.Emplace(selected);
            }

            bNeed2Collect = false;

            FMyMJGamePusherMadeChoiceNotifyCpp madeChoiceNotify, *pMadeChoiceNotify = &madeChoiceNotify;

            idxAttender = pContainor->getIdxAttender();
            pMadeChoiceNotify->init(idxAttender, m_iActionGroupId, selection, aSubSelections);
            m_pPusherIO->EnqueuePusher(*pMadeChoiceNotify);

            outHaveProgress = true;
        }
        else if (ret == ActionCollectEmpty) {

            bNeed2Collect = false;
        }
        else if (ret == ActionCollectWaiting) {
            int32 calcPriSub;
            if (m_bAllowSamePriAction && !bAlwaysCalcDist) {
                calcPriSub = 0;
            }
            else {
                calcPriSub = distancePri;
            }

            calcPri = selectionsPriMax * 10 + calcPriSub;

            if (m_iCalcActionCollectedPriMax <= calcPri) {
                //waiting for choice
                bAllCollected = false;
            }
            else {
                //the choice should be ignored

            }
        }
    }

    //recheck the priority
    if (outHaveProgress && !bAllCollected) {
        bool bAllIgnore = true;
        for (int i = 0; i < l; i++) {

            pContainor = m_aActionContainors[i];

            bool &bNeed2Collect = pContainor->getNeed2Collect();
            if (!bNeed2Collect) {
                continue;
            }

            int32 idxAttender = pContainor->getIdxAttender();
            int32 distancePri = 4 - ((idxAttender - m_iIdxAttenderHavePriMax + 4) % 4);
            
            int32 calcPriSub;
            if (m_bAllowSamePriAction && !pContainor->getAlwaysCheckDistWhenCalcPri()) {
                calcPriSub = 0;
            }
            else {
                calcPriSub = distancePri;
            }

            calcPri = pContainor->getPriorityMax() * 10 + calcPriSub;

            if (m_iCalcActionCollectedPriMax <= calcPriSub) {
                //waiting for choice
                bAllIgnore = false;
            }
            else {
                //the choice should be ignored

            }
        }

        if (bAllIgnore) {
            bAllCollected = true;
        }
    }


    if (!bAllCollected) {
        return bAllCollected;
    }

    outHaveProgress = true;
    m_bEnQueueDone = true; //one way to switch, never jump back unless reset
    for (int i = 0; i < l; i++) {
        pContainor = m_aActionContainors[i];
        pContainor->getNeed2Collect() = false;
    }

    //let's filter, only pick up actions == max
    l = m_aActionCollected.Num();



    TArray<TSharedPtr<FMyMJGameActionBaseCpp>> aPusher2Enqueue;
    bool bNeedTryOverridePrevAction = false;
    for (int i = 0; i < l; i++) {
        TSharedPtr<FMyMJGameActionBaseCpp> &pAction = m_aActionCollected[i];
        int32 idxAttender = pAction->getIdxAttender();
        int32 distancePri = 4 - ((idxAttender - m_iIdxAttenderHavePriMax + 4) % 4);
        int32 calcPriSub;
        if (m_bAllowSamePriAction && !pAction->isAlwaysCheckDistWhenCalcPri()) {
            calcPriSub = 0;
        }
        else {
            calcPriSub = distancePri;
        }

        calcPri = pAction->getPriority() * 10 + calcPriSub;

        if (calcPri == m_iCalcActionCollectedPriMax) {
            //m_pPusherIO->EnqueuePusher(*pAction);
            aPusher2Enqueue.Emplace(pAction);

            //special rule, not ending the game hu, let's continue state machine running
            if (pAction->getType() == MyMJGamePusherTypeCpp::ActionHu) {
                TSharedPtr<FMyMJGameActionHuCpp> pActionHu = StaticCastSharedPtr<FMyMJGameActionHuCpp>(pAction);
                if (pActionHu->m_bEndGame) {
                    bNeedTryOverridePrevAction = true;
                }
                else {

                }
            }
            else {
                bNeedTryOverridePrevAction = true;
            }
        }
    }
    m_aActionCollected.Reset();

    if (m_pPrevAction.IsValid()) {
        if (bNeedTryOverridePrevAction) {
            //simple calc, no consider distance
            calcPri = m_pPrevAction->getPriority() * 10 + 0;
            if (calcPri < m_iCalcActionCollectedPriMax) {
                m_pPrevAction = NULL;
            }
        }
    }

    if (m_pPrevAction.IsValid()) {
        m_pPusherIO->EnqueuePusher(*m_pPrevAction);
    }


    TSharedPtr<FMyMJGameCoreCpp> pCore = m_pCore.Pin();
    for (int32 i = 0; i < aPusher2Enqueue.Num(); i++) {
        TSharedPtr<FMyMJGameActionBaseCpp> &pAction = aPusher2Enqueue[i];
        pAction->resolveActionResult(*(pCore->getRealAttenderByIdx(pAction->getIdxAttender()).Get()));
        m_pPusherIO->EnqueuePusher(*aPusher2Enqueue[i]);
    }
    aPusher2Enqueue.Empty();

    if (m_pPostAction.IsValid()) {
        m_pPusherIO->EnqueuePusher(*m_pPostAction);
    }

    FMyMJGamePusherCountUpdateCpp pusherCountUpdate;
    pusherCountUpdate.m_bActionGroupIncrease = true;
    m_pPusherIO->EnqueuePusher(pusherCountUpdate);

    return bAllCollected;
}
