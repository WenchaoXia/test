// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameAttenderLocalCS.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

#include "MyMJGameCoreLocalCS.h"

void FMyMJGameAttenderLocalCSCpp::genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO)
{
    FMyMJGameCoreCpp *pCore = &getCoreRef();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    MyMJGameStateCpp eGameState = pD->m_eGameState;
    const FMyMJGameCfgCpp  *pGameCfg = &pD->m_cGameCfg;

    FMyMJGamePusherFillInActionChoicesCpp *pFillInPusher = new FMyMJGamePusherFillInActionChoicesCpp();
    pFillInPusher->init(pD->m_iActionGroupId, getIdx());

    bool bForceActionGenTimeLeft2AutoChooseMsZero = (pCore->m_iTrivalConfigMask & MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero) > 0;

    if (eGameState == MyMJGameStateCpp::CardsWaitingForThrowDicesToDistributeCards) {
        FMyMJGameActionThrowDicesCpp *pAction = new FMyMJGameActionThrowDicesCpp();
        pFillInPusher->m_cActionChoices.give(pAction);
        pAction->init(FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GameStart, getIdx(), pCore->getResManagerRef().getRandomStreamRef(), bForceActionGenTimeLeft2AutoChooseMsZero);
    }
    else if (eGameState == MyMJGameStateCpp::CardsWaitingForDistribution)
    {
        int32 idxHeadNow;
        TArray<int32> aIds;
        bool bIsLast = false;
        idxHeadNow = pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow;

        int32 handCardCount = m_cDataLogic.m_cHandCards.getCount();
        if (handCardCount < 12) {
            pCore->collectCardsFromUntakenSlot(idxHeadNow, 4, false, aIds);
        }
        else {
            int32 idxAttenderZhuang = pD->m_cGameRunData.m_iIdxAttenderMenFeng;
            int32 idxAttenderZhuangPre = pCore->findIdxAttenderStillInGame(idxAttenderZhuang, 1, true);
            MY_VERIFY(idxAttenderZhuang != idxAttenderZhuangPre);
            if (getIdx() == idxAttenderZhuang) {
                pCore->collectCardsFromUntakenSlot(idxHeadNow, 1, false, aIds);
                int32 idxHeadNowPlus2 = pCore->getIdxOfUntakenSlotHavingCard(idxHeadNow, 2, false);
                TArray<int32> aIds2;
                pCore->collectCardsFromUntakenSlot(idxHeadNowPlus2, 1, false, aIds2);
                aIds.Append(aIds2);

                MY_VERIFY(aIds.Num() == 2);
            }
            else if (getIdx() == idxAttenderZhuangPre) {
                pCore->collectCardsFromUntakenSlot(idxHeadNow, 1, false, aIds);
                bIsLast = true;
            }
            else {
                pCore->collectCardsFromUntakenSlot(idxHeadNow, 1, false, aIds);
            }
        }

        TArray<FMyIdValuePair> aIdValuePairs;
        pCardValuePack->helperIds2IdValuePairs(aIds, aIdValuePairs);

        //OK, all data collecteed, let's gen

        FMyMJGameActionDistCardAtStartCpp *pAction = new FMyMJGameActionDistCardAtStartCpp();
        //pFillInPusher->m_cActionChoices.give(pAction);
        pAction->init(getIdx(), aIdValuePairs, bIsLast);

        pPusherIO->GivePusher(pAction, (void **)&pAction);
    }
    else if (eGameState == MyMJGameStateCpp::JustStarted) {

        FMyMJHuActionAttrCpp huActionAttr;
        assembleHuActionAttr(-1, NULL, MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithOutTriggerCard(*pCardValuePack, pGameCfg->m_cHuCfg, pGameCfg->getSubLocalCSCfgRefConst(), pDPubD->m_aShowedOutWeaves, m_cDataLogic.m_cHandCards, huActionAttr, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            pAction->m_cHuScoreResultFinalGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(getIdx(), true);
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

        //bool bGangYaoedLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
        bool bGangYaoedLocalCS = pDPubD->m_bGangYaoedLocalCS;

        MY_VERIFY(!bGangYaoedLocalCS);
        pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(false, false));
    }
    else if (eGameState == MyMJGameStateCpp::JustHu) {
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
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("gen taken cards: %s"), *UMyCommonUtilsLibrary::formatStrIdValuePairs(pAction->m_aIdValuePairs));

            //pFillInPusher->m_cActionChoices.give(pAction);
            pPusherIO->GivePusher(pAction, (void **)&pAction);
        }
        else {
            //Todo, let's end game
            FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
            pActionUpdate->m_eStateNext = MyMJGameStateCpp::GameEnd;
            pActionUpdate->m_iAttenderMaskNext = 0;
            pActionUpdate->m_eReason = MyMJGameStateUpdateReasonCpp::NoCardLeft;

            pPusherIO->GivePusher(pActionUpdate, (void **)&pActionUpdate);
        }

    }
    else if (eGameState == MyMJGameStateCpp::TakenCard) {
        FMyMJHuActionAttrCpp huActionAttr;
        TArray<FMyIdValuePair> aTriggerCards;
        pCardValuePack->helperIds2IdValuePairs(pDPubD->m_aIdJustTakenCards, aTriggerCards);
        assembleHuActionAttr(-1, &aTriggerCards, MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(*pCardValuePack, pGameCfg->m_cHuCfg, pGameCfg->getSubLocalCSCfgRefConst(), pDPubD->m_aShowedOutWeaves, m_cDataLogic.m_cHandCards, huActionAttr, pDPriD->m_cHuScoreResultTingGroup, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            pAction->m_cHuScoreResultFinalGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(getIdx(), true);
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

            //bool bGangYaoedLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
            bool bGangYaoedLocalCS = pDPubD->m_bGangYaoedLocalCS;
            pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(bGangYaoedLocalCS, false));
        }
    }
    else if (eGameState == MyMJGameStateCpp::GivenOutCard) {

        genActionAfterGivenOutCards(pFillInPusher, false);

    }
    else if (eGameState == MyMJGameStateCpp::WeavedNotGang) {
        bool bLastCardOfGame = (pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead <= 0);
        MY_VERIFY(!bLastCardOfGame);

        //bool bGangYaoedLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
        bool bGangYaoedLocalCS = pDPubD->m_bGangYaoedLocalCS;
        MY_VERIFY(!bGangYaoedLocalCS);

        pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(bGangYaoedLocalCS, false));
    }
    else if (eGameState == MyMJGameStateCpp::WeavedGang) {

        //check whether can qiang gang hu
        MY_VERIFY(pD->m_cHelper.m_aHelperLastWeaves.Num() == 1);

        int32 idCardRepresent = pD->m_cHelper.m_aHelperLastWeaves[0].getRepresentCardId();
        int32 idxAttenderTrigger = pCardInfoPack->getByIdxConst(idCardRepresent)->m_cPosi.m_iIdxAttender;

        TArray<FMyIdValuePair> aTriggerCard;
        int32 idx = aTriggerCard.Emplace();
        aTriggerCard[idx].m_iId = idCardRepresent;
        aTriggerCard[idx].m_iValue = pCardValuePack->getByIdx(aTriggerCard[idx].m_iId);
        FMyMJHuActionAttrCpp huActionAttr;
        assembleHuActionAttr(idxAttenderTrigger, &aTriggerCard, MyMJHuTriggerCardSrcTypeCpp::GangWeaveShowedOut, huActionAttr);

        FMyMJHuScoreResultFinalGroupCpp resultGroup;
        if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(*pCardValuePack, pGameCfg->m_cHuCfg, pGameCfg->getSubLocalCSCfgRefConst(), pDPubD->m_aShowedOutWeaves, m_cDataLogic.m_cHandCards, huActionAttr, pDPriD->m_cHuScoreResultTingGroup, resultGroup)) {
            FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
            pAction->m_cHuScoreResultFinalGroup = resultGroup;
            pAction->initWithFinalGroupAlreadyInited(getIdx(), true);
            pFillInPusher->m_cActionChoices.give(pAction);

            FMyMJGameActionNoActCpp *pActionNoAct = new FMyMJGameActionNoActCpp();
            pActionNoAct->init(getIdx(), true, ActionGenTimeLeft2AutoChooseMsForImportant, bForceActionGenTimeLeft2AutoChooseMsZero);
            pFillInPusher->m_cActionChoices.give(pActionNoAct);
        }


    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangQiangGangChecked) {
        FMyMJGameActionThrowDicesCpp *pAction = new FMyMJGameActionThrowDicesCpp();
        pFillInPusher->m_cActionChoices.give(pAction);
        pAction->init(FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GangYaoLocalCS, getIdx(), pCore->getResManagerRef().getRandomStreamRef(), (pCore->m_iTrivalConfigMask & MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero) > 0);
    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangTakenCards) {
        //Check hu
        {
            FMyMJHuActionAttrCpp huActionAttr;
            TArray<FMyIdValuePair> aTriggerCards;
            pCardValuePack->helperIds2IdValuePairs(pDPubD->m_aIdJustTakenCards, aTriggerCards);
            assembleHuActionAttr(-1, &aTriggerCards, MyMJHuTriggerCardSrcTypeCpp::GangCardTaken, huActionAttr);

            FMyMJHuScoreResultFinalGroupCpp resultGroup;
            if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(*pCardValuePack, pGameCfg->m_cHuCfg, pGameCfg->getSubLocalCSCfgRefConst(), pDPubD->m_aShowedOutWeaves, m_cDataLogic.m_cHandCards, huActionAttr, pDPriD->m_cHuScoreResultTingGroup, resultGroup)) {
                FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
                pAction->m_cHuScoreResultFinalGroup = resultGroup;
                pAction->initWithFinalGroupAlreadyInited(getIdx(), true);
                pFillInPusher->m_cActionChoices.give(pAction);
            }
        }

        //or give out
        pFillInPusher->m_cActionChoices.give(genActionChoiceGiveOutCards(true, true));

    }
    else if (eGameState == MyMJGameStateCpp::WeavedGangGivenOutCards) {

        genActionAfterGivenOutCards(pFillInPusher, true);
    }

    else if (eGameState == MyMJGameStateCpp::CardsDistributed) {

        TArray<FMyMJHuScoreResultItemCpp> huScoreResultItems;
        TArray<int32> aIdsShowOutCards;

        const FMyMJGameCfgCpp *pCfg = &pD->m_cGameCfg;
        bool bHu = UMyMJUtilsLocalCSLibrary::checkHuLocalCSBorn(pCfg->getSubLocalCSCfgRefConst(),
                                                                m_cDataLogic.m_cHandCards, huScoreResultItems, aIdsShowOutCards);
        if (bHu) {
            TArray<FMyIdValuePair> aShowOutIdValuePairs;
            pCardValuePack->helperIds2IdValuePairs(aIdsShowOutCards, aShowOutIdValuePairs, true);
            FMyMJGameActionHuBornLocalCSCpp *pAction = new FMyMJGameActionHuBornLocalCSCpp();
            pAction->init(getIdx(), huScoreResultItems, aShowOutIdValuePairs);
            pFillInPusher->m_cActionChoices.give(pAction);

            FMyMJGameActionNoActCpp *pActionNoAct = new FMyMJGameActionNoActCpp();
            pActionNoAct->init(getIdx(), 0, 0, bForceActionGenTimeLeft2AutoChooseMsZero);
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

        int32 c = pGameCfg->getSubLocalCSCfgRefConst().m_iGangYaoCount;
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
        pPusherIO->GivePusher(pFillInPusher, (void **)&pFillInPusher);
    }
    else {
        delete(pFillInPusher);
    }
}

