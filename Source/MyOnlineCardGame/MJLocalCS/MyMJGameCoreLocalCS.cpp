// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreLocalCS.h"
#include "Kismet/KismetMathLibrary.h"


void FMyMJGameCoreLocalCSCpp::applyPusher(FMyMJGamePusherBaseCpp *pPusher)
{

    FMyMJGamePusherFillInActionChoicesCpp *pPusherFillInActionChoices;
    FMyMJGamePusherMadeChoiceNotifyCpp *pPusherMadeChoiceNotify;
    FMyMJGamePusherCountUpdateCpp *pPusherCountUpdate;
    FMyMJGamePusherResetGameCpp *pPusherResetGame;
    FMyMJGamePusherUpdateCardsCpp *pPusherUpdateCards;
    FMyMJGamePusherUpdateTingCpp *pPusherUpdateTing;

    FMyMJGameActionStateUpdateCpp *pActionStateUpdate;
    FMyMJGameActionNoActCpp *pActionNoAct;
    FMyMJGameActionThrowDicesCpp *pActionThrowDices;
    FMyMJGameActionDistCardAtStartCpp *pActionDistCardAtStart;
    FMyMJGameActionTakeCardsCpp *pActionTakeCards;
    FMyMJGameActionGiveOutCardsCpp *pActionGiveOutCards;
    FMyMJGameActionWeaveCpp *pActionWeave;

    FMyMJGameActionHuCpp *pActionHu;

    FMyMJGameActionHuBornLocalCSCpp  *pActionHuBornLocalCS;
    FMyMJGameActionZhaNiaoLocalCSCpp *pActionZhaNiaoLocalCS;


    {
        MyMJGamePusherTypeCpp ePusherType = pPusher->getType();

        switch (ePusherType)
        {
        case MyMJGamePusherTypeCpp::Invalid:
            MY_VERIFY(false);
            break;
        case MyMJGamePusherTypeCpp::PusherBase:
            MY_VERIFY(false);
            break;
        case MyMJGamePusherTypeCpp::PusherFillInActionChoices:
            pPusherFillInActionChoices = StaticCast<FMyMJGamePusherFillInActionChoicesCpp *>(pPusher);
            applyPusherFillInActionChoices(pPusherFillInActionChoices);
            break;
        case MyMJGamePusherTypeCpp::PusherMadeChoiceNotify:
            pPusherMadeChoiceNotify = StaticCast<FMyMJGamePusherMadeChoiceNotifyCpp *>(pPusher);
            applyPusherMadeChoiceNotify(pPusherMadeChoiceNotify);
            break;
        case MyMJGamePusherTypeCpp::PusherCountUpdate:
            pPusherCountUpdate = StaticCast<FMyMJGamePusherCountUpdateCpp *>(pPusher);
            applyPusherCountUpdate(pPusherCountUpdate);
            break;
        case MyMJGamePusherTypeCpp::PusherResetGame:
            pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
            applyPusherResetGame(pPusherResetGame);
            break;

        case MyMJGamePusherTypeCpp::PusherUpdateCards:
            pPusherUpdateCards = StaticCast<FMyMJGamePusherUpdateCardsCpp *>(pPusher);
            applyPusherUpdateCards(pPusherUpdateCards);
            break;

        case MyMJGamePusherTypeCpp::PusherUpdateTing:
            pPusherUpdateTing = StaticCast<FMyMJGamePusherUpdateTingCpp *>(pPusher);
            getRealAttenderByIdx(pPusherUpdateTing->m_iIdxAttender)->applyPusherUpdateTing(pPusherUpdateTing);
            break;


        case MyMJGamePusherTypeCpp::ActionBase:
            MY_VERIFY(false);
            break;
        case MyMJGamePusherTypeCpp::ActionStateUpdate:
            pActionStateUpdate = StaticCast<FMyMJGameActionStateUpdateCpp *>(pPusher);
            applyActionStateUpdate(pActionStateUpdate);
            break;
        case MyMJGamePusherTypeCpp::ActionNoAct:
            pActionNoAct = StaticCast<FMyMJGameActionNoActCpp *>(pPusher);
            getRealAttenderByIdx(pActionNoAct->getIdxAttender())->applyActionNoAct(pActionNoAct);
            break;
        case MyMJGamePusherTypeCpp::ActionThrowDices:
            pActionThrowDices = StaticCast<FMyMJGameActionThrowDicesCpp *>(pPusher);
            applyActionThrowDices(pActionThrowDices);
            break;
        case MyMJGamePusherTypeCpp::ActionDistCardsAtStart:
            pActionDistCardAtStart = StaticCast<FMyMJGameActionDistCardAtStartCpp *>(pPusher);
            applyActionDistCardsAtStart(pActionDistCardAtStart);
            break;

        case MyMJGamePusherTypeCpp::ActionTakeCards:
            pActionTakeCards = StaticCast<FMyMJGameActionTakeCardsCpp *>(pPusher);
            applyActionTakeCards(pActionTakeCards);
            break;

        case MyMJGamePusherTypeCpp::ActionGiveOutCards:
            pActionGiveOutCards = StaticCast<FMyMJGameActionGiveOutCardsCpp *>(pPusher);
            applyActionGiveOutCards(pActionGiveOutCards);
            break;
        case MyMJGamePusherTypeCpp::ActionWeave:
            pActionWeave = StaticCast<FMyMJGameActionWeaveCpp *>(pPusher);
            applyActionWeave(pActionWeave);
            break;


        case MyMJGamePusherTypeCpp::ActionHu:
            pActionHu = StaticCast<FMyMJGameActionHuCpp *>(pPusher);
            applyActionHu(pActionHu);
            break;

        case MyMJGamePusherTypeCpp::ActionHuBornLocalCS:
            pActionHuBornLocalCS = StaticCast<FMyMJGameActionHuBornLocalCSCpp *>(pPusher);
            getRealAttenderByIdx(pActionHuBornLocalCS->getIdxAttender())->applyActionHuBornLocalCS(pActionHuBornLocalCS);
            break;

        case MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS:
            pActionZhaNiaoLocalCS = StaticCast<FMyMJGameActionZhaNiaoLocalCSCpp *>(pPusher);
            applyActionZhaNiaoLocalCS(pActionZhaNiaoLocalCS);
            break;

        default:
            break;
        }
    }

}

