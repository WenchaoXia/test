// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

//#include "UnrealNetwork.h"

#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyOnlineCardGamePlayerController.generated.h"

#define MY_S2CPusherSync_DEBUG_MASK_C2S_SYNC_RESP 0x01
/**
 * 
 */
UCLASS()
class MYONLINECARDGAME_API AMyOnlineCardGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
    AMyOnlineCardGamePlayerController()
    {
    };

    virtual void PostInitProperties() override;

protected:

    /*
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override
    {
        DOREPLIFETIME(AMyOnlineCardGamePlayerController, m_pIONode);
    };
    */

};
