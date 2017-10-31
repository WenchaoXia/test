// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"


#include "TimerManager.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"
#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

void AMyMJGameCoreDataSourceCpp::BeginPlay()
{
    Super::BeginPlay();

    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(this);
    if (bHaveLogic) {
        setCoreFullPartEnabled(true);
    }

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCoreDataSourceCpp::BeginPlay(), %d."), bHaveLogic);
};

void AMyMJGameCoreDataSourceCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyMJGameCoreDataSourceCpp, m_pMJDataAll);
    DOREPLIFETIME(AMyMJGameCoreDataSourceCpp, m_iTest0);

};

void AMyMJGameCoreDataSourceCpp::doTestChange()
{
    if (m_pMJDataAll) {
        m_pMJDataAll->doTestChange();
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMJDataStorage is NULL!"));
    }

    m_iTest0 += 3;
};


FString AMyMJGameCoreDataSourceCpp::genDebugMsg() const
{
    FString ret = FString::Printf(TEXT("m_iTest0 %d. "), m_iTest0);
    if (m_pMJDataAll) {
        ret += m_pMJDataAll->genDebugMsg();
    }
    else {
        ret += TEXT("m_pMJDataStorage is NULL!");
    }

    return ret;
};

void AMyMJGameCoreDataSourceCpp::verifyEvents() const
{
    if (!IsValid(m_pMJDataAll)) {
        return;
    }
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("verifyEvents() exectuing."));

    for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
        UMyMJDataSequencePerRoleCpp*pSeq = m_pMJDataAll->getDataByRoleType((MyMJGameRoleTypeCpp)i, false);
        if (!IsValid(pSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("skipping role %d's data since not valid."), i);
            continue;
        }
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("verifyEvents() for role %d."), i);
        int32 iError = pSeq->getEventsRef().verifyData(true);
        if (iError != 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d's data have error %d."), i, iError);
            //MY_VERIFY(false);
        }
    }

};

void AMyMJGameCoreDataSourceCpp::startGameCoreTestInSubThread(bool showCoreLog, bool showDataLog, bool bAttenderRandomSelectHighPriActionFirst)
{
    int32 iMask = MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero;
    if (showCoreLog) {
        iMask |= MyMJGameCoreTrivalConfigMaskShowPusherLog;
    }

    while (!tryChangeMode(MyMJGameRuleTypeCpp::LocalCS, iMask)) {
        FPlatformProcess::Sleep(0.1);
    }

    m_pMJDataAll->setShowDebugLog(showDataLog);
    startGame(true, bAttenderRandomSelectHighPriActionFirst);
}


bool AMyMJGameCoreDataSourceCpp::tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask)
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

bool AMyMJGameCoreDataSourceCpp::startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst)
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

void AMyMJGameCoreDataSourceCpp::clearUp()
{
    if (m_pCoreFullWithThread.IsValid()) {
        m_pCoreFullWithThread->Stop();
    }
}

bool AMyMJGameCoreDataSourceCpp::getCoreFullPartEnabled() const
{
    return IsValid(m_pMJDataAll);
};

void AMyMJGameCoreDataSourceCpp::setCoreFullPartEnabled(bool bEnabled)
{
    if (bEnabled) {
        if (!getCoreFullPartEnabled()) {

            m_pMJDataAll = NewObject<UMyMJDataAllCpp>(this);
            m_pMJDataAll->createSubObjects();
            m_pMJDataAll->SetIsReplicated(true);

            UWorld *world = GetWorld();
            if (IsValid(world)) {
                world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
                world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJGameCoreDataSourceCpp::coreDataPullLoop, ((float)MY_MJ_GAME_CORE_MIRROR_TO_CORE_FULL_LOOP_TIME_MS) / (float)1000, true);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
                MY_VERIFY(false);
            }

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("CoreFullPartEnabled is changed to true."));
        }
    }
    else {

        m_pMJDataAll = NULL;

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

void AMyMJGameCoreDataSourceCpp::OnRep_MJDataAll()
{
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_MJDataAll()."), m_iTest0);
    for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
        UMyMJDataSequencePerRoleCpp* pSeq = m_pMJDataAll->getDataByRoleType((MyMJGameRoleTypeCpp)i, false);
        if (!IsValid(pSeq)) {
            continue;
        }

        pSeq->m_cReplicateDelegate.Clear();
        pSeq->m_cReplicateDelegate.AddUObject(this, &AMyMJGameCoreDataSourceCpp::OnRep_MJDataAllContent);
    }
}

