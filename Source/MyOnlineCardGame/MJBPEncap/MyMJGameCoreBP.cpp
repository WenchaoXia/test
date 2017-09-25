// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"


#include "TimerManager.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"
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

void UMyMJCoreFullCpp::testGameCoreInSubThread(bool showCoreLog, bool bAttenderRandomSelectHighPriActionFirst, bool bNeedLoopSelf)
{
    int32 iMask = MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero;
    if (showCoreLog) {
        iMask |= MyMJGameCoreTrivalConfigMaskShowPusherLog;
    }

    while (!tryChangeMode(MyMJGameRuleTypeCpp::LocalCS, iMask)) {
        FPlatformProcess::Sleep(0.1);
    }

    startGame(true, bAttenderRandomSelectHighPriActionFirst, bNeedLoopSelf);
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

bool UMyMJCoreFullCpp::startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst, bool bNeedLoopSelf)
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
        if (bNeedLoopSelf) {
            world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &UMyMJCoreFullCpp::loop, ((float)My_MJ_GAME_CORE_FULL_IO_DRAIN_LOOP_TIME_MS) / (float)1000, true);
        }
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

    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(this);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("PostInitializeComponents, bHaveLogic %d."), bHaveLogic);
    if (bHaveLogic) {
        m_pMJDataAll = NewObject<UMyMJDataAllCpp>(this);
        m_pMJDataAll->RegisterComponent();
        m_pMJDataAll->SetIsReplicated(true);

        //UMyMJBPUtilsLibrary::getEngineNetMode(this) == NM_DedicatedServer;
    }
};

void AMyMJCoreMirrorCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyMJCoreMirrorCpp, m_pMJDataAll);
    DOREPLIFETIME(AMyMJCoreMirrorCpp, m_iTest0);

};

bool AMyMJCoreMirrorCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("AMyMJCoreMirrorCpp::ReplicateSubobjects()."));

    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
    return WroteSomething;
};


void AMyMJCoreMirrorCpp::toCoreFullLoop()
{
    MY_VERIFY(IsValid(m_pMJDataAll));
    MY_VERIFY(IsValid(m_pCoreFull));

    m_pCoreFull->loop();

    UWorld *world = GetWorld();

    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        MY_VERIFY(false);
    }

    float timeNow = world->GetTimeSeconds();
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

void AMyMJCoreBaseForBpCpp::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    bool bHaveVisual = UMyMJBPUtilsLibrary::haveClientVisualLayer(this);
    if (bHaveVisual) {
        m_pDataHistoryBuffer = NewObject<UMyMJDataSequencePerRoleCpp>(this);
        m_pDataHistoryBuffer->resizeEvents(128);

        UWorld *world = GetWorld();

        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cForVisualLoopTimerHandle);
            world->GetTimerManager().SetTimer(m_cForVisualLoopTimerHandle, this, &AMyMJCoreBaseForBpCpp::forVisualLoop, ((float)MY_MJ_GAME_CORE_MIRROR_FOR_VISUAL_LOOP_TIME_MS) / (float)1000, true);
            //world->GetTimerManager().SetTimer(m_cForVisualLoopTimerHandle, this, &AMyMJCoreBaseForBpCpp::forVisualLoop, 5, true);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid, this is only valid for default object."));
        }
    }
};

bool AMyMJCoreBaseForBpCpp::tryAppendData2Buffer()
{
    MY_VERIFY(IsValid(m_pDataHistoryBuffer));

    if (!IsValid(m_pMJDataAll)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pMJDataAll is still null"));
        return false;
    }

    UMyMJDataSequencePerRoleCpp* pData = m_pMJDataAll->getDataByRoleType(m_pDataHistoryBuffer->m_eRole);
    if (!IsValid(pData)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("data is still null, role %d."), (uint8)m_pDataHistoryBuffer->m_eRole);
        return false;
    }

    return m_pDataHistoryBuffer->mergeDataFromOther(*pData);
};

