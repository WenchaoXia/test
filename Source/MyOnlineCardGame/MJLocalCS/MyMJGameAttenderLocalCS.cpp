// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameAttenderLocalCS.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

#include "MyMJGameCoreLocalCS.h"

void FMyMJGameAttenderLocalCSCpp::genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO)
{
    FMyMJGameCoreCpp *pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCardInfoPackCpp  *pCardInfoPack = pCore->getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = pCore->getpCardValuePack();

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);
    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    MyMJGameStateCpp eGameState = pD->m_eGameState;
    FMyMJGameCfgCpp  *pGameCfg = &pD->m_cGameCfg;

    FMyMJGamePusherFillInActionChoicesCpp iFillInPusher, *pFillInPusher = &iFillInPusher;
    pFillInPusher->init(pD->m_iActionGroupId, m_iIdx);

    if (eGameState == MyMJGameStateCpp::CardsShuffled) {
        FMyMJGameActionThrowDicesCpp *pAction = new FMyMJGameActionThrowDicesCpp();
        pFillInPusher->m_cActionChoices.give(pAction);
        pAction->init(MyMJGameActionThrowDicesSubTypeCpp::GameStart, m_iIdx, pCore->getpResManager()->getpRandomStream());
    }
    else if (eGameState == MyMJGameStateCpp::CardsWaitingForDistribution)
    {
        int32 idxHeadNow;
        TArray<int32> aIds;
        bool bIsLast = false;
        idxHeadNow = pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow;

        int32 handCardCount = m_cHandCards.getCount();
        if (handCardCount < 12) {
            pCore->collectCardsFromUntakenSlot(idxHeadNow, 4, false, aIds);
        }
        else {
            int32 idxAttenderZhuang = pD->m_cGameRunData.m_iIdxAttenderMenFeng;
            int32 idxAttenderZhuangPre = pCore->findIdxAttenderStillInGame(idxAttenderZhuang, 1, true);
            MY_VERIFY(idxAttenderZhuang != idxAttenderZhuangPre);
            if (m_iIdx == idxAttenderZhuang) {
                pCore->collectCardsFromUntakenSlot(idxHeadNow, 1, false, aIds);
                int32 idxHeadNowPlus2 = pCore->getIdxOfUntakenSlotHavingCard(idxHeadNow, 2, false);
                TArray<int32> aIds2;
                pCore->collectCardsFromUntakenSlot(idxHeadNowPlus2, 1, false, aIds2);
                aIds.Append(aIds2);

                MY_VERIFY(aIds.Num() == 2);
            }
            else if (m_iIdx == idxAttenderZhuangPre) {
                pCore->collectCardsFromUntakenSlot(idxHeadNow, 1, false, aIds);
                bIsLast = true;
            }
            else {
                pCore->collectCardsFromUntakenSlot(idxHeadNow, 1, false, aIds);
            }
        }

        TArray<FMyIdValuePair> aIdValuePairs;
        pCardValuePack->getIdValuePairsByIds(aIds, aIdValuePairs);

        //OK, all data collecteed, let's gen

        FMyMJGameActionDistCardAtStartCpp *pAction = new FMyMJGameActionDistCardAtStartCpp();
        pFillInPusher->m_cActionChoices.give(pAction);
        pAction->init(m_iIdx, aIdValuePairs, bIsLast);
    }
    else if (eGameState == MyMJGameStateCpp::JustStarted) {

        FMyMJHuActionAttrCpp huActionAttr;
        assembleHuActionAttr(-1, NULL, MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithOutTriggerCard(pGameCfg->m_cHuCfg, pGameCfg->m_cSubLocalCSCfg, m_aShowedOutWeaves, m_cHandCards, huActionAttr, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            pAction->m_cHuScoreResultFinalGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(m_iIdx, true);
            pFillInPusher->m_cActionChoices.give(pAction);
        }
       
        TArray<FMyMJGameActionWeaveCpp> aActionGangs;
        checkGang(NULL, false, aActionGangs);
        int32 l = aActionGangs.Num();
        for (int32 i = 0; i < l; i++) {
            FMyMJGameActionWeaveCpp *newAction = new FMyMJGameActionWeaveCpp();
            *newAction = aActionGangs[i];
            pFillInPusher->m_cActionChoices.give(newAction);
        }

        MY_VERIFY(!m_bGangYaoedLocalCS);
        pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(false, false));
    }
    else if (eGameState == MyMJGameStateCpp::JustHu) {
        int32 idxHeadNow = pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow;
        TArray<int32> aIds;

        int32 c = pGameCfg->m_cSubLocalCSCfg.m_iZhaNiaoCount;
        if (c <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("zhaniao count cfg <= 0."));
            return;
        }
        pCore->tryCollectCardsFromUntakenSlot(idxHeadNow, (uint32)c, false, aIds);
        c = aIds.Num();

        FMyMJGameActionZhaNiaoLocalCSCpp *pAction = new FMyMJGameActionZhaNiaoLocalCSCpp();
        if (c > 0) {

            for (int32 i = 0; i < c; i++) {
                int32 idx = pAction->m_aPickedIdValues.Emplace();
                pAction->m_aPickedIdValues[idx].m_iId = aIds[i];
                pAction->m_aPickedIdValues[idx].m_iValue = pCardValuePack->getByIdx(aIds[i]);
            }

        }
        else {
            //this is the last card
            const FMyIdValuePair& lastCard = pD->m_cHelperLastCardTakenInGame;
            MY_VERIFY(lastCard.m_iId >= 0);

            int32 idx = pAction->m_aPickedIdValues.Emplace();
            pAction->m_aPickedIdValues[idx] = lastCard;
        }

        pAction->initWithPickedIdValuesInited(m_iIdx);
        pFillInPusher->m_cActionChoices.give(pAction);

    }
    else if (eGameState == MyMJGameStateCpp::WaitingForTakeCard) {

        const FMyMJGameUntakenSlotInfoCpp *pUntakenInfo = &pD->m_cUntakenSlotInfo;
        int32 cardNumNormal = pUntakenInfo->getCardNumCanBeTakenNormally();
        if (cardNumNormal > 0) {
            FMyMJGameActionTakeCardsCpp *pAction = new FMyMJGameActionTakeCardsCpp();

            TArray<int32> aIds;
            int32 idxHead = pUntakenInfo->m_iIdxUntakenSlotHeadNow;
            pCore->collectCardsFromUntakenSlot(idxHead, 1, true, aIds);
            pCardValuePack->helperIds2IdValuePairs(aIds, pAction->m_aIdValuePairs);

            pAction->initWithIdValuePairsInited(getIdx(), false, MyMJGameCardTakenOrderCpp::Head);

            //debug
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("gen taken cards: %s"), *UMyMJUtilsLibrary::formatStrIdValuePairs(pAction->m_aIdValuePairs));

            pFillInPusher->m_cActionChoices.give(pAction);
        }
        else {
            //Todo, let's end game
            FMyMJGameActionStateUpdateCpp actionUpdate;
            FMyMJGameActionStateUpdateCpp *pActionUpdate = &actionUpdate;
            pActionUpdate->m_eStateNext = MyMJGameStateCpp::GameEnd;
            pActionUpdate->m_iAttenderMaskNext = 0;
            pActionUpdate->m_eReason = MyMJGameStateUpdateReasonCpp::NoCardLeft;

            pCore->getpPusherIOFull()->EnqueuePusher(*pActionUpdate);
        }

    }
    else if (eGameState == MyMJGameStateCpp::TakenCard) {
        FMyMJHuActionAttrCpp huActionAttr;
        TArray<FMyIdValuePair> aTriggerCards;
        pCardValuePack->helperIds2IdValuePairs(m_aIdJustTakenCards, aTriggerCards);
        assembleHuActionAttr(-1, &aTriggerCards, MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(pGameCfg->m_cHuCfg, pGameCfg->m_cSubLocalCSCfg, m_aShowedOutWeaves, m_cHandCards, huActionAttr, m_cHuScoreResultTingGroup, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            pAction->m_cHuScoreResultFinalGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(m_iIdx, true);
            pFillInPusher->m_cActionChoices.give(pAction);
        }

        bool bLastCardOfGame = (pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead <= 0);

        if (bLastCardOfGame) {
            pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(true, false));
        }
        else {
            TArray<FMyMJGameActionWeaveCpp> aActionGangs;
            checkGang(NULL, false, aActionGangs);
            int32 l = aActionGangs.Num();
            for (int32 i = 0; i < l; i++) {
                FMyMJGameActionWeaveCpp *newAction = new FMyMJGameActionWeaveCpp();
                *newAction = aActionGangs[i];
                pFillInPusher->m_cActionChoices.give(newAction);
            }

            pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(m_bGangYaoedLocalCS, false));
        }
    }
    else if (eGameState == MyMJGameStateCpp::GivenOutCard) {

        genActionAfterGivenOutCards(pFillInPusher, false);

    }
    else if (eGameState == MyMJGameStateCpp::WeavedNotGang) {
        bool bLastCardOfGame = (pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead <= 0);
        MY_VERIFY(!bLastCardOfGame);
        MY_VERIFY(!m_bGangYaoedLocalCS);

        pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(m_bGangYaoedLocalCS, false));
    }
    else if (eGameState == MyMJGameStateCpp::WeavedGang) {

        //check whether can qiang gang hu

        const TArray<FMyIdValuePair>& lastCards = pD->m_aHelperLastCardsGivenOutOrWeave;
        MY_VERIFY(lastCards.Num() > 0);
        int32 idxAttenderTrigger = pCardInfoPack->getByIdx(lastCards[0].m_iId)->m_cPosi.m_iIdxAttender;
        FMyMJHuActionAttrCpp huActionAttr;
        assembleHuActionAttr(idxAttenderTrigger, &lastCards, MyMJHuTriggerCardSrcTypeCpp::GangWeaveShowedOut, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(pGameCfg->m_cHuCfg, pGameCfg->m_cSubLocalCSCfg, m_aShowedOutWeaves, m_cHandCards, huActionAttr, m_cHuScoreResultTingGroup, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            pAction->m_cHuScoreResultFinalGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(m_iIdx, true);
            pFillInPusher->m_cActionChoices.give(pAction);

            FMyMJGameActionNoActCpp *pActionNoAct = new FMyMJGameActionNoActCpp();
            pActionNoAct->init(m_iIdx, MyMJGameActionNoActreserved0MaskPassPaoHu);
            pFillInPusher->m_cActionChoices.give(pActionNoAct);
        }


    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangQiangGangChecked) {
        FMyMJGameActionThrowDicesCpp *pAction = new FMyMJGameActionThrowDicesCpp();
        pFillInPusher->m_cActionChoices.give(pAction);
        pAction->init(MyMJGameActionThrowDicesSubTypeCpp::GangYaoLocalCS, m_iIdx, pCore->getpResManager()->getpRandomStream());
    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangTakenCards) {
        //Check hu
        {
            FMyMJHuActionAttrCpp huActionAttr;
            TArray<FMyIdValuePair> aTriggerCards;
            pCardValuePack->helperIds2IdValuePairs(m_aIdJustTakenCards, aTriggerCards);
            assembleHuActionAttr(-1, &aTriggerCards, MyMJHuTriggerCardSrcTypeCpp::GangCardTaken, huActionAttr);

            FMyMJHuScoreResultFinalGroupCpp resultGroup;
            if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(pGameCfg->m_cHuCfg, pGameCfg->m_cSubLocalCSCfg, m_aShowedOutWeaves, m_cHandCards, huActionAttr, m_cHuScoreResultTingGroup, resultGroup)) {
                FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
                pAction->m_cHuScoreResultFinalGroup = resultGroup;
                pAction->initWithFinalGroupAlreadyInited(m_iIdx, true);
                pFillInPusher->m_cActionChoices.give(pAction);
            }
        }

        //or give out
        pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(true, true));

    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangGivenOutCards) {

        genActionAfterGivenOutCards(pFillInPusher, true);
    }

    else if (eGameState == MyMJGameStateCpp::CardsDistributedWaitingForLittleHuLocalCS) {

        TArray<FMyMJHuScoreResultItemCpp> huScoreResultItems;
        TArray<int32> aIdsShowOutCards;

        FMyMJGameCfgCpp *pCfg = &pD->m_cGameCfg;
        bool bHu = UMyMJUtilsLocalCSLibrary::checkHuLocalCSBorn(pCfg->m_cSubLocalCSCfg,
                                                            m_cHandCards, huScoreResultItems, aIdsShowOutCards);
        if (bHu) {
            TArray<FMyIdValuePair> aShowOutIdValuePairs;
            pCardValuePack->getIdValuePairsByIds(aIdsShowOutCards, aShowOutIdValuePairs);
            FMyMJGameActionHuBornLocalCSCpp *pAction = new FMyMJGameActionHuBornLocalCSCpp();
            pAction->init(m_iIdx, huScoreResultItems, aShowOutIdValuePairs);
            pFillInPusher->m_cActionChoices.give(pAction);

            FMyMJGameActionNoActCpp *pActionNoAct = new FMyMJGameActionNoActCpp();
            pActionNoAct->init(m_iIdx, 0);
            pFillInPusher->m_cActionChoices.give(pActionNoAct);
        }

    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangSpBuZhangLocalCS) {
        const FMyMJGameUntakenSlotInfoCpp *pUntakenInfo = &pD->m_cUntakenSlotInfo;
        int32 cardLeftNum = pUntakenInfo->getCardNumCanBeTakenAll();
        MY_VERIFY(cardLeftNum > 0);

        FMyMJGameActionTakeCardsCpp *pAction = new FMyMJGameActionTakeCardsCpp();

        TArray<int32> aIds;
        int32 idxTail = pUntakenInfo->m_iIdxUntakenSlotTailNow;
        pCore->collectCardsFromUntakenSlot(idxTail, 1, true, aIds);
        pCardValuePack->helperIds2IdValuePairs(aIds, pAction->m_aIdValuePairs);

        pAction->initWithIdValuePairsInited(getIdx(), false, MyMJGameCardTakenOrderCpp::Tail);

        pFillInPusher->m_cActionChoices.give(pAction);
    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangDicesThrownLocalCS) {

        const FMyMJGameUntakenSlotInfoCpp *pUntakenInfo = &pD->m_cUntakenSlotInfo;
        int32 cardLeftNum = pUntakenInfo->getCardNumCanBeTakenAll();
        MY_VERIFY(cardLeftNum > 0);

        int32 c = pGameCfg->m_cSubLocalCSCfg.m_iGangYaoCount;
        MY_VERIFY(c >= 0);
        TArray<int32> aIds;
        int32 idxTail = pUntakenInfo->m_iIdxUntakenSlotTailNow;
        pCore->tryCollectCardsFromUntakenSlot(idxTail, c, true, aIds);
        MY_VERIFY(aIds.Num() > 0);

        FMyMJGameActionTakeCardsCpp *pAction = new FMyMJGameActionTakeCardsCpp();
        pCardValuePack->helperIds2IdValuePairs(aIds, pAction->m_aIdValuePairs);

        pAction->initWithIdValuePairsInited(getIdx(), true, MyMJGameCardTakenOrderCpp::NotFixed);

        pFillInPusher->m_cActionChoices.give(pAction);
    }


    if (pFillInPusher->m_cActionChoices.getCount() > 0) {
        pPusherIO->EnqueuePusher(*pFillInPusher);
    }
}

