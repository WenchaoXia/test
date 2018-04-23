// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameData.h"

#include "UnrealNetwork.h"
#include "Engine/ActorChannel.h"

void FMyMJDataDeltaCpp::copyWithRoleFromSysKeeperRole(MyMJGameRoleTypeCpp eTargetRole, FMyMJDataDeltaCpp& cTargetDelta) const
{
    MY_VERIFY(eTargetRole != MyMJGameRoleTypeCpp::SysKeeper)

    cTargetDelta = *this;

    //*(StaticCast<FMyMJGamePusherBaseCpp *>(&cTargetDelta)) = *this;

    //cTargetDelta.m_aCoreData = m_aCoreData;
    //cTargetDelta.m_aRoleDataAttender = m_aRoleDataAttender;
    //cTargetDelta.m_aRoleDataPrivate = m_aRoleDataPrivate;
    //cTargetDelta.m_iIdxAttenderActionInitiator = m_iIdxAttenderActionInitiator;
    //cTargetDelta.m_iGameId = m_iGameId;

    int32 l = cTargetDelta.m_aRoleDataAttender.Num();
    MY_VERIFY(l <= 1);
    for (int32 i = 0; i < l; i++) {
        if ((uint8)eTargetRole != i) {
            cTargetDelta.m_aRoleDataAttender[i].m_aDataPrivate.Reset();
        }
    }

    l = cTargetDelta.m_aRoleDataPrivate.Num();
    MY_VERIFY(l <= 1);
    if (l > 0) {
        bool bShouldKeepInfo = (cTargetDelta.m_aRoleDataPrivate[0].m_iRoleMaskForDataPrivateClone & (1 << (uint8)eTargetRole)) > 0;
        if (bShouldKeepInfo) {
            cTargetDelta.m_aRoleDataPrivate[0].m_eRoleType = eTargetRole;
        }
        else {
            cTargetDelta.m_aRoleDataPrivate.Reset();
        }
    }
};

FMyMJDataStructCpp::FMyMJDataStructCpp()
{
    m_eRole = MyMJGameRoleTypeCpp::Max;


    for (int i = 0; i < RoleDataAttenderNum; i++) {
        int32 idx = m_aRoleDataAttenders.Emplace();
        FMyMJRoleDataAttenderCpp *pD = &m_aRoleDataAttenders[idx];
        pD->resetup(i);
    }


    /*
    for (int i = 0; i < RoleDataAttenderNum; i++) {
        int32 idx = m_aTestArray.Emplace();
        m_aTestArray[idx] = idx;

        m_aTestPrivArray.Emplace();
        
        idx = m_aRoleDataAttenders.Emplace();
        FMyMJRoleDataAttenderCpp *pD = &m_aRoleDataAttenders[idx];
        pD->resetup(i);
    }
    */

};

FMyMJDataStructCpp::~FMyMJDataStructCpp()
{

};

int32 FMyMJDataStructCpp::checkPrivateDataInExpect() const
{
    const FMyMJCoreDataPublicCpp& cdp = getCoreDataPublicRefConst();
    if (cdp.m_iPusherIdLast < 0) {
        return 0;
    }

    if ((uint8)m_eRole >= (uint8)MyMJGameRoleTypeCpp::Max || (uint8)m_eRole < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d is not valid."), (uint8)m_eRole);
        return -1;
    }

    const FMyMJCardInfoPackCpp&  cCardInfoPack  = cdp.m_cCardInfoPack;
    const FMyMJCardValuePackCpp& cCardValuePack = getRoleDataPrivateRefConst().m_cCardValuePack;

    int32 cardCount = cdp.m_cCardInfoPack.getLength();
    bool bGameEnd = cdp.m_eGameState == MyMJGameStateCpp::GameEnd;

    if ((uint8)m_eRole < 4 || m_eRole == MyMJGameRoleTypeCpp::Observer) {
        for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
            const FMyMJRoleDataAttenderPrivateCpp& rdap = getRoleDataAttenderPrivateRefConst(idxAttender);
            if (idxAttender != (uint8)m_eRole) {
                if (!rdap.isEmpty()) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d: %d's private data not empty!"), (uint8)m_eRole, idxAttender);
                    return -10;
                }
            }
        }
    }

    for (int32 i = 0; i < cardCount; i++) {
        const FMyMJCardInfoCpp& cInfo = cCardInfoPack.getRefByIdxConst(i);
        int32 value = cCardValuePack.getByIdx(i);
        if (bGameEnd || m_eRole == MyMJGameRoleTypeCpp::SysKeeper) {
            if (value == 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d: game ended, unexpected card %s, value %d."), (uint8)m_eRole, *cInfo.genDebugMsg(), value);
                return -100;
            }
        }
        else if (cInfo.m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::Untaken) {
            if (value != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d: unexpected card %s, value %d."), (uint8)m_eRole, *cInfo.genDebugMsg(), value);
                return -110;
            }
        }
        else {
            if (cInfo.m_cPosi.m_iIdxAttender == (uint8)m_eRole || cInfo.m_eFlipState == MyBoxFlipStateCpp::Up) {
                //supposed to know
                if (value == 0) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d: unexpected card %s, value %d."), (uint8)m_eRole, *cInfo.genDebugMsg(), value);
                    return -120;
                }
            }
            else {
                if (value > 0) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d: unexpected card %s, value %d."), (uint8)m_eRole, *cInfo.genDebugMsg(), value);
                    return -130;
                }
            }
        }
    }


    return 0;
};