void AMyMJCoreBaseForBpCpp::forVisualLoop()
{
    tryAppendData2Buffer();

    UWorld* world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world of core is not valid!"));
        return;
    }

    uint32 clientTimeNow_ms = world->GetTimeSeconds() * 1000;
    clientTimeNow_ms = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(clientTimeNow_ms);

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("forVisualLoop."));
    if (m_eVisualMode == MyMJCoreBaseForBpVisualModeCpp::Normal) {
        forVisualLoopModeNormal(clientTimeNow_ms);
    }
    else if (m_eVisualMode == MyMJCoreBaseForBpVisualModeCpp::CatchUp) {
        forVisualLoopModeCatchUp(clientTimeNow_ms);
    }

};

void AMyMJCoreBaseForBpCpp::forVisualLoopModeNormal(uint32 clientTimeNow_ms)
{

    uint32 clientTimeNow_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(clientTimeNow_ms);

    //if you want to replay events that have passed, slow/faster the clock is not valid since it only affects the events in future, the only way is time revert. 

    //MY_VERIFY(m_cDataNow.m_aEventApplying.Num() <= 0);
    //MY_VERIFY(IsValid(m_pMJDataAll));

    UMyMJDataSequencePerRoleCpp* pBuffer = m_pDataHistoryBuffer;
    MY_VERIFY(IsValid(pBuffer));

    //we only care about deltas, which can calculate out all info

    while (pBuffer->getEventCount() > 0) {

        int32 iGameIdNow = m_cDataNow.getGameIdLast();
        int32 iPusherIdNow = m_cDataNow.getPusherIdLast();

        uint32 uiServerTimeNowCalced_ms = 0;
        bool bHaveTimeBond = m_cDataNow.m_cTimeBond.haveBond();
        if (bHaveTimeBond) {
            uiServerTimeNowCalced_ms = MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_cDataNow.m_cTimeBond.getCalculatedServerTime_data_unit(clientTimeNow_data_unit));
        }

        if (bHaveTimeBond && !m_cDataNow.isReadyToGiveNextPusherResult(uiServerTimeNowCalced_ms))
        {
            //previous not ended
            break;
        }

        FMyMJEventWithTimeStampBaseCpp& cEvent = pBuffer->getEventRefAt(0);
        uint32 uiEventStartTime = cEvent.getStartTime();
        uint32 uiDur = cEvent.getDuration();

        if (bHaveTimeBond && uiEventStartTime > 0 && uiEventStartTime > uiServerTimeNowCalced_ms) {
            //time not reached yet

            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time not reached, but server should have time line smoothed, not supposed to happen, event start %d.%03d, calced server time %d.%03d."), uiEventStartTime / 1000, uiEventStartTime % 1000, uiServerTimeNowCalced_ms / 1000, uiServerTimeNowCalced_ms % 1000);
            break;
        };

        bool bIsGameEndOrNotStatedNow = m_cDataNow.isGameEndOrNotStarted();
        bool bCanAppend = cEvent.canBeAppendedToPrev(iGameIdNow, iPusherIdNow, bIsGameEndOrNotStatedNow);

        if (bCanAppend) {

            //try fix the time dur
            if (uiDur > 0) {
                MY_VERIFY(uiEventStartTime > 0);
                if (bHaveTimeBond) {
                    uint32 uiDurFixed = uiDur;
                    uint32 uiServerTimeTargetEndTime = uiEventStartTime + uiDur;
                    
                    if (uiServerTimeTargetEndTime > uiServerTimeNowCalced_ms) {
                        uiDurFixed = uiServerTimeTargetEndTime - uiServerTimeNowCalced_ms;
                    }
                    else {
                        uiDurFixed = 0;
                    }

                    uiDurFixed = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(uiDurFixed);
                    if (uiDurFixed == 0) {
                        uiDurFixed = MY_MJ_GAME_WORLD_TIME_MS_RESOLUTION;
                    }

                    if (uiDurFixed != uiDur) {
                        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("fixing dur, calced server time %d.%03d,  %d.%03d -> %d.%03d ."), uiServerTimeNowCalced_ms / 1000, uiServerTimeNowCalced_ms % 1000, uiDur / 1000, uiDur % 1000, uiDurFixed / 1000, uiDurFixed % 1000);
                        cEvent.setDuration(uiDurFixed);

                        if (uiDurFixed <= (2 * MY_MJ_GAME_WORLD_TIME_MS_RESOLUTION)) {
                            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("the dur is too small, maybe machine too slow."));
                        }
                    }
                }


            }

            if (cEvent.getDuration() > 0) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("applying event with dur: %s."), *cEvent.genDebugMsg());
            }

            m_cDataNow.applyEvent(cEvent, FMyMJDataAtOneMomentCpp_eventApplyWay_Normal);

            if (cEvent.getDuration() > 0) {
                onEventAppliedWithDur(cEvent);
            }

            if (uiEventStartTime > 0) { 
                uint32 serverTimeNow_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(uiEventStartTime);
                if (!bHaveTimeBond) {
                    //we must form a bond, tell what time we present
                    m_cDataNow.m_cTimeBond.rebondTime(serverTimeNow_data_unit, clientTimeNow_data_unit);
                    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("time rebonded s: %d.%03d, c: %d.%03d."), uiEventStartTime / 1000, uiEventStartTime % 1000, clientTimeNow_ms / 1000, clientTimeNow_ms % 1000);
                }

                /*
                if (!bHaveTimeBond) {
                    //we must form a bond, tell what time we present
                    m_cDataNow.m_cTimeBond.rebondTime(serverTimeNow_data_unit, clientTimeNow_data_unit);
                }
                else {
                    //should we update
                    if (uiEventStartTime < uiServerTimeNowCalced_ms) {
                        //we are reverting time, for real time game, we shuld check if current state is correct and revert some actor to it's base time point
                        //two choice: let this action fast played, or revert and play normal
                        //turn based game, 2nd is better, revert
                        //note:: info actor's time affect many other actor, so for simple operate the global time for simple
                        m_cDataNow.m_cTimeBond.rebondTime(serverTimeNow_data_unit, clientTimeNow_data_unit);
                        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("time rebonded s: %d.%03d, c: %d.%03d."), uiEventStartTime/1000, uiEventStartTime%1000, clientTimeNow_ms/1000, clientTimeNow_ms%1000);
                    }
                    else if (uiEventStartTime == uiServerTimeNowCalced_ms) {
                        //good, nothing to be done
                    }
                    else {
                        //never happen
                    }
                }
                */
            }

            pBuffer->squashEventsToBase(1);

        }
        else {
            changeVisualMode(MyMJCoreBaseForBpVisualModeCpp::CatchUp);
            m_cDataNow.m_cTimeBond.clearBond();
            break;
        }


    }
};

void AMyMJCoreBaseForBpCpp::forVisualLoopModeCatchUp(uint32 clientTimeNow_ms)
{
    int32 iGameIdNow = m_cDataNow.getGameIdLast();
    int32 iPusherIdNow = m_cDataNow.getPusherIdLast();

    //MY_VERIFY(m_cDataNow.m_aEventApplying.Num() <= 0);
    //MY_VERIFY(IsValid(m_pMJDataAll));

    UMyMJDataSequencePerRoleCpp* pBuffer = m_pDataHistoryBuffer;
    MY_VERIFY(IsValid(pBuffer));

    //we only care about deltas, which can calculate out all info

    int32 l = pBuffer->getEventCount();

    int32 iPusherToSquash = l - 1;
    int32 iPusherToSquashMax = 30;
    iPusherToSquash = iPusherToSquash < iPusherToSquashMax ? iPusherToSquash : iPusherToSquashMax;
    if (iPusherToSquash <= 0) {
        m_cDataNow.resetWithBase(pBuffer->getBase());
        changeVisualMode(MyMJCoreBaseForBpVisualModeCpp::Normal);
        return;
    }

    pBuffer->squashEventsToBase(iPusherToSquash);

};