// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "TimerManager.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"
//#include "Kismet/KismetNodeHelperLibrary.h"


//#include "CoreMinimal.h"
//#include "Stats/Stats.h"
//#include "UObject/ObjectMacros.h"
//#include "UObject/UObjectBaseUtility.h"
//#include "UObject/Object.h"
//#include "InputCoreTypes.h"
//#include "Templates/SubclassOf.h"
//#include "UObject/CoreNet.h"
//#include "Engine/EngineTypes.h"
//#include "Engine/EngineBaseTypes.h"
//#include "ComponentInstanceDataCache.h"
//#include "Components/ChildActorComponent.h"
//#include "RenderCommandFence.h"
//#include "Misc/ITransaction.h"
//#include "Engine/Level.h"

void UMyMJPusherBufferCpp::trySyncDataFromCoreFull()
{
    /*
    MY_VERIFY(m_pConnectedCoreFull);
    if (!IsValid(m_pConnectedCoreFull)) {
        return;
    }
    
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* pPusherQueue = m_pConnectedCoreFull->getPusherQueue();
    MY_VERIFY(pPusherQueue);

    FMyMJGamePusherPointersCpp cPusherSegment;
    if (cPusherSegment.helperFillAsSegmentFromQueue(*pPusherQueue)) {
 
        m_cPusherBuffer.helperTryFillDataFromSegment(-1, cPusherSegment, true);

        int32 iGameId, iPusherIdLast;
        m_cPusherBuffer.getGameIdAndPusherIdLast(&iGameId, &iPusherIdLast);
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Got puser, now iGameid %d, iPusherIdLast %d"), iGameId, iPusherIdLast);

        m_cPusherUpdatedMultcastDelegate.Broadcast();

    }
    */
};

void UMyMJCoreFullCpp::testGameCoreInSubThread(bool showCoreLog, bool bAttenderRandomSelectHighPriActionFirst)
{
    int32 iMask = MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero;
    if (showCoreLog) {
        iMask |= MyMJGameCoreTrivalConfigMaskShowPusherLog;
    }

    while (!tryChangeMode(MyMJGameRuleTypeCpp::LocalCS, iMask)) {
        FPlatformProcess::Sleep(0.1);
    }

    startGame(true, bAttenderRandomSelectHighPriActionFirst);
}


bool UMyMJCoreFullCpp::tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask)
{

    if (m_pCoreFullWithThread.IsValid()) {
        if (m_pCoreFullWithThread->getRuleType() == eRuleType) {
            return true;
        }

        if (!m_pCoreFullWithThread->isInStartState()) {
            m_pCoreFullWithThread.Reset();

            UWorld *world = GetOuter()->GetWorld();
            if (IsValid(world)) {
                world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
            }
        }
        else {
            m_pCoreFullWithThread->Stop();
            return false;
        }
    }

    //recheck
    if (m_pCoreFullWithThread.IsValid()) {
        return false;
    }
    else {
        if (eRuleType == MyMJGameRuleTypeCpp::Invalid) {
            return true;
        }

        //want to create one new
        int32 iSeed;
        if (m_iSeed2OverWrite != 0) {
            iSeed = m_iSeed2OverWrite;
        }
        else {
            iSeed = UMyMJUtilsLibrary::nowAsMsFromTick();
        }
        //iSeed = 530820412;

        FMyMJGameCoreThreadControlCpp *pCoreFull = new FMyMJGameCoreThreadControlCpp(eRuleType, iSeed, iTrivalConfigMask);
        m_pCoreFullWithThread = MakeShareable<FMyMJGameCoreThreadControlCpp>(pCoreFull);

        return true;
    }


}

