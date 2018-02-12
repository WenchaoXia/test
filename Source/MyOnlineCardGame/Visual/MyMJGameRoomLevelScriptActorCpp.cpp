// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomLevelScriptActorCpp.h"

#include "MyMJGameRoom.h"

#include "Engine.h"


bool AMyMJGameRoomRootActorCpp::checkSettings() const
{
    if (!IsValid(m_pRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomActor %p is not valid."), m_pRoomActor);
        return false;
    }

    if (!m_pRoomActor->getResManagerVerified()->checkSettings(false))
    {
        return false;
    }

    if (!IsValid(m_pCoreDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreDataSource %p is not valid."), m_pCoreDataSource);
        return false;
    }

    if (!IsValid(m_pTrivalDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pTrivalDataSource %p is not valid."), m_pTrivalDataSource);
        return false;
    }

    return true;
};


AMyMJGameRoomRootActorCpp* AMyMJGameRoomRootActorCpp::helperGetRoomRootActor(const UObject* WorldContextObject, bool verifyValid)
{
    AMyMJGameRoomRootActorCpp* ret = NULL;
    while (1) {
        AMyMJGameRoomLevelScriptActorCpp* pLevel = AMyMJGameRoomLevelScriptActorCpp::helperGetLevel(WorldContextObject, verifyValid);
        if (pLevel == NULL) {
            break;
        }

        ret = pLevel->m_pRoomRootActor;
        if (!IsValid(ret)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomRootActor invalid: %p."), ret);
            ret = NULL;
            break;
        }

        break;
    }

    if (verifyValid) {
        if (!IsValid(ret)) {
            MY_VERIFY(false);
        }
    }

    return ret;
};

AMyMJGameRoomCpp* AMyMJGameRoomRootActorCpp::helperGetRoomActor(const UObject* WorldContextObject, bool verifyValid)
{
    AMyMJGameRoomCpp* ret = NULL;

    while (1)
    {
        AMyMJGameRoomRootActorCpp* pR = AMyMJGameRoomRootActorCpp::helperGetRoomRootActor(WorldContextObject, verifyValid);
        if (pR == NULL) {
            break;
        }

        ret = pR->m_pRoomActor;
        if (!IsValid(ret)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomActor invalid: %p."), ret);
            ret = NULL;
            break;
        }

        break;
    }

    if (verifyValid) {
        if (!IsValid(ret)) {
            MY_VERIFY(false);
        }
    }

    return ret;
}

bool AMyMJGameRoomLevelScriptActorCpp::checkSettings() const
{
    if (!IsValid(m_pRoomRootActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomRootActor not valid %p."), m_pRoomRootActor);
        return false;
    }

    return m_pRoomRootActor->checkSettings();
};

AMyMJGameRoomLevelScriptActorCpp* AMyMJGameRoomLevelScriptActorCpp::helperGetLevel(const UObject* WorldContextObject, bool verifyValid)
{
    AMyMJGameRoomLevelScriptActorCpp* ret = NULL;

    while (1) {
        if (!IsValid(GEngine)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("GEngine %p is not valid."), GEngine);
            break;
        }

        UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
        if (!IsValid(world)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
            break;
        }

        ULevel *pL = world->PersistentLevel;
        if (!IsValid(pL)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("persisten level is invalid, %p."), pL);
            break;
        }

        ALevelScriptActor* pLSA = pL->GetLevelScriptActor();

        if (!IsValid(pLSA)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ALevelScriptActoris invalid, %p."), pLSA);
            break;
        }

        ret = Cast<AMyMJGameRoomLevelScriptActorCpp>(pLSA);

        if (!IsValid(ret)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ALevelScriptActoris type cast fail: %p, class: %s."), ret, *pLSA->GetClass()->GetName());
            ret = NULL;
            break;
        }

        break;
    }

    if (verifyValid) {
        if (!IsValid(ret)) {
            MY_VERIFY(false);
        }
    }

    return ret;
};

void AMyMJGameRoomLevelScriptActorCpp::BeginPlay()
{
    Super::BeginPlay();

    if (!checkSettings())
    {
        return;
    }

    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(m_pRoomRootActor->m_pCoreDataSource);

    if (bHaveLogic) {
        const UMyMJGameInRoomVisualCfgType* pCfgObj = m_pRoomRootActor->m_pRoomActor->getResManagerVerified()->getVisualCfg();

        FMyMJGameEventTimeCfgCpp& cfg = m_pRoomRootActor->m_pCoreDataSource->getMJDataAll()->getEventTimeCfgRef();
        UMyMJGameInRoomVisualCfgType::helperMapToSimplifiedTimeCfg(pCfgObj->m_cEventCfg, cfg);

        m_pRoomRootActor->m_pCoreDataSource->setCoreFullPartEnabled(true);
    }
    else {
        m_pRoomRootActor->m_pCoreDataSource->setCoreFullPartEnabled(false);
    }

};


