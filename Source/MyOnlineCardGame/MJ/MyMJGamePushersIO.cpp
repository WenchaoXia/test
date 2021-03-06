// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePushersIO.h"
#include "MyMJGameAttender.h"
#include "MyMJGameCore.h"
#include "MyMJGameData.h"

bool FMyMJGamePusherIOComponentFullCpp::GivePusherResult(FMyMJGamePusherResultCpp*& pPusherResult)
{
    MY_VERIFY(pPusherResult);
    int32 iGameId, iPusherIdLast;

    pPusherResult->getGameIdAndPusherId(&iGameId, &iPusherIdLast);

    if (m_iEnqueuePusherCount == (0 + 1)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("a base pusher result detected: [%d:%d]."), iGameId, iPusherIdLast);
        MY_VERIFY(iPusherIdLast == 0);
    }

    if (m_pQueueRemote)
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("pusher result enqueued: [%d:%d]."), iGameId, iPusherIdLast);
        m_pQueueRemote->Enqueue(pPusherResult);
        pPusherResult = NULL;
        return true;
    }
    else {

        delete(pPusherResult);
        pPusherResult = NULL;

        return false;
    }
};

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
FMyMJGameActionContainorCpp::collectAction(int32 iTimePassedMs, int32 &outPriorityMax, bool &outAlwaysCheckDistWhenCalcPri, TSharedPtr<FMyMJGameActionBaseCpp> &outPSelected, int32 &outSelection, TArray<int32> &outSubSelections, MyCardGameAIStrategyTypeCpp &eAIStrategyTypeUsedForSelected, FRandomStream &RS)
{
    outPriorityMax = m_iPriorityMax;
    outAlwaysCheckDistWhenCalcPri = m_bAlwaysCheckDistWhenCalcPri;

    int32 choiceCount = getActionChoiceRealCount();

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
                FMyErrorCodeMJGameCpp errorCode = makeSelection(0);
                if (errorCode.hasError()) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("errorCode: %s"), *errorCode.ToString());
                    MY_VERIFY(false);
                }
            }

        }
    }

    //have choices, not choose yet, try random choose
    if (m_pSelected.Get() == NULL && m_eAIStrategyType != MyCardGameAIStrategyTypeCpp::Invalid && m_eAIStrategyType != MyCardGameAIStrategyTypeCpp::Disabled) {
        //try AI
        if (m_bWantAIControl) {
            makeAISelection(RS);
        }
        else {
            if (m_iIdleTimePassed_ms >= m_iIdleTimeToAIControl_ms) {
                makeAISelection(RS);
            }
            else {
                m_iIdleTimePassed_ms += iTimePassedMs;
            }
        }
    }

    //check again:
    if (m_pSelected.Get() == NULL) {
        return ActionCollectWaiting;
    }

    outPSelected = m_pSelected;
    outSelection = m_iSelectionInputed;
    outSubSelections = m_aSubSelectionsInputed;
    eAIStrategyTypeUsedForSelected = m_eAIStrategyTypeUsedForSelected;
    return ActionCollectOK;
};

void
FMyMJGameActionContainorCpp::makeAISelection(FRandomStream &RS)
{
    m_eAIStrategyTypeUsedForSelected = m_eAIStrategyType;

    int32 l = m_aActionChoices.Num();
    MY_VERIFY(l > 0);

    int32 selection0 = -1;
    if (m_eAIStrategyType == MyCardGameAIStrategyTypeCpp::StrategyRandom) {
        selection0 = RS.RandRange(0, l - 1);
    }
    else if (m_eAIStrategyType == MyCardGameAIStrategyTypeCpp::StrategyBestChanceToWin) {
        //Todo: improve the logic
        TArray<int32> aSelectionsPriMax;
        int32 priMax = -1;
        for (int32 i = 0; i < l; i++) {
            int32 iPri = m_aActionChoices[i]->getPriority();
            if (iPri > priMax) {
                aSelectionsPriMax.Reset();
                aSelectionsPriMax.Emplace(i);
                priMax = iPri;
            }
            else if (iPri == priMax) {
                aSelectionsPriMax.Emplace(i);
            }
            else {

            }
        }

        MY_VERIFY(aSelectionsPriMax.Num() > 0);
        int32 idx = RS.RandRange(0, aSelectionsPriMax.Num() - 1);
        selection0 = aSelectionsPriMax[idx];
    }
    MY_VERIFY(selection0 >= 0 && selection0 < l);

    TSharedPtr<FMyMJGameActionBaseCpp> pAction = m_aActionChoices[selection0];

    if (pAction->getRealCountOfSelection() < 2) {
        FMyErrorCodeMJGameCpp errorCode = makeSelection(selection0);
        if (errorCode.hasError()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("errorCode: %s"), *errorCode.ToString());
            MY_VERIFY(false);
        }
        return;
    }
    
    TArray<int32> subSelections;
    MY_VERIFY(pAction->genRandomSubSelections(RS, subSelections) == 0);

    FMyErrorCodeMJGameCpp errorCode = makeSelection(selection0, subSelections);
    if (errorCode.hasError()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("errorCode: %s"), *errorCode.ToString());
        MY_VERIFY(false);
    }
    return;
}

