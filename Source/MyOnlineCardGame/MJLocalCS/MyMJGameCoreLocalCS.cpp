// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreLocalCS.h"
#include "Kismet/KismetMathLibrary.h"

FMyMJGamePusherResultCpp* FMyMJGameCoreLocalCSCpp::genPusherResultAsSysKeeper(const FMyMJGamePusherBaseCpp &pusher)
{
    const FMyMJGamePusherBaseCpp *pPusherIn = &pusher;

    MyMJGamePusherTypeCpp ePusherType = pPusherIn->getType();

    const FMyMJCoreDataPublicCpp& coreData = getCoreDataRefConst();


    FMyMJGamePusherResultCpp* pRet = new FMyMJGamePusherResultCpp();

    //by default we gen a delta and set the common data
    int32 idxDelta = pRet->m_aResultDelta.Emplace();
    FMyMJDataDeltaCpp& delta = pRet->m_aResultDelta[idxDelta];

    delta.m_iGameId = coreData.m_iGameId;
    StaticCast<FMyMJGamePusherBaseCpp&>(delta) = pusher;

    if (ePusherType == MyMJGamePusherTypeCpp::PusherFillInActionChoices)
    {
        const FMyMJGamePusherFillInActionChoicesCpp& pusherFillIn = StaticCast< const FMyMJGamePusherFillInActionChoicesCpp&>(pusher);

        int32 idxAttender = pusherFillIn.getIdxAttender();

        int32 idx = delta.m_aRoleDataAttender.Emplace();
        FMyMJRoleDataAttenderDeltaCpp &deltaAttender = delta.m_aRoleDataAttender[idx];
        deltaAttender.m_iIdxAttender = idxAttender;

        idx = deltaAttender.m_aDataPrivate.Emplace();
        FMyMJRoleDataAttenderPrivateDeltaCpp& deltaAttenderPrivate = deltaAttender.m_aDataPrivate[idx];

        idx = deltaAttenderPrivate.m_aActionContainor.Emplace();
        FMyMJGameActionContainorForBPCpp& actionContainorForBP = deltaAttenderPrivate.m_aActionContainor[idx];

        int32 l = pusherFillIn.m_cActionChoices.getCount();
        MY_VERIFY(l > 0);
        for (int32 i = 0; i < l; i++) {
            const TSharedPtr<FMyMJGamePusherBaseCpp> pP = pusherFillIn.m_cActionChoices.getSharedPtrAtConst(i);

            MY_VERIFY((uint8)pP->getType() >(uint8)MyMJGamePusherTypeCpp::ActionBase);

            TSharedPtr<FMyMJGameActionBaseCpp> p2 = StaticCastSharedPtr<FMyMJGameActionBaseCpp>(pP);
            MY_VERIFY(p2.IsValid());

            idx = actionContainorForBP.m_aActionChoices.Emplace();
            bool bGened = p2->genActionUnified(&actionContainorForBP.m_aActionChoices[idx]);
            if (!bGened) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("psuher %s failed to gen unified action!"),
                    *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)p2->getType()));
                MY_VERIFY(false);
            }

        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify)
    {
        const FMyMJGamePusherMadeChoiceNotifyCpp& pusherMadeChoice = StaticCast< const FMyMJGamePusherMadeChoiceNotifyCpp&>(pusher);

        int32 idxAttender = pusherMadeChoice.getIdxAttenderConst();

        int32 idx = delta.m_aRoleDataAttender.Emplace();
        FMyMJRoleDataAttenderDeltaCpp &deltaAttender = delta.m_aRoleDataAttender[idx];
        deltaAttender.m_iIdxAttender = idxAttender;

        idx = deltaAttender.m_aDataPrivate.Emplace();
        FMyMJRoleDataAttenderPrivateDeltaCpp& deltaAttenderPrivate = deltaAttender.m_aDataPrivate[idx];

        idx = deltaAttenderPrivate.m_aActionContainor.Emplace();
        FMyMJGameActionContainorForBPCpp& actionContainorForBP = deltaAttenderPrivate.m_aActionContainor[idx];

        actionContainorForBP = getAttenderByIdx(idxAttender)->getRoleDataAttenderPrivateRefConst().m_cActionContainor;

        actionContainorForBP.m_iChoiceSelected = pusherMadeChoice.m_iSelection;
        actionContainorForBP.m_aSubDataChoiceSelected = pusherMadeChoice.m_aSubSelections;

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherCountUpdate)
    {
        const FMyMJGamePusherCountUpdateCpp& pusherCountUpdate = StaticCast< const FMyMJGamePusherCountUpdateCpp&>(pusher);

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        if (pusherCountUpdate.m_bActionGroupIncrease) {
            UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_IncreaseActionGroupId, true);
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame)
    {
        const FMyMJGamePusherResetGameCpp& pusherReset = StaticCast<const FMyMJGamePusherResetGameCpp &>(pusher);

        pRet->m_aResultDelta.Empty();

        int32 idxBase = pRet->m_aResultBase.Emplace();
        FMyMJDataStructCpp &base = pRet->m_aResultBase[idxBase];

        genBaseFromPusherResetGame(pusherReset, base);
        
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateAttenderCardsAndState)
    {
        const FMyMJGamePusherUpdateAttenderCardsAndStateCpp& pusherUpdateAttenderCardsAndState = StaticCast<const FMyMJGamePusherUpdateAttenderCardsAndStateCpp &>(pusher);

        const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
        const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();

        TArray<FMyIdValuePair> aIdValues2Reveal;
        TArray<FMyMJCardInfoCpp> aInfos2Update;

        int32 idxAttender = pusherUpdateAttenderCardsAndState.m_iIdxAttender;

        int32 l = pusherUpdateAttenderCardsAndState.m_aIdValues.Num();
        for (int32 i = 0; i < l; i++) {
            const FMyIdValuePair& targetIdValue = pusherUpdateAttenderCardsAndState.m_aIdValues[i];

            const FMyMJCardInfoCpp& currentInfo = pCardInfoPack->getRefByIdxConst(targetIdValue.m_iId); //do a copy

            //1st, check if we need to reveal card values
            //if up, it means already revealed to everyone, we don't need to reveal anymore
            if (currentInfo.m_eFlipState != MyMJCardFlipStateCpp::Up && targetIdValue.m_iValue > 0) {
                aIdValues2Reveal.Emplace(targetIdValue);
            }

            //2nd, update card info
            if (currentInfo.m_eFlipState != pusherUpdateAttenderCardsAndState.m_eTargetState) {
                int32 idx = aInfos2Update.Emplace(currentInfo);
                aInfos2Update[idx].m_eFlipState = pusherUpdateAttenderCardsAndState.m_eTargetState;
            }
  
        }

        if (aIdValues2Reveal.Num() > 0) {
            int32 idx = delta.m_aRoleDataPrivate.Emplace();
            FMyMJRoleDataPrivateDeltaCpp& roleDataPriv = delta.m_aRoleDataPrivate[idx];
            roleDataPriv.m_aIdValuePairs2Reveal = aIdValues2Reveal;

            roleDataPriv.m_eRoleType = (MyMJGameRoleTypeCpp)idxAttender;
            if (pusherUpdateAttenderCardsAndState.m_eTargetState == MyMJCardFlipStateCpp::Up) {
                roleDataPriv.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All;
            }
            else {
                roleDataPriv.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_One(idxAttender);
            }
        }

        if (aInfos2Update.Num() > 0) {
            int32 idx = delta.m_aCoreData.Emplace();
            FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];
            coreDataDelta.m_aCardInfos2Update = aInfos2Update;
        }



    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateTing)
    {
        const FMyMJGamePusherUpdateTingCpp& pusherUpdateTing = StaticCast<const FMyMJGamePusherUpdateTingCpp &>(pusher);

        int32 idxAttender = pusherUpdateTing.m_iIdxAttender;

        int32 idx = delta.m_aRoleDataAttender.Emplace();
        FMyMJRoleDataAttenderDeltaCpp &deltaAttender = delta.m_aRoleDataAttender[idx];
        deltaAttender.m_iIdxAttender = idxAttender;

        idx = deltaAttender.m_aDataPrivate.Emplace();
        FMyMJRoleDataAttenderPrivateDeltaCpp& deltaAttenderPrivate = deltaAttender.m_aDataPrivate[idx];

        idx = deltaAttenderPrivate.m_aHuScoreResultTingGroup.Emplace();
        FMyMJHuScoreResultTingGroupCpp& tingGroup = deltaAttenderPrivate.m_aHuScoreResultTingGroup[idx];

        tingGroup = pusherUpdateTing.m_cTingGroup;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionStateUpdate)
    {
        const FMyMJGameActionStateUpdateCpp& actionStateUpdate = StaticCast<const FMyMJGameActionStateUpdateCpp &>(pusher);

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];


        if (actionStateUpdate.m_eStateNext != MyMJGameStateCpp::Invalid) {
            //pD->m_eGameState = pAction->m_eStateNext;

            coreDataDelta.m_eGameState = actionStateUpdate.m_eStateNext;
            UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);
        }

        int32 iMask = actionStateUpdate.m_iMask;
        if ((iMask & MyMJGameActionStateUpdateMaskNotResetHelperLastCardsGivenOutOrWeave) == 0) {
            UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave, true);
 
        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionNoAct)
    {
        const FMyMJGameActionNoActCpp &actionNoAct = StaticCast<const FMyMJGameActionNoActCpp &>(pusher);
        bool bPassPaoHu = UMyMJUtilsLibrary::getBoolValueFromBitMask(actionNoAct.m_iMask0, (uint8)EMyMJGameActionUnfiedMask0::PassPaoHu);
        if (bPassPaoHu) {
            int32 idxAttender = actionNoAct.getIdxAttender();

            int32 idx = delta.m_aRoleDataAttender.Emplace();
            FMyMJRoleDataAttenderDeltaCpp &deltaAttender = delta.m_aRoleDataAttender[idx];
            deltaAttender.m_iIdxAttender = idxAttender;

            idx = deltaAttender.m_aDataPrivate.Emplace();
            FMyMJRoleDataAttenderPrivateDeltaCpp& deltaAttenderPrivate = deltaAttender.m_aDataPrivate[idx];
            
            UMyMJUtilsLibrary::setUpdateFlagAndBoolValueToDeltaBitMask(deltaAttenderPrivate.m_iMask0, FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS, FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS, bPassPaoHu);
        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS)
    {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS)
    {

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pusher [%s] not valid for this core."),
            *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
    }

    return pRet;
};

