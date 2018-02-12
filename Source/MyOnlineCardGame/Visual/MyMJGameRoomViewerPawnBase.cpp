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

struct FMyMJGameRoomViewerPawnExtraDataCpp : public FTransformUpdateSequencExtraDataBaseCpp
{

public:

    FMyMJGameRoomViewerPawnExtraDataCpp() : FTransformUpdateSequencExtraDataBaseCpp()
    {
        m_iIdxAttenderTarget = 0;
    };

    virtual ~FMyMJGameRoomViewerPawnExtraDataCpp()
    {
    };

    virtual FTransformUpdateSequencExtraDataBaseCpp* createOnHeap() override
    {
        FMyMJGameRoomViewerPawnExtraDataCpp* ret = new FMyMJGameRoomViewerPawnExtraDataCpp();
        MY_VERIFY(ret);
        return ret;
    };

    virtual void copyContentFrom(const FTransformUpdateSequencExtraDataBaseCpp& other) override
    {
        const FMyMJGameRoomViewerPawnExtraDataCpp *pOther = StaticCast<const FMyMJGameRoomViewerPawnExtraDataCpp *>(&other);
        *this = *pOther;
    };

    int32 m_iIdxAttenderTarget;

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

    m_pTransformUpdateSequence = CreateDefaultSubobject<UMyTransformUpdateSequenceMovementComponent>(TEXT("transform update sequence movement component"));

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


void AMyMJGameRoomViewerPawnBaseCpp::helperGetCameraData(int32 idxAttender, FMyCardGameCameraDataCpp& cameraData) const
{
    AMyMJGameRoomCpp* pRoom = AMyMJGameRoomRootActorCpp::helperGetRoomActor(this, false);
    if (pRoom == NULL) {
        return;
    }

    pRoom->getCameraData((MyMJGameRoleTypeCpp)idxAttender, cameraData);
};

void AMyMJGameRoomViewerPawnBaseCpp::changeInRoomViewRole(MyMJGameRoleTypeCpp roleType)
{
    int32 idxAttenderTarget = ((uint8)roleType) % 4;
    FMyCardGameCameraDataCpp cameraData;
    AMyMJGameRoomViewerPawnBaseCpp::helperGetCameraData(idxAttenderTarget, cameraData);

    FMyCardGameCameraDynamicDataCpp& dynDataEnd = cameraData.m_cDynamicData;

    FTransform transformEnd;
    UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(cameraData.m_cStaticData.m_cCenterPoint, dynDataEnd.m_cMyTransformOfZRotation, transformEnd);

    FMyCardGameCameraDynamicDataCpp dynDataStart;
    UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(cameraData.m_cStaticData.m_cCenterPoint, GetActorTransform(), dynDataStart.m_cMyTransformOfZRotation);
    dynDataStart.m_fFOV = getCameraComponent()->FieldOfView;

    //use the shortest rotate path
    float degreeDelta = FMath::FindDeltaAngleDegrees(dynDataStart.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane, dynDataEnd.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane);
    dynDataEnd.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane = dynDataStart.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane + degreeDelta;

    bool bAlreadyInRoom = dynDataStart.m_cMyTransformOfZRotation.m_cLocation.m_fRadiusOnXYPlane < (dynDataEnd.m_cMyTransformOfZRotation.m_cLocation.m_fRadiusOnXYPlane * JudgePawnAlreadyInRoomCoOfRadius);

    FMyMJGameRoomViewerPawnExtraDataCpp* pExtra = new FMyMJGameRoomViewerPawnExtraDataCpp();
    pExtra->m_iIdxAttenderTarget = idxAttenderTarget;

    pExtra->m_cCenterPoint = cameraData.m_cStaticData.m_cCenterPoint;
    pExtra->m_cDynamicDataStart = dynDataStart;
    pExtra->m_cDynamicDataEnd = dynDataEnd;

    //bAlreadyInRoom = false; //test
    if (bAlreadyInRoom) {
        pExtra->m_bSkipBasicTransformUpdate = true;
    }
    else {
    }

    pExtra->m_cUpdateDelegate.BindUObject(this, &AMyMJGameRoomViewerPawnBaseCpp::onTransformSeqUpdated);
    pExtra->m_cFinishDeleget.BindUObject(this, &AMyMJGameRoomViewerPawnBaseCpp::onTransformSeqFinished);


    UCurveVector* pCurve = cameraData.m_cStaticData.m_pMoveCurve;
    if (pCurve == NULL) {
        pCurve = UMyCommonUtilsLibrary::getCurveVectorDefaultLinear();
    }

    UMyTransformUpdateSequenceMovementComponent* pSeqComp = getTransformUpdateSequence(true);
    pSeqComp->clearSeq();

    FTransformUpdateSequencDataCpp data;
    data.helperSetDataBySrcAndDst(pSeqComp->getHelperTransformPrevRefConst(), transformEnd, cameraData.m_cStaticData.m_fMoveTime);
    data.m_pExtraDataOnHeap = pExtra;
    pSeqComp->addSeqToTail(data, pCurve);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setted: start %s, end %s."), *pExtra->m_cDynamicDataStart.ToString(), *pExtra->m_cDynamicDataEnd.ToString());
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

void AMyMJGameRoomViewerPawnBaseCpp::onTransformSeqUpdated(const struct FTransformUpdateSequencDataCpp& data, const FVector& vector)
{
    const FMyMJGameRoomViewerPawnExtraDataCpp* pExtra = StaticCast<FMyMJGameRoomViewerPawnExtraDataCpp*>(data.m_pExtraDataOnHeap);
    MY_VERIFY(pExtra);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setted: start %s, end %s."), *pExtra->m_cDynamicDataStart.ToString(), *pExtra->m_cDynamicDataEnd.ToString());

    FMyCardGameCameraDynamicDataCpp dynNow;
    FMyCardGameCameraDynamicDataCpp::interp(pExtra->m_cDynamicDataStart, pExtra->m_cDynamicDataEnd, vector.Y, dynNow);

    if (!FMath::IsNearlyEqual(getCameraComponent()->FieldOfView, dynNow.m_fFOV, FTransformUpdateSequencDataCpp_Delta_Min))
    {
        getCameraComponent()->SetFieldOfView(dynNow.m_fFOV);
    }

    if (pExtra->m_bSkipBasicTransformUpdate) {
        //use custom move
        FTransform transformNow;
        UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(pExtra->m_cCenterPoint, dynNow.m_cMyTransformOfZRotation, transformNow);

        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Updated: %s, %s, %s, %f. %s."), *dynNow.m_cMyTransformOfZRotation.ToString(), *pExtra->m_cDynamicDataStart.m_cMyTransformOfZRotation.ToString(), *pExtra->m_cDynamicDataEnd.m_cMyTransformOfZRotation.ToString(), vector.Y, *transformNow.ToString());

        if (!transformNow.Equals(GetActorTransform(), FTransformUpdateSequencDataCpp_Delta_Min)) {
            SetActorTransform(transformNow);
        }

    }

};

void AMyMJGameRoomViewerPawnBaseCpp::onTransformSeqFinished(const struct FTransformUpdateSequencDataCpp& data)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("onTransformSeqFinished."));