void
FMyMJGameActionCollectorCpp::setWorkingContainors(TArray<FMyMJGameActionContainorCpp *> &aActionContainors)
{
    //raw pointer can be assigned directly, unlike TSharedPointer
    m_aActionContainors = aActionContainors;
};

void
FMyMJGameActionCollectorCpp::resetForNewLoopForFullMode(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax, int32 iExpectedContainorDebug)
{
    clear();

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

    if (pPostAction) {
        m_pPostAction = MakeShareable<FMyMJGameActionBaseCpp>(pPostAction);
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
FMyMJGameActionCollectorCpp::collectAction(int32 iActionGroupId, int32 iTimePassedMs, bool &outHaveProgress, FRandomStream &RS)
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

        bool &bNeed2Collect = pContainor->getNeed2CollectRef();
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
        MyCardGameAIStrategyTypeCpp eAIStrategyTypeUsedForSelected = MyCardGameAIStrategyTypeCpp::Invalid;
        int ret = pContainor->collectAction(iTimePassedMs, selectionsPriMax, bAlwaysCalcDist, selected, selection, aSubSelections, eAIStrategyTypeUsedForSelected, RS);

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

            FMyMJGamePusherMadeChoiceNotifyCpp *pMadeChoiceNotify = new FMyMJGamePusherMadeChoiceNotifyCpp();

            idxAttender = pContainor->getIdxAttender();
            pMadeChoiceNotify->init(idxAttender, iActionGroupId, selection, aSubSelections, eAIStrategyTypeUsedForSelected);
            m_pPusherIO->GivePusher(pMadeChoiceNotify, (void **)&pMadeChoiceNotify);

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

            bool &bNeed2Collect = pContainor->getNeed2CollectRef();
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
        pContainor->getNeed2CollectRef() = false;
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

    uint32 pusherGivenCount = 0;
    TSharedPtr<FMyMJGameCoreCpp> pCore = m_pCore.Pin();

    if (m_pPrevAction.IsValid()) {
        int32 idxA = m_pPrevAction->getIdxAttender(false);
        if (idxA >= 0 && idxA < 4) {
            TSharedPtr<FMyMJGameAttenderCpp> pA = pCore->getRealAttenderByIdx(idxA, false);
            if (pA.IsValid()) {
                m_pPrevAction->resolveActionResult(*pA.Get());
            }
        }

        FMyMJGamePusherBaseCpp *p = m_pPrevAction->cloneDeep();
        m_pPusherIO->GivePusher(p, (void **)&p);
        pusherGivenCount++;
    }

    for (int32 i = 0; i < aPusher2Enqueue.Num(); i++) {
        TSharedPtr<FMyMJGameActionBaseCpp> &pAction = aPusher2Enqueue[i];
        pAction->resolveActionResult(*(pCore->getRealAttenderByIdx(pAction->getIdxAttender()).Get()));

        FMyMJGamePusherBaseCpp *p = pAction->cloneDeep();
        m_pPusherIO->GivePusher(p, (void **)&p);
        pusherGivenCount++;

    }
    aPusher2Enqueue.Empty();

    if (m_pPostAction.IsValid()) {
        int32 idxA = m_pPostAction->getIdxAttender(false);
        if (idxA >= 0 && idxA < 4) {
            TSharedPtr<FMyMJGameAttenderCpp> pA = pCore->getRealAttenderByIdx(idxA, false);
            if (pA.IsValid()) {
                m_pPostAction->resolveActionResult(*pA.Get());
            }
        }

        FMyMJGamePusherBaseCpp *p = m_pPostAction->cloneDeep();
        m_pPusherIO->GivePusher(p, (void **)&p);
        pusherGivenCount++;
    }

    //if (pusherGivenCount > 0) {
    MY_VERIFY(m_pCore.IsValid());
    bool bInGame = m_pCore.Pin()->getCoreDataPublicRefConst().isInGameState();

    if (bInGame) {
        FMyMJGamePusherCountUpdateCpp *pPusherCountUpdate = new FMyMJGamePusherCountUpdateCpp();
        pPusherCountUpdate->m_bActionGroupIncrease = true;
        m_pPusherIO->GivePusher(pPusherCountUpdate, (void **)(&pPusherCountUpdate));
    }

    return bAllCollected;
}
