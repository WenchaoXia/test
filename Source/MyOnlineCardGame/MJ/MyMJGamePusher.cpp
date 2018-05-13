// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePusher.h"
#include "MyMJGameData.h"
#include "MyMJGameCore.h"

#include "Utils/CommonUtils/MyCommonUtilsLibrary.h"

void
FMyMJGamePusherPointersCpp::copyDeep(const FMyMJGamePusherPointersCpp *pOther)
{
    clear();

    int32 l = pOther->m_aPushers.Num();;
    //MY_VERIFY(l > 0);

    for (int i = 0; i < l; i++) {
        FMyMJGamePusherBaseCpp *pNewRaw = pOther->m_aPushers[i]->cloneDeep();
        if (pNewRaw) {
            this->m_aPushers.Emplace(pNewRaw);
        }
        //TSharedPtr<FMyMJGamePusherBaseCpp> pNew = MakeShared<>
    }

    l = pOther->m_aPushersSharedPtr.Num();;
    //MY_VERIFY(l > 0);
    for (int i = 0; i < l; i++) {
        FMyMJGamePusherBaseCpp *pNewRaw = pOther->m_aPushersSharedPtr[i]->cloneDeep();
        if (pNewRaw) {
            this->m_aPushersSharedPtr.Emplace(pNewRaw);
        }
        //TSharedPtr<FMyMJGamePusherBaseCpp> pNew = MakeShared<>
    }

    //m_iCount = pOther->m_iCount;
    m_iTestCount = pOther->m_iTestCount;
    m_bSegmentClearTarget = pOther->m_bSegmentClearTarget;
    //MY_VERIFY(getCount() > 0);
}

bool FMyMJGamePusherPointersCpp::isStartsAsFullSequence() const
{
    MY_VERIFY(canProduceInLocalThreadSafely());

    int32 l = getCount();
    if (l > 0) {
        TSharedPtr<FMyMJGamePusherBaseCpp> pPusherShared0 = m_aPushersSharedPtr[0];
        if (pPusherShared0->getType() == MyMJGamePusherTypeCpp::PusherResetGame && m_aPushersSharedPtr[l - 1]->getId() == (l - 1)) {
            return true;
        }
        else {
            return false;
        }

    }

    return true;

}

//Possible not inserted, return < 0 means skipped
int32 FMyMJGamePusherPointersCpp::insertInLocalThreadWithLogicChecked(int32 iGameId, const TSharedPtr<FMyMJGamePusherBaseCpp> pPusher)
{
    int32 idx = -1;
    int32 iGameIdSelf = -1;
    int32 iPusherIdLastSelf = -1;
    MY_VERIFY(canProduceInLocalThreadSafely());

    MyMJGamePusherTypeCpp eType = pPusher->getType();

    if (eType == MyMJGamePusherTypeCpp::PusherResetGame) {
        clear();
        idx = insertInLocalThread(pPusher);
    }
    else {
        getGameIdAndPusherIdLast(&iGameIdSelf, &iPusherIdLastSelf);

        if (iPusherIdLastSelf >= 0 && (iPusherIdLastSelf + 1) == pPusher->getId() && (iGameId < 0 || (iGameId >= 0 && iGameId == iGameIdSelf))) {
            idx = insertInLocalThread(pPusher);
        }

    }

    if (idx >= 0) {
        if (idx != pPusher->getId()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("insertInLocalThreadWithLogicChecked(), pusher id not equal: %d, %d!"), idx, pPusher->getId());
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("insertInLocalThreadWithLogicChecked(), pusher skipped: %d:%d, %d:%d."), iGameIdSelf, iPusherIdLastSelf, iGameId, pPusher->getId());
    }


    return idx;
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