void FMyMJDataAccessorCpp::applyBase(const FMyMJDataStructCpp &base, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
{
    MY_VERIFY(m_pDataExt);

    *m_pDataExt = base;
    /*
    getCoreDataRef() = base.getCoreDataPublicRefConst();

    int32 l = (uint8)MyMJGameRoleTypeCpp::Max;
    for (int i = 0; i < l; i++) {

    if (i < 4)
    {
    getRoleDataAttenderPublicRef(i) = base.getRoleDataAttenderPublicRefConst(i);


    FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivSelf = getRoleDataAttenderPrivate(i);
    const FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivOther = &base.getRoleDataAttenderPrivateRefConst(i);
    if (pAttenderPrivSelf && pAttenderPrivOther) {
    *pAttenderPrivSelf = *pAttenderPrivOther;
    }
    else {
    //it is OK, but in a fine design this should be avoid
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's attender private data is NULL, self %p, dest %p."), i, pAttenderPrivSelf, pAttenderPrivOther);
    }

    }

    {
    FMyMJRoleDataPrivateCpp *pPrivSelf = getRoleDataPrivate(i);
    const FMyMJRoleDataPrivateCpp *pPrivOther = &base.getRoleDataPrivateRefConst(i);
    if (pPrivSelf && pPrivOther) {
    *pPrivSelf = *pPrivOther;
    }
    else {
    //it is OK, but in a fine design this should be avoid
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's private data is NULL, self %p, dest %p."), i, pPrivSelf, pPrivOther);
    }
    }
    }
    */

    getCoreDataRef().m_cGameCfg.prepareForUse(); //Todo: this can be done as Lazy work, since it is only needed when you want to run the core logic

    if (pDirtyRecord) {
        FMyMJDataAccessorCpp::helperSetCoreDataDirtyRecordAllDirty(*pDirtyRecord);
    }
}

void FMyMJDataAccessorCpp::applyDeltaStep0(const FMyMJDataDeltaCpp &delta, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
{
    FMyMJCoreDataPublicCpp &coreDataSelf = getCoreDataRef();
    if (!(coreDataSelf.m_iGameId == delta.m_iGameId && (coreDataSelf.m_iPusherIdLast + 1) == delta.getId())) {
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s failed to apply delta since id mismatch: self [%d, %d], delta [%d, %d]."), *genDebugStateString(), coreDataSelf.m_iGameId, coreDataSelf.m_iPusherIdLast, delta.m_iGameId, delta.getId());
        return;
    }

    while (delta.m_aRoleDataPrivate.Num() > 0) {
        MY_VERIFY(delta.m_aRoleDataPrivate.Num() == 1);
        const FMyMJRoleDataPrivateDeltaCpp& roleDataPriDelta = delta.m_aRoleDataPrivate[0];

        MyMJGameRoleTypeCpp eRoleType = roleDataPriDelta.m_eRoleType;
        if (getAccessRoleType() != eRoleType) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("tring to apply role private delta but access role is different, %d, %d."), (uint8)getAccessRoleType(), (uint8)eRoleType);
            MY_VERIFY(false);
            break;
        }

        FMyMJRoleDataPrivateCpp *pRDPriSelf = &getRoleDataPrivateRef();

        if (roleDataPriDelta.m_aIdValuePairs2Reveal.Num() > 0 && getAccessRoleType() != MyMJGameRoleTypeCpp::SysKeeper) {
            //syskeeper doesn't need to update any in progress, since when reset he knows all
            getCardValuePackRef().tryRevealCardValueByIdValuePairs(roleDataPriDelta.m_aIdValuePairs2Reveal);

        }

        break;
    }
}

