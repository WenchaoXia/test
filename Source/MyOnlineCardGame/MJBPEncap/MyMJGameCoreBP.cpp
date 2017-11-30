// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"


#include "TimerManager.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"
#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

AMyMJGameCoreDataSourceCpp::AMyMJGameCoreDataSourceCpp(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    m_iTest0 = 0;
    m_iSeed2OverWrite = 0;
    m_pCoreFullWithThread = NULL;
    m_pMJDataAll = NULL;

    bReplicates = true;
    bAlwaysRelevant = true;
    bNetLoadOnClient = true;
    NetUpdateFrequency = 10;
    m_bCoreFullPartEnabled = false;

    //m_pMJDataAll = NewObject<UMyMJDataAllCpp>(this);
    m_pMJDataAll = CreateDefaultSubobject<UMyMJDataAllCpp>(TEXT("mj data all"));
    //m_pMJDataAll->createSubObjects(true);
    //m_pMJDataAll->SetIsReplicated(true);

    //m_uiLastReplicateClientTimeMs = 0;
};

AMyMJGameCoreDataSourceCpp:: ~AMyMJGameCoreDataSourceCpp()
{
    stopGame();
};

void AMyMJGameCoreDataSourceCpp::BeginPlay()
{
    Super::BeginPlay();

    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(this);
    setCoreFullPartEnabled(bHaveLogic);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCoreDataSourceCpp::BeginPlay(), %d."), bHaveLogic);
};

void AMyMJGameCoreDataSourceCpp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    stopGame();

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCoreDataSourceCpp::EndPlay()."));
};

void AMyMJGameCoreDataSourceCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyMJGameCoreDataSourceCpp, m_pMJDataAll);
    DOREPLIFETIME(AMyMJGameCoreDataSourceCpp, m_iTest0);

};

void AMyMJGameCoreDataSourceCpp::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    UWorld* w = GetWorld();

    if (!IsValid(w)) {
        return;
    }

    MY_VERIFY(IsValid(m_pMJDataAll));

    float timeNow = w->GetTimeSeconds();
    uint32 timeNowMs = timeNow * 1000;

    if (m_pMJDataAll->getServerWorldTime_ms() == timeNowMs) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Same server time ms, %d."), timeNowMs);
    }

    m_pMJDataAll->setServerWorldTime_ms(timeNowMs);
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
        int32 iError = pSeq->getDeltaDataEvents(true)->verifyData(true);
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
    if (!getCoreFullPartEnabled())
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("full part is not enabled!"));
        return false;
    };

    if (m_pCoreFullWithThread.IsValid()) {
        FMyMJGameCoreRunnableCpp& cData = m_pCoreFullWithThread->getRunnableRef();
        if (cData.getRuleType() == eRuleType) {
            return true;
        }

        m_pCoreFullWithThread.Reset();
        //Todo: return false if subthread too long
        
        
        //if (!m_pCoreFullWithThread->isCreated()) {
            //m_pCoreFullWithThread.Reset();
        //}
        //else {
            //m_pCoreFullWithThread->destroy();
            //return false;
        //}
    }

    //recheck
    if (m_pCoreFullWithThread.IsValid()) {
        return false;
    }
    else {
        if (eRuleType == MyMJGameRuleTypeCpp::Invalid) {
            //this means we want to destroy
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

        FMyThreadControlCpp<FMyMJGameCoreRunnableCpp> *pCoreFull = new FMyThreadControlCpp<FMyMJGameCoreRunnableCpp>();
        pCoreFull->getRunnableRef().initData(eRuleType, iSeed, iTrivalConfigMask);
        MY_VERIFY(pCoreFull->create());

        m_pCoreFullWithThread = MakeShareable<FMyThreadControlCpp<FMyMJGameCoreRunnableCpp>>(pCoreFull);

        UWorld *world = GetWorld();
        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
            world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJGameCoreDataSourceCpp::coreDataPullLoop, ((float)MY_MJ_GAME_CORE_FULL_MAIN_THREAD_LOOP_TIME_MS) / (float)1000, true);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
            MY_VERIFY(false);
        }

        return true;
    }


}

