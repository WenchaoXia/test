// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomViewerPawn.h"
#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGameDeskVisualData.h"

void AMyMJGameRoomViewerPawnCpp::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority()) {
        //In most case, this equals netmode == DS, LS, standalone
        resetupWithRoleWithAuth(m_eRoleType);
    }


    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJGameRoomViewerPawnCpp::loop, (float)MyMJGameRoomViewerPawnLoopTimeMs / 1000);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
        MY_VERIFY(false);
    }

    m_eDebugNetmodeAtStart = GetNetMode();
};

void AMyMJGameRoomViewerPawnCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyMJGameRoomViewerPawnCpp, m_pExtRoomActor);
    DOREPLIFETIME(AMyMJGameRoomViewerPawnCpp, m_pExtRoomTrivalDataSource);
    DOREPLIFETIME(AMyMJGameRoomViewerPawnCpp, m_pExtRoomCoreDataSourceSeq);
};

bool AMyMJGameRoomViewerPawnCpp::resetupWithRoleWithAuth(MyMJGameRoleTypeCpp eRoleType)
{
    MY_VERIFY(HasAuthority());

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
    if (!IsValid(pLSASub)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to cast %p, original class name is %s."), pLSASub, *pLSA->GetClass()->GetName());
        return false;
    }

    m_pExtRoomActor = pLSASub->m_pRoomActor;
    if (!IsValid(m_pExtRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("room is invalid, not cfged?, %p."), m_pExtRoomActor);
        return false;
    }

    m_pExtRoomTrivalDataSource = pLSASub->m_pTrivalDataSource;
    if (!IsValid(m_pExtRoomTrivalDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomTrivalDataSource is invalid, not cfged?, %p."), m_pExtRoomTrivalDataSource);
        return false;
    }

    //at last, update the sequence
    if ((uint8)eRoleType >= (uint8)MyMJGameRoleTypeCpp::Max) {
        m_pExtRoomCoreDataSourceSeq = NULL;
        return true;
    }

    AMyMJGameCoreDataSourceCpp* pSource = pLSASub->m_pCoreDataSource;
    if (!IsValid(pSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data source is invalid, not cfged?, %p."), pSource);
        return false;
    }

    m_pExtRoomCoreDataSourceSeq = pSource->getMJDataAll()->getDataByRoleType(eRoleType, false);
    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomCoreDataSourceSeq is invalid, not cfged?, %p, role %d."), m_pExtRoomCoreDataSourceSeq, (uint8)eRoleType);
        MY_VERIFY(false);
    }

    return true;
};

void AMyMJGameRoomViewerPawnCpp::loop()
{
    ENetMode mode = GetNetMode();
    if (mode != m_eDebugNetmodeAtStart) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("net mode change detected: %d -> %d"), (uint8)m_eDebugNetmodeAtStart, (uint8)mode);
    }

    //network traffic handle
    if (mode == ENetMode::NM_DedicatedServer || mode == ENetMode::NM_ListenServer) {
        //we have network traffic to handle as network server
        loopOfSyncForMJCoreFullDataOnNetworkServer();
    }
    else if (mode == ENetMode::NM_Client)
    {
        loopOfSyncForMJCoreFullDataOnNetworkClient();
    }
};

void AMyMJGameRoomViewerPawnCpp::loopOfSyncForMJCoreFullDataOnNetworkServer()
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

    if (m_bNeedAnswerSyncForMJCoreFullData && (nowRealTime - m_fLastAnswerSyncForMJCoreFullDataWorldRealTime) >= ((float)MyMJGameRoomViewerAnswerSyncGapTimeMs / 1000)) {
        if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data sequence is invalid, not cfged?, %p."), m_pExtRoomCoreDataSourceSeq);
            return;
        }

        answerSyncForMJCoreFullDataOnClient(m_pExtRoomCoreDataSourceSeq->getRole(), m_pExtRoomCoreDataSourceSeq->getFullData());

        m_bNeedAnswerSyncForMJCoreFullData = false;
        m_fLastAnswerSyncForMJCoreFullDataWorldRealTime = nowRealTime;
    }
};

void AMyMJGameRoomViewerPawnCpp::loopOfSyncForMJCoreFullDataOnNetworkClient()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    if (m_bNeedAskSyncForMJCoreFullData && (nowRealTime - m_fLastAskSyncForMJCoreFullDataWorldRealTime) >= ((float)MyMJGameRoomViewerAskSyncGapTimeMs / 1000)) {
        if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data sequence is invalid, not cfged?, %p."), m_pExtRoomCoreDataSourceSeq);
            return;
        }

        askSyncForMJCoreFullDataOnServer();

        m_bNeedAskSyncForMJCoreFullData = false;
        m_fLastAskSyncForMJCoreFullDataWorldRealTime = nowRealTime;
    }
}

void AMyMJGameRoomViewerPawnCpp::setRoleTypeWithAuth(MyMJGameRoleTypeCpp eRoleType)
{
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("can't set Role Type without auth."));
        return;
    }

    if (eRoleType >= MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRoleType not valid"));
        return;
    }

    if (m_pExtRoomCoreDataSourceSeq == NULL || m_eRoleType != eRoleType) {
        //update the pointer
        resetupWithRoleWithAuth(eRoleType);
    }

    m_eRoleType = eRoleType;
};

bool AMyMJGameRoomViewerPawnCpp::tryFeedDataToConsumer()
{
    if (!IsValid(m_pExtRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pExtRoomActor is not valid now. %p."), m_pExtRoomActor);
        return false;
    }

    bool bRet = false;
    if (IsValid(m_pExtRoomCoreDataSourceSeq)) {
        bRet = m_pExtRoomActor->getRoomDataSuiteVerified()->getDeskDataObjVerified()->tryFeedData(m_pExtRoomCoreDataSourceSeq);
    }

    if (bRet) {
        markNeedAskSyncForMJCoreFullData();
    }

    return bRet;
};

void AMyMJGameRoomViewerPawnCpp::askSyncForMJCoreFullDataOnServer_Implementation()
{
    //FPlatformProcess::Sleep(v1);
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return;
    }

    markNeedAnswerSyncForMJCoreFullData();
}

bool AMyMJGameRoomViewerPawnCpp::askSyncForMJCoreFullDataOnServer_Validate()
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

void AMyMJGameRoomViewerPawnCpp::answerSyncForMJCoreFullDataOnClient_Implementation(MyMJGameRoleTypeCpp eRole, const FMyMJDataStructWithTimeStampBaseCpp& cFullData)
{
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
    if (tryFeedDataToConsumer()) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("full data updated but still need sync, need to note."));
    }
}


void AMyMJGameRoomViewerPawnCpp::OnRep_NewDataArrivedWithFilter()
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

    ENetMode mode = GetNetMode();
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("OnRep_NewDataArrivedWithFilter, ENetMode %d, NM_Standalone %d"), (uint8)mode, (uint8)ENetMode::NM_Standalone);
    tryFeedDataToConsumer();
};


