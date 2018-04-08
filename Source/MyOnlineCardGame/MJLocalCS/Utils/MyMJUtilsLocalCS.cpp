// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJUtilsLocalCS.h"
#include "Utils/CommonUtils/MyCommonDefines.h"
#include "Kismet/KismetMathLibrary.h"

void
UMyMJUtilsLocalCSLibrary::genDefaultCfg(FMyMJGameCfgCpp &outCfg)
{
    FMyMJGameCfgCpp defaultOne;
    defaultOne.m_eRuleType = MyMJGameRuleTypeCpp::LocalCS;
    int32 idx = defaultOne.m_aSubLocalCSCfg.Emplace();
    FMyMJGameSubLocalCSCfgCpp& localCSCfg = defaultOne.m_aSubLocalCSCfg[idx];
    TArray<FMyMJHuScoreAttrCpp> *pScoreAttrs = &localCSCfg.getHuBornScoreAttrsRef();
    

    FMyMJHuScoreAttrCpp *pAttr;
    MyMJHuScoreTypeCpp eScoreType;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSBornNoJiang258;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 1;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSBornLackNumCardType;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 1;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSBorn2KeZi;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 1;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSBornAnGangInHand;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 1;


    pScoreAttrs = &defaultOne.m_cHuCfg.getHuScoreAttrsRef();

    eScoreType = MyMJHuScoreTypeCpp::PingHu;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 1;

    eScoreType = MyMJHuScoreTypeCpp::PengPengHu;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::QingYiSe;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSAllJiang258;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::QuanQiuRen;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::QiDui;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSHaoHuaQiDui;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalCSShuangHaoHuaQiDui;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::MiaoShouHuiChun;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalHaiDiPao;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::GangShangKaiHua;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::QiangGangHu;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalGangShangPao;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalTianHu;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::LocalDiHu;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    eScoreType = MyMJHuScoreTypeCpp::MenQianQing;
    idx = pScoreAttrs->Emplace();
    pAttr = &(*pScoreAttrs)[idx];
    pAttr->m_eType = eScoreType;
    pAttr->m_iScorePerAttender = 6;

    outCfg = defaultOne;
}

struct FMyHuAttrTempIn
{

public:

    FMyHuAttrTempIn() {
        m_iPri = 0;
        m_pScoreAttr = NULL;
    };

    virtual ~FMyHuAttrTempIn() {};

    int32 m_iPri;
    const FMyMJHuScoreAttrCpp* m_pScoreAttr;
    TArray<int32> m_aIDs;
};

