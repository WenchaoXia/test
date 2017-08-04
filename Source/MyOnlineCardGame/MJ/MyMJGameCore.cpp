// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCore.h"
#include "Kismet/KismetMathLibrary.h"


int32
FMyMJGameCoreBaseCpp::findIdxAttenderStillInGame(int32 idxAttenderBase, uint32 delta, bool bReverse)
{
    TSharedPtr<FMyMJGameAttenderCpp> pAttender;

    MY_VERIFY(idxAttenderBase >= 0 && idxAttenderBase < 4);
    MY_VERIFY(delta < 4);

    bool bFound = false;
    int32 idxChecking = idxAttenderBase;

    for (int i = 0; i < 4; i++) {
        pAttender = m_aAttendersAll[idxChecking];

        if (pAttender->getIsStillInGameRef()) {
            //valid one found
            bFound = true;
            if (delta == 0) {
                break;
            }
            else {
                delta--;
            }
        }

        //goto next
        if (bReverse) {
            idxChecking = (idxChecking - 1 + 4) % 4;
        }
        else {
            idxChecking = (idxChecking + 1 + 4) % 4;
        }
    }

    MY_VERIFY(delta == 0);
    MY_VERIFY(bFound);

    return idxChecking;
}

int32 FMyMJGameCoreBaseCpp::genIdxAttenderStillInGameMaskOne(int32 idxAttender)
{
    int32 ret = 0;
    TSharedPtr<FMyMJGameAttenderCpp> pAttender;

    for (int i = 0; i < 4; i++) {
        pAttender = m_aAttendersAll[i];

        if (idxAttender == i) {
            MY_VERIFY(pAttender->getIsStillInGameRef());
            ret |= (1 << i);
            break;
        }

    }

    MY_VERIFY(ret > 0);
    return ret;
}

int32 FMyMJGameCoreBaseCpp::genIdxAttenderStillInGameMaskExceptOne(int32 idxAttender)
{
    int32 ret = 0;
    TSharedPtr<FMyMJGameAttenderCpp> pAttender;

    for (int i = 0; i < 4; i++) {
        pAttender = m_aAttendersAll[i];

        if (idxAttender != i) {
            if (pAttender->getIsStillInGameRef()) {
                ret |= (1 << i);
            }
        }

    }

    MY_VERIFY(ret > 0);
    return ret;
}

int32 FMyMJGameCoreBaseCpp::genIdxAttenderStillInGameMaskAll()
{
    int32 ret = 0;
    TSharedPtr<FMyMJGameAttenderCpp> pAttender;

    for (int i = 0; i < 4; i++) {
        pAttender = m_aAttendersAll[i];

        if (pAttender->getIsStillInGameRef()) {
            ret |= (1 << i);
        }

    }

    MY_VERIFY(ret > 0);
    return ret;
}



int32 FMyMJGameCoreCpp::calcUntakenSlotCardsLeftNumKeptFromTail()
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    FMyMJGameUntakenSlotInfoCpp *pInfo = &pD->m_cUntakenSlotInfo;

    int32 keptCount = pD->m_cGameCfg.m_cTrivialCfg.m_iStackNumKeptFromTail;
    int32 idxTail = pInfo->m_iIdxUntakenSlotTailAtStart;
    int32 totalL = pInfo->m_iUntakenSlotCardsLeftNumTotal;

    int32 idxWorking = idxTail;
    int32 stack2check = keptCount;

    int32 ret = 0;

    MY_VERIFY(totalL > 0);
    MY_VERIFY(keptCount >= 0);
    MY_VERIFY(idxTail >= 0);

    while (stack2check > 0) {

        ret += pD->m_aUntakenCardStacks[idxWorking].m_aIds.Num();


        idxWorking = (idxWorking - 1 + totalL) % totalL;
        stack2check--;
    }

    return ret;
}

bool FMyMJGameCoreCpp::isIdxUntakenSlotInKeptFromTailSegment(int32 idx)
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    FMyMJGameUntakenSlotInfoCpp *pInfo = &pD->m_cUntakenSlotInfo;

    int32 keptCount = pD->m_cGameCfg.m_cTrivialCfg.m_iStackNumKeptFromTail;
    int32 idxTail = pInfo->m_iIdxUntakenSlotLengthAtStart;
    int32 totalL = pInfo->m_iIdxUntakenSlotLengthAtStart;

    if (keptCount <= 0) {
        return false;
    }

    MY_VERIFY(totalL > 0);
    MY_VERIFY(idxTail >= 0);

    int32 idxEnd = (idxTail + 1) % totalL; // use < 
    int32 idxStart = (idxEnd - keptCount + totalL) % totalL;

    int32 idxStartFix0, idxStartFix1, idxEndFix0, idxEndFix1;
    if (idxStart < idxEnd) {
        idxStartFix0 = idxStart;
        idxEndFix0 = idxEnd;
        idxStartFix1 = 0;
        idxEndFix1 = 0;
    }
    else {
        MY_VERIFY(idxStart != idxEnd);
        idxStartFix0 = idxStart;
        idxEndFix0 = totalL;
        idxStartFix1 = 0;
        idxEndFix1 = idxEnd;
    }

    return (idxStartFix0 <= idx && idx < idxEndFix0) || (idxStartFix1 <= idx && idx < idxEndFix1);
}


