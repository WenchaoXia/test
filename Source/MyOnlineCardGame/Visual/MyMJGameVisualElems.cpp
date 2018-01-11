// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameVisualElems.h"

#include "Utils/CommonUtils/MyCommonUtilsLibrary.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Curves/CurveVector.h"

#include "Kismet/KismetMathLibrary.h"

#define MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT (64)

#define FTransformUpdateSequencDataCpp_Delta_Min (0.1f)

void FTransformUpdateSequencDataCpp::helperSetDataBySrcAndDst(const FTransform& cStart, const FTransform& cEnd, float fTime, int32 iLocalRollExtra, int32 iLocalPitchExtra, int32 iLocalYawExtra)
{
    m_cStart = cStart;
    m_cEnd = cEnd;
    m_fTime = fTime;

    m_cStart.NormalizeRotation();
    m_cEnd.NormalizeRotation();

    m_cLocalRotatorExtra.Roll = iLocalRollExtra * 360;
    m_cLocalRotatorExtra.Pitch = iLocalPitchExtra * 360;
    m_cLocalRotatorExtra.Yaw = iLocalYawExtra * 360;

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

    if (m_cEnd.GetLocation().Equals(m_cStart.GetLocation(), FTransformUpdateSequencDataCpp_Delta_Min)) {
        m_bLocationEnabledCache = false;
    }
    else {
        m_bLocationEnabledCache = true;
    }

    //since we allow roll at origin 360d, we can't use default isNealyZero() which treat that case zero
    if (relativeRota.Euler().IsNearlyZero(FTransformUpdateSequencDataCpp_Delta_Min)) {
        m_bRotatorBasicEnabledCache = false;
    }
    else {
        m_bRotatorBasicEnabledCache = true;
    }

    if (m_cLocalRotatorExtra.Euler().IsNearlyZero(FTransformUpdateSequencDataCpp_Delta_Min)) {
        m_bRotatorExtraEnabledCache = false;
    }
    else {
        m_bRotatorExtraEnabledCache = true;
    }

    if (m_cEnd.GetScale3D().Equals(m_cStart.GetScale3D(), FTransformUpdateSequencDataCpp_Delta_Min)) {
        m_bScaleEnabledCache = false;
    }
    else {
        m_bScaleEnabledCache = true;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%d, %d, %d, %d. r:  %s -> %s, relativeRota %s."), m_bLocationEnabledCache, m_bRotatorBasicEnabledCache, m_bRotatorExtraEnabledCache, m_bScaleEnabledCache,
    //         *cStart.GetRotation().Rotator().ToString(), *cEnd.GetRotation().Rotator().ToString(), *relativeRota.ToString());

};

UMyTransformUpdateSequenceMovementComponent::UMyTransformUpdateSequenceMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bUpdateOnlyIfRendered = false;

    bWantsInitializeComponent = true;
    bAutoActivate = false;
    SetTickableWhenPaused(false);

    m_cDataCycleBuffer.reinit(TEXT("transform update data Cycle buffer"), &m_aDataItems, &m_cDataMeta, MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT);
    m_cCurveCycleBuffer.reinit(TEXT("transform update curve Cycle buffer"), &m_aCurveItems, &m_cCurveMeta, MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT);

    m_cTimeLineVectorDelegate.BindUObject(this, &UMyTransformUpdateSequenceMovementComponent::onTimeLineUpdated);
    m_cTimeLineFinishEventDelegate.BindUObject(this, &UMyTransformUpdateSequenceMovementComponent::onTimeLineFinished);

    m_cTimeLine.Stop();

    m_fDebugTimeLineStartWorldTime = 0.f;

}


int32 UMyTransformUpdateSequenceMovementComponent::addSeqToTail(const FTransformUpdateSequencDataCpp& data, UCurveVector* curve)
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("addSeqToTail, rot %d, %s."), data.m_bRotatorEnabledCache, *data.m_cRotatorRelativeToStartDelta.ToString());

    if (!IsValid(curve)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("curve not valid %p."), curve);
        return -1;
    }

    int32 ret0, ret1;
    ret0 = m_cDataCycleBuffer.addToTail(&data, NULL);
    ret1 = m_cCurveCycleBuffer.addToTail(&curve, NULL);

    MY_VERIFY(ret0 == ret1);

    if (ret0 >= 0) {
        if (!m_cTimeLine.IsPlaying()) {
            tryStartNextSeq(TEXT("addSeqToTail trigger"));
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("addSeqToTail fail, ret %d."), ret0);
    }

    return ret0;
}

