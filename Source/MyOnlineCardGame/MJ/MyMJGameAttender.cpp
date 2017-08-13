// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameAttender.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

#include "MyMJGameCore.h"


void FMyMJGameAttenderCpp::removeCard(int32 id)
{
    FMyMJGameCoreCpp *pCore = m_pCore.Pin().Get();
    FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSys();

    FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
    MyMJCardSlotTypeCpp eType = pCardInfo->m_cPosi.m_eSlot;

    FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirect();
    MY_VERIFY(pDPubD);
    FMyMJRoleDataAttenderPrivateCpp *pDPriD = getDataPrivateDirect();
    MY_VERIFY(pDPriD);

    if (eType == MyMJCardSlotTypeCpp::JustTaken) {
        pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDPubD->m_aIdJustTakenCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::InHand) {
        int32 value = pCardValuePack->getByIdx(id);
        MY_VERIFY(value> 0);
        bool bRemoved = m_cDataLogic.m_cHandCards.remove(id, value);
        MY_VERIFY(bRemoved);
    }
    else if (eType == MyMJCardSlotTypeCpp::GivenOut) {
        pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDPubD->m_aIdGivenOutCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::Weaved) {
        bool bWeavesDeleted;

        MY_VERIFY(UMyMJUtilsLibrary::removeCardByIdInWeaves(pDPubD->m_aShowedOutWeaves, id, &bWeavesDeleted));

        if (bWeavesDeleted) {
            recalcMinorPosiOfCardsInShowedOutWeaves();
        }
    }
    else if (eType == MyMJCardSlotTypeCpp::WinSymbol) {
        pCardInfoPack->helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(pDPubD->m_aIdWinSymbolCards, id);
        //m_cHandCards.remove(id, pCard->m_iValue);
    }
    else
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("removeCard: not a valid slot type: %d"), (uint8)eType);
        MY_VERIFY(false);
    }

    pCardInfo->m_cPosi.reset();

}

void FMyMJGameAttenderCpp::insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot)
{
    FMyMJGameCoreCpp *pCore = m_pCore.Pin().Get();
    FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSys();

    FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
    MyMJCardSlotTypeCpp eType = eTargetSlot;

    pCardInfo->m_cPosi.m_iIdxAttender = getIdx();
    pCardInfo->m_cPosi.m_eSlot = eType;

    FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirect();
    MY_VERIFY(pDPubD);
    FMyMJRoleDataAttenderPrivateCpp *pDPriD = getDataPrivateDirect();
    MY_VERIFY(pDPriD);

    if (eType == MyMJCardSlotTypeCpp::JustTaken) {
        pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDPubD->m_aIdJustTakenCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::InHand) {
        int32 value = pCardValuePack->getByIdx(id);
        MY_VERIFY(value> 0);
        bool bInserted = m_cDataLogic.m_cHandCards.insert(id, value);
        MY_VERIFY(bInserted);

        pCardInfo->m_cPosi.resetMinorData();

    }
    else if (eType == MyMJCardSlotTypeCpp::GivenOut) {
        pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDPubD->m_aIdGivenOutCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::Weaved) {
        //We don't directly insert here, instead, operate weaves later in applyActionWeave

    }
    else if (eType == MyMJCardSlotTypeCpp::WinSymbol) {
        pCardInfoPack->helperInsertCardUniqueToIdArrayWithMinorPosiCalced(pDPubD->m_aIdWinSymbolCards, id);
    }
    else
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("insertCard: not a valid slot type: %d"), (uint8)eType);
        MY_VERIFY(false);
    }


}

void FMyMJGameAttenderCpp::recalcMinorPosiOfCardsInShowedOutWeaves()
{
    FMyMJGameCoreCpp *pCore = m_pCore.Pin().Get();
    FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPack();

    FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirect();
    MY_VERIFY(pDPubD);
    FMyMJRoleDataAttenderPrivateCpp *pDPriD = getDataPrivateDirect();
    MY_VERIFY(pDPriD);

    //fix helper posi in all weaves

    int32 l0 = pDPubD->m_aShowedOutWeaves.Num();
    for (int32 i = 0; i < l0; i++) {
        FMyMJWeaveCpp *pWeave = &pDPubD->m_aShowedOutWeaves[i];
        const TArray<int32> &aT = pWeave->getIdsRefConst();
        int32 l1 = aT.Num();
        for (int32 j = 0; j < l1; j++) {
            FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(aT[j]);
            pCardInfo->m_cPosi.m_iIdxInSlot0 = i;
            pCardInfo->m_cPosi.m_iIdxInSlot1 = j;
        }
    }
}