// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameData.h"

#include "UnrealNetwork.h"
#include "Engine/ActorChannel.h"

void UMyMJCoreDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJCoreDataForMirrorModeCpp, m_cDataPubicDirect);
};

void UMyMJRoleDataAttenderPublicForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataAttenderPublicForMirrorModeCpp, m_cDataAttenderPublic);
};

void UMyMJRoleDataAttenderPrivateForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataAttenderPrivateForMirrorModeCpp, m_cDataAttenderPrivate);
};

void UMyMJRoleDataPrivateForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataPrivateForMirrorModeCpp, m_cDataPrivate);
};

void UMyMJRoleDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataForMirrorModeCpp, m_pDataAttenderPublic);
    DOREPLIFETIME(UMyMJRoleDataForMirrorModeCpp, m_pDataAttenderPrivate);
    DOREPLIFETIME(UMyMJRoleDataForMirrorModeCpp, m_pDataPrivate);
};

/*
bool UMyMJRoleDataForMirrorModeCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJRoleDataForMirrorModeCpp::ReplicateSubobjects."));

    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);



    if (m_pDataPublic != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(m_pDataPublic, *Bunch, *RepFlags);
    }

    if (m_pDataPrivate != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(m_pDataPrivate, *Bunch, *RepFlags);
    }

    return WroteSomething;

};
*/

void UMyMJDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJDataForMirrorModeCpp, m_pCoreData);
    DOREPLIFETIME(UMyMJDataForMirrorModeCpp, m_aRoleDatas);
};

//Component replicate subobjects, just take care of child subobject, it's own property will be taken care by caller
bool UMyMJDataForMirrorModeCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJDataForMirrorModeCpp::ReplicateSubobjects 0."));

    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);


    if (m_pCoreData != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(m_pCoreData, *Bunch, *RepFlags);
    }

    int l = m_aRoleDatas.Num();
    for (int i = 0; i < l; i++) {
        //m_aAttenderDatas[i]->ReplicateSubobjects(Channel, Bunch, RepFlags);
        WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i], *Bunch, *RepFlags);

        if (m_aRoleDatas[i]->m_pDataAttenderPublic) {
            WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i]->m_pDataAttenderPublic, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("role attender public is NULL."));
        }

        if (m_aRoleDatas[i]->m_pDataAttenderPrivate) {
            WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i]->m_pDataAttenderPrivate, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("role attender private is NULL."));
        }

        if (m_aRoleDatas[i]->m_pDataPrivate) {
            WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i]->m_pDataPrivate, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("role private is NULL."));
        }
    }


    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJDataForMirrorModeCpp::ReplicateSubobjects 1, %d"), WroteSomething);

    return WroteSomething;
};


void UMyMJDataForMirrorModeCpp::createSubObjects(bool bBuildingDefault, bool bHavePrivate)
{
    if (bBuildingDefault) {
        m_pCoreData = CreateDefaultSubobject<UMyMJCoreDataForMirrorModeCpp>(TEXT("UMyMJCoreDataForMirrorModeCpp"));
    }
    else {
        m_pCoreData = NewObject<UMyMJCoreDataForMirrorModeCpp>(this);
    }

    m_aRoleDatas.Reset();
    for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
        UMyMJRoleDataForMirrorModeCpp *pRoleData;
        if (bBuildingDefault) {
            pRoleData = CreateDefaultSubobject<UMyMJRoleDataForMirrorModeCpp>(*FString::Printf(TEXT("UMyMJRoleDataForMirrorModeCpp_%d"), i));
        }
        else {
            pRoleData = NewObject<UMyMJRoleDataForMirrorModeCpp>(this);
        }

        pRoleData->createSubObjects(false, i < 4, bHavePrivate);
        pRoleData->resetup(i);

        int32 idx = m_aRoleDatas.Emplace(pRoleData);

        MY_VERIFY(idx == i);
    }
};