int32 UMyTransformUpdateSequenceMovementComponent::removeSeqFromHead(int32 iCount)
{
    int32 ret0, ret1;
    ret0 = m_cDataCycleBuffer.removeFromHead(iCount);
    ret1 = m_cCurveCycleBuffer.removeFromHead(iCount);

    MY_VERIFY(ret0 == ret1);

    if (getSeqCount() <= 0) {
        m_cTimeLine.Stop();
    }
    return ret0;
}

int32 UMyTransformUpdateSequenceMovementComponent::peekSeqAt(int32 idxFromHead, FTransformUpdateSequencDataCpp& outData, UCurveVector*& outCurve) const
{
    const FTransformUpdateSequencDataCpp* pData = NULL;
    UCurveVector* pCurve = NULL;
    int32 ret = peekSeqAtCpp(idxFromHead, pData, pCurve);

    if (ret >= 0) {
        MY_VERIFY(pData);
        outData = *pData;

        MY_VERIFY(pCurve);
        outCurve = pCurve;
    }

    return ret;
}

int32 UMyTransformUpdateSequenceMovementComponent::peekSeqAtCpp(int32 idxFromHead, const FTransformUpdateSequencDataCpp*& poutData, UCurveVector*& outCurve) const
{
    int32 ret0 = 0, ret1 = 0;
    poutData = m_cDataCycleBuffer.peekAt(idxFromHead, &ret0);
    UCurveVector* const * ppC = m_cCurveCycleBuffer.peekAt(idxFromHead, &ret1);
    if (ppC) {
        outCurve = *ppC;
    }
    else {
        outCurve = NULL;
    }

    MY_VERIFY(ret0 == ret1);
    return ret0;
}

int32 UMyTransformUpdateSequenceMovementComponent::peekSeqLast(FTransformUpdateSequencDataCpp& outData, UCurveVector*& outCurve) const
{
    int32 l = getSeqCount();
    if (l > 0) {
        return peekSeqAt(l - 1, outData, outCurve);
    }
    else {
        return -1;
    }
};

int32 UMyTransformUpdateSequenceMovementComponent::getSeqCount() const
{
    int32 ret0, ret1;
    ret0 = m_cDataCycleBuffer.getCount();
    ret1 = m_cCurveCycleBuffer.getCount();

    MY_VERIFY(ret0 == ret1);
    return ret0;
}

void UMyTransformUpdateSequenceMovementComponent::clearSeq()
{
    m_cDataCycleBuffer.clearInGame();
    m_cCurveCycleBuffer.clearInGame();

    MY_VERIFY(getSeqCount() == 0);
    m_cTimeLine.Stop();
}


//void UMyTransformUpdateSequenceMovementComponent::BeginPlay()
//{
//    Super::BeginPlay();
//
//}

void UMyTransformUpdateSequenceMovementComponent::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
    Super::ApplyWorldOffset(InOffset, bWorldShift);

}

bool UMyTransformUpdateSequenceMovementComponent::tryStartNextSeq(FString sDebugReason)
{
    if (m_cTimeLine.IsPlaying()) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("still playing, not start next seq now."));
        return false;
    }

    const FTransformUpdateSequencDataCpp* pData = NULL;
    UCurveVector* pCurve = NULL;
    
    if (peekSeqAtCpp(0, pData, pCurve) >= 0) {
        //we have data

        //the fuck is that: FTimeLine have NOT clear function for static bind type, so create new one
        FTimeline newTimeLine;
        m_cTimeLine = newTimeLine;

        MY_VERIFY(IsValid(pCurve));
        MY_VERIFY(pData);

        m_cTimeLine.AddInterpVector(pCurve, m_cTimeLineVectorDelegate);
        m_cTimeLine.SetTimelineFinishedFunc(m_cTimeLineFinishEventDelegate);

        float MinVal, MaxVal;
        pCurve->GetTimeRange(MinVal, MaxVal);

        m_cTimeLine.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
        m_cTimeLine.SetTimelineLength(MaxVal);

        m_cTimeLine.SetPlayRate(MaxVal / pData->m_fTime);

        m_cTimeLine.Play();

        setActivatedMyEncapped(true, sDebugReason + TEXT(", have new one"));

        //m_dDebugTimeLineStartRealTime = FPlatformTime::Seconds();
        UWorld *w = GetWorld();
        if (IsValid(w)) {
            m_fDebugTimeLineStartWorldTime = w->GetTimeSeconds();
        }
        else {
            MY_VERIFY(false);
        }

        return true;
    }
    else {
        //stop
        setActivatedMyEncapped(false, sDebugReason + TEXT(", no new seq."));
        return false;
    }
}

