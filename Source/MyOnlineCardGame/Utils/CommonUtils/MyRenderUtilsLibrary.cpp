// Fill out your copyright notice in the Description page of Project Settings.

#include "MyRenderUtilsLibrary.h"

#include "MyCommonUtils.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"
#include "Engine/SceneCapture2D.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Runtime/UMG/Public/Blueprint/WidgetLayoutLibrary.h"

#include "Runtime/UMG/Public/Components/CanvasPanelSlot.h"
#include "Widgets/Input/SButton.h"
#include "Components/ButtonSlot.h"

//#include "Classes/PaperSprite.h"

#if WITH_EDITOR
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "PackageTools.h"
#endif



#define MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT (64)

FMyWithCurveUpdaterBasicCpp::FMyWithCurveUpdaterBasicCpp()
{
    m_cStepDataItemsCycleBuffer.reinit(TEXT("step data items cycle buffer"), NULL, NULL, MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT);

    m_cTimeLineVectorDelegate.BindRaw(this, &FMyWithCurveUpdaterBasicCpp::onTimeLineUpdated); //binding to parent, should be safe
    m_cTimeLineFinishEventDelegate.BindRaw(this, &FMyWithCurveUpdaterBasicCpp::onTimeLineFinished);

    m_cTimeLine.Stop();
    reset();

    m_iType = MyTypeUnknown;
};

FMyWithCurveUpdaterBasicCpp::~FMyWithCurveUpdaterBasicCpp()
{

};

void FMyWithCurveUpdaterBasicCpp::tick(float deltaTime)
{
    if (m_cTimeLine.IsPlaying()) {
        //Todo: handle the case that tick() and addSeq() happen in one loop, which cause 1 tick time defloat
        m_fDebugTimePassedForOneStep += deltaTime;
        m_cTimeLine.TickTimeline(deltaTime);
    }
    else {
        tryStartNextStep(TEXT("in tick() timeline stopped."));
    }
};

int32 FMyWithCurveUpdaterBasicCpp::addStepToTail(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    if (!data.checkValid()) {
        return -1;
    }

    //Safe tp const cast since result is still a const
    //Todo:: better way to use constructor, remove const cast
    const FMyWithCurveUpdateStepDataItemCpp tempD(const_cast<FMyWithCurveUpdateStepDataBasicCpp *>(&data));
    int32 ret0 = m_cStepDataItemsCycleBuffer.addToTail(&tempD, NULL);

    if (ret0 >= 0) {
        if (!m_cTimeLine.IsPlaying()) {
            tryStartNextStep(TEXT("addSeqToTail trigger"));
        }

        //debug
        const FMyWithCurveUpdateStepDataItemCpp* pLast = m_cStepDataItemsCycleBuffer.peekLast();
        MY_VERIFY(pLast->getDataConst());
        MY_VERIFY(pLast->getDataConst() != &data);

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("addSeqToTail fail, ret %d."), ret0);
    }

    return ret0;
};

int32 FMyWithCurveUpdaterBasicCpp::removeStepsFromHead(int32 iCount)
{
    int32 ret0;
    ret0 = m_cStepDataItemsCycleBuffer.removeFromHead(iCount);

    if (getStepsCount() <= 0) {
        m_cTimeLine.Stop();
    }
    return ret0;
}

int32 FMyWithCurveUpdaterBasicCpp::peekStepAt(int32 idxFromHead, const FMyWithCurveUpdateStepDataBasicCpp*& poutData) const
{
    int32 ret0 = 0;
    const FMyWithCurveUpdateStepDataItemCpp* pD = m_cStepDataItemsCycleBuffer.peekAt(idxFromHead, &ret0);
    if (pD) {
        poutData = pD->getDataConst();
    }
    else {
        poutData = NULL;
    }
    return ret0;
};

int32 FMyWithCurveUpdaterBasicCpp::peekStepLast(const FMyWithCurveUpdateStepDataBasicCpp*& poutData) const
{
    int32 l = getStepsCount();
    if (l > 0) {
        return peekStepAt(l - 1, poutData);
    }
    else {
        return -1;
    }
}

int32 FMyWithCurveUpdaterBasicCpp::getStepsCount() const
{
    int32 ret0;
    ret0 = m_cStepDataItemsCycleBuffer.getCount();

    return ret0;
}

void FMyWithCurveUpdaterBasicCpp::clearSteps()
{
    m_cStepDataItemsCycleBuffer.clearInGame();

    MY_VERIFY(getStepsCount() == 0);
    m_cTimeLine.Stop();

    m_fDebugTimePassedForOneStep = 0;
}

//Todo:
bool FMyWithCurveUpdaterBasicCpp::tryStartNextStep(FString sDebugReason)
{
    if (m_cTimeLine.IsPlaying()) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("still playing, not start next seq now."));
        return false;
    }

    m_fDebugTimePassedForOneStep = 0;
    const FMyWithCurveUpdateStepDataBasicCpp* pData = NULL;

    //skip any step incorrectly set
    while (1)
    {
        if (peekStepAt(0, pData) < 0) {
            break;
        }

        if (pData->m_fTime < MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            //directly finish
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("step have too short dur seq: %f sec."), pData->m_fTime);
            finishOneStep(*pData);
            removeStepsFromHead(1);
        }
        else {
            break;
        }
    }

    //start next valid one
    if (peekStepAt(0, pData) >= 0) {

        //we have data
        MY_VERIFY(pData);
        MY_VERIFY(pData->m_fTime > 0);
        MY_VERIFY(pData->m_pCurve);

        //the fuck is that: FTimeLine have NOT clear function for static bind type, so create new one
        FTimeline newTimeLine;
        m_cTimeLine = newTimeLine;

        m_cTimeLine.AddInterpVector(pData->m_pCurve, m_cTimeLineVectorDelegate);
        m_cTimeLine.SetTimelineFinishedFunc(m_cTimeLineFinishEventDelegate);

        float MinVal, MaxVal;
        pData->m_pCurve->GetTimeRange(MinVal, MaxVal);

        m_cTimeLine.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
        m_cTimeLine.SetTimelineLength(MaxVal);

        m_cTimeLine.SetPlayRate(MaxVal / pData->m_fTime);

        m_cTimeLine.Play();

        m_cActivateTickDelegate.ExecuteIfBound(true, sDebugReason + TEXT(", activate since next step found."));

        return true;
    }
    else {
        //stop

        m_cActivateTickDelegate.ExecuteIfBound(false, sDebugReason + TEXT(", deactivate since no more step found."));

        return false;
    }
}


void FMyWithCurveUpdaterBasicCpp::onTimeLineUpdated(FVector vector)
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("onTimeLineUpdated."));

    const FMyWithCurveUpdateStepDataBasicCpp* pData = NULL;
    if (peekStepAt(0, pData) < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get data in the middle of timelineUpdate"));
        return;
    }

    MY_VERIFY(pData);

    if (!pData->m_bSkipCommonUpdateDelegate) {
        m_cCommonUpdateDelegate.ExecuteIfBound(*pData, vector);
    }

    pData->m_cStepUpdateDelegate.ExecuteIfBound(*pData, vector);

}

void FMyWithCurveUpdaterBasicCpp::onTimeLineFinished()
{
    const FMyWithCurveUpdateStepDataBasicCpp* pData = NULL;
    if (peekStepAt(0, pData) < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get data in timelinefinish"));
        return;
    }
    MY_VERIFY(pData);

    finishOneStep(*pData);

    if (!UKismetMathLibrary::NearlyEqual_FloatFloat(pData->m_fTime, m_fDebugTimePassedForOneStep, 0.1)) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("time line finished but time not quite equal: supposed %f, used %f."), pData->m_fTime, m_fDebugTimePassedForOneStep);
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("onTimeLineFinished, time used %f, time range %f, %f, value range %f, %f."), s1 - m_dDebugTimeLineStartRealTime, minTime, maxTime, minValue, maxValue);

    removeStepsFromHead(1);
    tryStartNextStep(TEXT("previous timeline finish"));
}

void FMyWithCurveUpdaterBasicCpp::finishOneStep(const FMyWithCurveUpdateStepDataBasicCpp& stepData)
{
    const FMyWithCurveUpdateStepDataBasicCpp* pData = &stepData;

    if (!pData->m_bSkipCommonFinishDelegate) {
        m_cCommonFinishDelegete.ExecuteIfBound(*pData);
    }

    pData->m_cStepFinishDelegete.ExecuteIfBound(*pData);

    m_cTimeLine.Stop();
}


void FMyWithCurveUpdateStepDataTransformWorld3DCpp::helperSetDataBySrcAndDst(float fTime, UCurveVector* pCurve, const FTransform& cStart, const FTransform& cEnd, FIntVector extraRotateCycle)
{
    m_cStart = cStart;
    m_cEnd = cEnd;
    m_fTime = fTime;
    m_pCurve = pCurve;

    m_cStart.NormalizeRotation();
    m_cEnd.NormalizeRotation();

    m_cLocalRotatorExtra.Roll = extraRotateCycle.X * 360;
    m_cLocalRotatorExtra.Pitch = extraRotateCycle.Y * 360;
    m_cLocalRotatorExtra.Yaw = extraRotateCycle.Z * 360;

    FQuat B = m_cStart.GetRotation();
    FQuat A = m_cEnd.GetRotation();
    FQuat NegA = A.Inverse();
    NegA.Normalize();
    FQuat X = B * NegA;

    FRotator relativeRota;
    relativeRota = X.Rotator();
    relativeRota.Normalize();

    //X.EnforceShortestArcWith(FRotator(0, 0, 0).Quaternion());

    //X = FRotator(0, 0, 0).Quaternion() * X;
    //UKismetMathLibrary::ComposeRotators();

    //FVector v(1, 1, 1), vz(0, 0, 0);
    //relativeRota = UKismetMathLibrary::FindLookAtRotation(vz, X.Rotator().RotateVector(v)) - UKismetMathLibrary::FindLookAtRotation(vz, v);
    //relativeRota.Clamp();

    //FRotator winding;
    //X.Rotator().GetWindingAndRemainder(winding, relativeRota);

    if (m_cEnd.GetLocation().Equals(m_cStart.GetLocation(), FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min)) {
        m_bLocationEnabledCache = false;
    }
    else {
        m_bLocationEnabledCache = true;
    }

    //since we allow roll at origin 360d, we can't use default isNealyZero() which treat that case zero
    if (relativeRota.Euler().IsNearlyZero(FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min)) {
        m_bRotatorBasicEnabledCache = false;
    }
    else {
        m_bRotatorBasicEnabledCache = true;
    }

    if (m_cLocalRotatorExtra.Euler().IsNearlyZero(FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min)) {
        m_bRotatorExtraEnabledCache = false;
    }
    else {
        m_bRotatorExtraEnabledCache = true;
    }

    if (m_cEnd.GetScale3D().Equals(m_cStart.GetScale3D(), FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min)) {
        m_bScaleEnabledCache = false;
    }
    else {
        m_bScaleEnabledCache = true;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%d, %d, %d, %d. r:  %s -> %s, relativeRota %s."), m_bLocationEnabledCache, m_bRotatorBasicEnabledCache, m_bRotatorExtraEnabledCache, m_bScaleEnabledCache,
    //         *cStart.GetRotation().Rotator().ToString(), *cEnd.GetRotation().Rotator().ToString(), *relativeRota.ToString());

};



UMyWithCurveUpdaterTransformWorld3DComponent::UMyWithCurveUpdaterTransformWorld3DComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    PrimaryComponentTick.TickInterval = 0;
    bUpdateOnlyIfRendered = false;

    bWantsInitializeComponent = true;
    bAutoActivate = false;
    SetTickableWhenPaused(false);

    m_bShowWhenActivated = false;
    m_bHideWhenInactivated = false;

    m_cUpdater.m_cCommonUpdateDelegate.BindUObject(this, &UMyWithCurveUpdaterTransformWorld3DComponent::updaterOnCommonUpdate);
    m_cUpdater.m_cCommonFinishDelegete.BindUObject(this, &UMyWithCurveUpdaterTransformWorld3DComponent::updaterOnCommonFinish);
    m_cUpdater.m_cActivateTickDelegate.BindUObject(this, &UMyWithCurveUpdaterTransformWorld3DComponent::updaterActivateTick);
}