void FMyMJDataAccessorCpp::applyDeltaStep0(const FMyMJDataDeltaCpp &delta)
{
    while (delta.m_aRoleDataPrivate.Num() > 0) {
        MY_VERIFY(delta.m_aRoleDataPrivate.Num() == 1);
        const FMyMJRoleDataPrivateDeltaCpp& roleDataPriDelta = delta.m_aRoleDataPrivate[0];

        MyMJGameRoleTypeCpp eRoleType = roleDataPriDelta.m_eRoleType;
        if (m_eAccessRoleType != eRoleType) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("tring to apply role private delta but access role is different, %d, %d."), (uint8)m_eAccessRoleType, (uint8)eRoleType);
            break;
        }

        FMyMJRoleDataPrivateCpp *pRDPriSelf = getRoleDataPrivate((uint8)eRoleType);
        if (!pRDPriSelf) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("tring to apply role %d's private delta but target is NULL."), (uint8)eRoleType);
            break;
        }

        if (roleDataPriDelta.m_aIdValuePairs2Reveal.Num() > 0) {
            getCardValuePackRef().tryRevealCardValueByIdValuePairs(roleDataPriDelta.m_aIdValuePairs2Reveal);

        }

        break;
    }
}

void FMyMJDataAccessorCpp::applyDeltaStep1(const FMyMJDataDeltaCpp &delta)
{
    FMyMJCoreDataPublicCpp &coreDataSelf = getCoreDataRef();

    MyMJGamePusherTypeCpp ePusherType = delta.getType();

    FMyMJCardInfoCpp *pCardInfo = NULL;

    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackRef();
    FMyMJCardInfoPackCpp  *pCardInfoPack = &coreDataSelf.m_cCardInfoPack;

    if (!(coreDataSelf.m_iGameId == delta.m_iGameId && coreDataSelf.m_iPusherIdLast == delta.getId())) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to apply delta since id mismatch: game id [%d, %d], push id [%d, %d]."), coreDataSelf.m_iGameId, delta.m_iGameId, coreDataSelf.m_iPusherIdLast, delta.getId());
        return;
    }

    if (delta.m_aCoreData.Num() > 0) {
        MY_VERIFY(delta.m_aCoreData.Num() == 1);
        const FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[0];

        //update card info
        int32 l = coreDataDelta.m_aCardInfos2Update.Num();
        for (int32 i = 0; i < l; i++) {
            const FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[i];
            FMyMJCardInfoCpp& cardInfoSelf = pCardInfoPack->getRefByIdx(cardInfoTarget.m_iId);

            MY_VERIFY(cardInfoSelf.m_iId == cardInfoTarget.m_iId);

            if (cardInfoSelf.m_cPosi != cardInfoTarget.m_cPosi) {
                //for safety, any different exist, we make movement happen
                moveCardFromOldPosi(cardInfoTarget.m_iId);
                moveCardToNewPosi(cardInfoTarget.m_iId, cardInfoTarget.m_cPosi.m_iIdxAttender, cardInfoTarget.m_cPosi.m_eSlot);
            }

            if (cardInfoTarget.m_eFlipState != MyMJCardFlipStateCpp::Invalid) {
                cardInfoSelf.m_eFlipState = cardInfoTarget.m_eFlipState;
            }

        }

        if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
            MY_VERIFY(l > 0);
            coreDataSelf.m_aHelperLastCardsGivenOutOrWeave.Reset();

            FMyIdValuePair cIdValue;
            for (int32 i = 0; i < l; i++) {

                cIdValue.m_iId = coreDataDelta.m_aCardInfos2Update[i].m_iId;
                cIdValue.m_iValue = pCardValuePack->getByIdx(cIdValue.m_iId);

                coreDataSelf.m_aHelperLastCardsGivenOutOrWeave.Emplace(cIdValue);
            }
        }

        if (UMyMJUtilsLibrary::getBoolValueFromBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateActionGroupId)) {
            coreDataSelf.m_iActionGroupId = coreDataDelta.m_iActionGroupId;
        }

        if (UMyMJUtilsLibrary::getBoolValueFromBitMask(coreDataDelta.m_iMask0, FMyMJCoreDataPublicDirectMask0_UpdateGameState)) {
            coreDataSelf.m_eGameState = coreDataDelta.m_eGameState;
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

                const TArray<int32>& aIds = cWeave.getIdsRefConst();
                int32 l = aIds.Num();
                for (int32 i = 0; i < l; i++) {
                    pCardInfo = pCardInfoPack->getByIdx(aIds[i]);
                    pCardInfo->m_cPosi.m_iIdxInSlot0 = idx;
                    pCardInfo->m_cPosi.m_iIdxInSlot1 = i;
                }

                //3rd, consider the type and update

                coreDataSelf.m_aHelperLastCardsGivenOutOrWeave.Reset();

                FMyIdValuePair cIdValue;
                cIdValue.m_iId = cWeave.getRepresentCardId();
                cIdValue.m_iValue = pCardValuePack->getByIdx(cIdValue.m_iId);

                coreDataSelf.m_aHelperLastCardsGivenOutOrWeave.Reset();
                coreDataSelf.m_aHelperLastCardsGivenOutOrWeave.Emplace(cIdValue);
            }

            if (roleDataAttenderPubDelta.m_aHuScoreResultFinalGroupLocal.Num() > 0) {
                MY_VERIFY(roleDataAttenderPubDelta.m_aHuScoreResultFinalGroupLocal.Num() == 1);
                const FMyMJHuScoreResultFinalGroupCpp& resultFinalGroup = roleDataAttenderPubDelta.m_aHuScoreResultFinalGroupLocal[0];

                pRDAttenderPubSelf->m_cHuScoreResultFinalGroupLocal = resultFinalGroup;
            }

            if (roleDataAttenderPubDelta.m_aHuScoreResultFinalGroup.Num() > 0) {
                MY_VERIFY(roleDataAttenderPubDelta.m_aHuScoreResultFinalGroup.Num() == 1);
                const FMyMJHuScoreResultFinalGroupCpp& resultFinalGroup = roleDataAttenderPubDelta.m_aHuScoreResultFinalGroup[0];

                pRDAttenderPubSelf->m_cHuScoreResultFinalGroup = resultFinalGroup;
            }

            UMyMJUtilsLibrary::testAndsetBoolValueToBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsRealAttender, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender);
            UMyMJUtilsLibrary::testAndsetBoolValueToBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsStillInGame, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame);
            UMyMJUtilsLibrary::testAndsetBoolValueToBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateGangYaoedLocalCS, FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS);
            UMyMJUtilsLibrary::testAndsetBoolValueToBitMask(pRDAttenderPubSelf->m_iMask0, roleDataAttenderPubDelta.m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateBanPaoHuLocalCS, FMyMJRoleDataAttenderPublicCpp_Mask0_BanPaoHuLocalCS);
        }


        if (roleDataAttenderDelta.m_aDataPrivate.Num() > 0) {
            MY_VERIFY(roleDataAttenderDelta.m_aDataPrivate.Num() == 1);
            const FMyMJRoleDataAttenderPrivateDeltaCpp& roleDataAttenderPriDelta = roleDataAttenderDelta.m_aDataPrivate[0];

            FMyMJRoleDataAttenderPrivateCpp *pRDAttenderPriSelf = getRoleDataAttenderPrivate(idxAttender);
            if (pRDAttenderPriSelf) {

                if (roleDataAttenderPriDelta.m_aHuScoreResultTingGroup.Num() > 0) {
                    MY_VERIFY(roleDataAttenderPriDelta.m_aHuScoreResultTingGroup.Num() == 1);

                    const FMyMJHuScoreResultTingGroupCpp &resultTingGroup = roleDataAttenderPriDelta.m_aHuScoreResultTingGroup[0];

                    pRDAttenderPriSelf->m_cHuScoreResultTingGroup = resultTingGroup;
                }

            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("tring to apply role attender %d's private delta but target is NULL."), idxAttender);
            }

        }

    }


    //We may need to apply role private data, but now we didn't have any except thos applied in step 1

    //all updated, let's increase pusher count
    coreDataSelf.m_iPusherIdLast++;
}


