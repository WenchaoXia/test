// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreLocalCS.h"
#include "Utils/MyMJUtilsLocalCS.h"
#include "Kismet/KismetMathLibrary.h"

void FMyMJGameCoreLocalCSCpp::genActionChoices()
{
    FMyMJGameCoreCpp::genActionChoices();

    const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();
    const FMyMJCardValuePackCpp& cardValuePack = getCardValuePackOfSysKeeperRefConst();

    MyMJGameStateCpp eGameState = coreDataSelf.m_eGameState;

    if (eGameState == MyMJGameStateCpp::JustHu) {
        int32 idxHeadNow = coreDataSelf.m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow;
        TArray<int32> aIds;

        int32 c = coreDataSelf.m_cGameCfg.getSubLocalCSCfgRefConst().m_iZhaNiaoCount;
        if (c <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("zhaniao count cfg <= 0."));
            return;
        }

        tryCollectCardsFromUntakenSlot(idxHeadNow, (uint32)c, false, aIds);
        c = aIds.Num();

        FMyMJGameActionZhaNiaoLocalCSCpp *pAction = new FMyMJGameActionZhaNiaoLocalCSCpp();
        if (c > 0) {

            for (int32 i = 0; i < c; i++) {
                int32 idx = pAction->m_aPickedIdValues.Emplace();
                pAction->m_aPickedIdValues[idx].m_iId = aIds[i];
                pAction->m_aPickedIdValues[idx].m_iValue = cardValuePack.getByIdx(aIds[i]);
            }

        }
        else {
            //this is the last card
            int32 idHaiDiCard = UMyMJUtilsLocalCSLibrary::getHaiDiCard(coreDataSelf.m_cHelper);

            if (idHaiDiCard >= 0) {
                //we have haidi card
                int32 idx = pAction->m_aPickedIdValues.Emplace();
                pAction->m_aPickedIdValues[idx].m_iId = idHaiDiCard;
                pAction->m_aPickedIdValues[idx].m_iValue = cardValuePack.getByIdx(idHaiDiCard);
            }
            else {
                delete(pAction);
                pAction = NULL;
            }

        }

        if (pAction) {
            /*
            int32 idxAttender = -1;
            int32 idxBase = coreDataSelf.m_cGameRunData.m_iIdxAttenderMenFeng;
            for (int32 i = 0; i < 4; i++) {
                const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublic = m_cDataAccessor.getRoleDataAttenderPublicRefConst((idxBase + 1) % 4);
                if (roleDataAttenderPublic.m_aHuScoreResultFinalGroups.Num() <= 0) {
                    continue;
                }

                if (roleDataAttenderPublic.m_aHuScoreResultFinalGroups.Top().m_eHuMainType == MyMJHuMainTypeCpp::Common) {
                    idxAttender = i;
                    break;
                }
            }

            MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
            */

            //find first hu attender
            int32 idxAttenderLoseOnlyOne = -1;
            int32 idxWinnder = -1;
            for (int32 i = 0; i < 4; i++) {
                const FMyMJRoleDataAttenderPublicCpp& cRoleDataAttenderPublic = getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(i);
                if (cRoleDataAttenderPublic.m_aHuScoreResultFinalGroups.Num() <= 0) {
                    continue;
                }
                if (idxWinnder < 0) {
                    idxWinnder = cRoleDataAttenderPublic.m_aHuScoreResultFinalGroups[0].m_iIdxAttenderWin;
                    MY_VERIFY(idxWinnder == i);
                }
                if (cRoleDataAttenderPublic.m_aHuScoreResultFinalGroups[0].m_iIdxAttenderLoseOnlyOne >= 0) {
                    idxAttenderLoseOnlyOne = cRoleDataAttenderPublic.m_aHuScoreResultFinalGroups[0].m_iIdxAttenderLoseOnlyOne;
                    break;
                }
            }
            MY_VERIFY(idxWinnder >= 0);
            if (idxAttenderLoseOnlyOne >= 0) {
                //search
                idxWinnder = -1;
                for (int32 i = 1; i < 4; i++) {
                    int32 idxAttender = (idxAttenderLoseOnlyOne + 1) % 4;
                    const FMyMJRoleDataAttenderPublicCpp& cRoleDataAttenderPublic = getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(idxAttender);
                    if (cRoleDataAttenderPublic.m_aHuScoreResultFinalGroups.Num() <= 0) {
                        continue;
                    }
                    idxWinnder = cRoleDataAttenderPublic.m_aHuScoreResultFinalGroups[0].m_iIdxAttenderWin;
                    MY_VERIFY(idxWinnder == idxAttender);
                    break;
                }
                MY_VERIFY(idxWinnder >= 0);
            }


            pAction->initWithPickedIdValuesInited(idxWinnder);

            getPusherIOFullRef().GivePusher(pAction, (void **)&pAction);
        }
    }
}

bool FMyMJGameCoreLocalCSCpp::prevApplyPusherResult(const FMyMJGamePusherResultCpp &pusherResult)
{
    bool bRet = FMyMJGameCoreCpp::prevApplyPusherResult(pusherResult);

    if (pusherResult.m_aResultDelta.Num() > 0) {
        const FMyMJDataDeltaCpp& delta = pusherResult.m_aResultDelta[0];
        MyMJGamePusherTypeCpp ePusherType = delta.getType();

        if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
            int32 idxAttender = delta.getIdxAttenderActionInitiator();

            //check if need bNeedUpdateTing, the code was here because before result apply we have a chance to detect if we can skip ting update for performance
            bool bNeedUpdateTing = true;

            //only one case we can skip: the cards just taken, exactly match the cards wii be given out
            if (delta.m_aCoreData.Num() > 0) {

                const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublic = m_cDataAccessor.getRoleDataAttenderPublicRefConst(idxAttender);
                int32 l0 = roleDataAttenderPublic.m_aIdJustTakenCards.Num();

                const FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[0];
                int32 l1 = coreDataDelta.m_aCardInfos2Update.Num();
                MY_VERIFY(l1 > 0);

                if (l0 == l1) {
                    bNeedUpdateTing = false;
                    for (int32 i = 0; i < l0; i++) {
                        int32 idCard = roleDataAttenderPublic.m_aIdJustTakenCards[i];
                        int32 idx = FMyMJCardInfoPackCpp::helperFindCardByIdInCardInfos(coreDataDelta.m_aCardInfos2Update, idCard);
                        if (idx >= 0 && coreDataDelta.m_aCardInfos2Update[idx].m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::GivenOut) {

                        }
                        else {
                            bNeedUpdateTing = true;
                            break;
                        }
                    }
                }


            }

            if (bNeedUpdateTing) {
                getRealAttenderByIdx(idxAttender)->tryGenAndEnqueueUpdateTingPusher();
            }
        }
    }

    return bRet;
}