bool FMyMJGameAttenderLocalCSCpp::tryGenAndEnqueueUpdateTingPusher()
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);


    int32 handCardCount = m_cHandCards.getCount();
    if ((handCardCount % 3) != 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("handCardCount: %d"), handCardCount);
        MY_VERIFY(false);
    }

    FMyMJGamePusherUpdateTingCpp pusherUpdateTing;
    bool bTing = UMyMJUtilsLocalCSLibrary::checkTingsLocalCSInGame(pD->m_cGameCfg, *pCore->getpCardValuePack(), m_aShowedOutWeaves, m_cHandCards, pusherUpdateTing.m_cTingGroup);

    if (m_cHuScoreResultTingGroup.getCount() == pusherUpdateTing.m_cTingGroup.getCount() && m_cHuScoreResultTingGroup.getCount() == 0) {
        //not ting yet, we don't need to gen pusher
        return bTing;
    }

    const FMyMJValueIdMapCpp *pHelperShowedOut2AllCards = &pD->m_cHelperShowedOut2AllCards;

    for (auto It = pusherUpdateTing.m_cTingGroup.m_mValueTingMap.CreateIterator(); It; ++It)
    {
        int32 triggerValue = It.Key();
        FMyMJHuScoreResultTingCpp *pR = &It.Value();

        pR->m_iTriggerCardLeftOnDesktop = (4 - pHelperShowedOut2AllCards->getCountByValue(triggerValue));
    }


    pusherUpdateTing.initWithTingGroupAlreadyInited(m_iIdx);
    pCore->getpPusherIOFull()->EnqueuePusher(pusherUpdateTing);

    return bTing;
}


