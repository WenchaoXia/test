// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomViewerPawnBase.h"
#include "MyMJGamePlayerController.h"
#include "MyMJGameRoom.h"

#include "Kismet/GameplayStatics.h"

#include "Runtime/UMG/Public/Blueprint/SlateBlueprintLibrary.h"
#include "Runtime/UMG/Public/Blueprint/WidgetLayoutLibrary.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGameDeskVisualData.h"

#define JudgePawnAlreadyInRoomCoOfRadius (1.2f)


//a step data support controlled roll
struct FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp : public FMyWithCurveUpdateStepDataTransformWorld3DCpp
{

public:

    FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp() : FMyWithCurveUpdateStepDataTransformWorld3DCpp()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp");
        reset(true);
    };

    virtual ~FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp()
    {
    };

    inline void reset(bool resetSubClassDataOnly = false)
    {
        if (!resetSubClassDataOnly) {
            FMyWithCurveUpdateStepDataTransformWorld3DCpp::reset();
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

    m_pMyTransformUpdaterComponent = CreateDefaultSubobject<UMyWithCurveUpdaterTransformWorld3DComponent>(TEXT("my transform updater component"));

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

AMyMJGamePlayerControllerCpp* AMyMJGameRoomViewerPawnBaseCpp::getMJGamePlayerController() const
{
    if (this->Controller == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Controller is NULL."));
        return NULL;
    }

    AMyMJGamePlayerControllerCpp* pC = Cast<AMyMJGamePlayerControllerCpp>(Controller);
    if (pC == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("failed to cast, it's class is %s."), *Controller->GetClass()->GetName());
        return NULL;
    }

    return pC;
}



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


    FMyWithCurveUpdaterTransformWorld3DCpp* pUpdater = &getMyWithCurveUpdaterTransformRef();
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


MyErrorCodeCommonPartCpp AMyMJGameRoomViewerPawnBaseCpp::OnPossessedByLocalPlayerController(APlayerController* newController)
{
    //Not doing anything now
    //int32 id = UGameplayStatics::GetPlayerControllerID(newController);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnPossessedByLocalPlayerController: %s, %d."), *newController->GetClass()->GetName(), id);

    return MyErrorCodeCommonPartCpp::NoError;
};

MyErrorCodeCommonPartCpp AMyMJGameRoomViewerPawnBaseCpp::OnUnPossessedByLocalPlayerController(APlayerController* oldController)
{
    //int32 id = UGameplayStatics::GetPlayerControllerID(oldController);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnUnPossessedByLocalPlayerController: %s, id %d."), *oldController->GetClass()->GetName(), id);

    return MyErrorCodeCommonPartCpp::NoError;
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

    if (!FMath::IsNearlyEqual(getCameraComponent()->FieldOfView, dynNow.m_fFOV, FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min))
    {
        getCameraComponent()->SetFieldOfView(dynNow.m_fFOV);
    }

    if (pData->m_bSkipCommonUpdateDelegate) {
        //use custom move
        FTransform transformNow;
        UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(pData->m_cCenterPoint, dynNow.m_cMyTransformOfZRotation, transformNow);

        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Updated: %s, %s, %s, %f. %s."), *dynNow.m_cMyTransformOfZRotation.ToString(), *pExtra->m_cDynamicDataStart.m_cMyTransformOfZRotation.ToString(), *pExtra->m_cDynamicDataEnd.m_cMyTransformOfZRotation.ToString(), vector.Y, *transformNow.ToString());
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Updated : %f, now %s, transform: %s."), vector.Y, *dynNow.m_cMyTransformOfZRotation.ToString(), *transformNow.ToString());

        if (!transformNow.Equals(GetActorTransform(), FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min)) {
            SetActorTransform(transformNow);
        }

    }

    tryUpdateUI(false);
};

void AMyMJGameRoomViewerPawnBaseCpp::updaterOnStepFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("onTransformSeqFinished."));

    const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformAndRotateAroundCenterCpp *>(&data);

    if (!FMath::IsNearlyEqual(getCameraComponent()->FieldOfView, pData->m_cDynamicDataEnd.m_fFOV, FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min))
    {
        getCameraComponent()->SetFieldOfView(pData->m_cDynamicDataEnd.m_fFOV);
    }

    if (pData->m_bSkipCommonFinishDelegate) {
        //let's do what we should, if skipped before
        SetActorTransform(pData->m_cEnd);
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("actor transform end: %s."), *data.m_cEnd.ToString());
        //data.m_cEnd.AddTranslations
    }

    tryUpdateUI(true);
}