void FMyMJDataAccessorCpp::applyDeltaStep1(const FMyMJDataDeltaCpp &delta, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
{
    FMyMJCoreDataPublicCpp &coreDataSelf = getCoreDataRef();

    MyMJGamePusherTypeCpp ePusherType = delta.getType();

    FMyMJCardInfoCpp *pCardInfo = NULL;

    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackRef();
    FMyMJCardInfoPackCpp  *pCardInfoPack = &coreDataSelf.m_cCardInfoPack;

    if (!(coreDataSelf.m_iGameId == delta.m_iGameId && (coreDataSelf.m_iPusherIdLast + 1) == delta.getId())) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s failed to apply delta since id mismatch: self [%d, %d], delta [%d, %d]."), *genDebugStateString(), coreDataSelf.m_iGameId, coreDataSelf.m_iPusherIdLast, delta.m_iGameId, delta.getId());
        return;
    }

    if (delta.m_aCoreData.Num() > 0) {
        
        MY_VERIFY(delta.m_aCoreData.Num() == 1);
        const FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[0];

        TArray<int32> aIdHelperMovedFromUntakenSlot;
        TArray<int32> aIdHelperMovedToGivenOutSlot;

        //update card info, move and flip
        int32 l = coreDataDelta.m_aCardInfos2Update.Num();
        for (int32 i = 0; i < l; i++) {
            const FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[i];
            FMyMJCardInfoCpp& cardInfoSelf = pCardInfoPack->getRefByIdx(cardInfoTarget.m_iId);

            MY_VERIFY(cardInfoSelf.m_iId == cardInfoTarget.m_iId);

            //move
            if (cardInfoSelf.m_cPosi != cardInfoTarget.m_cPosi) {

                if (cardInfoSelf.m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::Untaken && cardInfoTarget.m_cPosi.m_eSlot != cardInfoSelf.m_cPosi.m_eSlot) {
                    aIdHelperMovedFromUntakenSlot.Emplace(cardInfoSelf.m_iId);
                }

                if (cardInfoTarget.m_cPosi.m_eSlot == MyMJCardSlotTypeCpp::GivenOut && cardInfoTarget.m_cPosi.m_eSlot != cardInfoSelf.m_cPosi.m_eSlot) {
                    aIdHelperMovedToGivenOutSlot.Emplace(cardInfoSelf.m_iId);
                }

                //for safety, any different exist, we make movement happen
                moveCardFromOldPosi(cardInfoTarget.m_iId, pDirtyRecord);
                moveCardToNewPosi(cardInfoTarget.m_iId, cardInfoTarget.m_cPosi.m_iIdxAttender, cardInfoTarget.m_cPosi.m_eSlot, pDirtyRecord);

            }

            //flip
            if (cardInfoTarget.m_eFlipState != MyBoxFlipStateCpp::Invalid) {
                cardInfoSelf.m_eFlipState = cardInfoTarget.m_eFlipState;

                if (pDirtyRecord) {
                    pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::Card, cardInfoSelf.m_cPosi.m_iIdxAttender, (int32)cardInfoSelf.m_cPosi.m_eSlot, true);
                }
            }

        }


        //update helper
        if (coreDataSelf.m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead <= 0 && aIdHelperMovedFromUntakenSlot.Num() > 0) {
            coreDataSelf.m_cHelper.m_aIdHelperLastCardsTakenInWholeGame = aIdHelperMovedFromUntakenSlot;
            coreDataSelf.m_cHelper.m_eHelperGameStateJustBeforeLastCardsTakenInWholeGame = coreDataSelf.m_eGameState;

            if (m_bShowApplyInfo) {
                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Debug: setting last cards with game state %s."),
                    *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameStateCpp"), (uint8)coreDataSelf.m_eGameState));
            }
        }

        if (aIdHelperMovedToGivenOutSlot.Num() > 0) {
            coreDataSelf.m_cHelper.m_aIdHelperLastCardsGivenOut.Reset();
            coreDataSelf.m_cHelper.m_aHelperLastWeaves.Reset();

            coreDataSelf.m_cHelper.m_aIdHelperLastCardsGivenOut = aIdHelperMovedToGivenOutSlot;
        }



        int32 diceNumerNowMaskUpdateReason = UMyMJUtilsLibrary::getIntValueFromBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen);
        if (diceNumerNowMaskUpdateReason != FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_Invalid) {

            int32 diceNumerValue0 = UMyMJUtilsLibrary::getIntValueFromBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen);
            int32 diceNumerValue1 = UMyMJUtilsLibrary::getIntValueFromBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen);
            int32 idxAttender = delta.getIdxAttenderActionInitiator();

            MY_VERIFY(idxAttender == UMyMJUtilsLibrary::getIntValueFromBitMask(coreDataDelta.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitLen));

            if (diceNumerNowMaskUpdateReason == FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GameStart) {

                //just updat the untaken info
                FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublicSelf = getRoleDataAttenderPublicRef(idxAttender);

                int32 iBase = roleDataAttenderPublicSelf.m_cUntakenSlotSubSegmentInfo.m_iIdxStart;
                int32 len = coreDataSelf.m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal;
                MY_VERIFY(len > 0);

                int32 iIdxUntakenSlotHeadNow = (iBase + diceNumerValue0 + diceNumerValue1 - 1 + len) % len;
                coreDataSelf.m_cUntakenSlotInfo.setupWhenDicesThrownForCardDistAtStart(coreDataSelf.m_aUntakenCardStacks, iIdxUntakenSlotHeadNow);

            }
            else if (diceNumerNowMaskUpdateReason == FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GangYaoLocalCS) {

            }
            else {
                MY_VERIFY(false);
            }

            /*
            UMyMJUtilsLibrary::setIntValueToBitMask(coreDataSelf.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen, diceNumerValue0);
            UMyMJUtilsLibrary::setIntValueToBitMask(coreDataSelf.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen, diceNumerValue1);

            UMyMJUtilsLibrary::setIntValueToBitMask(coreDataSelf.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_LastThrowAttender_BitLen, idxAttender);
            UMyMJUtilsLibrary::setIntValueToBitMask(coreDataSelf.m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen, diceNumerNowMaskUpdateReason);
            */

            coreDataSelf.m_iDiceNumberNowMask = coreDataDelta.m_iDiceNumberNowMask;
            coreDataSelf.m_iDiceVisualStateKey = coreDataDelta.m_iDiceVisualStateKey;

            if (pDirtyRecord) {
                pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::Dice, 0, 0, true);
            }
        }


        //if (UMyMJUtilsLibrary::getBoolValueFromBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_IncreaseActionGroupId)) {
        if (coreDataDelta.m_bIncreaseActionGroupId) {
            if (ePusherType != MyMJGamePusherTypeCpp::PusherCountUpdate) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Increase action group id but type is %s."),
                    *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
                MY_VERIFY(false);
            }
            resetForNewActionLoop();
            coreDataSelf.m_iActionGroupId++;

        }

        //if (UMyMJUtilsLibrary::getBoolValueFromBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState)) {
        if (coreDataDelta.m_bUpdateGameState) {
            coreDataSelf.m_eGameState = coreDataDelta.m_eGameState;
        }

        //if (UMyMJUtilsLibrary::getBoolValueFromBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_ResetHelperLastCardsGivenOutOrWeave)) {
        if (coreDataDelta.m_bResetHelperLastCardsGivenOutOrWeave) {
            coreDataSelf.m_cHelper.m_aIdHelperLastCardsGivenOut.Reset();
            coreDataSelf.m_cHelper.m_aHelperLastWeaves.Reset();
        }

    }

    //todo: update it
    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    //    TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    if (delta.m_aRoleDataAttender.Num() > 0) {
        MY_VERIFY(delta.m_aRoleDataAttender.Num() == 1);
        const FMyMJRoleDataAttenderDeltaCpp& roleDataAttenderDelta = delta.m_aRoleDataAttender[0];
        int32 idxAttender = roleDataAttenderDelta.m_iIdxAttender;
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

        if (roleDataAttenderDelta.m_aDataPublic.Num() > 0) {

            MY_VERIFY(roleDataAttenderDelta.m_aDataPublic.Num() == 1);
            const FMyMJRoleDataAttenderPublicDeltaCpp& roleDataAttenderPubDelta = roleDataAttenderDelta.m_aDataPublic[0];

            FMyMJRoleDataAttenderPublicCpp *pRDAttenderPubSelf = &getRoleDataAttenderPublicRef(idxAttender);

            if (roleDataAttenderPubDelta.m_aWeave2Add.Num() > 0) {
                MY_VERIFY(roleDataAttenderPubDelta.m_aWeave2Add.Num() == 1);
                const FMyMJWeaveCpp& cWeave = roleDataAttenderPubDelta.m_aWeave2Add[0];

                MyMJWeaveTypeCpp eType = cWeave.getType();

                //1st, just teken card slot should be cleared in moving cards
                MY_VERIFY(pRDAttenderPubSelf->m_aIdJustTakenCards.Num() <= 0);

                //2nd, form a weave and add, and update helper minor posi info
                int32 idx = pRDAttenderPubSelf->m_aShowedOutWeaves.Emplace();
                pRDAttenderPubSelf->m_aShowedOutWeaves[idx] = cWeave;

                pCardInfoPack->helperRecalcMinorPosiOfCardsInShowedOutWeaves(pRDAttenderPubSelf->m_aShowedOutWeaves);
                /*
                const TArray<int32>& aIds = cWeave.getIdsRefConst();
                int32 l = aIds.Num();
                for (int32 i = 0; i < l; i++) {
                    pCardInfo = pCardInfoPack->getByIdx(aIds[i]);
                    pCardInfo->m_cPosi.m_iIdxInSlot0 = idx;
                    pCardInfo->m_cPosi.m_iIdxInSlot1 = i;
                }
                */

                //3rd, update helper
                coreDataSelf.m_cHelper.m_aIdHelperLastCardsGivenOut.Reset();
                coreDataSelf.m_cHelper.m_aHelperLastWeaves.Reset();

                coreDataSelf.m_cHelper.m_aHelperLastWeaves.Emplace(cWeave);

            }

            if (roleDataAttenderPubDelta.m_aHuScoreResultFinalGroup2Add.Num() > 0) {
                MY_VERIFY(roleDataAttenderPubDelta.m_aHuScoreResultFinalGroup2Add.Num() == 1);
                const FMyMJHuScoreResultFinalGroupCpp& resultFinalGroup2Add = roleDataAttenderPubDelta.m_aHuScoreResultFinalGroup2Add[0];

                pRDAttenderPubSelf->m_aHuScoreResultFinalGroups.Emplace(resultFinalGroup2Add);

                if (pDirtyRecord) {
                    pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePublic, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePublic_HuScoreResultFinalGroup, true);
                }
            }


            //UMyMJUtilsLibrary::testUpdateFlagAndsetBoolValueToStorageBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsRealAttender, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender);
            //UMyMJUtilsLibrary::testUpdateFlagAndsetBoolValueToStorageBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsStillInGame, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame);
            //UMyMJUtilsLibrary::testUpdateFlagAndsetBoolValueToStorageBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateGangYaoedLocalCS, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
        
            bool bTrivalDirty = false;
            if (roleDataAttenderPubDelta.m_bUpdateIsRealAttender) {
                pRDAttenderPubSelf->m_bIsRealAttender = roleDataAttenderPubDelta.m_bIsRealAttender;
                bTrivalDirty = true;
            }
            if (roleDataAttenderPubDelta.m_bUpdateIsStillInGame) {
                pRDAttenderPubSelf->m_bIsStillInGame = roleDataAttenderPubDelta.m_bIsStillInGame;
                bTrivalDirty = true;
            }
            if (roleDataAttenderPubDelta.m_bUpdateGangYaoedLocalCS) {
                pRDAttenderPubSelf->m_bGangYaoedLocalCS = roleDataAttenderPubDelta.m_bGangYaoedLocalCS;
                bTrivalDirty = true;
            }

            if (pDirtyRecord && bTrivalDirty) {
                pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePublic, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePublic_TrivalStates, true);
            }
        }


        if (roleDataAttenderDelta.m_aDataPrivate.Num() > 0) {
            MY_VERIFY(roleDataAttenderDelta.m_aDataPrivate.Num() == 1);
            const FMyMJRoleDataAttenderPrivateDeltaCpp& roleDataAttenderPriDelta = roleDataAttenderDelta.m_aDataPrivate[0];

            FMyMJRoleDataAttenderPrivateCpp *pRDAttenderPriSelf = getRoleDataAttenderPrivate(idxAttender);
            if (pRDAttenderPriSelf) {

                if (roleDataAttenderPriDelta.m_aActionContainor.Num() > 0) {
                    MY_VERIFY(roleDataAttenderPriDelta.m_aActionContainor.Num() == 1);

                    const FMyMJGameActionContainorForBPCpp &aActionContainor = roleDataAttenderPriDelta.m_aActionContainor[0];

                    pRDAttenderPriSelf->m_cActionContainor = aActionContainor;

                    if (pDirtyRecord) {
                        pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePrivate_ActionContainor, true);
                    }
                }

                if (roleDataAttenderPriDelta.m_aHuScoreResultTingGroup.Num() > 0) {
                    MY_VERIFY(roleDataAttenderPriDelta.m_aHuScoreResultTingGroup.Num() == 1);

                    const FMyMJHuScoreResultTingGroupCpp &resultTingGroup = roleDataAttenderPriDelta.m_aHuScoreResultTingGroup[0];

                    pRDAttenderPriSelf->m_cHuScoreResultTingGroup = resultTingGroup;

                    if (pDirtyRecord) {
                        pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePrivate_HuScoreResultTingGroup, true);
                    }
                }

            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("tring to apply role attender %d's private delta but target is NULL."), idxAttender);
            }

            //UMyMJUtilsLibrary::testUpdateFlagAndsetBoolValueToStorageBitMask(pRDAttenderPriSelf->m_iMask0, roleDataAttenderPriDelta.m_iMask0, FMyMJRoleDataAttenderPrivateCpp_Mask0_UpdateBanPaoHuLocalCS, FMyMJRoleDataAttenderPrivateCpp_Mask0_BanPaoHuLocalCS);
            if (roleDataAttenderPriDelta.m_bUpdateBanPaoHuLocalCS) {
                pRDAttenderPriSelf->m_bBanPaoHuLocalCS = roleDataAttenderPriDelta.m_bBanPaoHuLocalCS;
                if (pDirtyRecord) {
                    pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePrivate_TrivalStates, true);
                }
            }


        
        }

    }


    //We may need to apply role private data, but now we didn't have any except thos applied in step 1


    //all updated, let's increase pusher count
    coreDataSelf.m_iPusherIdLast++;
}