void FMyMJGameAttenderLocalCSCpp::applyPusherUpdateTing(FMyMJGamePusherUpdateTingCpp *pPusher)
{
    m_cHuScoreResultTingGroup = pPusher->m_cTingGroup;
}

void FMyMJGameAttenderLocalCSCpp::applyActionNoAct(FMyMJGameActionNoActCpp *pAction)
{
    if ((pAction->m_iReserved0 & MyMJGameActionNoActreserved0MaskPassPaoHu) > 0) {
        m_bBanPaoHuLocalCS = true;
    }
}

void FMyMJGameAttenderLocalCSCpp::applyActionHuBornLocalCS(FMyMJGameActionHuBornLocalCSCpp *pAction)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = pCore->getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = pCore->getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    int32 l = pAction->m_aShowOutIdValues.Num();

    for (int32 i = 0; i < l; i++) {
        //1, reveal value
        int32 id = pAction->m_aShowOutIdValues[i].m_iId;
        int32 value = pAction->m_aShowOutIdValues[i].m_iValue;
        pCardValuePack->revealCardValue(id, value);

        //2, make it stand
        FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
        pCardInfo->m_eFlipState = MyMJCardFlipStateCpp::Up;
    }

    FMyMJHuScoreResultFinalGroupCpp& scoreResultGroup = getHuScoreResultFinalGroupLocalCSBornRef();

    scoreResultGroup.m_iIdxAttenderWin = m_iIdx;
    int32 idx = scoreResultGroup.m_aScoreResults.Emplace();
    FMyMJHuScoreResultFinalCpp *pScore = &scoreResultGroup.m_aScoreResults[idx];
    pScore->append(pAction->m_aHuScoreResultItems);

    UMyMJUtilsLibrary::convertIdValuePairs2Ids(pAction->m_aShowOutIdValues, m_aIdHandCardShowedOutLocalCS);

}

