// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

//#include "UnrealNetwork.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameEventBase.h"

#include "MyMJGamePlayerControllerBase.generated.h"

UCLASS(Abstract)
class MYONLINECARDGAME_API AMyMJGamePlayerControllerBaseCpp : public APlayerController
{
	GENERATED_BODY()
	
public:

    AMyMJGamePlayerControllerBaseCpp() : Super()
    {
        m_eDataRoleType = MyMJGameRoleTypeCpp::Observer;
        m_eCmdRoleType = MyMJGameRoleTypeCpp::Observer;
    };

    inline
    MyMJGameRoleTypeCpp getDataRoleType() const
    {
        return m_eDataRoleType;
    };

    UFUNCTION(BlueprintPure)
    MyMJGameRoleTypeCpp getCmdRoleType() const
    {
        return m_eCmdRoleType;
    };


    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void setCmdRoleType(MyMJGameRoleTypeCpp eCmdRoleType)
    {
        m_eCmdRoleType = eCmdRoleType;
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
    MyMJGameRoleTypeCpp m_eDataRoleType;

    UPROPERTY(Replicated)
    MyMJGameRoleTypeCpp m_eCmdRoleType;
};
