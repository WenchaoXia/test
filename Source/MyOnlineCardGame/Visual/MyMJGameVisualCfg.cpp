// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameVisualCfg.h"

#define MyScreenAttender_13_AreaPointOverrideYPecent (0.1)

bool FMyMJGameCameraCfgCpp::checkSettings() const
{
    if (m_fAttenderCameraFOV <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_fAttenderCameraFOV should not be negative, value now %f."), m_fAttenderCameraFOV);
        return false;
    }

    FVector loc = m_cAttenderCameraRelativeTransformAsAttender0.GetLocation();
    if (loc.IsNearlyZero())
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cAttenderCameraRelativeTransform's location is near zero, value now %s."), *loc.ToString());
        return false;
    }

    if (m_fAttenderCameraMoveTime <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_fAttenderCameraMoveTime should not be negative, value now %f."), m_fAttenderCameraMoveTime);
        return false;
    }

    return true;
}

void FMyMJGameCameraCfgCpp::fillDefaultData()
{
    reset();

    m_cAttenderCameraRelativeTransformAsAttender0.SetLocation(FVector(-1220, 0, 1010));
    m_cAttenderCameraRelativeTransformAsAttender0.SetRotation(FRotator(-40, 0, 0).Quaternion());
    m_cAttenderCameraRelativeTransformAsAttender0.SetScale3D(FVector(1, 1, 1));

    m_fAttenderCameraFOV = 30;

    m_fAttenderCameraMoveTime = 1;
    m_cAttenderCameraMoveCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
}

bool FMyMJGameInGamePlayerScreenCfgCpp::checkSettings() const
{
    return true;
};

void FMyMJGameInGamePlayerScreenCfgCpp::fillDefaultData()
{
    for (int32 i = 0; i < 4; i++) {
        FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& area = m_aAttenderAreas[i];
        area.reset();
        area.m_cCardShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.3;
        area.m_cCardShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.35;

        area.m_cDiceShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.2;
        area.m_cDiceShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.15;

        area.m_cCommonActionShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.4;
        area.m_cCommonActionShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.25;

        /*
        FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp& pointCfg = area.m_cCommonActionShowPoint;
        if (i == 1)
        {
            area.m_bAttenderPointOnScreenOverride = true;
            area.m_cAttenderPointOnScreenPercentOverride.X = 1;
            area.m_cAttenderPointOnScreenPercentOverride.Y = MyScreenAttender_13_AreaPointOverrideYPecent;
        }
        else if (i == 3){
            area.m_bAttenderPointOnScreenOverride = true;
            area.m_cAttenderPointOnScreenPercentOverride.X = 0;
            area.m_cAttenderPointOnScreenPercentOverride.Y = MyScreenAttender_13_AreaPointOverrideYPecent;
        }
        */
    }

};

#define DefaultDistCardTimeBase (0.4f)
#define DefaultDistCardTimeInProgressCo (4.0f)
#define DefaultDistCardTimeLastExtra (1.0f)