void FMyMJGameAttenderLocalCSCpp::applyActionWeave(FMyMJGameActionWeaveCpp *pAction)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    FMyMJCardInfoPackCpp  *pCardInfoPack = pCore->getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = pCore->getpCardValuePack();

    MyMJWeaveTypeCpp eType = pAction->m_cWeave.getType();

    //1st, move and update cards
    const TArray<FMyIdValuePair>& aIdValuePairs = pAction->m_cWeave.getIdValuesRef();
    int32 l = aIdValuePairs.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyIdValuePair *pPair = &aIdValuePairs[i];
        pCardValuePack->revealCardValue(*pPair);

        pCore->moveCardFromOldPosi(pPair->m_iId);
        pCore->moveCardToNewPosi(pPair->m_iId, getIdx(), MyMJCardSlotTypeCpp::Weaved);

        pCardInfoPack->getByIdx(pPair->m_iId)->m_eFlipState = pAction->m_eTargetFlipState;
    }

    TArray<int32> aIdsJustTaken = getIdJustTakenCardsRef(); //do a copy
    int32 l0 = aIdsJustTaken.Num();
    for (int32 i = 0; i < l0; i++) {
        int32 idCard = aIdsJustTaken[i];
        pCore->moveCardFromOldPosi(idCard);
        pCore->moveCardToNewPosi(idCard, getIdx(), MyMJCardSlotTypeCpp::InHand);
    }
    MY_VERIFY(getIdJustTakenCardsRef().Num() <= 0);



    //2nd, form a weave and add, and update helper minor posi info
    int32 idx = m_aShowedOutWeaves.Emplace();
    m_aShowedOutWeaves[idx] = pAction->m_cWeave;

    l = aIdValuePairs.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyIdValuePair *pPair = &aIdValuePairs[i];
        FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(pPair->m_iId);
        pCardInfo->m_cPosi.m_iIdxInSlot0 = idx;
        pCardInfo->m_cPosi.m_iIdxInSlot1 = i;
    }

    //3rd, consider the type and update
    if (eType == MyMJWeaveTypeCpp::GangAn || eType == MyMJWeaveTypeCpp::GangMing) {

        int32 reserved0 = pAction->m_cWeave.getReserved0();
        if (pCore->getWorkMode() == MyMJGameCoreWorkModeCpp::Full) {
            bool bTIngNow = tryGenAndEnqueueUpdateTingPusher();
            if (reserved0 == FMyMJWeaveCppReserved0WhenGangValueGangYao) {
                MY_VERIFY(bTIngNow);
            }
        }

        if (reserved0 == FMyMJWeaveCppReserved0WhenGangValueGangYao) {
            m_bGangYaoedLocalCS = true;
        }

    }
    else if (eType == MyMJWeaveTypeCpp::KeZiMing || eType == MyMJWeaveTypeCpp::ShunZiMing) {

    }
    else {
        MY_VERIFY(false);
    }

}

void FMyMJGameAttenderLocalCSCpp::showOutCardsAfterHu()
{
    FMyMJGameCoreCpp* pCore = getpCore();
    FMyMJCardInfoPackCpp  *pCardInfoPack = pCore->getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = pCore->getpCardValuePack();

    TArray<int32> aIdsHandCards, aIdsWeaves, aIdsTemp;
    m_cHandCards.collectAll(aIdsHandCards);

    int32 l;
    l = m_aShowedOutWeaves.Num();
    for (int32 i = 0; i < l; i++) {
        m_aShowedOutWeaves[i].getIds(aIdsTemp);
        aIdsWeaves.Append(aIdsTemp);
    }

    TArray<int32> *pArrays[3];
    pArrays[0] = &aIdsHandCards;
    pArrays[1] = &aIdsWeaves;
    pArrays[2] = &m_aIdJustTakenCards;

    for (int k = 0; k < 3; k++) {
        TArray<int32> *pArray = pArrays[k];
        l = pArray->Num();
        for (int32 i = 0; i < l; i++) {
            pCardInfoPack->getByIdx((*pArray)[i])->m_eFlipState = MyMJCardFlipStateCpp::Up;
        }
    }

}

