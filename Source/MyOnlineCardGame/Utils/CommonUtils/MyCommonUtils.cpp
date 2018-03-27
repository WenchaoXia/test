// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCommonUtils.h"

DEFINE_LOG_CATEGORY(LogMyUtilsI);

void
FMyValueIdMapCpp::clear()
{
    m_mValueMap.Reset();
    m_iCount = 0;
    invalidCaches();
}

bool
FMyValueIdMapCpp::insert(int32 id, int32 value)
{
    /*
    if (m_iDebugLevel) {
        int32 debugCheckingIdx = 36;
        if (id == debugCheckingIdx) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("insert [%d: %d]."), id, value);
        }
    }
    */

    bool bNeedSort = false;
    FMyIdCollectionCpp* pMapElem;
    if (m_iKeepOrder > 0) {
        pMapElem = m_mValueMap.Find(value);
        if (pMapElem == NULL) {
            //pMapElem = &m_mValueMap.Add(value);
            pMapElem = &m_mValueMap.FindOrAdd(value);

            bNeedSort = true;
        }
    }
    else {
        pMapElem = &m_mValueMap.FindOrAdd(value);
    }

    bool bExist = pMapElem->m_aIds.Contains(id);
    if (!bExist) {
        pMapElem->m_aIds.Add(id);
        m_iCount++;
    }

    if (bNeedSort) {
        sortByValue((m_iKeepOrder - 1) > 0);
    }
    invalidCaches();

    return !bExist;
}

bool
FMyValueIdMapCpp::insert(const FMyIdValuePair &pair)
{
    return insert(pair.m_iId, pair.m_iValue);
}

bool
FMyValueIdMapCpp::remove(int32 id, int32 value)
{
    FMyIdCollectionCpp* pMapElem = m_mValueMap.Find(value);
    if (pMapElem == NULL) {
        return false;
    }

    //pMapElem->m_aIds.RemoveSwap(id);
    
    int32 idx;
    if (pMapElem->m_aIds.Find(id, idx)) {

        pMapElem->m_aIds.RemoveAtSwap(idx);
        if (pMapElem->m_aIds.Num() <= 0) {
            m_mValueMap.Remove(value);
            if (m_iKeepOrder > 0) {
                sortByValue((m_iKeepOrder - 1) > 0);
            }
        }
        m_iCount--;
        invalidCaches();
        return true;
    }
    else {
        return false;
    }

}

bool FMyValueIdMapCpp::insertIdValuePairs(const TArray<FMyIdValuePair>& aPairs, bool bAssertValueValid)
{
    bool bNoDup = true;
    int32 l = aPairs.Num();
    for (int32 i = 0; i < l; i++) {
        if (bAssertValueValid) {
            MY_VERIFY(aPairs[i].m_iValue > 0);
        }
        if (!insert(aPairs[i].m_iId, aPairs[i].m_iValue)) {
            bNoDup = false;
        }
    }

    return bNoDup;
}

void FMyValueIdMapCpp::insertIdValuePairsAssertNoDup(const TArray<FMyIdValuePair>& aPairs)
{
    MY_VERIFY(insertIdValuePairs(aPairs));
}



void FMyValueIdMapCpp::removeAllByValue(int32 value, TArray<FMyIdValuePair>& outPairs)
{
    outPairs.Reset();

    FMyIdCollectionCpp* pMapElem = m_mValueMap.Find(value);
    if (pMapElem == NULL) {
        return;
    }

    int32 lRemove = pMapElem->m_aIds.Num();
    for (int32 i = 0; i < lRemove; i++) {
        int32 idx = outPairs.Emplace();
        outPairs[idx].m_iId = pMapElem->m_aIds[i];
        outPairs[idx].m_iValue = value;
    }

    MY_VERIFY(m_mValueMap.Remove(value) == 1);
    m_iCount -= lRemove;
    invalidCaches();

    if (m_iKeepOrder > 0) {
        sortByValue((m_iKeepOrder - 1) > 0);
    }
}

void
FMyValueIdMapCpp::copyDeep(const FMyValueIdMapCpp *other)
{
    clear();
    this->m_mValueMap = other->m_mValueMap;
    this->m_iCount = other->m_iCount;
    this->m_iKeepOrder = other->m_iKeepOrder;
    this->m_aIdsAllCached = other->m_aIdsAllCached;

}

void
FMyValueIdMapCpp::collectByValue(int32 value, int32 numReq, TArray<int32>& outIds) const
{
    outIds.Reset();

    const FMyIdCollectionCpp* pMapElem = m_mValueMap.Find(value);
    if (pMapElem == NULL) {
        return;
    }


    if (numReq <= 0) {
        outIds = pMapElem->m_aIds;
    }
    else {
        int32 lMax = pMapElem->m_aIds.Num();
        if (lMax > numReq) {
            lMax = numReq;
        }

        for (int i = 0; i < lMax; i++) {
            outIds.Add(pMapElem->m_aIds[i]);
        }
    }

    return;
}