void FMyMJGameCoreLocalCSCpp::handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp *pCmd)
{
    if (pCmd->m_eType == MyMJGameCmdType::RestartGame) {
        if (eRoleTypeOfCmdSrc == MyMJGameRoleTypeCpp::SysKeeper) {

            FMyMJGameCmdRestartGameCpp *pCmdRestartGame = StaticCast<FMyMJGameCmdRestartGameCpp *>(pCmd);

            FMyMJGamePusherResetGameCpp p, *pPusherReset = &p;
            FRandomStream * pRS = m_pResManager->getpRandomStream();
            int32 iGameId = m_iGameId + 1;
            if (iGameId < 0) {
                iGameId = 0;
            }
            pPusherReset->init(iGameId, pRS, pCmdRestartGame->m_cGameCfg, pCmdRestartGame->m_cGameRunData, pCmdRestartGame->m_iAttenderRandomSelectMask);

            m_pPusherIOFull->EnqueuePusher(*pPusherReset);

            pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::None;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("requested reset gane from invalid role: %d"), (uint8)eRoleTypeOfCmdSrc);
            pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::HaveNoAuthority;
        }

    }
    else {
        MY_VERIFY(false);
    }
}

void FMyMJGameCoreLocalCSCpp::applyPusherFillInActionChoices(FMyMJGamePusherFillInActionChoicesCpp* pPusher)
{
    int32 idxAttender = pPusher->getIdxAttender();

    TArray<TSharedPtr<FMyMJGameActionBaseCpp>> actionChoices;
    int32 l = pPusher->m_cActionChoices.getCount();

    MY_VERIFY(l > 0);

    for (int32 i = 0; i < l; i++) {
        TSharedPtr<FMyMJGamePusherBaseCpp> pP = pPusher->m_cActionChoices.getSharedPtrAt(i);

        MY_VERIFY((uint8)pP->getType() > (uint8)MyMJGamePusherTypeCpp::ActionBase);

        TSharedPtr<FMyMJGameActionBaseCpp> p2 = StaticCastSharedPtr<FMyMJGameActionBaseCpp>(pP);
        MY_VERIFY(p2.IsValid());

        actionChoices.Emplace(p2);
    }

    MY_VERIFY(l == actionChoices.Num());

    m_aAttendersAll[idxAttender]->getpActionContainor()->fillInNewChoices(pPusher->m_iActionGroupId, actionChoices);
}

