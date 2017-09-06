// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"


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


/*
void UMyMJPusherBufferCpp::trySyncDataFromCoreFull()
{

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

};
*/

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

    /*
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
    */
}

void UMyMJCoreFullCpp::clearUp()
{
    //m_pPusherBuffer = NULL;
    //m_apNextNodes.Reset();
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

    //m_pPusherBuffer = NewObject<UMyMJPusherBufferCpp>(this);
    //m_pPusherBuffer->m_pConnectedCoreFull = this;

    //m_apNextNodes.Reset();

    for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {

        //UMyMJIONodeCpp *newNode = NewObject<UMyMJIONodeCpp>(this);
        //m_apNextNodes.Emplace(newNode);
        //newNode->m_eRoleType = (MyMJGameRoleTypeCpp)i;
    }
}


//virtual void PostInitProperties() override;
void AMyMJCoreMirrorCpp::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (HasAuthority()) {
        m_pMJDataAll = NewObject<UMyMJDataAllCpp>(this);
        m_pMJDataAll->RegisterComponent();
        m_pMJDataAll->SetIsReplicated(true);
    }
};

void AMyMJCoreMirrorCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyMJCoreMirrorCpp, m_pMJDataAll);
};


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

void AMyMJCoreMirrorCpp::toCoreFullLoop()
{
    MY_VERIFY(IsValid(m_pMJDataAll));
    MY_VERIFY(IsValid(m_pCoreFull));

    UWorld *world = GetWorld();

    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        MY_VERIFY(false);
    }

    float timeNow = world->GetRealTimeSeconds();
    uint32 timeNowMs = timeNow * 1000;
    timeNowMs = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(timeNowMs);

    //1st, handle cmd
    //Todo:


    //2nd, handle pusher result
    TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc>* pQ = m_pCoreFull->getPusherResultQueue();
    if (pQ == NULL) {
        //this means core didn't setup mode yet
        return;
    }
    else {
        bool bHaveNextPuhserResult = !pQ->IsEmpty();
        bool bIsReadyFOrNextPushserResult = m_pMJDataAll->isReadyToGiveNextPusherResult(timeNowMs);

        bool bHaveProgress = false;
        while (bHaveNextPuhserResult && bIsReadyFOrNextPushserResult) {
            FMyMJGamePusherResultCpp* pPusherResult = NULL;
            pQ->Dequeue(pPusherResult);
            MY_VERIFY(pPusherResult);

            m_pMJDataAll->addPusherResult(*pPusherResult, timeNowMs);
            delete(pPusherResult);
            bHaveProgress = true;

            bHaveNextPuhserResult = !pQ->IsEmpty();
            bIsReadyFOrNextPushserResult = m_pMJDataAll->isReadyToGiveNextPusherResult(timeNowMs);
        }

        if (bHaveProgress) {
            m_pMJDataAll->markAllDirtyForRep();
        }
    }
}

void AMyMJCoreMirrorCpp::OnRep_MJDataAll()
{

}

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