FMyMJGamePusherResultCpp* FMyMJGameCoreLocalCSCpp::genPusherResultAsSysKeeper(const FMyMJGamePusherBaseCpp &pusher)
{
    const FMyMJGamePusherBaseCpp *pPusherIn = &pusher;

    MyMJGamePusherTypeCpp ePusherType = pPusherIn->getType();

    const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();
    const FMyMJCardInfoPackCpp&  cardInfoPack = getCardInfoPackRefConst();
    const FMyMJCardValuePackCpp& cardValuePack = getCardValuePackOfSysKeeperRefConst();

    FMyMJGamePusherResultCpp* pRet = new FMyMJGamePusherResultCpp();

    //by default we gen a delta and set the common data
    int32 idxDelta = pRet->m_aResultDelta.Emplace();
    FMyMJDataDeltaCpp& delta = pRet->m_aResultDelta[idxDelta];

    delta.m_iGameId = coreDataSelf.m_iGameId;
    StaticCast<FMyMJGamePusherBaseCpp&>(delta) = pusher;

    if (ePusherType == MyMJGamePusherTypeCpp::PusherFillInActionChoices)
    {
        const FMyMJGamePusherFillInActionChoicesCpp& pusherFillIn = StaticCast< const FMyMJGamePusherFillInActionChoicesCpp&>(pusher);

        int32 idxAttender = pusherFillIn.getIdxAttender();
        //delta.m_iIdxAttenderInitiator = idxAttender;

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
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pusher %s failed to gen unified action!"),
                    *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)p2->getType()));
                MY_VERIFY(false);
            }

        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify)
    {
        const FMyMJGamePusherMadeChoiceNotifyCpp& pusherMadeChoice = StaticCast< const FMyMJGamePusherMadeChoiceNotifyCpp&>(pusher);

        int32 idxAttender = pusherMadeChoice.getIdxAttenderConst();
        //delta.m_iIdxAttenderInitiator = idxAttender;

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

        const FMyMJRoleDataAttenderPrivateCpp& rp = getAttenderByIdx(idxAttender)->getRoleDataAttenderPrivateRefConst();
        if (rp.m_eAIStrategyTypeUsedLast != pusherMadeChoice.m_eAIStrategyTypeUsedForSelected) {
            deltaAttenderPrivate.m_eAIStrategyTypeUsedLast = pusherMadeChoice.m_eAIStrategyTypeUsedForSelected;
            deltaAttenderPrivate.m_bUpdateAIStrategyTypeUsedLast = true;
        }
        else {
            deltaAttenderPrivate.m_bUpdateAIStrategyTypeUsedLast = false;
        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherCountUpdate)
    {
        const FMyMJGamePusherCountUpdateCpp& pusherCountUpdate = StaticCast< const FMyMJGamePusherCountUpdateCpp&>(pusher);

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        if (pusherCountUpdate.m_bActionGroupIncrease) {
            coreDataDelta.m_bIncreaseActionGroupId = true;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_IncreaseActionGroupId, true);
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame)
    {
        const FMyMJGamePusherResetGameCpp& pusherReset = StaticCast<const FMyMJGamePusherResetGameCpp &>(pusher);

        pRet->m_aResultDelta.Empty();

        int32 idxBase = pRet->m_aResultBase.Emplace();
        FMyMJDataStructCpp &base = pRet->m_aResultBase[idxBase];

        genBaseFromPusherResetGame(*m_pResManager, pusherReset, getCoreDataRefConst(), base);
        
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateAttenderCardsAndState)
    {
        const FMyMJGamePusherUpdateAttenderCardsAndStateCpp& pusherUpdateAttenderCardsAndState = StaticCast<const FMyMJGamePusherUpdateAttenderCardsAndStateCpp &>(pusher);

        TArray<FMyIdValuePair> aIdValues2Reveal;
        TArray<FMyMJCardInfoCpp> aInfos2Update;

        int32 idxAttender = pusherUpdateAttenderCardsAndState.m_iIdxAttender;

        int32 l = pusherUpdateAttenderCardsAndState.m_aIdValues.Num();
        for (int32 i = 0; i < l; i++) {
            const FMyIdValuePair& targetIdValue = pusherUpdateAttenderCardsAndState.m_aIdValues[i];

            const FMyMJCardInfoCpp& currentInfo = cardInfoPack.getRefByIdxConst(targetIdValue.m_iId); //do a copy

            //1st, check if we need to reveal card values
            //if up, it means already revealed to everyone, we don't need to reveal anymore
            if (currentInfo.m_eFlipState != MyBoxFlipStateCpp::Up && targetIdValue.m_iValue > 0) {
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

            roleDataPriv.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
            if (pusherUpdateAttenderCardsAndState.m_eTargetState == MyBoxFlipStateCpp::Up) {
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
            coreDataDelta.m_bUpdateGameState = true;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);
        }

        int32 iMask = actionStateUpdate.m_iMask;
        if ((iMask & MyMJGameActionStateUpdateMaskNotResetHelperLastCardsGivenOutOrWeave) == 0) {
            coreDataDelta.m_bResetHelperLastCardsGivenOutOrWeave = true;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave, true);
 
        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionNoAct)
    {
        const FMyMJGameActionNoActCpp &actionNoAct = StaticCast<const FMyMJGameActionNoActCpp &>(pusher);
        int32 idxAttender = actionNoAct.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        //bool bPassPaoHu = UMyMJUtilsLibrary::getBoolValueFromBitMask(actionNoAct.m_iMask0, (uint8)EMyMJGameActionUnfiedMask0::PassPaoHu);
        if (actionNoAct.m_bPassPaoHu) {

            int32 idx = delta.m_aRoleDataAttender.Emplace();
            FMyMJRoleDataAttenderDeltaCpp &deltaAttender = delta.m_aRoleDataAttender[idx];
            deltaAttender.m_iIdxAttender = idxAttender;

            idx = deltaAttender.m_aDataPrivate.Emplace();
            FMyMJRoleDataAttenderPrivateDeltaCpp& deltaAttenderPrivate = deltaAttender.m_aDataPrivate[idx];
            
            //UMyMJUtilsLibrary::setUpdateFlagAndBoolValueToDeltaBitMask(deltaAttenderPrivate.m_iMask0, FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS, FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS, bPassPaoHu);
            deltaAttenderPrivate.m_bUpdateBanPaoHuLocalCS = true;
            deltaAttenderPrivate.m_bBanPaoHuLocalCS = actionNoAct.m_bPassPaoHu;
        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices)
    {
        const FMyMJGameActionThrowDicesCpp &actionThrowDice = StaticCast<const FMyMJGameActionThrowDicesCpp &>(pusher);
        int32 idxAttender = actionThrowDice.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        int32 iDiceNumberNow0, iDiceNumberNow1;
        actionThrowDice.getDiceNumbers(&iDiceNumberNow0, &iDiceNumberNow1);

        int32 inGameDiceThrowCount = UMyMJUtilsLibrary::getIntValueFromBitMask(coreDataSelf.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_InGameDiceThrowCount_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_InGameDiceThrowCount_BitLen);

        UMyMJUtilsLibrary::setIntValueToBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen, iDiceNumberNow0);
        UMyMJUtilsLibrary::setIntValueToBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen, iDiceNumberNow1);

        int32 iReason = actionThrowDice.getDiceReason();
        if (iReason == FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GameStart) {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::CardsWaitingForDistribution;
            coreDataDelta.m_bUpdateGameState = true;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);

            UMyMJUtilsLibrary::setIntValueToBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GameStart);
        }
        else if (iReason == FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GangYaoLocalCS) {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedGangDicesThrownLocalCS;
            coreDataDelta.m_bUpdateGameState = true;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);

            UMyMJUtilsLibrary::setIntValueToBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GangYaoLocalCS);
        }
        else {
            MY_VERIFY(false);
        }

        UMyMJUtilsLibrary::setIntValueToBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitLen, idxAttender);
        UMyMJUtilsLibrary::setIntValueToBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_InGameDiceThrowCount_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_InGameDiceThrowCount_BitLen, inGameDiceThrowCount + 1);

        FRandomStream &RS = m_pResManager->getRandomStreamRef();
        coreDataDelta.m_iDiceVisualStateKey = RS.RandRange(INT32_MIN, INT32_MAX);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart)
    {
        const FMyMJGameActionDistCardAtStartCpp &actionDistCardAtStart = StaticCast<const FMyMJGameActionDistCardAtStartCpp &>(pusher);
        int32 idxAttender = actionDistCardAtStart.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataPrivate.Emplace();
        FMyMJRoleDataPrivateDeltaCpp& roleDataPrivDelta = delta.m_aRoleDataPrivate[idx];
        roleDataPrivDelta.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        roleDataPrivDelta.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_One(idxAttender);


        int32 l = actionDistCardAtStart.m_aIdValuePairs.Num();

        for (int32 i = 0; i < l; i++) {
            //1, reveal value
            int32 id = actionDistCardAtStart.m_aIdValuePairs[i].m_iId;
            int32 value = actionDistCardAtStart.m_aIdValuePairs[i].m_iValue;

            MY_VERIFY(value > 0);
            roleDataPrivDelta.m_aIdValuePairs2Reveal.Emplace(actionDistCardAtStart.m_aIdValuePairs[i]);
   
            //2, move card
            idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(id));
            FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];
            cardInfoTarget.m_cPosi.m_iIdxAttender = idxAttender;
            cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::InHand;
            cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Stand;

        }

        //3rd, update state
        if (actionDistCardAtStart.m_bLastCard) {

            coreDataDelta.m_eGameState = MyMJGameStateCpp::CardsDistributed;
            coreDataDelta.m_bUpdateGameState = true;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);

        }
        else {

        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards)
    {
        const FMyMJGameActionTakeCardsCpp &actionTakeCards = StaticCast<const FMyMJGameActionTakeCardsCpp &>(pusher);
        int32 idxAttender = actionTakeCards.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataPrivate.Emplace();
        FMyMJRoleDataPrivateDeltaCpp& roleDataPrivDelta = delta.m_aRoleDataPrivate[idx];
        roleDataPrivDelta.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        roleDataPrivDelta.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_One(idxAttender);


        //move card and flip
        int32 l = actionTakeCards.m_aIdValuePairs.Num();
        for (int32 i = 0; i < l; i++) {
  
            //1, reveal value
            int32 id = actionTakeCards.m_aIdValuePairs[i].m_iId;
            int32 value = actionTakeCards.m_aIdValuePairs[i].m_iValue;

            MY_VERIFY(value > 0);
            roleDataPrivDelta.m_aIdValuePairs2Reveal.Emplace(actionTakeCards.m_aIdValuePairs[i]);

            //2, move card
            idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(id));
            FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];
            cardInfoTarget.m_cPosi.m_iIdxAttender = idxAttender;
            cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::JustTaken;
            cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Stand;
        }

        if (actionTakeCards.m_bIsGang) {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedGangTakenCards;
        }
        else {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::TakenCard;
        }
        //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);
        coreDataDelta.m_bUpdateGameState = true;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards)
    {
        const FMyMJGameActionGiveOutCardsCpp &actionGiveOutCards = StaticCast<const FMyMJGameActionGiveOutCardsCpp &>(pusher);
        int32 idxAttender = actionGiveOutCards.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataPrivate.Emplace();
        FMyMJRoleDataPrivateDeltaCpp& roleDataPrivDelta = delta.m_aRoleDataPrivate[idx];
        roleDataPrivDelta.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        roleDataPrivDelta.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All;

        const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublicSelf = m_cDataAccessor.getRoleDataAttenderPublicRefConst(idxAttender);

        int32 selectedCount = actionGiveOutCards.m_aIdValuePairsSelected.Num();
        MY_VERIFY(selectedCount > 0);

        //1st reveal and move cards
        for (int32 i = 0; i < selectedCount; i++) {
            const FMyIdValuePair &pair = actionGiveOutCards.m_aIdValuePairsSelected[i];

            //1, reveal value
            int32 id = pair.m_iId;
            int32 value = pair.m_iValue;

            MY_VERIFY(value > 0);
            roleDataPrivDelta.m_aIdValuePairs2Reveal.Emplace(pair);

            //2, move card
            idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(id));
            FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];
            cardInfoTarget.m_cPosi.m_iIdxAttender = idxAttender;
            cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::GivenOut;
            cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Up;
        }

        //any card left in just taken slot, must go
        int32 l0 = roleDataAttenderPublicSelf.m_aIdJustTakenCards.Num();
        for (int32 i = 0; i < l0; i++) {
            int32 idCard = roleDataAttenderPublicSelf.m_aIdJustTakenCards[i];

            int32 idxTemp = FMyMJCardInfoPackCpp::helperFindCardByIdInCardInfos(coreDataDelta.m_aCardInfos2Update, idCard);
            if (idxTemp < 0 || coreDataDelta.m_aCardInfos2Update[idxTemp].m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::JustTaken) {
                //it is not moving out yet, make it move!
                idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(idCard));
                FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];

                cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::InHand;
                cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Stand;
            }
        }

        //2nd, update core state
        if (actionGiveOutCards.m_bIsGang) {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedGangGivenOutCards;
        }
        else {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::GivenOutCard;
        }

        //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);
        coreDataDelta.m_bUpdateGameState = true;

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave)
    {
        const FMyMJGameActionWeaveCpp &actionWeave = StaticCast<const FMyMJGameActionWeaveCpp &>(pusher);
        int32 idxAttender = actionWeave.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublicSelf = m_cDataAccessor.getRoleDataAttenderPublicRefConst(idxAttender);

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataAttender.Emplace();
        FMyMJRoleDataAttenderDeltaCpp &attenderDelta = delta.m_aRoleDataAttender[idx];
        attenderDelta.m_iIdxAttender = idxAttender;

        idx = attenderDelta.m_aDataPublic.Emplace();
        FMyMJRoleDataAttenderPublicDeltaCpp& attenderPublicDelta = attenderDelta.m_aDataPublic[idx];

        FMyMJRoleDataPrivateDeltaCpp* pRoleDataPrivDelta = NULL;

        if (actionWeave.m_eTargetFlipState == MyBoxFlipStateCpp::Up) {
            idx = delta.m_aRoleDataPrivate.Emplace();
            pRoleDataPrivDelta = &delta.m_aRoleDataPrivate[idx];
            pRoleDataPrivDelta->m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
            pRoleDataPrivDelta->m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All;
        }

        //1st, reveal and move cards
        const TArray<FMyIdValuePair> &aIdValuePairs = actionWeave.m_aCardValuesRelated;
        int32 l = aIdValuePairs.Num();

        MY_VERIFY(l > 0);
        FMyMJCardValuePackCpp::helperVerifyValuesInIdValuePairs(aIdValuePairs);

        for (int32 i = 0; i < l; i++) {
            const FMyIdValuePair& pair = aIdValuePairs[i];
            //pCardValuePack->revealCardValue(*pPair);

            //1st, reveal value
            if (actionWeave.m_eTargetFlipState == MyBoxFlipStateCpp::Up) {
                pRoleDataPrivDelta->m_aIdValuePairs2Reveal.Emplace(pair);
            }


            //2nd, move card and flop
            idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(pair.m_iId));
            FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];
            cardInfoTarget.m_cPosi.m_iIdxAttender = idxAttender;
            cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::Weaved;
            cardInfoTarget.m_eFlipState = actionWeave.m_eTargetFlipState;

        }

        //any card left in just taken slot, must go
        int32 l0 = roleDataAttenderPublicSelf.m_aIdJustTakenCards.Num();
        for (int32 i = 0; i < l0; i++) {
            int32 idCard = roleDataAttenderPublicSelf.m_aIdJustTakenCards[i];
            int32 idxTemp = FMyMJCardInfoPackCpp::helperFindCardByIdInCardInfos(coreDataDelta.m_aCardInfos2Update, idCard);
            if (idxTemp < 0 || coreDataDelta.m_aCardInfos2Update[idxTemp].m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::JustTaken) {
                //it is not moving out yet, make it move!
                idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(idCard));
                FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];

                cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::InHand;
                cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Up;
            }
        }

        //2nd, form a weave and add, and update helper minor posi info
        attenderPublicDelta.m_aWeave2Add.Emplace(actionWeave.m_cWeave);


        //3rd, update the state
        MyMJWeaveTypeCpp eType = actionWeave.m_cWeave.getType();

        if (eType == MyMJWeaveTypeCpp::GangAn || eType == MyMJWeaveTypeCpp::GangMing) {
            //int32 reserved0 = actionWeave.m_cWeave.getReserved0();
            //bool bIsBuZhang = (reserved0 & (uint8)EMyMJWeaveReserved0Mask::LocalCSGangBuZhang) > 0;
            bool bIsBuZhang = actionWeave.m_cWeave.getGangBuZhangLocalCS();

            if (!bIsBuZhang) {
                if (eType == MyMJWeaveTypeCpp::GangMing && actionWeave.m_cWeave.getTypeConsumed() == MyMJWeaveTypeCpp::ShunZiMing) {
                    coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedGang;
                }
                else {
                    //we can jump to next stage
                    coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedGangQiangGangChecked;
                }

            }
            else {
                coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedGangSpBuZhangLocalCS;
            }

        }
        else if (eType == MyMJWeaveTypeCpp::KeZiMing || eType == MyMJWeaveTypeCpp::ShunZiMing) {
            coreDataDelta.m_eGameState = MyMJGameStateCpp::WeavedNotGang;
        }
        else {
            MY_VERIFY(false);
        }

        //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);
        coreDataDelta.m_bUpdateGameState = true;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu)
    {
        const FMyMJGameActionHuCpp &actionHu = StaticCast<const FMyMJGameActionHuCpp &>(pusher);
        int32 idxAttender = actionHu.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublicSelf = m_cDataAccessor.getRoleDataAttenderPublicRefConst(idxAttender);

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataAttender.Emplace();
        FMyMJRoleDataAttenderDeltaCpp &attenderDelta = delta.m_aRoleDataAttender[idx];
        attenderDelta.m_iIdxAttender = idxAttender;

        idx = attenderDelta.m_aDataPublic.Emplace();
        FMyMJRoleDataAttenderPublicDeltaCpp& attenderPublicDelta = attenderDelta.m_aDataPublic[idx];

        idx = delta.m_aRoleDataPrivate.Emplace();
        FMyMJRoleDataPrivateDeltaCpp& roleDataPrivDelta = delta.m_aRoleDataPrivate[idx];
        roleDataPrivDelta.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        roleDataPrivDelta.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All;

        //TSharedPtr<FMyMJGameAttenderCpp> pAttender = getRealAttenderByIdx(idxAttender);

        //1st, reveal cards, move and flip
        int32 l = cardInfoPack.getLength();
        for (int32 i = 0; i < l; i++) {
            const FMyMJCardInfoCpp& cardInfo = cardInfoPack.getRefByIdxConst(i);
            if (cardInfo.m_eFlipState == MyBoxFlipStateCpp::Up) {
                continue;
            }

            idx = roleDataPrivDelta.m_aIdValuePairs2Reveal.Emplace();
            roleDataPrivDelta.m_aIdValuePairs2Reveal[idx].m_iId = cardInfo.m_iId;
            roleDataPrivDelta.m_aIdValuePairs2Reveal[idx].m_iValue = cardValuePack.getByIdx(cardInfo.m_iId);
        }

        //for this type of game, no more move, just flip
        for (int32 i = 0; i < 4; i++) {
            TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender2 = getAttenderByIdx(i);
            if (!pAttender2->getIsRealAttender()) {
                continue;
            }

            const FMyMJRoleDataAttenderPublicCpp& attenderPublicSelf = pAttender2->getRoleDataAttenderPublicRefConst();
            TArray<int32> aIds;
            aIds.Append(attenderPublicSelf.m_aIdGivenOutCards);
            aIds.Append(attenderPublicSelf.m_aIdHandCards);
            aIds.Append(attenderPublicSelf.m_aIdJustTakenCards);
            aIds.Append(attenderPublicSelf.m_aIdWinSymbolCards);
            
            for (int32 j = 0; j < attenderPublicSelf.m_aShowedOutWeaves.Num(); j++) {
                aIds.Append(attenderPublicSelf.m_aShowedOutWeaves[j].getIdsRefConst());
            }

            for (int32 j = 0; j < aIds.Num(); j++) {
                const FMyMJCardInfoCpp& cardInfoSelf = cardInfoPack.getRefByIdxConst(aIds[j]);
                if (cardInfoSelf.m_eFlipState == MyBoxFlipStateCpp::Up) {
                    continue;
                }

                int32 idxTemp = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoSelf);
                coreDataDelta.m_aCardInfos2Update[idxTemp].m_eFlipState = MyBoxFlipStateCpp::Up;
            }
        }

        //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave, true);
        coreDataDelta.m_bResetHelperLastCardsGivenOutOrWeave = true;

        if (actionHu.m_bEndGame) {

            const FMyMJRoleDataAttenderPublicCpp& attenderPublicSelf = m_cDataAccessor.getRoleDataAttenderPublicRefConst(idxAttender);
            const FMyMJRoleDataAttenderPublicCpp *pDPubD = &m_aAttendersAll[idxAttender]->getRoleDataAttenderPublicRefConst();

            idx = attenderPublicDelta.m_aHuScoreResultFinalGroup2Add.Emplace();
            attenderPublicDelta.m_aHuScoreResultFinalGroup2Add[idx] = actionHu.m_cHuScoreResultFinalGroup;

            coreDataDelta.m_eGameState = MyMJGameStateCpp::JustHu;
            //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState, true);
            coreDataDelta.m_bUpdateGameState = true;
        }
        else {
            MY_VERIFY(false); //CS MJ always end game when hu in game
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS)
    {
        const FMyMJGameActionHuBornLocalCSCpp &actionHuBornLocalCS = StaticCast<const FMyMJGameActionHuBornLocalCSCpp &>(pusher);
        int32 idxAttender = actionHuBornLocalCS.getIdxAttender(true);
        delta.m_iIdxAttenderActionInitiator = idxAttender;

        const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublicSelf = m_cDataAccessor.getRoleDataAttenderPublicRefConst(idxAttender);

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataAttender.Emplace();
        FMyMJRoleDataAttenderDeltaCpp &attenderDelta = delta.m_aRoleDataAttender[idx];
        attenderDelta.m_iIdxAttender = idxAttender;

        idx = attenderDelta.m_aDataPublic.Emplace();
        FMyMJRoleDataAttenderPublicDeltaCpp& attenderPublicDelta = attenderDelta.m_aDataPublic[idx];

        idx = delta.m_aRoleDataPrivate.Emplace();
        FMyMJRoleDataPrivateDeltaCpp& roleDataPrivDelta = delta.m_aRoleDataPrivate[idx];
        roleDataPrivDelta.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        roleDataPrivDelta.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All;

        int32 l = actionHuBornLocalCS.m_aShowOutIdValues.Num();

        for (int32 i = 0; i < l; i++) {
            //1, reveal value
            const FMyIdValuePair& pair = actionHuBornLocalCS.m_aShowOutIdValues[i];
            int32 id = pair.m_iId;
            int32 value = pair.m_iValue;
            MY_VERIFY(value > 0);
            roleDataPrivDelta.m_aIdValuePairs2Reveal.Emplace(pair);

            //2, move card and flip
            idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(id));
            FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];
            cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Up;
        }

        idx = attenderPublicDelta.m_aHuScoreResultFinalGroup2Add.Emplace();
        attenderPublicDelta.m_aHuScoreResultFinalGroup2Add[idx] = actionHuBornLocalCS.m_cHuScoreResultFinalGroup;

        //Todo: check following logic is added when generate:
        //scoreResultGroup.m_iIdxAttenderWin = idxAttender;

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS)
    {
        const FMyMJGameActionZhaNiaoLocalCSCpp &actionZhaNiaoLocalCS = StaticCast<const FMyMJGameActionZhaNiaoLocalCSCpp &>(pusher);
        //int32 idxAttender = actionZhaNiaoLocalCS.getIdxAttender(true);
        //delta.m_iIdxAttenderActionInitiator = idxAttender;

        int32 idx = delta.m_aCoreData.Emplace();
        FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[idx];

        idx = delta.m_aRoleDataPrivate.Emplace();
        FMyMJRoleDataPrivateDeltaCpp& roleDataPrivDelta = delta.m_aRoleDataPrivate[idx];
        roleDataPrivDelta.m_eRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        roleDataPrivDelta.m_iRoleMaskForDataPrivateClone = MyMJRoleDataPrivateDeltaCpp_RoleMaskForDataPrivateClone_All;

        //int32 idxAttenderBase = coreDataSelf.m_cGameRunData.m_iIdxAttenderMenFeng;
        //if (coreDataSelf.m_cGameCfg.getSubLocalCSCfgRefConst().m_bHuAttenderAsZhuangScore) {
            //idxAttenderBase = idxAttender;
        //}

        int32 l = actionZhaNiaoLocalCS.m_aPickedIdValues.Num();
        for (int32 i = 0; i < l; i++) {

            //1, reveal value
            const FMyIdValuePair& pair = actionZhaNiaoLocalCS.m_aPickedIdValues[i];
            int32 id = pair.m_iId;
            int32 value = pair.m_iValue;
            MY_VERIFY(value > 0);
            roleDataPrivDelta.m_aIdValuePairs2Reveal.Emplace(pair);

            //2, move card and flip

            idx = coreDataDelta.m_aCardInfos2Update.Emplace(cardInfoPack.getRefByIdxConst(id));
            FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[idx];
            //cardInfoTarget.m_cPosi.m_iIdxAttender = idxAttenderTarget;
            cardInfoTarget.m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::ShownOnDesktop;
            cardInfoTarget.m_eFlipState = MyBoxFlipStateCpp::Up;

        }

        //UMyMJUtilsLibrary::setBoolValueToBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave, true);
        coreDataDelta.m_bResetHelperLastCardsGivenOutOrWeave = true;
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pusher [%s] not valid for this core."),
            *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
    }

    return pRet;
};