void FMyMJGameAttenderLocalCSCpp::dataResetByMask(int32 iMaskAttenderDataReset)
{
    if (iMaskAttenderDataReset & MaskAttenderDataResetIdHandCardShowedOutLocalCS) {
        m_aIdHandCardShowedOutLocalCS.Reset();
    }
}

void FMyMJGameAttenderLocalCSCpp::onNewTurn(bool bIsWeave)
{
    m_iTurn++;

    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = pCore->getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = pCore->getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    if (bIsWeave) {
        if (pD->m_cGameCfg.m_cSubLocalCSCfg.m_bResetAttenderPaoHuBanStateAfterWeave) {
            m_bBanPaoHuLocalCS = false;
        }
    }
    else {
        m_bBanPaoHuLocalCS = false;
    }

    if (pCore->getWorkMode() == MyMJGameCoreWorkModeCpp::Full) {
        int32 l = m_aIdHandCardShowedOutLocalCS.Num();

        if (l > 0) {

            FMyMJGamePusherUpdateCardsCpp pusher, *pPusher = &pusher;
            for (int32 i = 0; i < l; i++) {
                int32 idCard = m_aIdHandCardShowedOutLocalCS[i];
                FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(idCard);
                if (pCardInfo->m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::InHand && pCardInfo->m_eFlipState != MyMJCardFlipStateCpp::Up) {

                    int32 idx = pPusher->m_aIdValues.Emplace();
                    pPusher->m_aIdValues[idx].m_iId = idCard;
                    pPusher->m_aIdValues[idx].m_iValue = pCardValuePack->getByIdx(idCard);

                }
            }

            pPusher->initWithCardsTargetStateAlreadyInited(m_iIdx, MyMJCardFlipStateCpp::Up, MyMJCardFlipStateCpp::Stand, MaskAttenderDataResetIdHandCardShowedOutLocalCS | 0);
            pCore->getpPusherIOFull()->EnqueuePusher(*pPusher);
        }

    }
}


void FMyMJGameAttenderLocalCSCpp::genActionAfterGivenOutCards(FMyMJGamePusherFillInActionChoicesCpp *pFillInPusher, bool bIsGang)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack = pCore->getpCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = pCore->getpCardValuePack();

    MY_VERIFY(pCardInfoPack);
    MY_VERIFY(pCardValuePack);

    const FMyMJGameCfgCpp *pGameCfg = &pD->m_cGameCfg;

    const TArray<FMyIdValuePair>& lastCards = pD->m_aHelperLastCardsGivenOutOrWeave;
    MY_VERIFY(lastCards.Num() > 0);
    int32 idxAttenderTrigger = pCardInfoPack->getByIdxConst(lastCards[0].m_iId)->m_cPosi.m_iIdxAttender;
    FMyMJHuActionAttrCpp huActionAttr;
    assembleHuActionAttr(idxAttenderTrigger, &lastCards, bIsGang? MyMJHuTriggerCardSrcTypeCpp::GangCardTaken : MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

    bool bHaveValidAction = false;
    //check hu
    bool bCanHu = false;
    FMyMJHuScoreResultFinalGroupCpp resultGroup;
    if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(pGameCfg->m_cHuCfg, pGameCfg->m_cSubLocalCSCfg, m_aShowedOutWeaves, m_cHandCards, huActionAttr, m_cHuScoreResultTingGroup, resultGroup)) {
        FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
        pAction->m_cHuScoreResultFinalGroup = resultGroup;
        pAction->initWithFinalGroupAlreadyInited(m_iIdx, true);
        pFillInPusher->m_cActionChoices.give(pAction);

        bCanHu = true;
        bHaveValidAction = true;
    }

    FMyMJCardCpp cTriggerCard;
    TArray<FMyMJGameActionWeaveCpp> aActionWeaves;

    int32 idxAttenderTriggerNext = pCore->findIdxAttenderStillInGame(idxAttenderTrigger, 1, false);
    int32 l;
    int32 lastCardsL = lastCards.Num();
    for (int32 idxLastCards = 0; idxLastCards < lastCardsL; idxLastCards++) {
        const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(lastCards[idxLastCards].m_iId);
        *StaticCast<FMyMJCardInfoCpp *>(&cTriggerCard) = *pCardInfo;
        cTriggerCard.m_iValue = pCardValuePack->getByIdx(cTriggerCard.m_iId);

        //check Gang
        checkGang(&cTriggerCard, true, aActionWeaves);
        l = aActionWeaves.Num();
        for (int32 i = 0; i < l; i++) {
            FMyMJGameActionWeaveCpp *newAction = new FMyMJGameActionWeaveCpp();
            *newAction = aActionWeaves[i];
            pFillInPusher->m_cActionChoices.give(newAction);

            bHaveValidAction = true;
        }

        //check Peng
        checkPeng(cTriggerCard, aActionWeaves);
        l = aActionWeaves.Num();
        for (int32 i = 0; i < l; i++) {
            FMyMJGameActionWeaveCpp *newAction = new FMyMJGameActionWeaveCpp();
            *newAction = aActionWeaves[i];
            pFillInPusher->m_cActionChoices.give(newAction);

            bHaveValidAction = true;
        }

        //check Chi
        if (idxAttenderTriggerNext == getIdx()) {
            checkChi(cTriggerCard, aActionWeaves);
            l = aActionWeaves.Num();
            for (int32 i = 0; i < l; i++) {
                FMyMJGameActionWeaveCpp *newAction = new FMyMJGameActionWeaveCpp();
                *newAction = aActionWeaves[i];
                pFillInPusher->m_cActionChoices.give(newAction);

                bHaveValidAction = true;
            }
        }
    }

    if (bHaveValidAction) {
        FMyMJGameActionNoActCpp *pActionNoAct = new FMyMJGameActionNoActCpp();
        pActionNoAct->init(m_iIdx, bCanHu ? MyMJGameActionNoActreserved0MaskPassPaoHu : 0);
        pFillInPusher->m_cActionChoices.give(pActionNoAct);
    }
}

