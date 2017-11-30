// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

//#include "UnrealNetwork.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameEventBase.h"

#include "MyMJGameViewerPawnBase.generated.h"

UCLASS(Abstract)
class MYONLINECARDGAME_API AMyMJGameViewerPawnBaseCpp : public APawn
{
	GENERATED_BODY()
	
public:

    AMyMJGameViewerPawnBaseCpp()
    {
        m_eRoleType = MyMJGameRoleTypeCpp::Observer;
    };

    inline
    void reset()
    {
        //m_mHelerReplicationServerWorldTimeMap.Reset();
    };

    inline
    MyMJGameRoleTypeCpp getRoleType() const
    {
        return m_eRoleType;
    };

    //Object id, serverWorldTime_ms
    //TMap<uint32, uint32> m_mHelerReplicationServerWorldTimeMap;

protected:
    
    virtual void PostInitProperties() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    /*
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override
    {
        DOREPLIFETIME(AMyOnlineCardGamePlayerController, m_pIONode);
    };
    */

    UPROPERTY(Replicated)
    MyMJGameRoleTypeCpp m_eRoleType;
};