void FMyMJGameCoreLocalCSCpp::applyPusher(const FMyMJGamePusherBaseCpp &pusher)
{
    const FMyMJGamePusherBaseCpp *pPusher = &pusher;

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

        const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();
        if (coreDataSelf.m_iActionGroupId != pusherMadeChoiceNotify.m_iActionGroupId) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("attender %d's pusher makde choice have unequal pusher id, core's: %d, pusher's: %d."),
                     idxAttender, coreDataSelf.m_iActionGroupId, pusherMadeChoiceNotify.m_iActionGroupId);
            MY_VERIFY(false);
        }
        

        m_aAttendersAll[idxAttender]->getActionContainorRef().showSelectionOnNotify(pusherMadeChoiceNotify.m_iSelection, pusherMadeChoiceNotify.m_aSubSelections);
    }

    else if (ePusherType == MyMJGamePusherTypeCpp::PusherCountUpdate) {
        const FMyMJGamePusherCountUpdateCpp& pusherCountUpdate = StaticCast<const FMyMJGamePusherCountUpdateCpp &>(pusher);

        if (pusherCountUpdate.m_bActionGroupIncrease) {
            resetForNewActionLoop();
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
            //resetForNewLoop(NULL, NULL, actionStateUpdate.m_iAttenderMaskNext, actionStateUpdate.m_bAllowSamePriAction, actionStateUpdate.m_iIdxAttenderHavePriMax);
            m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, NULL, actionStateUpdate.m_iAttenderMaskNext, actionStateUpdate.m_bAllowSamePriAction, actionStateUpdate.m_iIdxAttenderHavePriMax);

            if (actionStateUpdate.m_eStateNext == MyMJGameStateCpp::GameStarted) {
                FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
                pActionUpdate->m_eStateNext = MyMJGameStateCpp::CardsWaitingForThrowDicesToDistributeCards;
                pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(getCoreDataPublicRefConst().m_cGameRunData.m_iIdxAttenderMenFeng);

                m_pPusherIOFull->GivePusher(pActionUpdate, (void**)&pActionUpdate);
            }
            else if (actionStateUpdate.m_eStateNext == MyMJGameStateCpp::JustStarted) {
                const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();
                int32 idxZhuang = coreDataSelf.m_cGameRunData.m_iIdxAttenderMenFeng;
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
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionNoAct) {


    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {
        const FMyMJGameActionThrowDicesCpp& actionThrowDices = StaticCast<const FMyMJGameActionThrowDicesCpp &>(pusher);

        int32 idxAttender = actionThrowDices.getIdxAttender();

        int32 uMask = genIdxAttenderStillInGameMaskOne(idxAttender);
        m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, NULL, uMask, false, idxAttender);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
        const FMyMJGameActionDistCardAtStartCpp& actionDistCardAtStart = StaticCast<const FMyMJGameActionDistCardAtStartCpp &>(pusher);
        int32 idxAttender = actionDistCardAtStart.getIdxAttender();

        const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();
        //const FMyMJCardInfoPackCpp&  cardInfoPack = getCardInfoPackRefConst();
        //const FMyMJCardValuePackCpp& cardValuePack = getCardValuePackOfSysKeeperRefConst();

        if (actionDistCardAtStart.m_bLastCard) {

            getRealAttenderByIdx(idxAttender)->onNewTurn(false);

            MY_VERIFY(coreDataSelf.m_eGameState == MyMJGameStateCpp::CardsDistributed);

            int32 iMask = genIdxAttenderStillInGameMaskAll();
            int32 idxZhuang = coreDataSelf.m_cGameRunData.m_iIdxAttenderMenFeng;

            FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
            pActionUpdate->m_eStateNext = MyMJGameStateCpp::JustStarted;
            pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxZhuang);
            m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, pActionUpdate, iMask, true, idxZhuang);

        }
        else {
            int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);
            int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
            m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, NULL, iMask, false, idxAttenderNext);
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
        const FMyMJGameActionTakeCardsCpp& actionTakeCards = StaticCast<const FMyMJGameActionTakeCardsCpp &>(pusher);
        int32 idxAttender = actionTakeCards.getIdxAttender();

        getRealAttenderByIdx(idxAttender)->onNewTurn(false);

        int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);
        m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, NULL, iMask, false, idxAttender);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {

        //Note: update ting code is move into preApplyPusherResult for optimization
        const FMyMJGameActionGiveOutCardsCpp& actionGiveOutCards = StaticCast<const FMyMJGameActionGiveOutCardsCpp &>(pusher);
        int32 idxAttender = actionGiveOutCards.getIdxAttender();

        const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();

        int32 selectedCount = actionGiveOutCards.m_aIdValuePairsSelected.Num();
        MY_VERIFY(selectedCount > 0);

        //update core state
        int32 iMask = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
        int32 idxAttenderNext = findIdxAttenderStillInGame(idxAttender, 1, false);

        FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
        pActionUpdate->m_eStateNext = MyMJGameStateCpp::WaitingForTakeCard;
        pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxAttenderNext);
        m_cActionLoopHelperData.setupDataForNextActionLoop(pActionUpdate, NULL, iMask, coreDataSelf.m_cGameCfg.getSubLocalCSCfgRefConst().m_bHuAllowMultiple, idxAttenderNext);

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
        const FMyMJGameActionWeaveCpp& actionWeave = StaticCast<const FMyMJGameActionWeaveCpp&>(pusher);
        int32 idxAttender = actionWeave.getIdxAttender();

        const FMyMJCoreDataPublicCpp& coreDataSelf = getCoreDataPublicRefConst();
        //const FMyMJCardInfoPackCpp&  cardInfoPack = getCardInfoPackRefConst();
        //const FMyMJCardValuePackCpp& cardValuePack = getCardValuePackOfSysKeeperRefConst();


        //1st, attender apply
        TSharedPtr<FMyMJGameAttenderLocalCSCpp> pAttender = getRealAttenderByIdx(idxAttender);
        
        pAttender->onNewTurn(true);

        //2nd update state
        MyMJWeaveTypeCpp eType = actionWeave.m_cWeave.getType();

        int32 iAttenderMaskNow = 0;
        bool bAllowSamePriActionNow = false;
        int32 iIdxAttenderHavePriMaxNow = 0;
        FMyMJGameActionStateUpdateCpp *pActionUpdatePrev = NULL;
        if (eType == MyMJWeaveTypeCpp::GangAn || eType == MyMJWeaveTypeCpp::GangMing) {
            
            bool bTIngNow = pAttender->tryGenAndEnqueueUpdateTingPusher();

            //int32 reserved0 = actionWeave.m_cWeave.getReserved0();
            //bool bIsBuZhang = (reserved0 & (uint8)EMyMJWeaveReserved0Mask::LocalCSGangBuZhang) > 0;
            bool bIsBuZhang = actionWeave.m_cWeave.getGangBuZhangLocalCS();

            if (!bIsBuZhang) {
                MY_VERIFY(bTIngNow);
                if (eType == MyMJWeaveTypeCpp::GangMing && actionWeave.m_cWeave.getTypeConsumed() == MyMJWeaveTypeCpp::ShunZiMing) {
                    //can be qiang, give others chance to check
                    //MyMJGameStateCpp::WeavedGang;
                    iAttenderMaskNow = genIdxAttenderStillInGameMaskExceptOne(idxAttender);
                    bAllowSamePriActionNow = coreDataSelf.m_cGameCfg.getSubLocalCSCfgRefConst().m_bHuAllowMultiple;
                    iIdxAttenderHavePriMaxNow = idxAttender;

                    pActionUpdatePrev = new FMyMJGameActionStateUpdateCpp();
                    pActionUpdatePrev->m_eStateNext = MyMJGameStateCpp::WeavedGangQiangGangChecked;
                    pActionUpdatePrev->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(idxAttender);
                    pActionUpdatePrev->m_bAllowSamePriAction = false;
                    pActionUpdatePrev->m_iIdxAttenderHavePriMax = idxAttender;
                }
                else {
                    //we can jump to next stage
                    //MyMJGameStateCpp::WeavedGangQiangGangChecked;
                    iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
                    bAllowSamePriActionNow = false;
                    iIdxAttenderHavePriMaxNow = idxAttender;
                    pActionUpdatePrev = NULL;
                }

            }
            else {
                //MyMJGameStateCpp::WeavedGangSpBuZhangLocalCS;
                iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
                bAllowSamePriActionNow = false;
                iIdxAttenderHavePriMaxNow = idxAttender;
                pActionUpdatePrev = NULL;
            }

        }
        else if (eType == MyMJWeaveTypeCpp::KeZiMing || eType == MyMJWeaveTypeCpp::ShunZiMing) {
            //MyMJGameStateCpp::WeavedNotGang;
            iAttenderMaskNow = genIdxAttenderStillInGameMaskOne(idxAttender);
            bAllowSamePriActionNow = false;
            iIdxAttenderHavePriMaxNow = idxAttender;
            pActionUpdatePrev = NULL;
        }
        else {
            MY_VERIFY(false);
        }

        m_cActionLoopHelperData.setupDataForNextActionLoop(pActionUpdatePrev, NULL, iAttenderMaskNow, bAllowSamePriActionNow, iIdxAttenderHavePriMaxNow);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {
        const FMyMJGameActionHuCpp& actionHu = StaticCast<const FMyMJGameActionHuCpp &>(pusher);
        int32 idxAttender = actionHu.getIdxAttender();


        if (actionHu.m_bEndGame) {

            //pD->m_eGameState = MyMJGameStateCpp::JustHu;
            int32 iMask = genIdxAttenderStillInGameMaskOne(idxAttender);

            FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
            pActionUpdate->m_eStateNext = MyMJGameStateCpp::GameEnd;
            pActionUpdate->m_iAttenderMaskNext = 0;
            pActionUpdate->m_eReason = MyMJGameStateUpdateReasonCpp::AttenderHu;

            m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, pActionUpdate, iMask, false, idxAttender);

        }
        else {
            MY_VERIFY(false); //CS MJ always end game when hu in game
        }

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {

    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pusher [%s] not valid for this core."),
            *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
    }


}