FMyMJGameActionGiveOutCardsCpp* FMyMJGameAttenderLocalCSCpp::genActionChoiceGiveOutCards(bool bRestrict2SelectCardsJustTaken, bool bIsGang)
{
    TSharedPtr<FMyMJGameCoreCpp> pCore = m_pCore.Pin();
    MY_VERIFY(pCore.IsValid());

    TArray<int32> aIdHandCards;
    if (!bRestrict2SelectCardsJustTaken) {
        m_cHandCards.collectAll(aIdHandCards);
    }

    FMyMJGameActionGiveOutCardsCpp *pAction = new FMyMJGameActionGiveOutCardsCpp();
    pAction->init(m_iIdx, aIdHandCards, m_aIdJustTakenCards, bRestrict2SelectCardsJustTaken, bIsGang);

    return pAction;
}



void FMyMJGameAttenderLocalCSCpp::assembleHuActionAttr(int32 iIdxAttenderLoseOnlyOne, const TArray<FMyIdValuePair> *pTriggerCards, MyMJHuTriggerCardSrcTypeCpp eTriggerCardSrcType, FMyMJHuActionAttrCpp &outHuActionAttr)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    FMyMJGameRunDataCpp *pGameRunData = &pD->m_cGameRunData;
    const FMyMJValueIdMapCpp *pHelperShowedOutCards = &pD->m_cHelperShowedOut2AllCards;

    outHuActionAttr.reset();

    outHuActionAttr.m_iIdxAttenderWin = getIdx();
    outHuActionAttr.m_iIdxAttenderLoseOnlyOne = iIdxAttenderLoseOnlyOne;

    outHuActionAttr.m_iQUanFeng = pGameRunData->m_iIdxAttenderQuanFeng;
    outHuActionAttr.m_iMenFeng = pGameRunData->m_iIdxAttenderMenFeng;

    outHuActionAttr.m_eTrigerCardActionType = eTriggerCardSrcType;
    if (pTriggerCards) {
        int32 l = pTriggerCards->Num();
        for (int32 i = 0; i < l; i++) {
            const FMyIdValuePair *pTriggerCard = &(*pTriggerCards)[i];
            if (pTriggerCard->m_iId < 0) {
                continue;
            }

            int32 idx = outHuActionAttr.m_aTriggerDatas.Emplace();
            FMyTriggerDataCpp *pData = &outHuActionAttr.m_aTriggerDatas[idx];
            *pData = *pTriggerCard;
            MY_VERIFY(pTriggerCard->m_iValue > 0);

            int32 c = pHelperShowedOutCards->getCountByValue(pTriggerCard->m_iValue);

            if (outHuActionAttr.m_iIdxAttenderLoseOnlyOne < 0) {
                //ZiMo
                c++;
            }

            pData->m_iValueShowedOutCountAfter = c;
        }

    }
    else {
        outHuActionAttr.m_aTriggerDatas.Reset();
    }

    outHuActionAttr.m_iCardNumCanBeTakenNormally = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenNormally();
    outHuActionAttr.m_iAttenderTurn = m_iTurn;

    outHuActionAttr.m_iHuaCount = m_aIdWinSymbolCards.Num();
    outHuActionAttr.m_iTingCount = m_cHuScoreResultTingGroup.getCount();

    //special rule, CSMJ does not allow paohu always
    if (outHuActionAttr.geIsPao() && m_bBanPaoHuLocalCS) {
        outHuActionAttr.m_iIdxAttenderWin = -1;
        MY_VERIFY(!outHuActionAttr.getMayHu());
    }
    /*
    int32 m_iUntakenCardsLeft;
    //int32 m_iGameTurn;
    int32 m_iAttenderTurn;

    int32 m_iHuaCount;
    int32 m_iTingCount;
    */
}

