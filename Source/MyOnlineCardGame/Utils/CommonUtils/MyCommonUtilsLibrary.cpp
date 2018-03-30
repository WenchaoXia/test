// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCommonUtilsLibrary.h"

#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
//#include "Kismet/KismetRenderingLibrary.h"
//#include "Classes/PaperSprite.h"

#define MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT (64)

FMyWithCurveUpdaterBasicCpp::FMyWithCurveUpdaterBasicCpp()
{
    m_cStepDataItemsCycleBuffer.reinit(TEXT("step data items cycle buffer"), NULL, NULL, MY_TRANSFORM_UPDATE_CYCLE_BUFFER_COUNT);

    m_cTimeLineVectorDelegate.BindRaw(this, &FMyWithCurveUpdaterBasicCpp::onTimeLineUpdated); //binding to parent, should be safe
    m_cTimeLineFinishEventDelegate.BindRaw(this, &FMyWithCurveUpdaterBasicCpp::onTimeLineFinished);

    m_cTimeLine.Stop();
    reset();
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


void FMyWithCurveUpdateStepDataTransformCpp::helperSetDataBySrcAndDst(float fTime, UCurveVector* pCurve, const FTransform& cStart, const FTransform& cEnd, FIntVector extraRotateCycle)
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

    if (m_cEnd.GetLocation().Equals(m_cStart.GetLocation(), FMyWithCurveUpdateStepDataTransformCpp_Delta_Min)) {
        m_bLocationEnabledCache = false;
    }
    else {
        m_bLocationEnabledCache = true;
    }

    //since we allow roll at origin 360d, we can't use default isNealyZero() which treat that case zero
    if (relativeRota.Euler().IsNearlyZero(FMyWithCurveUpdateStepDataTransformCpp_Delta_Min)) {
        m_bRotatorBasicEnabledCache = false;
    }
    else {
        m_bRotatorBasicEnabledCache = true;
    }

    if (m_cLocalRotatorExtra.Euler().IsNearlyZero(FMyWithCurveUpdateStepDataTransformCpp_Delta_Min)) {
        m_bRotatorExtraEnabledCache = false;
    }
    else {
        m_bRotatorExtraEnabledCache = true;
    }

    if (m_cEnd.GetScale3D().Equals(m_cStart.GetScale3D(), FMyWithCurveUpdateStepDataTransformCpp_Delta_Min)) {
        m_bScaleEnabledCache = false;
    }
    else {
        m_bScaleEnabledCache = true;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%d, %d, %d, %d. r:  %s -> %s, relativeRota %s."), m_bLocationEnabledCache, m_bRotatorBasicEnabledCache, m_bRotatorExtraEnabledCache, m_bScaleEnabledCache,
    //         *cStart.GetRotation().Rotator().ToString(), *cEnd.GetRotation().Rotator().ToString(), *relativeRota.ToString());

};



UMyTransformUpdaterComponent::UMyTransformUpdaterComponent(const FObjectInitializer& ObjectInitializer)
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

    m_cUpdater.m_cCommonUpdateDelegate.BindUObject(this, &UMyTransformUpdaterComponent::updaterOnCommonUpdate);
    m_cUpdater.m_cCommonFinishDelegete.BindUObject(this, &UMyTransformUpdaterComponent::updaterOnCommonFinish);
    m_cUpdater.m_cActivateTickDelegate.BindUObject(this, &UMyTransformUpdaterComponent::updaterActivateTick);
}

void UMyTransformUpdaterComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
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

void UMyTransformUpdaterComponent::updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataTransformCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformCpp*>(&data);
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

void UMyTransformUpdaterComponent::updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    const FMyWithCurveUpdateStepDataTransformCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataTransformCpp*>(&data);
    MY_VERIFY(pData);

    MY_VERIFY(IsValid(UpdatedComponent));

    FTransform cT = UpdatedComponent->GetComponentTransform();
    if (!cT.Equals(pData->m_cEnd, 1.0f)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time line finished but not equal: now %s. target %s."), *cT.ToString(), *pData->m_cEnd.ToString());
    }

    UpdatedComponent->SetWorldTransform(pData->m_cEnd);
}

void UMyTransformUpdaterComponent::updaterActivateTick(bool bNew, FString reason)
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


void FMyLocationOfZRotationAroundPointCoordinateCpp::interp(const FMyLocationOfZRotationAroundPointCoordinateCpp& start, const FMyLocationOfZRotationAroundPointCoordinateCpp& end, float percent, FMyLocationOfZRotationAroundPointCoordinateCpp& result)
{
    float percentFixed = FMath::Clamp<float>(percent, 0, 1);
    result = start + (end - start) * percentFixed;
}

void FMyTransformOfZRotationAroundPointCoordinateCpp::interp(const FMyTransformOfZRotationAroundPointCoordinateCpp& start, const FMyTransformOfZRotationAroundPointCoordinateCpp& end, float percent, FMyTransformOfZRotationAroundPointCoordinateCpp& result)
{
    float percentFixed = FMath::Clamp<float>(percent, 0, 1);

    FMyLocationOfZRotationAroundPointCoordinateCpp::interp(start.m_cLocation, end.m_cLocation, percentFixed, result.m_cLocation);

    result.m_cRotatorOffsetFacingCenterPoint = UKismetMathLibrary::RLerp(start.m_cRotatorOffsetFacingCenterPoint, end.m_cRotatorOffsetFacingCenterPoint, percentFixed, true);

    //result.m_cRotatorOffsetFacingCenterPoint = FMath::RInterpTo(start.m_cRotatorOffsetFacingCenterPoint, end.m_cRotatorOffsetFacingCenterPoint, percentFixed, 1);
    
    //FQuat QNow = FMath::Lerp(start.m_cRotatorOffsetFacingCenterPoint.Quaternion(), end.m_cRotatorOffsetFacingCenterPoint.Quaternion(), percentFixed);
    //result.m_cRotatorOffsetFacingCenterPoint = QNow.Rotator();

    //result.m_cRotatorOffsetFacingCenterPoint = start.m_cRotatorOffsetFacingCenterPoint + (end.m_cRotatorOffsetFacingCenterPoint - start.m_cRotatorOffsetFacingCenterPoint) * percentFixed;
};