void UMyWithCurveUpdaterTransformWorld3DComponent::BeginPlay()
{
    Super::BeginPlay();
    m_bHelperTransformUpdated = false;
}

void UMyWithCurveUpdaterTransformWorld3DComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("TickComponent."));

    // skip if don't want component updated when not rendered or updated component can't move
    if (!UpdatedComponent || !bIsActive || ShouldSkipUpdate(DeltaTime))
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("UpdatedComponent %d, bIsActive %d, ShouldSkipUpdate(DeltaTime) %d."), UpdatedComponent, bIsActive, ShouldSkipUpdate(DeltaTime));
        return;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("TickComponent 1."));

    AActor* ActorOwner = UpdatedComponent->GetOwner();
    if (!ActorOwner || !ActorOwner->CheckStillInWorld())
    {
        return;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("TickComponent 2."));

    if (UpdatedComponent->IsSimulatingPhysics())
    {
        return;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("TickComponent 3."));

    DeltaTime /= ActorOwner->GetActorTimeDilation();

    m_cUpdater.tick(DeltaTime);

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("TickComponent out."));
}

void UMyWithCurveUpdaterTransformWorld3DComponent::updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataTransformWorld3DCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformWorld3DCpp*>(&data);
    MY_VERIFY(pData);

    MY_VERIFY(IsValid(UpdatedComponent));

    bool bHelperTransformUpdated = false;
    while (pData->m_bLocationEnabledCache || pData->m_bRotatorBasicEnabledCache || pData->m_bRotatorExtraEnabledCache) {
        FVector MoveDelta = FVector::ZeroVector;
        FQuat NewQuat = UpdatedComponent->GetComponentRotation().Quaternion();
        FVector NewLocation = UpdatedComponent->GetComponentLocation();
        if (pData->m_bLocationEnabledCache) {
            NewLocation = UKismetMathLibrary::VLerp(pData->m_cStart.GetLocation(), pData->m_cEnd.GetLocation(), vector.X);
            UpdatedComponent->SetWorldLocation(NewLocation);

            bHelperTransformUpdated = true;
            //FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
            //if (NewLocation != CurrentLocation)
            //{
            //MoveDelta = NewLocation - CurrentLocation;
            //}
        }

        if (pData->m_bRotatorBasicEnabledCache) {

            FRotator r = UKismetMathLibrary::RLerp(pData->m_cStart.GetRotation().Rotator(), pData->m_cEnd.GetRotation().Rotator(), vector.Y, true);
            if (r.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *r.ToString());
                break;
            }

            NewQuat = r.Quaternion();
            if (NewQuat.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *NewQuat.ToString());
                break;
            }
            //FQuat quatDelta = pData->m_cRotatorRelativeToStartDelta.Quaternion() * vector.Y;
            //NewQuat = quatDelta * pData->m_cStart.GetRotation();
            //quatDelta.
            /*
            FRotator rotDelta = pData->m_cRotatorRelativeToStartDelta * vector.Y;
            if (rotDelta.ContainsNaN())
            {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value 0, %s."), *rotDelta.ToString());
            return;
            }
            FQuat quatDelta = rotDelta.Quaternion();
            if (quatDelta.ContainsNaN())
            {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value 1, %s."), *quatDelta.ToString());
            return;
            }
            NewQuat = quatDelta * pData->m_cStart.GetRotation();
            if (NewQuat.ContainsNaN())
            {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value 2, %s."), *NewQuat.ToString());
            return;
            }
            */
        }

        if (pData->m_bRotatorExtraEnabledCache) {
            FRotator r = UKismetMathLibrary::RLerp(FRotator::ZeroRotator, pData->m_cLocalRotatorExtra, vector.Y, false);
            if (r.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *r.ToString());
                break;
            }

            FQuat q = r.Quaternion();
            if (q.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *q.ToString());
                break;
            }

            NewQuat = q * NewQuat;
            if (NewQuat.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *NewQuat.ToString());
                break;
            }
        }

        UpdatedComponent->SetWorldRotation(NewQuat);

        bHelperTransformUpdated = true;
        //MoveUpdatedComponent(MoveDelta, NewQuat, false);

        break;
    }

    if (pData->m_bScaleEnabledCache) {
        FVector NewScale = UKismetMathLibrary::VLerp(pData->m_cStart.GetScale3D(), pData->m_cEnd.GetScale3D(), vector.Z);
        UpdatedComponent->SetWorldScale3D(NewScale);

        bHelperTransformUpdated = true;
    }

    if (bHelperTransformUpdated) {
        m_bHelperTransformUpdated = bHelperTransformUpdated;
        m_cHelperTransformUpdated = UpdatedComponent->GetComponentTransform();
    }

}

void UMyWithCurveUpdaterTransformWorld3DComponent::updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    const FMyWithCurveUpdateStepDataTransformWorld3DCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformWorld3DCpp*>(&data);
    MY_VERIFY(pData);

    MY_VERIFY(IsValid(UpdatedComponent));

    FTransform cT = UpdatedComponent->GetComponentTransform();
    if (!cT.Equals(pData->m_cEnd, 1.0f)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time line finished but not equal: now %s. target %s."), *cT.ToString(), *pData->m_cEnd.ToString());
    }

    UpdatedComponent->SetWorldTransform(pData->m_cEnd);
}

void UMyWithCurveUpdaterTransformWorld3DComponent::updaterActivateTick(bool bNew, FString reason)
{
    if (IsActive() != bNew) {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("activate component change %d -> %d, reason %s, last tick time ms %u."), !bNew, bNew, *reason, m_uDebugLastTickWorldTime_ms);
        if (bNew) {
            Activate();
            if (m_bShowWhenActivated) {
                AActor* actor = GetOwner();
                if (IsValid(actor)) {
                    actor->SetActorHiddenInGame(false);
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("UMyTransformUpdateSequenceMovementComponent's owner actor is NULL!"));
                }
            }


        }
        else {
            Deactivate();
            if (m_bHideWhenInactivated) {
                AActor* actor = GetOwner();
                if (IsValid(actor)) {
                    actor->SetActorHiddenInGame(true);
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("UMyTransformUpdateSequenceMovementComponent's owner actor is NULL!"));
                }
            }
        }

    }
}



AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp() : Super()
{
    bNetLoadOnClient = true;

    m_bFakeUpdateSettings = false;

    createComponentsForCDO();
}

AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::~AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp()
{

}


MyErrorCodeCommonPartCpp AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::getModelInfoForUpdater(FMyModelInfoWorld3DCpp& modelInfo)
{
    //ignore the root scene/actor's scale, but calc from the box

    //FVector actorScale3D = GetActorScale3D();
    //m_pMainBox->GetScaledBoxExtent()

    modelInfo.reset(MyModelInfoType::BoxWorld3D);
    modelInfo.getBox3DRef().m_cBoxExtend = m_pMainBox->GetUnscaledBoxExtent() *  m_pMainBox->GetComponentScale();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("name %s, box scale %s."), *GetName(), *m_pMainBox->GetComponentScale().ToString());
    modelInfo.getBox3DRef().m_cCenterPointRelativeLocation = m_pMainBox->RelativeLocation;// * actorScale3D;

 
    if (modelInfo.getBox3DRef().m_cBoxExtend.Size() < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class %s: model size is too small: %s, fixing it now."), *this->GetClass()->GetName(), *modelInfo.getBox3DRef().m_cBoxExtend.ToString());
        modelInfo.getBox3DRef().m_cBoxExtend = FVector::OneVector;
    }

    return MyErrorCodeCommonPartCpp::NoError;
}

void AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::createComponentsForCDO()
{

    USceneComponent* pRootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    MY_VERIFY(IsValid(pRootSceneComponent));
    RootComponent = pRootSceneComponent;
    m_pRootScene = pRootSceneComponent;


    UBoxComponent* pBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("MainBox"));
    MY_VERIFY(IsValid(pBoxComponent));
    pBoxComponent->SetupAttachment(m_pRootScene);
    pBoxComponent->SetCollisionProfileName(TEXT("CollistionProfileBox"));
    pBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); //by default disable collision
    m_pMainBox = pBoxComponent;


    UStaticMeshComponent* pStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStaticMesh"));
    MY_VERIFY(IsValid(pStaticMeshComponent));
    pStaticMeshComponent->SetupAttachment(m_pMainBox);
    pStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    m_pMainStaticMesh = pStaticMeshComponent;

    m_pMyTransformUpdaterComponent = CreateDefaultSubobject<UMyWithCurveUpdaterTransformWorld3DComponent>(TEXT("transform updater component"));

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_pMainBox created as 0x%p, this 0x%p."), m_pMainBox, this);

}


#if WITH_EDITOR

void AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp, m_bFakeUpdateSettings))
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_bFakeUpdateSettings clicked."));
        updateSettings();
    }
    else {
    }

    Super::PostEditChangeProperty(e);
}

#endif


MyErrorCodeCommonPartCpp AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::updateSettings()
{
    if (!IsValid(m_pMainBox)) {
        UClass* uc = this->GetClass();
        UObject* CDO = uc->GetDefaultObject();

        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMainBox invalid 0x%p, this 0x%p, cdo 0x%p."), m_pMainBox, this, CDO);
        MY_VERIFY(false);
    }

    //MY_VERIFY(IsValid(m_pMainBox));
    MY_VERIFY(IsValid(m_pMainStaticMesh));

    UStaticMesh* pMeshNow = m_pMainStaticMesh->GetStaticMesh();

    FVector boxSizeFix = FVector::ZeroVector, meshOrigin = FVector::ZeroVector;

    if (IsValid(pMeshNow)) {

        FBox meshBox = pMeshNow->GetBoundingBox();
        FVector meshBoxSize = meshBox.Max - meshBox.Min;

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("mesh bounding box: %s."), *meshBox.ToString());

        boxSizeFix.X = UKismetMathLibrary::FCeil(meshBoxSize.X) / 2;
        boxSizeFix.Y = UKismetMathLibrary::FCeil(meshBoxSize.Y) / 2;
        boxSizeFix.Z = UKismetMathLibrary::FCeil(meshBoxSize.Z) / 2;
        boxSizeFix = boxSizeFix * m_pMainStaticMesh->RelativeScale3D;

        meshOrigin.X = (meshBox.Min.X + meshBox.Max.X) / 2;
        meshOrigin.Y = (meshBox.Min.Y + meshBox.Max.Y) / 2;
        meshOrigin.Z = (meshBox.Min.Z + meshBox.Max.Z) / 2;
        meshOrigin = meshOrigin * m_pMainStaticMesh->RelativeScale3D;
    }
    else {


    }


    m_pMainBox->SetRelativeLocation(FVector::ZeroVector);
    m_pMainBox->SetBoxExtent(boxSizeFix);

    m_pMainStaticMesh->SetRelativeLocation(-meshOrigin);

    return MyErrorCodeCommonPartCpp::NoError;
}


void AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::helperTestAnimationStep(float time, FString debugStr, const TArray<AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp*>& actors)
{
    FMyWithCurveUpdateStepMetaTransformWorld3DCpp meta;
    FMyWithCurveUpdateStepSettingsTransformWorld3DCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = time;
    meta.m_cModelInfo.reset(MyModelInfoType::BoxWorld3D);
    meta.m_cModelInfo.getBox3DRef().m_cBoxExtend = FVector(20, 30, 60);

    stepData.m_fTimePercent = 1;
    stepData.m_eLocationUpdateType = MyWithCurveUpdateStepSettingsLocationType::OffsetFromPrevLocation;
    stepData.m_cLocationOffsetPercent = FVector(0, 0, 1);

    TArray<FMyWithCurveUpdaterTransformWorld3DCpp *> updaterSortedGroup;

    for (int32 i = 0; i < actors.Num(); i++)
    {
        FMyWithCurveUpdaterTransformWorld3DCpp* pUpdater = &actors[i]->getMyWithCurveUpdaterTransformRef();
        FTransform targetT;
        targetT.SetLocation(FVector(0, 0, 100));
        pUpdater->setHelperTransformOrigin(actors[i]->GetActorTransform());
        pUpdater->setHelperTransformFinal(targetT);
        updaterSortedGroup.Emplace(pUpdater);
    }

    UMyRenderUtilsLibrary::helperUpdatersSetupStep(meta, stepData, updaterSortedGroup);
}

/*
void AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::OnConstruction(const FTransform& Transform)
{
Super::OnConstruction(Transform);

UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnConstruction m_pMainBox 0x%p, this 0x%p."), m_pMainBox, this);
}
*/

void UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (m_bUpdaterNeedTick) {
        m_cUpdater.tick(InDeltaTime);
    }
}

void UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataTransformWidget2DCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformWidget2DCpp*>(&data);
    MY_VERIFY(pData);

    FWidgetTransform wtNow;
    wtNow.Translation = FMath::Vector2DInterpTo(pData->m_cTransformWidget2DStart.Translation, pData->m_cTransformWidget2DEnd.Translation, vector.X, 1);

    wtNow.Angle = FMath::FInterpTo(pData->m_cTransformWidget2DStart.Angle, pData->m_cTransformWidget2DEnd.Angle, vector.Y, 1);
    wtNow.Shear = FMath::Vector2DInterpTo(pData->m_cTransformWidget2DStart.Shear, pData->m_cTransformWidget2DEnd.Shear, vector.Y, 1);

    wtNow.Scale = FMath::Vector2DInterpTo(pData->m_cTransformWidget2DStart.Scale, pData->m_cTransformWidget2DEnd.Scale, vector.Z, 1);

    SetRenderTransform(wtNow);
}

void UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    const FMyWithCurveUpdateStepDataTransformWidget2DCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformWidget2DCpp*>(&data);
    MY_VERIFY(pData);

    SetRenderTransform(pData->m_cTransformWidget2DEnd);
}

void UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::updaterActivateTick(bool activate, FString debugString)
{
    m_bUpdaterNeedTick = activate;

    if (activate) {
        if (m_bShowWhenActivated) {
            this->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else {
        if (m_bHideWhenInactivated) {
            this->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

MyErrorCodeCommonPartCpp UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::getDataByCacheRefConst_MyModelInfoWidget2D(const FMyModelInfoWidget2DCpp*& pModelInfo, bool verifyValid)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
    if (!m_cMyCachedData_MyModelInfoWidget2D.m_bValid) {
        ret = refillCachedData_MyModelInfoWidget2D();
    }
    pModelInfo = &m_cMyCachedData_MyModelInfoWidget2D.m_cModelInfo;

    if (pModelInfo->getType() != MyModelInfoType::BoxWidget2D) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: model type is supposed to be BoxWidget2D, but it is  %d."), *GetName(), (uint8)pModelInfo->getType());
        MyErrorCodeCommonPartJoin(ret, MyErrorCodeCommonPartCpp::TypeUnexpected);
    }

    if (UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_Bool(ret)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error in getModelInfoFromCacheRefConst(): %s."), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
        if (verifyValid) {
            MY_VERIFY(false);
        }
    }

    return ret;
};

MyErrorCodeCommonPartCpp UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::updateSlotSettingsToComply_MyModelInfoWidget2D()
{
    invalidCachedData_MyModelInfoWidget2D();
    return UMyRenderUtilsLibrary::updateUserWidgetInCanvasPanelWithPivot_Widget2D(this, m_cPivotExpected);
};

void UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::invalidCachedData_MyModelInfoWidget2D()
{
    m_cMyCachedData_MyModelInfoWidget2D.reset();
};

MyErrorCodeCommonPartCpp UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp::refillCachedData_MyModelInfoWidget2D()
{
    m_cMyCachedData_MyModelInfoWidget2D.reset();

    MyErrorCodeCommonPartCpp ret = UMyRenderUtilsLibrary::evaluateUserWidgetInCanvasPanelForModelInfo_Widget2D(this, m_cMyCachedData_MyModelInfoWidget2D.m_cModelInfo);

    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        m_cMyCachedData_MyModelInfoWidget2D.reset();
    }
    else {
        m_cMyCachedData_MyModelInfoWidget2D.m_bValid = true;
    }

    return ret;
}



#if WITH_EDITOR

void UMyFlipImage::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyFlipImage, m_pSettingsAsset))
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 1, this %p."), this);

        UTexture2D* pTexture = NULL;
        if (m_pSettingsAsset) {
            if (m_pSettingsAsset->m_cSettings.m_aFlipImageElems.Num() > 0) {
                pTexture = m_pSettingsAsset->m_cSettings.m_aFlipImageElems[0].m_pTexture;
                if (pTexture == NULL) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_aFlipImageElems[0].m_pTexture is NULL."));
                }
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT(".m_aFlipImageElems.Num() is 0."));
            }
        }
        else {
        }

        this->SetBrushFromTexture(pTexture);
    }

    Super::PostEditChangeProperty(e);
}

#endif

void UMyFlipImage::startImageFlip(float loopTime, int32 loopNum, bool matchSize)
{
    if (m_pSettingsAsset == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("settings not specified!"));
        return;
    }

    m_cSettingUsing = m_pSettingsAsset->m_cSettings;
    m_iIdxOfImageShowing = -1;
    m_iFrameOfImageShowing = 0;
    m_iLoopShowing = 0;

    int32 totalFrameNum = m_cSettingUsing.getTotalFrameNum();
    if (totalFrameNum <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough of frames specified: %d."), totalFrameNum);
        return;
    }

    //the time interval model is same as UE4's 3d flipbook
    if (FMath::IsNearlyEqual(loopTime, MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)) {
    
    }
    else {
        m_cSettingUsing.m_fFlipTime = loopTime / totalFrameNum;
    }

    if (m_cSettingUsing.m_fFlipTime < MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("flip time is too small: %f!"), m_cSettingUsing.m_fFlipTime);
    }

    m_cSettingUsing.m_iLoopNum = loopNum;
    m_cSettingUsing.m_bMatchSize = matchSize;

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &UMyFlipImage::loop, m_cSettingUsing.m_fFlipTime, true);

        tryNextFrame(0, 0);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
    }

    //world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
    //world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJGameCoreDataSourceCpp::loop, ((float)MY_MJ_GAME_CORE_FULL_MAIN_THREAD_LOOP_TIME_MS) / (float)1000, true);
}

void UMyFlipImage::stopImageFlip()
{
    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
    }
}

bool UMyFlipImage::isImageFlipping() const
{
    UWorld *world = GetWorld();
    if (IsValid(world)) {
        return world->GetTimerManager().IsTimerActive(m_cLoopTimerHandle);

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
        return false;
    }
};

int32 UMyFlipImage::getImageSize(FVector2D& size) const
{
    if (m_pSettingsAsset && m_pSettingsAsset->m_cSettings.m_aFlipImageElems.Num() > 0) {
        UTexture2D* pT = m_pSettingsAsset->m_cSettings.m_aFlipImageElems[0].m_pTexture;
        if (pT) {
            size.X = pT->GetSizeX();
            size.Y = pT->GetSizeY();
            return 0;
        }
    }

    size = FVector2D::ZeroVector;
    return -1;
}

void UMyFlipImage::BeginDestroy()
{
    Super::BeginDestroy();

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("BeginDestroy: debug, timer clear OK."));
    }
    else {
        if (this != GetClass()->GetDefaultObject()) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("BeginDestroy: world is invalid and object is not CDO! Check settings!"));
        }
    }
}

void UMyFlipImage::loop()
{
    //just go to next frame
    tryNextFrame(m_iIdxOfImageShowing, m_iFrameOfImageShowing + 1);
}

void UMyFlipImage::tryNextFrame(int32 idxOfImage, int32 frameOfImage)
{
    int32 elemL = m_cSettingUsing.m_aFlipImageElems.Num();
    MY_VERIFY(elemL > 0);

    const FMyFlipImageElemCpp* pElem = NULL;
    while (1)
    {
        if (idxOfImage >= 0 && idxOfImage < elemL) {
        }
        else {
            //out of range
            m_iLoopShowing++;

            bool isLastLoop = m_cSettingUsing.m_iLoopNum > 0 && m_iLoopShowing >= m_cSettingUsing.m_iLoopNum;
            OnFlipEnd.ExecuteIfBound(m_iLoopShowing, isLastLoop);

            if (isLastLoop) {
                stopImageFlip();
                if (m_bHideWhenFlipAllOver) {
                    this->SetVisibility(ESlateVisibility::Hidden);
                }
                return;
            }

            idxOfImage = 0;
            frameOfImage = 0;
        }

        pElem = &m_cSettingUsing.m_aFlipImageElems[idxOfImage];
        MY_VERIFY(pElem->m_iFrameOccupy > 0 || m_cSettingUsing.m_iLoopNum > 0); //avoid dead loop
        if (frameOfImage >= pElem->m_iFrameOccupy) {
            idxOfImage++;
            frameOfImage = 0;
            continue;
        }

        break;
    }

    MY_VERIFY(pElem);

    if (frameOfImage == 0) {
        //First frame
        if (pElem->m_pTexture) {
            SetBrushFromTexture(pElem->m_pTexture, m_cSettingUsing.m_bMatchSize);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("found a flip image elem with NULL texture, idx %d."), idxOfImage);
        }

        if (m_bShowWhenFlipStart) {
            this->SetVisibility(ESlateVisibility::Visible);
        }
    }

    m_iIdxOfImageShowing = idxOfImage;
    m_iFrameOfImageShowing = frameOfImage;

    //pElem->m_pTexture->GetSizeX();
}


UMyButton::UMyButton(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    m_bEnableTransformWidget2DPressed = true;
    m_cTransformWidget2DPressed.Scale.X = m_cTransformWidget2DPressed.Scale.Y = 0.8;
}

void UMyButton::tryUpdateStylesInUnifiedWay(const FSlateBrush& normalStyle, bool forceUpdate)
{
    if (!WidgetStyle.Normal.HasUObject() || forceUpdate) {
        WidgetStyle.Normal = normalStyle;
    }

    if (!WidgetStyle.Pressed.HasUObject() || forceUpdate) {
        WidgetStyle.Pressed = normalStyle;
    }
    if (!WidgetStyle.Hovered.HasUObject() || forceUpdate) {
        WidgetStyle.Hovered = normalStyle;
    }
    if (!WidgetStyle.Disabled.HasUObject() || forceUpdate) {
        WidgetStyle.Disabled = normalStyle;
    }
}

#if WITH_EDITOR
void UMyButton::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);

    FName PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, RenderTransform))
    {
        m_cTransformWidget2DNormal = RenderTransform;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, m_cTransformWidget2DNormal))
    {
        RenderTransform = m_cTransformWidget2DNormal;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, WidgetStyle))
    {
        if (WidgetStyle.Normal.HasUObject()) {

            FSlateBrush normalStyle = WidgetStyle.Normal;
            tryUpdateStylesInUnifiedWay(normalStyle, false);
        }
        RenderTransform = m_cTransformWidget2DNormal;
    }


    Super::PostEditChangeProperty(e);
}
#endif

