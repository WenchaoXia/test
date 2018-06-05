// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomUI.h"

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGamePlayerController.h"

#include "Public/Blueprint/WidgetBlueprintLibrary.h"

#define MyMJGameWidgetMainTypeEventVisual    (0x00)
#define MyMJGameWidgetMainTypeIncidentVisual (0x01)
#define MyMJGameWidgetKeyCalculation(mainType, subType) (((mainType & 0xff) << 8) | (((uint8)subType) & 0xff))


FMyErrorCodeMJGameCpp UMyMJGameInRoomPlayerInfoWidgetBaseCpp::showAttenderWeave(float dur, MyMJGameEventVisualTypeCpp weaveVisualType)
{
    const UMyMJGameInRoomVisualCfgCpp* pVisualCfg = AMyMJGameRoomLevelScriptActorCpp::helperGetVisualCfg(this, true);
    const FMyMJGameTrivalDataCpp* pTrivalData = AMyMJGameRoomLevelScriptActorCpp::helperGetMJGameTrivalData(this, true);

    int32 idxAttender = m_cRuntimeData.m_iIdxAttender;
    MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> widgetClass;
    widgetClass = pTrivalData->m_aAttenderDatas[idxAttender].m_cStyleSettings.m_cEvent.getWeaveWidgetByType(weaveVisualType);
    if (!IsValid(widgetClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("player info widget have not set its corresponding widget class for weave visual type, using default one. visual type: %s, meta: %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameEventVisualTypeCpp"), (uint8)weaveVisualType), *m_cRuntimeData.ToString());
        
        UMyMJGameInRoomUIMainWidgetBaseCpp* pUIMain = AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(this, false);
        if (IsValid(pUIMain)) {
            const FMyMJGameInRoomUIMainWidgetCfgCpp* pCfg = NULL;
            pUIMain->getCfgRefConstFromCache(pCfg, false);
            if (pCfg) {
                widgetClass = pCfg->m_cDefaultInRoomViewRoleStyle.m_cEvent.getWeaveWidgetByType(weaveVisualType);
            }
        }

        if (!IsValid(widgetClass)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get default widet class, visual type: %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameEventVisualTypeCpp"), (uint8)weaveVisualType));
            return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InternalError);
        }
    }

    MY_VERIFY(IsValid(widgetClass));

    int32 key;
    CalcKey_Macro_KeyAnd4IdxsMap(key, 0, 0, MyMJGameWidgetMainTypeEventVisual, (uint8)weaveVisualType);

    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* pW = getManagedWidget(key, widgetClass, true);
    
    pW->SetVisibility(ESlateVisibility::Visible);

    FWidgetTransform wt;
    pW->SetRenderTransform(wt);

    const FMyModelInfoWidget2DCpp* pModelInfo = NULL;
    pW->getDataByCacheRefConst_MyModelInfoWidget2D(pModelInfo, true);
    pModelInfo->getBox2DRefConst().m_cCenterPointRelativeLocation;

    FVector2D selfContentSize;
    IMyContentSizeWidget2DInterfaceCpp::Execute_getContentSizeFromCache(this, selfContentSize, true);

    IMyCardGameScreenPositionRelatedWidgetInterfaceCpp::Execute_restartMainAnimation(pW, m_cRuntimeData.m_iIdxPositionInBox, dur, (selfContentSize / 2 - pModelInfo->getBox2DRefConst().m_cCenterPointRelativeLocation), m_cRuntimeData.m_cLocationCommonActionShowPoint - m_cRuntimeData.m_cLocationSelf, m_cRuntimeData.m_cLocationUICenter - m_cRuntimeData.m_cLocationSelf);

    return FMyErrorCodeMJGameCpp(true);
};


UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* UMyMJGameInRoomPlayerInfoWidgetBaseCpp::getManagedWidget(int32 key, TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>& widgetClass, bool createIfNotExist)
{
    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* pW = NULL;
    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp** ppW = m_mManagedWidgets.Find(key);
    if (ppW && (*ppW)->GetClass() == widgetClass) {
        pW = *ppW;
    }


    if (pW == NULL && createIfNotExist) {
        //let's recreate

        if (ppW) {
            (*ppW)->RemoveFromParent();
        }

        APlayerController* pPC = this->GetOwningPlayer();
        MY_VERIFY(IsValid(pPC));

        pW = Cast<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(UWidgetBlueprintLibrary::Create(pPC, widgetClass, pPC)); //no player controller owns it but let this class manage it
        if (ppW) {
            *ppW = pW;
        }
        else {
            m_mManagedWidgets.Add(key) = pW;
        }

        UCanvasPanel* canvasPanel = NULL;
        IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp::Execute_getDynamicAllocationRootCanvasPanelFromCache(this, canvasPanel, true);
        MY_VERIFY(canvasPanel);

        canvasPanel->AddChild(pW);

        MyErrorCodeCommonPartCpp ret = pW->updateSlotSettingsToComply_MyModelInfoWidget2D();
        if (MyErrorCodeCommonPartCpp::NoError != ret) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateSlotSettingsToComply_MyModelInfoWidget2D() got error: %s."), *pW->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
        }

    }

    if (createIfNotExist) {
        MY_VERIFY(pW);
    }

    return pW;
};



#if WITH_EDITOR
void UMyMJGameInRoomChoiceSelectCommonWidgetBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);

    FName PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyMJGameInRoomChoiceSelectCommonWidgetBaseCpp, m_cCenterButtonStyleNormal))
    {
        tryUpdateCenterButtonStyle();
    }


    Super::PostEditChangeProperty(e);
}
#endif


