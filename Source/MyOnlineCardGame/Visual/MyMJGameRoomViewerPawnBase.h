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
class MYONLINECARDGAME_API AMyMJGameRoomViewerPawnBaseCpp : public APawn, public IMyPawnInterfaceCpp, public IMyWithCurveUpdaterTransformWorld3DInterfaceCpp
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


    //IMyWithCurveUpdaterTransformWorld3DInterfaceCpp begin


    virtual MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoWorld3DCpp& modelInfo, bool verify) const override
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameRoomViewerPawnBaseCpp::getModelInfo() not implemented yet!"));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedOnPurPose;
    };

    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformWorld3DEnsured(struct FMyWithCurveUpdaterTransformWorld3DCpp*& outUpdater) override
    {
        MY_VERIFY(m_pMyTransformUpdaterComponent != NULL);
        outUpdater = &m_pMyTransformUpdaterComponent->getMyWithCurveUpdaterTransformWorld3DRef();

        return MyErrorCodeCommonPartCpp::NoError;
    }

    //IMyWithCurveUpdaterTransformWorld3DInterfaceCpp end


    UFUNCTION(BlueprintCallable)
    void helperGetCameraData(int32 idxDeskPosition, struct FMyCardGameCameraDataCpp& cameraData) const;

    //stand in which attender's postion, range is always [0, 4)
    UFUNCTION(BlueprintCallable)
    void changeInRoomDeskPosition(int32 idxDeskPosition);

protected:
    

    //IMyPawnInterfaceCpp begin

    virtual MyErrorCodeCommonPartCpp OnPossessedByLocalPlayerController(APlayerController* newController) override;
    virtual MyErrorCodeCommonPartCpp OnUnPossessedByLocalPlayerController(APlayerController* oldController) override;

    //IMyPawnInterfaceCpp end


    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    void updaterOnStepUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector);
    void updaterOnStepFinish(const FMyWithCurveUpdateStepDataBasicCpp& data);


    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Instanced, meta = (DisplayName = "camera component"))
    UCameraComponent* m_pCamera;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "my transform updater component"))
    UMyWithCurveUpdaterTransformWorld3DComponent* m_pMyTransformUpdaterComponent;

};
