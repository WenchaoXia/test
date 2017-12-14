// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomLevelScriptActorCpp.h"

#include "Engine.h"


bool AMyMJGameRoomRootActorCpp::checkSettings() const
{
    if (!IsValid(m_pRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomActor %p is not valid."), m_pRoomActor);
        return false;
    }

    if (!m_pRoomActor->checkSettings()) {
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


AMyMJGameRoomRootActorCpp* AMyMJGameRoomRootActorCpp::helperGetRoomRootActor(const UObject* WorldContextObject)
{
    if (!IsValid(GEngine)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("GEngine %p is not valid."), GEngine);
        return NULL;
    }
    
    UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return NULL;
    }

    ULevel *pL = world->PersistentLevel;
    if (!IsValid(pL)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("persisten level is invalid, %p."), pL);
        return NULL;
    }

    ALevelScriptActor* pLSA = pL->GetLevelScriptActor();

    if (!IsValid(pLSA)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ALevelScriptActoris invalid, %p."), pLSA);
        return NULL;
    }

    AMyMJGameRoomLevelScriptActorCpp *pLSASub = Cast<AMyMJGameRoomLevelScriptActorCpp>(pLSA);
    if (!IsValid(pLSASub)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to cast %p, original class name is %s."), pLSASub, *pLSA->GetClass()->GetName());
        return NULL;
    }

    AMyMJGameRoomRootActorCpp* pRet = pLSASub->m_pRoomRootActor;
    if (!IsValid(pRet)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomRootActor not valid %p."), pRet);
        return NULL;
    }

    return pRet;
};

bool AMyMJGameRoomLevelScriptActorCpp::checkSettings() const
{
    if (!IsValid(m_pRoomRootActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRoomRootActor not valid %p."), m_pRoomRootActor);
        return false;
    }

    return m_pRoomRootActor->checkSettings();
};