bool FMyMJGameAttenderLocalCSCpp::tryGenAndEnqueueUpdateTingPusher()
{
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    int32 handCardCount = m_cDataLogic.m_cHandCards.getCount();
    if ((handCardCount % 3) != 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("handCardCount: %d"), handCardCount);
        MY_VERIFY(false);
    }

    FMyMJGamePusherUpdateTingCpp pusherUpdateTing;
    bool bTing = UMyMJUtilsLocalCSLibrary::checkTingsLocalCSInGame(pCore->getCardValuePackOfSysKeeperRefConst(), pD->m_cGameCfg, pDPubD->m_aShowedOutWeaves, m_cDataLogic.m_cHandCards, pusherUpdateTing.m_cTingGroup);

    if (pDPriD->m_cHuScoreResultTingGroup.getCount() == pusherUpdateTing.m_cTingGroup.getCount() && pDPriD->m_cHuScoreResultTingGroup.getCount() == 0) {
        //not ting yet, we don't need to gen pusher
        return bTing;
    }

    const FMyMJValueIdMapCpp *pHelperShowedOut2AllCards = &pCore->getDataLogicRefConst().m_cHelperShowedOut2AllCards;

    TArray<FMyMJHuScoreResultTingCpp>& aTings = pusherUpdateTing.m_cTingGroup.getTingsRef();
    int32  l = aTings.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJHuScoreResultTingCpp *pR = &aTings[i];
        int32 triggerValue = pR->m_iValueTriggerCard;
        pR->m_iTriggerCardLeftOnDesktop = (4 - pHelperShowedOut2AllCards->getCountByValue(triggerValue));
    }

    pusherUpdateTing.initWithTingGroupAlreadyInited(getIdx());

    FMyMJGamePusherBaseCpp *pP = pusherUpdateTing.cloneDeep();

    pCore->getPusherIOFullRef().GivePusher(pP, (void **)&pP);

    return bTing;
}


