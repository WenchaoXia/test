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


void FMyWithCurveUpdateStepDataWorldTransformCpp::helperSetDataBySrcAndDst(float fTime, UCurveVector* pCurve, const FTransform& cStart, const FTransform& cEnd, FIntVector extraRotateCycle)
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

    if (m_cEnd.GetLocation().Equals(m_cStart.GetLocation(), FMyWithCurveUpdateStepDataWorldTransformCpp_Delta_Min)) {
        m_bLocationEnabledCache = false;
    }
    else {
        m_bLocationEnabledCache = true;
    }

    //since we allow roll at origin 360d, we can't use default isNealyZero() which treat that case zero
    if (relativeRota.Euler().IsNearlyZero(FMyWithCurveUpdateStepDataWorldTransformCpp_Delta_Min)) {
        m_bRotatorBasicEnabledCache = false;
    }
    else {
        m_bRotatorBasicEnabledCache = true;
    }

    if (m_cLocalRotatorExtra.Euler().IsNearlyZero(FMyWithCurveUpdateStepDataWorldTransformCpp_Delta_Min)) {
        m_bRotatorExtraEnabledCache = false;
    }
    else {
        m_bRotatorExtraEnabledCache = true;
    }

    if (m_cEnd.GetScale3D().Equals(m_cStart.GetScale3D(), FMyWithCurveUpdateStepDataWorldTransformCpp_Delta_Min)) {
        m_bScaleEnabledCache = false;
    }
    else {
        m_bScaleEnabledCache = true;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%d, %d, %d, %d. r:  %s -> %s, relativeRota %s."), m_bLocationEnabledCache, m_bRotatorBasicEnabledCache, m_bRotatorExtraEnabledCache, m_bScaleEnabledCache,
    //         *cStart.GetRotation().Rotator().ToString(), *cEnd.GetRotation().Rotator().ToString(), *relativeRota.ToString());

};



UMyWithCurveUpdaterWorldTransformComponent::UMyWithCurveUpdaterWorldTransformComponent(const FObjectInitializer& ObjectInitializer)
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

    m_cUpdater.m_cCommonUpdateDelegate.BindUObject(this, &UMyWithCurveUpdaterWorldTransformComponent::updaterOnCommonUpdate);
    m_cUpdater.m_cCommonFinishDelegete.BindUObject(this, &UMyWithCurveUpdaterWorldTransformComponent::updaterOnCommonFinish);
    m_cUpdater.m_cActivateTickDelegate.BindUObject(this, &UMyWithCurveUpdaterWorldTransformComponent::updaterActivateTick);
}

void UMyWithCurveUpdaterWorldTransformComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
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

void UMyWithCurveUpdaterWorldTransformComponent::updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataWorldTransformCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataWorldTransformCpp*>(&data);
    MY_VERIFY(pData);

    MY_VERIFY(IsValid(UpdatedComponent));

    if (pData->m_bLocationEnabledCache || pData->m_bRotatorBasicEnabledCache || pData->m_bRotatorExtraEnabledCache) {
        FVector MoveDelta = FVector::ZeroVector;
        FQuat NewQuat = UpdatedComponent->GetComponentRotation().Quaternion();
        FVector NewLocation = UpdatedComponent->GetComponentLocation();
        if (pData->m_bLocationEnabledCache) {
            NewLocation = UKismetMathLibrary::VLerp(pData->m_cStart.GetLocation(), pData->m_cEnd.GetLocation(), vector.X);
            UpdatedComponent->SetWorldLocation(NewLocation);
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
                return;
            }

            NewQuat = r.Quaternion();
            if (NewQuat.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *NewQuat.ToString());
                return;
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
                return;
            }

            FQuat q = r.Quaternion();
            if (q.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *q.ToString());
                return;
            }

            NewQuat = q * NewQuat;
            if (NewQuat.ContainsNaN())
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got invalid value, %s."), *NewQuat.ToString());
                return;
            }
        }

        UpdatedComponent->SetWorldRotation(NewQuat);
        //MoveUpdatedComponent(MoveDelta, NewQuat, false);
    }

    if (pData->m_bScaleEnabledCache) {
        FVector NewScale = UKismetMathLibrary::VLerp(pData->m_cStart.GetScale3D(), pData->m_cEnd.GetScale3D(), vector.Z);
        UpdatedComponent->SetWorldScale3D(NewScale);
    }

}

