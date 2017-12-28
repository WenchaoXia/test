// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePlayerControllerBase.h"
#include "UnrealNetwork.h"

#define MY_PUSHER_LOOP_ON_PROXY_TIME_MS 1000
#define MY_PUSHER_LOOP_NO_SYNC_TRY_DOWN_LIMIT_TIME_MS 3000

void AMyMJGamePlayerControllerBaseCpp::PostInitProperties()
{
    Super::PostInitProperties();
}

void AMyMJGamePlayerControllerBaseCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyMJGamePlayerControllerBaseCpp, m_eViewRoleType);

   // UObject* pO;
    //pO->uni
};