void UMyMJGameInRoomOperationRootPanelWidgetBaseCpp::updateWithActionContainor(MyMJGameRoleTypeCpp eDataRoleType, MyMJGameRuleTypeCpp eRuleType, int32 iGameId, int32 iActionGroupId, int32 idxAttender,
                                                                                const FMyMJCardValuePackCpp& cardValuePack,
                                                                                const FMyMJGameActionContainorForBPCpp& actionContainor)
{
    FUIUpdateDataCpp cUIUpdateData;

    int32 l = actionContainor.m_aActionChoices.Num();
    for (int32 i = 0; i < l; i++) {
        const FMyMJGameActionUnfiedForBPCpp& action = actionContainor.m_aActionChoices[i];
        bool selected = actionContainor.m_iChoiceSelected == i;

        if (action.getType() == MyMJGamePusherTypeCpp::ActionWeave) {
            if (selected) {
                cUIUpdateData.m_iIdxOfSelected = i;
            }

            const FMyMJWeaveCpp& cWeave = action.m_cWeave;

            MyMJGameEventVisualTypeCpp eWeaveVisualType = UMyMJBPUtilsLibrary::helperGetEventVisualTypeFromWeave(eRuleType, cWeave);
            if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeaveChi) {

                int32 idxT = cUIUpdateData.m_aChiDatas.AddDefaulted();
                FMyMJGameActionChoiceDataChiCpp& chiData = cUIUpdateData.m_aChiDatas[idxT];

                const TArray<int32>& ids = cWeave.getIdsRefConst();
                for (int32 j = 0; j < ids.Num(); j++) {
                    int32 idxValues = chiData.m_aValues.Emplace(cardValuePack.getByIdx(ids[j]));
                    if (ids[j] == cWeave.getIdTriggerCard()) {
                        chiData.m_iIdxOfTriggerCardInValues = idxValues;
                    }
                }

                chiData.m_iIdxOfSelection = i;
                chiData.m_bSelected = selected;
                if (selected) {
                    cUIUpdateData.m_bHaveChiSelected = true;
                }

                MY_VERIFY(cUIUpdateData.m_aChiDatas.Num() <= 3);
                cUIUpdateData.m_iCountOfChoices++;
            }
            else if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeavePeng) {
                FMyMJGameActionChoiceDataOtherCpp* pD = &cUIUpdateData.m_cPengData;

                MY_VERIFY(!pD->isValid());
                pD->m_iIdxOfSelection = i;
                pD->m_bSelected = selected;
                cUIUpdateData.m_iCountOfChoices++;
            }
            else if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeaveGang) {
                FMyMJGameActionChoiceDataOtherCpp* pD = &cUIUpdateData.m_cGangData;

                MY_VERIFY(!pD->isValid());
                pD->m_iIdxOfSelection = i;
                pD->m_bSelected = selected;
                cUIUpdateData.m_iCountOfChoices++;
            }
            else if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeaveBu) {
                FMyMJGameActionChoiceDataOtherCpp* pD = &cUIUpdateData.m_cBuData;

                MY_VERIFY(!pD->isValid());
                pD->m_iIdxOfSelection = i;
                pD->m_bSelected = selected;
                cUIUpdateData.m_iCountOfChoices++;
            }
            else {
                MY_VERIFY(false);
            }
        }
        else if (action.getType() == MyMJGamePusherTypeCpp::ActionNoAct) {
            if (selected) {
                cUIUpdateData.m_iIdxOfSelected = i;
            }

            FMyMJGameActionChoiceDataOtherCpp* pD = &cUIUpdateData.m_cGuoData;

            MY_VERIFY(!pD->isValid());
            pD->m_iIdxOfSelection = i;
            pD->m_bSelected = selected;
            cUIUpdateData.m_iCountOfChoices++;
            cUIUpdateData.m_iCountOfGuoChoices++;
        }
        else if (action.getType() == MyMJGamePusherTypeCpp::ActionHu) {
            if (selected) {
                cUIUpdateData.m_iIdxOfSelected = i;
            }

            FMyMJGameActionChoiceDataOtherCpp* pD = &cUIUpdateData.m_cHuData;

            MY_VERIFY(!pD->isValid());
            pD->m_iIdxOfSelection = i;
            pD->m_bSelected = selected;
            cUIUpdateData.m_iCountOfChoices++;
        }
        else {

        }
    }

    cUIUpdateData.m_iGameId = iGameId;
    cUIUpdateData.m_iActionGroupId = iActionGroupId;
    cUIUpdateData.m_iIdxAttender = idxAttender;
    cUIUpdateData.m_eDataRoleType = eDataRoleType;

    cUIUpdateData.postProcessForInteractiveFlag();

    bool delayUpdate = m_cUpdateStateLast.m_iIdxOfSelected >= 0 && (!(cUIUpdateData.m_iCountOfChoices > 0)) &&     //if we have selected before and have no new choices now
                       cUIUpdateData.m_iGameId == m_cUpdateStateLast.m_iGameId && cUIUpdateData.m_iIdxAttender == m_cUpdateStateLast.m_iIdxAttender && cUIUpdateData.m_eDataRoleType == m_cUpdateStateLast.m_eDataRoleType && //in same game, same view role, same data role
                       m_iDelayedUIUpdateTimeMs > 0;

    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        return;
    }


    if (delayUpdate) {
        m_cDelayedUIUpdateData = cUIUpdateData;
        world->GetTimerManager().ClearTimer(m_cDelayedUIUpdateTimerHandle);
        world->GetTimerManager().SetTimer(m_cDelayedUIUpdateTimerHandle, this, &UMyMJGameInRoomOperationRootPanelWidgetBaseCpp::delayedUIUpdate, (float)m_iDelayedUIUpdateTimeMs / 1000, false);
    }
    else{
        world->GetTimerManager().ClearTimer(m_cDelayedUIUpdateTimerHandle);
        UIUpdate(cUIUpdateData);
    }


};

