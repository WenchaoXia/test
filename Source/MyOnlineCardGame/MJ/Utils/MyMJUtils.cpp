// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJUtils.h"
#include "Utils/CommonUtils/MyCommonDefines.h"
#include "Kismet/KismetMathLibrary.h"


FString FMyMJCardPosiCpp::genDebugMsg() const
{
    return FString::Printf(TEXT("[%d, %s, %d, %d]"), m_iIdxAttender, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJCardSlotTypeCpp"), (uint8)m_eSlot), m_iIdxInSlot0, m_iIdxInSlot1);
};

FString FMyMJCardInfoCpp::genDebugMsg() const
{
    return FString::Printf(TEXT("[id %d, flip %s]"), m_iId, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyCardGameBoxLikeElemFlipStateCpp"), (uint8)m_eFlipState)) + m_cPosi.genDebugMsg();
};

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

void FMyMJCardValuePackCpp::tryRevealCardValue(int32 id, int32 value)
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
FMyMJCardValuePackCpp::getValuesByIds(const TArray<int32> &aIds, TArray<int32> &outaValues, bool bVerifyValueValid) const
{

    int32  l = aIds.Num();
    outaValues.Reset(l);

    for (int32 i = 0; i < l; i++) {
        int32 v = getByIdx(aIds[i]);
        outaValues.Emplace(v);

        if (bVerifyValueValid) {
            if (v <= 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid id and value pair: %d:%d"), aIds[i], v);
                MY_VERIFY(false);
            }
        }
    }
}

void FMyMJCardValuePackCpp::helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs, bool bVerifyValueValid) const
{
    int32 l = aIds.Num();

    aIdValuePairs.Reset(l);

    for (int32 i = 0; i < l; i++) {
        int32 idx = aIdValuePairs.Emplace();
        FMyIdValuePair &pair = aIdValuePairs[idx];
        pair.m_iId = aIds[i];
        pair.m_iValue = getByIdx(pair.m_iId);

        if (bVerifyValueValid) {
            if (!(pair.m_iId >= 0 && pair.m_iValue > 0)) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid id and value pair: %d:%d"), pair.m_iId, pair.m_iValue);
                MY_VERIFY(false);
            }
        }
    }

}

void FMyMJCardValuePackCpp::helperResolveValue(FMyIdValuePair &pair, bool bVerifyValueValid) const
{
    int32 value = getByIdx(pair.m_iId);
    pair.revealValue(value);

    if (bVerifyValueValid) {
        if (value <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid id and value pair: %d:%d"), pair.m_iId, value);
            MY_VERIFY(false);
        }
    }
}

void FMyMJCardValuePackCpp::helperResolveValues(TArray<FMyIdValuePair> &aPairs, bool bVerifyValueValid) const
{
    int32 l = aPairs.Num();
    for (int32 i = 0; i < l; i++) {
        helperResolveValue(aPairs[i], bVerifyValueValid);
    }
}


void FMyMJWeaveCpp::addCard(int32 id)
{
    int32 idx = m_aIds.AddUnique(id);

    if (idx < 0) {

        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("id not unique, id %d, now array: %s ."), id, *UMyCommonUtilsLibrary::formatStrIds(m_aIds));
        MY_VERIFY(false);
    }

};

FString FMyMJWeaveCpp::ToString() const
{
    FString ret = FString::Printf(TEXT(" m_eType: %s, m_iIdTriggerCard %d, m_iIdxAttenderTriggerCardSrc %d, m_eTypeConsumed: %s, m_bGangBuZhangLocalCS: %d."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJWeaveTypeCpp"), (uint8)m_eType), m_iIdTriggerCard, m_iIdxAttenderTriggerCardSrc, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJWeaveTypeCpp"), (uint8)m_eTypeConsumed), m_bGangBuZhangLocalCS);

    //ret += UMyCommonUtilsLibrary::formatStrIdValuePairs(m_aIdValues);

    return ret;
};

void FMyMJWeaveCpp::getIdsSortedWithTriggerCardRule(int32 idxAtttenderThisWeaveBelong, TArray<int32>& outaIds) const
{
    int32 idTriggerCard = getIdTriggerCard();
    if (idTriggerCard >= 0 && getIdxAttenderTriggerCardSrc() >= 0 && getIdxAttenderTriggerCardSrc() != idxAtttenderThisWeaveBelong) {
        //sort it
        TArray<int32> aIdsWithoutTriggerCard;
        int32 l0 = m_aIds.Num();
        for (int32 j = 0; j < l0; j++) {
            if (m_aIds[j] == idTriggerCard) {

            }
            else {
                aIdsWithoutTriggerCard.Emplace(m_aIds[j]);
            }
        }

        MY_VERIFY((l0 - 1) == aIdsWithoutTriggerCard.Num());

        //the supposed alignment is left to right
        outaIds.Reset();
        if (getIdxAttenderTriggerCardSrc() == ((idxAtttenderThisWeaveBelong + 1) % 4)) {
            outaIds.Append(aIdsWithoutTriggerCard);
            outaIds.Emplace(idTriggerCard);
        }
        else if (getIdxAttenderTriggerCardSrc() == ((idxAtttenderThisWeaveBelong + 2) % 4)) {
            MY_VERIFY(aIdsWithoutTriggerCard.Num() > 0);
            outaIds.Emplace(aIdsWithoutTriggerCard[0]);
            outaIds.Emplace(idTriggerCard);
            for (int32 i = 1; i < aIdsWithoutTriggerCard.Num(); i++) {
                outaIds.Emplace(aIdsWithoutTriggerCard[1]);
            }
        }
        else if (getIdxAttenderTriggerCardSrc() == ((idxAtttenderThisWeaveBelong + 3) % 4)) {
            outaIds.Emplace(idTriggerCard);
            outaIds.Append(aIdsWithoutTriggerCard);
        }
        else {
            MY_VERIFY(false);
        }
    }
    else {
        if (&outaIds != &m_aIds) {
            outaIds = m_aIds;
        }
    }
};


FString FMyMJHuScoreAttrCpp::ToString() const
{
    return FString::Printf(TEXT("(%s, %d)"), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJHuScoreTypeCpp"), (uint8)m_eType), m_iScorePerAttender);
};


FString FMyMJHuScoreResultBaseCpp::ToString() const
{
    //FString::Printf(TEXT(" m_aWeavesShowedOut.NUm(): %d. m_aWeavesInHand.Num: %d."), m_aWeavesShowedOut.Num(), m_aWeavesInHand.Num());
    FString ret = FString::Printf(TEXT(" m_iScorePerAttenderTotal: %d. "), m_iScorePerAttenderTotal);

    /*
    for (auto It = m_mScoreTypeItemMap.CreateConstIterator(); It; ++It)
    {
        MyMJHuScoreTypeCpp eType = It.Key();

        ret += UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJHuScoreTypeCpp"), (uint8)eType) + ", ";

    }
    */

    int32 l = m_aScoreResultItems.Num();
    for (int32 i = 0; i < l; i++) {
        ret += FString::Printf(TEXT("(%s, %d, %d), "), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJHuScoreTypeCpp"), (uint8)m_aScoreResultItems[i].m_eType), m_aScoreResultItems[i].m_iScorePerAttender, m_aScoreResultItems[i].m_iCount);
        //ret += UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJHuScoreTypeCpp"), (uint8)m_aScoreTypeItem[i].m_eType) + ", ";
    }

    return ret;
};



void FMyMJHuCfgCpp::prepareForUse()
{
    UMyMJUtilsLibrary::array2MapForHuScoreAttr(m_aHuScoreAttrs, m_mHuScoreAttrs);
};


const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp>& FMyMJHuCfgCpp::getHuScoreAttrsRefConst() const
{
    MY_VERIFY(UMyMJUtilsLibrary::checkUniformOfArrayAndMapForHuScoreAttr(m_aHuScoreAttrs, m_mHuScoreAttrs, false));
    return m_mHuScoreAttrs;
};


void
FMyMJWeaveTreeNodeCpp::appendChild(TSharedPtr<FMyMJWeaveTreeNodeCpp> &pChild)
{
    m_aChilds.Emplace(pChild);
}

void
FMyMJWeaveTreeNodeCpp::appendOtherChilds(FMyMJWeaveTreeNodeCpp *other)
{
    int32 l = other->m_aChilds.Num();

    for (int32 i = 0; i < l; i++) {
        m_aChilds.Emplace(other->m_aChilds[i]);
    }
}

void
FMyMJWeaveTreeNodeCpp::convert2WeaveArray(TArray<FMyMJWeaveArrayCpp> &outArray)
{
    outArray.Empty();
    TArray<FMyMJWeaveCpp> root;
    convert2WeaveArrayIn(root, outArray);
}

FMyMJWeaveCpp*
FMyMJWeaveTreeNodeCpp::getWeave()
{
    return &m_sWeave;
}

void
FMyMJWeaveTreeNodeCpp::convert2WeaveArrayIn(TArray<FMyMJWeaveCpp> parentWeaves, TArray<FMyMJWeaveArrayCpp> &outArray)
{
    if (m_sWeave.getType() != MyMJWeaveTypeCpp::Invalid) {
        parentWeaves.Emplace(m_sWeave);
    }

    int l = m_aChilds.Num();
    if (l > 0) {
        //it is in progress
        for (int i = 0; i < l; i++) {
            m_aChilds[i]->convert2WeaveArrayIn(parentWeaves, outArray);
        }

    }
    else {
        //this is the end node
        int32 idx = outArray.Emplace();
        FMyMJWeaveArrayCpp &item = outArray[idx];
        item.m_aWeaves = parentWeaves;
    }
}

void
FMyStatisCountsPerCardValueTypeCpp::addCountByValue(int32 value, int32 count)
{
    m_iCountTotal += count;

    TArray<MyMJCardValueTypeCpp> touchedTypes;
    collectTouchedTypesByValue(value, touchedTypes);

    //let's go extra
    int32 l = touchedTypes.Num();
    MyMJCardValueTypeCpp type;

    for (int32 i = 0; i < l; i++) {
        type = touchedTypes[i];
        MY_VERIFY((uint8)type >= 0 && (uint8)type < (uint8)MyMJCardValueTypeCpp::ExtMax);
        int32 &c = m_mCountPerCardType.FindOrAdd(type); //int32's default constructor we assert to be zero !
        c += count;
    }
    
}

const int32
FMyStatisCountsPerCardValueTypeCpp::getCountByCardValueType(MyMJCardValueTypeCpp type)
{
    const int32* pC = m_mCountPerCardType.Find(type);
    if (pC == NULL) {
        return 0;
    }

    return *pC;
}

const int32
FMyStatisCountsPerCardValueTypeCpp::getTotalCount()
{
    return m_iCountTotal;
}


void
FMyStatisCountsPerCardValueTypeCpp::collectTouchedTypesByValue(int32 value, TArray<MyMJCardValueTypeCpp> &outTouchedTypes)
{
    int32 modV10, modV2;

    outTouchedTypes.Empty();
    outTouchedTypes.Emplace(UMyMJUtilsLibrary::getCardValueType(value));

    //let's go extra
    if (value >= 1 && value < 30) {
        //let's direct calc, instead of calling UMyMJUtilsLibrary
        modV10 = value % 10;
        if (modV10 == 2 || modV10 == 5 || modV10 == 8) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNum258);
        }
        else if (modV10 == 1 || modV10 == 9) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNum19);
        }

        if (modV10 < 4) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNumLit);
        }
        else if (modV10 < 7) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNumMid);
        }
        else {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNumBig);
        }

        if (modV10 < 5) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNumLessThan5);
        }
        else if (modV10 > 5) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNumBiggerThan5);
        }

        modV2 = value % 2;
        if (modV2 == 0) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtNumDouble);
        }

        if (
            (value >= 11 && value < 16) || (value >= 18 && value < 20) || //BingZi
            value == 22 || (value >= 24 && value < 27) || (value >= 28 && value < 30) || //TIaoZi
            value == 43 //BaiBan
            ) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtBuDao);
        }

        if (
            (value >= 22 && value < 25) || value == 26 || value == 28 || //TIaoZi
            value == 42 //FaCai
            ) {
            outTouchedTypes.Emplace(MyMJCardValueTypeCpp::ExtLv);
        }
    }
}


int32
FMyMJCardParseResultCpp::getShunZiSameGroupCount(int32 minCount)
{
    return getCountOfValuesFromValueCountMap(m_mShunZiMidValueCountMap, minCount, true, true);
};

int32
FMyMJCardParseResultCpp::getShunZiStep1or2IncreaseMax()
{
    int32 conitinueCountMax = 0;

    TMap<int32, int32> *pMap = &m_mShunZiMidValueCountMap;

    pMap->KeySort([](int32 A, int32 B) {
        return A < B; // sort keys in asc
    });

    int32 c;

    c = getContinueIncreaseMaxFromValueCountMap(*pMap, 1, true, NULL, NULL);
    if (conitinueCountMax < c) {
        conitinueCountMax = c;
    }

    c = getContinueIncreaseMaxFromValueCountMap(*pMap, 2, true, NULL, NULL);
    if (conitinueCountMax < c) {
        conitinueCountMax = c;
    }

    return conitinueCountMax;
};

int32
FMyMJCardParseResultCpp::getShunZiModV10SameGroupCountRestrictDifferentType(int32 minCount, bool bMustEqual)
{
    TMap<int32, int32> modVCountMap;
    buildModV10CountMapFromValueCountMap(m_mShunZiMidValueCountMap, true, true, modVCountMap);

    return getCountOfValuesFromValueCountMap(modVCountMap, minCount, bMustEqual, true);
}