void AMyMJGameCoreDataSourceCpp::OnRep_MJDataAllContent(MyMJGameRoleTypeCpp eRole)
{
    float clientTimeNow = 0;
    UWorld* world = GetWorld();
    if (IsValid(world)) {
        clientTimeNow = world->GetTimeSeconds();
    }
    uint32 clientTimeNowMs = clientTimeNow * 1000;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_MJDataAllContent: %d, %d"), clientTimeNowMs, (uint8)eRole);
    //filter out duplicated notify
    if (clientTimeNowMs == m_uiLastReplicateClientTimeMs) {
        return;
    }
    m_uiLastReplicateClientTimeMs = clientTimeNowMs;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("broadCast filtered notify: %d, %d"), m_uiLastReplicateClientTimeMs, (uint8)eRole);
    m_cReplicateFilteredDelegate.Broadcast(eRole);
}

void AMyMJGameCoreDataSourceCpp::coreDataPullLoop()
{
    MY_VERIFY(IsValid(m_pMJDataAll));

    //const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("coreDataPullLoop thread id: %d"), CurrentThreadId);

    TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc>* pQ;
    if (m_pCoreFullWithThread.IsValid()) {
        pQ = &m_pCoreFullWithThread->getIOGourpAll().m_cPusherResultQueue;
    }
    else {
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }

    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        MY_VERIFY(false);
    }

    float timeNow = world->GetTimeSeconds();
    uint32 timeNowMs = timeNow * 1000;
    timeNowMs = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(timeNowMs);


    //2nd, handle pusher result

    bool bHaveNextPuhserResult = !pQ->IsEmpty();
    bool bIsReadyFOrNextPushserResult = m_pMJDataAll->isReadyToGiveNextPusherResult(timeNowMs);

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("bHaveNextPuhserResult0 %d, bIsReadyFOrNextPushserResult %d."), bHaveNextPuhserResult, bIsReadyFOrNextPushserResult);

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

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("bHaveNextPuhserResult1 %d, bIsReadyFOrNextPushserResult %d."), bHaveNextPuhserResult, bIsReadyFOrNextPushserResult);
    }

    if (bHaveProgress) {
        m_pMJDataAll->markAllDirtyForRep();
    }

}


UMyMJGameCoreWithVisualCpp::UMyMJGameCoreWithVisualCpp() : Super()
{
    m_pDataSource = NULL;
    //m_pTestObj = CreateDefaultSubobject<UMyTestObject>(TEXT("test Obj"));
    //m_pDataHistoryBuffer = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("buffer"));
    //m_pDataHistoryBuffer0 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("history buffer 0"));
    //m_pDataHistoryBuffer2 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("history buffer 2"));
    m_pDataHistoryBuffer0 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("data history buffer"));
    //m_pDataHistoryBuffer2 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("data history buffer 2"));
    m_eVisualMode = MyMJCoreBaseForBpVisualModeCpp::Normal;
    m_cDataNow.getAccessorRef().setHelperAttenderSlotDirtyMasksEnabled(true);

    UClass* uc = this->GetClass();
    UObject* CDO = NULL;
    if (uc) {
        CDO = uc->GetDefaultObject();
    }

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJGameCoreWithVisualCpp(), %s, this %p, cdo %p."), uc ? *uc->GetFullName() : TEXT("NULL"), this, CDO);



    //m_pTestComp = CreateDefaultSubobject<UActorComponent>(TEXT("test Obj"));
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("0 m_pDataHistoryBuffer0 %p, m_pDataHistoryBuffer2 %p, m_pTestObj %p, this %p, cdo %p."), m_pDataHistoryBuffer0, m_pDataHistoryBuffer2, m_pTestObj, this, CDO);
};

UMyMJGameCoreWithVisualCpp::~UMyMJGameCoreWithVisualCpp()
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = NULL;
    //if (uc) {
        //CDO = uc->GetDefaultObject();
    //}

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~UMyMJGameCoreWithVisualCpp(), %s, this %p, cdo %p."), uc ? *uc->GetFullName() : TEXT("NULL"), this, CDO);
};

