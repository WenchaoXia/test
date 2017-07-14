// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePusher.h"
#include "MyMJGameCore.h"

void
FMyMJGamePusherPointersCpp::copyDeepWithRoleType(const FMyMJGamePusherPointersCpp *pOther, MyMJGameRoleTypeCpp eRoleType)
{
    clear();

    int32 l = pOther->m_aPushers.Num();;
    //MY_VERIFY(l > 0);

    for (int i = 0; i < l; i++) {
        FMyMJGamePusherBaseCpp *pNewRaw = pOther->m_aPushers[i]->cloneDeepWithRoleType(eRoleType);
        if (pNewRaw) {
            this->m_aPushers.Emplace(pNewRaw);
        }
        //TSharedPtr<FMyMJGamePusherBaseCpp> pNew = MakeShared<>
    }

    l = pOther->m_aPushersSharedPtr.Num();;
    //MY_VERIFY(l > 0);
    for (int i = 0; i < l; i++) {
        FMyMJGamePusherBaseCpp *pNewRaw = pOther->m_aPushersSharedPtr[i]->cloneDeepWithRoleType(eRoleType);
        if (pNewRaw) {
            this->m_aPushersSharedPtr.Emplace(pNewRaw);
        }
        //TSharedPtr<FMyMJGamePusherBaseCpp> pNew = MakeShared<>
    }

    MY_VERIFY(getCount() > 0);
}

int32 FMyMJGamePusherPointersCpp::getGameIdVerified() const
{
    MY_VERIFY(canProduceInLocalThreadSafely());
    int32 l = getCount();
    MY_VERIFY(l > 0);

    TSharedPtr<FMyMJGamePusherBaseCpp> pPusherShared0 = m_aPushersSharedPtr[0];
    MY_VERIFY(pPusherShared0->getType() == MyMJGamePusherTypeCpp::PusherResetGame);
    FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusherShared0.Get());

    return pPusherResetGame->m_iGameId;
}

bool FMyMJGamePusherPointersCpp::isSamePusherSequenceSimple(const FMyMJGamePusherPointersCpp &other) const
{
    MY_VERIFY(canProduceInLocalThreadSafely());
    MY_VERIFY(other.canProduceInLocalThreadSafely());

    if (this->getCount() > 0 && other.getCount() > 0) {
        return (this->getGameIdVerified() == other.getGameIdVerified());
    }

    return false;
}

bool FMyMJGamePusherPointersCpp::copyShallowAndLogicOptimized(const FMyMJGamePusherPointersCpp &other)
{
    bool bRet = true;
    int32 l0 = this->getCount();
    int32 l1 = other.getCount();
    if (isSamePusherSequenceSimple(other)) {

        if (l1 > l0) {
            for (int32 i = l0; i < l1; i++) {
                this->giveInLocalThread(other.getSharedPtrAtConst(i));
            }
        }
        else if (l1 == l0) {
            bRet = false;
        }
        else {
            this->m_aPushersSharedPtr = other.m_aPushersSharedPtr;
        }

    }
    else {
        if (l1 == l0 && l1 == 0) {
            bRet = false;
        }
        else {
            this->m_aPushersSharedPtr = other.m_aPushersSharedPtr;
        }
    }

    MY_VERIFY(this->getCount() == other.getCount());

    return bRet;
}

bool FMyMJGamePusherPointersCpp::helperTrySyncDataFromCoreIOGroup(FMyMJGameIOGroupCpp *IOGroup)
{
    MY_VERIFY(canProduceInLocalThreadSafely());

    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>& pusherQueue = IOGroup->getPusherOutputQueue();
    FMyMJGamePusherBaseCpp* pPusherGot = NULL;

    bool bRet = false;
    while (pusherQueue.Dequeue(pPusherGot)) {
        bRet = true;
        pPusherGot->onReachedConsumeThread();
        MyMJGamePusherTypeCpp eType = pPusherGot->getType();
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("trySyncBufferFromPrev(), got type %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)eType));
        if (eType == MyMJGamePusherTypeCpp::PusherResetGame) {
            clear();
        }

        int32 idx = giveInLocalThread(MakeShareable<FMyMJGamePusherBaseCpp>(pPusherGot));

        if (idx != pPusherGot->getId()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("helperTrySyncDataFromCore(), pusher id not equal: %d, %d"), idx, pPusherGot->getId());
            MY_VERIFY(false);
            break;
        }
    }

    return bRet;
}