void UMyTransformUpdateSequenceMovementComponent::onTimeLineUpdated(FVector vector)
{
    const FTransformUpdateSequencDataCpp* pData = NULL;
    UCurveVector* pCurve = NULL;

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("onTimeLineUpdated."));

    if (peekSeqAtCpp(0, pData, pCurve) < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get data in the middle of timelineUpdate"));
        return;
    }

    MY_VERIFY(IsValid(pCurve));
    MY_VERIFY(pData);

    MY_VERIFY(IsValid(UpdatedComponent));

    if (pData->m_bLocationEnabledCache || pData->m_bRotatorBasicEnabledCache || pData->m_bRotatorExtraEnabledCache) {
        FVector MoveDelta = FVector::ZeroVector;
        FQuat NewQuat = UpdatedComponent->GetComponentRotation().Quaternion();

        if (pData->m_bLocationEnabledCache) {
            FVector NewLocation = UKismetMathLibrary::VLerp(pData->m_cStart.GetLocation(), pData->m_cEnd.GetLocation(), vector.X);
            FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
            if (NewLocation != CurrentLocation)
            {
                MoveDelta = NewLocation - CurrentLocation;
            }
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

        MoveUpdatedComponent(MoveDelta, NewQuat, false);
    }

    if (pData->m_bScaleEnabledCache) {
        FVector NewScale = UKismetMathLibrary::VLerp(pData->m_cStart.GetScale3D(), pData->m_cEnd.GetScale3D(),vector.Z);
        UpdatedComponent->SetWorldScale3D(NewScale);
    }

}

void UMyTransformUpdateSequenceMovementComponent::onTimeLineFinished()
{
    const FTransformUpdateSequencDataCpp* pData = NULL;
    UCurveVector* pCurve = NULL;
    if (peekSeqAtCpp(0, pData, pCurve) < 0) {
        MY_VERIFY(false);
        return;
    }
    FTransform cT = UpdatedComponent->GetComponentTransform();
    if (!cT.Equals(pData->m_cEnd, 1.0f)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time line finished but not equal: now %s. target %s."), *cT.ToString(), *pData->m_cEnd.ToString());
    }

    //fix up any defloat by direct set transform
    UpdatedComponent->SetWorldTransform(pData->m_cEnd);

    MY_VERIFY(IsValid(pCurve));
    MY_VERIFY(pData);
    //float minValue, maxValue, minTime, maxTime;
    //pCurve->GetValueRange(minValue, maxValue);
    //pCurve->GetTimeRange(minTime, maxTime);

    UWorld *w = GetWorld();
    if (IsValid(w)) {
        float fTimePassed = w->GetTimeSeconds() - m_fDebugTimeLineStartWorldTime;
        if (!UKismetMathLibrary::NearlyEqual_FloatFloat(pData->m_fTime, fTimePassed, 0.1)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time line finished but time not quite equal: supposed %f, used %f."), pData->m_fTime, fTimePassed);
        }

    }
    else {
        MY_VERIFY(false);
    }
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("onTimeLineFinished, time used %f, time range %f, %f, value range %f, %f."), s1 - m_dDebugTimeLineStartRealTime, minTime, maxTime, minValue, maxValue);

    removeSeqFromHead(1);
    m_cTimeLine.Stop();
    tryStartNextSeq(TEXT("pre timeline finish"));
}


void UMyTransformUpdateSequenceMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    //QUICK_SCOPE_CYCLE_COUNTER(STAT_InterpToMovementComponent_TickComponent);
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

    if (m_cTimeLine.IsPlaying()) {
        //Todo: handle the case that tick() and addSeq() happen in one loop, which cause 1 tick time defloat
        m_cTimeLine.TickTimeline(DeltaTime);
    }
    else {
        setActivatedMyEncapped(false, TEXT("in tick() timeline stopped."));
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("TickComponent out."));
}