void FMyMJGameCoreLocalCSCpp::handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp &cmd)
{
    FMyMJGameCmdBaseCpp *pCmd = &cmd;
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    pCmd->m_cRespErrorCode.reset(true);

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
                pPusherReset->init(iGameId, RS, pCmdRestartGame->m_cGameCfg, pCmdRestartGame->m_cGameRunData);

                m_pPusherIOFull->GivePusher(pPusherReset, (void**)&pPusherReset);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("requested reset game but type not equal: cfg %d, self %d."), (uint8)pCmdRestartGame->m_cGameCfg.m_eRuleType, (uint8)getRuleType());
                pCmd->m_cRespErrorCode = FMyErrorCodeMJGameCpp(MyErrorCodeSubPartMJGameCpp::GameRuleTypeNotEqual);
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("requested reset game from invalid role: %d"), (uint8)eRoleTypeOfCmdSrc);
            pCmd->m_cRespErrorCode = FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::HaveNoAuthority);
        }

    }
    else if (pCmd->m_eType == MyMJGameCmdType::MakeSelection) {
        FMyMJGameCmdMakeSelectionCpp *pCmdMakeSelection = StaticCast<FMyMJGameCmdMakeSelectionCpp *>(pCmd);

        int32 idxAttender = pCmdMakeSelection->m_iIdxAttender;
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
        MY_VERIFY(idxAttender == (int32)eRoleTypeOfCmdSrc);

        const FMyMJCoreDataPublicCpp& coreDataPublic = getCoreDataPublicRefConst();
        if (coreDataPublic.m_iGameId != pCmdMakeSelection->m_iGameId || coreDataPublic.m_iActionGroupId != pCmdMakeSelection->m_iActionGroupId) {
            pCmdMakeSelection->m_cRespErrorCode.m_eCommonPart = MyErrorCodeCommonPartCpp::InputTimePassed;
            return;
        }

        FMyMJGameActionContainorCpp& actionContainor = getAttenderByIdx(idxAttender)->getActionContainorRef();
        pCmdMakeSelection->m_cRespErrorCode = actionContainor.makeSelection(pCmdMakeSelection->m_iSelection, pCmdMakeSelection->m_aSubSelections);
    }
    else {
        MY_VERIFY(false);
    }
}