//Warn:: check the code is accomplish with parent's function when UE4 upgrade
TSharedRef<SWidget> UMyButton::RebuildWidget()
{
    MyButton = SNew(SButton)
        .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClickedMy))
        .OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressedMy))
        .OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleasedMy))
        .OnHovered_UObject(this, &ThisClass::SlateHandleHoveredMy)
        .OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhoveredMy)
        .ButtonStyle(&WidgetStyle)
        .ClickMethod(ClickMethod)
        .TouchMethod(TouchMethod)
        .IsFocusable(IsFocusable)
        ;

    if (GetChildrenCount() > 0)
    {
        Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
    }

    return MyButton.ToSharedRef();
}

void UMyButton::SlateHandlePressedMy()
{
    if (m_bEnableTransformWidget2DPressed) {
        SetRenderTransform(m_cTransformWidget2DPressed);
    }

    SlateHandlePressed();

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("pressed: %s"), *genStyleString());
}

void UMyButton::SlateHandleReleasedMy()
{
    if (m_bEnableTransformWidget2DPressed) {
        SetRenderTransform(m_cTransformWidget2DNormal);
    }

    SlateHandleReleased();

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("released: %s"), *genStyleString());
}

FString UMyButton::genStyleString() const
{
    return FString::Printf(TEXT("normal: %s, pressed: %s"),
                           *UMyRenderUtilsLibrary::Conv_SlateBrush_String(this->WidgetStyle.Normal),
                           *UMyRenderUtilsLibrary::Conv_SlateBrush_String(this->WidgetStyle.Pressed));
  
}


/*
void UMyButton::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    OnPressed.Clear();
    OnPressed.AddDynamic(this, &UMyButton::onPressedInner);

    OnReleased.Clear();
    OnReleased.AddDynamic(this, &UMyButton::onReleasedInner);
}

void UMyButton::onPressedInner()
{
    SetRenderTransform(m_cTransformWidget2DPressed);
    OnPressedOverride.Broadcast();
}

void UMyButton::onReleasedInner()
{
    SetRenderTransform(m_cTransformWidget2DNormal);
    OnReleased.Broadcast();
}
*/


bool AMyTextureGenSuitBaseCpp::checkSettings() const
{
    if (!IsValid(m_pSceneCapture2D))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pSceneCapture2D invalid: %p."), m_pSceneCapture2D);
        return false;
    }

    if (m_aTargetActors.Num() <= 0)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_aTargetActors not specified."));
        return false;
    }

    if (!IsValid(m_pRenderTarget))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRenderTarget invalid: %p."), m_pRenderTarget);
        return false;
    }

    if (!IsValid(m_pRenderMaterial))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRenderMaterial invalid: %p."), m_pRenderMaterial);
        return false;
    }

    if (m_sRenderMaterialTextureParamName.IsNone())
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_sRenderMaterialTextureParamName not specified."));
        return false;
    };

    return true;
}

int32 AMyTextureGenSuitBaseCpp::genPrepare()
{
    if (!AMyTextureGenSuitBaseCpp::checkSettings()) {
        return -1;
    }

    //prepare the capture excutor
    USceneCaptureComponent2D* pCC = m_pSceneCapture2D->GetCaptureComponent2D();

    pCC->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
    pCC->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
    pCC->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    pCC->bCaptureEveryFrame = false;
    pCC->bCaptureOnMovement = false;
    pCC->ClearShowOnlyComponents(NULL);
    int32 l = m_aTargetActors.Num();
    for (int32 i = 0; i < l; i++)
    {
        pCC->ShowOnlyActorComponents(m_aTargetActors[i]);
    }

    if (IsValid(m_pTempRenderTarget)) {
        m_pTempRenderTarget->MarkPendingKill();
        m_pTempRenderTarget = NULL;
    }
    m_pTempRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, m_pRenderTarget->SizeX, m_pRenderTarget->SizeY, m_pRenderTarget->RenderTargetFormat);
    MY_VERIFY(IsValid(m_pTempRenderTarget));

    if (IsValid(m_pTempRenderMaterialInstance)) {
        m_pTempRenderMaterialInstance->MarkPendingKill();
        m_pTempRenderMaterialInstance = NULL;
    }

    m_pTempRenderMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, m_pRenderMaterial);
    MY_VERIFY(IsValid(m_pTempRenderMaterialInstance));

    return 0;
}

int32 AMyTextureGenSuitBaseCpp::genDo(FString newTextureName)
{
    if (newTextureName.IsEmpty()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("newTextureName not specified."));
        return -1;
    }

    if (!AMyTextureGenSuitBaseCpp::checkSettings()) 
    {
        return -2;
    }

    if (!IsValid(m_pTempRenderTarget)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pTempRenderTarget invalid: %p."), m_pTempRenderTarget);
        return -11;
    }

    if (!IsValid(m_pTempRenderMaterialInstance)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pTempRenderMaterialInstance invalid: %p."), m_pTempRenderMaterialInstance);
        return -12;
    }

    UTextureRenderTarget2D *pTarget0 = m_pTempRenderTarget, *pTarget1 = m_pRenderTarget;

    USceneCaptureComponent2D* pCC = m_pSceneCapture2D->GetCaptureComponent2D();
    pCC->TextureTarget = pTarget0;

    pCC->CaptureScene();

    m_pTempRenderMaterialInstance->SetTextureParameterValue(m_sRenderMaterialTextureParamName, pTarget0);

    UKismetRenderingLibrary::ClearRenderTarget2D(this, pTarget1);
    UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, pTarget1, m_pTempRenderMaterialInstance);


    UTexture2D* pNew = UMyRenderUtilsLibrary::RenderTargetCreateStaticTexture2DTryBest(pTarget1, newTextureName, TextureCompressionSettings::TC_Default, TMGS_NoMipmaps);

    if (IsValid(pNew))
    {
        return 0;
    }
    else {
        return - 100;
    }
}


FMyArrangeCfgOneDimensionCpp FMyArrangeCfgOneDimensionCpp::inv() const
{
    FMyArrangeCfgOneDimensionCpp ret;
    ret = *this;
    ret.m_eCellOrderType = UMyRenderUtilsLibrary::Inv_MyOrderType(m_eCellOrderType);
    ret.m_eCellGatherType = UMyRenderUtilsLibrary::Inv_MyAxisAlignmentType(m_eCellGatherType);

    return ret;
}


void FMyArrangeCoordinateWorld3DCpp::helperSetIdxColRowStack(const FMyArrangePointResolvedMetaWorld3DCpp& meta, int32 colArranged, int32 rowArranged, int32 stackArranged)
{
    m_cStack.m_iIdxElem = meta.getStackIdx(stackArranged);
    m_cRow.m_iIdxElem = meta.getRowIdx(stackArranged, rowArranged);
    m_cCol.m_iIdxElem = meta.getColIdx(rowArranged, colArranged);
}

void FMyArrangeCoordinateWorld3DCpp::helperSetIdxColRowStackByMyArrangeCoordinateMeta(const FMyArrangePointResolvedMetaWorld3DCpp& meta, const FMyArrangeCoordinateMetaCpp& myArrangeCoordinateMeta)
{
    int32 colArranged = myArrangeCoordinateMeta.m_iColArranged;
    int32 rowArranged = 0;
    int32 stackArranged = 0;
    if (myArrangeCoordinateMeta.m_bColNumMaxLimitEnabled) {
        rowArranged = myArrangeCoordinateMeta.m_iRowArranged;
        if (myArrangeCoordinateMeta.m_bRowNumMaxLimitEnabled) {
            stackArranged = myArrangeCoordinateMeta.m_iStackArranged;
        }
    }

    helperSetIdxColRowStack(meta, colArranged, rowArranged, stackArranged);
}

void FMyArrangeCfgResolvedOneDimCpp::rebuildFrom(const FMyArrangeCfgOneDimensionCpp& cfgDim, MyAxisTypeCpp axisType,
                                                const FVector& areaExtend,
                                                const FMyModelInfoBoxWorld3DCpp& modelInfoExpected,
                                                const FMyModelInfoBoxWorld3DCpp& modelInfoUnexpected)
{
    *StaticCast<FMyArrangeCfgOneDimensionCpp *>(this) = cfgDim;
    m_fAreaExtend = UMyRenderUtilsLibrary::getAxisFromVectorRefConst(areaExtend, axisType);
    m_fCellExtendExpected = UMyRenderUtilsLibrary::getAxisFromVectorRefConst(modelInfoExpected.m_cBoxExtend, axisType);
    m_fCellExtendUnexpected = UMyRenderUtilsLibrary::getAxisFromVectorRefConst(modelInfoUnexpected.m_cBoxExtend, axisType);

    m_eAxisType = axisType;
}


 void FMyArrangePointResolvedMetaWorld3DCpp::rebuildFrom(const FMyArrangePointCfgWorld3DCpp& cPointCfg, const FMyModelInfoBoxWorld3DCpp& cModelInfo)
 {
     m_cCenterPointTransform = cPointCfg.m_cCenterPointTransform;
     m_fModelUnifiedSize = UMyRenderUtilsLibrary::getUnifiedSize2D3D(cModelInfo);
     
     FMyModelInfoBoxWorld3DCpp modelInfoExpected = UMyRenderUtilsLibrary::applyMyRotateState90D_ModelInfoBox_World3D(cModelInfo, cPointCfg.m_cLimitedRotationAllExpected);
     FMyModelInfoBoxWorld3DCpp modelInfoUnexpected = UMyRenderUtilsLibrary::applyMyRotateState90D_ModelInfoBox_World3D(cModelInfo, cPointCfg.m_cLimitedRotationAllUnexpected);

     TArray<MyAxisTypeCpp> aAxis;
     UMyRenderUtilsLibrary::getSortedAxis3D(cPointCfg.m_eColAxisType, cPointCfg.m_eRowAxisType, aAxis);

     m_aDimCfgs[0].rebuildFrom(cPointCfg.m_cColArrange, aAxis[0], cPointCfg.m_cAreaBoxExtendFinal, modelInfoExpected, modelInfoUnexpected);
     m_aDimCfgs[1].rebuildFrom(cPointCfg.m_cRowArrange, aAxis[1], cPointCfg.m_cAreaBoxExtendFinal, modelInfoExpected, modelInfoUnexpected);
     m_aDimCfgs[2].rebuildFrom(cPointCfg.m_cStackArrange, aAxis[2], cPointCfg.m_cAreaBoxExtendFinal, modelInfoExpected, modelInfoUnexpected);

     m_aDimCfgs[0].postBuild(&m_aDimCfgs[1], m_fModelUnifiedSize);
     m_aDimCfgs[1].postBuild(&m_aDimCfgs[2], m_fModelUnifiedSize);
     m_aDimCfgs[2].postBuild(NULL, m_fModelUnifiedSize);

     m_cModelInfo = cModelInfo;
 }