void FMyMJGameCoreLocalCSCpp::applyPusherMadeChoiceNotify(FMyMJGamePusherMadeChoiceNotifyCpp *pPusher)
{
    int32 idxAttender = pPusher->getIdxAttenderRef();

    m_aAttendersAll[idxAttender]->getpActionContainor()->showSelectionOnNotify(pPusher->m_iActionGroupId, pPusher->m_iSelection, pPusher->m_aSubSelections);
}

void FMyMJGameCoreLocalCSCpp::applyPusherCountUpdate(FMyMJGamePusherCountUpdateCpp *pPusher)
{
    if (pPusher->m_bActionGroupIncrease) {
        m_iActionGroupId++;
        //if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full)) {
            //MY_VERIFY(m_pActionCollector.IsValid());
            m_pActionCollector->setActionGroupId(m_iActionGroupId);
        //}
    }
}

void FMyMJGameCoreLocalCSCpp::applyPusherResetGame(FMyMJGamePusherResetGameCpp *pPusher)
{
    //Stateless, never judge condition of previous state, just reset the whole core

    for (int i = 0; i < 4; i++) {
        m_aAttendersAll[i]->reset(false);
    }

    //let's construct things
    int32 iRealAttenderNum = pPusher->m_cGameCfg.m_cTrivialCfg.m_iGameAttenderNum;

    TArray<FMyMJGameActionContainorCpp *> aContainors;

    if (iRealAttenderNum == 2) {
        for (int32 i = 0; i < 2; i++) {

            TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i * 2];
            pAttender->setISRealAttender(true);
            pAttender->setIsStillInGame(true);
            aContainors.Emplace(pAttender->getpActionContainor());
        }
    }
    else if (iRealAttenderNum >= 3 && iRealAttenderNum < 5) {
        for (int32 i = 0; i < iRealAttenderNum; i++) {

            TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i];
            pAttender->setISRealAttender(true);
            pAttender->setIsStillInGame(true);
            aContainors.Emplace(pAttender->getpActionContainor());
        }
    }
    else {
        MY_VERIFY(false);
    }



    m_pActionCollector->resetForNewLoop(NULL, NULL, 0, false, 0);
    m_pActionCollector->reinit(aContainors, pPusher->m_iAttenderBehaviorRandomSelectMask);
    int32 uMask = genIdxAttenderStillInGameMaskOne(pPusher->m_cGameRunData.m_iIdxAttenderMenFeng);
    m_pActionCollector->resetForNewLoop(NULL, NULL, uMask, false, pPusher->m_cGameRunData.m_iIdxAttenderMenFeng);
    m_pActionCollector->setActionGroupId(0);


    m_iActionGroupId = 0;
    m_iGameId = pPusher->m_iGameId;
    m_iPusherId = 0;
    //m_iTurnId = 0;

    m_eGameState = MyMJGameStateCpp::CardsShuffled;
    m_eActionLoopState = MyMJActionLoopStateCpp::WaitingToGenAction;
    m_iMsLast = UMyMJUtilsLibrary::nowAsMsFromTick();

    m_iDiceNumberNow0 = -1;
    m_iDiceNumberNow1 = -1;
    m_cUntakenSlotInfo.reset();

    //let's fill in
    *m_pGameCfg = pPusher->m_cGameCfg;
    *m_pGameRunData = pPusher->m_cGameRunData;

    m_cCardPack.reset(pPusher->m_aShuffledIdValues);


    //move into untaken slots
    int32 perStack = m_pGameCfg->m_cTrivialCfg.m_iCardNumPerStackInUntakenSlot;
    MY_VERIFY(perStack > 0);

    const int32 cardCount = m_cCardPack.getLength();



    //do a guess to preserve memory
    int32 stackCount = cardCount / perStack;
    if ((cardCount % perStack) > 0) {
        stackCount++;
    }
    m_aUntakenCardStacks.Reset(stackCount);
    MY_VERIFY(m_aUntakenCardStacks.Num() == 0);

    int32 workingIdx = 0;

    FMyIdCollectionCpp *pC = NULL;
    while (workingIdx < cardCount) {
        int32 idx;
        if (pC == NULL || pC->m_aIds.Num() >= perStack) {
            idx = m_aUntakenCardStacks.Emplace();
            pC = &m_aUntakenCardStacks[idx];
        }

        idx = pC->m_aIds.Emplace(workingIdx);
        MY_VERIFY(pC->m_aIds[idx] == workingIdx);

        workingIdx++;
    }

    //let's recalc and arrange them
    stackCount = m_aUntakenCardStacks.Num();
    int32 stackArrayLAssumed = stackCount / 4;

    for (int32 i = 0; i < 4; i++) {
        int32 idxStart = i * stackArrayLAssumed;
        int32 stackL;

        if (i == 3) {
            stackL = stackCount - idxStart;
        }
        else {
            stackL = stackArrayLAssumed;
        }

        m_aAttendersAll[i]->getpUntakenSlotSubSegmentInfo()->m_iIdxStart = idxStart;
        m_aAttendersAll[i]->getpUntakenSlotSubSegmentInfo()->m_iLength = stackL;

        //Also update the card state
        for (int32 idxUntaken = idxStart; idxUntaken < (idxStart + stackL); idxUntaken++) {
            pC = &m_aUntakenCardStacks[idxUntaken];
            for (int32 idxInStack = 0; idxInStack < pC->m_aIds.Num(); idxInStack++) {
                int32 idxCard = pC->m_aIds[idxInStack];

                FMyMJCardCpp *pCard = m_cCardPack.getCardByIdx(idxCard);
                pCard->m_eFlipState = MyMJCardFlipStateCpp::Down;
                pCard->m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::Untaken;
                pCard->m_cPosi.m_iIdxAttender = i;
                pCard->m_cPosi.m_iIdxInSlot0 = idxUntaken;
                pCard->m_cPosi.m_iIdxInSlot1 = idxInStack;

            }
        }
    }

    m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal = cardCount;

    if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
        m_cCardPack.helperVerifyValues();
    }


    m_aHelperLastCardsGivenOutOrWeave.Reset();
    m_cHelperLastCardTakenInGame.reset(true);
    m_cHelperShowedOut2AllCards.clear();
}

