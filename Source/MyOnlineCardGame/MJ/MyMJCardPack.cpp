// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJCardPack.h"
#include "utils/CommonUtils/MyCardUtils.h"

void FMyMJCardPackCpp::reset(const TArray<int32> &aShuffledValues)
{
    int32 l = aShuffledValues.Num();

    m_aCards.Reset(l);
    for (int32 i = 0; i < l; i++) {
        int32 idx = m_aCards.Emplace();
        MY_VERIFY(i == idx);
        FMyMJCardCpp *pCard = &m_aCards[i];
        pCard->resetExceptValue();
        pCard->m_iId = i;
        pCard->m_iValue = aShuffledValues[i];
    }
}

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

void
FMyMJCardPackCpp::getValuesByIds(const TArray<int32> &aIds, TArray<int32> &outaValues) const
{
    outaValues.Reset();
    int32  l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyMJCardCpp *pCard = getCardByIdxConst(aIds[i]);
        outaValues.Emplace(pCard->m_iValue);
    }
}

void
FMyMJCardPackCpp::getIdValuePairsByIds(const TArray<int32> &aIds, TArray<FMyIdValuePair> &outaIdValuePairs) const
{
    outaIdValuePairs.Reset();
    int32  l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyMJCardCpp *pCard = getCardByIdxConst(aIds[i]);
        int32 idx = outaIdValuePairs.Emplace();
        outaIdValuePairs[idx] = *pCard;
    }
}

const FMyMJCardCpp*
FMyMJCardPackCpp::getCardByIdxConst(int32 idx) const
{
    int32 l = m_aCards.Num();
    if (idx >= 0 && idx < l) {
        return &m_aCards[idx];
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idx out of range: %d/%d"), idx, l);
        MY_VERIFY(false);
        return NULL;
    }
}

void FMyMJCardPackCpp::revealCardValue(int32 id, int32 value)
{
    if (value > 0) {

        FMyMJCardCpp *pCard = getCardByIdx(id);
        pCard->revealValue(value);

    }
}

void FMyMJCardPackCpp::helperVerifyValues()
{
    int32 l = m_aCards.Num();
    MY_VERIFY(l > 0);

    TMap<int32, int32> mValueCount;
    for (int i = 0; i < l; i++) {
        const FMyMJCardCpp *pCard = &m_aCards[i];
        MY_VERIFY(pCard->m_iId == i);

        if (pCard->m_iValue <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid card %s"), *pCard->genDebugStr());
            MY_VERIFY(false);
        }

        int32 &c = mValueCount.FindOrAdd(pCard->m_iValue);
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

void FMyMJCardPackCpp::helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs)
{
    int32 l = aIds.Num();

    aIdValuePairs.Reset(l);

    for (int32 i = 0; i < l; i++) {
        int32 idx = aIdValuePairs.Emplace();
        FMyIdValuePair &pair = aIdValuePairs[idx];
        pair.m_iId = aIds[i];
        pair.m_iValue = getCardByIdx(pair.m_iId)->m_iValue;

        if (!(pair.m_iId >= 0 && pair.m_iValue > 0)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid id and value pair: %d:%d"), pair.m_iId, pair.m_iValue);
            MY_VERIFY(false);
        }
    }

}

void FMyMJCardPackCpp::helperResolveValue(FMyIdValuePair &pair)
{
    int32 value = getCardByIdx(pair.m_iId)->m_iValue;
    pair.revealValue(value);
}

void FMyMJCardPackCpp::helperResolveValues(TArray<FMyIdValuePair> &aPairs)
{
    int32 l = aPairs.Num();
    for (int32 i = 0; i < l; i++) {
        helperResolveValue(aPairs[i]);
    }
}

void FMyMJCardPackCpp::helperInsertCardUniqueToIdArrayWithMinorPosiCalced(FMyMJCardPackCpp &cardPack, TArray<int32> &aIds, int32 id)
{
    int32 idx = aIds.AddUnique(id);
    MY_VERIFY(idx >= 0);

    FMyMJCardCpp *pCard = cardPack.getCardByIdx(id);
    pCard->m_cPosi.resetMinorData();
    pCard->m_cPosi.m_iIdxInSlot0 = idx;
}

bool FMyMJCardPackCpp::helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(FMyMJCardPackCpp &cardPack, TArray<int32> &aIds, int32 id)
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

        helperRecalcMinorPosiIfCardsInIdArray(cardPack, aIds);
        ret = true;
    }

    FMyMJCardCpp *pCard = cardPack.getCardByIdx(id);
    pCard->m_cPosi.resetMinorData();

    return ret;
}

void FMyMJCardPackCpp::helperRecalcMinorPosiIfCardsInIdArray(FMyMJCardPackCpp &cardPack, const TArray<int32> &aIds)
{
    int32 l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJCardCpp *pCard = cardPack.getCardByIdx(aIds[i]);
        pCard->m_cPosi.m_iIdxInSlot0 = i;
        pCard->m_cPosi.m_iIdxInSlot1 = 0;
    }
}