int32
FMyMJCardParseResultCpp::getKeZiOrGangModV10SameGroupCountRestrictDifferentTypeAndNumCard(int32 minCount, bool bMustEqual)
{
    TMap<int32, int32> modVCountMap;
    buildModV10CountMapFromValueCountMap(m_mKeZiOrGangValueCountMap, true, true, modVCountMap);

    return getCountOfValuesFromValueCountMap(modVCountMap, minCount, bMustEqual, true);
};

int32
FMyMJCardParseResultCpp::getKeZiOrGangStep1IncreaseMaxRestrictNumCard()
{

    TMap<int32, int32> *pMap = &m_mKeZiOrGangValueCountMap;

    pMap->KeySort([](int32 A, int32 B) {
        return A < B; // sort keys in asc
    });

    return getContinueIncreaseMaxFromValueCountMap(*pMap, 1, true, NULL, NULL);

};

int32
FMyMJCardParseResultCpp::getCountSanSeSanJieGao()
{
    return getCountOfStep1WithDifferentCardTypes(&m_mKeZiOrGangValueCountMap);
};


int32
FMyMJCardParseResultCpp::getCountSanSeSanBuGao()
{
    return getCountOfStep1WithDifferentCardTypes(&m_mShunZiMidValueCountMap);
};



int32
FMyMJCardParseResultCpp::getLaoShaoFuCountByType(MyMJCardValueTypeCpp type)
{
    if (type == MyMJCardValueTypeCpp::WangZi || type == MyMJCardValueTypeCpp::BingZi || type == MyMJCardValueTypeCpp::TiaoZi) {
        int32 retMin = 10, c;
        c = m_cChiLit.getCountByCardValueType(type);
        if (retMin > c) {
            retMin = c;
        }
        c = m_cChiBig.getCountByCardValueType(type);
        if (retMin > c) {
            retMin = c;
        }

        return retMin;
    }
    else {
        return 0;
    }

};


int32
FMyMJCardParseResultCpp::getCountLianLiu()
{
    TMap<int32, int32> *pMap = &m_mShunZiMidValueCountMap;

    pMap->KeySort([](int32 A, int32 B) {
        return A < B; // sort keys in asc
    });

    int32 minC = 0;
    int32 min = 2;
    getContinueIncreaseMaxFromValueCountMap(*pMap, 3, true, &min, &minC);

    return minC;
};

int32
FMyMJCardParseResultCpp::getCountYaoJiuKe()
{
    int32 ret = 0;
    for (auto It = m_mKeZiOrGangValueCountMap.CreateConstIterator(); It; ++It)
    {
        int32 v = It.Key();
        int32 c = It.Value();

        if (UMyMJUtilsLibrary::isCardValueYao(v)) {
            ret += c;
        }
    }

    return ret;
}

bool
FMyMJCardParseResultCpp::findLian7Dui()
{
    return (getContinueIncreaseMaxFromValueCountMap(m_mDuiZiOrGangValueCountMap, 1, true, NULL, NULL) >= 7);
}


MyMJCardValueTypeCpp
FMyMJCardParseResultCpp::findQingLong()
{

    int32 baseT = (uint8)MyMJCardValueTypeCpp::WangZi;
    int32 rangeT = (uint8)MyMJCardValueTypeCpp::TiaoZi - baseT + 1;
    for (int32 i = baseT; i < rangeT; i++) {
        if (m_cChiLit.getCountByCardValueType((MyMJCardValueTypeCpp)i) >= 1 &&
            m_cChiMid.getCountByCardValueType((MyMJCardValueTypeCpp)i) >= 1 &&
            m_cChiBig.getCountByCardValueType((MyMJCardValueTypeCpp)i) >= 1) {
            return (MyMJCardValueTypeCpp)i;
        }
    }

    return MyMJCardValueTypeCpp::Invalid;
};