void FMyMJGameCoreLocalCSCpp::applyPusherUpdateCards(FMyMJGamePusherUpdateCardsCpp *pPusher)
{
    FMyMJGameCoreCpp *pCore = this;
    FMyMJCardPackCpp *pCardPack = pCore->getpCardPack();

    int32 l = pPusher->m_aIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        FMyIdValuePair *pTargetIdValue = &pPusher->m_aIdValues[i];
        FMyMJCardCpp *pCard = pCardPack->getCardByIdx(pTargetIdValue->m_iId);

        if (pCard->m_eFlipState != pPusher->m_eTargetState) {
            pCard->m_eFlipState = pPusher->m_eTargetState;
        }

        pCardPack->revealCardValue(pTargetIdValue->m_iId, pTargetIdValue->m_iValue);
    }

    if (pPusher->m_iIdxAttender >= 0) {
        getRealAttenderByIdx(pPusher->m_iIdxAttender)->dataResetByMask(pPusher->m_iMaskAttenderDataReset);
    }
}


void FMyMJGameCoreLocalCSCpp::applyActionStateUpdate(FMyMJGameActionStateUpdateCpp *pAction)
{
    if (pAction->m_eStateNext != MyMJGameStateCpp::Invalid) {
        m_eGameState = pAction->m_eStateNext;

        if (m_pActionCollector.IsValid()) {
            m_pActionCollector->resetForNewLoop(NULL, NULL, pAction->m_iAttenderMaskNext, pAction->m_bAllowSamePriAction, pAction->m_iIdxAttenderHavePriMax);
        }
    }

    int32 iMask = pAction->m_iMask;
    if ((iMask & MyMJGameActionStateUpdateMaskNotResetHelperLastCardsGivenOutOrWeave) == 0) {
        m_aHelperLastCardsGivenOutOrWeave.Reset();
    }
}