FString
UMyCommonUtilsLibrary::getStringFromEnum(const TCHAR *enumName, uint8 value)
{
    //const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
    const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, enumName, true);
    if (!enumPtr)
    {
        return FString("Invalid Enum type");
    }

    //return FString::Printf(TEXT("%s(%d)"), *enumPtr->GetEnumNameStringByValue(value), value);
    //return enumPtr->GetEnumNameStringByValue(value);
    return FString::Printf(TEXT("%s(%d)"), *enumPtr->GetNameStringByValue(value), value);
    //GetNameStringByValue
}

int64
UMyCommonUtilsLibrary::nowAsMsFromTick()
{
    return UKismetMathLibrary::Now().GetTicks() / (ETimespan::TicksPerSecond / 1000);
}

FString UMyCommonUtilsLibrary::formatStrIds(const TArray<int32> &aIds)
{
    FString str;
    int32 l = aIds.Num();

    for (int32 i = 0; i < l; i++) {
        str += FString::Printf(TEXT("(%d), "), aIds[i]);
    }

    return str;
}

FString
UMyCommonUtilsLibrary::formatStrIdsValues(const TArray<int32> &aIds, const TArray<int32> &aValues)
{
    FString str;
    int32 l = aIds.Num();
    MY_VERIFY(l == aValues.Num());
    for (int32 i = 0; i < l; i++) {
        str += FString::Printf(TEXT("(%d, %d), "), aIds[i], aValues[i]);
    }

    return str;
}

FString
UMyCommonUtilsLibrary::formatStrIdValuePairs(const TArray<FMyIdValuePair> &aIdValues)
{
    FString str;
    int32 l = aIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        str += aIdValues[i].genDebugStr() + TEXT(",");
        //str += FString::Printf(TEXT("(%d, %d), "), aIdValues[i].m_iId, aIdValues[i].m_iValue);
    }

    return str;
}


FString
UMyCommonUtilsLibrary::formatMaskString(int32 iMask, uint32 uBitsCount)
{
    FString str = FString::Printf(TEXT("%d(bin: "), iMask);

    if (uBitsCount > 32) {
        uBitsCount = 32;
    }

    while (uBitsCount > 0) {
        uBitsCount--;
        str += FString::Printf(TEXT("%d"), (iMask >> uBitsCount) & 1);
    }

    str += TEXT(")");

    return str;

}

void UMyCommonUtilsLibrary::convertIdValuePairs2Ids(const TArray<FMyIdValuePair> &aIdValues, TArray<int32> &outaValues)
{
    outaValues.Reset();

    int32 l = aIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        outaValues.Emplace(aIdValues[i].m_iId);
    }
}

int32 UMyCommonUtilsLibrary::getEngineNetMode(AActor *actor)
{
    UWorld *world = actor->GetWorld();
    if (world) {
        return GEngine->GetNetMode(world);
    }
    else {
        return -1;
    }
};


//Todo: check this function's math correctness
FRotator UMyCommonUtilsLibrary::fixRotatorValuesIfGimbalLock(const FRotator& rotator, float PitchDeltaTolerance)
{
    float PitchNormalized = FRotator::NormalizeAxis(rotator.Pitch);

    if (FMath::IsNearlyEqual(PitchNormalized, 90, PitchDeltaTolerance))
    {
        FRotator ret = rotator;
        ret.Yaw = FRotator::NormalizeAxis(rotator.Yaw - rotator.Roll);
        ret.Roll = 0;
        return ret;
    }
    else if (FMath::IsNearlyEqual(PitchNormalized, -90, PitchDeltaTolerance))
    {
        FRotator ret = rotator;
        ret.Yaw = FRotator::NormalizeAxis(rotator.Yaw + rotator.Roll);
        ret.Roll = 0;
        return ret;
    }
    else {
        return rotator;
    }

};


