// Fill out your copyright notice in the Description page of Project Settings.

//#include "MyOnLineCardGame.h"
#include "MyOnlineCardGameModeCpp.h"

#include "Engine/World.h"
#include "Runtime/Engine/Classes/Engine//World.h"

bool AMyOnlineCardGameModeCpp::changeMode(bool bIsReplay)
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid world!"));
        return false;
    }

    if (bIsReplay) {
        if (IsValid(m_pCoreFull)) {
            m_pCoreFull->MarkPendingKill();
            m_pCoreFull = NULL;
        }
    }
    else {
        if (IsValid(m_pCoreFull)) {
        }
        else {

            //m_pCoreFull = world->SpawnActor<UMyMJGameCoreFullCpp>(UMyMJGameCoreFullCpp::StaticClass());
            m_pCoreFull = NewObject<UMyMJGameCoreFullCpp>(this);
            if (!IsValid(m_pCoreFull)) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to create core full!"));
                return false;
            }

        }
    }


    return true;
}

