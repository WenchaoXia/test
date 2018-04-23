// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomUI.h"

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.h"

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
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("weave visual type have not set its corresponding widget class, using default one. visual type: %s, meta: %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameEventVisualTypeCpp"), (uint8)weaveVisualType), *m_cRuntimeData.ToString());
        widgetClass = pVisualCfg->m_cUICfg.m_cDefaultInRoomViewRoleStyle.m_cEvent.getWeaveWidgetByType(weaveVisualType);
    }

    MY_VERIFY(IsValid(widgetClass));

    int32 key;
    CalcKey_Macro_KeyAnd4IdxsMap(key, 0, 0, MyMJGameWidgetMainTypeEventVisual, (uint8)weaveVisualType);

    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* pW = getManagedWidget(key, widgetClass, true);
    
    pW->SetVisibility(ESlateVisibility::Visible);

    FWidgetTransform wt;
    pW->SetRenderTransform(wt);

    IMyCardGameScreenPositionRelatedWidgetInterfaceCpp::Execute_restartMainAnimation(pW, m_cRuntimeData.m_iIdxPositionInBox, dur, m_cRuntimeData.m_cLocationCommonActionShowPoint - m_cRuntimeData.m_cLocationSelf, m_cRuntimeData.m_cLocationUICenter - m_cRuntimeData.m_cLocationSelf);

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




FMyErrorCodeMJGameCpp UMyMJGameInRoomUIMainWidgetBaseCpp::showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVsualType)
{
    UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = NULL;
    getInRoomPlayerInfoWidgetByIdxAttenderFromCache(idxAttender, pW, true);
    return pW->showAttenderWeave(dur, weaveVsualType);
};

FMyErrorCodeMJGameCpp UMyMJGameInRoomUIMainWidgetBaseCpp::showMyMJRoleDataAttenderPublicChanged(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic, int32 subType)
{
    FMyErrorCodeMJGameCpp ret(true);
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

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("setting player info widget %d to location %s."), idxAttender, *location.ToString());

        FWidgetTransform& rt = pW->RenderTransform;
        rt.Translation = location;
        pW->SetRenderTransform(rt);
    }

    m_cRuntimeData.m_idxAttenderForIdxPositionInBox0 = idxAttenderForIdxPositionInBox0;

    return FMyErrorCodeMJGameCpp(true);
};

MyErrorCodeCommonPartCpp UMyMJGameInRoomUIMainWidgetBaseCpp::refillCachedData()
{
    MyErrorCodeCommonPartCpp ret;

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
            ret = MyErrorCodeCommonPartCpp::ObjectNull;
            break;
        }

        ret = IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_getCfg(this, m_cCachedData.m_cCfg);
        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getCfg() returned error: %s."), *GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
            break;
        }

        break;
    }

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

    if (ret == MyErrorCodeCommonPartCpp::NoError) {
        m_cCachedData.m_bValid = true;
    }
    else {
        m_cCachedData.reset();
    }

    return ret;
}