MyErrorCodeCommonPartCpp AMyMJGameRoomViewerPawnBaseCpp::getProjectedAttenderPoints(AMyMJGamePlayerControllerCpp& myController, TArray<FVector2D>& aPoints)
{
    aPoints.Reset();

    FMyArrangePointCfgWorld3DCpp visualPoint;
    const FMyMJGameDeskVisualCfgCacheCpp& cfgCache = AMyMJGameRoomLevelScriptActorCpp::helperGetRoomActor(&myController, true)->getCfgCacheRefConst();
    if (!cfgCache.m_bValid) {
        return MyErrorCodeCommonPartCpp::InternalError;
    }

    //FGeometry geo = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(&myController);
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("player screen geo local size %s, absolute %s."),
        //*USlateBlueprintLibrary::GetLocalSize(geo).ToString(),
        //*USlateBlueprintLibrary::GetAbsoluteSize(geo).ToString());
    //FVector2D localSize = USlateBlueprintLibrary::GetLocalSize(geo);
    //float XYRatio = localSize.X / localSize.Y;


    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        if (0 != cfgCache.m_cPointCfg.getAttenderVisualPointCfg(idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp::OnDeskLocation, visualPoint)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when getAttenderVisualPointCfg, idxAttender %d."), idxAttender);
            return MyErrorCodeCommonPartCpp::InternalError;
        }

        FVector2D ProjectedPoint;
        if (!UGameplayStatics::ProjectWorldToScreen(&myController, visualPoint.m_cCenterPointTransform.GetLocation(), ProjectedPoint, true)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when project point to screen."));
            return MyErrorCodeCommonPartCpp::InternalError;
        }

        aPoints.Emplace(ProjectedPoint);

        /*
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("projected point 0: %s, local %s."), *ProjectedPoint.ToString(), *geo.AbsoluteToLocal(ProjectedPoint).ToString());

        UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(&myController, visualPoint.m_cCenterPointTransform.GetLocation(), ProjectedPoint);

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("projected point 1: %s."), *ProjectedPoint.ToString());
        */
    }


    return MyErrorCodeCommonPartCpp::NoError;
};

void AMyMJGameRoomViewerPawnBaseCpp::tryUpdateUI(bool changeModeAndCreate)
{
    AMyMJGamePlayerControllerCpp* pC = getMJGamePlayerController();
    if (pC == NULL) {
        return;
    }

    UMyMJGameUIManagerCpp* pUIManager = pC->getUIManagerVerified();
    if (changeModeAndCreate) {
        pUIManager->changeUIMode(MyMJGameUIModeCpp::InRoomPlay); //Todo:: use game settings
    }

    UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = pUIManager->getInRoomUIMain(changeModeAndCreate, false);
    if (pUI == NULL) {
        if (changeModeAndCreate) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pUI is NULL, check your settings, maybe forgot set it."));
        }
        return;
    }

    TArray<FVector2D> aPoints;
    if (getProjectedAttenderPoints(*pC, aPoints) != MyErrorCodeCommonPartCpp::NoError) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error for getProjectedAttenderPoints()."));
        return;
    }

    //FGeometry geo = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(&myController);
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("player screen geo local size %s, absolute %s."),
    //*USlateBlueprintLibrary::GetLocalSize(geo).ToString(),
    //*USlateBlueprintLibrary::GetAbsoluteSize(geo).ToString());
    //FVector2D localSize = USlateBlueprintLibrary::GetLocalSize(geo);
    //float XYRatio = localSize.X / localSize.Y;

    FGeometry geo = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(pC);
    FVector2D localSize = USlateBlueprintLibrary::GetLocalSize(geo);
    float XYRatio = localSize.X / localSize.Y;

    pUI->updateAttenderPositions(XYRatio, aPoints);
    if (changeModeAndCreate) {
        //we want to create, make sure it is updated
        //pUI->updateUI(); //for test purpose
    }
};