void UMyRenderUtilsLibrary::helperBoxModelResolveTransformWorld3D(const FMyArrangePointResolvedMetaWorld3DCpp& meta,
                                                                  const FMyArrangeCoordinateWorld3DCpp& coordinate,
                                                                  FTransform& outTransform)
{

    FMyModelInfoBoxWorld3DCpp modelInfoThisElem = UMyRenderUtilsLibrary::applyMyRotateState90D_ModelInfoBox_World3D(meta.m_cModelInfo, coordinate.m_cLimitedRotation);


    //let's locate the cell

    TArray<FMyArrangeCoordinateResolvedOneDimCpp> aCos;
    aCos.AddDefaulted(3);

    aCos[0].rebuildFrom(coordinate.m_cCol, meta.m_aDimCfgs[0].m_eAxisType, modelInfoThisElem);
    aCos[1].rebuildFrom(coordinate.m_cRow, meta.m_aDimCfgs[1].m_eAxisType, modelInfoThisElem);
    aCos[2].rebuildFrom(coordinate.m_cStack, meta.m_aDimCfgs[2].m_eAxisType, modelInfoThisElem);


    for (int32 i = 0; i < 3; i++) {
        const FMyArrangeCfgResolvedOneDimCpp& cfgDim = meta.m_aDimCfgs[i];
        FMyArrangeCoordinateResolvedOneDimCpp& coDim = aCos[i];

        float cellStart = cfgDim.getCellStart(meta.m_fModelUnifiedSize, coDim);


        MyContinuousAlignmentTypeCpp eElemInCellAlignmentType = cfgDim.m_eElemInCellAlignmentType;
        if (eElemInCellAlignmentType == MyContinuousAlignmentTypeCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eElemInCellAlignmentType invalid, using default. dim idx %d"), i);
            eElemInCellAlignmentType = MyContinuousAlignmentTypeCpp::Prev;
        }

        float elemStart = cellStart;
        if (eElemInCellAlignmentType == MyContinuousAlignmentTypeCpp::Prev) {
        }
        else if (eElemInCellAlignmentType == MyContinuousAlignmentTypeCpp::Mid) {
            elemStart += cfgDim.m_fCellExtendExpected - coDim.m_fElemExtend;
        }
        else {
            elemStart += (cfgDim.m_fCellExtendExpected - coDim.m_fElemExtend) * 2;
        }


        MyOrderTypeCpp eCellOrderType = cfgDim.m_eCellOrderType;
        if (eCellOrderType == MyOrderTypeCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eCellOrderType invalid, using default. dim idx %d"), i);
            eCellOrderType = MyOrderTypeCpp::ASC;
        }

        if (eCellOrderType == MyOrderTypeCpp::ASC) {
            coDim.m_fOutAxisValue = -cfgDim.m_fAreaExtend + elemStart - (coDim.m_fElemCenter - coDim.m_fElemExtend);
        }
        else {
            coDim.m_fOutAxisValue = cfgDim.m_fAreaExtend - elemStart - (coDim.m_fElemCenter + coDim.m_fElemExtend);
        }
    }


    FVector relativeLoc;
    for (int32 i = 0; i < 3; i++) {
        getAxisFromVectorRef(relativeLoc, aCos[i].m_eAxisType) = aCos[i].m_fOutAxisValue;
    }

    FRotator relativeRot = Conv_MyRotateState90DWorld3D_Rotator(coordinate.m_cLimitedRotation);

    FTransform relativeTransform;
    relativeTransform.SetLocation(relativeLoc);
    relativeTransform.SetRotation(relativeRot.Quaternion());

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("inputCoordinate : %s, model: %s."), *coordinate.ToString(), *meta.m_cModelInfo.ToString());
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("relativeTransform : %s."), *relativeTransform.ToString());

    outTransform = relativeTransform * meta.m_cCenterPointTransform;

    return;
}

void UMyRenderUtilsLibrary::helperBoxModelResolveTransformWidget2D(const FMyArrangePointResolvedMetaWorld3DCpp& meta,
                                                                   const FMyArrangeCoordinateWidget2DCpp& coordinate,
                                                                   FWidgetTransform& outTransform)
{
    FMyArrangeCoordinateWorld3DCpp co3D = Conv_MyArrangeCoordinateWidget2D_MyArrangeCoordinateWorld3D(coordinate);
    FTransform t3D;
    helperBoxModelResolveTransformWorld3D(meta, co3D, t3D);
    outTransform = Conv_TransformWorld3D_TransformWidget2D(t3D);
}

void UMyRenderUtilsLibrary::helperResolveTransformWorld3DFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
                                                                                                        float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                                        const FVector2D& targetPosiFixOnScreenPercent,
                                                                                                        float targetVOnScreenPercent,
                                                                                                        float targetModelHeightInWorld,
                                                                                                        FTransform &outTargetTranform)
{
    FVector2D popPoint = meta.m_cScreenCenter + targetPosiFromCenterToBorderOnScreenPercent * meta.m_fCenterToPointUntilBorderLength * meta.m_cDirectionCenterToPoint;

    popPoint += meta.m_cScreenCenter * 2 * targetPosiFixOnScreenPercent;

    float vAbsoluteOnScreen = targetVOnScreenPercent * meta.m_cScreenCenter.Y * 2;

    FVector cameraCenterLoc, cameraCenterDir;
    UMyCommonUtilsLibrary::helperResolveTransformWorld3DFromPlayerCameraByAbsolute(WorldContextObject, popPoint, vAbsoluteOnScreen, targetModelHeightInWorld, outTargetTranform, cameraCenterLoc, cameraCenterDir);
}

float UMyRenderUtilsLibrary::helperGetRemainTimePercent(const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp>& stepDatas)
{
    float total = 1;
    int32 l = stepDatas.Num();
    for (int32 i = 0; i < l; i++) {
        total -= stepDatas[i].m_fTimePercent;
    }

    if (total < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("total remain is negative: %f."), total);
        return 0;
    }

    return total;
}


void UMyRenderUtilsLibrary::helperUpdatersSetupStep(const FMyWithCurveUpdateStepMetaTransformWorld3DCpp& meta,
                                                                         const FMyWithCurveUpdateStepSettingsTransformWorld3DCpp& stepData,
                                                                         const TArray<FMyWithCurveUpdaterTransformWorld3DCpp *>& updatersSorted)
{
    float fTotalTime = meta.m_fTotalTime;

    const FTransform& pointTransform = meta.m_cPointTransform;
    const FTransform& disappearTransform = meta.m_cDisappearTransform;
    const FVector& modelBoxExtend = meta.m_cModelInfo.getBox3DRefConst().m_cBoxExtend;

    const FMyWithCurveUpdateStepSettingsTransformWorld3DCpp& cStepData = stepData;

    TArray<FTransform> aNextTransforms;

    int32 l = updatersSorted.Num();

    aNextTransforms.Reset();
    aNextTransforms.AddDefaulted(l);

    if (l <= 0) {
        return;
    };

    FVector finalLocationCenter = FVector::ZeroVector;
    for (int32 i = 0; i < l; i++) {
        if (updatersSorted[i] == NULL)
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("actorComponentsSortedGroup[%d] == NULL."), i);
            return;
        }
        FVector finalLocation = updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation();
        finalLocationCenter += finalLocation;

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("finalLocation: %s"), *finalLocation.ToString());
    }

    finalLocationCenter /= l;
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("l %d, finalLocationCenter: %s"), l, *finalLocationCenter.ToString());
    //find the middle one
    //int32 idxCenter = l / 2;
    //FMyWithCurveUpdaterTransformWorld3DCpp* updaterCenter = updatersSorted[idxCenter];
    //FTransform& nextTransformCenter = aNextTransforms[idxCenter];


    if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::PrevLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformPrevRefConst().GetLocation());
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::FinalLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation());
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::OffsetFromPrevLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformPrevRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::OffsetFromFinalLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::PointOnPlayerScreen)
    {
        FVector location = pointTransform.GetLocation();
        for (int32 i = 0; i < l; i++) {
            FVector locationForScreen = location + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2 + updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation() - finalLocationCenter;
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("locationForScreen: %s"), *locationForScreen.ToString());
            aNextTransforms[i].SetLocation(locationForScreen);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::DisappearAtAttenderBorderOnPlayerScreen)
    {
        FVector location = disappearTransform.GetLocation();
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(location + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2 + updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation() - finalLocationCenter);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::OffsetFromGroupPoint)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformGroupPointRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2);
        }
    }

    if (cStepData.m_eRotationUpdateType == MyWithCurveUpdateStepSettingsRotationType::PrevRotation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(updatersSorted[i]->getHelperTransformPrevRefConst().GetRotation());
        }
    }
    else if (cStepData.m_eRotationUpdateType == MyWithCurveUpdateStepSettingsRotationType::FinalRotation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(updatersSorted[i]->getHelperTransformFinalRefConst().GetRotation());
        }
    }
    else if (cStepData.m_eRotationUpdateType == MyWithCurveUpdateStepSettingsRotationType::FacingPlayerScreen)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(pointTransform.GetRotation());
        }
    }

    if (cStepData.m_eScaleUpdateType == MyWithCurveUpdateStepSettingsScaleType::PrevScale)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(updatersSorted[i]->getHelperTransformPrevRefConst().GetScale3D());
        }
    }
    else  if (cStepData.m_eScaleUpdateType == MyWithCurveUpdateStepSettingsScaleType::FinalScale)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(updatersSorted[i]->getHelperTransformFinalRefConst().GetScale3D());
        }
    }
    else  if (cStepData.m_eScaleUpdateType == MyWithCurveUpdateStepSettingsScaleType::Specified)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(cStepData.m_cScaleSpecified);
        }
    }


    float fDur = fTotalTime * cStepData.m_fTimePercent;
    UCurveVector* pCurve = UMyCommonUtilsLibrary::getCurveVectorFromSettings(cStepData.m_cCurve);
    for (int32 i = 0; i < l; i++) {

        FMyWithCurveUpdaterTransformWorld3DCpp *pUpdater = updatersSorted[i];

        FMyWithCurveUpdateStepDataTransformWorld3DCpp data;
        data.helperSetDataBySrcAndDst(fDur, pCurve, pUpdater->getHelperTransformPrevRefConst(), aNextTransforms[i], cStepData.m_cRotationUpdateExtraCycles);
        pUpdater->addStepToTail(data);
    }
};


void UMyRenderUtilsLibrary::helperUpdatersSetupSteps(const FMyWithCurveUpdateStepMetaTransformWorld3DCpp& meta,
                                                    const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp>& stepDatas,
                                                    const TArray<FMyWithCurveUpdaterTransformWorld3DCpp *>& updatersSorted)
{
    int32 l = stepDatas.Num();

    if (l <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("stepDatas num is zero, debugStr: %s."), *meta.m_sDebugString);
        return;
    }

    float total = 0;

    bool bTimePecentTotalExpectedNot100Pecent = false;

    for (int32 i = 0; i < l; i++) {
        helperUpdatersSetupStep(meta, stepDatas[i], updatersSorted);
        float f = stepDatas[i].m_fTimePercent;
        total += f;

        bTimePecentTotalExpectedNot100Pecent |= stepDatas[i].m_bTimePecentTotalExpectedNot100Pecent;
    }

    if ((!bTimePecentTotalExpectedNot100Pecent) && l > 0 && !FMath::IsNearlyEqual(total, 1, MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("total time is not 100%, str %s. now it is %f."), *meta.m_sDebugString, total);
    };
}

void UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(const UObject* WorldContextObject,
                                                                                    float totalDur,
                                                                                    const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                                    const FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                                    const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp>& stepDatas,
                                                                                    float extraDelayDur,
                                                                                    const TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp *>& updaterInterfaces,
                                                                                    FString debugName,
                                                                                    bool clearPrevSteps)
{
    FMyWithCurveUpdateStepMetaTransformWorld3DCpp meta, *pMeta = &meta;
    TArray<FMyWithCurveUpdaterTransformWorld3DCpp *> aUpdaters, *pUpdaters = &aUpdaters;

    //prepare meta
    pMeta->m_sDebugString = debugName;
    pMeta->m_fTotalTime = totalDur;
    if (updaterInterfaces.Num() <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("updaterInterfaces num is zero."));
        return;
    }

    pMeta->m_cModelInfo = updaterInterfaces[0]->getModelInfoForUpdaterEnsured();

    helperResolveTransformWorld3DFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, pointInfo.m_fShowPosiFromCenterToBorderPercent, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelInfo.getBox3DRefConst().m_cBoxExtend.Size() * 2, pMeta->m_cPointTransform);
    helperResolveTransformWorld3DFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, 1.2, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelInfo.getBox3DRefConst().m_cBoxExtend.Size() * 2, pMeta->m_cDisappearTransform);

    bool bDelay = extraDelayDur >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT;

    pUpdaters->Reset();
    int32 l = updaterInterfaces.Num();
    for (int32 i = 0; i < l; i++) {
        FMyWithCurveUpdaterTransformWorld3DCpp* pUpdater = &updaterInterfaces[i]->getMyWithCurveUpdaterTransformRef();
        pUpdaters->Emplace(pUpdater);
        if (clearPrevSteps) {
            pUpdater->clearSteps();
        }
    }

    if (bDelay) {
        UMyRenderUtilsLibrary::helperUpdatersAddWaitStep(extraDelayDur, debugName + TEXT(" wait"), *pUpdaters);
    }


    UMyRenderUtilsLibrary::helperUpdatersSetupSteps(meta, stepDatas, *pUpdaters);
}