bool AMyMJGameCoreDataSourceCpp::startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst)
{
    if (!getCoreFullPartEnabled())
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("full part is not enabled!"));
        return false;
    };

    if (!m_pCoreFullWithThread.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreFullWithThread invalid, call changeMode() first!"));
        return false;
    }

    FMyMJGameCoreRunnableCpp& cRunData = m_pCoreFullWithThread->getRunnableRef();
    if (cRunData.getRuleType() == MyMJGameRuleTypeCpp::Invalid) {
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
    cRunData.getIOGourpAll().m_aGroups[(uint8)MyMJGameRoleTypeCpp::SysKeeper].getCmdInputQueue().Enqueue(pCmdReset);

    m_pCoreFullWithThread->kick();

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("startGame() called"));

    return true;
}

void AMyMJGameCoreDataSourceCpp::stopGame()
{
    if (m_pCoreFullWithThread.IsValid()) {
        
        UWorld *world = GetWorld();
        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
            MY_VERIFY(false);
        }

        m_pCoreFullWithThread.Reset();
    }
}

bool AMyMJGameCoreDataSourceCpp::getCoreFullPartEnabled() const
{
    return m_bCoreFullPartEnabled;
};

void AMyMJGameCoreDataSourceCpp::setCoreFullPartEnabled(bool bEnabled)
{
    if (bEnabled) {
        if (!getCoreFullPartEnabled()) {

            m_pMJDataAll->setSubobjectBehaviors(MyMJDataSequencePerRoleFullDataRecordTypeTop, true);

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("CoreFullPartEnabled is changed to true."));

            m_bCoreFullPartEnabled = true;
        }
    }
    else {

        if (getCoreFullPartEnabled()) {

            m_pMJDataAll->clear();
            m_pMJDataAll->setSubobjectBehaviors(MyMJDataSequencePerRoleFullDataRecordTypeInvalid, false);

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("CoreFullPartEnabled is changed to false."));

            m_bCoreFullPartEnabled = false;
        }
    }
};

/*
void AMyMJGameCoreDataSourceCpp::OnRep_MJDataAll()
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_MJDataAll()."), m_iTest0);
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

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_MJDataAllContent: %d, %d"), clientTimeNowMs, (uint8)eRole);
    //filter out duplicated notify
    if (clientTimeNowMs == m_uiLastReplicateClientTimeMs) {
        return;
    }
    m_uiLastReplicateClientTimeMs = clientTimeNowMs;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("broadCast filtered notify: %d, %d"), m_uiLastReplicateClientTimeMs, (uint8)eRole);
    m_cReplicateFilteredDelegate.Broadcast(eRole);
}
*/

void AMyMJGameCoreDataSourceCpp::coreDataPullLoop()
{
    MY_VERIFY(IsValid(m_pMJDataAll));

    //const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("coreDataPullLoop thread id: %d"), CurrentThreadId);

    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        MY_VERIFY(false);
    }

    float timeNow = world->GetTimeSeconds();
    uint32 timeNowMs = timeNow * 1000;

    if (timeNowMs == 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("timeNowMs is zero, skip."));
        return; //wait for next loop
    }

    TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc>* pQ;
    if (m_pCoreFullWithThread.IsValid()) {
        pQ = &m_pCoreFullWithThread->getRunnableRef().getIOGourpAll().m_cPusherResultQueue;
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("thread is not setupped, check your code!"));
        return;
    }

    //2nd, handle pusher result

    bool bHaveProgress = false;

    while (1) {

        bool bHaveNextPuhserResult = !pQ->IsEmpty();
        bool bIsReadyFOrNextPushserResult = m_pMJDataAll->isReadyToGiveNextEvent(timeNowMs);

        if (!bHaveNextPuhserResult || !bIsReadyFOrNextPushserResult) {
            break;
        }

        FMyMJGamePusherResultCpp* pPusherResult = NULL;
        pQ->Dequeue(pPusherResult);
        MY_VERIFY(pPusherResult);

        m_pMJDataAll->addPusherResult(*pPusherResult, timeNowMs);
        delete(pPusherResult);
        bHaveProgress = true;

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("bHaveNextPuhserResult1 %d, bIsReadyFOrNextPushserResult %d."), bHaveNextPuhserResult, bIsReadyFOrNextPushserResult);
    }

    if (bHaveProgress) {
        m_pMJDataAll->markAllDirtyForRep();
    }


    //Todo: pull trival events
}

