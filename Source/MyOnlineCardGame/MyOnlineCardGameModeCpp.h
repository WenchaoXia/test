// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyOnlineCardGameModeCpp.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyOnlineCardGameModeCpp : public AGameMode
{
    GENERATED_BODY()

public:
    AMyOnlineCardGameModeCpp()
    {
        m_bReplayMode = false;
    };

    virtual ~AMyOnlineCardGameModeCpp()
    {

    };

    bool changeMode(bool bIsReplay);

    UPROPERTY()
    UMyMJGameCoreFullCpp *m_pCoreFull;

    bool m_bReplayMode;
	
};