bool
UMyMJUtilsLocalCSLibrary::checkHuLocalCSBorn(const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                        const FMyMJValueIdMapCpp &handCardMap,
                                        TArray<FMyMJHuScoreResultItemCpp> &outHuScoreResultItems,
                                        TArray<int32> &outShowOutCards)
{
    bool showAllCards = localCSCfg.m_bHuBornShowAllCards;
    bool allowMultipleHu = localCSCfg.m_bHuBornAllowMultiple;
    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> &mAllowedScoreAttrs = localCSCfg.getHuBornScoreAttrsRefConst();

    const FMyMJValueIdMapCpp *pTarget = &handCardMap;

    int32 huPriorityMaxFound = 0;

    int32 huPri;
    int32 l;
    
    outHuScoreResultItems.Reset();
    outShowOutCards.Reset();

    l = mAllowedScoreAttrs.Num();
    if (l <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("mAllowedScoreAttrs.Num() <= 0, %d"), l);
        return false;
    }


    //let's do a manual parse
    TArray<int32> aCardValues;
    TArray<int32> aIDs;
    handCardMap.keys(aCardValues);

    TArray<int32> aIDsKeZi;
    TArray<int32> aIDsAnGang;
    int32 count258Card = 0;
    int32 countPerTypes[(uint8)MyMJCardValueTypeCpp::Max];
    for (int32 i = 0; i < (uint8)MyMJCardValueTypeCpp::Max; i++) {
        countPerTypes[i] = 0;
    }

    l = aCardValues.Num();
    for (int32 i = 0; i < l; i++) {
        aIDs.Reset();
        int32 cardValue = aCardValues[i];
        handCardMap.collectByValue(cardValue, 0, aIDs);
        int32 cardCount = aIDs.Num();

        if (cardCount == 3) {
            if (aIDsKeZi.Num() < 6) {
                aIDsKeZi.Append(aIDs);
            }
        }
        else if (cardCount == 4) {
            if (aIDsAnGang.Num() < 4) {
                aIDsAnGang.Append(aIDs);
            }
        }

        if (UMyMJUtilsLibrary::isCardValue258(cardValue)) {
            count258Card += cardCount;
        }

        MyMJCardValueTypeCpp vType = UMyMJUtilsLibrary::getCardValueType(cardValue);
        if ((uint8)vType < (uint8)MyMJCardValueTypeCpp::Max) {
            countPerTypes[(uint8)vType] += cardCount;
        }

    }

    //For simple, use card type priority as score type priority
    TArray<FMyHuAttrTempIn> aFoundedHuAttrTemps;
    TArray<int32> aIDsShowOut;

    for (auto It = mAllowedScoreAttrs.CreateConstIterator(); It; ++It) {

        //const FMyMJHuCardTypeCfgCpp * pAttr = &inHuCardTypeCfgs[i];
        //MyMJHuCardTypeCpp eCardType = allowedCardTypes[i];
        MyMJHuScoreTypeCpp eScoreType = It.Key();
        const FMyMJHuScoreAttrCpp *pAttr = &It.Value();
        MY_VERIFY(pAttr->m_eType == eScoreType);

        huPri = 0;

        if (eScoreType == MyMJHuScoreTypeCpp::LocalCSBornNoJiang258) {

            if (count258Card <= 0) {
                huPri = MJHuCardTypePRiBornLocalNoJiang258;
                handCardMap.collectAll(aIDsShowOut);
            }

        }
        else if (eScoreType == MyMJHuScoreTypeCpp::LocalCSBornLackNumCardType) {

            if (countPerTypes[(uint8)MyMJCardValueTypeCpp::WangZi] <= 0 || countPerTypes[(uint8)MyMJCardValueTypeCpp::BingZi] <= 0 || countPerTypes[(uint8)MyMJCardValueTypeCpp::TiaoZi] <= 0) {

                huPri = MJHuCardTypePRiBornLocalLackNumCardType;
                handCardMap.collectAll(aIDsShowOut);
            }

        }
        else if (eScoreType == MyMJHuScoreTypeCpp::LocalCSBorn2KeZi) {

            if (aIDsKeZi.Num() >= 6) {

                huPri = MJHuCardTypePRiBornLocal2KeZi;
                if (showAllCards) {
                    handCardMap.collectAll(aIDsShowOut);
                }
                else {
                    aIDsShowOut = aIDsKeZi;
                }
            }

        }
        else if (eScoreType == MyMJHuScoreTypeCpp::LocalCSBornAnGangInHand) {

            if (aIDsAnGang.Num() >= 6) {

                huPri = MJHuCardTypePRiBornLocalAnGangInHand;
                if (showAllCards) {
                    handCardMap.collectAll(aIDsShowOut);
                }
                else {
                    aIDsShowOut = aIDsAnGang;
                }
            }

        }
        else {
            continue;
        }

        if (huPri > 0) {

            int32 idx = aFoundedHuAttrTemps.Emplace();
            FMyHuAttrTempIn *pTemp = &aFoundedHuAttrTemps[idx];

            pTemp->m_iPri = huPri;
            pTemp->m_pScoreAttr = pAttr;
            pTemp->m_aIDs = aIDsShowOut;

            if (huPriorityMaxFound < huPri) {
                huPriorityMaxFound = huPri;
            }
        }

    }


    l = aFoundedHuAttrTemps.Num();
    for (int32 i = 0; i < l; i++) {
        FMyHuAttrTempIn *pTemp = &aFoundedHuAttrTemps[i];

        if (!allowMultipleHu) {
            if (pTemp->m_iPri < huPriorityMaxFound) {
                continue;
            }
        }

        int32 idx = outHuScoreResultItems.Emplace();
        FMyMJHuScoreResultItemCpp *pItem = &outHuScoreResultItems[idx];
        pItem->m_eType = pTemp->m_pScoreAttr->m_eType;
        pItem->m_iScorePerAttender = pTemp->m_pScoreAttr->m_iScorePerAttender;
        pItem->m_iCount = 1;

        int32 lID = pTemp->m_aIDs.Num();
        for (int32 j = 0; j < lID; j++) {
            outShowOutCards.AddUnique(pTemp->m_aIDs[j]);
        }
    }


    return (outHuScoreResultItems.Num() > 0);
}

