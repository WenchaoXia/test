// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

//#include "UnrealNetwork.h"

#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyMJGamePlayerController.generated.h"

#define MY_S2CPusherSync_DEBUG_MASK_C2S_SYNC_RESP 0x01
/**
 * 
 */
UCLASS()
class MYONLINECARDGAME_API AMyMJGamePlayerControllerCpp : public APlayerController
{
	GENERATED_BODY()
	
public:
        AMyMJGamePlayerControllerCpp()
    {
        m_eRoleType = MyMJGameRoleTypeCpp::Observer;
    };

    virtual void PostInitProperties() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(Replicated)
    MyMJGameRoleTypeCpp m_eRoleType;

protected:

    /*
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override
    {
        DOREPLIFETIME(AMyOnlineCardGamePlayerController, m_pIONode);
    };
    */

};