void UMyRenderUtilsLibrary::helperUpdatersAddWaitStep(float waitTime, FString debugStr, const TArray<FMyWithCurveUpdaterTransformWorld3DCpp *>& updaters)
{
    FMyWithCurveUpdateStepMetaTransformWorld3DCpp meta;
    FMyWithCurveUpdateStepSettingsTransformWorld3DCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = waitTime;

    stepData.m_fTimePercent = 1;


    helperUpdatersSetupStep(meta, stepData, updaters);
}

void UMyRenderUtilsLibrary::helperUpdatersAddWaitStep(float waitTime, FString debugStr, const TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp *>& updaterInterfaces)
{

    TArray<FMyWithCurveUpdaterTransformWorld3DCpp *> aUpdaters;

    for (int32 i = 0; i < updaterInterfaces.Num(); i++)
    {
        aUpdaters.Emplace(&updaterInterfaces[i]->getMyWithCurveUpdaterTransformRef());
    }

    UMyRenderUtilsLibrary::helperUpdatersAddWaitStep(waitTime, debugStr, aUpdaters);
}


FString UMyRenderUtilsLibrary::Conv_SlateBrush_String(const FSlateBrush& brush)
{
    UObject* pRO = brush.GetResourceObject();
    FString ROStr = TEXT("NULL");
    if (pRO) {
        ROStr = pRO->GetPathName();
    }
    
    return FString::Printf(TEXT("RO: %s"), *ROStr);
}

bool UMyRenderUtilsLibrary::RenderTargetIsSizePowerOfTwo(class UTextureRenderTarget2D* inTextureRenderTarget)
{
    if (inTextureRenderTarget == NULL)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("inTextureRenderTarget is NULL."));
        return false;
    }

    const bool bIsValidSize = (inTextureRenderTarget->SizeX != 0 && !(inTextureRenderTarget->SizeX & (inTextureRenderTarget->SizeX - 1)) &&
        inTextureRenderTarget->SizeY != 0 && !(inTextureRenderTarget->SizeY & (inTextureRenderTarget->SizeY - 1)));

    return bIsValidSize;
}

UTexture2D* UMyRenderUtilsLibrary::RenderTargetConstructTexture2DIgnoreSizeLimit(class UTextureRenderTarget2D* inTextureRenderTarget, UObject* Outer, const FString& NewTexName, EObjectFlags InObjectFlags, uint32 Flags, TArray<uint8>* AlphaOverride)
{
    if (inTextureRenderTarget == NULL)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("inTextureRenderTarget is NULL."));
        return NULL;
    }

    Flags &= (~CTF_AllowMips); //we assume the size is not power of two, and forbid the mip feature
    UTexture2D* Result = NULL;

#if WITH_EDITOR

    // The r2t resource will be needed to read its surface contents
    FRenderTarget* RenderTarget = inTextureRenderTarget->GameThread_GetRenderTargetResource();

    const EPixelFormat PixelFormat = inTextureRenderTarget->GetFormat();
    ETextureSourceFormat TextureFormat = TSF_Invalid;
    TextureCompressionSettings CompressionSettingsForTexture = TC_Default;
    switch (PixelFormat)
    {
    case PF_B8G8R8A8:
        TextureFormat = TSF_BGRA8;
        break;
    case PF_FloatRGBA:
        TextureFormat = TSF_RGBA16F;
        CompressionSettingsForTexture = TC_HDR;
        break;
    }

    // exit if source is not compatible.
    if (TextureFormat == TSF_Invalid)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("TextureFormat is not supported in renderTarget."));
        return NULL;
    };


    // create the 2d texture
    Result = NewObject<UTexture2D>(Outer, FName(*NewTexName), InObjectFlags);
    // init to the same size as the 2d texture
    Result->Source.Init(inTextureRenderTarget->SizeX, inTextureRenderTarget->SizeY, 1, 1, TextureFormat);

    uint32* TextureData = (uint32*)Result->Source.LockMip(0);
    const int32 TextureDataSize = Result->Source.CalcMipSize(0);

    // read the 2d surface
    if (TextureFormat == TSF_BGRA8)
    {
        TArray<FColor> SurfData;
        RenderTarget->ReadPixels(SurfData);
        // override the alpha if desired
        if (AlphaOverride)
        {
            check(SurfData.Num() == AlphaOverride->Num());
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = (*AlphaOverride)[Pixel];
            }
        }
        else if (Flags & CTF_RemapAlphaAsMasked)
        {
            // if the target was rendered with a masked texture, then the depth will probably have been written instead of 0/255 for the
            // alpha, and the depth when unwritten will be 255, so remap 255 to 0 (masked out area) and anything else as 255 (written to area)
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = (SurfData[Pixel].A == 255) ? 0 : 255;
            }
        }
        else if (Flags & CTF_ForceOpaque)
        {
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = 255;
            }
        }
        // copy the 2d surface data to the first mip of the static 2d texture
        check(TextureDataSize == SurfData.Num() * sizeof(FColor));
        FMemory::Memcpy(TextureData, SurfData.GetData(), TextureDataSize);
    }
    else if (TextureFormat == TSF_RGBA16F)
    {
        TArray<FFloat16Color> SurfData;
        RenderTarget->ReadFloat16Pixels(SurfData);
        // override the alpha if desired
        if (AlphaOverride)
        {
            check(SurfData.Num() == AlphaOverride->Num());
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = ((float)(*AlphaOverride)[Pixel]) / 255.0f;
            }
        }
        else if (Flags & CTF_RemapAlphaAsMasked)
        {
            // if the target was rendered with a masked texture, then the depth will probably have been written instead of 0/255 for the
            // alpha, and the depth when unwritten will be 255, so remap 255 to 0 (masked out area) and anything else as 1 (written to area)
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = (SurfData[Pixel].A == 255) ? 0.0f : 1.0f;
            }
        }
        else if (Flags & CTF_ForceOpaque)
        {
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = 1.0f;
            }
        }
        // copy the 2d surface data to the first mip of the static 2d texture
        check(TextureDataSize == SurfData.Num() * sizeof(FFloat16Color));
        FMemory::Memcpy(TextureData, SurfData.GetData(), TextureDataSize);
    }
    Result->Source.UnlockMip(0);
    // if render target gamma used was 1.0 then disable SRGB for the static texture
    if (FMath::Abs(RenderTarget->GetDisplayGamma() - 1.0f) < KINDA_SMALL_NUMBER)
    {
        Flags &= ~CTF_SRGB;
    }

    Result->SRGB = (Flags & CTF_SRGB) != 0;
    Result->MipGenSettings = TMGS_FromTextureGroup;

    if ((Flags & CTF_AllowMips) == 0)
    {
        Result->MipGenSettings = TMGS_NoMipmaps;
    }

    Result->CompressionSettings = CompressionSettingsForTexture;
    if (Flags & CTF_Compress)
    {
        // Set compression options.
        Result->DeferCompression = (Flags & CTF_DeferCompression) ? true : false;
    }
    else
    {
        // Disable compression
        Result->CompressionNone = true;
        Result->DeferCompression = false;
    }
    Result->PostEditChange();

#else

    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("create static mesh only valid in editor env now!"));

#endif

    return Result;
};

UTexture2D* UMyRenderUtilsLibrary::RenderTargetCreateStaticTexture2DTryBest(UTextureRenderTarget2D* inTextureRenderTarget, FString InName, enum TextureCompressionSettings CompressionSettings, enum TextureMipGenSettings MipSettings)
{
#if WITH_EDITOR

    if (inTextureRenderTarget == NULL)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("inTextureRenderTarget is NULL."));
        return nullptr;
    }

    else if (!inTextureRenderTarget->Resource)
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("RenderTarget has been released."));
        return nullptr;
    }
    else
    {
        FString Name;
        FString PackageName;
        IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

        //Use asset name only if directories are specified, otherwise full path
        if (!InName.Contains(TEXT("/")))
        {
            FString AssetName = inTextureRenderTarget->GetOutermost()->GetName();
            const FString SanitizedBasePackageName = PackageTools::SanitizePackageName(AssetName);
            const FString PackagePath = FPackageName::GetLongPackagePath(SanitizedBasePackageName) + TEXT("/");
            AssetTools.CreateUniqueAssetName(PackagePath, InName, PackageName, Name);
        }
        else
        {
            InName.RemoveFromStart(TEXT("/"));
            InName.RemoveFromStart(TEXT("Content/"));
            InName.StartsWith(TEXT("Game/")) == true ? InName.InsertAt(0, TEXT("/")) : InName.InsertAt(0, TEXT("/Game/"));
            AssetTools.CreateUniqueAssetName(InName, TEXT(""), PackageName, Name);
        }

        UObject* NewObj = nullptr;
        UPackage* pPackage = CreatePackage(NULL, *PackageName);
        bool bIsSizePowerOfTwo = RenderTargetIsSizePowerOfTwo(inTextureRenderTarget);

        // create a static 2d texture
        if (bIsSizePowerOfTwo)
        {
            NewObj = inTextureRenderTarget->ConstructTexture2D(pPackage, Name, inTextureRenderTarget->GetMaskedFlags(), CTF_Default | CTF_AllowMips, NULL);
        }
        else {
            NewObj = RenderTargetConstructTexture2DIgnoreSizeLimit(inTextureRenderTarget, pPackage, Name, inTextureRenderTarget->GetMaskedFlags(), CTF_Default | CTF_AllowMips, NULL);
        }


        UTexture2D* NewTex = Cast<UTexture2D>(NewObj);

        if (NewTex != nullptr)
        {
            // package needs saving
            NewObj->MarkPackageDirty();

            // Notify the asset registry
            FAssetRegistryModule::AssetCreated(NewObj);

            // Update Compression and Mip settings
            NewTex->CompressionSettings = CompressionSettings;
            NewTex->MipGenSettings = MipSettings;
            NewTex->PostEditChange();

            return NewTex;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Failed to create a new texture, bIsSizePowerOfTwo %d."), bIsSizePowerOfTwo);
            return nullptr;
        }
    }
#else

    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Create static mesh from render target is Only supported in editor now."));
    return nullptr;

#endif

}


int32 UMyRenderUtilsLibrary::getCenterPointPositionForWidgetInCanvasWithPointAnchor(const FVector2D& canvasSize, const UWidget* widgetUnderCanvas, FVector2D& positionInCanvas)
{
    const UCanvasPanelSlot* pSlot = NULL;
    if (widgetUnderCanvas)
    {
        pSlot = Cast<const UCanvasPanelSlot>(widgetUnderCanvas->Slot);
        if (pSlot == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("widget is not under canvas, slot class name: %s."), *widgetUnderCanvas->Slot->GetClass()->GetName());
            return -10;
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("widgetUnderCanvas is NULL."), widgetUnderCanvas);
        return -1;
    }

    FAnchorData layoutData = pSlot->GetLayout();

    bool bIsAnchorPoint = (!layoutData.Anchors.IsStretchedHorizontal()) && (!layoutData.Anchors.IsStretchedVertical());
    if (!bIsAnchorPoint) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("widget is not anchored one point: min %s, max %s."), *layoutData.Anchors.Minimum.ToString(), *layoutData.Anchors.Maximum.ToString());
        return -20;
    }

    FVector2D anchorEndPosi = canvasSize * layoutData.Anchors.Minimum;
    FVector2D anchorStartPosi = anchorEndPosi + FVector2D(layoutData.Offsets.Left, layoutData.Offsets.Top);
    FVector2D sizeWidget2D = FVector2D(layoutData.Offsets.Right, layoutData.Offsets.Bottom);
    FVector2D widgetCenterOffsetFromAnchorStartPosi = (FVector2D(0.5, 0.5) - layoutData.Alignment) * sizeWidget2D;

    positionInCanvas = anchorStartPosi + widgetCenterOffsetFromAnchorStartPosi;

    return 0;
}