bool
UMyMJUtilsLocalCSLibrary::checkTingWithFixedCardTypeLocalCSInGame(const FMyMJHuCfgCpp &huCfg,
                                                              const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                                              const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                                              const FMyWeavesShowedOutStatisCpp &weavesShowedOutStatis,
                                                              const FMyMJValueIdMapCpp &handCardsMapIncludeTriggerCard,
                                                              FMyMJHuScoreResultTingCpp &outResultTing)

{
    FMyMJHuScoreResultTingCpp &outScoreResult = outResultTing;

    const FMyMJValueIdMapCpp *pTargetHandCardsMap = &handCardsMapIncludeTriggerCard;
    FMyMJValueIdMapCpp newMap;

    int32 i7Dui258DuiReq = localCSCfg.m_i7Dui258DuiReq;

    TArray<FMyMJWeaveArrayCpp> weaveCombinesHandCards;

    MyMJHuCardTypeCpp huCardTypeFoundMax = MyMJHuCardTypeCpp::Invalid;
    int32 huPriorityFoundMax = 0;


    FMyMJCardParseResultCpp parseResult;
    //FMyMJCardParseResultCpp *pComplexResult = &parseResult;
    FMyMJCardParseResultSimpleCpp *pSimpleResult = &parseResult;
    pTargetHandCardsMap->parseCards(false, parseResult);

    FMyStatisCountsPerCardValueTypeCpp *pStatisCounts0 = NULL, *pStatisCounts1 = NULL, *pStatisCounts2 = NULL;

    //handle weaves statis

    MyMJCardValueTypeCpp handCardUnifiedType = pTargetHandCardsMap->getUnifiedNumCardType();

    bool bQingYiSe;
    if (weavesShowedOutStatis.m_iWeaveCount > 0) {
        bQingYiSe = (weavesShowedOutStatis.m_eUnifiedNumCardType == handCardUnifiedType) && (handCardUnifiedType != MyMJCardValueTypeCpp::Invalid);
    }
    else {
        bQingYiSe = (handCardUnifiedType != MyMJCardValueTypeCpp::Invalid);
    }


    //TArray<MyMJHuCardTypeCpp> *pCardTypes = &huCfg.m_aHuCardTypesCfg;

    //int32 l = pCardTypes->Num();

    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> *pmHuAttrsCfg = &huCfg.getHuScoreAttrsRefConst();
    //FMyMJHuScoreAttrCpp **pAttr;

    FMyMJWeaveTreeNodeCpp rootNode;
    outScoreResult.reset();


    //pAttr = pHuAttrMap->Find(MyMJHuScoreTypeCpp::LocalCSAllJiang258);
    if (1) {
        //check local 258
        pStatisCounts0 = &pSimpleResult->m_cCardCounts;
        if (pStatisCounts0) {
            bool bHu = (pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258) >= 14);
            if (bHu) {
                bool bAdded = outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalCSAllJiang258);

                if (bAdded && huPriorityFoundMax < MJHuCardTypePRiLocalAllJiang258) {
                    huPriorityFoundMax = MJHuCardTypePRiLocalAllJiang258;
                    huCardTypeFoundMax = MyMJHuCardTypeCpp::LocalCSAllJiang258;
                    //outScoreResult.
                }
            }
        }
    }

    if (1) {
        //check 7 dui
        int32 DuiZiAll = 0;
        int32 DuiZiNum258All = 0;
        int32 GangAnCount = 0;

        pStatisCounts0 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::DuiZi);
        if (pStatisCounts0) {
            DuiZiAll += pStatisCounts0->getTotalCount();
            DuiZiNum258All += pStatisCounts0->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258);
        }

        pStatisCounts1 = pSimpleResult->m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
        if (pStatisCounts1) {
            GangAnCount += pStatisCounts1->getTotalCount();
            DuiZiAll += pStatisCounts1->getTotalCount() * 2;
            DuiZiNum258All += pStatisCounts1->getCountByCardValueType(MyMJCardValueTypeCpp::ExtNum258) * 2;
        }

        if (DuiZiAll >= 7 && DuiZiNum258All >= i7Dui258DuiReq) {
            //ditingiush its value
            bool bAdded = false;
            if (GangAnCount >= 2) {
                bAdded = outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalCSShuangHaoHuaQiDui);
            }

            if (!bAdded && GangAnCount >= 1) {
                bAdded = outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalCSHaoHuaQiDui);
            }

            if (!bAdded) {
                bAdded = outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::QiDui);
            }

            if (bAdded && huPriorityFoundMax < MJHuCardTypePRiSpecial7Dui) {
                huPriorityFoundMax = MJHuCardTypePRiSpecial7Dui;
                huCardTypeFoundMax = MyMJHuCardTypeCpp::Special7Dui;
            }
        }
    }

    if (1) {
        //check common hu

        bool bIsPengPengHu = false;
        bool bHu = false;

        //first test pengpengHu
        if (!weavesShowedOutStatis.m_bHaveShunZi) {
            //can test PengPengHu
            bHu = pTargetHandCardsMap->checkAllCanBeWeavedForHu(false, false, true, &rootNode);
            if (bHu) {
                bIsPengPengHu = true;
            }
        }

        //We can try again
        if (!bHu && bQingYiSe) {
            bHu = pTargetHandCardsMap->checkAllCanBeWeavedForHu(false, true, true, &rootNode);
        }


        //last, try common
        if (!bHu) {
            bHu = pTargetHandCardsMap->checkAllCanBeWeavedForHu(true, true, true, &rootNode);
        }


        if (bHu) {

            bool bAdded = false;
            if (bIsPengPengHu) {
                if (outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::PengPengHu)) {
                    bAdded = true;
                }
            }
            
            if (bQingYiSe) {
                if (outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::QingYiSe)) {
                    bAdded = true;
                }
            }


            if (outScoreResult.getScorePerAttenderTotal() <= 0) {
                if (outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::PingHu)) {
                    bAdded = true;
                }
            }


            if (bAdded && huPriorityFoundMax < MJHuCardTypePRiCommonHu) {
                huPriorityFoundMax = MJHuCardTypePRiCommonHu;
                huCardTypeFoundMax = MyMJHuCardTypeCpp::CommonHu;
            }
        }
    }

    //OK, if common hu is not possible,  it is for sure not jiangjianghu or 7dui card type, and it have only a chance for common hu
    if (huCardTypeFoundMax == MyMJHuCardTypeCpp::Invalid && weavesShowedOutStatis.m_iWeaveCount >= 4) {

        bool bHu = pTargetHandCardsMap->checkAllCanBeWeavedForHu(false, false, true, &rootNode);

        if (bHu) {
            outScoreResult.m_bBanZiMo = true;

            //PengPengHu Already checked before, see logic before
            bool bAdded = false;
            if (bQingYiSe) {
                if (outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::QingYiSe)) {
                    bAdded = true;
                }
            }

            if (outScoreResult.getScorePerAttenderTotal() <= 0) {
                if (outScoreResult.addType(*pmHuAttrsCfg, NULL, MyMJHuScoreTypeCpp::PingHu)) {
                    bAdded = true;
                }
            }

            if (bAdded && huPriorityFoundMax < MJHuCardTypePRiCommonHu) {
                huPriorityFoundMax = MJHuCardTypePRiCommonHu;
                huCardTypeFoundMax = MyMJHuCardTypeCpp::CommonHu;
            }
        }
    }

    MY_VERIFY((huCardTypeFoundMax == MyMJHuCardTypeCpp::Invalid) == (outScoreResult.getScorePerAttenderTotal() <= 0));

    if (huCardTypeFoundMax != MyMJHuCardTypeCpp::Invalid) {
        //outScoreResult.m_eHuCardType = huCardTypeFoundMax;
    }

    return (huCardTypeFoundMax != MyMJHuCardTypeCpp::Invalid);
}

