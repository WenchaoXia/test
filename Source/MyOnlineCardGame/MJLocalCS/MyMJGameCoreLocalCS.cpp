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
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    if (pCmd->m_eType == MyMJGameCmdType::RestartGame) {
        if (eRoleTypeOfCmdSrc == MyMJGameRoleTypeCpp::SysKeeper) {

            FMyMJGameCmdRestartGameCpp *pCmdRestartGame = StaticCast<FMyMJGameCmdRestartGameCpp *>(pCmd);

            MY_VERIFY(m_eRuleType != MyMJGameRuleTypeCpp::Invalid);
            if (pCmdRestartGame->m_cGameCfg.m_eRuleType == m_eRuleType) {
                FMyMJGamePusherResetGameCpp p, *pPusherReset = &p;
                FRandomStream * pRS = m_pResManager->getpRandomStream();
                int32 iGameId = pD->m_iGameId + 1;
                if (iGameId < 0) {
                    iGameId = 0;
                }
                pPusherReset->init(iGameId, pRS, pCmdRestartGame->m_cGameCfg, pCmdRestartGame->m_cGameRunData, pCmdRestartGame->m_iAttenderRandomSelectMask);

                m_pPusherIOFull->EnqueuePusher(*pPusherReset);

                pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::None;
            }
            else {
                pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::GameRuleTypeNotEqual;
            }
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
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    if (pPusher->m_bActionGroupIncrease) {
        pD->m_iActionGroupId++;
        //if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full)) {
            //MY_VERIFY(m_pActionCollector.IsValid());
            m_pActionCollector->setActionGroupId(pD->m_iActionGroupId);
        //}
    }
}

void FMyMJGameCoreLocalCSCpp::applyPusherResetGame(FMyMJGamePusherResetGameCpp *pPusher)
{
    //Stateless, never judge condition of previous state, just reset the whole core
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack  = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();
    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

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

    MY_VERIFY(m_eRuleType == pPusher->m_cGameCfg.m_eRuleType)


    m_pActionCollector->resetForNewLoop(NULL, NULL, 0, false, 0);
    m_pActionCollector->reinit(aContainors, pPusher->m_iAttenderBehaviorRandomSelectMask);
    int32 uMask = genIdxAttenderStillInGameMaskOne(pPusher->m_cGameRunData.m_iIdxAttenderMenFeng);
    m_pActionCollector->resetForNewLoop(NULL, NULL, uMask, false, pPusher->m_cGameRunData.m_iIdxAttenderMenFeng);
    m_pActionCollector->setActionGroupId(0);

    pD->reset();
    pD->m_iActionGroupId = 0;
    pD->m_iGameId = pPusher->m_iGameId;
    pD->m_iPusherIdLast = 0;
    //m_iTurnId = 0;

    pD->m_eGameState = MyMJGameStateCpp::CardsShuffled;
    pD->m_eActionLoopState = MyMJActionLoopStateCpp::WaitingToGenAction;
    m_iMsLast = UMyMJUtilsLibrary::nowAsMsFromTick();

    pD->m_iDiceNumberNow0 = -1;
    pD->m_iDiceNumberNow1 = -1;
    pD->m_cUntakenSlotInfo.reset();

    //let's fill in
    pD->m_cGameCfg = pPusher->m_cGameCfg;
    pD->m_cGameRunData = pPusher->m_cGameRunData;

    pCardInfoPack->reset(pPusher->m_aShuffledValues.Num());
    pCardValuePack->reset(pPusher->m_aShuffledValues);


    //move into untaken slots
    int32 perStack = pD->m_cGameCfg.m_cTrivialCfg.m_iCardNumPerStackInUntakenSlot;
    MY_VERIFY(perStack > 0);

    const int32 cardCount = pCardInfoPack->getLength();



    //do a guess to preserve memory
    int32 stackCount = cardCount / perStack;
    if ((cardCount % perStack) > 0) {
        stackCount++;
    }
    pD->m_aUntakenCardStacks.Reset(stackCount);
    MY_VERIFY(pD->m_aUntakenCardStacks.Num() == 0);

    int32 workingIdx = 0;

    FMyIdCollectionCpp *pC = NULL;
    while (workingIdx < cardCount) {
        int32 idx;
        if (pC == NULL || pC->m_aIds.Num() >= perStack) {
            idx = pD->m_aUntakenCardStacks.Emplace();
            pC = &pD->m_aUntakenCardStacks[idx];
        }

        idx = pC->m_aIds.Emplace(workingIdx);
        MY_VERIFY(pC->m_aIds[idx] == workingIdx);

        workingIdx++;
    }

    //let's recalc and arrange them
    stackCount = pD->m_aUntakenCardStacks.Num();
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
            pC = &pD->m_aUntakenCardStacks[idxUntaken];
            for (int32 idxInStack = 0; idxInStack < pC->m_aIds.Num(); idxInStack++) {
                int32 idxCard = pC->m_aIds[idxInStack];

                FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(idxCard);
                pCardInfo->m_eFlipState = MyMJCardFlipStateCpp::Down;
                pCardInfo->m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::Untaken;
                pCardInfo->m_cPosi.m_iIdxAttender = i;
                pCardInfo->m_cPosi.m_iIdxInSlot0 = idxUntaken;
                pCardInfo->m_cPosi.m_iIdxInSlot1 = idxInStack;

            }
        }
    }

    pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal = cardCount;

    if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
        pCardInfoPack->helperVerifyInfos();
        pCardValuePack->helperVerifyValues();
    }


    pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    pD->m_cHelperLastCardTakenInGame.reset(true);
    pD->m_cHelperShowedOut2AllCards.clear();
}