void FMyMJGamePusherPointersCpp::getGameIdAndPusherIdLast(int32 *pOutGameId, int32 *pOutPusherIdLast) const
{
    MY_VERIFY(canProduceInLocalThreadSafely());
    int32 gameId = -1, pusherIdLast = -1;
    int32 l = getCount();

    if (l > 0) {
        TSharedPtr<FMyMJGamePusherBaseCpp> pPusherShared0 = m_aPushersSharedPtr[0];
        if (pPusherShared0->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {

            if (m_aPushersSharedPtr[l - 1]->getId() == (l - 1)) {
                FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusherShared0.Get());
                gameId = pPusherResetGame->m_iGameId;
                pusherIdLast = l - 1;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getGameIdAndPusherId(), last element's id is not equal to its idx!"));
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getGameIdAndPusherId(), first element is not a reset game one!"));
        }
    }


    if (pOutGameId) {
        *pOutGameId = gameId;
    }

    if (pOutPusherIdLast) {
        *pOutPusherIdLast = pusherIdLast;
    }
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

bool FMyMJGamePusherPointersCpp::copyShallowAndLogicOptimized(const FMyMJGamePusherPointersCpp &other, bool bKeepDataIfOtherIsShorterWithSameSequence)
{
    bool bRet = true;
    int32 l0 = this->getCount();
    int32 l1 = other.getCount();
    if (isSamePusherSequenceSimple(other)) {

        if (l1 > l0) {
            for (int32 i = l0; i < l1; i++) {
                this->insertInLocalThread(other.getSharedPtrAtConst(i));
            }
        }
        else if (l1 == l0) {
            bRet = false;
        }
        else {
            if (bKeepDataIfOtherIsShorterWithSameSequence) {
                bRet = false;
            }
            else {
                this->m_aPushersSharedPtr = other.m_aPushersSharedPtr;
            }
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
    this->m_iTestCount = other.m_iTestCount;
    this->m_bSegmentClearTarget = other.m_bSegmentClearTarget;

    return bRet;
}

bool FMyMJGamePusherPointersCpp::helperFillAsSegmentFromQueue(TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>& pusherQueue)
{
    clear();
    MY_VERIFY(canProduceInLocalThreadSafely());

    FMyMJGamePusherBaseCpp* pPusherGot = NULL;

    bool bRet = false;
    while (pusherQueue.Dequeue(pPusherGot)) {
        bRet = true;
        pPusherGot->onReachedConsumeThread();
        int32 idx = giveInLocalThread(pPusherGot);
    }

    return bRet;
}


bool FMyMJGamePusherPointersCpp::helperTryFillDataFromSegment(int32 iGameIdOfSegment, const FMyMJGamePusherPointersCpp &segment, bool bVerifyAllInserted)
{
    bool bRet = false;

    if (segment.m_bSegmentClearTarget) {
        clear();
        return true;
    }

    int32 countSegment = segment.getCount();
    if (countSegment <= 0) {
        return false;
    }

    int32 iPusherIdStartSegment = segment.getSharedPtrAtConst(0)->getId();
    int32 idxSupposed = iPusherIdStartSegment;
    for (int32 i = 0; i < countSegment; i++) {
        MyMJGamePusherTypeCpp eType = segment.getSharedPtrAtConst(i)->getType();
        if (eType == MyMJGamePusherTypeCpp::PusherResetGame) {
            idxSupposed = 0;
        }

        if (!segment.getSharedPtrAtConst(i)->getId() == idxSupposed) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("helperTryFillDataFromSegment(), segment id not continues at idx %d."), i);
            if (bVerifyAllInserted) {
                MY_VERIFY(false);
            }
            return false;
        }

        idxSupposed++;
    }

    MY_VERIFY(canProduceInLocalThreadSafely());

    int32 iGameIdLatest = iGameIdOfSegment;

    for (int32 i = 0; i < countSegment; i++) {
        const TSharedPtr<FMyMJGamePusherBaseCpp> pPusher = segment.getSharedPtrAtConst(i);
        MyMJGamePusherTypeCpp eType = pPusher->getType();
        if (eType == MyMJGamePusherTypeCpp::PusherResetGame) {
            iGameIdLatest = StaticCastSharedPtr<FMyMJGamePusherResetGameCpp>(pPusher)->m_iGameId;
        }

        int32 idx = insertInLocalThreadWithLogicChecked(iGameIdLatest, pPusher);
        if (idx >= 0) {
            bRet = true;
        }
        else {
            if (bVerifyAllInserted) {
                MY_VERIFY(false);
            }
        }
    }
    
    return bRet;
}

bool FMyMJGamePusherPointersCpp::helperGenDeltaSegment(int32 iGameIdBase, int32 iPusherIdBase, int32 &outGameId, FMyMJGamePusherPointersCpp &outDeltaSegment)
{
    outGameId = -1;
    outDeltaSegment.clear();
    bool bRet = false;

    MY_VERIFY(canProduceInLocalThreadSafely());

    if (!isStartsAsFullSequence()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("helperGenDeltaSegment() but the source seems not in full sequence mode!"));
        return false;
    }

    int32 l = getCount();
    if (l <= 0) {
        if (iPusherIdBase < 0) {
            return false;
        }
        else {
            outDeltaSegment.m_bSegmentClearTarget = true;
            return true;
        }
    }

    //OK, we have content, tell the delta
    outDeltaSegment.m_bSegmentClearTarget = false;
    int32 iGameIdSelf;
    getGameIdAndPusherIdLast(&iGameIdSelf, NULL);
    outGameId = iGameIdSelf;

    if (iGameIdSelf == iGameIdBase && iPusherIdBase >= 0) {
        //the target have same sequence and contains some data, let's gen delta
        for (int32 i = (iPusherIdBase + 1); i < l; i++) {
            bRet = true;
            outDeltaSegment.insertInLocalThread(getSharedPtrAtConst(i));
        }
    }
    else {
        //target have different sequence or emty
        outDeltaSegment.copyShallowAndLogicOptimized(*this, false);
        bRet = true;
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

    UScriptStruct *pSC = StaticStruct();
    pSC->SerializeTaggedProperties(Ar, (uint8*)this, pSC, NULL);

    int32 l;
    bool bIsLoading;
    if (Ar.IsLoading()) {
        clear();
        bIsLoading = true;
    }
    else {
        if (!Ar.IsSaving()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("FMyMJGamePusherPointersCpp::trySerializeWithTag(): neither save or load"));
            return false;
        }
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
        else if (eType == MyMJGamePusherTypeCpp::PusherUpdateAttenderCardsAndState) {
            if (bIsLoading) {
                FMyMJGamePusherUpdateAttenderCardsAndStateCpp *pPusher0 = new FMyMJGamePusherUpdateAttenderCardsAndStateCpp();
                pS = pPusher0->StaticStruct();
                pPusherBase = StaticCast<FMyMJGamePusherBaseCpp *>(pPusher0);
            }
            else {
                FMyMJGamePusherUpdateAttenderCardsAndStateCpp *pPusher0 = StaticCast<FMyMJGamePusherUpdateAttenderCardsAndStateCpp *>(pPusherBase);
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
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unsupported type found in serialization, bIsLoading: %d, type: %s."), bIsLoading, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)eType));
            return false;
        }

        pS->SerializeTaggedProperties(Ar, reinterpret_cast<uint8*>(pPusherBase), pS, NULL);
        if (bIsLoading) {
            if (!pPusherBase) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("program error, data not set,  bIsLoading: %d, type: %s."), bIsLoading, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)eType));
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
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Serialize called, bIsLoading: %d"), bIsLoading);

    return trySerializeWithTag(Ar);
}

bool FMyMJGamePusherPointersCpp::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    bool bIsLoading = Ar.IsLoading();
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("NetSerialize called, bIsLoading: %d"), bIsLoading);

    return trySerializeWithTag(Ar);
}

