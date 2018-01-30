// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePlayerController.h"
#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGameDeskVisualData.h"

AMyMJGamePlayerControllerCpp::AMyMJGamePlayerControllerCpp() : Super()
{
    bReplicates = true;
    bAlwaysRelevant = false;
    bOnlyRelevantToOwner = true; //subclass can change it
    bNetLoadOnClient = true;
    NetUpdateFrequency = 5;

    m_pExtRoomActor = NULL;
    m_pExtRoomTrivalDataSource = NULL;
    m_pExtRoomCoreDataSourceSeq = NULL;

    m_fHelperFilterLastRepClientRealtTime = 0;

    m_fLastAnswerSyncForMJCoreFullDataWorldRealTime = 0;
    m_bNeedAnswerSyncForMJCoreFullData = false;
    m_fLastAskSyncForMJCoreFullDataWorldRealTime = 0;
    m_bNeedAskSyncForMJCoreFullData = false;
    m_bNeedRetryFeedDataForCore = false;
    m_bUseAsLocalClientDataBridge = false;

    m_eDebugNetmodeAtStart = ENetMode::NM_MAX;
    m_bDebugHaltFeedData = false;

};

AMyMJGamePlayerControllerCpp::~AMyMJGamePlayerControllerCpp()
{

};

void AMyMJGamePlayerControllerCpp::clearInGame()
{
    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }
};

void AMyMJGamePlayerControllerCpp::BeginPlay()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGamePlayerControllerCpp BeginPlay()"));

    Super::BeginPlay();

    if (UMyMJBPUtilsLibrary::haveServerLogicLayer(this)) {
        //In most case, this equals netmode == DS, LS, standalone

        //warn: we forbid the using local multiple player mode!
        m_bUseAsLocalClientDataBridge = IsLocalPlayerController();

        resetupWithViewRoleAndAuth(m_eDataRoleType, m_bUseAsLocalClientDataBridge);
    }
    else {
        m_bUseAsLocalClientDataBridge = false;
    }

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJGamePlayerControllerCpp::loop, (float)MyMJGamePlayerControllerLoopTimeMs / 1000);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
        MY_VERIFY(false);
    }

    m_eDebugNetmodeAtStart = GetNetMode();
};

void AMyMJGamePlayerControllerCpp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    clearInGame();
};


void AMyMJGamePlayerControllerCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyMJGamePlayerControllerCpp, m_pExtRoomActor);
    DOREPLIFETIME(AMyMJGamePlayerControllerCpp, m_pExtRoomTrivalDataSource);
    DOREPLIFETIME(AMyMJGamePlayerControllerCpp, m_pExtRoomCoreDataSourceSeq);
};

bool AMyMJGamePlayerControllerCpp::resetupWithViewRoleAndAuth(MyMJGameRoleTypeCpp eRoleType, bool bUseAsLocalClientDataBridge)
{
    MY_VERIFY(HasAuthority());

    AMyMJGameRoomRootActorCpp *pRS = AMyMJGameRoomRootActorCpp::helperGetRoomRootActor(this);

    /*
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return false;
    }

    ULevel *pL = world->PersistentLevel;
    if (!IsValid(pL)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("persisten level is invalid, %p."), pL);
        return false;
    }

    ALevelScriptActor* pLSA = pL->GetLevelScriptActor();

    if (!IsValid(pLSA)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ALevelScriptActoris invalid, %p."), pLSA);
        return false;
    }

    AMyMJGameRoomLevelScriptActorCpp *pLSASub = Cast<AMyMJGameRoomLevelScriptActorCpp>(pLSA);
    */

    if (!IsValid(pRS)) {
        return false;
    }

    m_pExtRoomActor = pRS->m_pRoomActor;
    if (!IsValid(m_pExtRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("room is invalid, not cfged?, %p."), m_pExtRoomActor);
        return false;
    }

    m_pExtRoomTrivalDataSource = pRS->m_pTrivalDataSource;
    if (!IsValid(m_pExtRoomTrivalDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomTrivalDataSource is invalid, not cfged?, %p."), m_pExtRoomTrivalDataSource);
        return false;
    }

    //at last, update the sequence
    if ((uint8)eRoleType >= (uint8)MyMJGameRoleTypeCpp::Max) {
        m_pExtRoomCoreDataSourceSeq = NULL;
        return true;
    }

    //return false; //test

    AMyMJGameCoreDataSourceCpp* pSource = pRS->m_pCoreDataSource;
    if (!IsValid(pSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data source is invalid, not cfged?, %p."), pSource);
        return false;
    }

    m_pExtRoomCoreDataSourceSeq = pSource->getMJDataAll()->getDataByRoleType(eRoleType, false);
    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomCoreDataSourceSeq is invalid, not cfged?, %p, role %d."), m_pExtRoomCoreDataSourceSeq, (uint8)eRoleType);
        MY_VERIFY(false);
    }

    if (bUseAsLocalClientDataBridge) {
        m_pExtRoomCoreDataSourceSeq->m_cReplicateDelegate.Clear();
        m_pExtRoomCoreDataSourceSeq->m_cReplicateDelegate.AddUObject(this, &AMyMJGamePlayerControllerCpp::tryFeedDataToConsumerWithFilter);
    }

    return true;
};

void AMyMJGamePlayerControllerCpp::loop()
{
    ENetMode mode = GetNetMode();
    if (mode != m_eDebugNetmodeAtStart) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("net mode change detected: %d -> %d"), (uint8)m_eDebugNetmodeAtStart, (uint8)mode);
    }

    //network traffic handle
    if (UMyMJBPUtilsLibrary::haveServerNetworkLayer(this)) {
        //we have network traffic to handle as network server
        loopOfSyncForMJCoreFullDataOnNetworkServer();
    }
    else if (UMyMJBPUtilsLibrary::haveClientNetworkLayer(this))
    {
        loopOfSyncForMJCoreFullDataOnNetworkClient();
    }
};