void UMyMJGameInRoomOperationRootPanelWidgetBaseCpp::makeSelection(int32 IdxOfSelection)
{
    if (m_cUpdateStateLast.m_iGameId < 0 || m_cUpdateStateLast.m_iActionGroupId < 0 || m_cUpdateStateLast.m_iIdxAttender < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("currently have no valid choice tag."));
        return;
    }

    //always succeed, or coredump
    AMyMJGamePlayerControllerCpp* pC = AMyMJGamePlayerControllerCpp::helperGetLocalController(this);
    if (m_cUpdateStateLast.m_iIdxAttender != (int32)pC->getCmdRoleType()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("can not give cmd for this attender, action idxAttender %d, player controller cmd role type %d."),
                  m_cUpdateStateLast.m_iIdxAttender, (int32)pC->getCmdRoleType());
        return;
    }

    FMyMJGameCmdMakeSelectionCpp cmdMakeSelection;

    cmdMakeSelection.m_iGameId = m_cUpdateStateLast.m_iGameId;
    cmdMakeSelection.m_iActionGroupId = m_cUpdateStateLast.m_iActionGroupId;
    cmdMakeSelection.m_iIdxAttender = m_cUpdateStateLast.m_iIdxAttender;
    cmdMakeSelection.m_iSelection = IdxOfSelection;

    pC->makeSelection(cmdMakeSelection);
}

void UMyMJGameInRoomOperationRootPanelWidgetBaseCpp::delayedUIUpdate()
{
    UIUpdate(m_cDelayedUIUpdateData);
}

void UMyMJGameInRoomOperationRootPanelWidgetBaseCpp::UIUpdate(const FUIUpdateDataCpp& data)
{
    UMyMJGameInRoomOperationLvl1ActionPanelWidgetBaseCpp* pLv1 = NULL;
    UMyMJGameInRoomOperationLvl2ChiPanelWidgetBaseCpp* pLv2 = NULL;
    int32 idxInSwitcher = -1;
    MyErrorCodeCommonPartCpp ret;

    ret = IMyMJGameInRoomOperationRootPanelWidgetInterfaceCpp::Execute_get1stLvlActionPanelWidget(this, idxInSwitcher, pLv1);
    if (ret != MyErrorCodeCommonPartCpp::NoError || !IsValid(pLv1)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: get1stLvlActionPanelWidget() failed: %s, %p."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret), pLv1);
    }
    else {

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s: updating chi  : %d, %d."), *GetClass()->GetName(), data.m_aChiDatas.Num() > 0, data.m_bHaveChiSelected);
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s: updating peng : %d, %d."), *GetClass()->GetName(), data.m_cPengData.m_iIdxOfSelection, data.m_cPengData.m_bSelected);

        IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp::Execute_updateChi(pLv1, data.m_aChiDatas.Num() > 0, data.m_bHaveChiSelected, data.m_aChiDatas.Num() > 0 && data.m_iIdxOfSelected < 0);

        const FMyMJGameActionChoiceDataOtherCpp* pCD;

        pCD = &data.m_cPengData;
        IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp::Execute_updatePeng(pLv1, pCD->m_iIdxOfSelection, pCD->m_bSelected, pCD->m_bInteractiveEnabled);
       
        pCD = &data.m_cGangData;
        IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp::Execute_updateGang(pLv1, pCD->m_iIdxOfSelection, pCD->m_bSelected, pCD->m_bInteractiveEnabled);

        pCD = &data.m_cBuData;
        IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp::Execute_updateBu(pLv1, pCD->m_iIdxOfSelection, pCD->m_bSelected, pCD->m_bInteractiveEnabled);
        
        pCD = &data.m_cHuData;
        IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp::Execute_updateHu(pLv1, pCD->m_iIdxOfSelection, pCD->m_bSelected, pCD->m_bInteractiveEnabled);
        
        pCD = &data.m_cGuoData;
        IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp::Execute_updateGuo(pLv1, pCD->m_iIdxOfSelection, pCD->m_bSelected, pCD->m_bInteractiveEnabled);
    }

    ret = IMyMJGameInRoomOperationRootPanelWidgetInterfaceCpp::Execute_get2ndLvlChiPanelWidget(this, idxInSwitcher, pLv2);
    if (ret != MyErrorCodeCommonPartCpp::NoError || !IsValid(pLv2)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: get1stLvlActionPanelWidget() failed: %s, %p."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret), pLv2);
    }
    else {
        IMyMJGameInRoomOperationLvl2ChiPanelWidgetInterfaceCpp::Execute_updateChiChoiceDatas(pLv2, data.m_aChiDatas);
    }

    ret = IMyMJGameInRoomOperationRootPanelWidgetInterfaceCpp::Execute_setChoiceFillState(this, data.m_iCountOfChoices > 0);
    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setChoiceFillState() failed: %s."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
    }


    if (m_cUpdateStateLast.equal(data)) {
        //In normal case this should not happen
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("state equal: new %s, old %s."), *data.ToString(), *m_cUpdateStateLast.ToString());
    }

    m_cUpdateStateLast = data;
}



