// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomLevelScriptActorCpp.h"

bool AMyMJGameRoomLevelScriptActorCpp::checkSettings() const
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