void FMyMJGameCoreLocalCSCpp::applyActionThrowDices(FMyMJGameActionThrowDicesCpp *pAction)
{
    //idx is always 1 - 4
    int32 idxAttender = pAction->getIdxAttender();

    pAction->getDiceNumbers(m_iDiceNumberNow0, m_iDiceNumberNow1);

    MyMJGameActionThrowDicesSubTypeCpp eSubType = pAction->getSubType();

    if (eSubType == MyMJGameActionThrowDicesSubTypeCpp::GameStart) {
        int32 iBase = m_aAttendersAll[idxAttender]->getpUntakenSlotSubSegmentInfo()->m_iIdxStart;
        //int32 len = m_aUntakenCardStacks.Num();
        int32 len = m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal;
        MY_VERIFY(len > 0);
        m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow = (iBase + m_iDiceNumberNow0 + m_iDiceNumberNow1 - 1 + len) % len;
        m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow = m_cUntakenSlotInfo.m_iIdxUntakenSlotTailAtStart = (m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow - 1 + len) % len;

        int32 cardNumKept = m_pGameCfg->m_cTrivialCfg.m_iStackNumKeptFromTail * m_pGameCfg->m_cTrivialCfg.m_iCardNumPerStackInUntakenSlot;
        MY_VERIFY(calcUntakenSlotCardsLeftNumKeptFromTail() == cardNumKept);
        m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail = cardNumKept;
        m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead = m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal - m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail;

        m_eGameState = MyMJGameStateCpp::CardsWaitingForDistribution;

        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            //int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);
            int32 uMask = genIdxAttenderStillInGameMaskOne(idxAttender);
            m_pActionCollector->resetForNewLoop(NULL, NULL, uMask, false, idxAttender);
        }
    }
    else if (eSubType == MyMJGameActionThrowDicesSubTypeCpp::GangYaoLocalCS) {
        m_eGameState = MyMJGameStateCpp::WeavedGangDicesThrownLocalCS;

        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            int32 uMask = genIdxAttenderStillInGameMaskOne(idxAttender);
            m_pActionCollector->resetForNewLoop(NULL, NULL, uMask, false, idxAttender);
        }
    }
    else {
        MY_VERIFY(false);
    }
}

void FMyMJGameCoreLocalCSCpp::applyActionDistCardsAtStart(FMyMJGameActionDistCardAtStartCpp *pAction)
{
    int32 idxAttender = pAction->getIdxAttender();

    int32 l = pAction->m_aIdValues.Num();

    for (int32 i = 0; i < l; i++) {
        //1, reveal value
        int32 id = pAction->m_aIdValues[i].m_iId;
        int32 value = pAction->m_aIdValues[i].m_iValue;
        m_cCardPack.revealCardValue(id, value);

        //2, move card
        moveCardFromOldPosi(id);
        moveCardToNewPosi(id, idxAttender, MyMJCardSlotTypeCpp::InHand);
    }
    updateUntakenInfoHeadOrTail(true, false);

    //3rd, update state
    if (pAction->m_bLastCard) {

        getRealAttenderByIdx(idxAttender)->onNewTurn(false);

        m_eGameState = MyMJGameStateCpp::CardsDistributedWaitingForLittleHuLocalCS;
        int32 iMask = genIdxAttenderStillInGameMaskAll();
        int32 idxZhuang = m_pGameRunData->m_iIdxAttenderMenFeng;

        FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
        pActionUpdate->m_eStateNext = MyMJGameStateCpp::JustStarted;
        pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxZhuang);
        m_pActionCollector->resetForNewLoop(NULL, pActionUpdate, iMask, true, idxZhuang);

        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            for (int32 i = 0; i < 4; i++) {

                TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender = getAttenderByIdx(i);
                MY_VERIFY(pAttender.IsValid());
                if (i == idxZhuang || pAttender->getIsStillInGame() == false) {
                    continue;
                }
                MY_VERIFY(pAttender->getISRealAttender());
                pAttender->tryGenAndEnqueueUpdateTingPusher();
            }
        }

    }
    else {
        int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);
        int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
        m_pActionCollector->resetForNewLoop(NULL, NULL, iMask, false, idxAttenderNext);
    }
}