void FMyMJGameAttenderLocalCSCpp::showOutCardsAfterHu()
{
    /*
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    TArray<int32> aIdsHandCards, aIdsWeaves, aIdsTemp;
    m_cDataLogic.m_cHandCards.collectAll(aIdsHandCards);

    int32 l;
    l = pDPubD->m_aShowedOutWeaves.Num();
    for (int32 i = 0; i < l; i++) {
        const TArray<int32> &aIdsTemp = pDPubD->m_aShowedOutWeaves[i].getIdsRefConst();
        aIdsWeaves.Append(aIdsTemp);
    }

    TArray<int32> *pArrays[3];
    pArrays[0] = &aIdsHandCards;
    pArrays[1] = &aIdsWeaves;
    pArrays[2] = &pDPubD->m_aIdJustTakenCards;

    for (int k = 0; k < 3; k++) {
        TArray<int32> *pArray = pArrays[k];
        l = pArray->Num();
        for (int32 i = 0; i < l; i++) {
            pCardInfoPack->getByIdx((*pArray)[i])->m_eFlipState = MyBoxFlipStateCpp::Up;
        }
    }
    */
}

void FMyMJGameAttenderLocalCSCpp::dataResetByMask(int32 iMaskAttenderDataReset)
{
    if (iMaskAttenderDataReset & MyMJGamePusherUpdateAttenderCardsAndState_Mask0_ResetIdHandCardShowedOutLocalCS) {
        m_aIdHandCardShowedOutLocalCS.Reset();
    }
}