void UMyCommonUtilsLibrary::rotateOriginWithPivot(const FTransform& originCurrentWorldTransform, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultWorldTransform)
{
    //FRotator originCurrentWorldRotator(originCurrentWorldTransform.GetRotation());
    //FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    //FVector originZeroRotateWorldLocation = originCurrentWorldTransform.GetLocation() - (originCurrentWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);

    //originResultWorldTransform.SetLocation(originZeroRotateWorldLocation + (originTargetWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    //originResultWorldTransform.SetRotation(FQuat(originTargetWorldRotator));
    //originResultWorldTransform.SetScale3D(originCurrentWorldTransform.GetScale3D());

    FQuat originCurrentWorldQuat = originCurrentWorldTransform.GetRotation();
    FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    FVector testV = originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("testV: (%f, %f, %f)."), testV.X, testV.Y, testV.Z);
    //return;

    FVector originZeroRotateWorldLocation = originCurrentWorldTransform.GetLocation() - (originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originZeroRotateWorldLocation: (%f, %f, %f)."), originZeroRotateWorldLocation.X, originZeroRotateWorldLocation.Y, originZeroRotateWorldLocation.Z);
    //return;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originTargetWorldRotator: (%f, %f, %f)."),
        originTargetWorldRotator.Roll, originTargetWorldRotator.Pitch, originTargetWorldRotator.Yaw);
    //FQuat originTargetWorldQuat(originTargetWorldRotator);
    FQuat originTargetWorldQuat = originTargetWorldRotator.Quaternion();
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originTargetWorldQuat: (%f, %f, %f, %f)."),
        originTargetWorldQuat.X, originTargetWorldQuat.Y, originTargetWorldQuat.Z, originTargetWorldQuat.W)
        //return;

        FRotator originTargetWorldRotator2(originTargetWorldQuat);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("rotator change: (%f, %f, %f) -> (%f, %f, %f)."),
        originTargetWorldRotator.Roll, originTargetWorldRotator.Pitch, originTargetWorldRotator.Yaw,
        originTargetWorldRotator2.Roll, originTargetWorldRotator2.Pitch, originTargetWorldRotator2.Yaw);
    //return;

    FVector locResultRotated = originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("locResultRotated: (%f, %f, %f)."), locResultRotated.X, locResultRotated.Y, locResultRotated.Z);

    originResultWorldTransform.SetLocation(originZeroRotateWorldLocation + (originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    originResultWorldTransform.SetRotation(originTargetWorldQuat);
    originResultWorldTransform.SetScale3D(originCurrentWorldTransform.GetScale3D());
};

FString UMyCommonUtilsLibrary::getDebugStringFromEWorldType(EWorldType::Type t)
{
    if (t == EWorldType::None)
    {
        return TEXT("None");
    }
    else if (t == EWorldType::Game)
    {
        return TEXT("Game");
    }
    else if (t == EWorldType::Editor)
    {
        return TEXT("Editor");
    }
    else if (t == EWorldType::PIE)
    {
        return TEXT("PIE");
    }
    else if (t == EWorldType::EditorPreview)
    {
        return TEXT("EditorPreview");
    }
    else if (t == EWorldType::GamePreview)
    {
        return TEXT("GamePreview");
    }
    else if (t == EWorldType::Inactive)
    {
        return TEXT("Inactive");
    }
    else
    {
        return TEXT("Invalid");
    }

};

FString UMyCommonUtilsLibrary::getDebugStringFromENetMode(ENetMode t)
{
    if (t == ENetMode::NM_Standalone)
    {
        return TEXT("NM_Standalone");
    }
    else if (t == ENetMode::NM_DedicatedServer)
    {
        return TEXT("NM_DedicatedServer");
    }
    else if (t == ENetMode::NM_ListenServer)
    {
        return TEXT("NM_ListenServer");
    }
    else if (t == ENetMode::NM_Client)
    {
        return TEXT("NM_Client");
    }
    else if (t == ENetMode::NM_MAX)
    {
        return TEXT("NM_MAX");
    }
    else {
        return TEXT("Invalid");
    }
}

void UMyCommonUtilsLibrary::MyBpLog(UObject* WorldContextObject, const FString& InString, bool bPrintToScreen, bool bPrintToLog, MyLogVerbosity eV, FLinearColor TextColor, float Duration)
{
    const FString *pStr = &InString;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);


    FString Prefix;
    if (World)
    {
        Prefix = getDebugStringFromEWorldType(World->WorldType) + TEXT(" ") + getDebugStringFromENetMode(World->GetNetMode()) + TEXT(": ");
        Prefix += InString;
        pStr = &Prefix;
    }

    if (bPrintToLog) {
        //the fuck is that, UE_LOG's log level is a compile time check
        if (eV == MyLogVerbosity::Display) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s"), **pStr);
        }
        else if (eV == MyLogVerbosity::Warning) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("%s"), **pStr);
        }
        else if (eV == MyLogVerbosity::Error) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s"), **pStr);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Log, TEXT("%s"), **pStr);
        }


        APlayerController* PC = (WorldContextObject ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : NULL);
        ULocalPlayer* LocalPlayer = (PC ? Cast<ULocalPlayer>(PC->Player) : NULL);
        if (LocalPlayer && LocalPlayer->ViewportClient && LocalPlayer->ViewportClient->ViewportConsole)
        {
            LocalPlayer->ViewportClient->ViewportConsole->OutputText(*pStr);
        }
    }

    if (bPrintToScreen)
    {
        if (GAreScreenMessagesEnabled)
        {
            if (GConfig && Duration < 0)
            {
                GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
            }

            bool bPrint = true;
            //#if (UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test
            bPrint = (uint8)eV >= (uint8)MyLogVerbosity::Display;
            //#endif
            if (bPrint) {
                GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), *pStr);
            }
        }
        else
        {
            UE_LOG(LogBlueprint, VeryVerbose, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
        }
    }

};

FString UMyCommonUtilsLibrary::getClassAssetPath(UClass* pC)
{
    //two way:
    // 1st is UPackage* pP = Cast<UPackage>(m_cCfgCardClass->GetDefaultObject()->GetOuter()); pP->FileName; result like /Game/CoreBPs/MyMJGameCardBaseBp
    // 2nd is m_cCfgCardClass->GetPathName(NULL), result like /Game/CoreBPs/MyMJGameCardBaseBp.MyMJGameCardBaseBp_C
    FString ret;

    if (!pC) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class is NULL"));
        return ret;
    }

    UObject* o = pC->GetDefaultObject()->GetOuter();
    if (!o) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("outer is NULL"));
        return ret;
    }
    UPackage* pP = Cast<UPackage>(o);
    if (!o) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to cast, class name is %s."), *o->GetClass()->GetName());
        return ret;
    }

    FString assetFullName = pP->FileName.ToString();
    int32 idxLastDelim = -1;
    if (!assetFullName.FindLastChar('/', idxLastDelim)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to find delimitor, orinin str %s."), *assetFullName);
        return ret;
    }

    MY_VERIFY(idxLastDelim >= 0);
    ret = assetFullName.Left(idxLastDelim);

    return ret;

    //UWidgetLayoutLibrary
}

struct FMyScreenDataCache
{
public:
    FSceneViewProjectionData m_cSceneViewProjectionData;
    FMatrix m_cViewProjMatrix;
    FMatrix m_cInvViewProjMatrix;

};

static FMyScreenDataCache g_sMyScreenDataCache;
static bool g_bMyScreenDataCacheValid;

void UMyCommonUtilsLibrary::invalidScreenDataCache()
{
    g_bMyScreenDataCacheValid = false;
}