//don't reenter this func, this may result stack overflow
bool FMyMJGameCoreCpp::actionLoop()
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    bool bRet = false;
    int64 iMsLast = UMyMJUtilsLibrary::nowAsMsFromTick();
    int64 iTimePassedMs64 = iMsLast - m_iMsLast;
    if (iTimePassedMs64 < 0) {
        iTimePassedMs64 = 0;
    }
    m_iMsLast = iMsLast;
    int32 iTimePassedMs32 = iTimePassedMs64 > MAX_int32 ? MAX_int32 : (int32)iTimePassedMs64;

    bRet |= findAndApplyPushers();
    if (findAndHandleCmd()) {
        bRet |= findAndApplyPushers();
    }

    MyMJActionLoopStateCpp &eActionLoopState = pD->m_eActionLoopState;

    TSharedPtr<FMyMJGameActionCollectorCpp>  &pCollector = m_pActionCollector;

    if (eActionLoopState == MyMJActionLoopStateCpp::WaitingToGenAction) {
        //gen action
        genActionChoices();
        eActionLoopState = MyMJActionLoopStateCpp::ActionGened;
        return true;
    }
    else if (eActionLoopState == MyMJActionLoopStateCpp::ActionGened) {
        //collect action
        bool bHaveProgres;
        MY_VERIFY(m_pResManager.IsValid());
        bool bAllCollected = pCollector->collectAction(iTimePassedMs32, bHaveProgres, m_pResManager->getRandomStreamRef());

        if (bAllCollected) {
            eActionLoopState = MyMJActionLoopStateCpp::ActionCollected;
        }

        bRet |= bHaveProgres;

        return bRet;
    }
    else if (eActionLoopState == MyMJActionLoopStateCpp::ActionCollected) {
        eActionLoopState = MyMJActionLoopStateCpp::WaitingToGenAction;
        return true;
    }

    return false;
}

bool FMyMJGameCoreCpp::findAndApplyPushers()
{
    bool bRet = false;
   

    //this is the end of pusher's life cycle
    int32 iGameId, iPusherIdLast;
    getGameIdAndPusherIdLast(&iGameId, &iPusherIdLast);

    TSharedPtr<FMyMJGamePusherBaseCpp> pPusherShared = m_pPusherIOFull->tryPullPusherFromLocal();


    while (pPusherShared.IsValid()) {

        bRet = true;

        /*
        MY_VERIFY(m_pCoreMirror->getWorkMode() == MyMJGameCoreWorkModeCpp::Mirror);

        int32 iAttenderMask;
        TArray<FMyIdValuePair> aRevealedCardValues;

        pPusher->getRevealedCardValues(iAttenderMask, aRevealedCardValues);
        if (iAttenderMask != 0) {


            int32 l;
            //l = MY_GET_ARRAY_LEN(m_aAttendersAll);
            l = m_aAttenderPawns.Num();

            MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);

            for (int32 i = 0; i < l; i++) {
                if ((iAttenderMask & (1 << i)) == 0) {
                    continue;
                }
                //need update
                m_aAttendersAll[i]->getDataPrivateDirect()->m_cCardValuePack.tryRevealCardValueByIdValuePairs(aRevealedCardValues);
            }
        }
        */

        makeProgressByPusher(pPusherShared.Get());

        getGameIdAndPusherIdLast(&iGameId, &iPusherIdLast);
        pPusherShared = m_pPusherIOFull->tryPullPusherFromLocal();
    }

    return bRet;
}