bool
UMyMJUtilsLocalCSLibrary::calcScoreFinalLocalCSInGame(const FMyMJHuCfgCpp &huCfg,
                                                const FMyWeavesShowedOutStatisCpp &weavesShowedOutStatis,
                                                const FMyMJHuScoreResultTingCpp &inResultTing,
                                                const FMyMJHuActionAttrBaseCpp &inHuActionAttrBase,
                                                const FMyTriggerDataCpp *pTriggerData,
                                                FMyMJHuScoreResultFinalCpp &outScoreResultFinal)
{
    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> *pmHuScoreAttrsCfg = &huCfg.getHuScoreAttrsRefConst();

    bool bLastCardTurn = (inHuActionAttrBase.m_iCardNumCanBeTakenNormally == 0);
    bool bPao = (inHuActionAttrBase.m_iIdxAttenderLoseOnlyOne >= 0);


    outScoreResultFinal.reset();
    if (!inResultTing.isRealTingWhenPao() && bPao) {
        return false;
    }
    if (!inResultTing.isRealTingWhenZiMo() && !bPao) {
        return false;
    }

    if (pTriggerData) {
        //we have a trigger, verifying it is checked before and match
        MY_VERIFY(pTriggerData->m_iValue == inResultTing.m_iValueTriggerCard);
    }


    FMyMJHuScoreResultBaseCpp *pOutBase = StaticCast<FMyMJHuScoreResultBaseCpp *>(&outScoreResultFinal);
    *pOutBase = inResultTing;

    if (bLastCardTurn && inHuActionAttrBase.m_eTrigerCardActionType == MyMJHuTriggerCardSrcTypeCpp::CommonInGame) {
        //haidi
        bool bAdded;
        if (bPao) {
            bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalHaiDiPao);
        }
        else {
            bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::HaiDiLaoYue);
        }

        if (bAdded) {
            outScoreResultFinal.removeType(MyMJHuScoreTypeCpp::PingHu);
        }
    }

    if (inHuActionAttrBase.m_eTrigerCardActionType == MyMJHuTriggerCardSrcTypeCpp::GangWeaveShowedOut) {
        MY_VERIFY(bPao);
        bool bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::QiangGangHu);
        if (bAdded) {
            outScoreResultFinal.removeType(MyMJHuScoreTypeCpp::PingHu);
        }
    }

    if (inHuActionAttrBase.m_eTrigerCardActionType == MyMJHuTriggerCardSrcTypeCpp::GangCardTaken) {
        bool bAdded;
        if (bPao) {
            bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalGangShangPao);
        }
        else {
            bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::GangShangKaiHua);
        }

        if (bAdded) {
            outScoreResultFinal.removeType(MyMJHuScoreTypeCpp::PingHu);
        }
    }

    if (inHuActionAttrBase.m_iAttenderTurn <= 1 && bPao == false) {
        bool bAdded;
        if (inHuActionAttrBase.m_iIdxAttenderWin == inHuActionAttrBase.m_iMenFeng) {
            bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalTianHu);
        }
        else {
            bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::LocalDiHu);
        }


        if (bAdded) {
            outScoreResultFinal.removeType(MyMJHuScoreTypeCpp::PingHu);
        }
    }

    bool bMenQing = weavesShowedOutStatis.m_bMenQing;
    if (bMenQing && !bPao) {
        if (outScoreResultFinal.findByType(MyMJHuScoreTypeCpp::QiDui) == NULL &&
            outScoreResultFinal.findByType(MyMJHuScoreTypeCpp::LocalCSHaoHuaQiDui) == NULL &&
            outScoreResultFinal.findByType(MyMJHuScoreTypeCpp::LocalCSShuangHaoHuaQiDui) == NULL) {

            //not a 
            bool bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::MenQianQing);
            if (bAdded) {
                outScoreResultFinal.removeType(MyMJHuScoreTypeCpp::PingHu);
            }

        }
    }
    
    if (weavesShowedOutStatis.m_iWeaveCount == 4 && bPao) {
        bool bAdded = outScoreResultFinal.addType(*pmHuScoreAttrsCfg, NULL, MyMJHuScoreTypeCpp::QuanQiuRen);
        if (bAdded) {
            outScoreResultFinal.removeType(MyMJHuScoreTypeCpp::PingHu);
        }
    }

    MY_VERIFY(outScoreResultFinal.getScorePerAttenderTotal() > 0);

    if (pTriggerData) {
        //set it if we have trigger card
        outScoreResultFinal.m_cIdValueTriggerCard = *pTriggerData;
    }

    return true;
}