/*
UMyMJGameCoreWithVisualCpp::UMyMJGameCoreWithVisualCpp() : Super()
{
    m_pDataSource = NULL;
    //m_pTestObj = CreateDefaultSubobject<UMyTestObject>(TEXT("test Obj"));
    //m_pDataHistoryBuffer = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("buffer"));
    //m_pDataHistoryBuffer0 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("history buffer 0"));
    //m_pDataHistoryBuffer2 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("history buffer 2"));
    m_pDataHistoryBuffer0 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("data history buffer"));
    m_pDataHistoryBuffer0->resizeEvents(MyMJGameVisualCoreHistoryBufferSize);
    //m_pDataHistoryBuffer2 = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("data history buffer 2"));

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

    if (!IsValid(m_pDataHistoryBuffer0)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataHistoryBuffer %p is not valid, program error!"), m_pDataHistoryBuffer0);
        return false;
    }

    if (!IsValid(m_pDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataSource is not valid, this is not suppsed to happen!, %p."), m_pDataSource);
        return false;

    }

    return true;
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

    const UMyMJDataSequencePerRoleCpp* pData = pMJDataAll->getDataByRoleTypeConst(m_pDataHistoryBuffer0->getRole());
    if (!IsValid(pData)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("data is still null, role %d."), (uint8)m_pDataHistoryBuffer0->getRole());
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

    //uint32 uiTime_ms = pMJDataAll->getServerWorldTime_ms_unresolved();
    //if (uiTime_ms == 0) {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("uiTime_data_unit is zero, skip."));
        //return false;
    //}

    return false;
    //return m_pDataHistoryBuffer0->mergeDataFromOther(*pData);
};

void UMyMJGameCoreWithVisualCpp::playGameProgressTo(uint32 uiServerTimeNew_ms, bool bCatchUp)
{
    UMyMJDataSequencePerRoleCpp* pBuffer = m_pDataHistoryBuffer0;
    MY_VERIFY(IsValid(pBuffer));

    uint32 uiNextEventTime_ms = 0;
    if (pBuffer->getEventCount() > 0) {
        uiNextEventTime_ms = pBuffer->peekEventRefAt(0).getStartTime_ms();
    }

    const FMyMJEventWithTimeStampBaseCpp* pEvent;
    bool bBaseReseted = false;
    TArray<const FMyMJEventWithTimeStampBaseCpp*> aEventsApplied;

    //we must keep the notification as less as possible in one loop
    while (1) {

        if (aEventsApplied.Num() < pBuffer->getEventCount()) {
            pEvent = &pBuffer->peekEventRefAt(aEventsApplied.Num());
        }
        else {
            pEvent = NULL;
        }

        if (pEvent == NULL || pEvent->getStartTime_ms() > uiServerTimeNew_ms) {
            break;
        }


        int32 iGameIdNow = m_cDataNow.getGameIdLast();
        int32 iPusherIdNow = m_cDataNow.getPusherIdLast();

        uint32 uiEventStartTime = pEvent->getStartTime_ms();
        uint32 uiDur = pEvent->getDuration_ms();

        MY_VERIFY(uiEventStartTime > 0);

        if (uiEventStartTime < uiServerTimeNew_ms) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("a event before base time detected!  %d < %d."), uiEventStartTime, uiServerTimeNew_ms);
        }

        bool bIsGameEndOrNotStatedNow = m_cDataNow.isGameEndOrNotStarted();
        bool bCanAppend = false;
        //bool bCanAppend = pEvent->canBeAppendedToPrev(iGameIdNow, iPusherIdNow, bIsGameEndOrNotStatedNow);

        if (bCanAppend) {

            if (pEvent->getDuration_ms() > 0) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("applying event with dur: %s."), *pEvent->genDebugMsg());
            }

            m_cDataNow.applyEvent(*pEvent);
            aEventsApplied.Emplace(pEvent);

            if (pEvent->getPusherResult(true)->m_aResultBase.Num() > 0) {
                bBaseReseted = true;
            }

        }
        else {
            m_cDataNow.resetWithBase(pBuffer->getBase());
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("reset base since buffer data can't be directly appended."));

            bBaseReseted = true;
        }


    }

    //OK, let's find what is changed and notify the UI to visualize them
    int32 l = aEventsApplied.Num();
    if (l > 0 && (bBaseReseted || bCatchUp)) {
        m_cBaseAllUpdatedDelegate.Broadcast();
    }
    else {
        for (int32 i = 0; i < l; i++) {
            pEvent = aEventsApplied[i];
            m_cEventAppliedDelegate.Broadcast(*pEvent);
        }
    }

    if (l > 0) {
        pBuffer->squashEventsToBase(l);
    }
    aEventsApplied.Reset();

    return;
}

*/