void FMyMJGameCoreLocalCSCpp::applyPusherUpdateCards(FMyMJGamePusherUpdateCardsCpp *pPusher)
{
    FMyMJCardInfoPackCpp  *pCardInfoPack = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();
    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    int32 l = pPusher->m_aIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        FMyIdValuePair *pTargetIdValue = &pPusher->m_aIdValues[i];
        FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(pTargetIdValue->m_iId);

        if (pCardInfo->m_eFlipState != pPusher->m_eTargetState) {
            pCardInfo->m_eFlipState = pPusher->m_eTargetState;
        }

        //pCardValuePack->revealCardValue(*pTargetIdValue);
    }

    if (pPusher->m_iIdxAttender >= 0) {
        getRealAttenderByIdx(pPusher->m_iIdxAttender)->dataResetByMask(pPusher->m_iMaskAttenderDataReset);
    }
}


void FMyMJGameCoreLocalCSCpp::applyActionStateUpdate(FMyMJGameActionStateUpdateCpp *pAction)
{
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    if (pAction->m_eStateNext != MyMJGameStateCpp::Invalid) {
        pD->m_eGameState = pAction->m_eStateNext;

        if (m_pActionCollector.IsValid()) {
            m_pActionCollector->resetForNewLoop(NULL, NULL, pAction->m_iAttenderMaskNext, pAction->m_bAllowSamePriAction, pAction->m_iIdxAttenderHavePriMax);
        }
    }

    int32 iMask = pAction->m_iMask;
    if ((iMask & MyMJGameActionStateUpdateMaskNotResetHelperLastCardsGivenOutOrWeave) == 0) {
        pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    }
}