bool
UMyMJUtilsLocalCSLibrary::checkTingsLocalCSInGame(const FMyMJCardValuePackCpp &inValuePack,
                                            const FMyMJGameCfgCpp &gameCfg,
                                             const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                             const FMyMJValueIdMapCpp &handCardsMapExcludeTriggerCard,
                                             FMyMJHuScoreResultTingGroupCpp &outResultTingGroup)
{


    FMyWeavesShowedOutStatisCpp weaveStatis;
    UMyMJUtilsLibrary::getWeavesShowedOutStatis(inValuePack, weavesShowedOut, weaveStatis);

    FMyMJValueIdMapCpp tempHandCards;
    tempHandCards.copyDeep(&handCardsMapExcludeTriggerCard);

    FMyMJValueIdMapCpp& checkingHandCards = tempHandCards;

    FMyMJHuScoreResultTingCpp resultTing;

    outResultTingGroup.reset();

    //loop the values
    for (int32 cardValue = 0; cardValue < 30; cardValue++) {
        if ((cardValue % 10) == 0) {
            continue;
        }

        checkingHandCards.insert(MyIDFake, cardValue);
        if (UMyMJUtilsLocalCSLibrary::checkTingWithFixedCardTypeLocalCSInGame(gameCfg.m_cHuCfg, gameCfg.getSubLocalCSCfgRefConst(), weavesShowedOut, weaveStatis, checkingHandCards, resultTing)) {
            FMyMJHuScoreResultTingCpp &item = outResultTingGroup.findOrAdd(cardValue);
            item = resultTing;

            item.m_iValueTriggerCard = cardValue;
        }

        checkingHandCards.remove(MyIDFake, cardValue);
    }

    if (outResultTingGroup.getCount() > 0) {
        TArray<int32> aIds;
        checkingHandCards.collectAll(aIds);

        inValuePack.getValuesByIds(aIds, outResultTingGroup.getValuesHandCardWhenCheckingRef());

        return true;
    }
    else {
        return false;
    }
}