bool FMyMJGameAttenderLocalCSCpp::checkGang(const FMyMJCardCpp *pTriggerCard, bool bLimited2TriggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves0)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    outActionWeaves0.Reset();

    int32 iCardLeftNum = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();

    //haidi can't gang
    if (iCardLeftNum <= 0) {
        return false;
    }

    bool bAllowBuZhangLocalCS = true;
    if (m_bGangYaoedLocalCS) {
        //special local CS rule, which ban card switch after GangYao
        bAllowBuZhangLocalCS = false;
        if (!pD->m_cGameCfg.m_cSubLocalCSCfg.m_bAllowGangYaoAfterGangYao) {
            return false;
        }
    }

    TArray<FMyMJGameActionWeaveCpp> actionWeavesBuZhangLocalCS;
    TArray<FMyMJGameActionWeaveCpp> actionWeavesGangYao;

    TArray<FMyMJWeaveCpp> &inWeavesShowedOut = m_aShowedOutWeaves;
    FMyMJValueIdMapCpp &inHandCardMap = m_cHandCards;
    int32 idxAttenderSelf = m_iIdx;

    TArray<FMyIdValuePair> aPairsTemp;

    if (pTriggerCard) {
        MY_VERIFY(pTriggerCard->m_cPosi.m_iIdxAttender >= 0 && pTriggerCard->m_cPosi.m_iIdxAttender < 4);
    }

    //1st, check peng+trigger case
    int32 l = inWeavesShowedOut.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyMJWeaveCpp *pWeave = &inWeavesShowedOut[i];
        MY_VERIFY(pWeave->getType() != MyMJWeaveTypeCpp::KeZiAn);
        if (inWeavesShowedOut[i].getType() == MyMJWeaveTypeCpp::KeZiMing) {
            int32 cardValue = pWeave->getMidValue();
            int32 cardIdFound = -1;
            int32 iIdTriggerCard = -1;
            int32 iIdxAttenderTriggerCardSrc = -1;

            if (pTriggerCard && pTriggerCard->m_iValue == cardValue && pTriggerCard->m_cPosi.m_iIdxAttender == idxAttenderSelf) {
                cardIdFound = pTriggerCard->m_iId;
                iIdTriggerCard = pTriggerCard->m_iId;
                iIdxAttenderTriggerCardSrc = pTriggerCard->m_cPosi.m_iIdxAttender;
            }
            else {
                if (!bLimited2TriggerCard) {
                    inHandCardMap.collectByValueWithValue(cardValue, 4, aPairsTemp);
                    if (aPairsTemp.Num() >= 1) {
                        cardIdFound = aPairsTemp[0].m_iId;
                    }
                }
            }

            if (cardIdFound >= 0) {
                //yeah we found one
                int32 idx = actionWeavesBuZhangLocalCS.Emplace();
                FMyMJGameActionWeaveCpp *pAction = &actionWeavesBuZhangLocalCS[idx];
                FMyMJWeaveCpp *pWeaveInAction = &pAction->m_cWeave;
                pWeaveInAction->appendIdVakuePairs(pWeave->getIdValuesRef());
                pWeaveInAction->addCard(cardIdFound, cardValue);

                pWeaveInAction->initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp::GangMing, iIdTriggerCard, iIdxAttenderTriggerCardSrc, MyMJWeaveTypeCpp::KeZiMing, FMyMJWeaveCppReserved0WhenGangValueBuZhang);
                pAction->initWithWeaveAlreadyInited(idxAttenderSelf, MyMJCardFlipStateCpp::Up);
            }
        }
    }

    //2nd, check hand cards + trigger
    TArray<int32> aHandValues;

    inHandCardMap.keys(aHandValues);
    l = aHandValues.Num();
    for (int32 i = 0; i < l; i++) {
        int32 cardValue = aHandValues[i];
        inHandCardMap.collectByValueWithValue(cardValue, 4, aPairsTemp);

        int32 iIdTriggerCard = -1;
        int32 iIdxAttenderTriggerCardSrc = -1;
        MyMJWeaveTypeCpp eType = MyMJWeaveTypeCpp::Invalid;
        MyMJCardFlipStateCpp eTargetFlipState = MyMJCardFlipStateCpp::Up;
        if (aPairsTemp.Num() >= 4) {
            if (!bLimited2TriggerCard) {
                eType = MyMJWeaveTypeCpp::GangAn;
                eTargetFlipState = MyMJCardFlipStateCpp::Down;
            }
        }
        else if (aPairsTemp.Num() >= 3 && pTriggerCard && pTriggerCard->m_iValue == cardValue) {
            iIdTriggerCard = pTriggerCard->m_iId;
            iIdxAttenderTriggerCardSrc = pTriggerCard->m_cPosi.m_iIdxAttender;
            if (pTriggerCard->m_cPosi.m_iIdxAttender == idxAttenderSelf) {
                eType = MyMJWeaveTypeCpp::GangAn;
                eTargetFlipState = MyMJCardFlipStateCpp::Down;
            }
            else {
                eType = MyMJWeaveTypeCpp::GangMing;
            }
        }

        if (eType != MyMJWeaveTypeCpp::Invalid) {
            //yeah we found one
            int32 idx = actionWeavesBuZhangLocalCS.Emplace();
            FMyMJGameActionWeaveCpp *pAction = &actionWeavesBuZhangLocalCS[idx];
            FMyMJWeaveCpp *pWeaveInAction = &pAction->m_cWeave;
            pWeaveInAction->appendIdVakuePairs(aPairsTemp);
            if (aPairsTemp.Num() < 4) {
                MY_VERIFY(pTriggerCard);
                pWeaveInAction->addCard(*pTriggerCard);
            }

            pWeaveInAction->initWitIdValuesAlreadyInited(eType, iIdTriggerCard, iIdxAttenderTriggerCardSrc, MyMJWeaveTypeCpp::Invalid, FMyMJWeaveCppReserved0WhenGangValueBuZhang);
            pAction->initWithWeaveAlreadyInited(idxAttenderSelf, eTargetFlipState);
        }
    }

    //Post process, check any GangYao Possible
    FMyMJHuScoreResultTingGroupCpp tingGroup;

    //TArray<FMyMJGameActionWeaveCpp> aExtraActions;

    l = actionWeavesBuZhangLocalCS.Num();
    if (l > 0) {
        FMyMJValueIdMapCpp handCardValueTemp;
        handCardValueTemp.copyDeep(&inHandCardMap);
        TArray<FMyMJWeaveCpp> weavesShowedOutTemp = inWeavesShowedOut;

        aPairsTemp.Reset();

        for (int32 i = 0; i < l; i++) {
            const FMyMJGameActionWeaveCpp *pAction = &actionWeavesBuZhangLocalCS[i];
            const FMyMJWeaveCpp *pWeave = &pAction->m_cWeave;
            int32 cardValue = pWeave->getMidValue();

            //Forcast the state after gang
            handCardValueTemp.removeAllByValue(cardValue, aPairsTemp);
            weavesShowedOutTemp.Push(*pWeave);

            bool bTing = UMyMJUtilsLocalCSLibrary::checkTingsLocalCSInGame(pD->m_cGameCfg, *pCore->getpCardValuePack(), weavesShowedOutTemp, handCardValueTemp, tingGroup);
            if (bTing) {
                bool bIsRealTingWhenZiMo;
                tingGroup.isRealTing(&bIsRealTingWhenZiMo, NULL);
                if (bIsRealTingWhenZiMo) {
                    //this is one style allow gang yao hu after gang
                    int32 idx = actionWeavesGangYao.Emplace();
                    FMyMJGameActionWeaveCpp *pActionNew = &actionWeavesGangYao[idx];
                    *pActionNew = *pAction;
                    pActionNew->m_cWeave.getReserved0Ref() = FMyMJWeaveCppReserved0WhenGangValueGangYao;
                }
            }

            weavesShowedOutTemp.Pop();
            handCardValueTemp.insertIdValuePairsAssertNoDup(aPairsTemp);
        }
    }

    if (bAllowBuZhangLocalCS) {
        outActionWeaves0.Append(actionWeavesBuZhangLocalCS);
    }

    outActionWeaves0.Append(actionWeavesGangYao);


    //outActionWeaves.Append(aExtraActions);
    return (outActionWeaves0.Num() > 0);
}