bool
FMyMJCardParseResultCpp::findHuaLong()
{

    int32 baseT = (uint8)MyMJCardValueTypeCpp::WangZi;
    int32 rangeT = (uint8)MyMJCardValueTypeCpp::TiaoZi - baseT + 1;
    for (int32 i = baseT; i < rangeT; i++) {
        //find head
        if (m_cChiLit.getCountByCardValueType((MyMJCardValueTypeCpp)i) >= 1) {
            for (int32 j = baseT; j < rangeT; j++) {
                //find mid
                if (i != j && m_cChiMid.getCountByCardValueType((MyMJCardValueTypeCpp)j) >= 1) {
                    for (int32 k = baseT; k < rangeT; k++) {
                        if (i != k && j != k && m_cChiBig.getCountByCardValueType((MyMJCardValueTypeCpp)k) >= 1) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
};


void
FMyMJCardParseResultCpp::buildModV10CountMapFromValueCountMap(TMap<int32, int32> &inValueCountMap, bool bRestrictNumCardValue, bool bAlwaysCountAsOne, TMap<int32, int32> &outModVCountMap)
{
    outModVCountMap.Empty();
    for (auto It = inValueCountMap.CreateConstIterator(); It; ++It)
    {
        int32 v = It.Key();
        int32 c = It.Value();
        MY_VERIFY(c >= 1);

        if (bRestrictNumCardValue) {
            if (v >= 1 && v < 30) {
            }
            else {
                continue;
            }
        }

        int32 modV = v % 10;
        int32 &c2 = outModVCountMap.FindOrAdd(modV);
        if (bAlwaysCountAsOne) {
            c2 += 1;
        }
        else {
            c2 += c;
        }

    }
};


int32
FMyMJCardParseResultCpp::getCountOfValuesFromValueCountMap(TMap<int32, int32> &inMap, int32 minCount, bool bMustEqual, bool bRestrictNumCardValue)
{
    int ret = 0;

    for (auto It = inMap.CreateConstIterator(); It; ++It)
    {
        int32 v = It.Key();
        int32 c = It.Value();

        if (bRestrictNumCardValue && (!(v >= 1 && v < 30))) {
            continue;
        }

        if (c < minCount || (bMustEqual && c != minCount)) {
            continue;
        }


        ret += c / minCount;

    }

    return ret;
}

int32
FMyMJCardParseResultCpp::getContinueIncreaseMaxFromValueCountMap(TMap<int32, int32> &inMap, int32 step, bool bRestrictNumCardValueAndBanCrossMod10, int32 *pInMin, int32 *pOutMinCount)
{
    int32 conitinueCountMax = 0;

    int32 lastV;
    int32 conitinueCount;

    lastV = -100;
    conitinueCount = 1;
    for (auto It = inMap.CreateConstIterator(); It; ++It)
    {
        int32 v = It.Key();
        int32 c = It.Value();

        if (bRestrictNumCardValueAndBanCrossMod10) {
            if (v >= 1 && v < 30) {
            }
            else {
                continue;
            }
        }


        if (bRestrictNumCardValueAndBanCrossMod10 && (v % 10) <= step) {
            lastV = -100;
        }

        if ((v - lastV) == step) {
            conitinueCount++;

            if (pInMin && pOutMinCount && *pInMin == conitinueCount) {
                *pOutMinCount++;
            }
        }
        else {
            conitinueCount = 1;
        }

        if (conitinueCount > conitinueCountMax) {
            conitinueCountMax = conitinueCount;
        }

        lastV = v;

    }

    return conitinueCountMax;
};


int32
FMyMJCardParseResultCpp::getCountOfStep1WithDifferentCardTypes(TMap<int32, int32> *pMap)
{
    int32 ret = 0;

    for (auto It = pMap->CreateConstIterator(); It; ++It)
    {
        int32 v = It.Key();
        int32 c = It.Value();
        MY_VERIFY(c >= 1);

        //test it
        int32 v1, v2, *c1, *c2;

        v1 = (v + 11) % 30;
        v2 = (v + 22) % 30;

        c1 = pMap->Find(v1);
        c2 = pMap->Find(v2);
        if (c1 && c2 && *c1 > 0 && *c2 > 0) {
            ret++;
        }

        v1 = (v + 21) % 30;
        v2 = (v + 12) % 30;

        c1 = pMap->Find(v1);
        c2 = pMap->Find(v2);
        if (c1 && c2 && *c1 > 0 && *c2 > 0) {
            ret++;
        }
    }

    return ret;
};





bool
FMyMJValueIdMapCpp::checkShunZi(MyMJChiTypeCpp type, int32 checkingValueInMap, const FMyIdValuePair *pExt, TArray<FMyIdValuePair> &outIdValuePairsInMap) const
{
    int checkingValue = checkingValueInMap;
    if (pExt) {
        checkingValue = pExt->m_iValue;
    }

    outIdValuePairsInMap.Reset();

    MyMJCardValueTypeCpp vType = UMyMJUtilsLibrary::getCardValueType(checkingValue);

    if (vType == MyMJCardValueTypeCpp::Invalid) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("checkingValue type is invalid: %d"), checkingValue);
        MY_VERIFY(false);
        return false;
    }

    if (!UMyMJUtilsLibrary::isCardTypeNumCard(vType)) {
        return false;
    }


    TArray<int> collectingValues;

    if (pExt) {
    }
    else {
        collectingValues.Emplace(checkingValue);
    }

    int modValue = checkingValue % 10;
    if (type == MyMJChiTypeCpp::ChiLeft) {
        if (modValue >= 8) {
            return false;
        }

        collectingValues.Emplace(checkingValue + 1);
        collectingValues.Emplace(checkingValue + 2);
    }
    else if (type == MyMJChiTypeCpp::ChiMid) {
        if (modValue >= 9 || modValue < 2) {
            return false;
        }

        collectingValues.Emplace(checkingValue - 1);
        collectingValues.Emplace(checkingValue + 1);
    }
    else if (type == MyMJChiTypeCpp::ChiRight) {
        if (modValue < 3) {
            return false;
        }

        collectingValues.Emplace(checkingValue - 2);
        collectingValues.Emplace(checkingValue - 1);
        //int32 t = type;
        //MyMJWeaveTypeCpp
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("chi type is invalid: %d"), (uint8)type);
        MY_VERIFY(false);
        return false;
    }

    //collecting the requirement
    TArray<int32> Ids;

    int l = collectingValues.Num();
    for (int32 i = 0; i < l; i++)
    {
        int v = collectingValues[i];
        collectByValue(v, 1, Ids);
        if (Ids.Num() < 1) {
            outIdValuePairsInMap.Reset();
            return false;
        }

        int32 idx = outIdValuePairsInMap.Emplace();
        outIdValuePairsInMap[idx].m_iId = Ids[0];
        outIdValuePairsInMap[idx].m_iValue = v;

    }

    return true;
}

bool FMyMJValueIdMapCpp::checkShunZi(MyMJChiTypeCpp type, int32 checkingValueInMap, const FMyIdValuePair *pExt, TArray<int32> &outIds, TArray<int32> &outValues) const
{
    TArray<FMyIdValuePair> aIdValuePairs;
    bool bRet = FMyMJValueIdMapCpp::checkShunZi(type, checkingValueInMap, pExt, aIdValuePairs);

    FMyIdValuePair::helperIdValuePairs2IdsValues(aIdValuePairs, &outIds, &outValues);

    return bRet;

}

/*
bool
FMyMJValueIdMapCpp::checkAllCanBeWeavedForHu(bool reqJiang258, bool allowJiangNot258ForLastDuiZi, bool weavesShowedMayPengPengHu, MyMJCardValueTypeCpp weaveShowedOutUnifiedType, int32 weaveShowedOutCount, bool findAllCombines, FMyMJWeaveTreeNodeCpp* outpParent) const
{
    FMyMJWeaveTreeNodeCpp rootNode;
    bool bCanWeave;

    if (reqJiang258) {
        //QingYiSe doesn't need Jiang258
        if (weaveShowedOutUnifiedType != MyMJCardValueTypeCpp::Invalid) {

            if (getUnifiedNumCardType() == weaveShowedOutUnifiedType) {
                reqJiang258 = false;
            }
        }
        else if (weaveShowedOutCount == 0) {
            if (getUnifiedNumCardType() != MyMJCardValueTypeCpp::Invalid) {
                reqJiang258 = false;
            }
        }
    }

    rootNode.reset();
    bCanWeave = FMyMJValueIdMapCpp::checkAllCanBeWeavedForHuIn(true, reqJiang258, true, findAllCombines, &rootNode);
    if (bCanWeave) {
        outpParent->appendOtherChilds(&rootNode);
        return true;
    }

    //still req258 jiang, and not hu yet, try again
    if (reqJiang258) {
        //dan diao jiang
        if (m_iCount <= 2 && allowJiangNot258ForLastDuiZi) {
            rootNode.reset();
            bCanWeave = FMyMJValueIdMapCpp::checkAllCanBeWeavedForHuIn(true, false, true, findAllCombines, &rootNode);
            if (bCanWeave) {
                outpParent->appendOtherChilds(&rootNode);
                return true;
            }
        }

        //PengPengHu
        if (weavesShowedMayPengPengHu) {
            rootNode.reset();
            bCanWeave = FMyMJValueIdMapCpp::checkAllCanBeWeavedForHuIn(true, false, false, findAllCombines, &rootNode);
            if (bCanWeave) {
                outpParent->appendOtherChilds(&rootNode);
                return true;
            }
        }
    }


    return false;
}
*/

bool
FMyMJValueIdMapCpp::checkAllCanBeWeavedForHu(bool reqJiang258, bool allowShunZi, bool findAllCombines, FMyMJWeaveTreeNodeCpp* outpParent) const
{
    FMyMJWeaveTreeNodeCpp rootNode;
    bool bCanWeave;

    rootNode.reset();
    bCanWeave = FMyMJValueIdMapCpp::checkAllCanBeWeavedForHuIn(true, reqJiang258, allowShunZi, findAllCombines, &rootNode);
    if (bCanWeave) {
        outpParent->appendOtherChilds(&rootNode);
        return true;
    }

    return false;
}

bool
FMyMJValueIdMapCpp::checkAllCanBeWeavedForHuIn(bool allowJiang, bool reqJiang258, bool allowShunZi, bool bFindAllWeaves, FMyMJWeaveTreeNodeCpp *outpParent) const
{

    int32 pengDupReq = 3;
    int32 totalCount = getCount(); //virtual card count all, including extra one if specified

    int32 checkingCardValue = 0;

    int32 id;
    if (false == getOneIdValue(id, checkingCardValue)) {
        return true;
    }

    if (checkingCardValue <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("checkingCardValue <= 0: %d"), checkingCardValue);
        MY_VERIFY(false);
        return false;
    }

    TArray<int32> Ids, Values;;
    TSharedPtr<FMyMJWeaveTreeNodeCpp> pElem;
    FMyMJWeaveCpp *pWeave;
    FMyMJValueIdMapCpp newMap;
    bool OK2Add;
    bool ret = false;

    //1st, let's try check Peng
    Ids.Empty();
    collectByValue(checkingCardValue, pengDupReq, Ids);
    if (Ids.Num() == pengDupReq) {
        pElem = MakeShareable(new FMyMJWeaveTreeNodeCpp());
        pWeave = pElem->getWeave();

        pWeave->getTypeRef() = MyMJWeaveTypeCpp::KeZiAn;

        for (int i = 0; i < pengDupReq; i++) {
            //pWeave->addCard(Ids[i], checkingCardValue);
            pWeave->addCard(Ids[i]);
        }

        if (totalCount <= 3) {
            //this is the last one
            OK2Add = true;
        }
        else {
            newMap.clear();
            newMap.copyDeep(this);
            for (int i = 0; i < pengDupReq; i++) {
                newMap.remove(Ids[i], checkingCardValue);
            }

            OK2Add = newMap.checkAllCanBeWeavedForHuIn(allowJiang, reqJiang258, allowShunZi, bFindAllWeaves, pElem.Get());
        }

        if (OK2Add) {
            outpParent->appendChild(pElem);
            ret = true;

            if (bFindAllWeaves == false) {
                return true;
            }
        }
    }

    bool bCanFormJiang = false;
    if (Ids.Num() >= (pengDupReq - 1) && allowJiang) {
        if (reqJiang258) {
            if (UMyMJUtilsLibrary::isCardValue258(checkingCardValue) == true) {
                bCanFormJiang = true;
            }
        }
        else {
            bCanFormJiang = true;
        }
    }

    if (bCanFormJiang) {
        pElem = MakeShareable(new FMyMJWeaveTreeNodeCpp());
        pWeave = pElem->getWeave();

        pWeave->getTypeRef() = MyMJWeaveTypeCpp::DuiZi;

        for (int i = 0; i < (pengDupReq - 1); i++) {
            //pWeave->addCard(Ids[i], checkingCardValue);
            pWeave->addCard(Ids[i]);
        }

        if (totalCount <= 2) {
            //this is the last one
            OK2Add = true;
        }
        else {
            newMap.clear();
            newMap.copyDeep(this);
            for (int i = 0; i < (pengDupReq - 1); i++) {
                newMap.remove(Ids[i], checkingCardValue);
            }

            OK2Add = newMap.checkAllCanBeWeavedForHuIn(false, reqJiang258, allowShunZi, bFindAllWeaves, pElem.Get());
        }

        if (OK2Add) {
            outpParent->appendChild(pElem);
            ret = true;

            if (bFindAllWeaves == false) {
                return true;
            }
        }
    }


    if (allowShunZi) {
        MyMJChiTypeCpp chiTypes[3];
        chiTypes[0] = MyMJChiTypeCpp::ChiLeft;
        chiTypes[1] = MyMJChiTypeCpp::ChiMid;
        chiTypes[2] = MyMJChiTypeCpp::ChiRight;

        for (int i = 0; i < 3; i++) {
            pElem = MakeShareable(new FMyMJWeaveTreeNodeCpp());
            pWeave = pElem->getWeave();

            pWeave->getTypeRef() = MyMJWeaveTypeCpp::ShunZiAn;

            OK2Add = checkShunZi(chiTypes[i], checkingCardValue, NULL, Ids, Values);
            if (OK2Add) {
                //pWeave->appendIdsValues(Ids, Values);
                pWeave->appendIds(Ids);

                if (totalCount <= 3) {
                    //this is the last one
                    OK2Add = true;
                }
                else {
                    newMap.clear();
                    newMap.copyDeep(this);
                    int l = Ids.Num();
                    for (int k = 0; k < l; k++) {
                        newMap.remove(Ids[k], Values[k]);
                    }

                    OK2Add = newMap.checkAllCanBeWeavedForHuIn(allowJiang, reqJiang258, allowShunZi, bFindAllWeaves, pElem.Get());
                }

                if (OK2Add) {
                    outpParent->appendChild(pElem);
                    ret = true;

                    if (bFindAllWeaves == false) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool
FMyMJValueIdMapCpp::checkSpecial13BuKaoIn(bool &outQiXingBuKao) const
{
    if (m_iCount != 14) {
        return false;
    }

    TArray<int32> values_369_147_258[3];
    TArray<int32> ids_369_147_258[3];

    TArray<int32> *pValueTargetArray;
    TArray<int32> *pIdTargetArray;

    int32 wordCardCount = 0;
    //TArray<int32> v258Values;
    //TArray<int32> v369Values;

    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        int32 cardValue = It.Key();
        const FMyIdCollectionCpp* pMapElem = &It.Value();
        int32 cardCount = pMapElem->m_aIds.Num();

        MY_VERIFY(cardCount > 0);

        if (cardCount > 1) {
            return false;
        }
        int32 cardId = pMapElem->m_aIds[0];


        MyMJCardValueTypeCpp vType = UMyMJUtilsLibrary::getCardValueType(cardValue);

        //let's check if value confirm condition
        if (vType == MyMJCardValueTypeCpp::WangZi || vType == MyMJCardValueTypeCpp::BingZi || vType == MyMJCardValueTypeCpp::TiaoZi) {
            int32 modV = cardValue % 10 % 3;
            if (modV == 0) {
                //3 6 9
            }
            else if (modV == 1) {
                //1 4 7
            }
            else {
                // 2 5 8
            }

            pValueTargetArray = &values_369_147_258[modV];
            pIdTargetArray = &ids_369_147_258[modV];
            
            if (pValueTargetArray->Num() <= 0) {
            }
            else {
                MyMJCardValueTypeCpp vTypeOld = UMyMJUtilsLibrary::getCardValueType((*pValueTargetArray)[0]);
                if (vTypeOld != vType) {
                    return false;
                }
            }

            //trick here, since they are same type, for example 1 4 7, and card value won't loop twice, they will always fill correctly
            pValueTargetArray->Emplace(cardValue);
            pIdTargetArray->Emplace(cardId);
        }
        else if (vType == MyMJCardValueTypeCpp::Feng || vType == MyMJCardValueTypeCpp::Jian) {
            wordCardCount++;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got an invalid valye: %d"), cardValue);
            MY_VERIFY(false);
        }
    }

    if (wordCardCount >= 7) {
        outQiXingBuKao = true;
    }
    else {
        outQiXingBuKao = false;
    }

    return true;
}

void
FMyMJValueIdMapCpp::fillInWeavesAssert7Dui(TArray<FMyMJWeaveCpp> &outWeaves) const
{
    outWeaves.Empty();

    TMap<int32, FMyIdCollectionCpp> *pMap, newMap;
    newMap = m_mValueMap;
    pMap = &newMap;

    pMap->KeySort([](int32 A, int32 B) {
        return A < B; // sort keys in asc
    });


    for (auto It = pMap->CreateConstIterator(); It; ++It)
    {
        const int32 &cardValue = It.Key();
        const FMyIdCollectionCpp* pMapElem = &It.Value();

        int32 l = pMapElem->m_aIds.Num();
        if (l != 2 && l != 4) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not duizi found : %d:%d"), cardValue, l);
            MY_VERIFY(false);
        }

        int32 duiZiCount = l / 2;
        for (int32 i = 0; i < duiZiCount; i++) {
            int32 idx = outWeaves.Emplace();
            FMyMJWeaveCpp *pWeave = &outWeaves[idx];
            pWeave->getTypeRef() = MyMJWeaveTypeCpp::DuiZi;
            //pWeave->addCard(pMapElem->m_aIds[i * 2], cardValue);
            //pWeave->addCard(pMapElem->m_aIds[i * 2 + 1], cardValue);

            pWeave->addCard(pMapElem->m_aIds[i * 2]);
            pWeave->addCard(pMapElem->m_aIds[i * 2 + 1]);
        }

    }
}

void
FMyMJValueIdMapCpp::fillInWeavesSpecialUnWeavedCards(TArray<FMyMJWeaveCpp> &outWeaves) const
{
    outWeaves.Empty();

    TMap<int32, FMyIdCollectionCpp> *pMap, newMap;
    newMap = m_mValueMap;
    pMap = &newMap;

    pMap->KeySort([](int32 A, int32 B) {
        return A < B; // sort keys in asc
    });


    int32 idx = outWeaves.Emplace();
    FMyMJWeaveCpp *pWeave = &outWeaves[idx];
    pWeave->getTypeRef() = MyMJWeaveTypeCpp::SpecialUnWeavedCards;

    for (auto It = pMap->CreateConstIterator(); It; ++It)
    {
        const int32 &cardValue = It.Key();
        const FMyIdCollectionCpp* pMapElem = &It.Value();

        int32 l = pMapElem->m_aIds.Num();

        for (int32 i = 0; i < l; i++) {

            //pWeave->addCard(pMapElem->m_aIds[i], cardValue);
            pWeave->addCard(pMapElem->m_aIds[i]);
        }

    }

}

bool
FMyMJValueIdMapCpp::checkSpecialZuHeLong(TArray<FMyMJWeaveArrayCpp> &outWeaveCombines) const
{
    //i means type, j means 369 147 258
    outWeaveCombines.Empty();

    int32 longs[3][3];
    for (int32 i = 0; i < 3; i++) {
        for (int32 j = 0; j < 3; j++) {
            longs[i][j] = 1;
        }
    }

    for (int32 i = 0; i < 3; i++) {
        //try 369
        for (int32 j = 1; j < 10; j++) {
            int32 v = i * 10 + j;

            int32 modV = v % 3;
            if (1 != getCountByValue(v)) {
                longs[i][modV] = 0;
            }
        }

        //Todo: optimize the speed here
    }

    FMyMJValueIdMapCpp newMap;
    FMyMJWeaveTreeNodeCpp rootNode;
    FMyMJWeaveCpp *pWeave;
    TArray<int32> Ids;
    TArray<FMyMJWeaveArrayCpp> weaveCombines;

    bool ret = false;


    //let's find out a complete longs
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j) {
                continue;
            }
            for (int k = 0; k < 3; k++) {
                if (i == k || j == k) {
                    continue;
                }

                if (longs[i][0] == 1 && longs[j][1] == 1 && longs[k][2] == 1) {
                    //found
                    rootNode.reset();
                    pWeave = rootNode.getWeave();
                    pWeave->getTypeRef() = MyMJWeaveTypeCpp::SpecialLong;

                    newMap.copyDeep(this);

                    for (int o = 0; o < 3; o++) {
                        int32 v;
                        int32 id;

                        v = i * 10 + 3 * o + 3;
                        collectByValue(v, 1, Ids);
                        if (Ids.Num() != 1) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected 369 card count: v %d:c %d"), v, Ids.Num());
                            MY_VERIFY(false);
                        }
                        id = Ids[0];
                        newMap.remove(id, v);
                        //pWeave->addCard(id, v);
                        pWeave->addCard(id);

                        v = j * 10 + 3 * o + 1;
                        collectByValue(v, 1, Ids);
                        if (Ids.Num() != 1) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected 147 card count: v %d:c %d"), v, Ids.Num());
                            MY_VERIFY(false);
                        }
                        id = Ids[0];
                        newMap.remove(id, v);
                        //pWeave->addCard(id, v);
                        pWeave->addCard(id);

                        v = k * 10 + 3 * o + 2;
                        collectByValue(v, 1, Ids);
                        if (Ids.Num() != 1) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected 258 card count: v %d:c %d"), v, Ids.Num());
                            MY_VERIFY(false);
                        }
                        id = Ids[0];
                        newMap.remove(id, v);
                        //pWeave->addCard(id, v);
                        pWeave->addCard(id);

                        bool bcanWeave = newMap.checkAllCanBeWeavedForHu(false, true, true, &rootNode);
                        if (bcanWeave) {
                            rootNode.convert2WeaveArray(weaveCombines);
                            ret = true;
                            outWeaveCombines.Append(weaveCombines);
                        }
                    }
                }
            }
        }
    }

    return ret;
}

MyMJCardValueTypeCpp
FMyMJValueIdMapCpp::getUnifiedNumCardType() const
{

    MyMJCardValueTypeCpp unifiedType = MyMJCardValueTypeCpp::Invalid;
    bool bFound = false;

    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        int32 v = It.Key();
        MyMJCardValueTypeCpp t = UMyMJUtilsLibrary::getCardValueType(v);
        if (t == MyMJCardValueTypeCpp::WangZi || t == MyMJCardValueTypeCpp::BingZi || t == MyMJCardValueTypeCpp::TiaoZi) {
            if (bFound) {
                if (unifiedType != t) {
                    unifiedType = MyMJCardValueTypeCpp::Invalid;
                    break;
                }
            }
            else {
                unifiedType = t;
                bFound = true;
            }
        }
        else {
            unifiedType = MyMJCardValueTypeCpp::Invalid;
            break;
        }
    }

    return unifiedType;
}