void UMyWithCurveUpdaterWorldTransformComponent::updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    const FMyWithCurveUpdateStepDataWorldTransformCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataWorldTransformCpp*>(&data);
    MY_VERIFY(pData);

    MY_VERIFY(IsValid(UpdatedComponent));

    FTransform cT = UpdatedComponent->GetComponentTransform();
    if (!cT.Equals(pData->m_cEnd, 1.0f)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time line finished but not equal: now %s. target %s."), *cT.ToString(), *pData->m_cEnd.ToString());
    }

    UpdatedComponent->SetWorldTransform(pData->m_cEnd);
}

void UMyWithCurveUpdaterWorldTransformComponent::updaterActivateTick(bool bNew, FString reason)
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



AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp() : Super()
{
    bNetLoadOnClient = true;

    m_bFakeUpdateSettings = false;

    createComponentsForCDO();
}

AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::~AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp()
{

}


//Todo:: verify its correctness when root scene scaled
MyErrorCodeCommonPartCpp AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::getModelInfo(FMyModelInfoCpp& modelInfo, bool verify) const
{
    //ignore the root scene/actor's scale, but calc from the box

    //FVector actorScale3D = GetActorScale3D();
    //m_pMainBox->GetScaledBoxExtent()
    modelInfo.reset(MyModelInfoType::Box3D);
    modelInfo.getBox3DRef().m_cBoxExtend = m_pMainBox->GetUnscaledBoxExtent() *  m_pMainBox->GetComponentScale();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("name %s, box scale %s."), *GetName(), *m_pMainBox->GetComponentScale().ToString());
    modelInfo.getBox3DRef().m_cCenterPointRelativeLocation = m_pMainBox->RelativeLocation;// * actorScale3D;

    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
    if (modelInfo.getBox3DRef().m_cBoxExtend.Size() < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class %s: model size is too small: %s."), *this->GetClass()->GetName(), *modelInfo.getBox3DRef().m_cBoxExtend.ToString());
        ret = MyErrorCodeCommonPartCpp::ModelSizeIncorrect;
    }

    if (verify) {
        MY_VERIFY(ret == MyErrorCodeCommonPartCpp::NoError);
    }

    return ret;
}

MyErrorCodeCommonPartCpp AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterBasicCpp*& outUpdater)
{
    MY_VERIFY(IsValid(m_pMyTransformUpdaterComponent));

    outUpdater = &m_pMyTransformUpdaterComponent->getMyWithCurveUpdaterTransformRef();

    return MyErrorCodeCommonPartCpp::NoError;
}

void AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::createComponentsForCDO()
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

    m_pMyTransformUpdaterComponent = CreateDefaultSubobject<UMyWithCurveUpdaterWorldTransformComponent>(TEXT("transform updater component"));

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_pMainBox created as 0x%p, this 0x%p."), m_pMainBox, this);

}


#if WITH_EDITOR

void AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp, m_bFakeUpdateSettings))
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_bFakeUpdateSettings clicked."));
        updateSettings();
    }
    else {
    }

    Super::PostEditChangeProperty(e);
}

#endif


MyErrorCodeCommonPartCpp AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::updateSettings()
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


void AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::helperTestAnimationStep(float time, FString debugStr, const TArray<AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp*>& actors)
{
    FMyTransformUpdateAnimationMetaWorldTransformCpp meta;
    FMyWithCurveUpdateStepSettingsWorldTransformCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = time;
    meta.m_cModelInfo.reset(MyModelInfoType::Box3D);
    meta.m_cModelInfo.getBox3DRef().m_cBoxExtend = FVector(20, 30, 60);

    stepData.m_fTimePercent = 1;
    stepData.m_eLocationUpdateType = MyWithCurveUpdateStepSettingsLocationType::OffsetFromPrevLocation;
    stepData.m_cLocationOffsetPercent = FVector(0, 0, 1);

    TArray<FMyWithCurveUpdaterBasicCpp *> updaterSortedGroup;

    for (int32 i = 0; i < actors.Num(); i++)
    {
        FMyWithCurveUpdaterWorldTransformCpp* pUpdater = &actors[i]->getMyWithCurveUpdaterWorldTransformRef();
        FTransform targetT;
        targetT.SetLocation(FVector(0, 0, 100));
        pUpdater->setHelperTransformOrigin(actors[i]->GetActorTransform());
        pUpdater->setHelperTransformFinal(targetT);
        updaterSortedGroup.Emplace(pUpdater);
    }

    UMyRenderUtilsLibrary::helperSetupTransformUpdateAnimationStep(meta, stepData, updaterSortedGroup);
}

/*
void AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp::OnConstruction(const FTransform& Transform)
{
Super::OnConstruction(Transform);

UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnConstruction m_pMainBox 0x%p, this 0x%p."), m_pMainBox, this);
}
*/


void UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (m_bUpdaterNeedTick) {
        m_cUpdater.tick(InDeltaTime);
    }
}

void UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataWidgetTransformCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataWidgetTransformCpp*>(&data);
    MY_VERIFY(pData);

    FWidgetTransform wtNow;
    wtNow.Translation = FMath::Vector2DInterpTo(pData->m_cWidgetTransformStart.Translation, pData->m_cWidgetTransformEnd.Translation, vector.X, 1);

    wtNow.Angle = FMath::FInterpTo(pData->m_cWidgetTransformStart.Angle, pData->m_cWidgetTransformEnd.Angle, vector.Y, 1);
    wtNow.Shear = FMath::Vector2DInterpTo(pData->m_cWidgetTransformStart.Shear, pData->m_cWidgetTransformEnd.Shear, vector.Y, 1);

    wtNow.Scale = FMath::Vector2DInterpTo(pData->m_cWidgetTransformStart.Scale, pData->m_cWidgetTransformEnd.Scale, vector.Z, 1);

    SetRenderTransform(wtNow);
}

void UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    const FMyWithCurveUpdateStepDataWidgetTransformCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataWidgetTransformCpp*>(&data);
    MY_VERIFY(pData);

    SetRenderTransform(pData->m_cWidgetTransformEnd);
}

void UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::updaterActivateTick(bool activate, FString debugString)
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
    m_bEnableWidgetTransformPressed = false;
    m_cWidgetTransformPressed.Scale.X = m_cWidgetTransformPressed.Scale.Y = 0.8;
}

void UMyButton::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);

    FName PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, RenderTransform))
    {
        m_cWidgetTransformNormal = RenderTransform;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, m_cWidgetTransformNormal))
    {
        RenderTransform = m_cWidgetTransformNormal;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, WidgetStyle))
    {
        if (WidgetStyle.Normal.HasUObject()) {
            //helper set unused styles
            if (!WidgetStyle.Pressed.HasUObject()) {
                WidgetStyle.Pressed = WidgetStyle.Normal;
            }
            if (!WidgetStyle.Hovered.HasUObject()) {
                WidgetStyle.Hovered = WidgetStyle.Normal;
            }
            if (!WidgetStyle.Disabled.HasUObject()) {
                WidgetStyle.Disabled = WidgetStyle.Normal;
                WidgetStyle.Disabled.SetResourceObject(NULL);
            }
        }
        RenderTransform = m_cWidgetTransformNormal;
    }


    Super::PostEditChangeProperty(e);
}


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
    if (m_bEnableWidgetTransformPressed) {
        SetRenderTransform(m_cWidgetTransformPressed);
    }

    SlateHandlePressed();

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("pressed: %s"), *genStyleString());
}