void
FMyValueIdMapCpp::collectByValueWithValue(int32 value, int32 numReq, TArray<FMyIdValuePair>& outPairs) const
{
    TArray<int32> aIds;
    collectByValue(value, numReq, aIds);

    outPairs.Reset();
    int32 l = aIds.Num();
    for (int32 i = 0; i < l; i++) {
        int32 idx = outPairs.Emplace();
        outPairs[idx].m_iId = aIds[i];
        outPairs[idx].m_iValue = value;
    }
}

void
FMyValueIdMapCpp::collectAll(TArray<int32>& outIds) const
{
    outIds.Reset();

    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        const FMyIdCollectionCpp* pMapElem = &It.Value();

        outIds.Append(pMapElem->m_aIds);

    }
}

const TArray<int32>& FMyValueIdMapCpp::getIdsAllCached()
{
    if (getCount() != m_aIdsAllCached.Num()) {
        collectAll(m_aIdsAllCached);
        MY_VERIFY(getCount() == m_aIdsAllCached.Num());
    }

    return m_aIdsAllCached;
}

void FMyValueIdMapCpp::collectAllWithValue(TArray<FMyIdValuePair> &outPairs) const
{
    outPairs.Reset();

    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        int32 cardValue = It.Key();
        const FMyIdCollectionCpp* pMapElem = &It.Value();
        int32  l = pMapElem->m_aIds.Num();
        for (int32 i = 0; i < l; i++) {
            int32 cardId = pMapElem->m_aIds[i];
            int32 idx = outPairs.Emplace();
            outPairs[idx].m_iId = cardId;
            outPairs[idx].m_iValue = cardValue;
        }
    }

    if (getCount() != outPairs.Num()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("all collected but count not equal: %d, %d."), getCount(), outPairs.Num());
        MY_VERIFY(false);
    }
}

int32
FMyValueIdMapCpp::getCountByValue(int32 value) const
{
    const FMyIdCollectionCpp* pMapElem = m_mValueMap.Find(value);

    if (pMapElem == NULL) {
        return 0;
    }

    return pMapElem->m_aIds.Num();
}


bool
FMyValueIdMapCpp::contains(int32 id, int32 value) const
{
    const FMyIdCollectionCpp* pMapElem = m_mValueMap.Find(value);
    if (pMapElem == NULL) {
        return false;
    }

    return pMapElem->m_aIds.Contains(id);
}


bool
FMyValueIdMapCpp::getOneIdValue(int32 &outId, int32 &outValue) const
{
    bool bFound = false;
    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        const FMyIdCollectionCpp* pMapElem = &It.Value();

        if (pMapElem->m_aIds.Num() > 0) {
            outId = pMapElem->m_aIds.Top();
            outValue = It.Key();
            bFound = true;
            break;
        }

    }

    return bFound;
}

void
FMyValueIdMapCpp::keys(TArray<int32>& outKeys) const
{
    m_mValueMap.GenerateKeyArray(outKeys);
}

void FMyValueIdMapCpp::sortByValue(bool bBig2Little)
{
    if (m_iDebugLevel > 0) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s: bBig2Little %d, before sort: %s."), *m_sDebugString, bBig2Little, *dump());
    }

    if (bBig2Little) {
        m_mValueMap.KeySort([](int32 v0, int32 v1) {
            return  v0 > v1;
        });
    }
    else {
        m_mValueMap.KeySort([](int32 v0, int32 v1) {
            return  v0 < v1;
        });
    }

    if (m_iDebugLevel > 0) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s: bBig2Little %d, after sort: %s."), *m_sDebugString, bBig2Little, *dump());
    }
}

FString FMyValueIdMapCpp::dump() const
{
    FString ret = FString::Printf(TEXT("%d elems: "), getCount());
    ret += TEXT("[");

    int32 debugIdCount = 0;
    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        int32 cardValue = It.Key();
        const FMyIdCollectionCpp* pMapElem = &It.Value();
        int32  l = pMapElem->m_aIds.Num();
        for (int32 i = 0; i < l; i++) {
            int32 cardId = pMapElem->m_aIds[i];
            ret += FString::Printf(TEXT("(%d,%d)"), cardId, cardValue);
            debugIdCount++;
        }
    }

    ret += TEXT("]");

    if (getCount() != debugIdCount || (m_aIdsAllCached.Num() > 0 && debugIdCount != m_aIdsAllCached.Num())) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("count not equal: count %d, cached %d, real %d"), getCount(), m_aIdsAllCached.Num(), debugIdCount);
        MY_VERIFY(false);
    }

    return ret;
}

FThreadSafeCounter FMyRunnableBaseCpp::s_iThreadCount = 0;


FMyIntVector2D const FMyIntVector2D::UnitValue = FMyIntVector2D(1, 1);