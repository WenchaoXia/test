// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyOnlineCardGameStateCpp.generated.h"

/**
 * 
 */
UCLASS()
class MYONLINECARDGAME_API AMyOnlineCardGameStateCpp : public AGameState
{
	GENERATED_BODY()
	
public:

    //@&aAttenderPawns must equal (uint8)MyMJGameRoleTypeCpp::Max
    UFUNCTION(BlueprintCallable)
    void setup(AMyMJCoreMirrorCpp *pCoreMirror) {
        if (pCoreMirror->checkLevelSettings()) {
            m_pCoreMirror = pCoreMirror;
        }
    }

    UPROPERTY()
    AMyMJCoreMirrorCpp *m_pCoreMirror;
};