void UMyButton::SlateHandleReleasedMy()
{
    if (m_bEnableWidgetTransformPressed) {
        SetRenderTransform(m_cWidgetTransformNormal);
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
    SetRenderTransform(m_cWidgetTransformPressed);
    OnPressedOverride.Broadcast();
}

void UMyButton::onReleasedInner()
{
    SetRenderTransform(m_cWidgetTransformNormal);
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



void UMyRenderUtilsLibrary::helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
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
    UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByAbsolute(WorldContextObject, popPoint, vAbsoluteOnScreen, targetModelHeightInWorld, outTargetTranform, cameraCenterLoc, cameraCenterDir);
}

float UMyRenderUtilsLibrary::helperGetRemainTimePercent(const TArray<FMyWithCurveUpdateStepSettingsWorldTransformCpp>& stepDatas)
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


void UMyRenderUtilsLibrary::helperSetupWorldTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaWorldTransformCpp& meta,
                                                                         const FMyWithCurveUpdateStepSettingsWorldTransformCpp& stepData,
                                                                         const TArray<FMyWithCurveUpdaterWorldTransformCpp *>& updatersSorted)
{
    float fTotalTime = meta.m_fTotalTime;

    const FTransform& pointTransform = meta.m_cPointTransform;
    const FTransform& disappearTransform = meta.m_cDisappearTransform;
    const FVector& modelBoxExtend = meta.m_cModelInfo.getBox3DRefConst().m_cBoxExtend;

    const FMyWithCurveUpdateStepSettingsWorldTransformCpp& cStepData = stepData;

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
    //FMyWithCurveUpdaterWorldTransformCpp* updaterCenter = updatersSorted[idxCenter];
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

        FMyWithCurveUpdaterWorldTransformCpp *pUpdater = updatersSorted[i];

        FMyWithCurveUpdateStepDataWorldTransformCpp data;
        data.helperSetDataBySrcAndDst(fDur, pCurve, pUpdater->getHelperTransformPrevRefConst(), aNextTransforms[i], cStepData.m_cRotationUpdateExtraCycles);
        pUpdater->addStepToTail(data);
    }
};

void UMyRenderUtilsLibrary::helperSetupTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaBaseCpp& meta,
                                                                    const FMyWithCurveUpdateStepSettingsBasicCpp& stepData,
                                                                    const TArray<FMyWithCurveUpdaterBasicCpp *>& updatersSorted)
{
    if (meta.getType() == MyTransformTypeWorldTransform) {
        const FMyTransformUpdateAnimationMetaWorldTransformCpp& metaWorldTransform = FMyTransformUpdateAnimationMetaWorldTransformCpp::castFromBaseRefConst(meta);
        const FMyWithCurveUpdateStepSettingsWorldTransformCpp& stepDataWorldTransform = FMyWithCurveUpdateStepSettingsWorldTransformCpp::castFromBaseConst(stepData);

        TArray<FMyWithCurveUpdaterWorldTransformCpp *> updatersWorldTransformSorted;
        for (int32 i = 0; i < updatersSorted.Num(); i++) {
            FMyWithCurveUpdaterWorldTransformCpp* pUpdater = &FMyWithCurveUpdaterWorldTransformCpp::castFromBaseRef(*updatersSorted[i]);
            updatersWorldTransformSorted.Emplace(pUpdater);
        }

        helperSetupWorldTransformUpdateAnimationStep(metaWorldTransform, stepDataWorldTransform, updatersWorldTransformSorted);
    }
    else if  (meta.getType() == MyTransformTypeWidgetTransform) {
    }
    else {
        //Todo
    }
};