void
FMyMJValueIdMapCpp::parseCards(bool bComplexParse, FMyMJCardParseResultCpp &outResult) const
{
    outResult.reset();

    FMyMJCardParseResultSimpleCpp *pSimpleResult = &outResult;
    FMyMJCardParseResultCpp *pComplexResult = &outResult;

    const FMyIdCollectionCpp *pMapElem;
    int32 cardValue, cardCount;

    FMyStatisCountsPerCardValueTypeCpp *pStatisCounts;
    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        cardValue = It.Key();
        pMapElem = &It.Value();
        cardCount = pMapElem->m_aIds.Num();

        MY_VERIFY(cardCount > 0);

        pSimpleResult->m_cCardCounts.addCountByValue(cardValue, cardCount);

        int32 duiZiCount = 0;
        if (cardCount == 2) {
            pStatisCounts = &pSimpleResult->m_mWeaveCounts.FindOrAdd(MyMJWeaveTypeCpp::DuiZi);
            pStatisCounts->addCountByValue(cardValue, 1);
            duiZiCount = 1;
        }
        else if (cardCount == 3) {
            pStatisCounts = &pSimpleResult->m_mWeaveCounts.FindOrAdd(MyMJWeaveTypeCpp::KeZiAn);
            pStatisCounts->addCountByValue(cardValue, 1);
        }
        else if (cardCount == 4) {
            pStatisCounts = &pSimpleResult->m_mWeaveCounts.FindOrAdd(MyMJWeaveTypeCpp::GangAn);
            pStatisCounts->addCountByValue(cardValue, 1);
            duiZiCount = 2;
        }

        if (!bComplexParse) {
            continue;
        }

        if (duiZiCount > 0) {
            int32 &c = pComplexResult->m_mDuiZiOrGangValueCountMap.FindOrAdd(cardValue);
            c += duiZiCount;
        }
    }

}

int32
FMyMJValueIdMapCpp::getCountOfGroupSameCardValue(int32 minCount, bool bAlwaysCountAsOne) const
{
    MY_VERIFY(minCount > 0);
    int32 ret = 0;
    for (auto It = m_mValueMap.CreateConstIterator(); It; ++It)
    {
        const FMyIdCollectionCpp* pMapElem = &It.Value();

        int32 c = pMapElem->m_aIds.Num();
        if (c >= minCount) {
            if (bAlwaysCountAsOne) {
                ret += 1;
            }
            else {
                ret += c / minCount;
            }
        }

    }

    return ret;
}

void UMyMJUtilsLibrary::array2MapForHuScoreAttr(const TArray<FMyMJHuScoreAttrCpp>& aHuBornScoreAttrs, TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp>& mHuBornScoreAttrs)
{
    mHuBornScoreAttrs.Reset();
    int32 l = aHuBornScoreAttrs.Num();
    for (int32 i = 0; i < l; i++) {
        FMyMJHuScoreAttrCpp& attr = mHuBornScoreAttrs.FindOrAdd(aHuBornScoreAttrs[i].m_eType);
        attr = aHuBornScoreAttrs[i];
    }

    MY_VERIFY(checkUniformOfArrayAndMapForHuScoreAttr(aHuBornScoreAttrs, mHuBornScoreAttrs, true));
}

bool UMyMJUtilsLibrary::checkUniformOfArrayAndMapForHuScoreAttr(const TArray<FMyMJHuScoreAttrCpp>& aHuBornScoreAttrs, const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp>& mHuBornScoreAttrs, bool bComplexCheck)
{
    int32 l = aHuBornScoreAttrs.Num();

    if (l != mHuBornScoreAttrs.Num()) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("array and map's num not equal: %d, %d"), l, mHuBornScoreAttrs.Num());
        return false;
    }

    if (bComplexCheck == false) {
        return true;
    }

    for (int32 i = 0; i < l; i++) {
        const FMyMJHuScoreAttrCpp* pAttrOfArray = &aHuBornScoreAttrs[i];
        const FMyMJHuScoreAttrCpp* pAttrOfMap = mHuBornScoreAttrs.Find(pAttrOfArray->m_eType);
        if (pAttrOfMap == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("array elem %s not found in map."), *pAttrOfArray->ToString());
            return false;
        }

        if (!pAttrOfArray->equal(*pAttrOfMap)) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("array and map elem not equal: %s, %s."), *pAttrOfArray->ToString(), *pAttrOfMap->ToString());
            return false;
        }
    }

    return true;
}

bool UMyMJUtilsLibrary::removeCardByIdInWeaves(TArray<FMyMJWeaveCpp> &inWeaves, int32 id, bool *pOutWeaveDeleted)
{
    bool bFound = false;
    if (pOutWeaveDeleted) {
        *pOutWeaveDeleted = false;
    }

    int32 l1 = inWeaves.Num();
    for (int32 i = 0; i < l1; i++) {
        bFound = inWeaves[i].removeById(id);

        if (bFound) {
            if (inWeaves[i].getCount() <= 0) {
                inWeaves.RemoveAt(i);
                if (pOutWeaveDeleted) {
                    *pOutWeaveDeleted = true;
                }
            }
            break;
        }
    }

    return bFound;
}

void UMyMJUtilsLibrary::resetCardValueInIdValuePairs(TArray<FMyIdValuePair> &m_aIdValuePairs)
{
    int32 l = m_aIdValuePairs.Num();
    for (int32 i = 0; i < l; i++) {
        m_aIdValuePairs[i].m_iValue = 0;
    }
}

MyMJCardValueTypeCpp
UMyMJUtilsLibrary::getCardValueType(int32 cardValue)
{
    if (cardValue >= 1 && cardValue < 10) {
        return MyMJCardValueTypeCpp::WangZi;
    } else if (cardValue >= 11 && cardValue < 20) {
        return MyMJCardValueTypeCpp::BingZi;
    } else if (cardValue >= 21 && cardValue < 30) {
        return MyMJCardValueTypeCpp::TiaoZi;
    } else if (cardValue >= 31 && cardValue < 35) {
        return MyMJCardValueTypeCpp::Feng;
    } else if (cardValue >= 41 && cardValue < 44) {
        return MyMJCardValueTypeCpp::Jian;
    } else if (cardValue >= 51 && cardValue < 59) {
        return MyMJCardValueTypeCpp::Hua;
    } else {
        return MyMJCardValueTypeCpp::Invalid;
    }
}

bool
UMyMJUtilsLibrary::isCardTypeNumCard(MyMJCardValueTypeCpp type)
{
    if (type == MyMJCardValueTypeCpp::WangZi || type == MyMJCardValueTypeCpp::BingZi || type == MyMJCardValueTypeCpp::TiaoZi) {
        return true;
    }
    else {
        return false;
    }
}

bool
UMyMJUtilsLibrary::isCardValue258(int32 cardValue)
{
    int modV = cardValue % 10;
    return (cardValue >= 1 && cardValue < 30) && (modV == 2 || modV == 5 || modV == 8);
}

bool
UMyMJUtilsLibrary::isCardValueYao(int32 cardValue)
{
    if (cardValue >= 1 && cardValue < 30) {
        int modV = cardValue % 10;
        return (modV == 1 || modV == 9);
    }
    else if ((cardValue >= 31 && cardValue < 35) || (cardValue >= 41 && cardValue < 44)) {
        return true;
    }
    else {
        return false;
    }
}

bool
UMyMJUtilsLibrary::isMenQing(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut)
{
    int32 l = inWeavesShowedOut.Num();

    bool ret = true;
    for (int i = 0; i < l; i++) {
        if (inWeavesShowedOut[i].getType() != MyMJWeaveTypeCpp::GangAn) {
            ret = false;
            break;
        }
    }

    return ret;
}

void UMyMJUtilsLibrary::testSleep(float seconds)
{
    FPlatformProcess::Sleep(1);
}

void
UMyMJUtilsLibrary:: parseWeaves(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves, const int32 *pInTriggerCardIdx, FMyMJCardParseResultCpp &outResult, int32 *pOutIdxTriggerWeave)
{
    outResult.reset();

    FMyMJCardParseResultSimpleCpp *pSimple = &outResult;
    FMyMJCardParseResultCpp *pComplex = &outResult;
    MyMJWeaveTypeCpp weaveType;

    FMyStatisCountsPerCardValueTypeCpp *pStatisCounts0;

    int32 l, id, value;
    for (int32 i = 0; i < inWeaves.Num(); i++) {
        const FMyMJWeaveCpp *pWeave = &inWeaves[i];

        TArray<FMyIdValuePair> aIdValues; // = pWeave->getIdValuesRef();
        pWeave->getIdValues(inValuePack, aIdValues, true);

        l = aIdValues.Num();

        if (l <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idValues num %d"), l);
            MY_VERIFY(false);
        }

        weaveType = pWeave->getType();
        int32 weaveValue = pWeave->getMidValue(inValuePack);

        pStatisCounts0 = &pSimple->m_mWeaveCounts.FindOrAdd(weaveType);
        pStatisCounts0->addCountByValue(weaveValue, 1);


        for (int32 j = 0; j < l; j++) {
            id = aIdValues[j].m_iId;
            value = aIdValues[j].m_iValue;
            pSimple->m_cCardCounts.addCountByValue(value, 1);

            if (pInTriggerCardIdx && pOutIdxTriggerWeave && *pInTriggerCardIdx == id) {
                *pOutIdxTriggerWeave = i;
            }

        }


        //complex parsing

        if (weaveType == MyMJWeaveTypeCpp::DuiZi) {

            if (pComplex->m_iFirstDuiZiValue < 0) {
                pComplex->m_iFirstDuiZiValue = weaveValue;
            }

            if ((weaveValue >= 1 && weaveValue < 30) && (weaveValue % 10) == 5) {
                pComplex->m_iWeavesCountHaveNumCard5++;
            }

            if (isCardValueYao(weaveValue)) {
                pComplex->m_iWeaveCountHaveYaoCard++;
            }


        }
        else if (weaveType == MyMJWeaveTypeCpp::ShunZiAn || weaveType == MyMJWeaveTypeCpp::ShunZiMing) {

            int32 modV = weaveValue % 10;

            if (weaveValue >= 1 && weaveValue < 30) {
                if (modV == 2) {
                    pComplex->m_cChiLit.addCountByValue(weaveValue, 1);
                    pComplex->m_iWeaveCountHaveYaoCard++;
                }
                else if (modV == 5) {
                    pComplex->m_cChiMid.addCountByValue(weaveValue, 1);
                }
                else if (modV == 8) {
                    pComplex->m_cChiBig.addCountByValue(weaveValue, 1);
                    pComplex->m_iWeaveCountHaveYaoCard++;
                }
            }

            int32 *pC = &pComplex->m_mShunZiMidValueCountMap.FindOrAdd(weaveValue);
            *pC = *pC + 1;

            if ((weaveValue >= 1 && weaveValue < 30) && (modV >= 4 && modV < 7)) {
                pComplex->m_iWeavesCountHaveNumCard5++;
            }


        }
        else if (weaveType == MyMJWeaveTypeCpp::KeZiAn || weaveType == MyMJWeaveTypeCpp::KeZiMing) {

            int32 *pC = &pComplex->m_mKeZiOrGangValueCountMap.FindOrAdd(weaveValue);
            *pC = *pC + 1;

            if ((weaveValue >= 1 && weaveValue < 30) && (weaveValue % 10) == 5) {
                pComplex->m_iWeavesCountHaveNumCard5++;
            }

            if (isCardValueYao(weaveValue)) {
                pComplex->m_iWeaveCountHaveYaoCard++;
            }


        }
        else if (weaveType == MyMJWeaveTypeCpp::GangAn || weaveType == MyMJWeaveTypeCpp::GangMing) {

            int32 *pC = &pComplex->m_mKeZiOrGangValueCountMap.FindOrAdd(weaveValue);
            *pC = *pC + 1;

            if ((weaveValue >= 1 && weaveValue < 30) && (weaveValue % 10) == 5) {
                pComplex->m_iWeavesCountHaveNumCard5++;
            }

            if (isCardValueYao(weaveValue)) {
                pComplex->m_iWeaveCountHaveYaoCard++;
            }
        }

    }


}

int32
UMyMJUtilsLibrary::getCountOfValueHave4CardsExceptGang(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves)
{
    TMap<int32, int32> valueCountMapTemp;
    MyMJWeaveTypeCpp weaveType;
    const FMyMJWeaveCpp *pWeave;
    int32 l0, l1, value, ret = 0;

    TArray<FMyIdValuePair> aIdValues;
    l0 = inWeaves.Num();
    for (int32 i = 0; i < l0; i++) {
        pWeave = &inWeaves[i];
        weaveType = pWeave->getType();

        if (weaveType == MyMJWeaveTypeCpp::DuiZi ||
            weaveType == MyMJWeaveTypeCpp::ShunZiAn || weaveType == MyMJWeaveTypeCpp::ShunZiMing ||
            weaveType == MyMJWeaveTypeCpp::KeZiAn || weaveType == MyMJWeaveTypeCpp::KeZiMing) {

            pWeave->getIdValues(inValuePack, aIdValues, true);
            l1 = aIdValues.Num();

            for (int32 j = 0; j < l1; j++) {
                value = aIdValues[j].m_iValue;
                int32 &c = valueCountMapTemp.FindOrAdd(value);
                c += 1;
            }

        }
    }

    for (auto It = valueCountMapTemp.CreateConstIterator(); It; ++It)
    {
        const int32 &c = It.Value();

        if (c >= 4) {
            ret++;
        }

    }

    return ret;
}