FMyErrorCodeMJGameCpp UMyMJGameInRoomUIMainWidgetBaseCpp::showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVsualType)
{
    UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = NULL;
    getInRoomPlayerInfoWidgetByIdxAttenderFromCache(idxAttender, pW, true);
    return pW->showAttenderWeave(dur, weaveVsualType);
};

FMyErrorCodeMJGameCpp UMyMJGameInRoomUIMainWidgetBaseCpp::showMyMJRoleDataAttenderPublicChanged(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic, int32 subType)
{
    FMyErrorCodeMJGameCpp ret(true);

    if (m_cRuntimeData.m_idxAttenderForIdxPositionInBox0 < 0 || m_cRuntimeData.m_idxAttenderForIdxPositionInBox0 >= 4) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cRuntimeData.m_idxAttenderForIdxPositionInBox0 not valid yet, now %d."), m_cRuntimeData.m_idxAttenderForIdxPositionInBox0);
        ret.join(MyErrorCodeCommonPartCpp::ParamInvalid);
        return ret;
    }

    if (m_cRuntimeData.m_idxAttenderForIdxPositionInBox0 != idxAttender) {
        return ret;
    }

    if (subType == MyMJGameCoreDataDirtySubType_AttenderStatePublic_HuScoreResultFinalGroup) {

    }


    return ret;
};

FMyErrorCodeMJGameCpp UMyMJGameInRoomUIMainWidgetBaseCpp::showMyMJRoleDataAttenderPrivateChanged(int32 idxAttender, const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate, int32 subType)
{
    FMyErrorCodeMJGameCpp ret(true);
    return ret;
};