void UMyRenderUtilsLibrary::helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaBaseCpp& meta,
                                                                     const TArray<const FMyWithCurveUpdateStepSettingsBasicCpp *>& stepDatas,
                                                                     const TArray<FMyWithCurveUpdaterBasicCpp *>& updatersSorted)
{
    int32 l = stepDatas.Num();

    if (l <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("stepDatas num is zero, debugStr: %s."), *meta.m_sDebugString);
        return;
    }

    float total = 0;

    bool bTimePecentTotalExpectedNot100Pecent = false;

    for (int32 i = 0; i < l; i++) {
        helperSetupTransformUpdateAnimationStep(meta, *stepDatas[i], updatersSorted);
        float f = stepDatas[i]->m_fTimePercent;
        total += f;

        bTimePecentTotalExpectedNot100Pecent |= stepDatas[i]->m_bTimePecentTotalExpectedNot100Pecent;
    }

    if ((!bTimePecentTotalExpectedNot100Pecent) && l > 0 && !FMath::IsNearlyEqual(total, 1, MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("total time is not 100%, str %s. now it is %f."), *meta.m_sDebugString, total);
    };
}

void UMyRenderUtilsLibrary::helperSetupWorldTransformUpdateAnimationStepsForPoint(const UObject* WorldContextObject,
                                                                                    float totalDur,
                                                                                    const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                                    const FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                                    const TArray<FMyWithCurveUpdateStepSettingsWorldTransformCpp>& stepDatas,
                                                                                    float extraDelayDur,
                                                                                    const TArray<IMyWithCurveUpdaterTransformInterfaceCpp *>& updaterInterfaces,
                                                                                    FString debugName,
                                                                                    bool clearPrevSteps)
{
    FMyTransformUpdateAnimationMetaWorldTransformCpp meta, *pMeta = &meta;
    TArray<FMyWithCurveUpdaterBasicCpp *> aUpdaters, *pUpdaters = &aUpdaters;

    //prepare meta
    pMeta->m_sDebugString = debugName;
    pMeta->m_fTotalTime = totalDur;
    if (updaterInterfaces.Num() <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("updaterInterfaces num is zero."));
        return;
    }

    updaterInterfaces[0]->getModelInfo(pMeta->m_cModelInfo);

    helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, pointInfo.m_fShowPosiFromCenterToBorderPercent, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelInfo.getBox3DRefConst().m_cBoxExtend.Size() * 2, pMeta->m_cPointTransform);
    helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, 1.2, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelInfo.getBox3DRefConst().m_cBoxExtend.Size() * 2, pMeta->m_cDisappearTransform);

    bool bDelay = extraDelayDur >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT;

    pUpdaters->Reset();
    int32 l = updaterInterfaces.Num();
    for (int32 i = 0; i < l; i++) {
        FMyWithCurveUpdaterBasicCpp* pUpdater = &updaterInterfaces[i]->getMyWithCurveUpdaterTransformRef();
        pUpdaters->Emplace(pUpdater);
        if (clearPrevSteps) {
            pUpdater->clearSteps();
        }
    }

    if (bDelay) {
        UMyRenderUtilsLibrary::helperAddWaitStep(extraDelayDur, debugName + TEXT(" wait"), *pUpdaters);
    }

    TArray<const FMyWithCurveUpdateStepSettingsBasicCpp *> stepDatasBase;
    stepDatasBase.Reserve(stepDatas.Num());
    for (int32 i = 0; i < stepDatas.Num(); i++) {
        stepDatasBase.Emplace(&stepDatas[i]);
    }

    UMyRenderUtilsLibrary::helperSetupTransformUpdateAnimationSteps(meta, stepDatasBase, *pUpdaters);
}

void UMyRenderUtilsLibrary::helperAddWaitStep(float waitTime, FString debugStr, const TArray<FMyWithCurveUpdaterBasicCpp *>& updaters)
{
    FMyTransformUpdateAnimationMetaWorldTransformCpp meta;
    FMyWithCurveUpdateStepSettingsWorldTransformCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = waitTime;

    stepData.m_fTimePercent = 1;


    helperSetupTransformUpdateAnimationStep(meta, stepData, updaters);
}