bool FMyMJGamePusherPointersCpp::NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
{
    //bool bIsLoading = Ar.IsLoading();
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("NetDeltaSerialize called, W %p, R %p"), DeltaParms.Writer, DeltaParms.Reader);

    return false;
}

bool FMyMJGamePusherPointersCpp::SerializeFromMismatchedTag(FPropertyTag const& Tag, FArchive& Ar)
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("SerializeFromMismatchedTag %s"), *Tag.Name.ToString());
    return trySerializeWithTag(Ar);
}


/*
bool FMyMJGamePusherPointersCpp::ExportTextItem(FString& ValueStr, FMyMJGamePusherPointersCpp const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ExportTextItem"));
    return false;
}

bool FMyMJGamePusherPointersCpp::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* OwnerObject, FOutputDevice* ErrorText)
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ImportTextItem"));
    return false;
}
*/


FMyMJGamePusherBaseCpp*
FMyMJGamePusherFillInActionChoicesCpp::cloneDeep() const
{
    //First, attender only knows himself's choices
    FMyMJGamePusherFillInActionChoicesCpp *pRet = new FMyMJGamePusherFillInActionChoicesCpp();



    *StaticCast<FMyMJGamePusherBaseCpp *>(pRet) = *StaticCast<const FMyMJGamePusherBaseCpp *>(this);
    pRet->m_iActionGroupId = m_iActionGroupId;
    pRet->m_iIdxAttender = m_iIdxAttender;


    pRet->m_cActionChoices.copyDeep(&m_cActionChoices); //If datastream contains choice info(attender matches), always keep max info


    return pRet;
}

FMyMJGamePusherBaseCpp*
FMyMJGamePusherMadeChoiceNotifyCpp::cloneDeep() const
{
    FMyMJGamePusherMadeChoiceNotifyCpp *pRet = NULL;
 
    pRet = new FMyMJGamePusherMadeChoiceNotifyCpp();

    if (pRet) {
        *pRet = *this;
    }

    return pRet;
}

FMyMJGamePusherBaseCpp* 
FMyMJGamePusherResetGameCpp::cloneDeep() const
{
    FMyMJGamePusherResetGameCpp *pRet = new FMyMJGamePusherResetGameCpp();
    *pRet = *this;

    return pRet;
}