void FMyMJGameAttenderLocalCSCpp::onNewTurn(bool bIsWeave)
{
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    m_cDataLogic.m_iTurn++;

    /*
    if (bIsWeave) {
        if (pD->m_cGameCfg.m_cSubLocalCSCfg.m_bResetAttenderPaoHuBanStateAfterWeave) {
            m_bBanPaoHuLocalCS = false;
        }
    }
    else {
        m_bBanPaoHuLocalCS = false;
    }
    */


    int32 l = m_aIdHandCardShowedOutLocalCS.Num();

    if (l > 0) {

        FMyMJGamePusherUpdateAttenderCardsAndStateCpp *pPusher = new FMyMJGamePusherUpdateAttenderCardsAndStateCpp();
        for (int32 i = 0; i < l; i++) {
            int32 idCard = m_aIdHandCardShowedOutLocalCS[i];
            const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(idCard);
            if (pCardInfo->m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::InHand && pCardInfo->m_eFlipState != MyBoxFlipStateCpp::Up) {

                int32 idx = pPusher->m_aIdValues.Emplace();
                pPusher->m_aIdValues[idx].m_iId = idCard;
                //pPusher->m_aIdValues[idx].m_iValue = pCardValuePack->getByIdx(idCard);

            }
        }

        pPusher->initWithCardsTargetStateAlreadyInited(getIdx(), MyBoxFlipStateCpp::Up, MyBoxFlipStateCpp::Stand, MyMJGamePusherUpdateAttenderCardsAndState_Mask0_ResetIdHandCardShowedOutLocalCS | 0);

        pCore->getPusherIOFullRef().GivePusher(pPusher, (void **)&pPusher);
    }


}