void UMyRenderUtilsLibrary::helperAddWaitStep(float waitTime, FString debugStr, const TArray<IMyWithCurveUpdaterTransformInterfaceCpp *>& updaterInterfaces)
{

    TArray<FMyWithCurveUpdaterBasicCpp *> aUpdaters;

    for (int32 i = 0; i < updaterInterfaces.Num(); i++)
    {
        aUpdaters.Emplace(&updaterInterfaces[i]->getMyWithCurveUpdaterTransformRef());
    }

    UMyRenderUtilsLibrary::helperAddWaitStep(waitTime, debugStr, aUpdaters);
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
    FVector2D widgetSize = FVector2D(layoutData.Offsets.Right, layoutData.Offsets.Bottom);
    FVector2D widgetCenterOffsetFromAnchorStartPosi = (FVector2D(0.5, 0.5) - layoutData.Alignment) * widgetSize;

    positionInCanvas = anchorStartPosi + widgetCenterOffsetFromAnchorStartPosi;

    return 0;
}


void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeCalcTotalSize(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupCount, const FIntVector& elemCount, FVector& totalSize)
{
    FVector distanceIgnorePadding;
    myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(meta, groupCount, elemCount, distanceIgnorePadding);

    totalSize = meta.m_cPaddingPercent.GetDesiredSize3D() * meta.m_cElemSize + distanceIgnorePadding;
}

void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeGetElemCenterPositionArrangeDirectionAllPositive(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& idxGroup, const FIntVector& idxElem, FVector& centerPosition)
{
    FVector distanceIgnorePadding;
    myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(meta, idxGroup, idxElem, distanceIgnorePadding);

    FVector distanceCenterIgnorePadding = distanceIgnorePadding + meta.m_cElemSize / 2;

    //direction is from negtive to positve
    FVector sourcePaddingPecent(meta.m_cPaddingPercent.Left, meta.m_cPaddingPercent.Top, meta.m_cPaddingPercent.ZNegative);

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

void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupWalked, const FIntVector& elemWalked, FVector& distanceIgnorePadding)
{
    FIntVector groupDelimiterNumber, elemDelimiterNumber;
    myElemAndGroupCalcDelimiterNumber(groupWalked, elemWalked, groupDelimiterNumber, elemDelimiterNumber);

    distanceIgnorePadding = 
        FVector(groupDelimiterNumber) * meta.m_cGroupSpacingPercent * meta.m_cElemSize +
        FVector(elemDelimiterNumber)  * meta.m_cElemSpacingPercent * meta.m_cElemSize +
        FVector(elemWalked) * meta.m_cElemSize;
}

/*
void UMyRenderUtilsLibrary::myElem2DDynamicArrangeCalcTotalSize(const FMyElem2DDynamicArrangeMetaCpp& meta, FMyIntVector2D groupCount, FMyIntVector2D elemCount, FVector2D& totalSize)
{


    FMyIntVector2D groupDelim, elemDelim;



    totalSize = 
    meta.m_cPaddingPercent.GetDesiredSize() * meta.m_cElemSize +
        groupDelim.ToVector2D() * meta.m_cGroupSpacingPercent * meta.m_cElemSize +
        elemDelim.ToVector2D()  * meta.m_cElemSpacingPercent * meta.m_cElemSize +
        elemCount.ToVector2D() * meta.m_cElemSize;
}

void UMyRenderUtilsLibrary::myElem2DDynamicArrangeGetElemCenterPosition(const FMyElem2DDynamicArrangeMetaCpp& meta, FMyIntVector2D idxGroup, FMyIntVector2D idxElem, FMyIntVector2D coordinateNegativeFlag, FVector2D& centerPosition)
{
}
*/