bool UMyMJCoreFullCpp::startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst)
{
    if (!m_pCoreFullWithThread.IsValid() || m_pCoreFullWithThread->getRuleType() == MyMJGameRuleTypeCpp::Invalid) {
        return false;
    }

    int8 iAttenderRandomMask = 0;
    if (bAttenderRandomSelectDo) {
        iAttenderRandomMask |= MyMJGameActionContainorCpp_RandomMask_DoRandomSelect;
    }
    if (bAttenderRandomSelectHighPriActionFirst) {
        iAttenderRandomMask |= MyMJGameActionContainorCpp_RandomMask_HighPriActionFirst;
    }
    int32 iAttendersAllRandomSelectMask = MyMJGameDup8BitMaskForSingleAttenderTo32BitMaskForAll(iAttenderRandomMask);

    FMyMJGameCmdRestartGameCpp *pCmdReset = new FMyMJGameCmdRestartGameCpp();
    pCmdReset->m_iAttendersAllRandomSelectMask = iAttendersAllRandomSelectMask;
    UMyMJUtilsLocalCSLibrary::genDefaultCfg(pCmdReset->m_cGameCfg);
    m_pCoreFullWithThread->getIOGourpAll().m_aGroups[(uint8)MyMJGameRoleTypeCpp::SysKeeper].getCmdInputQueue().Enqueue(pCmdReset);

    m_pCoreFullWithThread->kick();

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("startGame() called"));

    UWorld *world = GetOuter()->GetWorld();
    
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &UMyMJCoreFullCpp::loop, ((float)My_MJ_GAME_IO_DRAIN_LOOP_TIME_MS) / (float)1000, true);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return false;
    }

    return true;
}

void UMyMJCoreFullCpp::loop()
{
    if (!m_pCoreFullWithThread.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreFullWithThread invalid!"));
        return;
    }

    MY_VERIFY(m_pPusherBuffer);
    if (IsValid(m_pPusherBuffer)) {
        m_pPusherBuffer->trySyncDataFromCoreFull();
    }

    int32 l = m_apNextNodes.Num();
    if (l != (uint8)MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough nodes next were set, only %d got!"), l);
        return;
    }

    FMyMJGameCmdPointersCpp cCmdSegment;
    for (int32 i = 0; i < l; i++) {
        FMyMJGameIOGroupCpp *pGroup = &m_pCoreFullWithThread->getIOGourpAll().m_aGroups[i];
        UMyMJIONodeCpp *pNode = m_apNextNodes[i];
        MY_VERIFY(pNode->m_eRoleType == (MyMJGameRoleTypeCpp)i);
        //pNode->pushPushers(pGroup);
        if (IsValid(pNode)) {

            bool bHaveNew = cCmdSegment.helperFillAsSegmentFromIOGroup(pGroup);
            if (bHaveNew) {
                pNode->onCmdUpdated(cCmdSegment);
            }
        }
    }

}

void UMyMJCoreFullCpp::clearUp()
{
    m_pPusherBuffer = NULL;
    m_apNextNodes.Reset();
    if (m_pCoreFullWithThread.IsValid()) {
        m_pCoreFullWithThread->Stop();
    }

    UWorld *world = GetOuter()->GetWorld();

    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
    }
}

void UMyMJCoreFullCpp::PostInitProperties()
{
    Super::PostInitProperties();

    m_pPusherBuffer = NewObject<UMyMJPusherBufferCpp>(this);
    m_pPusherBuffer->m_pConnectedCoreFull = this;

    m_apNextNodes.Reset();

    for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {

        UMyMJIONodeCpp *newNode = NewObject<UMyMJIONodeCpp>(this);
        m_apNextNodes.Emplace(newNode);
        newNode->m_eRoleType = (MyMJGameRoleTypeCpp)i;
    }
}



FMyMJGamePusherBaseCpp* AMyMJCoreMirrorCpp::tryCheckAndGetNextPusher()
{
    /*
    int32 iGameId = -1, iPusherIdLast = -1;

    if (IsValid(m_pPusherBuffer)) {

        if (!getbHaltForGraphic()) {
            if (m_pCoreMirror.IsValid()) {
                m_pCoreMirror->getGameIdAndPusherIdLast(&iGameId, &iPusherIdLast);
            }
            return m_pPusherBuffer->m_cPusherBuffer.helperTryPullPusher(iGameId, iPusherIdLast + 1).Get();
        }
        else {
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pIONodeAsSys invalid!"));
    }
    */
    return NULL;
}

