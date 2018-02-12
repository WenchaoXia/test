// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CardGameUtils/MyCardGameUtilsLibrary.h"
#include "MJBPEncap/Utils/MyMJBPUtils.h"

#include "MyMJGameVisualInterfaces.h"
#include "Camera/CameraComponent.h"

#include "MyMJGameVisualCfg.h"

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
    inline UCameraComponent* getCameraComponent() const
    {
        return m_pCamera;
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


    UFUNCTION(BlueprintCallable)
    void helperGetCameraData(int32 idxAttender, struct FMyCardGameCameraDataCpp& cameraData) const;
    
    UFUNCTION(BlueprintCallable)
    void changeInRoomViewRole(MyMJGameRoleTypeCpp roleType);

    UFUNCTION(BlueprintCallable)
    void changeInRoomViewRole2(int32 idxAttender)
    {
        changeInRoomViewRole((MyMJGameRoleTypeCpp) idxAttender);
    };

    //IMyTransformUpdateSequenceInterface end
protected:
    
    //IMyPawnUIInterface begin

    virtual void OnPossessedByLocalPlayerController(APlayerController* newController) override;
    virtual void OnUnPossessedByLocalPlayerController(APlayerController* oldController) override;

    //IMyPawnUIInterface end

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    void onTransformSeqUpdated(const struct FTransformUpdateSequencDataCpp& data, const FVector& vector);
    void onTransformSeqFinished(const struct FTransformUpdateSequencDataCpp& data);

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Instanced, meta = (DisplayName = "camera component"))
    UCameraComponent* m_pCamera;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "transform update sequence"))
    UMyTransformUpdateSequenceMovementComponent* m_pTransformUpdateSequence;

};