void FMyMJGameEventPusherCfgCpp::fillDefaultData()
{
    FMyActorTransformUpdateAnimationStepCpp step;
    FMyActorTransformUpdateAnimationStepCpp* pStep = &step;
    TArray<FMyActorTransformUpdateAnimationStepCpp> *pSteps;

    m_cClientCommonUpdate.reset();
    m_cClientCommonUpdate.m_fTotalTime = 1;
    m_cClientCommonUpdate.m_cMoveCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;

    m_cFullBaseResetAtStart.reset();
    m_cFullBaseResetAtStart.m_fTotalTime = 1;

    m_cGameStart.m_fTotalTime = 0.5;

    m_cThrowDices.reset();
    m_cThrowDices.m_fTotalTime = 1.5;
    pSteps = &m_cThrowDices.m_aDiceSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.15;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::PrevRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.5;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PrevLocation;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.2;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);


    m_cDistCards.reset();
    m_cDistCards.m_fTotalTime = DefaultDistCardTimeBase;

    pSteps = &m_cDistCards.m_aCardsFocusedSteps;
    pStep->reset();
    pStep->m_bTimePecentTotalExpectedNot100Pecent = true;
    pStep->m_fTimePercent = 0.3 * DefaultDistCardTimeInProgressCo;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::OffsetFromGroupPoint;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.3 * DefaultDistCardTimeInProgressCo;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.2 * DefaultDistCardTimeInProgressCo;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::OffsetFromFinalLocation;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.2 * DefaultDistCardTimeInProgressCo;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pSteps->Emplace(*pStep);

    m_cDistCards.m_fTotalTimeOverrideForLast = DefaultDistCardTimeBase * DefaultDistCardTimeInProgressCo + DefaultDistCardTimeLastExtra;
    m_cDistCards.m_aCardsFocusedStepsOverrideForLast = m_cDistCards.m_aCardsFocusedSteps;
    for (int32 i = 0; i < m_cDistCards.m_aCardsFocusedStepsOverrideForLast.Num(); i++)
    {
        m_cDistCards.m_aCardsFocusedStepsOverrideForLast[i].m_bTimePecentTotalExpectedNot100Pecent = false;

        float oldTimeAbs = m_cDistCards.m_aCardsFocusedStepsOverrideForLast[i].m_fTimePercent * m_cDistCards.m_fTotalTime;
        m_cDistCards.m_aCardsFocusedStepsOverrideForLast[i].m_fTimePercent = oldTimeAbs / m_cDistCards.m_fTotalTimeOverrideForLast;
    }
    pStep->reset();
    pStep->m_fTimePercent = DefaultDistCardTimeLastExtra / m_cDistCards.m_fTotalTimeOverrideForLast;
    m_cDistCards.m_aCardsFocusedStepsOverrideForLast.Emplace(*pStep);

    m_cDistCards.m_fTotalTimeOverridedForCardsUnfocused = DefaultDistCardTimeBase * DefaultDistCardTimeInProgressCo;
    //m_cDistCards.m_fDelayTimeForCardsUnfocused = m_cDistCards.m_aCardsFocusedSteps[0].m_fTimePercent * m_cDistCards.m_fTotalTime;
    pSteps = &m_cDistCards.m_aCardsOtherSteps;

    pStep->reset();
    pStep->m_fTimePercent = 0.6;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.4;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pSteps->Emplace(*pStep);


    m_cTakeCards.reset();
    m_cTakeCards.m_fTotalTime = 1;

    pSteps = &m_cTakeCards.m_aCardsFocusedSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.4;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::OffsetFromFinalLocation;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.4;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);


    m_cGiveOutCards.reset();
    m_cGiveOutCards.m_fTotalTime = 1;

    pSteps = &m_cGiveOutCards.m_aCardsFocusedSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.3;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.5;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    m_cGiveOutCards.m_fDelayTimeForCardsUnfocused = m_cGiveOutCards.m_fTotalTime;

    pSteps = &m_cGiveOutCards.m_aCardsInsertedToHandSlotSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.2;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::OffsetFromPrevLocation;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.6;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::OffsetFromFinalLocation;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pSteps->Emplace(*pStep);


    m_cWeaveChi.reset();
    *static_cast<FMyMJGameEventCardsRelatedVisualDataCpp *>(&m_cWeaveChi) = *static_cast<FMyMJGameEventCardsRelatedVisualDataCpp *>(&m_cGiveOutCards);
    m_cWeaveChi.m_fTotalTime = 1.5;

    pSteps = &m_cWeaveChi.m_aDancingActor0Steps;
    pSteps->Reset();

    pStep->reset();
    pStep->m_fTimePercent = 0.001;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::DisappearAtAttenderBorderOnPlayerScreen;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FacingPlayerScreen;
    pStep->m_cCurve.m_eCurveType = MyCurveAssetType::DefaultAccelerate0;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.4;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FacingPlayerScreen;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pSteps->Emplace(*pStep);

    m_cWeavePeng.copyExceptClass(m_cWeaveChi);
    m_cWeaveGang.copyExceptClass(m_cWeaveChi);
    m_cWeaveBu.copyExceptClass(m_cWeaveChi);


    m_cHu.reset();
    m_cHu.m_fTotalTime = 0.5;

    m_cGameEnd.m_fTotalTime = 0.5;

    m_cLocalCSHuBorn.reset();
    m_cLocalCSHuBorn.m_fTotalTime = 0.5;

    m_cLocalCSZhaNiao.reset();
    m_cLocalCSZhaNiao.m_fTotalTime = 0.5;
};