void FMyMJGameCoreLocalCSCpp::applyPusher(const FMyMJGamePusherBaseCpp &pusher)
{
    const FMyMJGamePusherBaseCpp *pPusher = &pusher;


    const FMyMJGameActionNoActCpp *pActionNoAct;
    const FMyMJGameActionThrowDicesCpp *pActionThrowDices;
    const FMyMJGameActionDistCardAtStartCpp *pActionDistCardAtStart;
    const FMyMJGameActionTakeCardsCpp *pActionTakeCards;
    const FMyMJGameActionGiveOutCardsCpp *pActionGiveOutCards;
    const FMyMJGameActionWeaveCpp *pActionWeave;

    const FMyMJGameActionHuCpp *pActionHu;

    const FMyMJGameActionHuBornLocalCSCpp  *pActionHuBornLocalCS;
    const FMyMJGameActionZhaNiaoLocalCSCpp *pActionZhaNiaoLocalCS;

    MyMJGamePusherTypeCpp ePusherType = pPusher->getType();

    if (ePusherType == MyMJGamePusherTypeCpp::PusherFillInActionChoices)
    {
        const FMyMJGamePusherFillInActionChoicesCpp& pusherFillInActionChoices = StaticCast<const FMyMJGamePusherFillInActionChoicesCpp &>(pusher);

        int32 idxAttender = pusherFillInActionChoices.getIdxAttender();

        TArray<TSharedPtr<FMyMJGameActionBaseCpp>> actionChoices;
        int32 l = pusherFillInActionChoices.m_cActionChoices.getCount();

        MY_VERIFY(l > 0);

        for (int32 i = 0; i < l; i++) {
            const TSharedPtr<FMyMJGamePusherBaseCpp> pP = pusherFillInActionChoices.m_cActionChoices.getSharedPtrAtConst(i);

            MY_VERIFY((uint8)pP->getType() >(uint8)MyMJGamePusherTypeCpp::ActionBase);

            TSharedPtr<FMyMJGameActionBaseCpp> p2 = StaticCastSharedPtr<FMyMJGameActionBaseCpp>(pP);
            MY_VERIFY(p2.IsValid());

            actionChoices.Emplace(p2);
        }

        MY_VERIFY(l == actionChoices.Num());

        m_aAttendersAll[idxAttender]->getActionContainorRef().fillInNewChoices(pusherFillInActionChoices.m_iActionGroupId, actionChoices);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify) {
        const FMyMJGamePusherMadeChoiceNotifyCpp& pusherMadeChoiceNotify = StaticCast<const FMyMJGamePusherMadeChoiceNotifyCpp &>(pusher);

        int32 idxAttender = pusherMadeChoiceNotify.getIdxAttenderConst();

        m_aAttendersAll[idxAttender]->getActionContainorRef().showSelectionOnNotify(pusherMadeChoiceNotify.m_iActionGroupId, pusherMadeChoiceNotify.m_iSelection, pusherMadeChoiceNotify.m_aSubSelections);
    }

    else if (ePusherType == MyMJGamePusherTypeCpp::PusherCountUpdate) {
        const FMyMJGamePusherCountUpdateCpp& pusherCountUpdate = StaticCast<const FMyMJGamePusherCountUpdateCpp &>(pusher);

        const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

        if (pusherCountUpdate.m_bActionGroupIncrease) {
            MY_VERIFY(m_pActionCollector.IsValid());
            m_pActionCollector->setActionGroupId(pD->m_iActionGroupId);
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame) {
        const FMyMJGamePusherResetGameCpp& pusherResetGame = StaticCast<const FMyMJGamePusherResetGameCpp &>(pusher);

        applyPusherResetGame(pusherResetGame);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateAttenderCardsAndState) {
        const FMyMJGamePusherUpdateAttenderCardsAndStateCpp& pusherUpdateAttenderCardsAndState = StaticCast<const FMyMJGamePusherUpdateAttenderCardsAndStateCpp &>(pusher);

        if (pusherUpdateAttenderCardsAndState.m_iIdxAttender >= 0) {
            getRealAttenderByIdx(pusherUpdateAttenderCardsAndState.m_iIdxAttender)->dataResetByMask(pusherUpdateAttenderCardsAndState.m_iMask0);
        }
        else {
            MY_VERIFY(false);
        }
    }

    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateTing) {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionStateUpdate) {
        const FMyMJGameActionStateUpdateCpp &actionStateUpdate = StaticCast<const FMyMJGameActionStateUpdateCpp& >(pusher);

        if (actionStateUpdate.m_eStateNext != MyMJGameStateCpp::Invalid) {
            //pD->m_eGameState = pAction->m_eStateNext;
            resetForNewLoop(NULL, NULL, actionStateUpdate.m_iAttenderMaskNext, actionStateUpdate.m_bAllowSamePriAction, actionStateUpdate.m_iIdxAttenderHavePriMax);
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionNoAct) {
        pActionNoAct = StaticCast<const FMyMJGameActionNoActCpp *>(pPusher);
        //getRealAttenderByIdx(pActionNoAct->getIdxAttender())->applyActionNoAct(pActionNoAct);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {
        pActionThrowDices = StaticCast<const FMyMJGameActionThrowDicesCpp *>(pPusher);
        applyActionThrowDices(*pActionThrowDices);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
        pActionDistCardAtStart = StaticCast<const FMyMJGameActionDistCardAtStartCpp *>(pPusher);
        applyActionDistCardsAtStart(*pActionDistCardAtStart);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
        pActionTakeCards = StaticCast<const FMyMJGameActionTakeCardsCpp *>(pPusher);
        applyActionTakeCards(*pActionTakeCards);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
        pActionGiveOutCards = StaticCast<const FMyMJGameActionGiveOutCardsCpp *>(pPusher);
        applyActionGiveOutCards(*pActionGiveOutCards);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
        pActionWeave = StaticCast<const FMyMJGameActionWeaveCpp *>(pPusher);
        applyActionWeave(*pActionWeave);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {
        pActionHu = StaticCast<const FMyMJGameActionHuCpp *>(pPusher);
        applyActionHu(*pActionHu);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {
        pActionHuBornLocalCS = StaticCast<const FMyMJGameActionHuBornLocalCSCpp *>(pPusher);
        getRealAttenderByIdx(pActionHuBornLocalCS->getIdxAttender())->applyActionHuBornLocalCS(*pActionHuBornLocalCS);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {
        pActionZhaNiaoLocalCS = StaticCast<const FMyMJGameActionZhaNiaoLocalCSCpp *>(pPusher);
        applyActionZhaNiaoLocalCS(*pActionZhaNiaoLocalCS);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pusher [%s] not valid for this core."),
            *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
    }


}

void FMyMJGameCoreLocalCSCpp::handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp &cmd)
{
    FMyMJGameCmdBaseCpp *pCmd = &cmd;
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    if (pCmd->m_eType == MyMJGameCmdType::RestartGame) {
        if (eRoleTypeOfCmdSrc == MyMJGameRoleTypeCpp::SysKeeper) {

            FMyMJGameCmdRestartGameCpp *pCmdRestartGame = StaticCast<FMyMJGameCmdRestartGameCpp *>(pCmd);

            MY_VERIFY(getRuleType() != MyMJGameRuleTypeCpp::Invalid);
            if (pCmdRestartGame->m_cGameCfg.m_eRuleType == getRuleType()) {
                FMyMJGamePusherResetGameCpp *pPusherReset = new FMyMJGamePusherResetGameCpp();
                FRandomStream &RS = m_pResManager->getRandomStreamRef();
                int32 iGameIdOld = pD->m_iGameId;

                int32 iGameId = iGameIdOld + 1;
                if (iGameId < 0) {
                    iGameId = 0;
                }
                pPusherReset->init(iGameId, RS, pCmdRestartGame->m_cGameCfg, pCmdRestartGame->m_cGameRunData, pCmdRestartGame->m_iAttenderRandomSelectMask);

                m_pPusherIOFull->GivePusher(pPusherReset, (void**)&pPusherReset);

                pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::None;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("requested reset game but type not equal: cfg %d, self %d."), (uint8)pCmdRestartGame->m_cGameCfg.m_eRuleType, (uint8)getRuleType());
                pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::GameRuleTypeNotEqual;
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("requested reset game from invalid role: %d"), (uint8)eRoleTypeOfCmdSrc);
            pCmd->m_eRespErrorCode = MyMJGameErrorCodeCpp::HaveNoAuthority;
        }

    }
    else {
        MY_VERIFY(false);
    }
}


void FMyMJGameCoreLocalCSCpp::genBaseFromPusherResetGame(const FMyMJGamePusherResetGameCpp &pusherReset, FMyMJDataStructCpp &outBase)
{
    FMyMJDataStructCpp &base = outBase;

    //Stateless, never judge condition of previous state, just reset the whole core
    FMyMJCoreDataPublicCpp *pD = const_cast<FMyMJCoreDataPublicCpp *> (&base.getCoreDataRefConst());

    //pD->reinit(pPusher->m_cGameCfg.m_eRuleType);

    FMyMJCardInfoPackCpp  *pCardInfoPack = &pD->m_cCardInfoPack;

    //let's construct things
    int32 iRealAttenderNum = pusherReset.m_cGameCfg.m_cTrivialCfg.m_iGameAttenderNum;


    if (iRealAttenderNum == 2) {
        for (int32 i = 0; i < 2; i++) {

            FMyMJRoleDataAttenderPublicCpp &cRoleDataAttenderPub = const_cast<FMyMJRoleDataAttenderPublicCpp &> (base.getRoleDataAttenderPublicRefConst(i * 2));
            UMyMJUtilsLibrary::setBoolValueToBitMask(cRoleDataAttenderPub.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender, true);
            UMyMJUtilsLibrary::setBoolValueToBitMask(cRoleDataAttenderPub.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame, true);
        }
    }
    else if (iRealAttenderNum >= 3 && iRealAttenderNum < 5) {
        for (int32 i = 0; i < iRealAttenderNum; i++) {

            FMyMJRoleDataAttenderPublicCpp &cRoleDataAttenderPub = const_cast<FMyMJRoleDataAttenderPublicCpp &>  (base.getRoleDataAttenderPublicRefConst(i));
            UMyMJUtilsLibrary::setBoolValueToBitMask(cRoleDataAttenderPub.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender, true);
            UMyMJUtilsLibrary::setBoolValueToBitMask(cRoleDataAttenderPub.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame, true);
        }
    }
    else {
        MY_VERIFY(false);
    }

    pD->m_iGameId = pusherReset.m_iGameId;
    pD->m_iPusherIdLast = 0;
    pD->m_iActionGroupId = 0;


    pD->m_eGameState = MyMJGameStateCpp::CardsShuffled;

    pD->m_iDiceNumberNowMask = 0;
    pD->m_cUntakenSlotInfo.reset();

    //let's fill in
    pD->m_cGameCfg = pusherReset.m_cGameCfg;
    pD->m_cGameRunData = pusherReset.m_cGameRunData;

    int32 iCardNum = pusherReset.m_aShuffledValues.Num();
    pCardInfoPack->reset(iCardNum);

    //assert we are full mode
    //if (getWorkMode() == MyMJGameCoreWorkModeCpp::Full) {
    {

        for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            FMyMJCardValuePackCpp *pCardValuePack = const_cast<FMyMJCardValuePackCpp *> (&base.getRoleDataPrivateRefConst(i).m_cCardValuePack);
            if (i == (uint8)MyMJGameRoleTypeCpp::SysKeeper) {
                pCardValuePack->resetAndRevealAll(pusherReset.m_aShuffledValues);
                pCardValuePack->helperVerifyValues();
            }
            else {
                pCardValuePack->reset(iCardNum);
            }
        }
    }


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

        //pRet->getRoleDataAttenderPublicRefConst(i);

        FMyMJRoleDataAttenderPublicCpp *pRoleDataAttenderPub = const_cast<FMyMJRoleDataAttenderPublicCpp *> (&base.getRoleDataAttenderPublicRefConst(i));

        pRoleDataAttenderPub->m_cUntakenSlotSubSegmentInfo.m_iIdxStart = idxStart;
        pRoleDataAttenderPub->m_cUntakenSlotSubSegmentInfo.m_iLength = stackL;
        //m_aAttendersAll[i]->getpUntakenSlotSubSegmentInfo()->m_iIdxStart = idxStart;
        //m_aAttendersAll[i]->getpUntakenSlotSubSegmentInfo()->m_iLength = stackL;

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

    pCardInfoPack->helperVerifyInfos();
}


void FMyMJGameCoreLocalCSCpp::applyPusherResetGame(const FMyMJGamePusherResetGameCpp &pusher)
{
    resetDatasOwned();
    for (int i = 0; i < 4; i++) {
        TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i];
        pAttender->resetDatasOwned();

    }

    //let's construct things
    int32 iRealAttenderNum = pusher.m_cGameCfg.m_cTrivialCfg.m_iGameAttenderNum;

    TArray<FMyMJGameActionContainorCpp *> aContainors;

    if (iRealAttenderNum == 2) {
        for (int32 i = 0; i < 2; i++) {

            TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i * 2];

            aContainors.Emplace(&pAttender->getActionContainorRef());
        }
    }
    else if (iRealAttenderNum >= 3 && iRealAttenderNum < 5) {
        for (int32 i = 0; i < iRealAttenderNum; i++) {

            TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i];

            aContainors.Emplace(&pAttender->getActionContainorRef());
        }
    }
    else {
        MY_VERIFY(false);
    }

    MY_VERIFY(getRuleType() == pusher.m_cGameCfg.m_eRuleType)


    if (m_pActionCollector.IsValid()) {
        m_pActionCollector->reinit(aContainors, pusher.m_iAttenderBehaviorRandomSelectMask);
    }
    else {
        MY_VERIFY(false);
    }

    int32 uMask = genIdxAttenderStillInGameMaskOne(pusher.m_cGameRunData.m_iIdxAttenderMenFeng);
    resetForNewLoop(NULL, NULL, uMask, false, pusher.m_cGameRunData.m_iIdxAttenderMenFeng);

    m_cDataLogic.m_eActionLoopState = MyMJActionLoopStateCpp::WaitingToGenAction;
    m_cDataLogic.m_iMsLast = UMyMJUtilsLibrary::nowAsMsFromTick();

}