void AMyMJCoreMirrorCpp::loop()
{

    FMyMJGamePusherBaseCpp *pPusher = tryCheckAndGetNextPusher();

    while (pPusher) {
        MyMJGamePusherTypeCpp ePusherType = pPusher->getType();

        //1st, make sure mirror core is correctly set
        if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame) {
            FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
            MyMJGameRuleTypeCpp eDestType = pPusherResetGame->m_cGameCfg.m_eRuleType;
            MY_VERIFY(eDestType != MyMJGameRuleTypeCpp::Invalid);
            MyMJGameRuleTypeCpp eNowType = MyMJGameRuleTypeCpp::Invalid;
            if (m_pCoreMirror.IsValid()) {
                eNowType = m_pCoreMirror->getRuleType();
            }

            if (eNowType != eDestType) {
                int32 iSeed;
                if (m_iSeed2OverWrite != 0) {
                    iSeed = m_iSeed2OverWrite;
                }
                else {
                    iSeed = UMyMJUtilsLibrary::nowAsMsFromTick();
                }

                bool bOK = checkLevelSettings();
                if (!bOK) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("level setting is not correct!"));
                    MY_VERIFY(false);
                }



                FMyMJGameCoreCpp *pCore = UMyMJBPUtilsLibrary::helperCreateCoreByRuleType(eDestType, iSeed, MyMJGameCoreTrivalConfigMaskShowPusherLog);
                m_pCoreMirror = MakeShareable<FMyMJGameCoreCpp>(pCore);

                UMyMJDataForMirrorModeCpp *pMJData = getpMJData();

                //m_pCoreMirror->initMirrorMode(m_pCoreMirror, pMJData);
            }

            m_iPusherApplyState = 0;
        }

        if (!m_pCoreMirror.IsValid()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreMirror invalid!"));
            MY_VERIFY(false);
            break;
        }

        //2nd, apply
        {
            if (m_iPusherApplyState == 0) {
                m_iPusherApplyState = 1;

                //When full mode, all values revealed in system role, but when mirror mode, we need to reveal them in each role's private data
                //MY_VERIFY(m_pCoreMirror->getWorkMode() == MyMJGameElemWorkModeCpp::Mirror);

                int32 iAttenderMask = 0;
                TArray<FMyIdValuePair> aRevealedCardValues;

                //pPusher->getRevealedCardValues(iAttenderMask, aRevealedCardValues);
                if (iAttenderMask != 0) {


                    //int32 l;
                    //l = MY_GET_ARRAY_LEN(m_aAttendersAll);

                    /*
                    TArray<AMyMJAttenderPawnBPCpp *>& aAttenderPawns = getAttenderPawnsRef();
                    l = aAttenderPawns.Num();

                    MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);

                    for (int32 i = 0; i < l; i++) {
                        if ((iAttenderMask & (1 << i)) == 0) {
                            continue;
                        }
                        //need update
                        aAttenderPawns[i]->m_pDataPrivate->m_cDataPrivateDirect.m_cCardValuePack.tryRevealCardValueByIdValuePairs(aRevealedCardValues);
                        //m_aAttendersAll[i]->getDataPrivateDirect()->m_cCardValuePack.tryRevealCardValueByIdValuePairs(aRevealedCardValues);
                    }
                    */
                }


                if (prePusherApplyForGraphic(pPusher)) {
                    //we halt for graphic
                    break;
                }
            }


            if (m_iPusherApplyState == 1) {
                m_iPusherApplyState = 2;
                //m_pCoreMirror->makeProgressByPusher(pPusher);
            }
            
            if (m_iPusherApplyState == 2) {
                m_iPusherApplyState = 0;
                if (postPusherApplyForGraphic(pPusher)) {
                    //we halt for graphic
                    break;
                }
            }
        }



        pPusher = tryCheckAndGetNextPusher();
    }
}