bool
UMyMJUtilsLibrary::isWeavesHaveShunZi(const TArray<FMyMJWeaveCpp> &inWeaves)
{
    int32 l = inWeaves.Num();
    bool ret = false;
    MyMJCardValueTypeCpp unifiedType = MyMJCardValueTypeCpp::Invalid;
    bool bFound = false;
    for (int32 i = 0; i < l; i++) {
        const FMyMJWeaveCpp *pWeave = &inWeaves[i];
        if (pWeave->getType() != MyMJWeaveTypeCpp::ShunZiAn || pWeave->getType() != MyMJWeaveTypeCpp::ShunZiMing) {
            ret = true;
            break;
        }
    }

    return ret;
}

MyMJCardValueTypeCpp
UMyMJUtilsLibrary::getWeavesUnifiedNumCardType(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves)
{
    int32 l = inWeaves.Num();

    MyMJCardValueTypeCpp unifiedType = MyMJCardValueTypeCpp::Invalid;
    bool bFound = false;
    for (int32 i = 0; i < l; i++) {
        const FMyMJWeaveCpp *pWeave = &inWeaves[i];

        //MY_VERIFY(pWeave->m_aValues.Num() > 0);
        //int32 v = pWeave->m_aValues[0];
        int32 v = pWeave->getMidValue(inValuePack);
        MyMJCardValueTypeCpp t = getCardValueType(v);
        if (t == MyMJCardValueTypeCpp::WangZi || t == MyMJCardValueTypeCpp::BingZi || t == MyMJCardValueTypeCpp::TiaoZi) {
            if (bFound) {
                if (unifiedType != t) {
                    unifiedType = MyMJCardValueTypeCpp::Invalid;
                    break;
                }
            }
            else {
                unifiedType = t;
                bFound = true;
            }
        }
        else {
            unifiedType = MyMJCardValueTypeCpp::Invalid;
            break;
        }
    }

    return unifiedType;
}

void UMyMJUtilsLibrary::getWeavesShowedOutStatis(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves, FMyWeavesShowedOutStatisCpp &outResult)
{
    outResult.reset();
    outResult.m_bMenQing = isMenQing(inWeaves);
    outResult.m_bHaveShunZi = isWeavesHaveShunZi(inWeaves);
    outResult.m_eUnifiedNumCardType = getWeavesUnifiedNumCardType(inValuePack, inWeaves);
}


bool
UMyMJUtilsLibrary::checkHu(const FMyMJCardValuePackCpp &inValuePack,
                            const FMyMJHuCommonCfg &inHuCommonCfg,
                            const TArray<MyMJHuCardTypeCpp> inHuCardTypeCfgs,
                            const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                            const FMyMJValueIdMapCpp &inHandCardMap,
                            int32 extraId,
                            int32 extraValue,
                            MyMJHuCardTypeCpp &outHuType)
{

    const FMyMJValueIdMapCpp *pTarget = &inHandCardMap;
    FMyMJValueIdMapCpp newMap;

    bool bInReqDuiZi258ForCommonHu = true;
    bool bInAllowJiangNot258ForLastDuiZi = false;
    int32 iInDuiZi258CountMinFor7Dui = 2;

    if (extraValue > 0) {

        MY_VERIFY(extraId >= 0 || extraId == MyIDFake);


        newMap.copyDeep(&inHandCardMap);
        newMap.insert(extraId, extraValue);
        pTarget = &newMap;
    }

    MyMJHuCardTypeCpp huTypeFound = MyMJHuCardTypeCpp::Invalid;
    int32 huPriorityFound = 0;
    int32 huPri = 0;
    FMyMJCardParseResultCpp parseResult;
    //FMyMJCardParseResultCpp *pComplexResult = &parseResult;
    FMyMJCardParseResultSimpleCpp *pSimpleResult = &parseResult;

    FMyStatisCountsPerCardValueTypeCpp *pStatisCounts0 = NULL, *pStatisCounts1 = NULL, *pStatisCounts2 = NULL;

    bool bParsed = false;
    TArray<FMyMJWeaveArrayCpp> weaveCombines;

    bool bHu;
    bool bQiXingBuKao;
    int32 DuiZiAll, DuiZiNum258All;
    bool bMenQing = UMyMJUtilsLibrary::isMenQing(inWeavesShowedOut);

    bool bWeavesShowedOutMayPengPengHu = !isWeavesHaveShunZi(inWeavesShowedOut);
    MyMJCardValueTypeCpp  weavesShowedOutUnifiedType = getWeavesUnifiedNumCardType(inValuePack, inWeavesShowedOut);
    int32 weavesShowedOutCount = inWeavesShowedOut.Num();

    //Now all cards are pushed in, and we assume the priority is higher at the tail, let's loop from tail

    int32 l = inHuCardTypeCfgs.Num();
    for (int i = l - 1; i >= 0; i--) {
        //const FMyMJHuCardTypeCfgCpp * pAttr = &inHuCardTypeCfgs[i];
        MyMJHuCardTypeCpp eType = inHuCardTypeCfgs[i];

        bHu = false;
        if (eType == MyMJHuCardTypeCpp::CommonHu) {

            huPri = MJHuCardTypePRiCommonHu;
            if (huPri < huPriorityFound) {
                continue;
            }

            FMyMJWeaveTreeNodeCpp rootNode;
            bHu = pTarget->checkAllCanBeWeavedForHu(bInReqDuiZi258ForCommonHu, true, false, &rootNode);
        }
        else if (eType == MyMJHuCardTypeCpp::Special7Dui) {

            huPri = MJHuCardTypePRiSpecial7Dui;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            DuiZiAll = 0;
            DuiZiNum258All = 0;

            pStatisCounts0 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::DuiZi);
            if (pStatisCounts0) {
                DuiZiAll += pStatisCounts0->getTotalCount();
                DuiZiNum258All += pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258);
            }

            pStatisCounts1 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
            if (pStatisCounts1) {
                DuiZiAll += pStatisCounts1->getTotalCount() * 2;
                DuiZiNum258All += pStatisCounts1->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258) * 2;
            }

            if (DuiZiAll >= 7 && DuiZiNum258All >= iInDuiZi258CountMinFor7Dui) {
                bHu = true;
            }

        }
        else if (eType == MyMJHuCardTypeCpp::Special13Yao) {

            huPri = MJHuCardTypePRiSpecial13Yao;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            pStatisCounts0 = &pSimpleResult->m_cCardCounts;

            if (pStatisCounts0) {
                int32 count13YaoCard = pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) + pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::Feng) + pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);
                int32 countDuiZi = 0;
                pStatisCounts1 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::DuiZi);
                if (pStatisCounts1) {
                    countDuiZi = pStatisCounts1->getTotalCount();
                }

                int32 countKeZiAn = 0;
                pStatisCounts1 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::KeZiAn);
                if (pStatisCounts1) {
                    countKeZiAn = pStatisCounts1->getTotalCount();
                }

                int32 countGangAn = 0;
                pStatisCounts2 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
                if (pStatisCounts2) {
                    countGangAn = pStatisCounts2->getTotalCount();
                }

                if (count13YaoCard >= 14 && countDuiZi == 1 && countKeZiAn == 0 && countGangAn == 0) {
                    bHu = true;
                }
            }
        }
        else if (eType == MyMJHuCardTypeCpp::Special13BuKao) {

            huPri = MJHuCardTypePRiSpecial13BuKao;
            if (huPri < huPriorityFound) {
                continue;
            }

            bHu = pTarget->checkSpecial13BuKaoIn(bQiXingBuKao);
        }
        else if (eType == MyMJHuCardTypeCpp::SpecialZuHeLong) {

            huPri = MJHuCardTypePRiSpecialZuHeLong;
            if (huPri < huPriorityFound) {
                continue;
            }

            /* Todo
            if (inHuCommonCfg.m_bReqMenQingForZuHeLong) {
                if (false == bMenQing) {
                    continue;
                }
            }
            */
            bHu = pTarget->checkSpecialZuHeLong(weaveCombines);
        }

        //local styles
        else if (eType == MyMJHuCardTypeCpp::LocalCSAllJiang258) {

            huPri = MJHuCardTypePRiLocalAllJiang258;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            pStatisCounts0 = &pSimpleResult->m_cCardCounts;
            if (pStatisCounts0) {
                bHu = (pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258) >= 14);
            }
        }

        else if (eType == MyMJHuCardTypeCpp::LocalCSBornNoJiang258) {

            huPri = MJHuCardTypePRiBornLocalNoJiang258;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            pStatisCounts0 = &pSimpleResult->m_cCardCounts;
            if (pStatisCounts0) {
                bHu = (pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258) <= 0);
            }

        }
        else if (eType == MyMJHuCardTypeCpp::LocalCSBornLackNumCardType) {

            huPri = MJHuCardTypePRiBornLocalLackNumCardType;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            pStatisCounts0 = &pSimpleResult->m_cCardCounts;
            if (pStatisCounts0) {
                bHu = (pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::WangZi) <= 0 ||
                    pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::BingZi) <= 0 ||
                    pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::TiaoZi) <= 0);
            }

        }
        else if (eType == MyMJHuCardTypeCpp::LocalCSBorn2KeZi) {

            huPri = MJHuCardTypePRiBornLocal2KeZi;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            int32 KeZiOrGangAll = 0;
            pStatisCounts0 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::KeZiAn);
            if (pStatisCounts0) {
                KeZiOrGangAll += pStatisCounts0->getTotalCount();
            }
            pStatisCounts1 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
            if (pStatisCounts1) {
                KeZiOrGangAll += pStatisCounts1->getTotalCount();
            }

            bHu = (KeZiOrGangAll >= 2);

        }
        else if (eType == MyMJHuCardTypeCpp::LocalCSBornAnGangInHand) {

            huPri = MJHuCardTypePRiBornLocalAnGangInHand;
            if (huPri < huPriorityFound) {
                continue;
            }

            if (!bParsed) {
                pTarget->parseCards(false, parseResult);
                bParsed = true;
            }

            pStatisCounts0 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
            if (pStatisCounts0) {
                bHu = (pStatisCounts0->getTotalCount() >= 1);
            }

        }

        if (bHu) {
            huTypeFound = eType;
            huPriorityFound = huPri;
        }

    }

    outHuType = huTypeFound;
    return huPriorityFound > 0;

}


#define BanFengLaoLianAllowedFanDefault 100