void FMyMJDataAccessorCpp::applyDelta(const FMyMJDataDeltaCpp &delta)
{
    applyDeltaStep0(delta);
    applyDeltaStep1(delta);

};


void FMyMJDataAccessorCpp::moveCardFromOldPosi(int32 id)
{
    FMyMJCoreDataPublicCpp *pDCoreData = &getCoreDataRef();

    FMyMJCardInfoPackCpp  *pCardInfoPack = &pDCoreData->m_cCardInfoPack;
    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackRef();

    FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
    MyMJCardSlotTypeCpp eSlotSrc = pCardInfo->m_cPosi.m_eSlot;
    int32 idxAttender = pCardInfo->m_cPosi.m_iIdxAttender;
    if (eSlotSrc == MyMJCardSlotTypeCpp::Untaken) {
        MY_VERIFY(pCardInfo->m_cPosi.m_iIdxInSlot0 >= 0 && pCardInfo->m_cPosi.m_iIdxInSlot0 < pDCoreData->m_aUntakenCardStacks.Num());
        int32 idx = pCardInfo->m_cPosi.m_iIdxInSlot0;
        FMyIdCollectionCpp *pCollection = &pDCoreData->m_aUntakenCardStacks[idx];
        //MY_VERIFY(pCard->m_cPosi.m_iIdxInSlot1 >= 0 && pCard->m_cPosi.m_iIdxInSlot1 < pCollection->m_aIds.Num());

        MY_VERIFY(pCollection->m_aIds.Pop() == id);

        pCardInfo->m_cPosi.reset();

        if (pDCoreData->m_cUntakenSlotInfo.isIdxUntakenSlotInKeptFromTailSegment(idx)) {
            pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail--;
        }
        else {
            pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead--;
            if (pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead == 0) {
                pDCoreData->m_cHelperLastCardTakenInGame.m_iId = id;
                pDCoreData->m_cHelperLastCardTakenInGame.m_iValue = pCardValuePack->getByIdx(id);
            }
        }
        pDCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal--;

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
        else
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("FMyMJDataAccessorCpp removeCard: not a valid slot type: %d"), (uint8)eType);
            MY_VERIFY(false);
        }

        pCardInfo->m_cPosi.reset();
    }
    else {
        MY_VERIFY(false);
    }
};