void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeCalcTotalSize(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupCount, const FIntVector& elemCount, FVector& totalSize)
{
    FVector distanceIgnorePadding;
    myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(meta, groupCount, elemCount, distanceIgnorePadding);

    totalSize = meta.m_cAllMarginPercent.GetDesiredSize3D() * meta.m_cElemSize + distanceIgnorePadding;
}

void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeGetElemCenterPositionArrangeDirectionAllPositive(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& idxGroup, const FIntVector& idxElem, FVector& centerPosition)
{
    FVector distanceIgnorePadding;
    myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(meta, idxGroup, idxElem, distanceIgnorePadding);

    FVector distanceCenterIgnorePadding = distanceIgnorePadding + meta.m_cElemSize / 2;

    //direction is from negtive to positve
    FVector sourcePaddingPecent(meta.m_cAllMarginPercent.Left, meta.m_cAllMarginPercent.Top, meta.m_cAllMarginPercent.ZNegative);

    centerPosition = distanceCenterIgnorePadding + sourcePaddingPecent * meta.m_cElemSize;
}

void UMyRenderUtilsLibrary::myElemAndGroupCalcDelimiterNumber(const FIntVector &groupWalked, const FIntVector &elemWalked, FIntVector &groupDelimiterNumber, FIntVector &elemDelimiterNumber)
{
    MY_VERIFY(groupWalked.X >= 0 && groupWalked.Y >= 0 && groupWalked.Z >= 0);
    MY_VERIFY(elemWalked.X >= 0 && elemWalked.Y >= 0 && elemWalked.Z >= 0);

    groupDelimiterNumber.X = groupWalked.X > 1 ? groupWalked.X - 1 : 0;
    groupDelimiterNumber.Y = groupWalked.Y > 1 ? groupWalked.Y - 1 : 0;
    groupDelimiterNumber.Z = groupWalked.Z > 1 ? groupWalked.Z - 1 : 0;

    elemDelimiterNumber.X = elemWalked.X - 1 - groupDelimiterNumber.X;
    if (elemDelimiterNumber.X < 0) elemDelimiterNumber.X = 0;

    elemDelimiterNumber.Y = elemWalked.Y - 1 - groupDelimiterNumber.Y;
    if (elemDelimiterNumber.Y < 0) elemDelimiterNumber.Y = 0;

    elemDelimiterNumber.Z = elemWalked.Z - 1 - groupDelimiterNumber.Z;
    if (elemDelimiterNumber.Z < 0) elemDelimiterNumber.Z = 0;
}

/*
FVector2D UMyRenderUtilsLibrary::Map_BoxWidget2D_BoxWidget2D(const FVector2D& boxExtendA, const FVector2D& boxExtendB, const FVector2D& coordinateInA)
{
    //MyDeNominatorAsZeroTolerance
    //FMath::IsNealyEqual();
};
*/

float UMyRenderUtilsLibrary::getDegreeInSquareFromPointInSquare_Widget2D(const FVector2D& pointFromCenterInSquare)
{
    float d = UKismetMathLibrary::DegAtan2(pointFromCenterInSquare.Y, pointFromCenterInSquare.X);
    float DegreeInSquare = -d + 90;
    return FMath::UnwindDegrees(DegreeInSquare);
}

void UMyRenderUtilsLibrary::helperArrangePointsToPositionsInBoxEvenly_Widget2D(float boxXYRatio, const TArray<FVector2D>& pointsInBox, TArray<int32>& outPositions, TArray<float>& outDeltas)
{
    outPositions.Reset();
    outDeltas.Reset();

    int32 l = pointsInBox.Num();
    if (l <= 0) {
        return;
    }

    FVector2D center = FVector2D::ZeroVector;
    for (int32 i = 0; i < l; i++) {
        center += pointsInBox[i];
    }
    center /= l;

    TArray<float> aSquareScreenDegrees;
    for (int32 i = 0; i < l; i++) {
        FVector2D p = pointsInBox[i] - center;
        float d = getDegreeInSquareFromPointInSquare_Widget2D(Map_BoxWidget2D_SquareWidget2D(boxXYRatio, p));
        aSquareScreenDegrees.Emplace(d);

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%d: mapped degree in square %f."), i, d);
    }

    int32 pointArrangedCountMaxPerPosition = l / 4;
    if ((l % 4) > 0) {
        pointArrangedCountMaxPerPosition += 1;
    }

    TArray<int32> aPointArrangedCountForPositions;
    aPointArrangedCountForPositions.AddZeroed(4);

    //let's arrange them
    for (int32 i = 0; i < l; i++) {
        float pointSquareScreenDegree = aSquareScreenDegrees[i];
        int32 idxScreenPositionFreeAndDeltaAbsMin = -1;
        float deltaMinSquareScreenDegree = 1000;
        float deltaAbsMinSquareScreenDegree = 1000;
        for (int32 j = 0; j < 4; j++) {
            int32& pointCount = aPointArrangedCountForPositions[j];
            if (pointCount >= pointArrangedCountMaxPerPosition) {
                continue;
            }

            float centerSquareScreenDegree = FMath::UnwindDegrees(90 * j);
            float deltaSquareScreenDegree = FMath::UnwindDegrees(pointSquareScreenDegree - centerSquareScreenDegree);
            float deltaAbsSquareScreenDegree = FMath::Abs<float>(deltaSquareScreenDegree);
            if (deltaAbsSquareScreenDegree < deltaAbsMinSquareScreenDegree) {
                deltaMinSquareScreenDegree = deltaSquareScreenDegree;
                deltaAbsMinSquareScreenDegree = deltaAbsSquareScreenDegree;
                idxScreenPositionFreeAndDeltaAbsMin = j;
            }
        }

        MY_VERIFY(idxScreenPositionFreeAndDeltaAbsMin >= 0);
        outPositions.Emplace(idxScreenPositionFreeAndDeltaAbsMin);
        outDeltas.Emplace(deltaMinSquareScreenDegree);
        aPointArrangedCountForPositions[idxScreenPositionFreeAndDeltaAbsMin]++;

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%d output: %d, %f."), i, idxScreenPositionFreeAndDeltaAbsMin, deltaMinSquareScreenDegree);
    }

    return;
};

void UMyRenderUtilsLibrary::helperGenMappedPointsForMyAreaAlignToBorders_Widget2D(const FVector2D& boxCenter, const FVector2D& boxExtend, const TArray<const FMyAreaAlignToBorderCfgOneDimCpp*>& boxAlignedAreas,
                                                                                   bool alignAtBorderOutside, bool minMaxClockWise,
                                                                                   TArray<FMyAreaAlignToBorderMappedPointsWidget2DCpp>& outAreaMappedPoints)
{
    MY_VERIFY(boxAlignedAreas.Num() == 4);
    
    int32 l = boxAlignedAreas.Num();
    outAreaMappedPoints.Reset();
    outAreaMappedPoints.AddDefaulted(l);

    for (int32 i = 0; i < l; i++) {

        const FMyAreaAlignToBorderCfgOneDimCpp* pCfgDim = boxAlignedAreas[i];
        MY_VERIFY(pCfgDim);
        const FMyAreaAlignToBorderCfgOneDimCpp& cfgDim = *pCfgDim;

        FMyAreaAlignToBorderMappedPointsWidget2DCpp& outArea = outAreaMappedPoints[i];

        outArea.m_fDeltaMappedDegreeClampPercent = pCfgDim->m_fDeltaMappedDegreeClampPercent;


        FVector2D& minP = outArea.m_cMin;
        FVector2D& centerP = outArea.m_cExpectedCenter;
        FVector2D& maxP = outArea.m_cMax;
        MyAxisTypeCpp& eToBorderAxisType = outArea.m_eToBorderAxisType;
        MyAxisAlignmentTypeCpp& eToBorderAlignmentType = outArea.m_eToBorderAlignmentType;

        //try align inside box border, counterClockWise
        if (i == 0) {

            eToBorderAxisType = MyAxisTypeCpp::Y;
            eToBorderAlignmentType = MyAxisAlignmentTypeCpp::Positive;

            float targetY = boxExtend.Y;

            minP.X = -boxExtend.X + cfgDim.m_fMinPercent * boxExtend.X * 2;
            minP.Y = targetY;

            centerP.X = -boxExtend.X + cfgDim.m_fExpectedCenterPercent * boxExtend.X * 2;
            centerP.Y = targetY;

            maxP.X = -boxExtend.X + cfgDim.m_fMaxPercent * boxExtend.X * 2;
            maxP.Y = targetY;
        }
        else if (i == 1) {

            eToBorderAxisType = MyAxisTypeCpp::X;
            eToBorderAlignmentType = MyAxisAlignmentTypeCpp::Positive;

            float targetX = boxExtend.X;

            minP.X = targetX;
            minP.Y = -boxExtend.Y + cfgDim.m_fMaxPercent * boxExtend.Y * 2;

            centerP.X = targetX;
            centerP.Y = -boxExtend.Y + cfgDim.m_fExpectedCenterPercent * boxExtend.Y * 2;

            maxP.X = targetX;
            maxP.Y = -boxExtend.Y + cfgDim.m_fMinPercent * boxExtend.Y * 2;
        }
        else if (i == 2) {

            eToBorderAxisType = MyAxisTypeCpp::Y;
            eToBorderAlignmentType = MyAxisAlignmentTypeCpp::Negative;

            float targetY = -boxExtend.Y;

            minP.X = -boxExtend.X + cfgDim.m_fMaxPercent * boxExtend.X * 2;
            minP.Y = targetY;

            centerP.X = -boxExtend.X + cfgDim.m_fExpectedCenterPercent * boxExtend.X * 2;
            centerP.Y = targetY;

            maxP.X = -boxExtend.X + cfgDim.m_fMinPercent * boxExtend.X * 2 ;
            maxP.Y = targetY;
        }
        else if (i == 3) {

            eToBorderAxisType = MyAxisTypeCpp::X;
            eToBorderAlignmentType = MyAxisAlignmentTypeCpp::Negative;

            float targetX = -boxExtend.X;

            minP.X = targetX;
            minP.Y = -boxExtend.Y + cfgDim.m_fMinPercent * boxExtend.Y * 2;

            centerP.X = targetX;
            centerP.Y = -boxExtend.Y + cfgDim.m_fExpectedCenterPercent * boxExtend.Y * 2;

            maxP.X = targetX;
            maxP.Y = -boxExtend.Y + cfgDim.m_fMaxPercent * boxExtend.Y * 2;

        }
        else {
            MY_VERIFY(false);
        }

        if (alignAtBorderOutside) {
            eToBorderAlignmentType = Inv_MyAxisAlignmentType(eToBorderAlignmentType);
        }

        if (minMaxClockWise) {
            FVector2D t = minP;
            minP = maxP;
            maxP = t;
        }

        minP += boxCenter;
        centerP += boxCenter;
        maxP += boxCenter;
    }
}