/*
//simply consider weave card type(common hu type), not including other card type
void
UMyMJUtilsLibrary::calcWeaveScore( const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                    const TArray<FMyMJWeaveCpp> &inWeavesInHand,
                                    const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                                    const FMyMJHuActionAttrCpp &inHuActionAttr,
                                    FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;
    scoreResult.reset();

    TArray<FMyMJWeaveCpp> weavesAll = inWeavesShowedOut;
    weavesAll.Append(inWeavesInHand);

    FMyMJCardParseResultCpp weaveParseResult;

    FMyStatisCountsPerCardValueTypeCpp *pCountsGangAn, *pCountsGangMing, *pCountsKeZiAn, *pCountsKeZiMing, *pCountsCard, *pCountsChiLit, *pCountsChiBig, *pTemp;

    int32 idxTriggerWeave = -1;
    UMyMJUtilsLibrary::parseWeaves(weavesAll, &inHuActionAttr.m_cTriggerPair.m_iId, weaveParseResult, &idxTriggerWeave);

    int32 KeZiOrGangFengTotal = 0, KeZiOrGangJianTotal = 0, duiZiFengTotal = 0, duiZiJianTotal = 0, KeZiOrGangAnTotal = 0, KeZiOrGangTotal = 0;

    int32 GangTotal = 0;

    bool bMenQing = UMyMJUtilsLibrary::isMenQing(inWeavesShowedOut);

    //Since the rule forbid the weaves to be recalculated in different score type, but we didn't check that, and the problem happens in BanFengLaoLian, let's use it as short cut
    int32 BanFengLaoLianAllowedFanRemain = BanFengLaoLianAllowedFanDefault;

    int32 shunZiTotal = 0;
    pTemp = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::ShunZiAn);
    if (pTemp) {
        shunZiTotal = pTemp->getTotalCount();
    }
    pTemp = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::ShunZiMing);
    if (pTemp) {
        shunZiTotal = pTemp->getTotalCount();
    }


    MyMJHuScoreTypeCpp checkingScoreType;

    pCountsCard = &weaveParseResult.m_cCardCounts;
    pCountsGangAn = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
    pCountsGangMing = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangMing);
    pCountsKeZiAn = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::KeZiAn);
    pCountsKeZiMing = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::KeZiMing);

    pCountsChiLit = &weaveParseResult.m_cChiLit;
    pCountsChiBig = &weaveParseResult.m_cChiBig;

    pTemp = pCountsGangAn;
    if (pTemp) {
        KeZiOrGangFengTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
        KeZiOrGangJianTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);
        GangTotal += pTemp->getTotalCount();

        KeZiOrGangAnTotal += pTemp->getTotalCount();
        KeZiOrGangTotal += pTemp->getTotalCount();
    }

    pTemp = pCountsGangMing;
    if (pTemp) {
        KeZiOrGangFengTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
        KeZiOrGangJianTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);
        GangTotal += pTemp->getTotalCount();

        KeZiOrGangTotal += pTemp->getTotalCount();
    }

    pTemp = pCountsKeZiAn;
    if (pTemp) {
        KeZiOrGangFengTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
        KeZiOrGangJianTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);

        KeZiOrGangAnTotal += pTemp->getTotalCount();
        KeZiOrGangTotal += pTemp->getTotalCount();
    }

    pTemp = pCountsKeZiMing;
    if (pTemp) {
        KeZiOrGangFengTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
        KeZiOrGangJianTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);

        KeZiOrGangTotal += pTemp->getTotalCount();
    }


    pTemp = weaveParseResult.m_mWeaveCounts.Find(MyMJWeaveTypeCpp::DuiZi);
    if (pTemp) {
        duiZiFengTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
        duiZiJianTotal += pTemp->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);
    }

    int32 countGangMing = 0;
    if (pCountsGangMing) {
        countGangMing = pCountsGangMing->getTotalCount();
    }
    int32 countGangMingRemain = countGangMing;

    int32 countGangAn = 0;
    if (pCountsGangAn) {
        countGangAn = pCountsGangAn->getTotalCount();
    }
    int32 countGangAnRemain = countGangAn;

    //88 fan
    checkingScoreType = MyMJHuScoreTypeCpp::DaSiXi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangFengTotal >= 4) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::QuanFengKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::MenFengKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::SanFengKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);
            scoreResult.banType(MyMJHuScoreTypeCpp::YaoJiuKe);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::XiaoSiXi);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::DaSanYuan;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangJianTotal >= 3) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::ShuangJianKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::JianKe);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::XiaoSanYuan);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::LvYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtLv) >= (14 + GangTotal)) {
            scoreResult.addType(checkingScoreType);
        }
    }

    
    int32 wangZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::WangZi);
    int32 bingZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::BingZi);
    int32 tiaoZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::TiaoZi);
    int32 fengCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
    int32 jianCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);

    int32 wordCardCount = fengCount + jianCount;


    int32 qingYiSeV = -1;
    if (wangZiCount >= (14 + GangTotal)) {
        qingYiSeV = 0;
    }
    else if (bingZiCount >= (14 + GangTotal)) {
        qingYiSeV = 1;
    }
    else if (tiaoZiCount >= (14 + GangTotal)) {
        qingYiSeV = 2;
    }

    checkingScoreType = MyMJHuScoreTypeCpp::JiuLianBaoDeng;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (qingYiSeV > 0 && inCardsValueIdMapInHand.getCount() == 14) {

            if (inCardsValueIdMapInHand.getCountByValue(qingYiSeV * 10 + 1) >= 3 && inCardsValueIdMapInHand.getCountByValue(qingYiSeV * 10 + 9) >= 3) {
                bool bOK = true;
                for (int32 i = 2; i < 9; i++) {
                    if (inCardsValueIdMapInHand.getCountByValue(qingYiSeV * 10 + i) >= 1) {

                    }
                    else {
                        bOK = false;
                        break;
                    }
                }

                if (bOK && inHuActionAttr.m_cTriggerPair.m_iValue > 0) {
                    //CHeck if trigger card is the extra one
                    int32 cmin = 2;
                    if (inHuActionAttr.m_cTriggerPair.m_iValue % 10 == 1 || inHuActionAttr.m_cTriggerPair.m_iValue % 10 == 9) {
                        cmin = 4;
                    }
                    if (inCardsValueIdMapInHand.getCountByValue(inHuActionAttr.m_cTriggerPair.m_iValue) < cmin) {
                        bOK = false;
                    }
                }

                if (bOK) {
                    scoreResult.addType(checkingScoreType);

                    scoreResult.banType(MyMJHuScoreTypeCpp::QingYiSe);
                    scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
                    scoreResult.banType(MyMJHuScoreTypeCpp::YaoJiuKe);

                }
            }
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SiGang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (GangTotal >= 4) {
            scoreResult.addType(checkingScoreType);

            // basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::SanGang);
        }
    }
    

    //64 fan
    checkingScoreType = MyMJHuScoreTypeCpp::QingYaoJiu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) >= (14 + GangTotal)) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);
            scoreResult.banType(MyMJHuScoreTypeCpp::QuanDaiYao);
            scoreResult.banType(MyMJHuScoreTypeCpp::YaoJiuKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::HunYaoJiu);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::XiaoSiXi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangFengTotal >= 3 && duiZiFengTotal == 1) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::SanFengKe);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::XiaoSanYuan;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangJianTotal >= 2 && duiZiJianTotal == 1) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::JianKe);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::ZiYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if ((pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Feng) + pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Feng)) >= (14 + GangTotal)) {

            scoreResult.addType(checkingScoreType);
            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SiAnKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangAnTotal >= 4) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::SanAnKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::ShuangAnKe);

        }
    }


    int32 laoShaoFuCountWangZi = weaveParseResult.getLaoShaoFuCountByType(MyMJCardValueTypeCpp::WangZi);
    int32 laoShaoFuCountBingZi = weaveParseResult.getLaoShaoFuCountByType(MyMJCardValueTypeCpp::BingZi);
    int32 laoShaoFuCountTiaoZi = weaveParseResult.getLaoShaoFuCountByType(MyMJCardValueTypeCpp::TiaoZi);



    checkingScoreType = MyMJHuScoreTypeCpp::YiSeShuangLongHui;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 targetV = -100;
        if (laoShaoFuCountWangZi >= 2) {
            targetV = 5;
        }
        else if (laoShaoFuCountBingZi >= 2) {
            targetV = 15;
        }
        else if (laoShaoFuCountTiaoZi >= 2) {
            targetV = 25;
        }

        if (targetV > 0) {

            if (targetV == weaveParseResult.m_iFirstDuiZiValue) {
                scoreResult.addType(checkingScoreType);

                scoreResult.banType(MyMJHuScoreTypeCpp::PingHu);
                scoreResult.banType(MyMJHuScoreTypeCpp::QiDui);
                scoreResult.banType(MyMJHuScoreTypeCpp::QingYiSe);
                scoreResult.banType(MyMJHuScoreTypeCpp::YiBanGao);
                scoreResult.banType(MyMJHuScoreTypeCpp::LaoShaoFu);

                //logic ban
                scoreResult.banType(MyMJHuScoreTypeCpp::SanSeShuangLongHui);

                BanFengLaoLianAllowedFanRemain = 0;
            }
        }

    }


    int32 KeZiOrGangNumCardStep1IncreaseMax = weaveParseResult.getKeZiOrGangStep1IncreaseMaxRestrictNumCard();
    int32 shunZiStep1or2IncreaseMax = weaveParseResult.getShunZiStep1or2IncreaseMax();

    //48 fan
    checkingScoreType = MyMJHuScoreTypeCpp::YiSeSiTongShun;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (weaveParseResult.getShunZiSameGroupCount(4) >= 1) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanJieGao);
            scoreResult.banType(MyMJHuScoreTypeCpp::YiBanGao);
            scoreResult.banType(MyMJHuScoreTypeCpp::SiGuiYi);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanTongShun);

            BanFengLaoLianAllowedFanRemain = 0;
       }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::YiSeSiJieGao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangNumCardStep1IncreaseMax >= 4) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanTongShun);
            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanJieGao);
        }
    }

    //32 fan
    checkingScoreType = MyMJHuScoreTypeCpp::YiSeSiBuGao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (shunZiStep1or2IncreaseMax >= 4) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::LianLiu);
            scoreResult.banType(MyMJHuScoreTypeCpp::LaoShaoFu);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanBuGao);

            BanFengLaoLianAllowedFanRemain = 0;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanGang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (GangTotal >= 3) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::LianLiu);
            scoreResult.banType(MyMJHuScoreTypeCpp::LaoShaoFu);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HunYaoJiu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 hunYaoJiuCardCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) + wordCardCount;
        if (hunYaoJiuCardCount >= (14 + GangTotal) && pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) > 0 && wordCardCount > 0) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);
            scoreResult.banType(MyMJHuScoreTypeCpp::QuanDaiYao);

        }
    }


    //24 fan

    checkingScoreType = MyMJHuScoreTypeCpp::QuanShuangKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNumDouble) >= (14 + GangTotal)) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);
            scoreResult.banType(MyMJHuScoreTypeCpp::DuanYao);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QingYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (qingYiSeV > 0) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);
            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::YiSeSanTongShun;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (weaveParseResult.getShunZiSameGroupCount(3) >= 1) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);
            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanJieGao);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::YiBanGao);

            BanFengLaoLianAllowedFanRemain = 1;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::YiSeSanJieGao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangNumCardStep1IncreaseMax >= 3) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);
            scoreResult.banType(MyMJHuScoreTypeCpp::YiSeSanTongShun);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QuanDa;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNumBig) >= (14 + GangTotal)) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);
            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QuanZhong;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNumMid) >= (14 + GangTotal)) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);
            scoreResult.banType(MyMJHuScoreTypeCpp::DuanYao);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QuanXiao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNumLit) >= (14 + GangTotal)) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

        }
    }


    //16 fan
    checkingScoreType = MyMJHuScoreTypeCpp::QingLong;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (weaveParseResult.findQingLong() != MyMJCardValueTypeCpp::Invalid) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);

            BanFengLaoLianAllowedFanRemain = 1;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanSeShuangLongHui;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 targetV = -100;
        if (laoShaoFuCountWangZi == 1 && laoShaoFuCountBingZi == 1) {
            targetV = 25;
        }
        else if (laoShaoFuCountWangZi == 1 && laoShaoFuCountTiaoZi == 1) {
            targetV = 15;
        }
        else if (laoShaoFuCountBingZi == 1 && laoShaoFuCountTiaoZi == 1) {
            targetV = 5;
        }

        if (targetV > 0) {

            if (targetV == weaveParseResult.m_iFirstDuiZiValue) {
                scoreResult.addType(checkingScoreType);

                scoreResult.banType(MyMJHuScoreTypeCpp::XiXiangFeng);
                scoreResult.banType(MyMJHuScoreTypeCpp::LaoShaoFu);
                scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);
                scoreResult.banType(MyMJHuScoreTypeCpp::PingHu);

                BanFengLaoLianAllowedFanRemain = 0;
            }
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::YiSeSanBuGao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (shunZiStep1or2IncreaseMax >= 3) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);

            BanFengLaoLianAllowedFanRemain = 1;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QuanDaiWu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (weaveParseResult.m_iWeavesCountHaveNumCard5 >= 5) {
            //this is weaves card type, means no 7dui, and all double means no SHunZi, so it is for certain Kezi/Gang/Jiang

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::DuanYao);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanTongKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = weaveParseResult.getKeZiOrGangModV10SameGroupCountRestrictDifferentTypeAndNumCard(3, true);
        if (c >= 1) {

            scoreResult.addTypeWithCount(checkingScoreType, c);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::ShuangTongKe);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanAnKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangAnTotal >= 3) {

            scoreResult.addType(checkingScoreType);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::ShuangAnKe);

        }
    }


    //12 fan
    checkingScoreType = MyMJHuScoreTypeCpp::DaYuWu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNumBiggerThan5) >= (14 + GangTotal)) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::XiaoYuWu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNumLessThan5) >= (14 + GangTotal)) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanFengKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangFengTotal >= 3) {

            scoreResult.addType(checkingScoreType);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

        }
    }


    //8 fan
    checkingScoreType = MyMJHuScoreTypeCpp::HuaLong;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (weaveParseResult.findHuaLong()) {

            scoreResult.addType(checkingScoreType);

            BanFengLaoLianAllowedFanRemain = 1;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::TuiBuDao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtBuDao) >= (14 + GangTotal)) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::QueYiMen);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanSeSanTongShun;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = weaveParseResult.getShunZiModV10SameGroupCountRestrictDifferentType(3, true);
        if (c > 0) {

            scoreResult.addTypeWithCount(checkingScoreType, c);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::XiXiangFeng);

            BanFengLaoLianAllowedFanRemain = 1;

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanSeSanJieGao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = weaveParseResult.getCountSanSeSanJieGao();
        if (c > 0) {

            scoreResult.addTypeWithCount(checkingScoreType, c);

        }
    }


    //6 fan
    checkingScoreType = MyMJHuScoreTypeCpp::PengPengHu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangTotal >= 4) {

            scoreResult.addTypeWithCount(checkingScoreType, 1);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HunYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        int32 zeroTypeCount = 0;
        if (wangZiCount <= 0) {
            zeroTypeCount++;
        }
        if (bingZiCount <= 0) {
            zeroTypeCount++;
        }
        if (tiaoZiCount <= 0) {
            zeroTypeCount++;
        }

        if (zeroTypeCount == 2 && wordCardCount > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, 1);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SanSeSanBuGao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = weaveParseResult.getCountSanSeSanBuGao();
        if (c > 0) {

            scoreResult.addTypeWithCount(checkingScoreType, c);

            BanFengLaoLianAllowedFanRemain = 1;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::WuMenQi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (wangZiCount > 0 && bingZiCount > 0 && tiaoZiCount > 0 && fengCount > 0 && jianCount > 0) {

            scoreResult.addType(checkingScoreType);

        }
    }


    checkingScoreType = MyMJHuScoreTypeCpp::ShuangAnGang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (countGangAnRemain >= 2) {

            scoreResult.addTypeWithCount(checkingScoreType, countGangAnRemain / 2);


            countGangAnRemain = countGangAnRemain % 2;

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::ShuangJianKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (KeZiOrGangJianTotal >= 2) {

            scoreResult.addType(checkingScoreType);

            //basic rule ban
            scoreResult.banType(MyMJHuScoreTypeCpp::JianKe);

        }
    }


    //4 fan
    checkingScoreType = MyMJHuScoreTypeCpp::QuanDaiYao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (weaveParseResult.m_iWeaveCountHaveYaoCard >= 5) {

            scoreResult.addType(checkingScoreType);

        }
    }


    checkingScoreType = MyMJHuScoreTypeCpp::ShuangMingGang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (countGangMingRemain >= 2) {

            scoreResult.addTypeWithCount(checkingScoreType, countGangMingRemain / 2);

            countGangMingRemain = countGangMingRemain % 2;

        }
    }


    //2 fan
    checkingScoreType = MyMJHuScoreTypeCpp::JianKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (KeZiOrGangJianTotal >= 1) {

            scoreResult.addType(checkingScoreType);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QuanFengKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        int32 *pC = weaveParseResult.m_mKeZiOrGangValueCountMap.Find(inHuActionAttr.m_iQUanFeng);
        if (pC && *pC >= 1) {

            scoreResult.addType(checkingScoreType);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::MenFengKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        int32 *pC = weaveParseResult.m_mKeZiOrGangValueCountMap.Find(inHuActionAttr.m_iMenFeng);
        if (pC && *pC >= 1) {

            scoreResult.addType(checkingScoreType);

        }
    }


    checkingScoreType = MyMJHuScoreTypeCpp::PingHu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (shunZiTotal >= 4 && (weaveParseResult.m_iFirstDuiZiValue >= 1 && weaveParseResult.m_iFirstDuiZiValue < 30)) {

            scoreResult.addType(checkingScoreType);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::SiGuiYi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = getCountOfValueHave4CardsExceptGang(weavesAll);
        if (c > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, c);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::ShuangTongKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = weaveParseResult.getKeZiOrGangModV10SameGroupCountRestrictDifferentTypeAndNumCard(2, true);
        if (c >= 1) {

            scoreResult.addTypeWithCount(checkingScoreType, c);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::ShuangAnKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (KeZiOrGangAnTotal >= 2) {
            scoreResult.addType(checkingScoreType);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::AnGang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (countGangAnRemain >= 1) {

            scoreResult.addTypeWithCount(checkingScoreType, countGangAnRemain);

            countGangAnRemain = 0;

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::DuanYao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) == 0 && wordCardCount == 0) {

            scoreResult.addType(checkingScoreType);

        }
    }

    if (BanFengLaoLianAllowedFanRemain == BanFengLaoLianAllowedFanDefault) {
        if (shunZiTotal == 4) {
            BanFengLaoLianAllowedFanRemain = 3;
        }
        else if (shunZiTotal == 3) {
            BanFengLaoLianAllowedFanRemain = 2;
        }
    }

    //1 fan
    checkingScoreType = MyMJHuScoreTypeCpp::YiBanGao;
    if (scoreResult.isTypeAllowed(checkingScoreType) && BanFengLaoLianAllowedFanRemain > 0) {
        int32 c = weaveParseResult.getShunZiSameGroupCount(2);
        if (c > 0) {

            scoreResult.addTypeWithCount(checkingScoreType, c);
            BanFengLaoLianAllowedFanRemain -= c;

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::XiXiangFeng;
    if (scoreResult.isTypeAllowed(checkingScoreType) && BanFengLaoLianAllowedFanRemain > 0) {
        int32 c = weaveParseResult.getShunZiModV10SameGroupCountRestrictDifferentType(2, true); 
        if (c > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, c);
            BanFengLaoLianAllowedFanRemain -= c;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::LianLiu;
    if (scoreResult.isTypeAllowed(checkingScoreType) && BanFengLaoLianAllowedFanRemain > 0) {
        int32 c = weaveParseResult.getCountLianLiu();
        if (c > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, c);
            BanFengLaoLianAllowedFanRemain -= c;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::LaoShaoFu;
    if (scoreResult.isTypeAllowed(checkingScoreType) && BanFengLaoLianAllowedFanRemain > 0) {
        int32 c = weaveParseResult.getLaoShaoFuCountByType(MyMJCardValueTypeCpp::WangZi) + weaveParseResult.getLaoShaoFuCountByType(MyMJCardValueTypeCpp::BingZi) + weaveParseResult.getLaoShaoFuCountByType(MyMJCardValueTypeCpp::TiaoZi);
        if (c > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, c);
            BanFengLaoLianAllowedFanRemain -= c;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::YaoJiuKe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = weaveParseResult.getCountYaoJiuKe();
        if (c > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, c);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::MingGang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (countGangMingRemain >= 1) {

            scoreResult.addTypeWithCount(checkingScoreType, countGangMingRemain);
            countGangMingRemain = 0;
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QueYiMen;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (wangZiCount == 0 || bingZiCount == 0 || tiaoZiCount == 0) {
            scoreResult.addType(checkingScoreType);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::WuZi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (wordCardCount== 0) {
            scoreResult.addType(checkingScoreType);
        }
    }

    if (inHuActionAttr.m_iTingCount == 1 && idxTriggerWeave >= 0) {
        MY_VERIFY(idxTriggerWeave < weavesAll.Num());
        FMyMJWeaveCpp &weave = weavesAll[idxTriggerWeave];

        if (weave.getType() == MyMJWeaveTypeCpp::DuiZi) {

            checkingScoreType = MyMJHuScoreTypeCpp::DanDiaoJiang;
            if (scoreResult.isTypeAllowed(checkingScoreType)) {
                scoreResult.addType(checkingScoreType);
            }
        }
        else if (weave.getType() == MyMJWeaveTypeCpp::ShunZiAn || weave.getType() == MyMJWeaveTypeCpp::ShunZiMing) { //actually ShunZiMing not possible
            int32 midV = weave.getMidValue();
            int32 delta = midV - inHuActionAttr.m_cTriggerPair.m_iValue;
            if (!(delta >= -1 && delta < 2)) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected rigger weave midV and trigger card value: %d, %d"), midV, inHuActionAttr.m_cTriggerPair.m_iValue);
                MY_VERIFY(false);
            }
            if (delta == 0) {
                checkingScoreType = MyMJHuScoreTypeCpp::KanZhang;
                if (scoreResult.isTypeAllowed(checkingScoreType)) {
                    scoreResult.addType(checkingScoreType);
                }
            }
            else {
                checkingScoreType = MyMJHuScoreTypeCpp::BianZhang;
                if (scoreResult.isTypeAllowed(checkingScoreType)) {
                    scoreResult.addType(checkingScoreType);
                }
            }
        }
    }


    calcHuScorePostProcess(inHuActionAttr, inWeavesShowedOut, scoreResult);
}
*/