bool FMyMJGameAttenderLocalCSCpp::checkPeng(const FMyMJCardCpp &triggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    outActionWeaves.Reset();
    int32 iCardLeftNum = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();

    //Haidi can't peng
    if (iCardLeftNum <= 0) {
        return false;
    }

    if (m_bGangYaoedLocalCS) {
        //special local CS rule, which ban card switch after GangYao
        return false;

    }

    TArray<FMyIdValuePair> aPairsTemp;
    FMyMJValueIdMapCpp &inHandCardMap = m_cHandCards;

    TArray<int32> aHandValues;
    inHandCardMap.keys(aHandValues);
    int32 l = aHandValues.Num();
    for (int32 i = 0; i < l; i++) {
        int32 cardValue = aHandValues[i];
        inHandCardMap.collectByValueWithValue(cardValue, 4, aPairsTemp);

        if (aPairsTemp.Num() >= 2 && cardValue == triggerCard.m_iValue) {
            //yeah we found one
            int32 idx = outActionWeaves.Emplace();
            FMyMJGameActionWeaveCpp *pAction = &outActionWeaves[idx];
            FMyMJWeaveCpp *pWeaveInAction = &pAction->m_cWeave;
            pWeaveInAction->appendIdVakuePairsWithLimit(aPairsTemp, 2);
    
            pWeaveInAction->addCard(triggerCard);

            pWeaveInAction->initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp::KeZiMing, triggerCard.m_iId, triggerCard.m_cPosi.m_iIdxAttender, MyMJWeaveTypeCpp::Invalid, 0);
            pAction->initWithWeaveAlreadyInited(getIdx(), MyMJCardFlipStateCpp::Up);
        }
    }

    return (outActionWeaves.Num() > 0);
}

bool FMyMJGameAttenderLocalCSCpp::checkChi(const FMyMJCardCpp &triggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves)
{
    FMyMJGameCoreCpp* pCore = getpCore();
    MY_VERIFY(pCore);

    FMyMJCoreDataDirectPublicCpp *pD = pCore->getDataDirectPublic();
    MY_VERIFY(pD);

    outActionWeaves.Reset();

    int32 iCardLeftNum = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();

    //Haidi can't peng
    if (iCardLeftNum <= 0) {
        return false;
    }

    if (m_bGangYaoedLocalCS) {
        //special local CS rule, which ban card switch after GangYao
        return false;

    }

    MyMJChiTypeCpp chiTypes[3];
    chiTypes[0] = MyMJChiTypeCpp::ChiLeft;
    chiTypes[1] = MyMJChiTypeCpp::ChiMid;
    chiTypes[2] = MyMJChiTypeCpp::ChiRight;

    FMyIdValuePair triggerCardIdValuePair;
    triggerCard.convert2IdValuePair(triggerCardIdValuePair);

    TArray<FMyIdValuePair> aIdValuePairsInMap;
    for (int32 i = 0; i < 3; i++) {

        if (m_cHandCards.checkShunZi(chiTypes[i], -1, &triggerCardIdValuePair, aIdValuePairsInMap)) {
            int32 idx = outActionWeaves.Emplace();
            FMyMJGameActionWeaveCpp *pAction = &outActionWeaves[idx];
            FMyMJWeaveCpp *pWeaveInAction = &pAction->m_cWeave;
            pWeaveInAction->appendIdVakuePairsWithLimit(aIdValuePairsInMap, 2);

            pWeaveInAction->addCard(triggerCardIdValuePair);

            pWeaveInAction->initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp::ShunZiMing, triggerCard.m_iId, triggerCard.m_cPosi.m_iIdxAttender, MyMJWeaveTypeCpp::Invalid, 0);
            pAction->initWithWeaveAlreadyInited(getIdx(), MyMJCardFlipStateCpp::Up);
        }
        //bool checkShunZi(MyMJChiTypeCpp type, int32 checkingValueInMap, int32 extraValue, TArray<int32> &outIds, TArray<int32> &outValues) const;
    }

    return (outActionWeaves.Num() > 0);
}

/*
bool FMyMJGameAttenderLocalCSCpp::checkHuInGame(FMyMJCardCpp &triggerCard, FMyMJHuActionAttrCpp &huActionAttr)
{
    int32 handCardCount = m_cHandCards.getCount();
    int32 modV = handCardCount % 3;

    if (modV == 2) {
        //all card in

        FMyMJHuActionAttrCpp huActionAttr;
        assembleHuActionAttr(-1, NULL, MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithOutTriggerCard(pGameCfg->m_cHuCfg, pGameCfg->m_cSubLocalCSCfg, m_aShowedOutWeaves, m_cHandCards, huActionAttr, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            FMyMJHuScoreResultFinalGroupCpp *pGroup = &pAction->m_cHuScoreResultFinalGroup;
            *pGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(m_iIdx, true);
            pFillInPusher->m_cActionChoices.give(pAction);
        }

    }
    else if (modV == 1) {
        //ting card state
    }
    else {
        MY_VERIFY(false);
    }

    return false;
}
*/