void FMyMJGameCoreLocalCSCpp::applyActionTakeCards(FMyMJGameActionTakeCardsCpp *pAction)
{
    int32 idxAttender = pAction->getIdxAttender();

    //1st, reveal value
    m_cCardPack.revealCardValueByIdValuePairs(pAction->m_aIdValuePairs);

    //2nd, move card and flip
    int32 l = pAction->m_aIdValuePairs.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJCardCpp *pCard = m_cCardPack.getCardByIdx(pAction->m_aIdValuePairs[i].m_iId);
        moveCardFromOldPosi(pCard->m_iId);
        moveCardToNewPosi(pCard->m_iId, idxAttender, MyMJCardSlotTypeCpp::JustTaken);

        pCard->m_eFlipState = MyMJCardFlipStateCpp::Stand;
    }

    MyMJGameCardTakenOrderCpp eTakenOrder = pAction->m_eTakenOrder;
    bool bUpdateHead = false, bUpdateTail = false;
    if (eTakenOrder == MyMJGameCardTakenOrderCpp::Head) {
        bUpdateHead = true;
    }
    else if (eTakenOrder == MyMJGameCardTakenOrderCpp::Tail) {
        bUpdateTail = true;
    }
    else if (eTakenOrder == MyMJGameCardTakenOrderCpp::NotFixed) {
        bUpdateHead = true;
        bUpdateTail = true;
    }
    else {
        MY_VERIFY(false);
    }
    updateUntakenInfoHeadOrTail(bUpdateHead, bUpdateTail);

    //3rd, update state
    getRealAttenderByIdx(idxAttender)->onNewTurn(false);

    if (pAction->m_bIsGang) {
        m_eGameState = MyMJGameStateCpp::WeavedGangTakenCards;
    }
    else {
        m_eGameState = MyMJGameStateCpp::TakenCard;
    }
    int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);

    m_pActionCollector->resetForNewLoop(NULL, NULL, iMask, false, idxAttender);

}

void FMyMJGameCoreLocalCSCpp::applyActionGiveOutCards(FMyMJGameActionGiveOutCardsCpp *pAction)
{
    int32 idxAttender = pAction->getIdxAttender();
    MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
    TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender = getRealAttenderByIdx(idxAttender);
    MY_VERIFY(pAttender.IsValid());

    int32 selectedCount = pAction->m_aIdValuePairsSelected.Num();
    MY_VERIFY(selectedCount > 0);

    //1st, update card value
    m_cCardPack.revealCardValueByIdValuePairs(pAction->m_aIdValuePairsSelected);

    //precheck if ting update is needed
    bool bNeedUpdateTing = false;
    
    if (pAction->m_bRestrict2SelectCardsJustTaken) {

    }
    else {
        MY_VERIFY(selectedCount == 1);
        int32 valueSelected = m_cCardPack.getCardByIdx(pAction->m_aIdValuePairsSelected[0].m_iId)->m_iValue;
        MY_VERIFY(valueSelected > 0);


        {
            TArray<int32> &aIdCardsTaken = pAttender->getIdJustTakenCardsRef();
            int32 l = aIdCardsTaken.Num();
            if (l > 0) {
                MY_VERIFY(l == 1);
                for (int32 i = 0; i < l; i++) {
                    if (valueSelected != m_cCardPack.getCardByIdx(aIdCardsTaken[i])->m_iValue) {
                        bNeedUpdateTing = true;
                        break;
                    }
                }
            }
            else {
                bNeedUpdateTing = true;
            }
        }
    }

    //2nd, move cards and flip
    for (int32 i = 0; i < selectedCount; i++) {
        FMyIdValuePair &pair = pAction->m_aIdValuePairsSelected[i];
        moveCardFromOldPosi(pair.m_iId);
        moveCardToNewPosi(pair.m_iId, pAction->getIdxAttender(), MyMJCardSlotTypeCpp::GivenOut);

        FMyMJCardCpp *pCard = m_cCardPack.getCardByIdx(pair.m_iId);
        pCard->m_eFlipState = MyMJCardFlipStateCpp::Up;
    }

    TArray<int32> aIdsJustTaken = pAttender->getIdJustTakenCardsRef(); //do a copy
    int32 l0 = aIdsJustTaken.Num();
    for (int32 i = 0; i < l0; i++) {
        int32 idCard = aIdsJustTaken[i];
        moveCardFromOldPosi(idCard);
        moveCardToNewPosi(idCard, idxAttender, MyMJCardSlotTypeCpp::InHand);
    }

    //3rd, update helper
    m_aHelperLastCardsGivenOutOrWeave.Reset();
    for (int32 i = 0; i < selectedCount; i++) {
        FMyIdValuePair &pair = pAction->m_aIdValuePairsSelected[i];
        m_aHelperLastCardsGivenOutOrWeave.Emplace(pair);

        m_cHelperShowedOut2AllCards.insert(pair);
    }


    //4th, update core state
    if (pAction->m_bIsGang) {
        m_eGameState = MyMJGameStateCpp::WeavedGangGivenOutCards;
    }
    else {
        m_eGameState = MyMJGameStateCpp::GivenOutCard;
    }
    int32 iMask = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
    int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);

    FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
    pActionUpdate->m_eStateNext = MyMJGameStateCpp::WaitingForTakeCard;
    pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
    m_pActionCollector->resetForNewLoop(pActionUpdate, NULL, iMask, m_pGameCfg->m_cSubLocalCSCfg.m_bHuAllowMultiple, idxAttenderNext);



    //5th, updateTing
    if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full && bNeedUpdateTing) {
        pAttender->tryGenAndEnqueueUpdateTingPusher();
    }


}

