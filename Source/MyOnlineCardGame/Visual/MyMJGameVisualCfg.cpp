// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameVisualCfg.h"

#define MyScreenAttender_13_AreaPointOverrideYPecent (0.1)
void FMyMJGameInGamePlayerScreenCfgCpp::fillDefaultData()
{
    for (int32 i = 0; i < 4; i++) {
        FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& area = m_aAttenderAreas[i];
        area.reset();
        area.m_cCardShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.3;
        area.m_cCardShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.1;

        area.m_cCommonActionShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.4;
        area.m_cCommonActionShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.25;

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
    }
};

void FMyMJGameEventPusherCfgCpp::fillDefaultData()
{
    FMyActorTransformUpdateAnimationStepCpp step;
    FMyActorTransformUpdateAnimationStepCpp* pStep = &step;
    TArray<FMyActorTransformUpdateAnimationStepCpp> *pSteps;

    m_cResyncUnexpectedIngame.m_fTotalTime = 1;

    m_cResyncNormalAtStart.m_fTotalTime = 1;

    m_cGameStart.m_fTotalTime = 0.5;

    m_cThrowDices.m_fTotalTime = 0.5;

    m_cDistCardsDone.m_fTotalTime = 0.5;

    
    m_cTakeCards.reset();
    m_cTakeCards.m_fTotalTime = 1;

    pSteps = &m_cTakeCards.m_aCardsFocusedSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.4;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::offsetFromFinalLocation;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.4;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pSteps->Emplace(*pStep);


    m_cGiveOutCards.reset();
    m_cGiveOutCards.m_fTotalTime = 1;

    pSteps = &m_cGiveOutCards.m_aCardsFocusedSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.3;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen;
    pStep->m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::FinalRotation;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.5;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = UMyCommonUtilsLibrary::helperGetRemainTimePercent(*pSteps);
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::FinalLocation;
    pSteps->Emplace(*pStep);

    m_cGiveOutCards.m_fDelayTimeForCardsUnfocused = m_cGiveOutCards.m_fTotalTime;

    pSteps = &m_cGiveOutCards.m_aCardsInsertedToHandSlotSteps;
    pStep->reset();
    pStep->m_fTimePercent = 0.2;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::offsetFromPrevLocation;
    pStep->m_cLocationOffsetPercent.Z = 1;
    pSteps->Emplace(*pStep);

    pStep->reset();
    pStep->m_fTimePercent = 0.6;
    pStep->m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::offsetFromFinalLocation;
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
}

bool UMyMJGameInRoomVisualCfgType::checkSettings() const
{
    if (!m_cEventCfg.checkSettings())
    {
        return false;
    }

    if (!m_cMainActorClassCfg.checkSettings())
    {
        return false;
    }

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
    outSimplifiedTimeCfg.m_uiBaseResetAtStart = cPusherCfg.m_cResyncNormalAtStart.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiGameStarted =cPusherCfg.m_cGameStart.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiThrowDices =cPusherCfg.m_cThrowDices.m_fTotalTime * 1000;
    outSimplifiedTimeCfg.m_uiDistCardsDone =cPusherCfg.m_cDistCardsDone.m_fTotalTime * 1000;

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