bool FMyMJGameCoreCpp::findAndHandleCmd()
{
    if (m_eWorkMode != MyMJGameCoreWorkModeCpp::Full) {
        return false;
    }

    MY_VERIFY(m_pCmdIO.IsValid());
    MY_VERIFY(m_pPusherIOFull.IsValid());
    bool bNewCmdGot = false;


    FMyMJGameCmdBaseCpp *pCmd;
    int32 l = m_pCmdIO->m_aIOs.Num();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGameCoreLocalCSCpp::findAndHandlecmdWhenFullMode(), l: %d."), l);

    for (int32 i = 0; i < l; i++) {
        MyMJGameRoleTypeCpp eRoleType = (MyMJGameRoleTypeCpp)i;
        FMyMJGameCmdIOCpp *pCmdIO = &m_pCmdIO->m_aIOs[i];
        while (pCmdIO->m_pInputQueueRemote->Dequeue(pCmd)) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("recieved cmd, role %d, cmd %d"), (uint8)eRoleType, (uint8)pCmd->m_eType);
            bNewCmdGot = true;

            handleCmd(eRoleType, pCmd);

            //found one cmd, and the cmd life cycle is also manager by external, return it
            pCmdIO->m_pOutputQueueRemote->Enqueue(pCmd);
        }
    }

    return bNewCmdGot;
}

void FMyMJGameCoreCpp::genActionChoices()
{
    MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Full);
    MY_VERIFY(m_pPusherIOFull.IsValid());


    for (int i = 0; i < 4; i++) {
        FMyMJGameAttenderCpp *pAttender = m_aAttendersAll[i].Get();
        MY_VERIFY(pAttender);
        if (!pAttender->getIsStillInGameRef()) {
            continue;
        }

        FMyMJGameActionContainorCpp *pContainor = pAttender->getActionContainor();
        if (pContainor->getNeed2Collect() == true) {
            pAttender->genActionChoices(m_pPusherIOFull.Get());
        }
    }
};

void FMyMJGameCoreCpp::resetForNewLoop(FMyMJGameActionBaseCpp *pPrevAction, FMyMJGameActionBaseCpp *pPostAction, int32 iIdxAttenderMask, bool bAllowSamePriAction, int32 iIdxAttenderHavePriMax)
{
    //step1: reset containors:
    int32 iRealAttenderCount = 0;
    for (int i = 0; i < 4; i++) {
        FMyMJGameAttenderCpp *pAttender = m_aAttendersAll[i].Get();
        MY_VERIFY(pAttender);
        if (!pAttender->getIsStillInGameRef()) {
            continue;
        }
        iRealAttenderCount++;
        FMyMJGameActionContainorCpp *pContainor = pAttender->getActionContainor();
        pContainor->resetForNewLoop();
        int32 idxAttender = pAttender->getIdx();
        MY_VERIFY(i == idxAttender);
        if ((iIdxAttenderMask & (1 << idxAttender)) > 0) {
            pContainor->getNeed2Collect() = true;
        }
        else {
            pContainor->getNeed2Collect() = false;
        }
    }

    if (m_eWorkMode != MyMJGameCoreWorkModeCpp::Full) {
        MY_VERIFY(!m_pActionCollector.IsValid());
        return;
    }

    MY_VERIFY(m_pActionCollector.IsValid());

    //step2: reset collector
    m_pActionCollector->resetForNewLoopForFullMode(pPrevAction, pPostAction, bAllowSamePriAction, iIdxAttenderHavePriMax, iRealAttenderCount);
}

/*
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformSplash.h"
#elif PLATFORM_PS4
#include "PS4/PS4Splash.h"
#elif PLATFORM_XBOXONE
#include "XboxOne/XboxOneSplash.h"
#elif PLATFORM_MAC
#include "Mac/MacPlatformSplash.h"
#elif PLATFORM_IOS
#include "IOS/IOSPlatformSplash.h"
#elif PLATFORM_ANDROID
#include "Android/AndroidSplash.h"
#elif PLATFORM_WINRT
#include "WinRT/WinRTSplash.h"
#elif PLATFORM_HTML5
#include "HTML5/HTML5PlatformSplash.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxPlatformSplash.h"
#endif
*/

//#define MY_GET_FILENAME(FULL_PATH_AND_NAME)

void FMyMJGameCoreCpp::tryProgressInFullMode()
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    //the sequence ensure pusher is drained out clean before cmd applies
    findAndApplyPushers();
    if (findAndHandleCmd()) {
        findAndApplyPushers();
    }

    if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {

        bool bIsInGameState = pD->isInGameState();
        bool bHaveProgress = true;

        while (bIsInGameState && bHaveProgress) {
            bHaveProgress = actionLoop();
            bIsInGameState = pD->isInGameState();
        }
    }
    else {
        MY_VERIFY(false);
    }
}