bool
UMyMJUtilsLocalCSLibrary::checkHuWithOutTriggerCard(const FMyMJCardValuePackCpp &inValuePack,
                                                const FMyMJHuCfgCpp &huCfg,
                                                const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                                const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                                const FMyMJValueIdMapCpp &handCardsMap,
                                                const FMyMJHuActionAttrBaseCpp &inHuActionAttrBase,
                                                FMyMJHuScoreResultFinalGroupCpp &outScoreResultFinalGroup)
{
    FMyMJHuScoreResultFinalGroupCpp *pGroup = &outScoreResultFinalGroup;
    pGroup->reset();
    pGroup->m_eHuMainType = MyMJHuMainTypeCpp::Common;

    if (!inHuActionAttrBase.getMayHu()) {
        return false;
    }

    FMyWeavesShowedOutStatisCpp weaveStatis;
    UMyMJUtilsLibrary::getWeavesShowedOutStatis(inValuePack, weavesShowedOut, weaveStatis);

    FMyMJHuScoreResultTingCpp resultTing;
 

    bool bHu = UMyMJUtilsLocalCSLibrary::checkTingWithFixedCardTypeLocalCSInGame(huCfg, localCSCfg, weavesShowedOut, weaveStatis, handCardsMap, resultTing);
    if (bHu) {

        int32 idx = pGroup->m_aScoreResults.Emplace();
        bHu = UMyMJUtilsLocalCSLibrary::calcScoreFinalLocalCSInGame(huCfg, weaveStatis, resultTing, inHuActionAttrBase, NULL, pGroup->m_aScoreResults[idx]);
        if (bHu) {

            UMyMJUtilsLocalCSLibrary::setHuScoreResultFinalGroupMetaData(weavesShowedOut, handCardsMap, inHuActionAttrBase, outScoreResultFinalGroup);

            return true;
        }
        else {
            pGroup->reset();
        }
    }

    return false;

}