void FMyMJGameAttenderLocalCSCpp::genActionAfterGivenOutCards(FMyMJGamePusherFillInActionChoicesCpp *pFillInPusher, bool bIsGang)
{
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();
    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    const FMyMJGameCfgCpp *pGameCfg = &pD->m_cGameCfg;

    TArray<FMyIdValuePair> aIdValuePairsLastCards;
    pCardValuePack->helperIds2IdValuePairs(pD->m_cHelper.m_aIdHelperLastCardsGivenOut, aIdValuePairsLastCards);
    MY_VERIFY(aIdValuePairsLastCards.Num() > 0); //possible > 1

    int32 idxAttenderTrigger = pCardInfoPack->getByIdxConst(aIdValuePairsLastCards[0].m_iId)->m_cPosi.m_iIdxAttender;
    FMyMJHuActionAttrCpp huActionAttr;
    assembleHuActionAttr(idxAttenderTrigger, &aIdValuePairsLastCards, bIsGang? MyMJHuTriggerCardSrcTypeCpp::GangCardTaken : MyMJHuTriggerCardSrcTypeCpp::CommonInGame, huActionAttr);

    bool bHaveValidAction = false;
    //check hu
    bool bCanHu = false;
    FMyMJHuScoreResultFinalGroupCpp resultGroup;
    if (UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(*pCardValuePack, pGameCfg->m_cHuCfg, pGameCfg->getSubLocalCSCfgRefConst(), pDPubD->m_aShowedOutWeaves, m_cDataLogic.m_cHandCards, huActionAttr, pDPriD->m_cHuScoreResultTingGroup, resultGroup)) {
        FMyMJGameActionHuCpp *pAction = new FMyMJGameActionHuCpp();
        pAction->m_cHuScoreResultFinalGroup = resultGroup;
        pAction->initWithFinalGroupAlreadyInited(getIdx(), true);
        pFillInPusher->m_cActionChoices.give(pAction);

        bCanHu = true;
        bHaveValidAction = true;
    }

    FMyMJCardCpp cTriggerCard;
    TArray<FMyMJGameActionWeaveCpp> aActionWeaves;

    int32 idxAttenderTriggerNext = pCore->findIdxAttenderStillInGame(idxAttenderTrigger, 1, false);
    int32 l;
    int32 lastCardsL = aIdValuePairsLastCards.Num();
    for (int32 idxLastCards = 0; idxLastCards < lastCardsL; idxLastCards++) {
        const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(aIdValuePairsLastCards[idxLastCards].m_iId);
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
        pActionNoAct->init(getIdx(), bCanHu, 0, (pCore->m_iTrivalConfigMask & MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero) > 0);
        pFillInPusher->m_cActionChoices.give(pActionNoAct);
    }
}

FMyMJGameActionGiveOutCardsCpp* FMyMJGameAttenderLocalCSCpp::genActionChoiceGiveOutCards(bool bRestrict2SelectCardsJustTaken, bool bIsGang)
{
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    //const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();
    //const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    //const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    //const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    TArray<int32> aIdHandCards;
    if (!bRestrict2SelectCardsJustTaken) {
        m_cDataLogic.m_cHandCards.collectAll(aIdHandCards);
    }

    FMyMJGameActionGiveOutCardsCpp *pAction = new FMyMJGameActionGiveOutCardsCpp();
    pAction->init(getIdx(), aIdHandCards, pDPubD->m_aIdJustTakenCards, bRestrict2SelectCardsJustTaken, bIsGang);

    return pAction;
}



