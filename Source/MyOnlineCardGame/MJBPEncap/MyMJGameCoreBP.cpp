// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"


#include "TimerManager.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"
#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"


void UMyMJGameCoreFullCpp::testGameCoreInSubThread(bool showCoreLog, bool bAttenderRandomSelectHighPriActionFirst)
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


bool UMyMJGameCoreFullCpp::tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask)
{

    if (m_pCoreFullWithThread.IsValid()) {
        if (m_pCoreFullWithThread->getRuleType() == eRuleType) {
            return true;
        }

        if (!m_pCoreFullWithThread->isInStartState()) {
            m_pCoreFullWithThread.Reset();
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

bool UMyMJGameCoreFullCpp::startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst)
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

    return true;
}

void UMyMJGameCoreFullCpp::clearUp()
{
    if (m_pCoreFullWithThread.IsValid()) {
        m_pCoreFullWithThread->Stop();
    }
}


/*
void AMyMJGameCoreMirrorCpp::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(this);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("PostInitializeComponents, bHaveLogic %d."), bHaveLogic);

    if (bHaveLogic) {

        m_pCoreFull = NewObject<UMyMJGameCoreFullCpp>(this);

        m_pMJDataAll = NewObject<UMyMJDataAllCpp>(this);
        m_pMJDataAll->RegisterComponent();
        m_pMJDataAll->SetIsReplicated(true);

        //we have mutlicast delegate once setupped with IO Node to trigger action, but in first we may miss some, so set a timer to do loop action once
        UWorld *world = GetWorld();
        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
            world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJGameCoreMirrorCpp::toCoreFullLoop, ((float)MY_MJ_GAME_CORE_MIRROR_TO_CORE_FULL_LOOP_TIME_MS) / (float)1000, true);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
            MY_VERIFY(false);
        }
    }
};
*/

void AMyMJGameCoreMirrorCpp::BeginPlay()
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCoreMirrorCpp::BeginPlay()."));
    Super::BeginPlay();

    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(this);
    if (bHaveLogic) {
        setCoreFullPartEnabled(true);
    }
};

void AMyMJGameCoreMirrorCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyMJGameCoreMirrorCpp, m_pMJDataAll);
    DOREPLIFETIME(AMyMJGameCoreMirrorCpp, m_iTest0);

};

void AMyMJGameCoreMirrorCpp::setCoreFullPartEnabled(bool bEnabled)
{
    if (bEnabled) {
        if (!getCoreFullPartEnabled()) {

            m_pCoreFull = NewObject<UMyMJGameCoreFullCpp>(this);
            
            //for replication component, only authority touch it
            if (HasAuthority()) {
                m_pMJDataAll = NewObject<UMyMJDataAllCpp>(this);
                m_pMJDataAll->SetIsReplicated(true);
            }

            UWorld *world = GetWorld();
            if (IsValid(world)) {
                world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
                world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJGameCoreMirrorCpp::toCoreFullLoop, ((float)MY_MJ_GAME_CORE_MIRROR_TO_CORE_FULL_LOOP_TIME_MS) / (float)1000, true);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
                MY_VERIFY(false);
            }

        }
    }
    else {
        m_pCoreFull = NULL;

        if (HasAuthority()) {
            m_pMJDataAll = NULL;
        }

        UWorld *world = GetWorld();
        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
            MY_VERIFY(false);
        }
    }
};


void AMyMJGameCoreMirrorCpp::toCoreFullLoop()
{
    MY_VERIFY(IsValid(m_pMJDataAll));
    MY_VERIFY(IsValid(m_pCoreFull));

    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        MY_VERIFY(false);
    }

    float timeNow = world->GetTimeSeconds();
    uint32 timeNowMs = timeNow * 1000;
    timeNowMs = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(timeNowMs);


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


AMyMJGameCoreWithVisualCpp::AMyMJGameCoreWithVisualCpp() : Super()
{
    m_pDataHistoryBuffer = NULL;
    m_eVisualMode = MyMJCoreBaseForBpVisualModeCpp::Normal;
    m_uiReplicateClientTimeMs = 0;

    //UClass* uc = this->GetClass();
    //UObject* CDO = NULL;
    //if (uc) {
    //    CDO = uc->GetDefaultObject();
    //}

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCoreWithVisualCpp(), %s, this %p, cdo %p."), uc ? *uc->GetFullName() : TEXT("NULL"), this, CDO);

    m_cDataNow.getAccessorRef().setHelperAttenderSlotDirtyMasksEnabled(true);
};

AMyMJGameCoreWithVisualCpp::~AMyMJGameCoreWithVisualCpp()
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = NULL;
    //if (uc) {
        //CDO = uc->GetDefaultObject();
    //}

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~AMyMJGameCoreWithVisualCpp(), %s, this %p, cdo %p."), uc ? *uc->GetFullName() : TEXT("NULL"), this, CDO);
};

void AMyMJGameCoreWithVisualCpp::BeginPlay()
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCoreWithVisualCpp::BeginPlay()."));
    Super::BeginPlay();

    bool bHaveVisual = UMyMJBPUtilsLibrary::haveClientVisualLayer(this);
    if (bHaveVisual) {
        setCoreMirrorPartEnabled(true);
    }
};

void AMyMJGameCoreWithVisualCpp::setCoreMirrorPartEnabled(bool bEnabled)
{
    if (bEnabled) {
        if (!getCoreMirrorPartEnabled()) {
            m_pDataHistoryBuffer = NewObject<UMyMJDataSequencePerRoleCpp>(this);
            m_pDataHistoryBuffer->resizeEvents(128);

            UWorld *world = GetWorld();
            if (IsValid(world)) {
                world->GetTimerManager().ClearTimer(m_cForVisualLoopTimerHandle);
                world->GetTimerManager().SetTimer(m_cForVisualLoopTimerHandle, this, &AMyMJGameCoreWithVisualCpp::forVisualLoop, ((float)MY_MJ_GAME_CORE_MIRROR_FOR_VISUAL_LOOP_TIME_MS) / (float)1000, true);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid, this is only valid for default object."));
            }
        }
    }
    else {
        m_pDataHistoryBuffer = NULL;
        UWorld *world = GetWorld();
        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cForVisualLoopTimerHandle);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid, this is only valid for default object."));
        }
    }
}

bool AMyMJGameCoreWithVisualCpp::tryAppendData2Buffer()
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

void AMyMJGameCoreWithVisualCpp::forVisualLoop()
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("forVisualLoop."));

    tryAppendData2Buffer();

    UWorld* world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world of core is not valid!"));
        return;
    }

    uint32 clientTimeNow_ms = world->GetTimeSeconds() * 1000;
    clientTimeNow_ms = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(clientTimeNow_ms);

    if (m_eVisualMode == MyMJCoreBaseForBpVisualModeCpp::Normal) {
        forVisualLoopModeNormal(clientTimeNow_ms);
    }
    else if (m_eVisualMode == MyMJCoreBaseForBpVisualModeCpp::CatchUp) {
        forVisualLoopModeCatchUp(clientTimeNow_ms);
    }

};

void AMyMJGameCoreWithVisualCpp::forVisualLoopModeNormal(uint32 clientTimeNow_ms)
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

void AMyMJGameCoreWithVisualCpp::forVisualLoopModeCatchUp(uint32 clientTimeNow_ms)
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