void FMyMJGameCoreLocalCSCpp::applyActionThrowDices(FMyMJGameActionThrowDicesCpp *pAction)
{
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    //idx is always 1 - 4
    int32 idxAttender = pAction->getIdxAttender();

    pAction->getDiceNumbers(pD->m_iDiceNumberNow0, pD->m_iDiceNumberNow1);

    MyMJGameActionThrowDicesSubTypeCpp eSubType = pAction->getSubType();

    if (eSubType == MyMJGameActionThrowDicesSubTypeCpp::GameStart) {
        int32 iBase = m_aAttendersAll[idxAttender]->getpUntakenSlotSubSegmentInfo()->m_iIdxStart;
        //int32 len = m_aUntakenCardStacks.Num();
        int32 len = pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal;
        MY_VERIFY(len > 0);
        pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow = (iBase + pD->m_iDiceNumberNow0 + pD->m_iDiceNumberNow1 - 1 + len) % len;
        pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow = pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailAtStart = (pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow - 1 + len) % len;

        int32 cardNumKept = pD->m_cGameCfg.m_cTrivialCfg.m_iStackNumKeptFromTail * pD->m_cGameCfg.m_cTrivialCfg.m_iCardNumPerStackInUntakenSlot;
        MY_VERIFY(calcUntakenSlotCardsLeftNumKeptFromTail() == cardNumKept);
        pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail = cardNumKept;
        pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead = pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal - pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail;

        pD->m_eGameState = MyMJGameStateCpp::CardsWaitingForDistribution;

        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            //int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);
            int32 uMask = genIdxAttenderStillInGameMaskOne(idxAttender);
            m_pActionCollector->resetForNewLoop(NULL, NULL, uMask, false, idxAttender);
        }
    }
    else if (eSubType == MyMJGameActionThrowDicesSubTypeCpp::GangYaoLocalCS) {
        pD->m_eGameState = MyMJGameStateCpp::WeavedGangDicesThrownLocalCS;

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
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    int32 idxAttender = pAction->getIdxAttender();

    int32 l = pAction->m_aIdValues.Num();

    for (int32 i = 0; i < l; i++) {
        //1, reveal value
        int32 id = pAction->m_aIdValues[i].m_iId;
        int32 value = pAction->m_aIdValues[i].m_iValue;
        //pCardValuePack->revealCardValue(id, value);

        //2, move card
        moveCardFromOldPosi(id);
        moveCardToNewPosi(id, idxAttender, MyMJCardSlotTypeCpp::InHand);
    }
    updateUntakenInfoHeadOrTail(true, false);

    //3rd, update state
    if (pAction->m_bLastCard) {

        getRealAttenderByIdx(idxAttender)->onNewTurn(false);

        pD->m_eGameState = MyMJGameStateCpp::CardsDistributedWaitingForLittleHuLocalCS;
        int32 iMask = genIdxAttenderStillInGameMaskAll();
        int32 idxZhuang = pD->m_cGameRunData.m_iIdxAttenderMenFeng;

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
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    int32 idxAttender = pAction->getIdxAttender();

    //1st, reveal value
    //m_cCardPack.revealCardValueByIdValuePairs(pAction->m_aIdValuePairs);

    //2nd, move card and flip
    int32 l = pAction->m_aIdValuePairs.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(pAction->m_aIdValuePairs[i].m_iId);
        moveCardFromOldPosi(pCardInfo->m_iId);
        moveCardToNewPosi(pCardInfo->m_iId, idxAttender, MyMJCardSlotTypeCpp::JustTaken);

        pCardInfo->m_eFlipState = MyMJCardFlipStateCpp::Stand;
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
        pD->m_eGameState = MyMJGameStateCpp::WeavedGangTakenCards;
    }
    else {
        pD->m_eGameState = MyMJGameStateCpp::TakenCard;
    }
    int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);

    m_pActionCollector->resetForNewLoop(NULL, NULL, iMask, false, idxAttender);

}

