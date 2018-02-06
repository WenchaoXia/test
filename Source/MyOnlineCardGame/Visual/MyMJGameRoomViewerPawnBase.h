// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CardGameUtils/MyCardGameUtilsLibrary.h"
#include "MJBPEncap/Utils/MyMJBPUtils.h"

#include "MyMJGameVisualInterfaces.h"
#include "Camera/CameraComponent.h"

#include "MyMJGameRoomViewerPawnBase.generated.h"


//Our player controller will help do replication work, and only replication help code goes here
UCLASS()
class MYONLINECARDGAME_API AMyMJGameRoomViewerPawnBaseCpp : public APawn, public IMyPawnUIInterface, public IMyTransformUpdateSequenceInterface
{
	GENERATED_BODY()

public:
    AMyMJGameRoomViewerPawnBaseCpp();
    virtual ~AMyMJGameRoomViewerPawnBaseCpp();

    //always succeed
    inline UCameraComponent* getCameraComponent()
    {
        UCameraComponent* ret = getCameraComponentInner();
        MY_VERIFY(ret);
        return ret;
    };

    //IMyTransformUpdateSequenceInterface begin

    virtual int32 getModelInfo(FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const override
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not implemented yet!"));
        return -1;
    };

    virtual UMyTransformUpdateSequenceMovementComponent* getTransformUpdateSequence(bool verify = true) override
    {
        if (verify) {
            MY_VERIFY(m_pTransformUpdateSequence);
        }

        return m_pTransformUpdateSequence;
    };

    //IMyTransformUpdateSequenceInterface end
protected:
    
    //IMyPawnUIInterface begin

    virtual void OnPossessedByLocalPlayerController(APlayerController* newController) override;
    virtual void OnUnPossessedByLocalPlayerController(APlayerController* oldController) override;

    //IMyPawnUIInterface end

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    //must succeed
    UFUNCTION(BlueprintNativeEvent)
    UCameraComponent* getCameraComponentInner();

    UCameraComponent* getCameraComponentInner_Implementation()
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameRoomViewerPawnBaseCpp::getCameraComponentInner() must be overrided by blueprint subclass!"));
        return NULL;
    };

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "transform update sequence"))
    UMyTransformUpdateSequenceMovementComponent* m_pTransformUpdateSequence;

};