void FMyMJGameCoreLocalCSCpp::applyActionThrowDices(const FMyMJGameActionThrowDicesCpp &action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    //idx is always 1 - 4
    int32 idxAttender = action.getIdxAttender();

    int32 iDiceNumberNow0, iDiceNumberNow1, trash;
    action.getDiceNumbers(iDiceNumberNow0, iDiceNumberNow1, trash);

    MyMJGameActionThrowDicesSubTypeCpp eSubType = action.getSubType();

    if (eSubType == MyMJGameActionThrowDicesSubTypeCpp::GameStart) {

        const FMyMJRoleDataAttenderPublicCpp *pDPubD = &m_aAttendersAll[idxAttender]->getRoleDataAttenderPublicRefConst();

        //FMyMJRoleDataAttenderPrivateCpp *pDPriD = getDataPrivateDirect();
        //MY_VERIFY(pDPriD);


        int32 iBase = pDPubD->m_cUntakenSlotSubSegmentInfo.m_iIdxStart;
        //int32 len = m_aUntakenCardStacks.Num();
        int32 len = pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal;
        MY_VERIFY(len > 0);
        //pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow = (iBase + iDiceNumberNow0 + iDiceNumberNow1 - 1 + len) % len;
        //pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow = pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailAtStart = (pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow - 1 + len) % len;

        int32 cardNumKept = pD->m_cGameCfg.m_cTrivialCfg.m_iStackNumKeptFromTail * pD->m_cGameCfg.m_cTrivialCfg.m_iCardNumPerStackInUntakenSlot;
        MY_VERIFY(pD->m_cUntakenSlotInfo.calcUntakenSlotCardsLeftNumKeptFromTailConst(pD->m_aUntakenCardStacks) == cardNumKept);
        //pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail = cardNumKept;
        //pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead = pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal - pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail;

        //pD->m_eGameState = MyMJGameStateCpp::CardsWaitingForDistribution;

        //if (getWorkMode() == MyMJGameCoreWorkModeCpp::Full) {
            //int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);
            int32 uMask = genIdxAttenderStillInGameMaskOne(idxAttender);
            resetForNewLoop(NULL, NULL, uMask, false, idxAttender);
        //}
    }
    else if (eSubType == MyMJGameActionThrowDicesSubTypeCpp::GangYaoLocalCS) {
        //pD->m_eGameState = MyMJGameStateCpp::WeavedGangDicesThrownLocalCS;

        //if (getWorkMode() == MyMJGameCoreWorkModeCpp::Full) {
            int32 uMask = genIdxAttenderStillInGameMaskOne(idxAttender);
            resetForNewLoop(NULL, NULL, uMask, false, idxAttender);
        //}
    }
    else {
        MY_VERIFY(false);
    }
}