void
UMyMJUtilsLibrary::calc7DuiScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                 const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                                 const FMyMJHuActionAttrCpp &inHuActionAttr,
                                 FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;
    scoreResult.reset();

    FMyMJCardParseResultCpp handCardParseResult;
    inCardsValueIdMapInHand.parseCards(true, handCardParseResult);

    MyMJHuScoreTypeCpp checkingScoreType;
    FMyStatisCountsPerCardValueTypeCpp *pCountsCard = &handCardParseResult.m_cCardCounts;

    MY_VERIFY(pCountsCard->getTotalCount() == 14);

    int32 wangZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::WangZi);
    int32 bingZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::BingZi);
    int32 tiaoZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::TiaoZi);
    int32 fengCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
    int32 jianCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);

    int32 wordCardCount = fengCount + jianCount;

    checkingScoreType = MyMJHuScoreTypeCpp::LvYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtLv) >= (14)) {

            scoreResult.addType(checkingScoreType);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::LianQiDui;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (handCardParseResult.findLian7Dui()) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::QiDui);
            scoreResult.banType(MyMJHuScoreTypeCpp::QingYiSe);
            scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
            scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QingYaoJiu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) >= (14)) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);
            scoreResult.banType(MyMJHuScoreTypeCpp::QuanDaiYao);
            scoreResult.banType(MyMJHuScoreTypeCpp::YaoJiuKe);
            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::ZiYiSe);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::ZiYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (wordCardCount >= 14) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HunYaoJiu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if ((wordCardCount + pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19)) >= 14 && wordCardCount > 0 && pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) > 0) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::PengPengHu);
            scoreResult.banType(MyMJHuScoreTypeCpp::QuanDaiYao);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QiDui;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        MY_VERIFY(handCardParseResult.getDuiZiOrGangTotal() == 7);
        scoreResult.addType(checkingScoreType);

        scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
        scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);

    }

    checkingScoreType = MyMJHuScoreTypeCpp::QingYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (wangZiCount >= 14 ||
            bingZiCount >= 14 ||
            tiaoZiCount >= 14) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuZi);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::TuiBuDao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtBuDao) >= 14) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::QueYiMen);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HunYiSe;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        int32 zeroTypeCount = 0;
        if (wangZiCount <= 0) {
            zeroTypeCount++;
        }
        if (bingZiCount <= 0) {
            zeroTypeCount++;
        }
        if (tiaoZiCount <= 0) {
            zeroTypeCount++;
        }

        if (zeroTypeCount == 2 && wordCardCount > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, 1);
        }

        if (zeroTypeCount == 2 && wordCardCount > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, 1);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::WuMenQi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (wangZiCount > 0 && bingZiCount > 0 && tiaoZiCount > 0 && fengCount > 0 && jianCount > 0) {

            scoreResult.addType(checkingScoreType);

        }
    }


    //2 fan
    checkingScoreType = MyMJHuScoreTypeCpp::SiGuiYi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        int32 c = inCardsValueIdMapInHand.getCountOfGroupSameCardValue(4, true);
        if (c > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, c);

        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::DuanYao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum19) == 0 && wordCardCount == 0) {

            scoreResult.addType(checkingScoreType);

        }
    }


    //1 fan
    checkingScoreType = MyMJHuScoreTypeCpp::QueYiMen;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (wangZiCount == 0 || bingZiCount == 0 || tiaoZiCount == 0) {
            scoreResult.addType(checkingScoreType);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::WuZi;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (wordCardCount == 0) {
            scoreResult.addType(checkingScoreType);
        }
    }

    //Todo: fix
    //calcHuScorePostProcess(inHuActionAttr, inWeavesShowedOut, scoreResult);
}


void
UMyMJUtilsLibrary::calc13YaoScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                  const FMyMJHuActionAttrCpp &inHuActionAttr,
                                  FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;
    scoreResult.reset();

    MyMJHuScoreTypeCpp checkingScoreType;

    checkingScoreType = MyMJHuScoreTypeCpp::ShiSanYao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        scoreResult.addType(checkingScoreType);

        scoreResult.banType(MyMJHuScoreTypeCpp::WuMenQi);
        scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
        scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);
    }

    //Todo: fix
    //calcHuScorePostProcess(inHuActionAttr, inWeavesShowedOut, scoreResult);
}


void
UMyMJUtilsLibrary::calc13BuKaoScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                    const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                                    const FMyMJHuActionAttrCpp &inHuActionAttr,
                                    FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;
    scoreResult.reset();

    FMyMJCardParseResultCpp handCardParseResult;
    inCardsValueIdMapInHand.parseCards(true, handCardParseResult);

    MyMJHuScoreTypeCpp checkingScoreType;
    FMyStatisCountsPerCardValueTypeCpp *pCountsCard = &handCardParseResult.m_cCardCounts;

    int32 wangZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::WangZi);
    int32 bingZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::BingZi);
    int32 tiaoZiCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::TiaoZi);
    int32 fengCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Feng);
    int32 jianCount = pCountsCard->getCountByCardValueType(MyMJCardValueTypeCpp::Jian);

    int32 wordCardCount = fengCount + jianCount;

    checkingScoreType = MyMJHuScoreTypeCpp::QiXingBuKao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (wordCardCount == 7) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuMenQi);
            scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
            scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::QuanBuKao);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QuanBuKao;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuMenQi);
            scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
            scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);

    }

    checkingScoreType = MyMJHuScoreTypeCpp::ZuHeLong;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (wordCardCount == 5) {
            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::WuMenQi);
            scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
            scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);

        }
    }

    //Todo: fix
    //calcHuScorePostProcess(inHuActionAttr, inWeavesShowedOut, scoreResult);
}


void
UMyMJUtilsLibrary::calcZuHeLongScore(const FMyMJHuCommonCfg &inHuCommonCfg,
                                     const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                     const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                                     const FMyMJHuActionAttrCpp &inHuActionAttr,
                                     FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;
    scoreResult.reset();

    FMyMJCardParseResultCpp handCardParseResult;
    inCardsValueIdMapInHand.parseCards(true, handCardParseResult);

    MyMJHuScoreTypeCpp checkingScoreType;
    FMyStatisCountsPerCardValueTypeCpp *pCountsCard = &handCardParseResult.m_cCardCounts;

    TArray<FMyMJWeaveArrayCpp> weaveCombines;

    bool b = inCardsValueIdMapInHand.checkSpecialZuHeLong(weaveCombines);
    MY_VERIFY(b);
    MY_VERIFY(weaveCombines.Num() > 0);
    
    TArray<FMyMJWeaveCpp> *pWeaves = &weaveCombines[0].m_aWeaves;

    scoreResult.m_aWeavesInHand = *pWeaves;
    int32 l = pWeaves->Num();
    MY_VERIFY(l > 0);


    bool bPingHu = false;
    for (int i = 0; i < l; i++) {
        const FMyMJWeaveCpp *pWeave = &(*pWeaves)[i];
        if (pWeave->getType() == MyMJWeaveTypeCpp::DuiZi) {
            continue;
        }

        if (pWeave->getType() == MyMJWeaveTypeCpp::ShunZiAn || pWeave->getType() == MyMJWeaveTypeCpp::ShunZiMing) {
            bPingHu = true;
        }

        break;
    }

    checkingScoreType = MyMJHuScoreTypeCpp::ZuHeLong;
    scoreResult.addType(checkingScoreType);
    scoreResult.banType(MyMJHuScoreTypeCpp::WuMenQi);
    scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);

    /* Todo
    if (inHuCommonCfg.m_bReqMenQingForZuHeLong) {
        scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
    }
    */

    checkingScoreType = MyMJHuScoreTypeCpp::PingHu;
    if (bPingHu) {
        scoreResult.addType(checkingScoreType);
    }

    //Todo: fix
    //calcHuScorePostProcess(inHuActionAttr, inWeavesShowedOut, scoreResult);
}

