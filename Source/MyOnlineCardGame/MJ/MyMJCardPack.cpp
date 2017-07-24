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

void FMyMJCardInfoPackCpp::helperInsertCardUniqueToIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id)
{
    int32 idx = aIds.AddUnique(id);
    MY_VERIFY(idx >= 0);

    FMyMJCardInfoCpp *pInfo = getByIdx(id);
    pInfo->m_cPosi.resetMinorData();
    pInfo->m_cPosi.m_iIdxInSlot0 = idx;
}

bool FMyMJCardInfoPackCpp::helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id)
{
    bool ret = false;
    if (aIds.Top() == id) {
        aIds.Pop();
    }
    else {
        int32 l = aIds.Remove(id);
        if (l != 1) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("removed card is not 1: %d"), l);
            MY_VERIFY(false);
        }

        helperRecalcMinorPosiIfCardsInIdArray(aIds);
        ret = true;
    }

    FMyMJCardInfoCpp *pInfo = getByIdx(id);
    pInfo->m_cPosi.resetMinorData();

    return ret;
}

void FMyMJCardInfoPackCpp::helperRecalcMinorPosiIfCardsInIdArray(const TArray<int32> &aIds)
{
    int32 l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJCardInfoCpp *pInfo = getByIdx(aIds[i]);
        pInfo->m_cPosi.m_iIdxInSlot0 = i;
        pInfo->m_cPosi.m_iIdxInSlot1 = 0;
    }
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


void FMyMJCardValuePackCpp::helperVerifyValues() const
{
    int32 l = m_aCardValues.Num();
    MY_VERIFY(l > 0);

    TMap<int32, int32> mValueCount;
    for (int i = 0; i < l; i++) {
        int32 value = getByIdx(i);

        if (value <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid card value, id %d, value %d."), i, value);
            MY_VERIFY(false);
        }

        int32 &c = mValueCount.FindOrAdd(value);
        c++;
    }

    for (auto It = mValueCount.CreateConstIterator(); It; ++It)
    {
        int32 cardValue = It.Key();
        int32 count = It.Value();

        MyMJCardValueTypeCpp eType = UMyMJUtilsLibrary::getCardValueType(cardValue);
        if (eType == MyMJCardValueTypeCpp::WangZi || eType == MyMJCardValueTypeCpp::BingZi || eType == MyMJCardValueTypeCpp::TiaoZi || eType == MyMJCardValueTypeCpp::Feng || eType == MyMJCardValueTypeCpp::Jian) {
            if (count != 4) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected cardValue %d, count %d."), cardValue, count);
                MY_VERIFY(false);
            }
        }
        else if (eType == MyMJCardValueTypeCpp::Hua) {
            if (count != 1) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected cardValue %d, count %d."), cardValue, count);
                MY_VERIFY(false);
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid cardValue %d, count %d."), cardValue, count);
            MY_VERIFY(false);
        }

    }
}

void FMyMJCardValuePackCpp::revealCardValue(int32 id, int32 value)
{
    if (value > 0) {
        int32 &destV = getByIdxRef(id);
        bool bOk = FMyIdValuePair::helperTryRevealValue(destV, value);
        if (!bOk) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("revealCardValue failed: id %d, destV %d, value %d."), id, destV, value);
            MY_VERIFY(false);
        }

    }
}

void
FMyMJCardValuePackCpp::getValuesByIds(const TArray<int32> &aIds, TArray<int32> &outaValues) const
{
    outaValues.Reset();
    int32  l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        int32 v = getByIdx(aIds[i]);
        outaValues.Emplace(v);
    }
}

void
FMyMJCardValuePackCpp::getIdValuePairsByIds(const TArray<int32> &aIds, TArray<FMyIdValuePair> &outaIdValuePairs) const
{
    outaIdValuePairs.Reset();
    int32  l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        int32 id = aIds[i];
        int32 value = getByIdx(id);
        int32 idx = outaIdValuePairs.Emplace();
        outaIdValuePairs[idx].m_iId = id;
        outaIdValuePairs[idx].m_iValue = value;
    }
}


void FMyMJCardValuePackCpp::helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs)
{
    int32 l = aIds.Num();

    aIdValuePairs.Reset(l);

    for (int32 i = 0; i < l; i++) {
        int32 idx = aIdValuePairs.Emplace();
        FMyIdValuePair &pair = aIdValuePairs[idx];
        pair.m_iId = aIds[i];
        pair.m_iValue = getByIdx(pair.m_iId);

        if (!(pair.m_iId >= 0 && pair.m_iValue > 0)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid id and value pair: %d:%d"), pair.m_iId, pair.m_iValue);
            MY_VERIFY(false);
        }
    }

}

void FMyMJCardValuePackCpp::helperResolveValue(FMyIdValuePair &pair)
{
    int32 value = getByIdx(pair.m_iId);
    pair.revealValue(value);
}

void FMyMJCardValuePackCpp::helperResolveValues(TArray<FMyIdValuePair> &aPairs)
{
    int32 l = aPairs.Num();
    for (int32 i = 0; i < l; i++) {
        helperResolveValue(aPairs[i]);
    }
}
