// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyOnlineCardGameMode.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyOnlineCardGameModeCpp : public AGameMode
{
    GENERATED_BODY()

public:
    AMyOnlineCardGameModeCpp() : Super()
    {
        m_bReplayMode = false;
    };

    virtual ~AMyOnlineCardGameModeCpp()
    {

    };

    bool changeMode(bool bIsReplay);


    bool m_bReplayMode;
	
};