/*
//return invalid means server time should not progress
//return normal means server time should progress as normal
//return catchup means server time should reset and goto reset mode
MyMJCoreBaseForBpVisualModeCpp UMyMJGameCoreWithVisualCpp::pullNextMainData(uint32 uiServerTimeNow_ms, uint32 uiServerTimeLast_ms)
{
    UMyMJDataSequencePerRoleCpp* pBuffer = m_pDataHistoryBuffer0;
    MY_VERIFY(IsValid(pBuffer));

    //todo: move this check to outer loop
    if (!isReadyForNextMainData(uiServerTimeNow_ms)) {
        return MyMJCoreBaseForBpVisualModeCpp::Normal;
    }

    if (pBuffer->getEventCount() <= 0) {
        return MyMJCoreBaseForBpVisualModeCpp::Invalid;
    }

    //precheck conditions of buffer .0that need to switch to catchup mode

    int32 bufferFilledPercentage = pBuffer->getEventCount() * 100 / pBuffer->getEventSizeMax();
    if (bufferFilledPercentage >= MyMJGameVisualCoreHistoryBufferSaturatePercentage) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("need to change to catchUp mode since buffer is too full %d/100."), bufferFilledPercentage);
        return MyMJCoreBaseForBpVisualModeCpp::CatchUp;
    }

    if (uiServerTimeNow_ms > 0) {
        if (!(uiServerTimeNow_ms > uiServerTimeLast_ms)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("uiServerTimeNow_ms %d, uiServerTimeLast_ms %d, incorrect! time screwed?"), uiServerTimeNow_ms, uiServerTimeLast_ms);
            MY_VERIFY(false);
        }

        const UMyMJGameEventCycleBuffer* pEvents = pBuffer->getEvents(true);
        if (pEvents->getLastEventStartTime() > uiServerTimeNow_ms && (pEvents->getLastEventStartTime() - uiServerTimeNow_ms) >= MyMJGameVisualCoreHistoryMaxFallBehindTimeMs) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("need to change to catchUp mode since fall befind too far, time gap: %d ms."), (pEvents->getLastEventStartTime() - uiServerTimeNow_ms));
            return MyMJCoreBaseForBpVisualModeCpp::CatchUp;
        }
    }

    while (pBuffer->getEventCount() > 0) {

        int32 iGameIdNow = m_cDataNow.getGameIdLast();
        int32 iPusherIdNow = m_cDataNow.getPusherIdLast();

        FMyMJEventWithTimeStampBaseCpp& cEvent = pBuffer->getEventRefAt(0);
        uint32 uiEventStartTime = cEvent.getStartTime();
        uint32 uiDur = cEvent.getDuration();

        if (uiServerTimeNow_ms > 0 && uiEventStartTime > 0) {
            if (uiServerTimeNow_ms >= uiEventStartTime) {
                //time reached
                if (uiServerTimeLast_ms >= uiEventStartTime) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("uiServerTimeLast_ms %d, uiEventStartTime %d, incorrect! time screwed? last event missed?."), uiServerTimeLast_ms, uiEventStartTime);
                    MY_VERIFY(false);
                }
            }
            else {
                //not reached yet
                return MyMJCoreBaseForBpVisualModeCpp::Normal;
            }
        };

        bool bIsGameEndOrNotStatedNow = m_cDataNow.isGameEndOrNotStarted();
        bool bCanAppend = cEvent.canBeAppendedToPrev(iGameIdNow, iPusherIdNow, bIsGameEndOrNotStatedNow);

        if (bCanAppend) {

            if (cEvent.getDuration() > 0) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("applying event with dur: %s."), *cEvent.genDebugMsg());
            }

            m_cDataNow.applyEvent(cEvent, FMyMJDataAtOneMomentCpp_eventApplyWay_ApplyAhead);

            pBuffer->squashEventsToBase(1);

            if (uiEventStartTime > 0) {

            }


        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("need to change to catchUp mode since data can't be appended."));
            return MyMJCoreBaseForBpVisualModeCpp::CatchUp;
        }

    }
};
*/