FVector2D UMyRenderUtilsLibrary::helperResolveLocationForMyAreaAlignToBorderMappedPoints_Widget2D(const TArray<FMyAreaAlignToBorderMappedPointsWidget2DCpp>& areaMappedPoints,
                                                                                                  const FMyModelInfoBoxWidget2DCpp& widgetModelInfo, int32 widgetIdxPositionInBox, float widgetDeltaFromExpectedDegreeInSquare)
{
    MY_VERIFY(widgetIdxPositionInBox >= 0 && widgetIdxPositionInBox < 4);
    MY_VERIFY(areaMappedPoints.Num() == 4);


    const FMyAreaAlignToBorderMappedPointsWidget2DCpp& areaMapped= areaMappedPoints[widgetIdxPositionInBox];

    float clampV = 45 * FMath::Clamp<float>(areaMapped.m_fDeltaMappedDegreeClampPercent, 0.1, 1);

    FVector2D targetLocation;
    if (widgetDeltaFromExpectedDegreeInSquare >= 0) {
        float dCamped = FMath::Clamp<float>(widgetDeltaFromExpectedDegreeInSquare, 0, clampV);
        float percent = dCamped / clampV;
        targetLocation = FMath::Vector2DInterpTo(areaMapped.m_cExpectedCenter, areaMapped.m_cMax, percent, 1);

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("1: dCamped %f, percent %f, range (%s, %s), output: %s."), dCamped, percent,
                   //*areaMapped.m_cExpectedCenter.ToString(), *areaMapped.m_cMax.ToString(), *targetLocation.ToString());
    }
    else {
        float dCamped = FMath::Clamp<float>(widgetDeltaFromExpectedDegreeInSquare, -clampV, 0);
        float percent = (dCamped - (-clampV)) / clampV;
        targetLocation = FMath::Vector2DInterpTo(areaMapped.m_cMin, areaMapped.m_cExpectedCenter, percent, 1);


        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("2: dCamped %f, percent %f, range (%s, %s), output: %s."), dCamped, percent,
                  //*areaMapped.m_cMin.ToString(), *areaMapped.m_cExpectedCenter.ToString(), *targetLocation.ToString());
    }

    //aligne to center
    targetLocation -= widgetModelInfo.m_cCenterPointRelativeLocation;

    //algin to border
    if (areaMapped.m_eToBorderAlignmentType == MyAxisAlignmentTypeCpp::Negative) {
        getAxisFromVector2DRef(targetLocation, areaMapped.m_eToBorderAxisType) += getAxisFromVector2DRefConst(widgetModelInfo.m_cBoxExtend, areaMapped.m_eToBorderAxisType);
    }
    else if (areaMapped.m_eToBorderAlignmentType == MyAxisAlignmentTypeCpp::Mid) {

    }
    else if (areaMapped.m_eToBorderAlignmentType == MyAxisAlignmentTypeCpp::Positive) {
        getAxisFromVector2DRef(targetLocation, areaMapped.m_eToBorderAxisType) -= getAxisFromVector2DRefConst(widgetModelInfo.m_cBoxExtend, areaMapped.m_eToBorderAxisType);
    }
    else {
        MY_VERIFY(false);
    }

    return targetLocation;
}


MyErrorCodeCommonPartCpp UMyRenderUtilsLibrary::checkUserWidgetInCanvasPanelComplyModelInfo_Base_Widget2D(UUserWidget* childWidget)
{
    UCanvasPanelSlot* pCPSlot = Cast<UCanvasPanelSlot>(childWidget->Slot);
    if (pCPSlot == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: widget slot type is not canvas panel slot, it is %s."), *childWidget->GetName(), *childWidget->Slot->GetClass()->GetName());
        return MyErrorCodeCommonPartCpp::UIPanelSlotTypeUnexpected;
    }

    const FAnchorData& ad = pCPSlot->LayoutData;

    if ((!ad.Anchors.Minimum.Equals(FVector2D::ZeroVector)) || (!ad.Anchors.Maximum.Equals(FVector2D::ZeroVector))) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvas panel slot anchor unexpected, now only support left top anchor."), *childWidget->GetName());
        return MyErrorCodeCommonPartCpp::UIPanelSlotDataUnexpected;
    }

    if (!childWidget->RenderTransformPivot.Equals(ad.Alignment, MyCommonMinDelta)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: require canvas panel slot algnment equal to render transform pivot, %s, %s."),
            *childWidget->GetName(), *ad.Alignment.ToString(), *childWidget->RenderTransformPivot.ToString());
        return MyErrorCodeCommonPartCpp::UIPanelSlotDataUnexpected;
    }

    if ((!FMath::IsNearlyEqual(ad.Offsets.Left, 0.f, MyCommonMinDelta)) || (!FMath::IsNearlyEqual(ad.Offsets.Top, 0.f, MyCommonMinDelta))) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: require offset left top as zero, now is, left %f, top %f."),
            *childWidget->GetName(), ad.Offsets.Left, ad.Offsets.Top);
        return MyErrorCodeCommonPartCpp::UIPanelSlotDataUnexpected;
    }

    return MyErrorCodeCommonPartCpp::NoError;

    /*
    FVector2D renderTransformPivotExpected = UMyRenderUtilsLibrary::getOriginPointRelativeToXYMinByMyModelInfoBoxWidget2D(modelInfoBox) / (modelInfoBox.m_cBoxExtend * 2);
    if (!renderTransformPivotExpected.Equals(widgetRenderTransformPivot, MyCommonMinDelta)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("widgetRenderTransformPivot is not as expected: expected %s, now %s. modelInfoBox %s."),
            *renderTransformPivotExpected.ToString(), *widgetRenderTransformPivot.ToString(), *modelInfoBox.ToString());
        return false;
    }

    return true;
    */
}

MyErrorCodeCommonPartCpp UMyRenderUtilsLibrary::updateUserWidgetInCanvasPanelWithModelInfo_Widget2D(UUserWidget* childWidget, const FMyModelInfoWidget2DCpp& modelInfo)
{
    if (modelInfo.getType() != MyModelInfoType::BoxWidget2D) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("target %s: model type is supposed to be mode2d, but it is  %d."), *childWidget->GetName(), (uint8)modelInfo.getType());
        return MyErrorCodeCommonPartCpp::TypeUnexpected;
    }

    UCanvasPanelSlot* pCPSlot = Cast<UCanvasPanelSlot>(childWidget->Slot);
    if (pCPSlot == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: widget slot type is not canvas panel slot, it is %s."), *childWidget->GetName(), *childWidget->Slot->GetClass()->GetName());
        return MyErrorCodeCommonPartCpp::UIPanelSlotTypeUnexpected;
    }

    const FMyModelInfoBoxWidget2DCpp& modelInfoBox = modelInfo.getBox2DRefConst();


    FVector2D originPointRelativeToXYMin = getOriginPointRelativeToXYMinByMyModelInfoBoxWidget2D(modelInfoBox);
    FVector2D originPercent = originPointRelativeToXYMin / (modelInfoBox.m_cBoxExtend * 2);


    FAnchorData ad;
    
    ad.Anchors.Minimum = FVector2D::ZeroVector;
    ad.Anchors.Maximum = FVector2D::ZeroVector;
    
    //make origin at zero point
    ad.Offsets.Left = 0;
    ad.Offsets.Top = 0;
    ad.Offsets.Right = modelInfoBox.m_cBoxExtend.X * 2;
    ad.Offsets.Bottom = modelInfoBox.m_cBoxExtend.Y * 2;

    ad.Alignment = originPercent;

    pCPSlot->SetLayout(ad);


    childWidget->SetRenderTransformPivot(originPercent);

    //recheck
    FMyModelInfoWidget2DCpp modelInfo2;
    MyErrorCodeCommonPartCpp ret = evaluateUserWidgetInCanvasPanelForModelInfo_Widget2D(childWidget, modelInfo2);
    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: failed to retrieve model info back after update, erroCode %d."), *childWidget->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
    }
    else {
        if (!modelInfo.equals(modelInfo2, MyCommonMinDelta)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: modelInfo retrieved not equal after update, in %s. retrieved %s."), *childWidget->GetName(),
                                                       *modelInfo.ToString(), *modelInfo2.ToString());
            ret = MyErrorCodeCommonPartCpp::InternalError;
        }
    }

    return ret;
}

MyErrorCodeCommonPartCpp UMyRenderUtilsLibrary::evaluateUserWidgetInCanvasPanelForModelInfo_Widget2D(UUserWidget* childWidget, FMyModelInfoWidget2DCpp& modelInfo)
{
    modelInfo.reset(MyModelInfoType::BoxWidget2D);

    MyErrorCodeCommonPartCpp ret = checkUserWidgetInCanvasPanelComplyModelInfo_Base_Widget2D(childWidget);
    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        return ret;
    }

    FMyModelInfoBoxWidget2DCpp& modelInfoBox = modelInfo.getBox2DRef();


    UCanvasPanelSlot* pCPSlot = Cast<UCanvasPanelSlot>(childWidget->Slot);
    const FAnchorData& ad = pCPSlot->LayoutData;


    FVector2D boxExtend;
    boxExtend.X = ad.Offsets.Right / 2;
    boxExtend.Y = ad.Offsets.Bottom / 2;
 
    const FVector2D& originPercent = ad.Alignment;
    FVector2D originPointRelativeToXYMin = originPercent * (boxExtend * 2);

    modelInfoBox = getMyModelInfoBoxByOriginPointRelativeToXYMinWidget2D(boxExtend, originPointRelativeToXYMin);

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyRenderUtilsLibrary::updateUserWidgetInCanvasPanelWithPivot_Widget2D(UUserWidget* childWidget, const FVector2D& pivot)
{
    if (!childWidget->GetClass()->ImplementsInterface(UMyContentSizeWidget2DInterfaceCpp::StaticClass())) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: did not implements UMyContentSizeWidget2DInterfaceCpp."), *childWidget->GetName());
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    }

    FVector2D contentSize;
    MyErrorCodeCommonPartCpp ret = IMyContentSizeWidget2DInterfaceCpp::Execute_getContentSize(childWidget, contentSize);

    if (MyErrorCodeCommonPartCpp::NoError != ret) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getContentSize() got error: %s."), *childWidget->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
        return ret;
    }

    FMyModelInfoWidget2DCpp modelInfo(MyModelInfoType::BoxWidget2D);
    modelInfo.getBox2DRef() = getMyModelInfoBoxByOriginPointRelativeToXYMinWidget2D(contentSize / 2, contentSize * pivot);


    ret = updateUserWidgetInCanvasPanelWithModelInfo_Widget2D(childWidget, modelInfo);

    return ret;
}


void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupWalked, const FIntVector& elemWalked, FVector& distanceIgnorePadding)
{
    FIntVector groupDelimiterNumber, elemDelimiterNumber;
    myElemAndGroupCalcDelimiterNumber(groupWalked, elemWalked, groupDelimiterNumber, elemDelimiterNumber);

    distanceIgnorePadding = 
        FVector(groupDelimiterNumber) * meta.m_cGroupMarginPercent * meta.m_cElemSize +
        FVector(elemDelimiterNumber)  * meta.m_cCellMarginPercent * meta.m_cElemSize +
        FVector(elemWalked) * meta.m_cElemSize;
}

/*
void UMyRenderUtilsLibrary::myElem2DDynamicArrangeCalcTotalSize(const FMyElem2DDynamicArrangeMetaCpp& meta, FMyIntVector2D groupCount, FMyIntVector2D elemCount, FVector2D& totalSize)
{


    FMyIntVector2D groupDelim, elemDelim;



    totalSize = 
    meta.m_cAllMarginPercent.GetDesiredSize() * meta.m_cElemSize +
        groupDelim.ToVector2D() * meta.m_cGroupMarginPercent * meta.m_cElemSize +
        elemDelim.ToVector2D()  * meta.m_cCellMarginPercent * meta.m_cElemSize +
        elemCount.ToVector2D() * meta.m_cElemSize;
}

void UMyRenderUtilsLibrary::myElem2DDynamicArrangeGetElemCenterPosition(const FMyElem2DDynamicArrangeMetaCpp& meta, FMyIntVector2D idxGroup, FMyIntVector2D idxElem, FMyIntVector2D coordinateNegativeFlag, FVector2D& centerPosition)
{
}
*/