void AMyMJGamePlayerControllerCpp::loopOfSyncForMJCoreFullDataOnNetworkServer()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    //server
    if (GetRemoteRole() == ENetRole::ROLE_None) {
        //not a network mode
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("remote role not exist, maybe connection lost?"));
        return;
    }

    if (m_bNeedAnswerSyncForMJCoreFullData && (nowRealTime - m_fLastAnswerSyncForMJCoreFullDataWorldRealTime) >= ((float)MyMJGamePlayerControllerAnswerSyncGapTimeMs / 1000)) {
        if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data sequence is invalid, not cfged?, %p."), m_pExtRoomCoreDataSourceSeq);
            return;
        }

        answerSyncForMJCoreFullDataOnClient(m_pExtRoomCoreDataSourceSeq->getRole(), m_pExtRoomCoreDataSourceSeq->getFullData());

        m_bNeedAnswerSyncForMJCoreFullData = false;
        m_fLastAnswerSyncForMJCoreFullDataWorldRealTime = nowRealTime;
    }
};

void AMyMJGamePlayerControllerCpp::loopOfSyncForMJCoreFullDataOnNetworkClient()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    if (m_bNeedAskSyncForMJCoreFullData && (nowRealTime - m_fLastAskSyncForMJCoreFullDataWorldRealTime) >= ((float)MyMJGamePlayerControllerAskSyncGapTimeMs / 1000)) {
        if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data sequence is invalid, not cfged?, %p."), m_pExtRoomCoreDataSourceSeq);
            return;
        }

        askSyncForMJCoreFullDataOnServer();

        m_bNeedAskSyncForMJCoreFullData = false;
        m_fLastAskSyncForMJCoreFullDataWorldRealTime = nowRealTime;
    }

    if (m_bNeedRetryFeedDataForCore) {
        tryFeedDataToConsumer();
    }
}

void AMyMJGamePlayerControllerCpp::setDataRoleTypeWithAuth(MyMJGameRoleTypeCpp eRoleType)
{
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("can't set Role Type without auth."));
        return;
    }

    if (eRoleType >= MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRoleType not valid"));
        return;
    }

    if (m_pExtRoomCoreDataSourceSeq == NULL || m_eDataRoleType != eRoleType) {
        //update the pointer
        resetupWithViewRoleAndAuth(eRoleType, m_bUseAsLocalClientDataBridge);
    }

    m_eDataRoleType = eRoleType;
};

void AMyMJGamePlayerControllerCpp::askSyncForMJCoreFullDataOnServer_Implementation()
{
    //FPlatformProcess::Sleep(v1);
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return;
    }

    markNeedAnswerSyncForMJCoreFullData();
}

bool AMyMJGamePlayerControllerCpp::askSyncForMJCoreFullDataOnServer_Validate()
{
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return false;
    }

    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomCoreDataSourceSeq not valid, %p"), m_pExtRoomCoreDataSourceSeq);
        return false;
    }

    return true;

}

void AMyMJGamePlayerControllerCpp::answerSyncForMJCoreFullDataOnClient_Implementation(MyMJGameRoleTypeCpp eRole, const FMyMJDataStructWithTimeStampBaseCpp& cFullData)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("answerSyncForMJCoreFullDataOnClient_Implementation()!"));

    //FPlatformProcess::Sleep(v1);
    if (HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return;
    }

    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtCoreDataSeq not valid, %p"), m_pExtRoomCoreDataSourceSeq);
        return;
    }

    m_pExtRoomCoreDataSourceSeq->tryUpdateFullDataFromExternal(eRole, cFullData);
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("full data updated!"));
    if (tryFeedDataToConsumer()) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("full data updated but still need sync, need to note."));
    }
}


void AMyMJGamePlayerControllerCpp::tryFeedDataToConsumerWithFilter()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    if (nowRealTime <= m_fHelperFilterLastRepClientRealtTime) {
        return;
    }

    m_fHelperFilterLastRepClientRealtTime = nowRealTime;

    //ENetMode mode = GetNetMode();
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("OnRep_NewDataArrivedWithFilter, ENetMode %d, NM_Standalone %d"), (uint8)mode, (uint8)ENetMode::NM_Standalone);
    
    tryFeedDataToConsumer();
};


bool AMyMJGamePlayerControllerCpp::tryFeedDataToConsumer()
{
    if (m_bDebugHaltFeedData) {
        return false;
    }

    if (!IsValid(m_pExtRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pExtRoomActor is not valid now. %p."), m_pExtRoomActor);
        return false;
    }

    bool bRet = false;
    if (IsValid(m_pExtRoomCoreDataSourceSeq)) {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("feeding data %d."), m_bDebugHaltFeedData);
        bRet = m_pExtRoomActor->getRoomDataSuiteVerified()->getDeskDataObjVerified()->tryFeedData(m_pExtRoomCoreDataSourceSeq, &m_bNeedRetryFeedDataForCore);
    }

    if (bRet) {
        markNeedAskSyncForMJCoreFullData();
    }

    return bRet;
};