void FMyMJDataAccessorCpp::moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst)
{
    FMyMJCoreDataPublicCpp *pCoreData = &getCoreDataRef();

    FMyMJCardInfoPackCpp  *pCardInfoPack = &pCoreData->m_cCardInfoPack;
    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackRef();

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
        else
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("insertCard: not a valid slot type: %d"), (uint8)eType);
            MY_VERIFY(false);
        }
    }
    else {
        MY_VERIFY(false);
    }
};

void FMyMJDataAccessorCpp::updateUntakenInfoHeadOrTail(bool bUpdateHead, bool bUpdateTail)
{
    FMyMJCoreDataPublicCpp *pCoreData = &getCoreDataRef();

    if (pCoreData->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal <= 0) {
        return;
    }

    if (bUpdateHead) {
        int32 idx = UMyMJUtilsLibrary::getIdxOfUntakenSlotHavingCard(pCoreData->m_aUntakenCardStacks, pCoreData->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow, 0, false);
        MY_VERIFY(idx >= 0);
        pCoreData->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow = idx;
    }
    if (bUpdateTail) {
        int32 idx = UMyMJUtilsLibrary::getIdxOfUntakenSlotHavingCard(pCoreData->m_aUntakenCardStacks, pCoreData->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow, 0, true);
        MY_VERIFY(idx >= 0);
        pCoreData->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow = idx;
    }
}