void UMyCommonUtilsLibrary::refillScreenDataCache(const UObject* WorldContextObject)
{
    g_bMyScreenDataCacheValid = false;
    APlayerController* Player = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
    if (LP && LP->ViewportClient)
    {
        // get the projection data
        if (LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ g_sMyScreenDataCache.m_cSceneViewProjectionData))
        {
            g_sMyScreenDataCache.m_cViewProjMatrix = g_sMyScreenDataCache.m_cSceneViewProjectionData.ComputeViewProjectionMatrix();
            g_sMyScreenDataCache.m_cInvViewProjMatrix = g_sMyScreenDataCache.m_cViewProjMatrix.InverseFast();
            g_bMyScreenDataCacheValid = true;

            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("GetConstrainedViewRect(): %s; %s"),
             //*g_sMyScreenDataCache.m_cSceneViewProjectionData.GetViewRect().ToString(),
             //*g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().ToString());
            /*
            FMatrix const ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
            const bool bResult = FSceneView::ProjectWorldToScreen(WorldPosition, ProjectionData.GetConstrainedViewRect(), ViewProjectionMatrix, ScreenPosition);

            if (bPlayerViewportRelative)
            {
            ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
            }

            return bResult;
            */
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to GetProjectionData()."));
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ViewPort related pointer invalid: %p, %p, %p."), Player, LP, LP ? LP->ViewportClient : NULL);
    }
}

void UMyCommonUtilsLibrary::playerScreenFullPosiAbsoluteToPlayerScreenConstrainedPosiAbsolute(const UObject* WorldContextObject, const FVector2D& FullPosiAbsolute, FVector2D& ConstrainedPosiAbsolute)
{
    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        ConstrainedPosiAbsolute = FVector2D::ZeroVector;
        return;
    }

    ConstrainedPosiAbsolute = FullPosiAbsolute - FVector2D(g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().Min);
}

void UMyCommonUtilsLibrary::playerScreenConstrainedPosiAbsoluteToPlayerScreenFullPosiAbsolute(const UObject* WorldContextObject, const FVector2D& ConstrainedPosiAbsolute, FVector2D& FullPosiAbsolute)
{
    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        FullPosiAbsolute = FVector2D::ZeroVector;
        return;
    }

    FullPosiAbsolute = ConstrainedPosiAbsolute + FVector2D(g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().Min);
}

void UMyCommonUtilsLibrary::playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(const UObject* WorldContextObject, const FVector2D& ConstrainedPosiPercent, FVector2D& ConstrainedPosiAbsolute)
{
    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        ConstrainedPosiAbsolute = FVector2D::ZeroVector;
        return;
    }

    const FIntRect& r = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect();
    ConstrainedPosiAbsolute.X = ConstrainedPosiPercent.X * r.Width();
    ConstrainedPosiAbsolute.Y = ConstrainedPosiPercent.Y * r.Height();
}

void UMyCommonUtilsLibrary::getPlayerScreenSizeAbsolute(const UObject* WorldContextObject, FVector2D& ConstrainedSize, FVector2D& FullSize)
{
    ConstrainedSize = FullSize = FVector2D(100, 100);

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return;
    }

    ConstrainedSize = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().Size();
    FullSize = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetViewRect().Size();
};

bool UMyCommonUtilsLibrary::myProjectWorldToScreen(const UObject* WorldContextObject, const FVector& WorldPosition, bool ShouldOutScreenPosiAbsoluteConstrained, FVector2D& OutScreenPosiAbsolute)
{
    OutScreenPosiAbsolute = FVector2D::ZeroVector;

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return false;
    }

    const FSceneViewProjectionData& ProjectionData = g_sMyScreenDataCache.m_cSceneViewProjectionData;
    const FMatrix& ViewProjectionMatrix = g_sMyScreenDataCache.m_cViewProjMatrix;

    FVector2D posi;
    const bool bResult = FSceneView::ProjectWorldToScreen(WorldPosition, ProjectionData.GetConstrainedViewRect(), ViewProjectionMatrix, posi);

    if (bResult) {
        if (ShouldOutScreenPosiAbsoluteConstrained) {
            playerScreenFullPosiAbsoluteToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, posi, OutScreenPosiAbsolute);
        }
        else {
            OutScreenPosiAbsolute = posi;
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed in ProjectWorldToScreen()."));
    }

    return bResult;

};

bool UMyCommonUtilsLibrary::myDeprojectScreenToWorld(const UObject* WorldContextObject, const FVector2D& PosiAbsolute, bool IsPosiConstrained, FVector& WorldPosition, FVector& WorldDirection)
{
    WorldPosition = FVector::ZeroVector;
    WorldDirection = FVector::ZeroVector;

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return false;
    }

    FVector2D ScreenPosition = PosiAbsolute;
    if (IsPosiConstrained) {
        playerScreenConstrainedPosiAbsoluteToPlayerScreenFullPosiAbsolute(WorldContextObject, PosiAbsolute, ScreenPosition);
    }

    const FSceneViewProjectionData& ProjectionData = g_sMyScreenDataCache.m_cSceneViewProjectionData;
    const FMatrix& InvViewProjMatrix = g_sMyScreenDataCache.m_cInvViewProjMatrix;
    FSceneView::DeprojectScreenToWorld(ScreenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ WorldPosition, /*out*/ WorldDirection);
    return true;
}