void FMyMJGameAttenderLocalCSCpp::assembleHuActionAttr(int32 iIdxAttenderLoseOnlyOne, const TArray<FMyIdValuePair> *pTriggerCards, MyMJHuTriggerCardSrcTypeCpp eTriggerCardSrcType, FMyMJHuActionAttrCpp &outHuActionAttr)
{
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();
    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    const FMyMJGameRunDataCpp *pGameRunData = &pD->m_cGameRunData;
    const FMyMJValueIdMapCpp *pHelperShowedOutCards = &pCore->getDataLogicRefConst().m_cHelperShowedOut2AllCards;

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
    outHuActionAttr.m_iAttenderTurn = m_cDataLogic.m_iTurn;

    outHuActionAttr.m_iHuaCount = pDPubD->m_aIdWinSymbolCards.Num();
    outHuActionAttr.m_iTingCount = pDPriD->m_cHuScoreResultTingGroup.getCount();

    //bool bBanPaoHuLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPriD->m_iMask0, FMyMJRoleDataAttenderPrivateCpp_Mask0_BanPaoHuLocalCS);
    bool bBanPaoHuLocalCS = pDPriD->m_bBanPaoHuLocalCS;

    //special rule, CSMJ does not allow paohu always
    if (outHuActionAttr.geIsPao() && bBanPaoHuLocalCS) {
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
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();
    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();

    outActionWeaves0.Reset();

    int32 iCardLeftNum = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();

    //haidi can't gang
    if (iCardLeftNum <= 0) {
        return false;
    }

    bool bAllowBuZhangLocalCS = true;
    //bool bGangYaoedLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
    bool bGangYaoedLocalCS = pDPubD->m_bGangYaoedLocalCS;
    if (bGangYaoedLocalCS) {
        //special local CS rule, which ban card switch after GangYao
        bAllowBuZhangLocalCS = false;
        if (!pD->m_cGameCfg.getSubLocalCSCfgRefConst().m_bAllowGangYaoAfterGangYao) {
            return false;
        }
    }

    TArray<FMyMJGameActionWeaveCpp> actionWeavesBuZhangLocalCS;
    TArray<FMyMJGameActionWeaveCpp> actionWeavesGangYao;

    const TArray<FMyMJWeaveCpp> &inWeavesShowedOut = pDPubD->m_aShowedOutWeaves;
    FMyMJValueIdMapCpp &inHandCardMap = m_cDataLogic.m_cHandCards;
    int32 idxAttenderSelf = getIdx();

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
            int32 cardValue = pWeave->getMidValue(*pCardValuePack);
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
                //pWeaveInAction->appendIdVakuePairs(pWeave->getIdValuesRef());
                //pWeaveInAction->addCard(cardIdFound, cardValue);
                pWeaveInAction->appendIds(pWeave->getIdsRefConst());
                pWeaveInAction->addCard(cardIdFound);

                pWeaveInAction->initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp::GangMing, iIdTriggerCard, iIdxAttenderTriggerCardSrc, MyMJWeaveTypeCpp::KeZiMing, true);
                pAction->initWithWeaveAlreadyInited(*pCardValuePack, idxAttenderSelf, MyBoxFlipStateCpp::Up);
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
        MyBoxFlipStateCpp eTargetFlipState = MyBoxFlipStateCpp::Up;
        if (aPairsTemp.Num() >= 4) {
            if (!bLimited2TriggerCard) {
                eType = MyMJWeaveTypeCpp::GangAn;
                eTargetFlipState = MyBoxFlipStateCpp::Down;
            }
        }
        else if (aPairsTemp.Num() >= 3 && pTriggerCard && pTriggerCard->m_iValue == cardValue) {
            iIdTriggerCard = pTriggerCard->m_iId;
            iIdxAttenderTriggerCardSrc = pTriggerCard->m_cPosi.m_iIdxAttender;
            if (pTriggerCard->m_cPosi.m_iIdxAttender == idxAttenderSelf) {
                eType = MyMJWeaveTypeCpp::GangAn;
                eTargetFlipState = MyBoxFlipStateCpp::Down;
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
                //pWeaveInAction->addCard(*pTriggerCard);
                pWeaveInAction->addCard(pTriggerCard->m_iId);
            }

            pWeaveInAction->initWitIdValuesAlreadyInited(eType, iIdTriggerCard, iIdxAttenderTriggerCardSrc, MyMJWeaveTypeCpp::Invalid, true);
            pAction->initWithWeaveAlreadyInited(*pCardValuePack, idxAttenderSelf, eTargetFlipState);
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
            int32 cardValue = pWeave->getMidValue(*pCardValuePack);

            //Forcast the state after gang
            handCardValueTemp.removeAllByValue(cardValue, aPairsTemp);
            weavesShowedOutTemp.Push(*pWeave);

            bool bTing = UMyMJUtilsLocalCSLibrary::checkTingsLocalCSInGame(*pCardValuePack, pD->m_cGameCfg, weavesShowedOutTemp, handCardValueTemp, tingGroup);
            if (bTing) {
                bool bIsRealTingWhenZiMo;
                tingGroup.isRealTing(&bIsRealTingWhenZiMo, NULL);
                if (bIsRealTingWhenZiMo) {
                    //this is one style allow gang yao hu after gang
                    int32 idx = actionWeavesGangYao.Emplace();
                    FMyMJGameActionWeaveCpp *pActionNew = &actionWeavesGangYao[idx];
                    *pActionNew = *pAction;
                    pActionNew->m_cWeave.setGangBuZhangLocalCS(false);
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
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();



    outActionWeaves.Reset();
    int32 iCardLeftNum = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();

    //Haidi can't peng
    if (iCardLeftNum <= 0) {
        return false;
    }

    //bool bGangYaoedLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
    bool bGangYaoedLocalCS = pDPubD->m_bGangYaoedLocalCS;
    if (bGangYaoedLocalCS) {
        //special local CS rule, which ban card switch after GangYao
        return false;

    }

    TArray<FMyIdValuePair> aPairsTemp;
    FMyMJValueIdMapCpp &inHandCardMap = m_cDataLogic.m_cHandCards;

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
    
            //pWeaveInAction->addCard(triggerCard);
            pWeaveInAction->addCard(triggerCard.m_iId);

            pWeaveInAction->initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp::KeZiMing, triggerCard.m_iId, triggerCard.m_cPosi.m_iIdxAttender, MyMJWeaveTypeCpp::Invalid, 0);
            pAction->initWithWeaveAlreadyInited(pCore->getCardValuePackOfSysKeeperRefConst(), getIdx(), MyBoxFlipStateCpp::Up);
        }
    }

    return (outActionWeaves.Num() > 0);
}