bool FMyMJGameInRoomMainActorClassCfgCpp::checkSettings() const
{
    //FString debugStr;
    const TSubclassOf<AMyMJGameCardBaseCpp> &cClass = m_cCardClass;
    if (!IsValid(cClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class not valid."));
        return false;
    }

    if (cClass == AMyMJGameCardBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class must be a child class of AMyMJGameCardBaseCpp!"));
        return false;
    }

    if (!IsValid(m_cDiceClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dice class not valid."));
        return false;
    }

    if (m_cDiceClass == AMyMJGameDiceBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class must be a child class of AMyMJGameDiceBaseCpp!"));
        return false;
    }


    if (!IsValid(m_cInRoomUIMainWidgetClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("in room ui main widget class not valid."));
        return false;
    }

    if (m_cInRoomUIMainWidgetClass == UMyMJGameInRoomUIMainWidgetBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("in room ui main widget class must be a child class of AMyMJGameCardBaseCpp!"));
        return false;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class is %s."), *cClass->GetName());
    //TSubclassOf<UMyMJGameInRoomUIMainWidgetBaseCpp> m_cInRoomUIMainWidgetClass;

    return true;
};


void UMyMJGameInRoomVisualCfgType::fillDefaultData()
{
    m_cPlayerScreenCfg.fillDefaultData();
    m_cEventCfg.fillDefaultData();

    m_cCameraCfg.fillDefaultData();
}

bool UMyMJGameInRoomVisualCfgType::checkSettings() const
{
    if (!m_cPlayerScreenCfg.checkSettings())
    {
        return false;
    }

    if (!m_cEventCfg.checkSettings())
    {
        return false;
    }

    if (!m_cMainActorClassCfg.checkSettings())
    {
        return false;
    }

    if (!m_cCameraCfg.checkSettings())
    {
        return false;
    };

    return true;
}

#if WITH_EDITOR

void UMyMJGameInRoomVisualCfgType::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyMJGameInRoomVisualCfgType, m_iResetDefault))
    {
        if (m_iResetDefault == 10) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("filling default calue."));
            fillDefaultData();
        }

    }
    else {

    }

    Super::PostEditChangeProperty(e);
}

#endif

void UMyMJGameInRoomVisualCfgType::helperMapToSimplifiedTimeCfg(const FMyMJGameInRoomEventCfgCpp& eventCfg, FMyMJGameEventTimeCfgCpp& outSimplifiedTimeCfg)
{
    const FMyMJGameEventPusherCfgCpp &cPusherCfg = eventCfg.m_cPusherCfg;
    outSimplifiedTimeCfg.m_uiBaseResetAtStart = cPusherCfg.m_cFullBaseResetAtStart.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiGameStarted =cPusherCfg.m_cGameStart.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiThrowDices =cPusherCfg.m_cThrowDices.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiDistCards =cPusherCfg.m_cDistCards.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiDistCardsLast = cPusherCfg.m_cDistCards.m_fTotalTimeOverrideForLast * 1000;

    outSimplifiedTimeCfg.m_uiTakeCards =cPusherCfg.m_cTakeCards.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiGiveCards =cPusherCfg.m_cGiveOutCards.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiWeaveChi =cPusherCfg.m_cWeaveChi.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiWeavePeng =cPusherCfg.m_cWeavePeng.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiWeaveGang =cPusherCfg.m_cWeaveGang.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiWeaveGangBuZhangLocalCS =cPusherCfg.m_cWeaveBu.m_fTotalTime * 1000;

    outSimplifiedTimeCfg.m_uiHu =cPusherCfg.m_cHu.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiGameEnded =cPusherCfg.m_cGameEnd.m_fTotalTime * 1000;

    outSimplifiedTimeCfg.m_uiHuBornLocalCS =cPusherCfg.m_cLocalCSHuBorn.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiZhaNiaoLocalCS =cPusherCfg.m_cLocalCSZhaNiao.m_fTotalTime * 1000;
};