void UMyCommonUtilsLibrary::playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(const UObject* WorldContextObject, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, float &DistanceFromCamera, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    DistanceFromCamera = 100;
    CameraCenterWorldPosition = CameraCenterDirection = FVector(0, 0, 1);

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return;
    }

    const FIntRect& r = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect();

    FVector2D p0, p1, pCenter;
    p0.X = r.Width() / 2;
    p0.Y = r.Height() / 2 - ConstrainedVLengthAbsoluteInCamera / 2;

    p1.X = r.Width() / 2;
    p1.Y = r.Height() / 2 + ConstrainedVLengthAbsoluteInCamera / 2;

    pCenter.X = r.Width() / 2;
    pCenter.Y = r.Height() / 2;

    FVector worldPosition0, worldDirection0, worldPosition1, worldDirection1;

    if (!myDeprojectScreenToWorld(WorldContextObject, p0, true, worldPosition0, worldDirection0)) {
        return;
    }

    if (!myDeprojectScreenToWorld(WorldContextObject, p1, true, worldPosition1, worldDirection1)) {
        return;
    }

    float l0 = (worldPosition0 - worldPosition1).Size();
    float deltal = (worldDirection1 - worldDirection0).Size();

    if (deltal < 0.00001) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("delta %f is too small, fixing it."), deltal);
        deltal = 0.00001;
    }

    DistanceFromCamera = (ModelInWorldHeight - l0) / deltal;

    if (DistanceFromCamera < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("lengthFromCamera %f is too small, fixing it."), DistanceFromCamera);
        DistanceFromCamera = 0;
    }

    if (!myDeprojectScreenToWorld(WorldContextObject, pCenter, true, CameraCenterWorldPosition, CameraCenterDirection)) {
        return;
    }

    //rotatorFacingCameraInWorld = UKismetMathLibrary::FindLookAtRotation(worldDirection0 * 100, FVector::ZeroVector);

    return;
}


void UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByAbsolute(const UObject* WorldContextObject, FVector2D ConstrainedPosiAbsoluteInCamera, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    float distanceFromCamera = 10;
    playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(WorldContextObject, ConstrainedVLengthAbsoluteInCamera, ModelInWorldHeight, distanceFromCamera, CameraCenterWorldPosition, CameraCenterDirection);

    FVector worldPosition0(0, 0, 0), worldDirection0(0, 0, 0);
    myDeprojectScreenToWorld(WorldContextObject, ConstrainedPosiAbsoluteInCamera, true, worldPosition0, worldDirection0);

    FVector loc = worldPosition0 + (worldDirection0 * distanceFromCamera);
    ResultTransform.SetLocation(loc);
    ResultTransform.SetRotation(UKismetMathLibrary::FindLookAtRotation(CameraCenterDirection * 100, FVector::ZeroVector).Quaternion());
    ResultTransform.SetScale3D(FVector(1, 1, 1));
}

void UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByPercent(const UObject* WorldContextObject, FVector2D ConstrainedPosiPercentInCamera, float ConstrainedVLengthPercentInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    FVector2D ConstrainedPosiAbsoluteInCamera;
    playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, ConstrainedPosiPercentInCamera, ConstrainedPosiAbsoluteInCamera);

    FVector2D vPercent(0, 0), vAbsolute(0, 0);
    vPercent.Y = ConstrainedVLengthPercentInCamera;
    playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, vPercent, vAbsolute);

    helperResolveWorldTransformFromPlayerCameraByAbsolute(WorldContextObject, ConstrainedPosiAbsoluteInCamera, vAbsolute.Y, ModelInWorldHeight, ResultTransform, CameraCenterWorldPosition, CameraCenterDirection);
}

#define MyArtDirBase (TEXT("/Game/Art"))
#define MyArtDirNameCommon (TEXT("Common"))
#define MyArtDirNameCurves (TEXT("Curves"))
#define MyArtFileNameCurveVectorDefaultLinear (TEXT("CurveVectorDefaultLinear"))
#define MyArtFileNameCurveVectorDefaultAccelerate0 (TEXT("CurveVectorDefaultAccelerate0"))

UCurveVector* UMyCommonUtilsLibrary::getCurveVectorByType(MyCurveAssetType curveType)
{
    FString fullName = FString(MyArtDirBase) + TEXT("/") + MyArtDirNameCommon + TEXT("/") + MyArtDirNameCurves + TEXT("/");

    if (curveType == MyCurveAssetType::DefaultLinear)
    {
        fullName += MyArtFileNameCurveVectorDefaultLinear;
    }
    else if (curveType == MyCurveAssetType::DefaultAccelerate0)
    {
        fullName += MyArtFileNameCurveVectorDefaultAccelerate0;
    }

    UCurveVector* pRet = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UCurveVector>(NULL, fullName);
    MY_VERIFY(pRet);

    return pRet;
};

UCurveVector* UMyCommonUtilsLibrary::getCurveVectorFromSettings(const FMyCurveVectorSettingsCpp& settings)
{
    UCurveVector* ret = NULL;
    if (settings.m_pCurveOverride) {
        ret = settings.m_pCurveOverride;
    }
    else {
        ret = getCurveVectorByType(settings.m_eCurveType);
    }

    MY_VERIFY(ret);

    return ret;
};

void UMyCommonUtilsLibrary::helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
                                                                                                        float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                                        const FVector2D& targetPosiFixOnScreenPercent,
                                                                                                        float targetVOnScreenPercent,
                                                                                                        float targetModelHeightInWorld,
                                                                                                        FTransform &outTargetTranform)
{
    FVector popPointMapped = meta.m_cScreenCenterMapped + targetPosiFromCenterToBorderOnScreenPercent * meta.m_fCenterToPointUntilBorderLength * meta.m_cDirectionCenterToPointMapped;
    FVector2D popPoint;
    popPoint.X = popPointMapped.X;
    popPoint.Y = popPointMapped.Y;

    popPoint.X += meta.m_cScreenCenterMapped.X * 2 * targetPosiFixOnScreenPercent.X;
    popPoint.Y += meta.m_cScreenCenterMapped.Y * 2 * targetPosiFixOnScreenPercent.Y;

    float vAbsoluteOnScreen = targetVOnScreenPercent * meta.m_cScreenCenterMapped.Y * 2;

    FVector cameraCenterLoc, cameraCenterDir;
    UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByAbsolute(WorldContextObject, popPoint, vAbsoluteOnScreen, targetModelHeightInWorld, outTargetTranform, cameraCenterLoc, cameraCenterDir);
}

float UMyCommonUtilsLibrary::helperGetRemainTimePercent(const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas)
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

void UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                                    const FMyActorTransformUpdateAnimationStepCpp& stepData,
                                                                    const TArray<FMyWithCurveUpdaterTransformCpp *>& updatersSorted)
{
    float fTotalTime = meta.m_fTotalTime;
    const FTransform& pointTransform = meta.m_cPointTransform;
    const FTransform& disappearTransform = meta.m_cDisappearTransform;
    const FVector& modelBoxExtend = meta.m_cModelBoxExtend;

    const FMyActorTransformUpdateAnimationStepCpp& cStepData = stepData;

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
    //FMyWithCurveUpdaterTransformCpp* updaterCenter = updatersSorted[idxCenter];
    //FTransform& nextTransformCenter = aNextTransforms[idxCenter];


    if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::PrevLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformPrevRefConst().GetLocation());
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::FinalLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation());
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::OffsetFromPrevLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformPrevRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::OffsetFromFinalLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen)
    {
        FVector location = pointTransform.GetLocation();
        for (int32 i = 0; i < l; i++) {
            FVector locationForScreen = location + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2 + updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation() - finalLocationCenter;
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("locationForScreen: %s"), *locationForScreen.ToString());
            aNextTransforms[i].SetLocation(locationForScreen);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::DisappearAtAttenderBorderOnPlayerScreen)
    {
        FVector location = disappearTransform.GetLocation();
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(location + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2 + updatersSorted[i]->getHelperTransformFinalRefConst().GetLocation() - finalLocationCenter);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::OffsetFromGroupPoint)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(updatersSorted[i]->getHelperTransformGroupPointRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend.Size() * 2);
        }
    }

    if (cStepData.m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::PrevRotation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(updatersSorted[i]->getHelperTransformPrevRefConst().GetRotation());
        }
    }
    else if (cStepData.m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::FinalRotation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(updatersSorted[i]->getHelperTransformFinalRefConst().GetRotation());
        }
    }
    else if (cStepData.m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::FacingPlayerScreen)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(pointTransform.GetRotation());
        }
    }

    if (cStepData.m_eScaleUpdateType == MyActorTransformUpdateAnimationScaleType::PrevScale)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(updatersSorted[i]->getHelperTransformPrevRefConst().GetScale3D());
        }
    }
    else  if (cStepData.m_eScaleUpdateType == MyActorTransformUpdateAnimationScaleType::FinalScale)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(updatersSorted[i]->getHelperTransformFinalRefConst().GetScale3D());
        }
    }
    else  if (cStepData.m_eScaleUpdateType == MyActorTransformUpdateAnimationScaleType::Specified)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(cStepData.m_cScaleSpecified);
        }
    }


    float fDur = fTotalTime * cStepData.m_fTimePercent;
    UCurveVector* pCurve = UMyCommonUtilsLibrary::getCurveVectorFromSettings(cStepData.m_cCurve);
    for (int32 i = 0; i < l; i++) {

        FMyWithCurveUpdaterTransformCpp *pUpdater = updatersSorted[i];

        FMyWithCurveUpdateStepDataTransformCpp data;
        data.helperSetDataBySrcAndDst(fDur, pCurve, pUpdater->getHelperTransformPrevRefConst(), aNextTransforms[i], cStepData.m_cRotationUpdateExtraCycles);
        pUpdater->addStepToTail(data);
    }
};

void UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                                     const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                                     const TArray<FMyWithCurveUpdaterTransformCpp *>& updatersSorted)
{
    int32 l = stepDatas.Num();

    if (l <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("stepDatas num is zero, debugStr: %s."), *meta.m_sDebugString);
        return;
    }

    float total = 0;

    bool bTimePecentTotalExpectedNot100Pecent = false;

    for (int32 i = 0; i < l; i++) {
        helperSetupTransformUpdateAnimationStep(meta, stepDatas[i], updatersSorted);
        float f = stepDatas[i].m_fTimePercent;
        total += f;

        bTimePecentTotalExpectedNot100Pecent |= stepDatas[i].m_bTimePecentTotalExpectedNot100Pecent;
    }

    if ((!bTimePecentTotalExpectedNot100Pecent) && l > 0 && !FMath::IsNearlyEqual(total, 1, MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("total time is not 100%, str %s. now it is %f."), *meta.m_sDebugString, total);
    };
}

void UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(const UObject* WorldContextObject,
                                                                             float totalDur,
                                                                             const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                             const FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                             const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                                             float extraDelayDur,
                                                                             const TArray<IMyTransformUpdaterInterfaceCpp *>& updaterInterfaces,
                                                                             FString debugName,
                                                                             bool clearPrevSteps)
{
    FMyTransformUpdateAnimationMetaCpp meta, *pMeta = &meta;
    TArray<FMyWithCurveUpdaterTransformCpp *> aUpdaters, *pUpdaters = &aUpdaters;

    //prepare meta
    pMeta->m_sDebugString = debugName;
    pMeta->m_fTotalTime = totalDur;
    if (updaterInterfaces.Num() <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("updaterInterfaces num is zero."));
        return;
    }
    FMyActorModelInfoBoxCpp modelInfo;
    updaterInterfaces[0]->getModelInfo(modelInfo);
    pMeta->m_cModelBoxExtend = modelInfo.m_cBoxExtend;

    helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, pointInfo.m_fShowPosiFromCenterToBorderPercent, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelBoxExtend.Size() * 2, pMeta->m_cPointTransform);
    helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, 1.2, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelBoxExtend.Size() * 2, pMeta->m_cDisappearTransform);

    bool bDelay = extraDelayDur >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT;

    pUpdaters->Reset();
    int32 l = updaterInterfaces.Num();
    for (int32 i = 0; i < l; i++) {
        FMyWithCurveUpdaterTransformCpp* pUpdater = &updaterInterfaces[i]->getMyWithCurveUpdaterTransformRef();
        pUpdaters->Emplace(pUpdater);
        if (clearPrevSteps) {
            pUpdater->clearSteps();
        }
    }

    if (bDelay) {
        UMyCommonUtilsLibrary::helperAddWaitStep(extraDelayDur, debugName + TEXT(" wait"), *pUpdaters);
    }

    UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationSteps(meta, stepDatas, *pUpdaters);
}