TSharedPtr<FMyMJGamePusherBaseCpp> FMyMJGamePusherPointersCpp::helperTryPullPusher(int32 iGameId, int32 iPusherId)
{
    if (iPusherId < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid pusherId %d."), iPusherId);
        return NULL;
    }

    int32 l = getCount();
    if (l > 0) {
        TSharedPtr<FMyMJGamePusherBaseCpp> pPusherShared0 = getSharedPtrAt(0);
        MY_VERIFY(pPusherShared0->getType() == MyMJGamePusherTypeCpp::PusherResetGame);
        FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusherShared0.Get());
        if (iGameId == pPusherResetGame->m_iGameId) {
            if (iPusherId < l) {
                return getSharedPtrAt(iPusherId);
            }
            else {
                return NULL;
            }
        }
        else {
            return pPusherShared0;
        }
    }

    return NULL;
}

bool FMyMJGamePusherPointersCpp::trySerializeWithTag(FArchive &Ar)
{

    if (!canProduceInLocalThreadSafely()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("fail to serialize since unsafe to do so."));
        return false;
    }

    int32 l;
    bool bIsLoading;
    if (Ar.IsLoading()) {
        clear();
        bIsLoading = true;
    }
    else {
        check(Ar.IsSaving());
        l = m_aPushersSharedPtr.Num();
        bIsLoading = false;
    }

    Ar << l;

    for (int32 i = 0; i < l; i++) {
        MyMJGamePusherTypeCpp eType = MyMJGamePusherTypeCpp::Invalid;
        FMyMJGamePusherBaseCpp *pPusherBase = NULL;
        if (bIsLoading) {

        }
        else {
            pPusherBase = m_aPushersSharedPtr[i].Get();
            eType = pPusherBase->getType();

        }
        Ar << eType;

        UScriptStruct *pS = NULL;
        if (eType == MyMJGamePusherTypeCpp::PusherFillInActionChoices) {
            if (bIsLoading) {
                FMyMJGamePusherFillInActionChoicesCpp *pPusher0 = new FMyMJGamePusherFillInActionChoicesCpp();
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherFillInActionChoicesCpp *pPusher0 = StaticCast<FMyMJGamePusherFillInActionChoicesCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();
            }
        }
        else if (eType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify) {
            if (bIsLoading) {
                FMyMJGamePusherMadeChoiceNotifyCpp *pPusher0 = new FMyMJGamePusherMadeChoiceNotifyCpp();
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherMadeChoiceNotifyCpp *pPusher0 = StaticCast<FMyMJGamePusherMadeChoiceNotifyCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();
            }
        }
        else if (eType == MyMJGamePusherTypeCpp::PusherCountUpdate) {
            if (bIsLoading) {
                FMyMJGamePusherCountUpdateCpp *pPusher0 = new FMyMJGamePusherCountUpdateCpp();
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherCountUpdateCpp *pPusher0 = StaticCast<FMyMJGamePusherCountUpdateCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
        }
        else if (eType == MyMJGamePusherTypeCpp::PusherResetGame) {
            if (bIsLoading) {
                FMyMJGamePusherResetGameCpp *pPusher0 = new FMyMJGamePusherResetGameCpp();
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherResetGameCpp *pPusher0 = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }
        }
        else if (eType == MyMJGamePusherTypeCpp::PusherUpdateCards) {
            if (bIsLoading) {
                FMyMJGamePusherUpdateCardsCpp *pPusher0 = new FMyMJGamePusherUpdateCardsCpp();
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherUpdateCardsCpp *pPusher0 = StaticCast<FMyMJGamePusherUpdateCardsCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();
            }

        }
        else if (eType == MyMJGamePusherTypeCpp::PusherUpdateTing) {
            if (bIsLoading) {
                FMyMJGamePusherUpdateTingCpp *pPusher0 = new FMyMJGamePusherUpdateTingCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherUpdateTingCpp *pPusher0 = StaticCast<FMyMJGamePusherUpdateTingCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }
        }

        else if (eType == MyMJGamePusherTypeCpp::ActionStateUpdate) {
            if (bIsLoading) {
                FMyMJGameActionStateUpdateCpp *pPusher0 = new FMyMJGameActionStateUpdateCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionStateUpdateCpp *pPusher0 = StaticCast<FMyMJGameActionStateUpdateCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionNoAct) {
            if (bIsLoading) {
                FMyMJGameActionNoActCpp *pPusher0 = new FMyMJGameActionNoActCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionNoActCpp *pPusher0 = StaticCast<FMyMJGameActionNoActCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionThrowDices) {
            if (bIsLoading) {
                FMyMJGameActionThrowDicesCpp *pPusher0 = new FMyMJGameActionThrowDicesCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionThrowDicesCpp *pPusher0 = StaticCast<FMyMJGameActionThrowDicesCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
            if (bIsLoading) {
                FMyMJGameActionDistCardAtStartCpp *pPusher0 = new FMyMJGameActionDistCardAtStartCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionDistCardAtStartCpp *pPusher0 = StaticCast<FMyMJGameActionDistCardAtStartCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionTakeCards) {
            if (bIsLoading) {
                FMyMJGameActionTakeCardsCpp *pPusher0 = new FMyMJGameActionTakeCardsCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionTakeCardsCpp *pPusher0 = StaticCast<FMyMJGameActionTakeCardsCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }
        }
        else if (eType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
            if (bIsLoading) {
                FMyMJGameActionGiveOutCardsCpp *pPusher0 = new FMyMJGameActionGiveOutCardsCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionGiveOutCardsCpp *pPusher0 = StaticCast<FMyMJGameActionGiveOutCardsCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionWeave) {
            if (bIsLoading) {
                FMyMJGameActionWeaveCpp *pPusher0 = new FMyMJGameActionWeaveCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionWeaveCpp *pPusher0 = StaticCast<FMyMJGameActionWeaveCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionHu) {
            if (bIsLoading) {
                FMyMJGameActionHuCpp *pPusher0 = new FMyMJGameActionHuCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionHuCpp *pPusher0 = StaticCast<FMyMJGameActionHuCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {
            if (bIsLoading) {
                FMyMJGameActionHuBornLocalCSCpp *pPusher0 = new FMyMJGameActionHuBornLocalCSCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionHuBornLocalCSCpp *pPusher0 = StaticCast<FMyMJGameActionHuBornLocalCSCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else if (eType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {
            if (bIsLoading) {
                FMyMJGameActionZhaNiaoLocalCSCpp *pPusher0 = new FMyMJGameActionZhaNiaoLocalCSCpp();
                pS = pPusher0->StaticStruct();

                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGameActionZhaNiaoLocalCSCpp *pPusher0 = StaticCast<FMyMJGameActionZhaNiaoLocalCSCpp *>(pPusherBase);
                pS = pPusher0->StaticStruct();

            }

        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unsupported type found in serialization, bIsLoading: %d, type: %s."), bIsLoading, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)eType));
            return false;
        }

        pS->SerializeTaggedProperties(Ar, reinterpret_cast<uint8*>(pPusherBase), pS, NULL);
        if (bIsLoading) {
            if (!pPusherBase) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("program error, data not set,  bIsLoading: %d, type: %s."), bIsLoading, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)eType));
                MY_VERIFY(false);
            }
            giveInLocalThread(pPusherBase);
        }
        else {
        }

    }

    return true;
}

bool FMyMJGamePusherPointersCpp::Serialize(FArchive& Ar)
{
    bool bIsLoading = Ar.IsLoading();
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Serialize called, bIsLoading: %d"), bIsLoading);

    return trySerializeWithTag(Ar);
}

bool FMyMJGamePusherPointersCpp::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    bool bIsLoading = Ar.IsLoading();
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("NetSerialize called, bIsLoading: %d"), bIsLoading);

    return trySerializeWithTag(Ar);
}

bool FMyMJGamePusherPointersCpp::NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
{
    //bool bIsLoading = Ar.IsLoading();
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("NetDeltaSerialize called, W %p, R %p"), DeltaParms.Writer, DeltaParms.Reader);

    return false;
}


FMyMJGamePusherBaseCpp*
FMyMJGamePusherFillInActionChoicesCpp::cloneDeepWithRoleType(MyMJGameRoleTypeCpp eRoleType) const
{
    //First, attender only knows himself's choices
    FMyMJGamePusherFillInActionChoicesCpp *pRet = new FMyMJGamePusherFillInActionChoicesCpp();
    bool bFillIn = false;
    if (eRoleType == MyMJGameRoleTypeCpp::SysKeeper) {
        bFillIn = true;
    }
    else if ((uint8)eRoleType < 4) {
        if (m_iIdxAttender == (uint8)eRoleType) {
            bFillIn = true;
        }
    }

    *StaticCast<FMyMJGamePusherBaseCpp *>(pRet) = *StaticCast<const FMyMJGamePusherBaseCpp *>(this);
    pRet->m_iActionGroupId = m_iActionGroupId;
    pRet->m_iIdxAttender = m_iIdxAttender;

    if (bFillIn) {
        pRet->m_cActionChoices.copyDeepWithRoleType(&m_cActionChoices, MyMJGameRoleTypeCpp::SysKeeper); //If datastream contains choice info(attender matches), always keep max info
    }

    return pRet;
}

FMyMJGamePusherBaseCpp*
FMyMJGamePusherMadeChoiceNotifyCpp::cloneDeepWithRoleType(MyMJGameRoleTypeCpp eRoleType) const
{
    FMyMJGamePusherMadeChoiceNotifyCpp *pRet = NULL;
    if (eRoleType == MyMJGameRoleTypeCpp::SysKeeper) {
        pRet = new FMyMJGamePusherMadeChoiceNotifyCpp();
    }
    else if ((uint8)eRoleType < 4) {
        if (m_iIdxAttender == (uint8)eRoleType) {
            pRet = new FMyMJGamePusherMadeChoiceNotifyCpp();
        }
    }

    if (pRet) {
        *pRet = *this;
    }

    return pRet;
}

FMyMJGamePusherBaseCpp* 
FMyMJGamePusherResetGameCpp::cloneDeepWithRoleType(MyMJGameRoleTypeCpp eRoleType) const
{
    FMyMJGamePusherResetGameCpp *pRet = new FMyMJGamePusherResetGameCpp();
    *pRet = *this;

    if (eRoleType != MyMJGameRoleTypeCpp::SysKeeper) {
        int l = pRet->m_aShuffledValues.Num();
        for (int i = 0; i < l; i++) {
            pRet->m_aShuffledValues[i] = 0; //0 means unknown
        }
    }

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FMyMJGamePusherResetGameCpp clones with roleType %d"), (uint8)eRoleType);

    return pRet;
}

void
FMyMJGamePusherResetGameCpp::init(int32 iGameId, FRandomStream *pRandomStream, FMyMJGameCfgCpp &cGameCfg, FMyMJGameRunDataCpp &cGameRunData, int32 iAttenderBehaviorRandomSelectMask)
{
    m_iGameId = iGameId;
    m_cGameCfg = cGameCfg;
    m_cGameRunData = cGameRunData;
    m_iAttenderBehaviorRandomSelectMask = iAttenderBehaviorRandomSelectMask;

    TArray<int32> &outValues = m_aShuffledValues;
    outValues.Reset(160);


    bool bHaveWordCards = m_cGameCfg.m_cCardPackCfg.m_bHaveWordCards;
    bool bHaveHuaCards = m_cGameCfg.m_cCardPackCfg.m_bHaveHuaCards;
    bool bHaveZhong = m_cGameCfg.m_cCardPackCfg.m_bHaveZhongCards;

    int32 idx;
    int32 cardValue;

    for (int32 i = 0; i < 3; i++) {
        for (int32 j = 1; j < 10; j++) {
            cardValue = i * 10 + j;

            for (int32 k = 0; k < 4; k++) {
                idx = outValues.Emplace();
                outValues[idx] = cardValue;
            }
        }
    }

    if (bHaveWordCards) {
        for (cardValue = 31; cardValue < 35; cardValue++) {

            for (int32 k = 0; k < 4; k++) {
                idx = outValues.Emplace();
                outValues[idx] = cardValue;
            }

        }

        for (cardValue = 41; cardValue < 44; cardValue++) {

            for (int32 k = 0; k < 4; k++) {
                idx = outValues.Emplace();
                outValues[idx] = cardValue;
            }

        }
    }

    if (bHaveHuaCards) {
        for (cardValue = 51; cardValue < 59; cardValue++) {

            idx = outValues.Emplace();
            outValues[idx] = cardValue;
        }
    }

    if (bHaveZhong && !bHaveWordCards) {
        cardValue = 41;
        for (int32 k = 0; k < 4; k++) {
            idx = outValues.Emplace();
            outValues[idx] = cardValue;
        }
    }

    //let's shuffle
    int32 remainingCards = outValues.Num();

    while (remainingCards > 2) {
        int32 idxPicked = pRandomStream->RandRange(0, remainingCards - 2);
        int32 &pickedCardValue = outValues[idxPicked];
        int32 tempCardValue = pickedCardValue;
        outValues[idxPicked] = outValues[remainingCards - 1];
        outValues[remainingCards - 1] = tempCardValue;

        remainingCards--;
    }

}

FMyMJGamePusherBaseCpp*
FMyMJGameActionThrowDicesCpp::cloneDeepWithRoleType(MyMJGameRoleTypeCpp eRoleType) const
{
    FMyMJGameActionThrowDicesCpp *pRet = NULL;
    pRet = new FMyMJGameActionThrowDicesCpp();

    *pRet = *this;

    return pRet;
}

void FMyMJGameActionThrowDicesCpp::init(MyMJGameActionThrowDicesSubTypeCpp eSubType, int32 idxAttender, FRandomStream *pRandomStream)
{
    m_eSubType = eSubType;
    m_iIdxAttender = idxAttender;
    m_iDiceNumber0 = pRandomStream->RandRange(1, 6);
    m_iDiceNumber1 = pRandomStream->RandRange(1, 6);
}

void FMyMJGameActionThrowDicesCpp::getDiceNumbers(int32 &outDiceNumber0, int32 &outDiceNumber1) const
{
    MY_VERIFY(m_iDiceNumber0 >= 1 && m_iDiceNumber0 < 7);
    MY_VERIFY(m_iDiceNumber1 >= 1 && m_iDiceNumber1 < 7);

    outDiceNumber0 = m_iDiceNumber0;
    outDiceNumber1 = m_iDiceNumber1;
}

MyMJGameActionThrowDicesSubTypeCpp FMyMJGameActionThrowDicesCpp::getSubType() const
{
    return m_eSubType;
}

FMyMJGamePusherBaseCpp*
FMyMJGameActionDistCardAtStartCpp::cloneDeepWithRoleType(MyMJGameRoleTypeCpp eRoleType) const
{
    FMyMJGameActionDistCardAtStartCpp *pRet = NULL;
    pRet = new FMyMJGameActionDistCardAtStartCpp();

    *pRet = *this;

    if (eRoleType == MyMJGameRoleTypeCpp::SysKeeper) {

    }
    else if ((uint8)eRoleType < 4 && m_iIdxAttender == (uint8)eRoleType) {
        //himself's data
    }
    else {
        //erase the value data
        int32 l = pRet->m_aIdValues.Num();
        for (int32 i = 0; i < l; i++) {
            pRet->m_aIdValues[i].m_iValue = 0;
        }
    }

    return pRet;
}

int32 FMyMJGameActionGiveOutCardsCpp::makeSubSelection(TArray<int32> &subSelections)
{


    if (m_bRestrict2SelectCardsJustTaken) {
        return -4;
    }
    else {
        if (subSelections.Num() != 1) {
            return -1;
        }

        int32 selectedId = subSelections[0];
        if (selectedId < 0) {
            return -2;
        }

        if (m_aOptionIdsHandCard.Find(selectedId) == INDEX_NONE && m_aOptionIdsJustTaken.Find(selectedId) == INDEX_NONE) {
            return -3;
        }

        m_aIdValuePairsSelected.Reset();
        int32 idx = m_aIdValuePairsSelected.Emplace();
        m_aIdValuePairsSelected[idx].m_iId = selectedId;
    }

    return 0;
};

int32 FMyMJGameActionGiveOutCardsCpp::genRandomSubSelections(FRandomStream *pRandomStream, TArray<int32> &outSubSelections)
{
    outSubSelections.Reset();
    if (m_bRestrict2SelectCardsJustTaken) {
        MY_VERIFY(false);
        return -10;
    }
    else {


        int32 c = getRealCountOfSelection();
        if (c < 1) {
            return -1;
        }

        int32 count0 = m_aOptionIdsHandCard.Num();
        int32 count1 = m_aOptionIdsJustTaken.Num();

        int32 retSelectedId;
        int32 randNo = pRandomStream->RandRange(0, c - 1);
        if (randNo < count0) {
            retSelectedId = m_aOptionIdsHandCard[randNo];
        }
        else {
            MY_VERIFY(count1 > 0);
            int idx = randNo - count0;
            MY_VERIFY(idx < count1);
            retSelectedId = m_aOptionIdsJustTaken[idx];
        }

        outSubSelections.Emplace(retSelectedId);

        return 0;
    }
}

void FMyMJGameActionGiveOutCardsCpp::resolveActionResult(FMyMJGameAttenderCpp &attender)
{
    MY_VERIFY(m_aIdValuePairsSelected.Num() > 0);

    attender.getpCore()->getpCardPack()->helperResolveValues(m_aIdValuePairsSelected);

    TArray<FMyIdValuePair>& aTargets = m_aIdValuePairsSelected;
    int32 l = aTargets.Num();
    MY_VERIFY(l > 0);
    for (int32 i = 0; i < l; i++) {
        MY_VERIFY(aTargets[i].m_iId >= 0);
        MY_VERIFY(aTargets[i].m_iValue > 0);
    }
    return;

}

void FMyMJGameActionHuCpp::resolveActionResult(FMyMJGameAttenderCpp &attender)
{
    m_aRevealingCards.Reset();

    FMyMJCardPackCpp *pCardPack = attender.getpCore()->getpCardPack();
    int32 l = pCardPack->getLength();
    for (int32 i = 0; i < l; i++) {
        FMyMJCardCpp *pCard = pCardPack->getCardByIdx(i);
        if (pCard->m_eFlipState == MyMJCardFlipStateCpp::Up) {
            //already revealed
            continue;
        }

        int32 idx = m_aRevealingCards.Emplace();
        m_aRevealingCards[idx].m_iId = pCard->m_iId;
        pCardPack->helperResolveValue(m_aRevealingCards[idx]);

    }

}