bool
UMyMJUtilsLocalCSLibrary::checkHuWithTriggerCard(const FMyMJCardValuePackCpp &inValuePack,
                                            const FMyMJHuCfgCpp &huCfg,
                                            const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                            const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                            const FMyMJValueIdMapCpp &handCardsMap,
                                            const FMyMJHuActionAttrCpp &huActionAttr,
                                            const FMyMJHuScoreResultTingGroupCpp &tingGroup,
                                            FMyMJHuScoreResultFinalGroupCpp &outScoreResultFinalGroup)
{
    outScoreResultFinalGroup.reset();
    outScoreResultFinalGroup.m_eHuMainType = MyMJHuMainTypeCpp::Common;
    if (!huActionAttr.getMayHu()) {
        return false;
    }

    FMyWeavesShowedOutStatisCpp weaveStatis;
    UMyMJUtilsLibrary::getWeavesShowedOutStatis(inValuePack, weavesShowedOut, weaveStatis);

    const TArray<FMyTriggerDataCpp>  &aTriggerDatas = huActionAttr.m_aTriggerDatas;




    int32 l = aTriggerDatas.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyTriggerDataCpp &triggerData = aTriggerDatas[i];
        const FMyMJHuScoreResultTingCpp *pResultTing = tingGroup.findConst(triggerData.m_iValue);
        if (pResultTing == NULL) {
            continue;
        }

        int32 idx = outScoreResultFinalGroup.m_aScoreResults.Emplace();
        bool bHu = UMyMJUtilsLocalCSLibrary::calcScoreFinalLocalCSInGame(huCfg, weaveStatis, *pResultTing, huActionAttr, &triggerData, outScoreResultFinalGroup.m_aScoreResults[idx]);
        if (bHu) {

        }
        else {
            outScoreResultFinalGroup.m_aScoreResults.RemoveAt(idx);
        }

    }

    if (outScoreResultFinalGroup.m_aScoreResults.Num() <= 0) {
        return false;
    }

    UMyMJUtilsLocalCSLibrary::setHuScoreResultFinalGroupMetaData(weavesShowedOut, handCardsMap, huActionAttr, outScoreResultFinalGroup);

    return true;
}


void
UMyMJUtilsLocalCSLibrary::setHuScoreResultFinalGroupMetaData(const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                                        const FMyMJValueIdMapCpp &handCardsMap,
                                                        const FMyMJHuActionAttrBaseCpp &huActionAttrBase,
                                                        FMyMJHuScoreResultFinalGroupCpp &outScoreResultFinalGroup)
{
    outScoreResultFinalGroup.m_iIdxAttenderWin = huActionAttrBase.m_iIdxAttenderWin;
    outScoreResultFinalGroup.m_iIdxAttenderLoseOnlyOne = huActionAttrBase.m_iIdxAttenderLoseOnlyOne;
    outScoreResultFinalGroup.m_aWeavesShowedOut = weavesShowedOut;

    outScoreResultFinalGroup.m_aWeavesInHand.Reset();
    int32 idx = outScoreResultFinalGroup.m_aWeavesInHand.Emplace();
    FMyMJWeaveCpp *pWeave = &outScoreResultFinalGroup.m_aWeavesInHand[idx];
    pWeave->buildUnweavedInstanceFromValueIdMapCpp(handCardsMap);


    return;
}