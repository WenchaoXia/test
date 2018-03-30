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

        if (pAttender->getIsStillInGame()) {
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
            MY_VERIFY(pAttender->getIsStillInGame());
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
            if (pAttender->getIsStillInGame()) {
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

        if (pAttender->getIsStillInGame()) {
            ret |= (1 << i);
        }

    }

    MY_VERIFY(ret > 0);
    return ret;
}



//don't reenter this func, this may result stack overflow
bool FMyMJGameCoreCpp::actionLoop()
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    bool bRet = false;
    int64 iMsLast = UMyCommonUtilsLibrary::nowAsMsFromTick();
    int64 iTimePassedMs64 = iMsLast - m_cDataLogic.m_iMsLast;
    if (iTimePassedMs64 < 0) {
        iTimePassedMs64 = 0;
    }
    m_cDataLogic.m_iMsLast = iMsLast;
    int32 iTimePassedMs32 = iTimePassedMs64 > MAX_int32 ? MAX_int32 : (int32)iTimePassedMs64;

    bRet |= findAndApplyPushers();
    if (findAndHandleCmd()) {
        bRet |= findAndApplyPushers();
    }

    MyMJActionLoopStateCpp &eActionLoopState = m_cDataLogic.m_eActionLoopState;

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
        bool bAllCollected = pCollector->collectAction(pD->m_iActionGroupId, iTimePassedMs32, bHaveProgres, m_pResManager->getRandomStreamRef());

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
   
    FMyMJDataAccessorCpp& ac = getDataAccessorRef();
    FMyMJGamePusherIOComponentFullCpp &pusherIO = getPusherIOFullRef();

    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();


    int32 iGameId, iPusherIdLast;
    TSharedPtr<FMyMJGamePusherBaseCpp> pPusher;

    iGameId = pD->m_iGameId;
    iPusherIdLast = pD->m_iPusherIdLast;
    pPusher = m_pPusherIOFull->tryPullPusherFromLocal();     //this is the end of pusher's life cycle

    while (pPusher.IsValid()) {

        bRet = true;

        if ((m_iTrivalConfigMask & MyMJGameCoreTrivalConfigMaskShowPusherLog) > 0 && pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("got pusher of reset game."));
        }

        if ((m_iTrivalConfigMask & MyMJGameCoreTrivalConfigMaskShowPusherLog) > 0) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("core full [%s:%d:%d]: Applying: %s"),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)getRuleType()), pD->m_iActionGroupId, iPusherIdLast,
                *pPusher->ToString());
        }

        FMyMJGamePusherResultCpp* pusherResult = genPusherResultAsSysKeeper(*pPusher.Get());
        if (pusherResult == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("core full [%s]: pusher %s generated NULL result."),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)getRuleType()),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pPusher->getType()));
            MY_VERIFY(false);
        }

        if (!pusherResult->checkHaveValidResult()) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("core full [%s]: pusher %s generated an invalid result."),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)getRuleType()),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pPusher->getType()));
            MY_VERIFY(false);
        }



        bool bNeedVerify = prevApplyPusherResult(*pusherResult);

        if (pusherResult->m_aResultBase.Num() > 0) {
            ac.applyBase(pusherResult->m_aResultBase[0], NULL);
        }
        if (pusherResult->m_aResultDelta.Num() > 0) {
            ac.applyDelta(pusherResult->m_aResultDelta[0], NULL);
        }

        if (pusherIO.GivePusherResult(pusherResult)) {

        }
        else {
            delete(pusherResult);
            pusherResult = NULL;
        }
        MY_VERIFY(pusherResult == NULL);


        /// this should be done by apply pusher
        if (pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            iGameId = pD->m_iGameId;
            iPusherIdLast = -1;
        }

        applyPusher(*pPusher.Get());


        if (!(pD->m_iPusherIdLast == pPusher->getId())) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core full [%s:%d:%d], pusher [%s] id not equal: %d, %d."),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)getRuleType()), pD->m_iActionGroupId, iPusherIdLast,
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)pPusher->getType()), pD->m_iPusherIdLast, pPusher->getId());
            MY_VERIFY(false);
        }

        if (bNeedVerify) {
            MY_VERIFY(verifyDataUniformationAfterPusherAndResultApplied());
        }

        iGameId = pD->m_iGameId;
        iPusherIdLast = pD->m_iPusherIdLast;
        pPusher = m_pPusherIOFull->tryPullPusherFromLocal();
    }

    return bRet;
}

bool FMyMJGameCoreCpp::findAndHandleCmd()
{

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

            handleCmd(eRoleType, *pCmd);

            //found one cmd, and the cmd life cycle is also manager by external, return it
            pCmdIO->m_pOutputQueueRemote->Enqueue(pCmd);
        }
    }

    return bNewCmdGot;
}

