// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomViewerPawnBase.h"
#include "MyMJGamePlayerController.h"
#include "MyMJGameRoom.h"

#include "Kismet/GameplayStatics.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGameDeskVisualData.h"

#define JudgePawnAlreadyInRoomCoOfRadius (1.2f)


//a step data support controlled roll
struct FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp : public FMyWithCurveUpdateStepDataTransformCpp
{

public:

    FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp() : FMyWithCurveUpdateStepDataTransformCpp()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp");
        reset(true);
    };

    virtual ~FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp()
    {
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            FMyWithCurveUpdateStepDataTransformCpp::reset();
        }

        m_iIdxDeskPositionTarget = 0;

        FTransform t;
        m_cCenterPoint = t;

        m_cDynamicDataStart.reset();
        m_cDynamicDataEnd.reset();
    };

    virtual FMyWithCurveUpdateStepDataBasicCpp* createOnHeap() override
    {
        return new FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp *>(&other);
        *this = *pOther;
    };

    int32 m_iIdxDeskPositionTarget;

    FTransform m_cCenterPoint;

    FMyCardGameCameraDynamicDataCpp m_cDynamicDataStart;
    FMyCardGameCameraDynamicDataCpp m_cDynamicDataEnd;
};


AMyMJGameRoomViewerPawnBaseCpp::AMyMJGameRoomViewerPawnBaseCpp() : Super()
{
    //ASpectatorPawn a;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
    bReplicates = true;
    NetPriority = 3.0f;
    NetUpdateFrequency = 1.f;
    bReplicateMovement = true;

    bCanBeDamaged = false;

    BaseEyeHeight = 0.0f;
    bCollideWhenPlacing = false;
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    m_pMyTransformUpdaterComponent = CreateDefaultSubobject<UMyTransformUpdaterComponent>(TEXT("my transform updater component"));

    USceneComponent* pRootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    MY_VERIFY(IsValid(pRootSceneComponent));
    RootComponent = pRootSceneComponent;
    m_pRootScene = pRootSceneComponent;

    m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("camera component"));
    m_pCamera->SetupAttachment(m_pRootScene);

    m_pCamera->ProjectionMode = ECameraProjectionMode::Perspective;
    m_pCamera->bConstrainAspectRatio = true;
    m_pCamera->AspectRatio = (float)1920 / 1080;
};

AMyMJGameRoomViewerPawnBaseCpp::~AMyMJGameRoomViewerPawnBaseCpp()
{

};


void AMyMJGameRoomViewerPawnBaseCpp::helperGetCameraData(int32 idxDeskPosition, FMyCardGameCameraDataCpp& cameraData) const
{
    AMyMJGameRoomCpp* pRoom = AMyMJGameRoomLevelScriptActorCpp::helperGetRoomActor(this, false);
    if (pRoom == NULL) {
        return;
    }

    pRoom->getCameraData(idxDeskPosition, cameraData);
};

void AMyMJGameRoomViewerPawnBaseCpp::changeInRoomDeskPosition(int32 idxDeskPosition)
{
    idxDeskPosition = idxDeskPosition % 4;
    FMyCardGameCameraDataCpp cameraData;
    AMyMJGameRoomViewerPawnBaseCpp::helperGetCameraData(idxDeskPosition, cameraData);

    FMyCardGameCameraDynamicDataCpp& dynDataEnd = cameraData.m_cDynamicData;

    FTransform transformEnd;
    UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(cameraData.m_cStaticData.m_cCenterPoint, dynDataEnd.m_cMyTransformOfZRotation, transformEnd);

    FMyCardGameCameraDynamicDataCpp dynDataStart;
    FTransform transformNow = GetActorTransform();
    UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(cameraData.m_cStaticData.m_cCenterPoint, transformNow, dynDataStart.m_cMyTransformOfZRotation);
    dynDataStart.m_fFOV = getCameraComponent()->FieldOfView;

    //use the shortest rotate path
    float degreeDelta = FMath::FindDeltaAngleDegrees(dynDataStart.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane, dynDataEnd.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane);
    dynDataEnd.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane = dynDataStart.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane + degreeDelta;

    bool bAlreadyInRoom = dynDataStart.m_cMyTransformOfZRotation.m_cLocation.m_fRadiusOnXYPlane < (dynDataEnd.m_cMyTransformOfZRotation.m_cLocation.m_fRadiusOnXYPlane * JudgePawnAlreadyInRoomCoOfRadius);


    FMyWithCurveUpdaterTransformCpp* pUpdater = &getMyWithCurveUpdaterTransformRef();
    pUpdater->clearSteps();
    pUpdater->setHelperTransformOrigin(GetActorTransform());

    UCurveVector* pCurve = cameraData.m_cStaticData.m_pMoveCurve;
    MY_VERIFY(pCurve);

    FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp stepData, *pStepData = &stepData;
    pStepData->helperSetDataBySrcAndDst(cameraData.m_cStaticData.m_fMoveTime, pCurve, pUpdater->getHelperTransformPrevRefConst(), transformEnd);

    pStepData->m_iIdxDeskPositionTarget = idxDeskPosition;

    pStepData->m_cCenterPoint = cameraData.m_cStaticData.m_cCenterPoint;
    pStepData->m_cDynamicDataStart = dynDataStart;
    pStepData->m_cDynamicDataEnd = dynDataEnd;

    //bAlreadyInRoom = false; //test
    if (bAlreadyInRoom) {
        pStepData->m_bSkipCommonUpdateDelegate = true;
        pStepData->m_bSkipCommonFinishDelegate = true;
    }
    else {
    }

    pStepData->m_cStepUpdateDelegate.BindUObject(this, &AMyMJGameRoomViewerPawnBaseCpp::updaterOnStepUpdate);
    pStepData->m_cStepFinishDelegete.BindUObject(this, &AMyMJGameRoomViewerPawnBaseCpp::updaterOnStepFinish);

    pUpdater->addStepToTail(*pStepData);


    //Debug:

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setted: start %s, end %s."), *pExtra->m_cDynamicDataStart.ToString(), *pExtra->m_cDynamicDataEnd.ToString());

    //UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(cameraData.m_cStaticData.m_cCenterPoint, GetActorTransform(), dynDataStart.m_cMyTransformOfZRotation);
    //dynDataStart.m_fFOV = getCameraComponent()->FieldOfView;

    //FTransform transformRecovered;
    //UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(cameraData.m_cStaticData.m_cCenterPoint, dynDataStart.m_cMyTransformOfZRotation, transformRecovered);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("transformNow %s, transformRecovered %s."), *transformNow.ToString(), *transformRecovered.ToString());

};