//TOdo: gang code path, and TArray member delete when using tmap

void FMyMJGameCoreLocalCSCpp::applyActionWeave(FMyMJGameActionWeaveCpp *pAction)
{
    int32 idxAttender = pAction->getIdxAttender();

    //1st, attender apply
    getRealAttenderByIdx(idxAttender)->applyActionWeave(pAction);

    MyMJWeaveTypeCpp eType = pAction->m_cWeave.getType();

    //2nd, update helper
    m_aHelperLastCardsGivenOutOrWeave.Reset();
    if (pAction->m_eTargetFlipState == MyMJCardFlipStateCpp::Up) {
        const TArray<FMyIdValuePair>& aIdValues = pAction->m_cWeave.getIdValuesRef();
        m_cHelperShowedOut2AllCards.insertIdValuePairs(aIdValues);
    }

    //3rd, update the state
    getRealAttenderByIdx(idxAttender)->onNewTurn(true);

    MyMJGameStateCpp eGameStateNow = MyMJGameStateCpp::Invalid;
    int32 iAttenderMaskNow = 0;
    bool bAllowSamePriActionNow = false;
    int32 iIdxAttenderHavePriMaxNow = 0;
    FMyMJGameActionStateUpdateCpp *pActionUpdatePrev = NULL;
    if (eType == MyMJWeaveTypeCpp::GangAn || eType == MyMJWeaveTypeCpp::GangMing) {
        int32 reserved0 = pAction->m_cWeave.getReserved0();
        if (reserved0 == FMyMJWeaveCppReserved0WhenGangValueGangYao) {
            if (eType == MyMJWeaveTypeCpp::GangMing && pAction->m_cWeave.getTypeConsumed() == MyMJWeaveTypeCpp::ShunZiMing) {
                //can be qiang, check first
                eGameStateNow = MyMJGameStateCpp::WeavedGang;
                iAttenderMaskNow = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
                bAllowSamePriActionNow = m_pGameCfg->m_cSubLocalCSCfg.m_bHuAllowMultiple;
                iIdxAttenderHavePriMaxNow = idxAttender;

                pActionUpdatePrev = new FMyMJGameActionStateUpdateCpp();
                pActionUpdatePrev->m_eStateNext = MyMJGameStateCpp::WeavedGangQiangGangChecked;
                pActionUpdatePrev->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxAttender);
                pActionUpdatePrev->m_bAllowSamePriAction = false;
                pActionUpdatePrev->m_iIdxAttenderHavePriMax = idxAttender;
            }
            else {
               //we can jump to next stage
                eGameStateNow = MyMJGameStateCpp::WeavedGangQiangGangChecked;
                iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
                bAllowSamePriActionNow = false;
                iIdxAttenderHavePriMaxNow = idxAttender;
                pActionUpdatePrev = NULL;
            }

        }
        else if (reserved0 == FMyMJWeaveCppReserved0WhenGangValueBuZhang) {
            eGameStateNow = MyMJGameStateCpp::WeavedGangSpBuZhangLocalCS;
            iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
            bAllowSamePriActionNow = false;
            iIdxAttenderHavePriMaxNow = idxAttender;
            pActionUpdatePrev = NULL;
        }
        else {
            MY_VERIFY(false);
        }

    }
    else if (eType == MyMJWeaveTypeCpp::KeZiMing || eType == MyMJWeaveTypeCpp::ShunZiMing) {
        eGameStateNow = MyMJGameStateCpp::WeavedNotGang;
        iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
        bAllowSamePriActionNow = false;
        iIdxAttenderHavePriMaxNow = idxAttender;
        pActionUpdatePrev = NULL;
    }
    else {
        MY_VERIFY(false);
    }

    //extra step to setup helper data
    FMyMJCardPackCpp *pCardPack = getpCardPack();
    FMyMJCardCpp *pCard = pCardPack->getCardByIdx(pAction->m_cWeave.getRepresentCardId());
    m_aHelperLastCardsGivenOutOrWeave.Reset();
    m_aHelperLastCardsGivenOutOrWeave.Emplace(*pCard);

    m_eGameState = eGameStateNow;

    m_pActionCollector->resetForNewLoop(pActionUpdatePrev, NULL, iAttenderMaskNow, bAllowSamePriActionNow, iIdxAttenderHavePriMaxNow);
}