void
FMyMJGamePusherResetGameCpp::init(int32 iGameId, FRandomStream &RS, FMyMJGameCfgCpp &cGameCfg, FMyMJGameRunDataCpp &cGameRunData)
{
    m_iGameId = iGameId;
    m_cGameCfg = cGameCfg;
    m_cGameRunData = cGameRunData;

    TArray<int32> outValues;
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
    UMyCommonUtilsLibrary::shuffleArrayWithRandomStream<int32>(RS, outValues);

    /*
    int32 remainingCards = outValues.Num();

    while (remainingCards > 2) {
        int32 idxPicked = RS.RandRange(0, remainingCards - 2);
        int32 &pickedCardValue = outValues[idxPicked];
        int32 tempCardValue = pickedCardValue;
        outValues[idxPicked] = outValues[remainingCards - 1];
        outValues[remainingCards - 1] = tempCardValue;

        remainingCards--;
    }
    */

    m_aShuffledValues = outValues;

    /*
    int32 l = outValues.Num();
    m_aShuffledIdValues.Reset(l);
    for (int32 i = 0; i < l; i++) {
        int32 idx = m_aShuffledIdValues.Emplace();
        m_aShuffledIdValues[idx].m_iId = i;
        m_aShuffledIdValues[idx].m_iValue = outValues[i];
    }
    */
}

FMyMJGamePusherBaseCpp*
FMyMJGameActionThrowDicesCpp::cloneDeep() const
{
    FMyMJGameActionThrowDicesCpp *pRet = NULL;
    pRet = new FMyMJGameActionThrowDicesCpp();

    *pRet = *this;

    return pRet;
}

void FMyMJGameActionThrowDicesCpp::init(int32 iReason, int32 idxAttender, FRandomStream &RS, bool bForceActionGenTimeLeft2AutoChooseMsZero)
{
    m_iIdxAttender = idxAttender;

    int32 iDiceNumber0 = RS.RandRange(1, 6);
    int32 iDiceNumber1 = RS.RandRange(1, 6);

    if (iReason == FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_GangYaoLocalCS && !bForceActionGenTimeLeft2AutoChooseMsZero) {
        m_iTimeLeft2AutoChooseMs = ActionGenTimeLeft2AutoChooseMsForImportant;
    }

    UMyMJUtilsLibrary::setIntValueToBitMask(m_iDiceNumberMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen, iDiceNumber0);
    UMyMJUtilsLibrary::setIntValueToBitMask(m_iDiceNumberMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen, iDiceNumber1);
    UMyMJUtilsLibrary::setIntValueToBitMask(m_iDiceNumberMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen, iReason);
}


void FMyMJGameActionThrowDicesCpp::getDiceNumbers(int32 *poutDiceNumber0, int32 *poutDiceNumber1) const
{
    int32 iDiceNumber0 = UMyMJUtilsLibrary::getIntValueFromBitMask(m_iDiceNumberMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen);
    int32 iDiceNumber1 = UMyMJUtilsLibrary::getIntValueFromBitMask(m_iDiceNumberMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen);

    MY_VERIFY(iDiceNumber0 >= 1 && iDiceNumber0 < 7);
    MY_VERIFY(iDiceNumber1 >= 1 && iDiceNumber1 < 7);

    if (poutDiceNumber0) {
        *poutDiceNumber0 = iDiceNumber0;
    }
    if (poutDiceNumber1) {
        *poutDiceNumber1 = iDiceNumber1;
    }
}

int32 FMyMJGameActionThrowDicesCpp::getDiceReason() const
{
    return UMyMJUtilsLibrary::getIntValueFromBitMask(m_iDiceNumberMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen);
}

FMyMJGamePusherBaseCpp*
FMyMJGameActionDistCardAtStartCpp::cloneDeep() const
{
    FMyMJGameActionDistCardAtStartCpp *pRet = NULL;
    pRet = new FMyMJGameActionDistCardAtStartCpp();

    *pRet = *this;

    return pRet;
}

int32 FMyMJGameActionGiveOutCardsCpp::makeSubSelection(const TArray<int32> &subSelections)
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

int32 FMyMJGameActionGiveOutCardsCpp::genRandomSubSelections(FRandomStream &RS, TArray<int32> &outSubSelections)
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
        int32 randNo = RS.RandRange(0, c - 1);
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

    attender.getCoreRefConst().getCardValuePackOfSysKeeperRefConst().helperResolveValues(m_aIdValuePairsSelected, true);

    return;

}