void UMyCommonUtilsLibrary::helperAddWaitStep(float waitTime, FString debugStr, const TArray<FMyWithCurveUpdaterTransformCpp *>& updaters)
{
    FMyTransformUpdateAnimationMetaCpp meta;
    FMyActorTransformUpdateAnimationStepCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = waitTime;

    stepData.m_fTimePercent = 1;


    helperSetupTransformUpdateAnimationStep(meta, stepData, updaters);
}

void UMyCommonUtilsLibrary::helperAddWaitStep(float waitTime, FString debugStr, const TArray<IMyTransformUpdaterInterfaceCpp*>& updaterInterfaces)
{

    TArray<FMyWithCurveUpdaterTransformCpp *> aUpdaters;

    for (int32 i = 0; i < updaterInterfaces.Num(); i++)
    {
        aUpdaters.Emplace(&updaterInterfaces[i]->getMyWithCurveUpdaterTransformRef());
    }

    UMyCommonUtilsLibrary::helperAddWaitStep(waitTime, debugStr, aUpdaters);
}

void UMyCommonUtilsLibrary::calcPointTransformWithLocalOffset(const FTransform& pointTransform, FVector localOffset, FTransform& pointTransformFixed)
{
    FTransform offset;
    offset.SetLocation(localOffset);

    pointTransformFixed = offset * pointTransform;

    //faster path that ignored scale
    //if (FMath::IsNearlyEqual(localOffset.X, 0), MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT) {
    //
    //}
}

FString UMyCommonUtilsLibrary::Conv_MyTransformZRotation_String(const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation)
{
    return myTransformZRotation.ToString();

    return TEXT("[") + myTransformZRotation.m_cLocation.ToString() + TEXT("(") + myTransformZRotation.m_cRotatorOffsetFacingCenterPoint.ToString() + TEXT(")]");
};


//Todo: if point is above center as YAW = 90D, the rotation compute is not stable when interp, fix it later
void UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(const FTransform& centerPointTransformWorld, const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation, FTransform& transformWorld)
{
    //MY_VERIFY(transformZRotation.m_cLocation.m_fRadius >= 0);

    float radiansYaw = FMath::DegreesToRadians(myTransformZRotation.m_cLocation.m_fYawOnXYPlane);
    float s, c;
    FMath::SinCos(&s, &c, radiansYaw);

    FVector loc0;
    loc0.X = c * myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane;
    loc0.Y = s * myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane;
    loc0.Z = myTransformZRotation.m_cLocation.m_fZoffset;

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("0 loc0: %s, s %f, r %f."), *loc0.ToString(), s, radiansYaw);

    //FQuat toCenter = FRotationMatrix::MakeFromX(-loc0).ToQuat();
    FRotator toCenterR = FRotationMatrix::MakeFromX(-loc0).Rotator();
    toCenterR.Roll = 0;
    FQuat toCenter = toCenterR.Quaternion();
    //FQuat toCenter = UKismetMathLibrary::FindLookAtRotation(loc0, FVector::ZeroVector).Quaternion();
    //toCenter = fixRotatorValuesIfGimbalLock(toCenter.Rotator()).Quaternion();
    //FQuat toCenter = UKismetMathLibrary::FindLookAtRotation(loc0, FVector::ZeroVector).Quaternion();

    //FRotator UKismetMathLibrary::FindLookAtRotation(const FVector& Start, const FVector& Target)
    //FRotator UKismetMathLibrary::MakeRotFromX(const FVector& X)

    FQuat quat0 = toCenter * myTransformZRotation.m_cRotatorOffsetFacingCenterPoint.Quaternion();

    //FRotator rot0 = UKismetMathLibrary::ComposeRotators(ringPointLocalRotator, FRotator(0, FMath::RadiansToDegrees(radiansFixed), 0));

    FTransform T0;
    T0.SetLocation(loc0);
    T0.SetRotation(quat0);

    transformWorld = T0 * centerPointTransformWorld;

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("debug: loc0 %s, toCenter %s %s, quat0 r %s, final %s."), *loc0.ToString(), *toCenter.ToString(), *toCenter.Rotator().ToString(), *quat0.Rotator().ToString(), *transformWorld.GetRotation().Rotator().ToString());
}

void UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(const FTransform& centerPointTransformWorld, const FTransform& transformWorld, FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation)
{
    FTransform T0;
    T0 = transformWorld * centerPointTransformWorld.Inverse();

    FVector loc0 = T0.GetLocation();
    FQuat quat0 = T0.GetRotation();

    myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane = FMath::Sqrt(loc0.X * loc0.X + loc0.Y * loc0.Y);
    float s = loc0.Y / myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane;;
    //float radiansYaw = FMath::Asin(s);
    float radiansYaw = FMath::Atan2(loc0.Y, loc0.X); //ATan use UE4's X Y coordinate
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("1 loc0: %s, s %f, r %f."), *loc0.ToString(), s, radiansYaw);

    myTransformZRotation.m_cLocation.m_fYawOnXYPlane = FRotator::ClampAxis(FMath::RadiansToDegrees(radiansYaw));
    myTransformZRotation.m_cLocation.m_fZoffset = loc0.Z;

    //FQuat toCenter = FRotationMatrix::MakeFromX(-loc0).ToQuat();
    
    FRotator toCenterR = FRotationMatrix::MakeFromX(-loc0).Rotator();
    toCenterR.Roll = 0;
    FQuat toCenter = toCenterR.Quaternion();

    //toCenter = fixRotatorValuesIfGimbalLock(toCenter.Rotator()).Quaternion();

    //myTransformZRotation.m_cRotatorOffsetFacingCenterPoint = (toCenter.Inverse() * quat0).Rotator();
    FRotator r0 = (toCenter.Inverse() * quat0).Rotator();

    myTransformZRotation.m_cRotatorOffsetFacingCenterPoint = r0; // fixRotatorValuesIfGimbalLock(r0);
}