void FMyMJDataAccessorCpp::resetForNewActionLoop()
{
    for (int32 i = 0; i < 4; i++) {
        FMyMJRoleDataAttenderPrivateCpp* pRoleDataAttenderPriv = getRoleDataAttenderPrivate(i);

        if (m_bDebugIsFullMode) {
            MY_VERIFY(pRoleDataAttenderPriv);
        }

        if (pRoleDataAttenderPriv == NULL) {
            continue;
        }

        pRoleDataAttenderPriv->m_cActionContainor.resetForNewActionLoop();
    }
};

void FMyMJDataAccessorCpp::moveCardFromOldPosi(int32 id, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
{
    FMyMJCoreDataPublicCpp *pDCoreData = &getCoreDataRef();

    FMyMJCardInfoPackCpp  *pCardInfoPack = &pDCoreData->m_cCardInfoPack;
    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackRef();

    FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
    MyMJCardSlotTypeCpp eSlotSrc = pCardInfo->m_cPosi.m_eSlot;
    int32 idxAttender = pCardInfo->m_cPosi.m_iIdxAttender;

    if (idxAttender >= 0 && idxAttender < 4 && (uint8)eSlotSrc > (uint8)MyMJCardSlotTypeCpp::InvalidIterateMin && (uint8)eSlotSrc < (uint8)MyMJCardSlotTypeCpp::InvalidIterateMax) {

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("found invalid idxAttender %d, eslot %d."), idxAttender, (uint8)eSlotSrc);
    }

    if (eSlotSrc == MyMJCardSlotTypeCpp::Untaken) {
        MY_VERIFY(pCardInfo->m_cPosi.m_iIdxInSlot0 >= 0 && pCardInfo->m_cPosi.m_iIdxInSlot0 < pDCoreData->m_aUntakenCardStacks.Num());
        int32 idx = pCardInfo->m_cPosi.m_iIdxInSlot0;
        FMyIdCollectionCpp *pCollection = &pDCoreData->m_aUntakenCardStacks[idx];
        //MY_VERIFY(pCard->m_cPosi.m_iIdxInSlot1 >= 0 && pCard->m_cPosi.m_iIdxInSlot1 < pCollection->m_aIds.Num());

        MY_VERIFY(pCollection->m_aIds.Pop() == id);

        pCardInfo->m_cPosi.reset();

        pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal--;
        if (pDCoreData->m_cUntakenSlotInfo.isIdxUntakenSlotInKeptFromTailSegment(idx)) {
            pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail--;

            pDCoreData->m_cUntakenSlotInfo.updateUntakenInfoHeadOrTail(pDCoreData->m_aUntakenCardStacks, false, true);
        }
        else {
            pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead--;

            pDCoreData->m_cUntakenSlotInfo.updateUntakenInfoHeadOrTail(pDCoreData->m_aUntakenCardStacks, true, false);
        }

        pCardInfo->m_cPosi.reset();

    }
    else if (idxAttender >= 0 && idxAttender < 4) {

        MyMJCardSlotTypeCpp eType = pCardInfo->m_cPosi.m_eSlot;

        FMyMJRoleDataAttenderPublicCpp *pDAttenderPubD = &getRoleDataAttenderPublicRef(idxAttender);
        MY_VERIFY(pDAttenderPubD);
        FMyMJRoleDataAttenderPrivateCpp *pDAttenderPriD = getRoleDataAttenderPrivate(idxAttender);
        MY_VERIFY(pDAttenderPriD);

        bool bRemovedOne = false;
        if (eType == MyMJCardSlotTypeCpp::JustTaken) {
            pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdJustTakenCards, id, &bRemovedOne);
            MY_VERIFY(bRemovedOne);
        }
        else if (eType == MyMJCardSlotTypeCpp::InHand) {

            pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdHandCards, id, &bRemovedOne);
            MY_VERIFY(bRemovedOne);

        }
        else if (eType == MyMJCardSlotTypeCpp::GivenOut) {

            pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdGivenOutCards, id, &bRemovedOne);
            MY_VERIFY(bRemovedOne);
        }
        else if (eType == MyMJCardSlotTypeCpp::Weaved) {
            bool bWeavesDeleted;

            MY_VERIFY(UMyMJUtilsLibrary::removeCardByIdInWeaves(pDAttenderPubD->m_aShowedOutWeaves, id, &bWeavesDeleted));

            if (bWeavesDeleted) {
                pCardInfoPack->helperRecalcMinorPosiOfCardsInShowedOutWeaves(pDAttenderPubD->m_aShowedOutWeaves);
            }

        }
        else if (eType == MyMJCardSlotTypeCpp::WinSymbol) {

            pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdWinSymbolCards, id, &bRemovedOne);
            MY_VERIFY(bRemovedOne);

        }
        else if (eType == MyMJCardSlotTypeCpp::ShownOnDesktop) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("removing card from shownOnDesktop slot, this logic is not supposed to run in current game rules."));

            pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdShownOnDesktopCards, id, &bRemovedOne);
            MY_VERIFY(bRemovedOne);
        }
        else
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("FMyMJDataAccessorCpp removeCard: not a valid slot type: %d"), (uint8)eType);
            MY_VERIFY(false);
        }

        /*
        if (m_bUpdateVisualData &&
            (eType == MyMJCardSlotTypeCpp::JustTaken || eType == MyMJCardSlotTypeCpp::InHand)) {
            pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdHandAndJustTakenCardsSortedByValueForVisual, id, &bRemovedOne);
            MY_VERIFY(bRemovedOne);
        }
        */

        pCardInfo->m_cPosi.reset();

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid idxAttender %d, eslot %d."), idxAttender, (uint8)eSlotSrc);
        MY_VERIFY(false);
    }


    //dirty mask handle
    if (pDirtyRecord) {
        pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::Card, idxAttender, (int32)eSlotSrc, true);
    }

};