void FMyMJGameCoreLocalCSCpp::genBaseFromPusherResetGame(FMyMJGameResManager& RM, const FMyMJGamePusherResetGameCpp &pusherReset, const FMyMJDataStructCpp &BaseBefore, FMyMJDataStructCpp &outBase)
{
    FRandomStream &RS = RM.getRandomStreamRef();

    FMyMJDataStructCpp &base = outBase;

    //Stateless, never judge condition of previous state, just reset the whole core
    FMyMJCoreDataPublicCpp *pD = const_cast<FMyMJCoreDataPublicCpp *> (&base.getCoreDataPublicRefConst());
    pD->reset();

    //pD->reinit(pPusher->m_cGameCfg.m_eRuleType);

    FMyMJCardInfoPackCpp  *pCardInfoPack = &pD->m_cCardInfoPack;

    //let's construct things
    MY_VERIFY(pusherReset.m_cGameCfg.m_aAttenderCfgs.Num() == 4);
    int32 realAttenderCount = 0;

    for (int32 i = 0; i < 4; i++) {
        FMyMJRoleDataAttenderPublicCpp &cRoleDataAttenderPub = const_cast<FMyMJRoleDataAttenderPublicCpp &>(base.getRoleDataAttenderPublicRefConst(i));

        //UMyMJUtilsLibrary::setBoolValueToBitMask(cRoleDataAttenderPub.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender, true);
        //UMyMJUtilsLibrary::setBoolValueToBitMask(cRoleDataAttenderPub.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame, true);
        bool bIsRealAttender = pusherReset.m_cGameCfg.m_aAttenderCfgs[i].m_bIsRealAttender;
        cRoleDataAttenderPub.m_bIsRealAttender = bIsRealAttender;
        cRoleDataAttenderPub.m_bIsStillInGame = bIsRealAttender;

        if (bIsRealAttender) {
            realAttenderCount++;
        }

        FMyMJRoleDataAttenderPrivateCpp&cRoleDataAttenderPriv = const_cast<FMyMJRoleDataAttenderPrivateCpp &>(base.getRoleDataAttenderPrivateRefConst(i));
        cRoleDataAttenderPriv.reset();
    }
    
    MY_VERIFY(realAttenderCount >= 2 && realAttenderCount <= 4);


    pD->m_iGameId = pusherReset.m_iGameId;
    pD->m_iPusherIdLast = 0;
    pD->m_iActionGroupId = 0;


    pD->m_eGameState = MyMJGameStateCpp::GameReseted;

    pD->m_iDiceNumberNowMask = 0;

    int32 m_iDiceNumberNowMaskBefore = BaseBefore.getCoreDataPublicRefConst().m_iDiceNumberNowMask;
    int32 diceValue0 = UMyMJUtilsLibrary::getIntValueFromBitMask(m_iDiceNumberNowMaskBefore, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen);
    int32 diceValue1 = UMyMJUtilsLibrary::getIntValueFromBitMask(m_iDiceNumberNowMaskBefore, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen);

    if (diceValue0 < 1 || diceValue0 > 6 || diceValue1 < 1 || diceValue1 > 6) {
        //regenerate one
        FRandomStream RSTemp;
        RSTemp.Initialize(RM.getSeed());

        diceValue0 = RSTemp.RandRange(1, 6);
        diceValue1 = RSTemp.RandRange(1, 6);
    }

    UMyMJUtilsLibrary::setIntValueToBitMask(pD->m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen, diceValue0);
    UMyMJUtilsLibrary::setIntValueToBitMask(pD->m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen, diceValue1);
    
    UMyMJUtilsLibrary::setIntValueToBitMask(pD->m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_Invalid);
    UMyMJUtilsLibrary::setIntValueToBitMask(pD->m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitLen, 6);
    UMyMJUtilsLibrary::setIntValueToBitMask(pD->m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_InGameDiceThrowCount_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_InGameDiceThrowCount_BitLen, 0);
   
    pD->m_iDiceVisualStateKey = BaseBefore.getCoreDataPublicRefConst().m_iDiceVisualStateKey;


    pD->m_cUntakenSlotInfo.reset();

    //let's fill in
    pD->m_cGameCfg = pusherReset.m_cGameCfg;
    pD->m_cGameRunData = pusherReset.m_cGameRunData;
    pD->m_iSeed = RM.getSeed();

    int32 iCardNum = pusherReset.m_aShuffledValues.Num();
    pCardInfoPack->reset(iCardNum);

    FMyMJCardValuePackCpp *pCardValuePack = const_cast<FMyMJCardValuePackCpp *> (&base.getRoleDataPrivateRefConst().m_cCardValuePack);
    pCardValuePack->resetAndRevealAll(pusherReset.m_aShuffledValues);
    pCardValuePack->helperVerifyValues();


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

    //let's recalc and arrange them to attender's
    MY_VERIFY(stackCount == pD->m_aUntakenCardStacks.Num());
    //stackCount = pD->m_aUntakenCardStacks.Num();

    //arrange the dangling stack
    int32 stackNotEven = stackCount % 4;
    MY_VERIFY(stackNotEven < 4);
    int32 tempIdxs[4], tempResults[4];
    for (int32 i = 0; i < 4; i++) {
        tempIdxs[i] = i;
        tempResults[i] = 0;
    }
    for (int32 i = 0; i < stackNotEven; i++) {
        int32 picked = RS.RandRange(0, 3 - i);
        int32 idxPicked = tempIdxs[picked];

        tempResults[idxPicked] = 1;

        //refill for next pick
        tempIdxs[picked] = tempIdxs[3 - i];
    }
    //verify
    int32 tempCount = 0;
    for (int32 i = 0; i < 4; i++) {
        MY_VERIFY(tempResults[i] <= 1);
        tempCount += tempResults[i];
    }
    MY_VERIFY(tempCount == stackNotEven);
    //final result
    tempCount = 0;
    int32 stackLengths[4];
    for (int32 i = 0; i < 4; i++) {
        int32 l0 = stackCount / 4 + tempResults[i];
        tempCount += l0;
        stackLengths[i] = l0;
    }
    MY_VERIFY(tempCount == stackCount);

    int32 idxStart = 0;
    for (int32 i = 0; i < 4; i++) {

        int32 stackL = stackLengths[i];

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
                pCardInfo->m_eFlipState = MyBoxFlipStateCpp::Down;
                pCardInfo->m_cPosi.m_eSlot = MyMJCardSlotTypeCpp::Untaken;
                pCardInfo->m_cPosi.m_iIdxAttender = i;
                pCardInfo->m_cPosi.m_iIdxInSlot0 = idxUntaken;
                pCardInfo->m_cPosi.m_iIdxInSlot1 = idxInStack;

            }
        }

        idxStart += stackL;
    }


    //update untaken slot info
    pD->m_cUntakenSlotInfo.initWhenCardsFilledInUntakenSlot(stackCount, cardCount, pD->m_cGameCfg.m_cTrivialCfg.m_iStackNumKeptFromTail);

    pCardInfoPack->helperVerifyInfos();

    base.setRole(MyMJGameRoleTypeCpp::SysKeeper);

    FMyMJDataAccessorCpp cA;
    cA.setupDataExt(&base, true);
    cA.resetForNewActionLoop();
}