void AMyMJGameRoomViewerPawnBaseCpp::OnPossessedByLocalPlayerController(APlayerController* newController)
{
    //Not doing anything now
    //int32 id = UGameplayStatics::GetPlayerControllerID(newController);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnPossessedByLocalPlayerController: %s, %d."), *newController->GetClass()->GetName(), id);
};

void AMyMJGameRoomViewerPawnBaseCpp::OnUnPossessedByLocalPlayerController(APlayerController* oldController)
{
    //int32 id = UGameplayStatics::GetPlayerControllerID(oldController);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnUnPossessedByLocalPlayerController: %s, id %d."), *oldController->GetClass()->GetName(), id);
};

void AMyMJGameRoomViewerPawnBaseCpp::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //NewController->GetClass()->GetName();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PossessedBy: %s."), *NewController->GetClass()->GetName());
};

void AMyMJGameRoomViewerPawnBaseCpp::UnPossessed()
{
    AController* const OldController = Controller;
    Super::UnPossessed();
};


void AMyMJGameRoomViewerPawnBaseCpp::updaterOnStepUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp *>(&data);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setted: start %s, end %s."), *pExtra->m_cDynamicDataStart.ToString(), *pExtra->m_cDynamicDataEnd.ToString());

    FMyCardGameCameraDynamicDataCpp dynNow;
    FMyCardGameCameraDynamicDataCpp::interp(pData->m_cDynamicDataStart, pData->m_cDynamicDataEnd, vector.Y, dynNow);

    if (!FMath::IsNearlyEqual(getCameraComponent()->FieldOfView, dynNow.m_fFOV, FMyWithCurveUpdateStepDataTransformCpp_Delta_Min))
    {
        getCameraComponent()->SetFieldOfView(dynNow.m_fFOV);
    }

    if (pData->m_bSkipCommonUpdateDelegate) {
        //use custom move
        FTransform transformNow;
        UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(pData->m_cCenterPoint, dynNow.m_cMyTransformOfZRotation, transformNow);

        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Updated: %s, %s, %s, %f. %s."), *dynNow.m_cMyTransformOfZRotation.ToString(), *pExtra->m_cDynamicDataStart.m_cMyTransformOfZRotation.ToString(), *pExtra->m_cDynamicDataEnd.m_cMyTransformOfZRotation.ToString(), vector.Y, *transformNow.ToString());
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Updated : %f, now %s, transform: %s."), vector.Y, *dynNow.m_cMyTransformOfZRotation.ToString(), *transformNow.ToString());

        if (!transformNow.Equals(GetActorTransform(), FMyWithCurveUpdateStepDataTransformCpp_Delta_Min)) {
            SetActorTransform(transformNow);
        }

    }

};

void AMyMJGameRoomViewerPawnBaseCpp::updaterOnStepFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("onTransformSeqFinished."));

    const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp *>(&data);

    if (!FMath::IsNearlyEqual(getCameraComponent()->FieldOfView, pData->m_cDynamicDataEnd.m_fFOV, FMyWithCurveUpdateStepDataTransformCpp_Delta_Min))
    {
        getCameraComponent()->SetFieldOfView(pData->m_cDynamicDataEnd.m_fFOV);
    }

    if (pData->m_bSkipCommonFinishDelegate) {
        //let's do what we should, if skipped before
        SetActorTransform(pData->m_cEnd);
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("actor transform end: %s."), *data.m_cEnd.ToString());
        //data.m_cEnd.AddTranslations
    }

    if (this->Controller == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Controller is NULL."));
        return;
    }

    AMyMJGamePlayerControllerCpp* pC = Cast<AMyMJGamePlayerControllerCpp>(Controller);
    if (pC == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("failed to cast, it's class is %s."), *Controller->GetClass()->GetName());
        return;
    }

    UMyMJGameUIManagerCpp* pUIManager = pC->getUIManagerVerified();
    pUIManager->changeUIMode(MyMJGameUIModeCpp::InRoomPlay);

    UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = pUIManager->getInRoomUIMain(true, false);

    if (pUI) {
        if (pUI->GetClass()->ImplementsInterface(UMyMJGameInRoomUIMainInterfaceCpp::StaticClass()))
        {
            pUI->changeDeskPositionOfIdxScreenPosition0(pData->m_iIdxDeskPositionTarget);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("pUI not implemented interface UMyPawnUIInterfaceCpp."));
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pUI is NULL, check your settings, maybe forgot set it."));
    }
}