void FMyMJDataAccessorCpp::moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
{
    FMyMJCoreDataPublicCpp *pCoreData = &getCoreDataRef();

    FMyMJCardInfoPackCpp  *pCardInfoPack = &pCoreData->m_cCardInfoPack;
    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackRef();

    if (idxAttender >= 0 && idxAttender < 4 && (uint8)eSlotDst >(uint8)MyMJCardSlotTypeCpp::InvalidIterateMin && (uint8)eSlotDst < (uint8)MyMJCardSlotTypeCpp::InvalidIterateMax) {

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("found invalid idxAttender %d, eslot %d."), idxAttender, (uint8)eSlotDst);
    }

    if (eSlotDst == MyMJCardSlotTypeCpp::Untaken) {
        MY_VERIFY(false);
    }
    else if (idxAttender >= 0 && idxAttender < 4) {

        FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
        MyMJCardSlotTypeCpp eType = eSlotDst;

        pCardInfo->m_cPosi.m_iIdxAttender = idxAttender;
        pCardInfo->m_cPosi.m_eSlot = eType;

        FMyMJRoleDataAttenderPublicCpp *pDAttenderPubD = &getRoleDataAttenderPublicRef(idxAttender);
        MY_VERIFY(pDAttenderPubD);
        FMyMJRoleDataAttenderPrivateCpp *pDAttenderPriD = getRoleDataAttenderPrivate(idxAttender);
        MY_VERIFY(pDAttenderPriD);

        if (eType == MyMJCardSlotTypeCpp::JustTaken) {
            pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdJustTakenCards, id);
        }
        else if (eType == MyMJCardSlotTypeCpp::InHand) {
            pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdHandCards, id);
        }
        else if (eType == MyMJCardSlotTypeCpp::GivenOut) {
            pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdGivenOutCards, id);
        }
        else if (eType == MyMJCardSlotTypeCpp::Weaved) {
            //We don't directly insert here, instead, operate weaves later in applyActionWeave
        }
        else if (eType == MyMJCardSlotTypeCpp::WinSymbol) {
            pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdWinSymbolCards, id);
        }
        else if (eType == MyMJCardSlotTypeCpp::ShownOnDesktop) {
            pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDAttenderPubD->m_aIdShownOnDesktopCards, id);
        }
        else
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid idxAttender %d, eslot %d."), idxAttender, (uint8)eType);
            MY_VERIFY(false);
        }

    }
    else {
        MY_VERIFY(false);
    }


    //dirty mask handle
    if (pDirtyRecord) {
        pDirtyRecord->setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::Card, idxAttender, (int32)eSlotDst, true);
    }
};


void FMyMJDataAccessorCpp::helperSetCoreDataDirtyRecordAllDirty(FMyDirtyRecordWithKeyAnd4IdxsMapCpp &cDirtyRecord)
{
    cDirtyRecord.reset();

    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        for (int32 eSlot = ((int32)MyMJCardSlotTypeCpp::InvalidIterateMin + 1); eSlot < ((int32)MyMJCardSlotTypeCpp::InvalidIterateMax); eSlot++) {
            cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::Card, idxAttender, (int32)eSlot, true);
        }
    }

    cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::Dice, 0, 0, true);

    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePublic, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePublic_HuScoreResultFinalGroup, true);
        cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePublic, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePublic_TrivalStates, true);

        cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePrivate_ActionContainor, true);
        cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePrivate_HuScoreResultTingGroup, true);
        cDirtyRecord.setDirtyWith3Idxs((int32)MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate, idxAttender, MyMJGameCoreDataDirtySubType_AttenderStatePrivate_TrivalStates, true);
    }

    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("helperSetCoreDataDirtyRecordAllDirty, result %d."), cDirtyRecord.getRecordSet().Num());
}