void FMyMJGameCoreLocalCSCpp::applyActionDistCardsAtStart(const FMyMJGameActionDistCardAtStartCpp &action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();

    int32 idxAttender = action.getIdxAttender();

    int32 l = action.m_aIdValues.Num();

    for (int32 i = 0; i < l; i++) {
        //1, reveal value
        int32 id = action.m_aIdValues[i].m_iId;
        int32 value = action.m_aIdValues[i].m_iValue;
        //pCardValuePack->revealCardValue(id, value);

        //2, move card
        moveCardFromOldPosi(id);
        moveCardToNewPosi(id, idxAttender, MyMJCardSlotTypeCpp::InHand);
    }
    updateUntakenInfoHeadOrTail(true, false);

    //3rd, update state
    if (action.m_bLastCard) {

        getRealAttenderByIdx(idxAttender)->onNewTurn(false);

        //pD->m_eGameState = MyMJGameStateCpp::CardsDistributedWaitingForLittleHuLocalCS;
        int32 iMask = genIdxAttenderStillInGameMaskAll();
        int32 idxZhuang = pD->m_cGameRunData.m_iIdxAttenderMenFeng;

        FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
        pActionUpdate->m_eStateNext = MyMJGameStateCpp::JustStarted;
        pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxZhuang);
        resetForNewLoop(NULL, pActionUpdate, iMask, true, idxZhuang);


            for (int32 i = 0; i < 4; i++) {

                TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender = getAttenderByIdx(i);
                MY_VERIFY(pAttender.IsValid());
                if (i == idxZhuang || pAttender->getIsStillInGame() == false) {
                    continue;
                }
                MY_VERIFY(pAttender->getIsRealAttender());
                pAttender->tryGenAndEnqueueUpdateTingPusher();
            }


    }
    else {
        int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);
        int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
        resetForNewLoop(NULL, NULL, iMask, false, idxAttenderNext);
    }
}