AMyMoveWithSeqActorBaseCpp::AMyMoveWithSeqActorBaseCpp() : Super()
{
    bNetLoadOnClient = true;

    m_bFakeUpdateSettings = false;

    createComponentsForCDO();
}

AMyMoveWithSeqActorBaseCpp::~AMyMoveWithSeqActorBaseCpp()
{

}


void AMyMoveWithSeqActorBaseCpp::createComponentsForCDO()
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

    m_pTransformUpdateSequence = CreateDefaultSubobject<UMyTransformUpdateSequenceMovementComponent>(TEXT("transform update sequence movement component"));

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_pMainBox created as 0x%p, this 0x%p."), m_pMainBox, this);

}

int32 AMyMoveWithSeqActorBaseCpp::getModelInfo(FMyMJGameActorModelInfoBoxCpp& modelInfo) const
{
    //ignore the root scene/actor's scale, but calc from the box

    //FVector actorScale3D = GetActorScale3D();
    //m_pMainBox->GetScaledBoxExtent()
    modelInfo.m_cBoxExtend = m_pMainBox->GetUnscaledBoxExtent() *  m_pMainBox->GetComponentScale();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("name %s, box scale %s."), *GetName(), *m_pMainBox->GetComponentScale().ToString());
    modelInfo.m_cCenterPointRelativeLocation = m_pMainBox->RelativeLocation;// * actorScale3D;

    return 0;
};


#if WITH_EDITOR

void AMyMoveWithSeqActorBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyMoveWithSeqActorBaseCpp, m_bFakeUpdateSettings))
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_bFakeUpdateSettings clicked."));
        updateSettings();
    }
    else {
    }

    Super::PostEditChangeProperty(e);
}

#endif


int32 AMyMoveWithSeqActorBaseCpp::updateSettings()
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
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("updating mesh and material for this %p."), this);

        FBox meshBox = pMeshNow->GetBoundingBox();
        FVector meshBoxSize = meshBox.Max - meshBox.Min;

        boxSizeFix.X = UKismetMathLibrary::FCeil(meshBoxSize.X) / 2;
        boxSizeFix.Y = UKismetMathLibrary::FCeil(meshBoxSize.Y) / 2;
        boxSizeFix.Z = UKismetMathLibrary::FCeil(meshBoxSize.Z) / 2;


        meshOrigin.X = (meshBox.Min.X + meshBox.Max.X) / 2;
        meshOrigin.Y = (meshBox.Min.Y + meshBox.Max.Y) / 2;
        meshOrigin.Z = (meshBox.Min.Z + meshBox.Max.Z) / 2;

    }
    else {


    }

    m_pMainBox->SetRelativeLocation(FVector::ZeroVector);
    m_pMainBox->SetBoxExtent(boxSizeFix);

    m_pMainStaticMesh->SetRelativeLocation(-meshOrigin);

    return 0;
}

/*
void AMyMoveWithSeqActorBaseCpp::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnConstruction m_pMainBox 0x%p, this 0x%p."), m_pMainBox, this);
}
*/

AMyMJGameCardBaseCpp::AMyMJGameCardBaseCpp() : Super()
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = uc->GetDefaultObject();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCardBaseCpp, %s, this: %p, uc %s, cdo %p."), *m_cResPath.Path, this, *uc->GetFullName(), CDO);

    m_iValueShowing = MyMJGameCardBaseCppDefaultShowingValue;
    m_iValueUpdatedBefore = MyMJGameCardBaseCppDefaultShowingValue - 1;

    m_cResPath.Path.Reset();
    m_pResMesh = NULL;
    m_pResMI = NULL;
}

AMyMJGameCardBaseCpp::~AMyMJGameCardBaseCpp()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~AMyMJGameCardBaseCpp, this: %p."), this);
};

/* it has priority over default constructor, and we don't need it yet */

/*
AMyMJGameCardBaseCpp::AMyMJGameCardBaseCpp(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Initialize CDO properties here.
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCardBaseCpp construct 2, %s"), *m_sModelAssetPath);
}
*/