void FMyMJGameCoreLocalCSCpp::applyActionHu(FMyMJGameActionHuCpp *pAction)
{
    int32 idxAttender = pAction->getIdxAttender();
    TSharedPtr<FMyMJGameAttenderCpp> pAttender = getRealAttenderByIdx(idxAttender);

    int32 l = pAction->m_aRevealingCards.Num();
    for (int32 i = 0; i < l; i++) {
        getpCardPack()->revealCardValue(pAction->m_aRevealingCards[i]);
    }

    for (int32 i = 0; i < 4; i++) {
        TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender2 = getAttenderByIdx(i);
        if (!pAttender2->getIsStillInGame()) {
            continue;
        }
        pAttender2->showOutCardsAfterHu();
    }


    //Update state
    m_aHelperLastCardsGivenOutOrWeave.Reset();

    if (pAction->m_bEndGame) {
        pAttender->getHuScoreResultFinalGroupRef() = pAction->m_cHuScoreResultFinalGroup;

        m_eGameState = MyMJGameStateCpp::JustHu;
        int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);

        FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
        pActionUpdate->m_eStateNext = MyMJGameStateCpp::GameEnd;
        pActionUpdate->m_iAttenderMaskNext = 0;
        pActionUpdate->m_eReason = MyMJGameStateUpdateReasonCpp::AttenderHu;

        m_pActionCollector->resetForNewLoop(NULL, pActionUpdate, iMask, false, idxAttender);

    }
    else {
        MY_VERIFY(false); //CS MJ always end game when hu in game
    }
}

void FMyMJGameCoreLocalCSCpp::applyActionZhaNiaoLocalCS(FMyMJGameActionZhaNiaoLocalCSCpp *pAction)
{
    FMyMJCardPackCpp *pCardPack = getpCardPack();

    pCardPack->revealCardValueByIdValuePairs(pAction->m_aPickedIdValues);

    int32 idxAttenderBase = pAction->getIdxAttender();
    MY_VERIFY(idxAttenderBase >= 0 && idxAttenderBase < 4);

    int32 l = pAction->m_aPickedIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        FMyIdValuePair &card = pAction->m_aPickedIdValues[i];
        int32 cardId = card.m_iId;
        int32 cardValue = card.m_iValue;
        int32 modVFix = cardValue % 10 - 1;
        MY_VERIFY(modVFix >= 0 && modVFix < 9);

        int32 idxAttenderTarget = findIdxAttenderStillInGame(idxAttenderBase, modVFix, false);
        moveCardFromOldPosi(cardId);
        moveCardToNewPosi(cardId, idxAttenderTarget, MyMJCardSlotTypeCpp::WinSymbol); //We use WinSymbol slot as niao in local CS game

        m_cHelperShowedOut2AllCards.insert(card);
    }

    //Update state
    m_aHelperLastCardsGivenOutOrWeave.Reset();
}