void FMyMJGameCoreLocalCSCpp::applyActionTakeCards(const FMyMJGameActionTakeCardsCpp& action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();

    int32 idxAttender = action.getIdxAttender();

    //1st, reveal value
    //m_cCardPack.revealCardValueByIdValuePairs(pAction->m_aIdValuePairs);

    //2nd, move card and flip
    int32 l = action.m_aIdValuePairs.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(action.m_aIdValuePairs[i].m_iId);
        moveCardFromOldPosi(pCardInfo->m_iId);
        moveCardToNewPosi(pCardInfo->m_iId, idxAttender, MyMJCardSlotTypeCpp::JustTaken);

        //pCardInfo->m_eFlipState = MyMJCardFlipStateCpp::Stand;
    }

    MyMJGameCardTakenOrderCpp eTakenOrder = action.m_eTakenOrder;
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

    if (action.m_bIsGang) {
        //pD->m_eGameState = MyMJGameStateCpp::WeavedGangTakenCards;
    }
    else {
        //pD->m_eGameState = MyMJGameStateCpp::TakenCard;
    }
    int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);

    resetForNewLoop(NULL, NULL, iMask, false, idxAttender);

}

void FMyMJGameCoreLocalCSCpp::applyActionGiveOutCards(const FMyMJGameActionGiveOutCardsCpp& action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();

    int32 idxAttender = action.getIdxAttender();
    MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
    TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender = getRealAttenderByIdx(idxAttender);
    MY_VERIFY(pAttender.IsValid());

    int32 selectedCount = action.m_aIdValuePairsSelected.Num();
    MY_VERIFY(selectedCount > 0);

    //1st, update card value
    //m_cCardPack.revealCardValueByIdValuePairs(pAction->m_aIdValuePairsSelected);

    //precheck if ting update is needed
    bool bNeedUpdateTing = false;
    
    if (action.m_bRestrict2SelectCardsJustTaken) {

    }
    else {
        MY_VERIFY(selectedCount == 1);
        int32 valueSelected = pCardValuePack->getByIdx(action.m_aIdValuePairsSelected[0].m_iId);
        MY_VERIFY(valueSelected > 0);


        {
            const FMyMJRoleDataAttenderPublicCpp *pDPubD = &pAttender->getRoleDataAttenderPublicRefConst();
            MY_VERIFY(pDPubD);
            //FMyMJRoleDataAttenderPrivateCpp *pDPriD = getDataPrivateDirect();
            //MY_VERIFY(pDPriD);

            const TArray<int32> &aIdCardsTaken = pDPubD->m_aIdJustTakenCards;// pAttender->getIdJustTakenCardsRef();
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
        const FMyIdValuePair &pair = action.m_aIdValuePairsSelected[i];
        moveCardFromOldPosi(pair.m_iId);
        moveCardToNewPosi(pair.m_iId, action.getIdxAttender(), MyMJCardSlotTypeCpp::GivenOut);

        const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(pair.m_iId);
        //pCardInfo->m_eFlipState = MyMJCardFlipStateCpp::Up;
    }

    const FMyMJRoleDataAttenderPublicCpp *pDPubD = &pAttender->getRoleDataAttenderPublicRefConst();

    TArray<int32> aIdsJustTaken = pDPubD->m_aIdJustTakenCards;// pAttender->getIdJustTakenCardsRef(); //do a copy
    int32 l0 = aIdsJustTaken.Num();
    for (int32 i = 0; i < l0; i++) {
        int32 idCard = aIdsJustTaken[i];
        moveCardFromOldPosi(idCard);
        moveCardToNewPosi(idCard, idxAttender, MyMJCardSlotTypeCpp::InHand);
    }

    //3rd, update helper
    //pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    for (int32 i = 0; i < selectedCount; i++) {
        const FMyIdValuePair &pair = action.m_aIdValuePairsSelected[i];
        //pD->m_aHelperLastCardsGivenOutOrWeave.Emplace(pair);

        m_cDataLogic.m_cHelperShowedOut2AllCards.insert(pair);
    }


    //4th, update core state
    if (action.m_bIsGang) {
        //pD->m_eGameState = MyMJGameStateCpp::WeavedGangGivenOutCards;
    }
    else {
       // pD->m_eGameState = MyMJGameStateCpp::GivenOutCard;
    }
    int32 iMask = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
    int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);

    FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
    pActionUpdate->m_eStateNext = MyMJGameStateCpp::WaitingForTakeCard;
    pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
    resetForNewLoop(pActionUpdate, NULL, iMask, pD->m_cGameCfg.m_cSubLocalCSCfg.m_bHuAllowMultiple, idxAttenderNext);



    //5th, updateTing
    //if (getWorkMode() == MyMJGameCoreWorkModeCpp::Full && bNeedUpdateTing) {
    if (bNeedUpdateTing) {
        pAttender->tryGenAndEnqueueUpdateTingPusher();
    }


}