void AMyMJGameCardBaseCpp::OnConstruction(const FTransform& Transform)
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = uc->GetDefaultObject();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnConstruction, this %p, cdo %p."), this, CDO);
    Super::OnConstruction(Transform);

    updateVisual();
}

void AMyMJGameCardBaseCpp::PostInitializeComponents()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostInitializeComponents, this %p, %s, %p, %p, compo: %p."), this, *m_cResPath.Path, m_pResMesh, m_pResMI, m_pMainStaticMesh);
    Super::PostInitializeComponents();

    updateVisual();
}

#if WITH_EDITOR

#define MY_MODEL_RES_RELATIVE_PATH TEXT("Res")

void AMyMJGameCardBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyMJGameCardBaseCpp, m_iValueShowing))
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 1, this %p."), this);

        //this may invlove CDO, so update all, since CDO may have not executed onContruct() nor PostInitializeComponents() before

        updateVisual();
        //updateWithValue(m_iValueShowing);
    } 
    else {
        PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

        if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyMJGameCardBaseCpp, m_cResPath))
        {
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 2, this %p, %s"), this, *m_cResPath.Path);
            if (0 != checkAndLoadCardBasicResources(m_cResPath.Path)) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Retrying for default settings."));
                m_cResPath.Path = UMyCommonUtilsLibrary::getClassAssetPath(this->GetClass()) + TEXT("/") + MY_MODEL_RES_RELATIVE_PATH;
                checkAndLoadCardBasicResources(m_cResPath.Path);
            }
            updateVisual();
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif

int32 AMyMJGameCardBaseCpp::updateSettings()
{
    int32 ret = Super::updateSettings();
    if (ret != 0) {
        return ret;
    }

    FVector boxSize = m_pMainBox->GetScaledBoxExtent();
    FVector alignPoint = FVector::ZeroVector;

    alignPoint.X = -boxSize.X;
    alignPoint.Z = -boxSize.Z;

    m_pMainBox->SetRelativeLocation(-alignPoint);

    return 0;
}

int32 AMyMJGameCardBaseCpp::checkAndLoadCardBasicResources(const FString &inPath)
{
    int32 ret = 0;
    if (inPath.IsEmpty()) {
        m_pResMesh = nullptr;
        m_pResMI = nullptr;
        m_cResPath.Path.Reset();

        return 0;
    }


    const FString &modelAssetPath = inPath;

    FString meshFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMesh;
    FString matDefaultInstFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMeshDefaultMI;

    UStaticMesh *pMeshAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(NULL, meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load mesh asset from %s."), *meshFullPathName);
        m_pResMesh = nullptr;
        ret |= 0x01;
    }
    else {
        m_pResMesh = pMeshAsset;
    }

    UMaterialInstance *pMatInstAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load material default instance asset from %s."), *matDefaultInstFullPathName);
        m_pResMI = nullptr;
        ret |= 0x10;
    }
    else {
        m_pResMI = pMatInstAsset;
    }

    m_cResPath.Path = modelAssetPath;

    return ret;
}

int32 AMyMJGameCardBaseCpp::updateVisual()
{
    //if (m_cResPath.Path.IsEmpty()) {
        //return -1;
    //}

    updateWithCardBasicResources();
    updateWithValue();

    return 0;
}

int32 AMyMJGameCardBaseCpp::updateWithCardBasicResources()
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
    if (pMeshNow != m_pResMesh) {
        //only update when mesh changes, give BP a chance to adjust them manually
        m_pMainStaticMesh->SetStaticMesh(m_pResMesh);
        updateSettings();
    }


    UMaterialInstanceDynamic* pMIDNow = Cast<UMaterialInstanceDynamic>(m_pMainStaticMesh->GetMaterial(0));
    if (IsValid(m_pResMI)) {
        if (IsValid(pMIDNow) && pMIDNow->Parent == m_pResMI) {
            //equal
        }
        else {
            UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(m_pResMI, m_pMainStaticMesh);
            MY_VERIFY(IsValid(pMID));
            m_pMainStaticMesh->SetMaterial(0, pMID);
        }
    }
    else {
        //simple, target is to clear
        if (!IsValid(pMIDNow)) {
            //equal
        }
        else {
            m_pMainStaticMesh->SetMaterial(0, nullptr);
        }
    }
 
    return 0;
}