bool UMyMJGameCoreWithVisualCpp::checkSettings() const
{
    if (IsValid(m_pDataSource)) {
        return true;

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataSource is not valid, this is not suppsed to happen!, %p."), m_pDataSource);
        return false;
    }
};

void UMyMJGameCoreWithVisualCpp::test0()
{
    UClass* uc = this->GetClass();
    UObject* CDO = NULL;
    if (uc) {
        CDO = uc->GetDefaultObject();
    }

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("2 m_pDataHistoryBuffer0 %p, this %p, cdo %p."), this, CDO);
};


void UMyMJGameCoreWithVisualCpp::BeginPlay()
{
    Super::BeginPlay();

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJGameCoreWithVisualCpp::BeginPlay()."));

    UClass* uc = this->GetClass();
    UObject* CDO = NULL;
    if (uc) {
        CDO = uc->GetDefaultObject();
    }
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("1 m_pDataHistoryBuffer0 %p, m_pDataHistoryBuffer2 %p, m_pTestObj %p, this %p, cdo %p."), m_pDataHistoryBuffer0, m_pDataHistoryBuffer2, m_pTestObj, this, CDO);



    if (IsValid(m_pDataSource)) {
        m_pDataSource->m_cReplicateFilteredDelegate.Clear();
        m_pDataSource->m_cReplicateFilteredDelegate.AddUObject(this, &UMyMJGameCoreWithVisualCpp::onDataSeqReplicated);

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataSource is not valid, this is not suppsed to happen!, %p."), m_pDataSource);
    }
};

void UMyMJGameCoreWithVisualCpp::onDataSeqReplicated(MyMJGameRoleTypeCpp eRole)
{
    tryAppendDataToHistoryBuffer();
};

bool UMyMJGameCoreWithVisualCpp::tryAppendDataToHistoryBuffer()
{
    if (!IsValid(m_pDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pDataSource is still null"));
        return false;
    }

    const UMyMJDataAllCpp* pMJDataAll = m_pDataSource->getMJDataAllConst();

    if (!IsValid(pMJDataAll)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("pMJDataAll is still null"));
        return false;
    }

    const UMyMJDataSequencePerRoleCpp* pData = pMJDataAll->getDataByRoleTypeConst(m_pDataHistoryBuffer0->m_eRole);
    if (!IsValid(pData)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("data is still null, role %d."), (uint8)m_pDataHistoryBuffer0->m_eRole);
        return false;
    }

    if (!IsValid(m_pDataHistoryBuffer0)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataHistoryBuffer0 is invalid %p."), m_pDataHistoryBuffer0);
        MY_VERIFY(false);
    }

    if (pData->getEvents(false) == NULL || m_pDataHistoryBuffer0->getEvents(false) == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("events is null, maybe replication not complete? %p, %p"), pData->getEvents(false), m_pDataHistoryBuffer0->getEvents(false));
        return false;
    }

    return m_pDataHistoryBuffer0->mergeDataFromOther(*pData);
};

void UMyMJGameCoreWithVisualCpp::forVisualLoop()
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("forVisualLoop."));

    tryAppendDataToHistoryBuffer();

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

void UMyMJGameCoreWithVisualCpp::forVisualLoopModeNormal(uint32 clientTimeNow_ms)
{

    uint32 clientTimeNow_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(clientTimeNow_ms);

    //if you want to replay events that have passed, slow/faster the clock is not valid since it only affects the events in future, the only way is time revert. 

    //MY_VERIFY(m_cDataNow.m_aEventApplying.Num() <= 0);
    //MY_VERIFY(IsValid(m_pMJDataAll));

    UMyMJDataSequencePerRoleCpp* pBuffer = m_pDataHistoryBuffer0;
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

void UMyMJGameCoreWithVisualCpp::forVisualLoopModeCatchUp(uint32 clientTimeNow_ms)
{
    int32 iGameIdNow = m_cDataNow.getGameIdLast();
    int32 iPusherIdNow = m_cDataNow.getPusherIdLast();

    //MY_VERIFY(m_cDataNow.m_aEventApplying.Num() <= 0);
    //MY_VERIFY(IsValid(m_pMJDataAll));

    UMyMJDataSequencePerRoleCpp* pBuffer = m_pDataHistoryBuffer0;
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