void
UMyMJUtilsLibrary::calcHuOtherLocalScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                         const FMyMJHuActionAttrCpp &inHuActionAttr,
                                         FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;
    scoreResult.reset();

    //MyMJHuScoreTypeCpp checkingScoreType;

    //local hu type
    /*
    if (inHuActionAttr.m_eType == MyMJHuCardTypeCpp::LocalCSAllJiang258) {
        checkingScoreType = MyMJHuScoreTypeCpp::LocalCSAllJiang258;
        if (scoreResult.isTypeAllowed(checkingScoreType)) {

            scoreResult.addType(checkingScoreType);

        }
    }
    */

    //Todo: fix
    //calcHuScorePostProcess(inHuActionAttr, inWeavesShowedOut, scoreResult);
}

/*
void
UMyMJUtilsLibrary::calHuScoreAllCardIn(const FMyMJHuCfgCpp &inHuCfg,
                                        const FMyMJHuActionAttrCpp &inHuActionAttr,
                                        const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                        const FMyMJValueIdMapCpp &inHandCardMap,
                                        FMyMJScoreCalcResultCpp &outScoreResultMax)
{


    bool inReqJiang258 = true;
    bool inAllowJiangNot258ForLastDuiZi = false;
    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> &inHuScoreSettings = inHuCfg.getHuScoreAttrsCfgRef();
    //MyMJHuCardTypeCpp huType  = inHuActionAttr.m_eType;
    MyMJHuCardTypeCpp huType = MyMJHuCardTypeCpp::Invalid; //TODO: fix the logic
    bool bPao = (inHuActionAttr.m_iIdxAttenderLoseOnlyOne >= 0);

    FMyMJScoreCalcResultCpp outScoreResult;
    outScoreResult.init(inHuScoreSettings);

    FMyMJWeaveTreeNodeCpp rootNode;
    bool bCanWeave;
    TArray<FMyMJWeaveArrayCpp> weaveCombines;

    bool bWeavesShowedOutMayPengPengHu = !isWeavesHaveShunZi(inWeavesShowedOut);
    MyMJCardValueTypeCpp  weavesShowedOutUnifiedType = getWeavesUnifiedNumCardType(inWeavesShowedOut);
    int32 weavesShowedOutCount = inWeavesShowedOut.Num();

    if (huType == MyMJHuCardTypeCpp::CommonHu) {
        rootNode.reset();
        bCanWeave = inHandCardMap.checkAllCanBeWeavedForHu(inReqJiang258, true, true, &rootNode);
        if (bCanWeave) {
            rootNode.convert2WeaveArray(weaveCombines);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Common Hu but can't weave!"));
            MY_VERIFY(false);
        }

        int32 l = weaveCombines.Num();
        MY_VERIFY(l > 0);
        for (int32 i = 0; i < l; i++) {

            calcWeaveScore(inWeavesShowedOut, weaveCombines[i].m_aWeaves, inHandCardMap, inHuActionAttr, outScoreResult);

            if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
                outScoreResultMax = outScoreResult;
                outScoreResultMax.m_eHuCardType = huType;
                outScoreResultMax.m_aWeavesShowedOut = inWeavesShowedOut;
                outScoreResultMax.m_aWeavesInHand = weaveCombines[i].m_aWeaves;
                outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
            }
        }

    }
    else if (huType == MyMJHuCardTypeCpp::Special7Dui) {

        //7Dui may also hu common, let's check and pick out the highest way
        rootNode.reset();
        bCanWeave = inHandCardMap.checkAllCanBeWeavedForHu(inReqJiang258, true, true, &rootNode);
        if (bCanWeave) {
            rootNode.convert2WeaveArray(weaveCombines);

            int32 l = weaveCombines.Num();
            MY_VERIFY(l > 0);
            for (int32 i = 0; i < l; i++) {

                calcWeaveScore(inWeavesShowedOut, weaveCombines[i].m_aWeaves, inHandCardMap, inHuActionAttr, outScoreResult);

                if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
                    outScoreResultMax = outScoreResult;
                    outScoreResultMax.m_eHuCardType = MyMJHuCardTypeCpp::CommonHu;

                    outScoreResultMax.m_aWeavesShowedOut = inWeavesShowedOut;
                    outScoreResultMax.m_aWeavesInHand = weaveCombines[i].m_aWeaves;

                    outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
                }
            }


            outScoreResult.reset();
            calc7DuiScore(inWeavesShowedOut, inHandCardMap, inHuActionAttr, outScoreResult);
            if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
                outScoreResultMax = outScoreResult;
                outScoreResultMax.m_eHuCardType = huType;

                inHandCardMap.fillInWeavesAssert7Dui(outScoreResultMax.m_aWeavesInHand);
                
                outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
            }
        }
    }
    else if (huType == MyMJHuCardTypeCpp::Special13Yao) {

        calc13YaoScore(inWeavesShowedOut, inHuActionAttr, outScoreResult);
        if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
            outScoreResultMax = outScoreResult;
            outScoreResultMax.m_eHuCardType = huType;

            inHandCardMap.fillInWeavesSpecialUnWeavedCards(outScoreResultMax.m_aWeavesInHand);

            outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
        }

    }
    else if (huType == MyMJHuCardTypeCpp::Special13BuKao) {

        calc13BuKaoScore(inWeavesShowedOut, inHandCardMap, inHuActionAttr, outScoreResult);
        if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
            outScoreResultMax = outScoreResult;
            outScoreResultMax.m_eHuCardType = huType;

            inHandCardMap.fillInWeavesSpecialUnWeavedCards(outScoreResultMax.m_aWeavesInHand);

            outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
        }
    }
    else if (huType == MyMJHuCardTypeCpp::SpecialZuHeLong) {

        calcZuHeLongScore(inHuCfg.m_cHuCommonCfg, inWeavesShowedOut, inHandCardMap, inHuActionAttr, outScoreResult);
        if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
            outScoreResultMax = outScoreResult;
            outScoreResultMax.m_eHuCardType = huType;

            //hand weaves already filled in in previous calcZuHeLongScore()  

            outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
        }
    }

    else if (huType == MyMJHuCardTypeCpp::LocalCSAllJiang258) {

        rootNode.reset();
        bCanWeave = inHandCardMap.checkAllCanBeWeavedForHu(inReqJiang258, true, true, &rootNode);
        if (bCanWeave) {
            rootNode.convert2WeaveArray(weaveCombines);

            int32 l = weaveCombines.Num();
            MY_VERIFY(l > 0);
            for (int32 i = 0; i < l; i++) {

                calcWeaveScore(inWeavesShowedOut, weaveCombines[i].m_aWeaves, inHandCardMap, inHuActionAttr, outScoreResult);

                if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
                    outScoreResultMax = outScoreResult;
                    outScoreResultMax.m_eHuCardType = MyMJHuCardTypeCpp::CommonHu;

                    outScoreResultMax.m_aWeavesShowedOut = inWeavesShowedOut;
                    outScoreResultMax.m_aWeavesInHand = weaveCombines[i].m_aWeaves;

                    outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
                }
            }
        }

        calcHuOtherLocalScore(inWeavesShowedOut, inHuActionAttr, outScoreResult);
        if (outScoreResultMax.isScoreLessThan(outScoreResult)) {
            outScoreResultMax = outScoreResult;
            outScoreResultMax.m_eHuCardType = huType;

            outScoreResultMax.m_aWeavesShowedOut = inWeavesShowedOut;
            inHandCardMap.fillInWeavesSpecialUnWeavedCards(outScoreResultMax.m_aWeavesInHand);

            outScoreResultMax.m_iIdxTriggerCard = inHuActionAttr.m_cTriggerPair.m_iId;
        }

    }

    MY_VERIFY(outScoreResultMax.getScore() > 0);
}
*/


void
UMyMJUtilsLibrary::calcHuScorePostProcess(const FMyMJHuActionAttrBaseCpp &inHuActionAttrBase, const FMyTriggerDataCpp *pTriggerData, const TArray<FMyMJWeaveCpp> &inWeavesShowedOut, FMyMJScoreCalcResultCpp &outScoreResult)
{
    FMyMJScoreCalcResultCpp &scoreResult = outScoreResult;

    bool bMenQing = isMenQing(inWeavesShowedOut);

    MyMJHuScoreTypeCpp checkingScoreType;

    bool bLastCardTurn = false;
    bool bLastCardOfTriggerCardValue = false;
    if (pTriggerData) {
        bLastCardTurn = (inHuActionAttrBase.m_iCardNumCanBeTakenNormally == 0);
        bLastCardOfTriggerCardValue = (pTriggerData->m_iValueShowedOutCountAfter >= 4);
    }

    bool bPao = (inHuActionAttrBase.m_iIdxAttenderLoseOnlyOne >= 0);


    //Begin check hu action related
    checkingScoreType = MyMJHuScoreTypeCpp::MiaoShouHuiChun;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        bool cond = bLastCardTurn && bPao == false && inHuActionAttrBase.m_eTrigerCardActionType == MyMJHuTriggerCardSrcTypeCpp::CommonInGame;

        if (cond) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::ZiMo);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HaiDiLaoYue;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        bool cond = bLastCardTurn && bPao == true;

        if (cond) {

            scoreResult.addType(checkingScoreType);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::ZiMo);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::GangShangKaiHua;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        bool cond = bPao == false && inHuActionAttrBase.m_eTrigerCardActionType == MyMJHuTriggerCardSrcTypeCpp::GangCardTaken;

        if (cond) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::ZiMo);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::QiangGangHu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        bool cond = bPao == true && inHuActionAttrBase.m_eTrigerCardActionType == MyMJHuTriggerCardSrcTypeCpp::GangWeaveShowedOut;

        if (cond) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::HuJueZhang);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::ZiMo);
        }
    }
    
    // 6 fan
    checkingScoreType = MyMJHuScoreTypeCpp::QuanQiuRen;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (inWeavesShowedOut.Num() >= 4 && bPao == true) {

            scoreResult.addType(checkingScoreType);

            scoreResult.banType(MyMJHuScoreTypeCpp::DanDiaoJiang);

        }
    }


    //4 fan
    checkingScoreType = MyMJHuScoreTypeCpp::BuQiuRen;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (bMenQing && bPao == false) {

            scoreResult.addType(checkingScoreType);

            //logic ban
            scoreResult.banType(MyMJHuScoreTypeCpp::MenQianQing);
            scoreResult.banType(MyMJHuScoreTypeCpp::ZiMo);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HuJueZhang;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (bLastCardOfTriggerCardValue && inHuActionAttrBase.m_eTrigerCardActionType != MyMJHuTriggerCardSrcTypeCpp::GangWeaveShowedOut) {

            scoreResult.addType(checkingScoreType);

        }
    }

    //2 fan
    checkingScoreType = MyMJHuScoreTypeCpp::MenQianQing;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {

        if (bMenQing && bPao == true) {

            scoreResult.addType(checkingScoreType);

        }
    }

    //1 fan
    checkingScoreType = MyMJHuScoreTypeCpp::ZiMo;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (bPao == false) {
            scoreResult.addType(checkingScoreType);
        }
    }

    checkingScoreType = MyMJHuScoreTypeCpp::HuaPai;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        if (inHuActionAttrBase.m_iHuaCount > 0) {
            scoreResult.addTypeWithCount(checkingScoreType, inHuActionAttrBase.m_iHuaCount);
        }
    }

    //standard post process
    checkingScoreType = MyMJHuScoreTypeCpp::WuFanHu;
    if (scoreResult.isTypeAllowed(checkingScoreType)) {
        bool bHaveStandardFan = true;
        if (scoreResult.m_mAddedTypeCountMap.Num() <= 0) {
            bHaveStandardFan = false;
        }
        else if (scoreResult.m_mAddedTypeCountMap.Num() == 1) {
            if (scoreResult.getFirstAddedScoreType() == MyMJHuScoreTypeCpp::HuaPai) {
                bHaveStandardFan = false;
            }
        }

        if (bHaveStandardFan == false) {
            scoreResult.addType(checkingScoreType);
        }
    }


}

int32 UMyMJUtilsLibrary::getIdxOfUntakenSlotHavingCard(const TArray<FMyIdCollectionCpp> &aUntakenCardStacks, int32 idxBase, uint32 delta, bool bReverse)
{

    int32 l = aUntakenCardStacks.Num();

    MY_VERIFY(idxBase >= 0 && idxBase < l);

    int32 idxChecking = idxBase;
    for (int32 i = 0; i < l; i++) {
        const FMyIdCollectionCpp *pC = &aUntakenCardStacks[idxChecking];
        int32 l2 = pC->m_aIds.Num();

        if (l2 > 0) {
            //this is a valid stack
            if (delta == 0) {
                break;
            }
            else {
                delta--;
            }
        }

        //goto next
        if (bReverse) {
            idxChecking = (idxChecking - 1 + l) % l;
        }
        else {
            idxChecking = (idxChecking + 1) % l;
        }
    }

    if (delta == 0) {
        return idxChecking;
    }
    else {
        return -1;
    }
}
