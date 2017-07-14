// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameAttender.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

#include "MyMJGameCore.h"


//JustTaken = 3                      UMETA(DisplayName = "JustTaken"),
//InHand = 4                        UMETA(DisplayName = "InHand"),
//GivenOut = 5                        UMETA(DisplayName = "GivenOut"),
//Weaved = 6                        UMETA(DisplayName = "Weaved"),
//WinSymbol = 7                        UMETA(DisplayName = "WinSymbol")

void FMyMJGameAttenderCpp::removeCard(int32 id)
{
    FMyMJCardPackCpp *pCardPack = m_pCore.Pin()->getpCardPack();
    FMyMJCardCpp *pCard = pCardPack->getCardByIdx(id);
    MyMJCardSlotTypeCpp eType = pCard->m_cPosi.m_eSlot;


    if (eType == MyMJCardSlotTypeCpp::JustTaken) {
        FMyMJCardPackCpp::helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(*pCardPack, m_aIdJustTakenCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::InHand) {
        MY_VERIFY(pCard->m_iValue > 0);
        bool bRemoved = m_cHandCards.remove(id, pCard->m_iValue);
        MY_VERIFY(bRemoved);
    }
    else if (eType == MyMJCardSlotTypeCpp::GivenOut) {
        FMyMJCardPackCpp::helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(*pCardPack, m_aIdGivenOutCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::Weaved) {
        bool bWeavesDeleted;

        MY_VERIFY(UMyMJUtilsLibrary::removeCardByIdInWeaves(m_aShowedOutWeaves, id, &bWeavesDeleted));

        if (bWeavesDeleted) {
            recalcMinorPosiOfCardsInShowedOutWeaves();
        }
    }
    else if (eType == MyMJCardSlotTypeCpp::WinSymbol) {
        FMyMJCardPackCpp::helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(*pCardPack, m_aIdWinSymbolCards, id);
        //m_cHandCards.remove(id, pCard->m_iValue);
    }
    else
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("removeCard: not a valid slot type: %d"), (uint8)eType);
        MY_VERIFY(false);
    }

    pCard->m_cPosi.reset();

}

void FMyMJGameAttenderCpp::insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot)
{
    FMyMJCardPackCpp *pCardPack = m_pCore.Pin()->getpCardPack();
    FMyMJCardCpp *pCard = pCardPack->getCardByIdx(id);
    MyMJCardSlotTypeCpp eType = eTargetSlot;

    pCard->m_cPosi.m_iIdxAttender = m_iIdx;
    pCard->m_cPosi.m_eSlot = eType;


    if (eType == MyMJCardSlotTypeCpp::JustTaken) {
        FMyMJCardPackCpp::helperInsertCardUniqueToIdArrayWithMinorPosiCalced(*pCardPack, m_aIdJustTakenCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::InHand) {
        MY_VERIFY(pCard->m_iValue > 0); //If it is inserted into hand, its value must be revealed first
        bool bInserted = m_cHandCards.insert(id, pCard->m_iValue);
        MY_VERIFY(bInserted);

        pCard->m_cPosi.resetMinorData();

    }
    else if (eType == MyMJCardSlotTypeCpp::GivenOut) {
        FMyMJCardPackCpp::helperInsertCardUniqueToIdArrayWithMinorPosiCalced(*pCardPack, m_aIdGivenOutCards, id);
    }
    else if (eType == MyMJCardSlotTypeCpp::Weaved) {
        //We don't directly insert here, instead, operate weaves later in applyActionWeave

    }
    else if (eType == MyMJCardSlotTypeCpp::WinSymbol) {
        FMyMJCardPackCpp::helperInsertCardUniqueToIdArrayWithMinorPosiCalced(*pCardPack, m_aIdWinSymbolCards, id);
    }
    else
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("insertCard: not a valid slot type: %d"), (uint8)eType);
        MY_VERIFY(false);
    }


}

void FMyMJGameAttenderCpp::recalcMinorPosiOfCardsInShowedOutWeaves()
{
    FMyMJCardPackCpp *pCardPack = getpCore()->getpCardPack();

    //fix helper posi in all weaves
    int32 l0 = m_aShowedOutWeaves.Num();
    for (int32 i = 0; i < l0; i++) {
        FMyMJWeaveCpp *pWeave = &m_aShowedOutWeaves[i];
        const TArray<FMyIdValuePair>& aT = pWeave->getIdValuesRef();
        int32 l1 = aT.Num();
        for (int32 j = 0; j < l1; j++) {
            FMyMJCardCpp *pCard = pCardPack->getCardByIdx(aT[j].m_iId);
            pCard->m_cPosi.m_iIdxInSlot0 = i;
            pCard->m_cPosi.m_iIdxInSlot1 = j;
        }
    }
}