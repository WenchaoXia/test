// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJCardPack.h"
#include "utils/CommonUtils/MyCardUtils.h"

const FMyMJCardInfoCpp*
FMyMJCardInfoPackCpp::getByIdxConst(int32 idx) const
{
    int32 l = m_aCardInfos.Num();
    if (idx >= 0 && idx < l) {
        return &m_aCardInfos[idx];
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idx out of range: %d/%d"), idx, l);
        MY_VERIFY(false);
        return NULL;
    }
}

void FMyMJCardInfoPackCpp::helperVerifyInfos() const
{
    int32 l = getLength();
    MY_VERIFY(l > 0);

    TMap<int32, int32> mValueCount;
    for (int i = 0; i < l; i++) {
        const FMyMJCardInfoCpp* pInfo = getByIdxConst(i);

        if (pInfo->m_iId != i) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("id not equal %d:%d."), pInfo->m_iId, i);
            MY_VERIFY(false);
        }
    }
}

bool FMyMJCardInfoPackCpp::helperInsertCardUniqueToIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id)
{
    bool ret = false;
    bool bHaveInsertedOne = false;

    int32 idx = aIds.AddUnique(id);
    if (idx >= 0) {

        bHaveInsertedOne = true;

        if (aIds.Num() == (idx + 1)) {
            
            //top
            FMyMJCardInfoCpp *pInfo = getByIdx(id);
            pInfo->m_cPosi.resetMinorData();
            pInfo->m_cPosi.m_iIdxInSlot0 = idx;

        }
        else {

            //Mid
            helperRecalcMinorPosiOfCardsInIdArray(aIds);
            ret = true;
        }

    }
    else {
        //not added , then it is not unique
        MY_VERIFY(false);
    }

    //if (outbHaveInsertedOne) {
        //*outbHaveInsertedOne = bHaveInsertedOne;
    //}

    return ret;
}

bool FMyMJCardInfoPackCpp::helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id, bool *outbHaveRemovedOne)
{
    bool ret = false;
    bool bHaveRemovedOne = false;

    if (aIds.Top() == id) {
        aIds.Pop();
        bHaveRemovedOne = true;

        //risky here as not unique, but ok for most cases
    }
    else {

        int32 l = aIds.Remove(id);
        if (l < 1) {

        }
        else if (l == 1) {
            bHaveRemovedOne = true;

            helperRecalcMinorPosiOfCardsInIdArray(aIds);
            ret = true;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("removed card is not unique: %d removed"), l);
            MY_VERIFY(false);
        }

    }

    FMyMJCardInfoCpp *pInfo = getByIdx(id);
    pInfo->m_cPosi.resetMinorData();


    if (outbHaveRemovedOne) {
        *outbHaveRemovedOne = bHaveRemovedOne;
    }

    return ret;
}

void FMyMJCardInfoPackCpp::helperRecalcMinorPosiOfCardsInIdArray(const TArray<int32> &aIds)
{
    int32 l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJCardInfoCpp *pInfo = getByIdx(aIds[i]);
        pInfo->m_cPosi.resetMinorData();
        pInfo->m_cPosi.m_iIdxInSlot0 = i;
    }
}

void FMyMJCardInfoPackCpp::helperRecalcMinorPosiOfCardsInShowedOutWeaves(const TArray<FMyMJWeaveCpp> &aShowedOutWeaves)
{
    //fix helper posi in all weaves
    int32 l0 = aShowedOutWeaves.Num();
    for (int32 i = 0; i < l0; i++) {
        const FMyMJWeaveCpp *pWeave = &aShowedOutWeaves[i];
        const TArray<int32>& aT = pWeave->getIdsRefConst();
        int32 l1 = aT.Num();
        for (int32 j = 0; j < l1; j++) {
            FMyMJCardInfoCpp *pInfo = getByIdx(aT[j]);
            pInfo->m_cPosi.resetMinorData();
            pInfo->m_cPosi.m_iIdxInSlot0 = i;
            pInfo->m_cPosi.m_iIdxInSlot1 = j;
        }
    }
}

int32 FMyMJCardInfoPackCpp::helperFindCardByIdInCardInfos(const TArray<FMyMJCardInfoCpp>& aCardInfos, int32 idCard)
{
    int32 ret = -1;
    int32 l = aCardInfos.Num();

    for (int32 i = 0; i < l; i++) {
        if (aCardInfos[i].m_iId == idCard) {
            ret = i;
            break;
        }
    }

    return ret;
}

/*
void FMyMJCardPackCpp::reset(const TArray<FMyIdValuePair> &aShuffledIdValues)
{
    int32 l = aShuffledIdValues.Num();

    m_aCards.Reset(l);
    for (int32 i = 0; i < l; i++) {
        int32 idx = m_aCards.Emplace();
        MY_VERIFY(i == idx);
        FMyMJCardCpp *pCard = &m_aCards[i];
        pCard->resetExceptValue();
        //*pCard = aShuffledIdValues[i];
        pCard->m_iId = aShuffledIdValues[i].m_iId;
        pCard->m_iValue = aShuffledIdValues[i].m_iValue;
    }
}
*/