FMyErrorCodeMJGameCpp UMyMJGameInRoomUIMainWidgetBaseCpp::updateAttenderPositions(float XYRatioOfplayerScreen, const TArray<FVector2D>& projectedPointsInPlayerScreen_unit_absolute)
{
    const FMyMJGameInRoomUIMainWidgetCfgCpp* pCfg = NULL;
    FMyErrorCodeMJGameCpp ret = getCfgRefConstFromCache(pCfg, false);
    if (ret.hasError()) {
        return ret;
    }
    MY_VERIFY(pCfg);


    TArray<const FMyAreaAlignToBorderCfgOneDimCpp*> boxAlignedAreas;
    for (int32 i = 0; i < pCfg->m_aBorders.Num(); i++) {
        boxAlignedAreas.Emplace(&pCfg->m_aBorders[i].m_cAreaAlignToBorder);
    }
    TArray<FMyAreaAlignToBorderMappedPointsWidget2DCpp> areaMappedPoints;

    FVector2D contentSizeMainUI;
    ret.join(IMyContentSizeWidget2DInterfaceCpp::Execute_getContentSizeFromCache(this, contentSizeMainUI, false));
    if (ret.hasError()) {
        return ret;
    }

    //UMyRenderUtilsLibrary::evaluateUserWidgetInCanvasPanelForModelInfo_Widget2D(UUserWidget* childWidget, FMyModelInfoWidget2DCpp& modelInfo);

    const FVector2D& UIExtend = contentSizeMainUI / 2;
    UMyRenderUtilsLibrary::helperGenMappedPointsForMyAreaAlignToBorders_Widget2D(UIExtend, UIExtend, boxAlignedAreas, false, false, areaMappedPoints);

    //Todo: hide object with room rule

    int32 l = projectedPointsInPlayerScreen_unit_absolute.Num();
    MY_VERIFY(l == 4);

    TArray<int32> aPositionInBoxs;
    TArray<float> aDeltas;
    UMyRenderUtilsLibrary::helperArrangePointsToPositionsInBoxEvenly_Widget2D(XYRatioOfplayerScreen, projectedPointsInPlayerScreen_unit_absolute, aPositionInBoxs, aDeltas);

    int32 idxAttenderForIdxPositionInBox0 = -1;

    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = NULL;
        ret.join(getInRoomPlayerInfoWidgetByIdxAttenderFromCache(idxAttender, pW, false));
        if (ret.hasError()) {
            return ret;
        }

        const FMyModelInfoWidget2DCpp* pModelInfoSub = NULL;
        ret.join(pW->getDataByCacheRefConst_MyModelInfoWidget2D(pModelInfoSub, false));
        if (ret.hasError()) {
            return ret;
        }

        const FMyModelInfoBoxWidget2DCpp& modelInfoBoxSub = pModelInfoSub->getBox2DRefConst();
        FVector2D location = UMyRenderUtilsLibrary::helperResolveLocationForMyAreaAlignToBorderMappedPoints_Widget2D(areaMappedPoints, modelInfoBoxSub, aPositionInBoxs[idxAttender], aDeltas[idxAttender]);

        FMyPlayerInfoWidgetRuntimeDataCpp& cRTData = pW->getRuntimeDataRef();

        cRTData.reset();
        cRTData.m_iIdxAttender = idxAttender; //no corresponding attender, set it later
        cRTData.m_iIdxPositionInBox = aPositionInBoxs[idxAttender];

        cRTData.m_cLocationSelf = location;
        cRTData.m_cLocationUICenter = UIExtend;

        MY_VERIFY(cRTData.m_iIdxPositionInBox >= 0 && cRTData.m_iIdxPositionInBox < 4);
        const FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp& commonShowPoint = pCfg->m_aBorders[cRTData.m_iIdxPositionInBox].m_cCommonEventShowPoint;

        cRTData.m_cLocationCommonActionShowPoint = (cRTData.m_cLocationSelf - cRTData.m_cLocationUICenter) * commonShowPoint.m_fShowPosiFromCenterToBorderPercent + cRTData.m_cLocationUICenter + UIExtend * 2 * commonShowPoint.m_cExtraOffsetScreenPercent;

        if (cRTData.m_iIdxPositionInBox == 0) {
            idxAttenderForIdxPositionInBox0 = idxAttender;
        }

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("setting player info widget %d to location %s."), idxAttender, *location.ToString());

        FWidgetTransform& rt = pW->RenderTransform;
        rt.Translation = location;
        pW->SetRenderTransform(rt);
    }


    AMyMJGamePlayerControllerCpp* pC = AMyMJGamePlayerControllerCpp::helperGetLocalController(this);

    bool bCanGiveCmd = idxAttenderForIdxPositionInBox0 == (int32)pC->getCmdRoleType();

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("idxAttenderForIdxPositionInBox0 %d, pC->getCmdRoleType() %d, dataRoleType %d."), idxAttenderForIdxPositionInBox0, (int32)pC->getCmdRoleType(), (int32)pC->getDataRoleType());

    if ((!m_cRuntimeData.m_bInited) || m_cRuntimeData.m_bCanGiveCmd != bCanGiveCmd) {
        IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_changeStateCanGiveCmd(this, bCanGiveCmd);
    }

    m_cRuntimeData.m_idxAttenderForIdxPositionInBox0 = idxAttenderForIdxPositionInBox0;
    m_cRuntimeData.m_bCanGiveCmd = bCanGiveCmd;
    m_cRuntimeData.m_bInited = true;

    return FMyErrorCodeMJGameCpp(true);
};

