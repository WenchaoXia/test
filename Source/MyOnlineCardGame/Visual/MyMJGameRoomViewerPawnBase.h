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
class MYONLINECARDGAME_API AMyMJGameRoomViewerPawnBaseCpp : public APawn, public IMyPawnUIInterfaceCpp, public IMyTransformUpdaterInterfaceCpp
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

    //IMyTransformUpdaterInterfaceCpp begin

    virtual int32 getModelInfo(FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const override
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not implemented yet!"));
        return -1;
    };

    virtual FMyWithCurveUpdaterTransformCpp& getMyWithCurveUpdaterTransformRef() override
    {

        MY_VERIFY(m_pMyTransformUpdaterComponent != NULL);

        return m_pMyTransformUpdaterComponent->getMyWithCurveUpdaterTransformRefRef();
    };


    UFUNCTION(BlueprintCallable)
    void helperGetCameraData(int32 idxDeskPosition, struct FMyCardGameCameraDataCpp& cameraData) const;

    //stand in which attender's postion, range is always [0, 4)
    UFUNCTION(BlueprintCallable)
    void changeInRoomDeskPosition(int32 idxDeskPosition);

    //IMyTransformUpdaterInterfaceCpp end
protected:
    
    //IMyPawnUIInterfaceCpp begin

    virtual void OnPossessedByLocalPlayerController(APlayerController* newController) override;
    virtual void OnUnPossessedByLocalPlayerController(APlayerController* oldController) override;

    //IMyPawnUIInterfaceCpp end

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    void updaterOnStepUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector);
    void updaterOnStepFinish(const FMyWithCurveUpdateStepDataBasicCpp& data);


    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Instanced, meta = (DisplayName = "camera component"))
    UCameraComponent* m_pCamera;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "my transform updater component"))
    UMyTransformUpdaterComponent* m_pMyTransformUpdaterComponent;

};