bool FMyMJGameAttenderLocalCSCpp::checkChi(const FMyMJCardCpp &triggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves)
{
    FMyMJGameCoreCpp* pCore = &getCoreRef();

    const FMyMJCoreDataPublicCpp *pD = &pCore->getCoreDataPublicRefConst();

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &getRoleDataAttenderPublicRefConst();
    const FMyMJRoleDataAttenderPrivateCpp *pDPriD = &getRoleDataAttenderPrivateRefConst();


    outActionWeaves.Reset();

    int32 iCardLeftNum = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();

    //Haidi can't peng
    if (iCardLeftNum <= 0) {
        return false;
    }


    //bool bGangYaoedLocalCS = UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
    bool bGangYaoedLocalCS = pDPubD->m_bGangYaoedLocalCS;
    if (bGangYaoedLocalCS) {
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

        if (m_cDataLogic.m_cHandCards.checkShunZi(chiTypes[i], -1, &triggerCardIdValuePair, aIdValuePairsInMap)) {
            int32 idx = outActionWeaves.Emplace();
            FMyMJGameActionWeaveCpp *pAction = &outActionWeaves[idx];
            FMyMJWeaveCpp *pWeaveInAction = &pAction->m_cWeave;
            pWeaveInAction->appendIdVakuePairsWithLimit(aIdValuePairsInMap, 2);

            //pWeaveInAction->addCard(triggerCardIdValuePair);
            pWeaveInAction->addCard(triggerCardIdValuePair.m_iId);

            pWeaveInAction->initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp::ShunZiMing, triggerCard.m_iId, triggerCard.m_cPosi.m_iIdxAttender, MyMJWeaveTypeCpp::Invalid, 0);
            pAction->initWithWeaveAlreadyInited(pCore->getCardValuePackOfSysKeeperRefConst(), getIdx(), MyBoxFlipStateCpp::Up);
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