MyErrorCodeCommonPartCpp UMyMJGameInRoomUIMainWidgetBaseCpp::refillCachedData()
{
    MyErrorCodeCommonPartCpp retFinal = MyErrorCodeCommonPartCpp::NoError, ret = MyErrorCodeCommonPartCpp::NoError;

    m_cCachedData.reset();

    while (1) {

        FVector2D contentSizeMainUI;

        ret = IMyContentSizeWidget2DInterfaceCpp::Execute_getContentSize(this, contentSizeMainUI);
        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getContentSizeFromCache() returned error: %s."), *GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
            break;
        }

        FMyModelInfoWidget2DCpp& modelInfo = m_cCachedData.m_cModelInfo;
        modelInfo.reset(MyModelInfoType::BoxWidget2D);
        modelInfo.getBox2DRef() = UMyRenderUtilsLibrary::getMyModelInfoBoxByOriginPointRelativeToXYMinWidget2D(contentSizeMainUI / 2, contentSizeMainUI * MyUIWidgetPivotDefault);

        ret = IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp::Execute_getDynamicAllocationRootCanvasPanel(this, m_cCachedData.m_pDynamicAllocationCanvasPanel);
        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getDynamicAllocationRootCanvasPanel() returned error: %s."), *GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
            break;
        }

        if (!IsValid(m_cCachedData.m_pDynamicAllocationCanvasPanel)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getDynamicAllocationRootCanvasPanel() returned OK, but pointer not valid: %p."), *GetName(), m_cCachedData.m_pDynamicAllocationCanvasPanel);
            MyErrorCodeCommonPartJoin(ret, MyErrorCodeCommonPartCpp::ObjectNull);
            break;
        }

        ret = IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_getCfg(this, m_cCachedData.m_cCfg);
        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getCfg() returned error: %s."), *GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
            break;
        }

        break;
    }
    MyErrorCodeCommonPartJoin(retFinal, ret);


    m_cCachedData.m_aPlayerInfoWidgets.Reset();
    for (int32 i = 0; i < 4; i++) {
        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = NULL;

        ret = IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_getInRoomPlayerInfoWidgetByIdxAttender(this, i, pW);
        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getInRoomPlayerInfoWidgetByIdxAttender() with idx %d returned error: %s."), *GetName(), i, *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
            break;
        }
        if (pW == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getInRoomPlayerInfoWidgetByIdxAttender() returned OK, but pointer not valid: %p."), *GetName(), pW);
            ret = MyErrorCodeCommonPartCpp::ObjectNull;
            break;
        }

        m_cCachedData.m_aPlayerInfoWidgets.Emplace(pW);
    }
    MyErrorCodeCommonPartJoin(retFinal, ret);


    while (1) {
        ret = IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_getOperationRootPanelWidget(this, m_cCachedData.m_pOperationRootPanelWidget);
        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getOperationRootPanelWidget() returned error: %s."), *GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
            break;
        }

        if (m_cCachedData.m_pOperationRootPanelWidget == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getOperationRootPanelWidget() returned OK, but pointer not valid: %p."), *GetName(), m_cCachedData.m_pOperationRootPanelWidget);
            ret = MyErrorCodeCommonPartCpp::ObjectNull;
            break;
        }

        break;
    }
    MyErrorCodeCommonPartJoin(retFinal, ret);


    if (retFinal == MyErrorCodeCommonPartCpp::NoError) {
        m_cCachedData.m_bValid = true;
    }
    else {
        m_cCachedData.reset();
    }

    return retFinal;
}


