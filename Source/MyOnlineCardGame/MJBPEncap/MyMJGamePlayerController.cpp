// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePlayerController.h"
#include "UnrealNetwork.h"

#define MY_PUSHER_LOOP_ON_PROXY_TIME_MS 1000
#define MY_PUSHER_LOOP_NO_SYNC_TRY_DOWN_LIMIT_TIME_MS 3000

void AMyMJGamePlayerControllerCpp::PostInitProperties()
{
    Super::PostInitProperties();
}

void AMyMJGamePlayerControllerCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyMJGamePlayerControllerCpp, m_eRoleType);
};