void FMyMJGameCoreLocalCSCpp::applyActionGiveOutCards(FMyMJGameActionGiveOutCardsCpp *pAction)
{
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    int32 idxAttender = pAction->getIdxAttender();
    MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
    TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender = getRealAttenderByIdx(idxAttender);
    MY_VERIFY(pAttender.IsValid());

    int32 selectedCount = pAction->m_aIdValuePairsSelected.Num();
    MY_VERIFY(selectedCount > 0);

    //1st, update card value
    //m_cCardPack.revealCardValueByIdValuePairs(pAction->m_aIdValuePairsSelected);

    //precheck if ting update is needed
    bool bNeedUpdateTing = false;
    
    if (pAction->m_bRestrict2SelectCardsJustTaken) {

    }
    else {
        MY_VERIFY(selectedCount == 1);
        int32 valueSelected = pCardValuePack->getByIdx(pAction->m_aIdValuePairsSelected[0].m_iId);
        MY_VERIFY(valueSelected > 0);


        {
            TArray<int32> &aIdCardsTaken = pAttender->getIdJustTakenCardsRef();
            int32 l = aIdCardsTaken.Num();
            if (l > 0) {
                MY_VERIFY(l == 1);
                for (int32 i = 0; i < l; i++) {
                    if (valueSelected != pCardValuePack->getByIdx(aIdCardsTaken[i])) {
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

        FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(pair.m_iId);
        pCardInfo->m_eFlipState = MyMJCardFlipStateCpp::Up;
    }

    TArray<int32> aIdsJustTaken = pAttender->getIdJustTakenCardsRef(); //do a copy
    int32 l0 = aIdsJustTaken.Num();
    for (int32 i = 0; i < l0; i++) {
        int32 idCard = aIdsJustTaken[i];
        moveCardFromOldPosi(idCard);
        moveCardToNewPosi(idCard, idxAttender, MyMJCardSlotTypeCpp::InHand);
    }

    //3rd, update helper
    pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    for (int32 i = 0; i < selectedCount; i++) {
        FMyIdValuePair &pair = pAction->m_aIdValuePairsSelected[i];
        pD->m_aHelperLastCardsGivenOutOrWeave.Emplace(pair);

        pD->m_cHelperShowedOut2AllCards.insert(pair);
    }


    //4th, update core state
    if (pAction->m_bIsGang) {
        pD->m_eGameState = MyMJGameStateCpp::WeavedGangGivenOutCards;
    }
    else {
        pD->m_eGameState = MyMJGameStateCpp::GivenOutCard;
    }
    int32 iMask = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
    int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);

    FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
    pActionUpdate->m_eStateNext = MyMJGameStateCpp::WaitingForTakeCard;
    pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
    m_pActionCollector->resetForNewLoop(pActionUpdate, NULL, iMask, pD->m_cGameCfg.m_cSubLocalCSCfg.m_bHuAllowMultiple, idxAttenderNext);



    //5th, updateTing
    if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full && bNeedUpdateTing) {
        pAttender->tryGenAndEnqueueUpdateTingPusher();
    }


}

//TOdo: gang code path, and TArray member delete when using tmap

void FMyMJGameCoreLocalCSCpp::applyActionWeave(FMyMJGameActionWeaveCpp *pAction)
{
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    int32 idxAttender = pAction->getIdxAttender();

    //1st, attender apply
    getRealAttenderByIdx(idxAttender)->applyActionWeave(pAction);

    MyMJWeaveTypeCpp eType = pAction->m_cWeave.getType();

    //2nd, update helper
    pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    if (pAction->m_eTargetFlipState == MyMJCardFlipStateCpp::Up) {
        const TArray<FMyIdValuePair>& aIdValues = pAction->m_cWeave.getIdValuesRef();
        pD->m_cHelperShowedOut2AllCards.insertIdValuePairs(aIdValues);
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
        bool bIsBuZhang = (reserved0 & (uint8)EMyMJWeaveReserved0Mask::LocalCSGangBuZhang) > 0;

        if (!bIsBuZhang) {
            if (eType == MyMJWeaveTypeCpp::GangMing && pAction->m_cWeave.getTypeConsumed() == MyMJWeaveTypeCpp::ShunZiMing) {
                //can be qiang, check first
                eGameStateNow = MyMJGameStateCpp::WeavedGang;
                iAttenderMaskNow = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
                bAllowSamePriActionNow = pD->m_cGameCfg.m_cSubLocalCSCfg.m_bHuAllowMultiple;
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
        else {
            eGameStateNow = MyMJGameStateCpp::WeavedGangSpBuZhangLocalCS;
            iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
            bAllowSamePriActionNow = false;
            iIdxAttenderHavePriMaxNow = idxAttender;
            pActionUpdatePrev = NULL;
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
    FMyMJCardInfoPackCpp  *pCardInfoPack = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();

    FMyIdValuePair cIdValue;
    cIdValue.m_iId = pAction->m_cWeave.getRepresentCardId();
    cIdValue.m_iValue = pCardValuePack->getByIdx(cIdValue.m_iId);

    pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    pD->m_aHelperLastCardsGivenOutOrWeave.Emplace(cIdValue);

    pD->m_eGameState = eGameStateNow;

    m_pActionCollector->resetForNewLoop(pActionUpdatePrev, NULL, iAttenderMaskNow, bAllowSamePriActionNow, iIdxAttenderHavePriMaxNow);
}

void FMyMJGameCoreLocalCSCpp::applyActionHu(FMyMJGameActionHuCpp *pAction)
{
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    int32 idxAttender = pAction->getIdxAttender();
    TSharedPtr<FMyMJGameAttenderCpp> pAttender = getRealAttenderByIdx(idxAttender);

    int32 l = pAction->m_aRevealingCards.Num();
    for (int32 i = 0; i < l; i++) {
        //getpCardPack()->revealCardValue(pAction->m_aRevealingCards[i]);
    }

    for (int32 i = 0; i < 4; i++) {
        TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender2 = getAttenderByIdx(i);
        if (!pAttender2->getIsStillInGame()) {
            continue;
        }
        pAttender2->showOutCardsAfterHu();
    }


    //Update state
    pD->m_aHelperLastCardsGivenOutOrWeave.Reset();

    if (pAction->m_bEndGame) {
        pAttender->getHuScoreResultFinalGroupRef() = pAction->m_cHuScoreResultFinalGroup;

        pD->m_eGameState = MyMJGameStateCpp::JustHu;
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
    FMyMJCoreDataDirectPublicCpp *pD = getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    //pCardPack->revealCardValueByIdValuePairs(pAction->m_aPickedIdValues);

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

        pD->m_cHelperShowedOut2AllCards.insert(card);
    }

    //Update state
    pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
}