    const FMyMJGameRoomViewerPawnExtraDataCpp* pExtra = StaticCast<FMyMJGameRoomViewerPawnExtraDataCpp*>(data.m_pExtraDataOnHeap);
    MY_VERIFY(pExtra);

    if (!FMath::IsNearlyEqual(getCameraComponent()->FieldOfView, pExtra->m_cDynamicDataEnd.m_fFOV, FTransformUpdateSequencDataCpp_Delta_Min))
    {
        getCameraComponent()->SetFieldOfView(pExtra->m_cDynamicDataEnd.m_fFOV);
    }

    if (pExtra->m_bSkipBasicTransformUpdate) {
        //let's do what we should, if skipped before
        SetActorTransform(data.m_cEnd);
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

    UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = pUIManager->getInRoomUIMain(true, true);

    if (pUI->GetClass()->ImplementsInterface(UMyMJGameInRoomUIMainInterface::StaticClass()))
    {
        IMyMJGameInRoomUIMainInterface::Execute_changeViewPosition(pUI, pExtra->m_iIdxAttenderTarget);
    }

    /*
    if (pUI->GetClass()->ImplementsInterface(UMyPawnUIInterface::StaticClass()))
    {
        pUI->Execute_changeViewPosition(this, pExtra->m_iIdxAttenderTarget);
    }
    */
    else {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("pUI not implemented interface UMyPawnUIInterface."));
    }
}