void UMyMJGameInRoomUIMainWidgetBaseCpp::updateUI()
{
    const TSet<int32>& sD = m_cDirtyRecords.m_cCoreDataDirtyRecord.getRecordSetRefConst();
    
    if (!IsValid(m_pDataSourceRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: m_pDataSourceRoomActor not valid: %p."), *GetName(), m_pDataSourceRoomActor);
        return;
    }

    UMyMJGameDeskVisualDataObjCpp* pDesjVisualDataObj = m_pDataSourceRoomActor->getRoomDataSuiteVerified()->getDeskDataObjVerified();
    const FMyMJGameDeskVisualDataAllCpp& deskVisualDataAll = pDesjVisualDataObj->getDataAllRefConst();
    const FMyMJGameDeskVisualDataCpp& deskVisualDataNow = deskVisualDataAll.m_cDeskVisualDataNow;
    const FMyMJDataStructWithTimeStampBaseCpp& cCoreData = deskVisualDataNow.getCoreDataRefConst();

    for (auto& Elem : sD)
    {
        int32 v = Elem;

        int32 subIdx0, subIdx1, subIdx2;
        m_cDirtyRecords.m_cCoreDataDirtyRecord.recordValueToIdxValuesWith3Idxs(v, subIdx0, subIdx1, subIdx2);
        MyMJGameCoreDataDirtyMainTypeCpp eMainType = MyMJGameCoreDataDirtyMainTypeCpp(subIdx0);

        if (eMainType == MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePublic) {
            int32 idxAttender = subIdx1;
            const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic = cCoreData.getRoleDataAttenderPublicRefConst(idxAttender);



        }
        else if (eMainType == MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate) {
            int32 idxAttender = subIdx1;
            const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate = cCoreData.getRoleDataAttenderPrivateRefConst(idxAttender);

            if (subIdx2 == MyMJGameCoreDataDirtySubType_AttenderStatePrivate_ActionContainor) {
                if (idxAttender != m_cRuntimeData.m_idxAttenderForIdxPositionInBox0) {
                    continue;
                }

                UMyMJGameInRoomOperationRootPanelWidgetBaseCpp* pPanelWidget = NULL;
                getOperationRootPanelWidgetRefFromCache(pPanelWidget, false);
                if (pPanelWidget) {

                    const FMyMJCoreDataPublicCpp& coreDataPublic = cCoreData.getCoreDataPublicRefConst();
                    pPanelWidget->updateWithActionContainor(cCoreData.getRole(), coreDataPublic.m_cGameCfg.m_eRuleType, coreDataPublic.m_iGameId, coreDataPublic.m_iActionGroupId,
                                                            idxAttender, cCoreData.getRoleDataPrivateRefConst().m_cCardValuePack, dataAttenderPrivate.m_cActionContainor);
                }

                bool bNeedGiveOutCards = false;
                if (m_cRuntimeData.m_bCanGiveCmd) {
                    int32 al = dataAttenderPrivate.m_cActionContainor.m_aActionChoices.Num();
                    for (int32 idxActionChoice = 0; idxActionChoice < al; idxActionChoice++) {
                        const FMyMJGameActionUnfiedForBPCpp& actionUnified = dataAttenderPrivate.m_cActionContainor.m_aActionChoices[idxActionChoice];
                        if (actionUnified.getType() == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
                            bNeedGiveOutCards = true;
                            break;
                        }
                    }
                }

                if (m_cRuntimeData.m_bNeedGiveOutCards != bNeedGiveOutCards) {
                    IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_changeStateNeedGiveOutCards(this, bNeedGiveOutCards);
                    AMyMJGamePlayerControllerCpp* pPC = AMyMJGamePlayerControllerCpp::helperGetLocalController(this);
                    if (bNeedGiveOutCards) {
                        pPC->getSelectManagerVerified()->setSelectedActorNumMax(1, true);
                        pPC->setDragEndActionType(MyMJGamePlayerControllerDragEndActionTypeCpp::GiveOutCards);
                    }
                    else {
                        pPC->getSelectManagerVerified()->setSelectedActorNumMax(0, true);
                        pPC->setDragEndActionType(MyMJGamePlayerControllerDragEndActionTypeCpp::Invalid);
                    }
                }
                m_cRuntimeData.m_bNeedGiveOutCards = bNeedGiveOutCards;
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected dirty main type %d."), (int32)eMainType);
        }
    }

}