//TOdo: gang code path, and TArray member delete when using tmap

void FMyMJGameCoreLocalCSCpp::applyActionWeave(const FMyMJGameActionWeaveCpp& action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();

    int32 idxAttender = action.getIdxAttender();

    //1st, attender apply
    getRealAttenderByIdx(idxAttender)->applyActionWeave(action);

    MyMJWeaveTypeCpp eType = action.m_cWeave.getType();

    //2nd, update helper
    //pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    if (action.m_eTargetFlipState == MyMJCardFlipStateCpp::Up) {
        TArray<FMyIdValuePair> aIdValues;
        action.m_cWeave.getIdValues(*pCardValuePack, aIdValues, false);
        m_cDataLogic.m_cHelperShowedOut2AllCards.insertIdValuePairs(aIdValues, true);
    }

    //3rd, update the state
    getRealAttenderByIdx(idxAttender)->onNewTurn(true);

    MyMJGameStateCpp eGameStateNow = MyMJGameStateCpp::Invalid;
    int32 iAttenderMaskNow = 0;
    bool bAllowSamePriActionNow = false;
    int32 iIdxAttenderHavePriMaxNow = 0;
    FMyMJGameActionStateUpdateCpp *pActionUpdatePrev = NULL;
    if (eType == MyMJWeaveTypeCpp::GangAn || eType == MyMJWeaveTypeCpp::GangMing) {
        int32 reserved0 = action.m_cWeave.getReserved0();
        bool bIsBuZhang = (reserved0 & (uint8)EMyMJWeaveReserved0Mask::LocalCSGangBuZhang) > 0;

        if (!bIsBuZhang) {
            if (eType == MyMJWeaveTypeCpp::GangMing && action.m_cWeave.getTypeConsumed() == MyMJWeaveTypeCpp::ShunZiMing) {
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

    FMyIdValuePair cIdValue;
    cIdValue.m_iId = action.m_cWeave.getRepresentCardId();
    cIdValue.m_iValue = pCardValuePack->getByIdx(cIdValue.m_iId);

    //pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
    //pD->m_aHelperLastCardsGivenOutOrWeave.Emplace(cIdValue);

    //pD->m_eGameState = eGameStateNow;

    resetForNewLoop(pActionUpdatePrev, NULL, iAttenderMaskNow, bAllowSamePriActionNow, iIdxAttenderHavePriMaxNow);
}

void FMyMJGameCoreLocalCSCpp::applyActionHu(const FMyMJGameActionHuCpp& action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    int32 idxAttender = action.getIdxAttender();
    TSharedPtr<FMyMJGameAttenderCpp> pAttender = getRealAttenderByIdx(idxAttender);

    int32 l = action.m_aRevealingCards.Num();
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
    //pD->m_aHelperLastCardsGivenOutOrWeave.Reset();

    if (action.m_bEndGame) {

        const FMyMJRoleDataAttenderPublicCpp *pDPubD = &m_aAttendersAll[idxAttender]->getRoleDataAttenderPublicRefConst();
        MY_VERIFY(pDPubD);
        //FMyMJRoleDataAttenderPrivateCpp *pDPriD = getDataPrivateDirect();
        //MY_VERIFY(pDPriD);

        //pAttender->getHuScoreResultFinalGroupRef() = pAction->m_cHuScoreResultFinalGroup;
        //pDPubD->m_cHuScoreResultFinalGroup = pAction->m_cHuScoreResultFinalGroup;

        //pD->m_eGameState = MyMJGameStateCpp::JustHu;
        int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);

        FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
        pActionUpdate->m_eStateNext = MyMJGameStateCpp::GameEnd;
        pActionUpdate->m_iAttenderMaskNext = 0;
        pActionUpdate->m_eReason = MyMJGameStateUpdateReasonCpp::AttenderHu;

        resetForNewLoop(NULL, pActionUpdate, iMask, false, idxAttender);

    }
    else {
        MY_VERIFY(false); //CS MJ always end game when hu in game
    }
}

void FMyMJGameCoreLocalCSCpp::applyActionZhaNiaoLocalCS(const FMyMJGameActionZhaNiaoLocalCSCpp& action)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataRefConst();

    const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();


    //pCardPack->revealCardValueByIdValuePairs(pAction->m_aPickedIdValues);

    int32 idxAttenderBase = action.getIdxAttender();
    MY_VERIFY(idxAttenderBase >= 0 && idxAttenderBase < 4);

    int32 l = action.m_aPickedIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyIdValuePair &card = action.m_aPickedIdValues[i];
        int32 cardId = card.m_iId;
        int32 cardValue = card.m_iValue;
        int32 modVFix = cardValue % 10 - 1;
        MY_VERIFY(modVFix >= 0 && modVFix < 9);

        int32 idxAttenderTarget = findIdxAttenderStillInGame(idxAttenderBase, modVFix, false);
        moveCardFromOldPosi(cardId);
        moveCardToNewPosi(cardId, idxAttenderTarget, MyMJCardSlotTypeCpp::WinSymbol); //We use WinSymbol slot as niao in local CS game

        m_cDataLogic.m_cHelperShowedOut2AllCards.insert(card);
    }

    //Update state
    //pD->m_aHelperLastCardsGivenOutOrWeave.Reset();
}