void FMyMJGameCoreCpp::genActionChoices()
{
    MY_VERIFY(m_pPusherIOFull.IsValid());

    for (int i = 0; i < 4; i++) {
        FMyMJGameAttenderCpp *pAttender = m_aAttendersAll[i].Get();
        MY_VERIFY(pAttender);
        if (!pAttender->getIsStillInGame()) {
            continue;
        }

        const FMyMJGameActionContainorCpp *pContainor = &pAttender->getDataLogicRefConst().m_cActionContainor;
        if (pContainor->getNeed2CollectConst() == true) {
            pAttender->genActionChoices(m_pPusherIOFull.Get());
        }
    }
};


void FMyMJGameCoreCpp::resetForNewActionLoop()
{
    if (!m_cActionLoopHelperData.getHaveSetupDataConst()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("full core have not setup loop helper data, m_eState: %s."),
                            *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameStateCpp"), (uint8)getCoreDataPublicRefConst().m_eGameState));
        MY_VERIFY(false);
    }

    FMyMJGameActionBaseCpp* pPrev = m_cActionLoopHelperData.takePrevAction();
    FMyMJGameActionBaseCpp* pPost = m_cActionLoopHelperData.takePostAction();
    int32 iIdxAttenderMask = m_cActionLoopHelperData.m_iIdxAttenderMask;
    bool bAllowSamePriAction = m_cActionLoopHelperData.m_bAllowSamePriAction;
    int32 iIdxAttenderHavePriMax = m_cActionLoopHelperData.m_iIdxAttenderHavePriMax;

    m_cActionLoopHelperData.clear();

    //step1: reset containors:
    int32 iRealAttenderCount = 0;
    for (int i = 0; i < 4; i++) {
        FMyMJGameAttenderCpp *pAttender = m_aAttendersAll[i].Get();
        int32 idxAttender = pAttender->getIdx();
        MY_VERIFY(i == idxAttender);

        bool bIsStillInGame = pAttender->getIsStillInGame();
        bool  bNeed2Collect = ((iIdxAttenderMask & (1 << idxAttender)) > 0) && bIsStillInGame;
        pAttender->resetForNewActionLoop(bNeed2Collect);

        if (pAttender->getIsRealAttender()) {
            iRealAttenderCount++;
        }
    }


    MY_VERIFY(m_pActionCollector.IsValid());

    //step2: reset collector
    m_pActionCollector->resetForNewLoopForFullMode(pPrev, pPost, bAllowSamePriAction, iIdxAttenderHavePriMax, iRealAttenderCount);
}

bool FMyMJGameCoreCpp::prevApplyPusherResult(const FMyMJGamePusherResultCpp &pusherResult)
{
    bool bRet = false;

    if (pusherResult.m_aResultBase.Num() > 0) {

        bRet = true;
    }

    if (pusherResult.m_aResultDelta.Num() > 0) {
        const FMyMJDataDeltaCpp& delta = pusherResult.m_aResultDelta[0];
        if (delta.m_aCoreData.Num() > 0) {
            const TArray<FMyMJCardInfoCpp>& aCardInfos2Update = delta.m_aCoreData[0].m_aCardInfos2Update;
            int l = aCardInfos2Update.Num();
            if (l > 0) {
                const FMyMJCardInfoPackCpp  *pCardInfoPack = &getCardInfoPackRefConst();
                const FMyMJCardValuePackCpp& cardValuePack = getCardValuePackOfSysKeeperRefConst();

                for (int i = 0; i < aCardInfos2Update.Num(); i++) {
                    const FMyMJCardInfoCpp& cardInfoTarget = aCardInfos2Update[i];
                    const FMyMJCardInfoCpp& cardInfoSelf = *pCardInfoPack->getByIdxConst(cardInfoTarget.m_iId);

                    if (cardInfoSelf.m_cPosi != cardInfoTarget.m_cPosi) {
                        bRet = true;
                        //for safety, any different exist, we make movement happen
                        moveCardFromOldPosi(cardInfoTarget.m_iId);
                        moveCardToNewPosi(cardInfoTarget.m_iId, cardInfoTarget.m_cPosi.m_iIdxAttender, cardInfoTarget.m_cPosi.m_eSlot);
                    }

                    if (cardInfoSelf.m_eFlipState != cardInfoTarget.m_eFlipState && cardInfoTarget.m_eFlipState == MyMJCardFlipStateCpp::Up) {
                        int32 idCard = cardInfoTarget.m_iId;
                        int32 valueCard = cardValuePack.getByIdx(idCard);
                        MY_VERIFY(valueCard > 0);
                        m_cDataLogic.m_cHelperShowedOut2AllCards.insert(idCard, valueCard);
                    }
                }
            }
        }

        MyMJGamePusherTypeCpp ePusherType = delta.getType();
        if (ePusherType == MyMJGamePusherTypeCpp::PusherFillInActionChoices || ePusherType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify || ePusherType == MyMJGamePusherTypeCpp::PusherCountUpdate) {
            bRet = true;
        }

    }

    //testing: overwrite it
    bRet = true;
    return bRet;
};