int32 FMyMJGameCoreCpp::getIdxOfUntakenSlotHavingCard(int32 idxBase, uint32 delta, bool bReverse)
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    int32 l = pD->m_aUntakenCardStacks.Num();
    MY_VERIFY(idxBase >= 0 && idxBase < l);

    int32 idxChecking = idxBase;
    for (int32 i = 0; i < l; i++) {
        FMyIdCollectionCpp *pC = &pD->m_aUntakenCardStacks[idxChecking];
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

void FMyMJGameCoreCpp::collectCardsFromUntakenSlot(int32 idxBase, uint32 len, bool bReverse, TArray<int32> &outIds)
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    int32 l = pD->m_aUntakenCardStacks.Num();
    MY_VERIFY(idxBase >= 0 && idxBase < l);
    MY_VERIFY(len > 0 && len < 20);
    outIds.Empty();

    int32 idxChecking = getIdxOfUntakenSlotHavingCard(idxBase, 0, bReverse);

    //loop limited cycle
    for (int32 i = 0; i < l && idxChecking >= 0; i++) {
        FMyIdCollectionCpp *pC = &pD->m_aUntakenCardStacks[idxChecking];

        int32 l2 = pC->m_aIds.Num();
        int32 Num2Collect = l2 < (int32)len ? l2 : (int32)len;

        for (int32 idx = (l2 - 1); idx >= (l2 - Num2Collect); idx--) {
            outIds.Emplace(pC->m_aIds[idx]);
            len--;
        }

        if (len == 0) {
            break;
        }

        //Have remaining cards
        idxChecking = getIdxOfUntakenSlotHavingCard(idxChecking, 1, bReverse);
    }

    MY_VERIFY(len == 0);

    return;
}

void FMyMJGameCoreCpp::tryCollectCardsFromUntakenSlot(int32 idxBase, uint32 len, bool bReverse, TArray<int32> &outIds)
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    int32 cardsleftAll = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();
    int32 l = (int32)len < cardsleftAll ? (int32)len : cardsleftAll;
    return collectCardsFromUntakenSlot(idxBase, l, bReverse, outIds);
}

void FMyMJGameCoreCpp::moveCardFromOldPosi(int32 id)
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    FMyMJCardInfoPackCpp  *pCardInfoPack =  &getCardInfoPack();
    FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSys();

    FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdx(id);
    MyMJCardSlotTypeCpp eSlotSrc = pCardInfo->m_cPosi.m_eSlot;
    int32 idxAttender = pCardInfo->m_cPosi.m_iIdxAttender;
    if (eSlotSrc == MyMJCardSlotTypeCpp::Untaken) {
        MY_VERIFY(pCardInfo->m_cPosi.m_iIdxInSlot0 >= 0 && pCardInfo->m_cPosi.m_iIdxInSlot0 < pD->m_aUntakenCardStacks.Num());
        int32 idx = pCardInfo->m_cPosi.m_iIdxInSlot0;
        FMyIdCollectionCpp *pCollection = &pD->m_aUntakenCardStacks[idx];
        //MY_VERIFY(pCard->m_cPosi.m_iIdxInSlot1 >= 0 && pCard->m_cPosi.m_iIdxInSlot1 < pCollection->m_aIds.Num());
        
        MY_VERIFY(pCollection->m_aIds.Pop() == id);

        pCardInfo->m_cPosi.reset();

        if (isIdxUntakenSlotInKeptFromTailSegment(idx)) {
            pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumKeptFromTail--;
        }
        else {
            pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead--;
            if (pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumNormalFromHead == 0) {
                pD->m_cHelperLastCardTakenInGame.m_iId = id;
                pD->m_cHelperLastCardTakenInGame.m_iValue = pCardValuePack->getByIdx(id);
            }
        }
        pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal--;

    }
    else if (idxAttender >= 0 && idxAttender < 4) {
        m_aAttendersAll[idxAttender]->removeCard(id);
    }
    else {
        MY_VERIFY(false);
    }

}

void FMyMJGameCoreCpp::moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst)
{

    if (eSlotDst == MyMJCardSlotTypeCpp::Untaken) {
        MY_VERIFY(false);
    }
    else if (idxAttender >= 0 && idxAttender < 4) {
        m_aAttendersAll[idxAttender]->insertCard(id, eSlotDst);
    }
    else {
        MY_VERIFY(false);
    }
}

void FMyMJGameCoreCpp::updateUntakenInfoHeadOrTail(bool bUpdateHead, bool bUpdateTail)
{
    FMyMJCoreDataPublicDirectCpp *pD = getDataPublicDirect();
    MY_VERIFY(pD);

    if (pD->m_cUntakenSlotInfo.m_iUntakenSlotCardsLeftNumTotal <= 0) {
        return;
    }

    if (bUpdateHead) {
        int32 idx = getIdxOfUntakenSlotHavingCard(pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow, 0, false);
        MY_VERIFY(idx >= 0);
        pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotHeadNow = idx;
    }
    if (bUpdateTail) {
        int32 idx = getIdxOfUntakenSlotHavingCard(pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow, 0, true);
        MY_VERIFY(idx >= 0);
        pD->m_cUntakenSlotInfo.m_iIdxUntakenSlotTailNow = idx;
    }
}