/*
void UMyCommonUtilsLibrary::calcRingPointTransformAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, FTransform& ringPointTransform, FVector ringPointLocalOffset, FRotator ringPointLocalRotator)
{
 */
 /*
    FTransform TA, TB;
    FVector vA = ringPointLocalOffset;
    vA.X += ringRadius;

    TA.SetLocation(vA);
    TA.SetRotation(ringPointLocalRotator.Quaternion());


    FRotator rB = FRotator::ZeroRotator;
    rB.Yaw = degree;

    TB.SetRotation(rB.Quaternion());

    FTransform T0 = TA * TB;

    ringPointTransform = T0 * centerPointTransform;
    */

    //use 2d->3d translate style so we can map in two sides in one-one way

    /*
    float radiusFixed, radiansDelta;
    if (0 != fixRadiusAndRadiansForLocalOffsetOn2DCycle(ringRadius, ringPointLocalOffset.X, ringPointLocalOffset.Y, radiusFixed, radiansDelta))
    {
        return;
    }

    float radiansFixed = FMath::DegreesToRadians(degree) + radiansDelta;

    calcRingPointTransformAroundCenterPointZAxisWithFixedData(centerPointTransform, radiusFixed, radiansFixed, ringPointTransform, ringPointLocalOffset.Z, ringPointLocalRotator);
}


void UMyCommonUtilsLibrary::calcWorldTransformFromWorldOffsetAndDegreeForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, const FTransform& worldOffset, FTransform& worldTransform, FVector ringPointLocalOffset, FRotator ringPointLocalRotator)
{
    float radiusFixed, radiansDelta;
    if (0 != fixRadiusAndRadiansForLocalOffsetOn2DCycle(ringRadius, ringPointLocalOffset.X, ringPointLocalOffset.Y, radiusFixed, radiansDelta))
    {
        return;
    }

    float radiansFixed = FMath::DegreesToRadians(degree) + radiansDelta;

    FTransform ringPointTransform;
    calcRingPointTransformAroundCenterPointZAxisWithFixedData(centerPointTransform, radiusFixed, radiansFixed, ringPointTransform, ringPointLocalOffset.Z, ringPointLocalRotator);

    FQuat ringPointFRotator(0, 0, FMath::RadiansToDegrees(radiansFixed)).Quaternion() * centerPointTransform.GetRotation();

    FVector locLocalOffsetTowardOuter;

    worldTransform.SetLocation(worldOffset.GetLocation() + ringPointTransform.GetLocation());
    worldTransform.SetRotation((worldOffset.GetRotation().Rotator() + ringPointTransform.GetRotation().Rotator()).Quaternion());
    worldTransform.SetScale3D(FVector(1, 1, 1));
}

void UMyCommonUtilsLibrary::calcWorldOffsetAndDegreeFromWorldTransformForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, const FTransform& worldTransform, float &degree, FTransform& worldOffset, FVector ringPointLocalOffset, FRotator ringPointLocalRotator)
{
    float radiusFixed, radiansDelta;
    if (0 != fixRadiusAndRadiansForLocalOffsetOn2DCycle(ringRadius, ringPointLocalOffset.X, ringPointLocalOffset.Y, radiusFixed, radiansDelta))
    {
        return;
    }

    FQuat centerPointQuat = centerPointTransform.GetRotation();
    FVector worldPointRelativePosi = worldTransform.GetLocation() - centerPointTransform.GetLocation();

    FVector posiMapped;
    posiMapped.X = FVector::DotProduct(centerPointQuat.GetAxisX(), worldPointRelativePosi);
    posiMapped.Y = FVector::DotProduct(centerPointQuat.GetAxisY(), worldPointRelativePosi);
    float radiansMapped = posiMapped.HeadingAngle();

    FTransform ringPointTransform;
    calcRingPointTransformAroundCenterPointZAxisWithFixedData(centerPointTransform, radiusFixed, radiansMapped, ringPointTransform, ringPointLocalOffset.Z, ringPointLocalRotator);

    degree = FMath::RadiansToDegrees(radiansMapped - radiansDelta);

    worldOffset.SetLocation(worldTransform.GetLocation() - ringPointTransform.GetLocation());
    worldOffset.SetRotation((worldTransform.GetRotation().Rotator() - ringPointTransform.GetRotation().Rotator()).Quaternion());
    worldOffset.SetScale3D(FVector(1, 1, 1));
}

void UMyCommonUtilsLibrary::calcRingPointTransformAroundCenterPointZAxisWithFixedData(const FTransform& centerPointTransform, float radiusFixed, float radiansFixed, FTransform& ringPointTransform, float ringPointLocalZOffset, const FRotator& ringPointLocalRotator)
{
    float s, c;
    FMath::SinCos(&s, &c, radiansFixed);

    //enter 3d domain
    FVector loc0;
    loc0.X = c * radiusFixed;
    loc0.Y = s * radiusFixed;
    loc0.Z = ringPointLocalZOffset;

    FRotator rot0 = UKismetMathLibrary::ComposeRotators(ringPointLocalRotator, FRotator(0, FMath::RadiansToDegrees(radiansFixed), 0));

    FTransform T0;
    T0.SetLocation(loc0);
    T0.SetRotation(rot0.Quaternion());

    ringPointTransform = T0 * centerPointTransform;
}
*/

int32 UMyCommonUtilsLibrary::fixRadiusAndRadiansForLocalOffsetOn2DCycle(float radius, float xOffset, float yOffset, float &radiusFixed, float &radiansDelta)
{
    if (radius < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("radius is negative: %f."), radius);
        return -1;
    }

    radiusFixed = FMath::Sqrt(radius * radius + yOffset * yOffset) + xOffset;

    if (radiusFixed < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("radiusFixed is negative: %f, radius %f, xOffset %f."), radiusFixed, radius, xOffset);
        return -2;
    }

    radiansDelta = FMath::Atan2(yOffset, radius);

    return 0;
}