bool FMyMJGameCoreCpp::verifyDataUniformationAfterPusherAndResultApplied()
{
    bool bRet = true;

    MY_VERIFY(m_pActionCollector.IsValid());
    //int32 actionGroupIdCollectorLogic = m_pActionCollector->getActionGroupId();

    int32 actionGroupIdData = getCoreDataPublicRefConst().m_iActionGroupId;
    //if (actionGroupIdCollectorLogic != actionGroupIdData) {
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("action group id not uniform, logic %d, data %d."), actionGroupIdCollectorLogic, actionGroupIdData);
        //return false;
    //}



    for (int i = 0; i < 4; i++) {
        const FMyMJAttenderDataLogicOnlyCpp& attenderDataLogic = getAttenderByIdx(i)->getDataLogicRefConst();
        const FMyMJRoleDataAttenderPublicCpp& attenderDataPublic = getAttenderByIdx(i)->getRoleDataAttenderPublicRefConst();
        const FMyMJRoleDataAttenderPrivateCpp& attenderDataPrivate = getAttenderByIdx(i)->getRoleDataAttenderPrivateRefConst();

        int32 l0, l1;
        l0 = attenderDataLogic.m_cHandCards.getCount();
        l1 = attenderDataPublic.m_aIdHandCards.Num();

        if (l0 != l1) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("attender %d's hand card not uniform, logic %d, data %d."), i, l0, l1);
            bRet = false;
            break;
        }

        bool bEqual = attenderDataPrivate.m_cActionContainor.equal(attenderDataLogic.m_cActionContainor);

        if (!bEqual) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("attender %d's action containor not uniform."), i);
            bRet = false;
            break;
        }

        //int32 actionGroupIdAttenderLogic = attenderDataLogic.m_cActionContainor.getActionGroupId();
        //if (actionGroupIdCollectorLogic != actionGroupIdAttenderLogic) {
        //    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("attender %d's logic action group id not equal to core collector,, collector's %d, attender's %d."), actionGroupIdCollectorLogic, actionGroupIdAttenderLogic);
        //    return false;
        //}
    }




    return bRet;
};

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
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    //the sequence ensure pusher is drained out clean before cmd applies
    findAndApplyPushers();
    if (findAndHandleCmd()) {
        findAndApplyPushers();
    }

    bool bIsInGameState = pD->isInGameState();
    bool bHaveProgress = true;

    while (bIsInGameState && bHaveProgress) {
        bHaveProgress = actionLoop();
        bIsInGameState = pD->isInGameState();
    }

}

int32 FMyMJGameCoreCpp::getIdxOfUntakenSlotHavingCard(int32 idxBase, uint32 delta, bool bReverse)
{
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    int32 l = pD->m_aUntakenCardStacks.Num();
    MY_VERIFY(idxBase >= 0 && idxBase < l);

    int32 idxChecking = idxBase;
    for (int32 i = 0; i < l; i++) {
        const FMyIdCollectionCpp *pC = &pD->m_aUntakenCardStacks[idxChecking];
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
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    int32 l = pD->m_aUntakenCardStacks.Num();
    MY_VERIFY(idxBase >= 0 && idxBase < l);
    MY_VERIFY(len > 0 && len < 20);
    outIds.Empty();

    int32 idxChecking = getIdxOfUntakenSlotHavingCard(idxBase, 0, bReverse);

    //loop limited cycle
    for (int32 i = 0; i < l && idxChecking >= 0; i++) {
        const FMyIdCollectionCpp *pC = &pD->m_aUntakenCardStacks[idxChecking];

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
    const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    int32 cardsleftAll = pD->m_cUntakenSlotInfo.getCardNumCanBeTakenAll();
    int32 l = (int32)len < cardsleftAll ? (int32)len : cardsleftAll;
    return collectCardsFromUntakenSlot(idxBase, l, bReverse, outIds);
}

void FMyMJGameCoreCpp::moveCardFromOldPosi(int32 id)
{
    //const FMyMJCoreDataPublicCpp *pD = &getCoreDataPublicRefConst();

    const FMyMJCardInfoPackCpp  *pCardInfoPack =  &getCardInfoPackRefConst();
    //const FMyMJCardValuePackCpp *pCardValuePack = &getCardValuePackOfSysKeeperRefConst();

    const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(id);
    MyMJCardSlotTypeCpp eSlotSrc = pCardInfo->m_cPosi.m_eSlot;
    int32 idxAttender = pCardInfo->m_cPosi.m_iIdxAttender;
    if (eSlotSrc == MyMJCardSlotTypeCpp::Untaken) {
        //MY_VERIFY(pCardInfo->m_cPosi.m_iIdxInSlot0 >= 0 && pCardInfo->m_cPosi.m_iIdxInSlot0 < pD->m_aUntakenCardStacks.Num());
        //int32 idx = pCardInfo->m_cPosi.m_iIdxInSlot0;
        //FMyIdCollectionCpp *pCollection = &pD->m_aUntakenCardStacks[idx];
        //MY_VERIFY(pCard->m_cPosi.m_iIdxInSlot1 >= 0 && pCard->m_cPosi.m_iIdxInSlot1 < pCollection->m_aIds.Num());
        
        //MY_VERIFY(pCollection->m_aIds.Pop() == id);

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