void FMyMJGameCoreLocalCSCpp::applyPusherResetGame(const FMyMJGamePusherResetGameCpp &pusher)
{
    resetDatasOwned();
    for (int i = 0; i < 4; i++) {
        TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i];
        pAttender->resetDatasOwned();
    }

    MY_VERIFY(pusher.m_cGameCfg.m_aAttenderCfgs.Num() == 4);

    //let's construct things
    //int32 iRealAttenderNum = pusher.m_cGameCfg.m_cTrivialCfg.m_iGameAttenderNum;

    TArray<FMyMJGameActionContainorCpp *> aContainors;
    for (int32 i = 0; i < 4; i++) {
        const FMyMJGameAttenderCfgCpp& attenderCfg = pusher.m_cGameCfg.m_aAttenderCfgs[i];
        TSharedPtr<FMyMJGameAttenderCpp> pAttender = m_aAttendersAll[i];
        pAttender->getDataLogicRef().reinit(attenderCfg.m_eAIStrategyType, attenderCfg.m_iIdleTimeToAIControl_ms);
        if (pusher.m_cGameCfg.m_aAttenderCfgs[i].m_bIsRealAttender) {
            aContainors.Emplace(&pAttender->getActionContainorRef());
        }
    }

   

    MY_VERIFY(aContainors.Num() >= 2 && aContainors.Num() <= 4);


    MY_VERIFY(getRuleType() == pusher.m_cGameCfg.m_eRuleType)

    m_pActionCollector->setWorkingContainors(aContainors);

    m_cDataLogic.m_eActionLoopState = MyMJActionLoopStateCpp::WaitingToGenAction;
    m_cDataLogic.m_iMsLast = UMyCommonUtilsLibrary::nowAsMsFromTick();

    //int32 uMask = genIdxAttenderStillInGameMaskOne(pusher.m_cGameRunData.m_iIdxAttenderMenFeng);
    //m_cActionLoopHelperData.setupDataForNextActionLoop(NULL, NULL, uMask, false, pusher.m_cGameRunData.m_iIdxAttenderMenFeng);
    //resetForNewActionLoop();

    //Warn: this assume the core drains out all pusher before gen next action
    FMyMJGameActionStateUpdateCpp *pActionUpdate = new FMyMJGameActionStateUpdateCpp();
    pActionUpdate->m_eStateNext = MyMJGameStateCpp::GameStarted;
    //pActionUpdate->m_iAttenderMaskNext = genIdxAttenderStillInGameMaskOne(pusher.m_cGameRunData.m_iIdxAttenderMenFeng);

    m_pPusherIOFull->GivePusher(pActionUpdate, (void**)&pActionUpdate);

    //FMyMJGamePusherCountUpdateCpp *pPusherCountUpdate = new FMyMJGamePusherCountUpdateCpp();
    //m_pPusherIOFull->GivePusher(pPusherCountUpdate, (void**)&pPusherCountUpdate);
}