void AMyMJCoreBaseForBpCpp::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    MY_VERIFY(m_aAttenderPawns.Num() <= 0);
    MY_VERIFY(m_pMJData == NULL);

    if (m_pMJData) {
        m_pMJData->DestroyComponent();
        m_pMJData = NULL;
    }

    m_pMJData = NewObject<UMyMJDataForMirrorModeCpp>(this);
    m_pMJData->createSubObjects(false, true);
    m_pMJData->RegisterComponent();
    m_pMJData->SetIsReplicated(true);
};

void AMyMJCoreBaseForBpCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyMJCoreBaseForBpCpp, m_pMJData);
};

bool AMyMJCoreBaseForBpCpp::prePusherApplyForGraphic(FMyMJGamePusherBaseCpp *pPusher)
{
    return false;
}

bool AMyMJCoreBaseForBpCpp::postPusherApplyForGraphic(FMyMJGamePusherBaseCpp *pPusher)
{
    //let's filter out important pusher and notify blueprint
    MyMJGamePusherTypeCpp ePusherType = pPusher->getType();
    bool ret = false;


    if (ePusherType == MyMJGamePusherTypeCpp::PusherFillInActionChoices) {
        FMyMJGamePusherFillInActionChoicesCpp *pPusherFillInActionChoices = StaticCast<FMyMJGamePusherFillInActionChoicesCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify) {
        FMyMJGamePusherMadeChoiceNotifyCpp *pPusherMadeChoiceNotify = StaticCast<FMyMJGamePusherMadeChoiceNotifyCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame) {

        //reflect data
       // m_cCoreDataDirect.m_iCardNumCanBeTakenNormally = m_pCoreMirror->getUntakenSlotInfoRef().getCardNumCanBeTakenNormally();
        //m_cCoreDataDirect.m_aUntakenCardStacks = m_pCoreMirror->getUntakenCardStacksRef();
        //m_cCoreDataDirect.m_cGameCfg = *m_pCoreMirror->getpGameCfg();
        FMyMJGamePusherResetGameCpp *pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
        ret = postPusherApplyResetGame(*pPusherResetGame);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateTing) {
        FMyMJGamePusherUpdateTingCpp *pPusherUpdateTing = StaticCast<FMyMJGamePusherUpdateTingCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionNoAct) {
        FMyMJGameActionNoActCpp *pActionNoAct = StaticCast<FMyMJGameActionNoActCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {
        FMyMJGameActionThrowDicesCpp *pActionThrowDices = StaticCast<FMyMJGameActionThrowDicesCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
        FMyMJGameActionDistCardAtStartCpp *pActionDistCardAtStart = StaticCast<FMyMJGameActionDistCardAtStartCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
        FMyMJGameActionTakeCardsCpp *pActionTakeCards = StaticCast<FMyMJGameActionTakeCardsCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
        FMyMJGameActionGiveOutCardsCpp *pActionGiveOutCards = StaticCast<FMyMJGameActionGiveOutCardsCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
        FMyMJGameActionWeaveCpp *pActionWeave = StaticCast<FMyMJGameActionWeaveCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {
        FMyMJGameActionHuCpp *pActionHu = StaticCast<FMyMJGameActionHuCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {
        FMyMJGameActionHuBornLocalCSCpp *pActionHuBornLocalCS = StaticCast<FMyMJGameActionHuBornLocalCSCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {
        FMyMJGameActionZhaNiaoLocalCSCpp *pActionZhaNiaoLocalCS = StaticCast<FMyMJGameActionZhaNiaoLocalCSCpp *>(pPusher);
    }

    return ret;
}