int32 AMyMJGameCardBaseCpp::updateWithValue()
{
    if (m_iValueUpdatedBefore == m_iValueShowing) {
        return 0;
    }
    m_iValueUpdatedBefore = m_iValueShowing;

    int32 newValue = m_iValueShowing;
    UTexture* pTargetBaseColorTexture = NULL;

    FString vPrefix;
    if (!m_cResPath.Path.IsEmpty()) {
        if (UMyMJUtilsLibrary::getCardValueType(newValue) != MyMJCardValueTypeCpp::Invalid || newValue == 0) {
            if (newValue > 0) {
                vPrefix = FString::Printf(MyCardAssetPartialNamePrefixValueNormal, newValue);
            }
            else if (newValue == 0) {
                vPrefix = MyCardAssetPartialNamePrefixValueUnknown;
            }
        }

    }

    if (!vPrefix.IsEmpty()) {
        helperTryLoadCardRes(m_cResPath.Path, vPrefix, &pTargetBaseColorTexture);
    }

    return updateCardStaticMeshMIDParams(pTargetBaseColorTexture);
}

int32 AMyMJGameCardBaseCpp::updateCardStaticMeshMIDParams(class UTexture* InBaseColor)
{
    MY_VERIFY(IsValid(m_pMainStaticMesh));

    UMaterialInterface* pMat = m_pMainStaticMesh->GetMaterial(0);
    if (IsValid(pMat)) {
        UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(pMat);
        if (!DynamicMaterial)
        {
            //our design need MID in any case
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cast to dynamic material instance fail: this %p, %s, %s."), this, *pMat->GetClass()->GetFullName(), *pMat->GetFullName());
            if (InBaseColor == nullptr) {
                return 0;
            }
            else {
                return -30;
            }
        }

        class UTexture* baseColorNow = NULL;
        if (DynamicMaterial->GetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, baseColorNow)) {
            if (baseColorNow == InBaseColor) {
                //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("skip SetTextureParameterValue since target is same, this %p."), this);
                return 0;
            }
        }
        
        //if (InBaseColor == nullptr) {
            //test
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("doing test."));
            //m_pMainStaticMesh->SetMaterial(0, nullptr);
            //return 0;
        //}

        DynamicMaterial->SetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, InBaseColor);
        return 0;
    }
    else {
        if (InBaseColor == nullptr) {
            return 0;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("try to set MID param but material is NULL."));
            return -31;
        }
    }
};

void AMyMJGameCardBaseCpp::setValueShowing(int32 newValue)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setValueShowing %d."), newValue);

    if (m_iValueShowing == newValue) {
        return;
    }
    m_iValueShowing = newValue;

    updateWithValue();
}

int32 AMyMJGameCardBaseCpp::getValueShowing() const
{
    return m_iValueShowing;
}

void AMyMJGameCardBaseCpp::setResPath(const FDirectoryPath& newResPath)
{
    setResPathWithRet(newResPath);
}

bool AMyMJGameCardBaseCpp::setResPathWithRet(const FDirectoryPath& newResPath)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setResPath %s."), *newResPath.Path);

    if (m_cResPath.Path == newResPath.Path) {
        return true;
    }

    bool ret = true;
    FDirectoryPath oldPath = m_cResPath;
    m_cResPath = newResPath;

    if (0 == checkAndLoadCardBasicResources(m_cResPath.Path))
    {
    }
    else {
        m_cResPath = oldPath;
        ret = false;
    }

    updateVisual();

    return ret;
}

const FDirectoryPath& AMyMJGameCardBaseCpp::getResPath() const
{
    return m_cResPath;
}

bool AMyMJGameCardBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture)
{
    bool bRet = true;

    if (ppOutBaseColorTexture) {
        FString baseColorFullPathName = modelAssetPath + TEXT("/") + valuePrefix + MyCardAssetPartialNameSuffixValueBaseColorTexture;
        UTexture* pTBaseColor = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutBaseColorTexture = pTBaseColor;
        }
        else {
            *ppOutBaseColorTexture = NULL;
            bRet = false;
        }
    }

    return bRet;
};


AMyMJGameTrivalDancingActorBaseCpp::AMyMJGameTrivalDancingActorBaseCpp() : Super()
{
};

AMyMJGameTrivalDancingActorBaseCpp::~AMyMJGameTrivalDancingActorBaseCpp()
{
};