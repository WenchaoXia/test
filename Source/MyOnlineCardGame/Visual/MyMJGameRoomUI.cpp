// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomUI.h"

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.h"

#include "Public/Blueprint/WidgetBlueprintLibrary.h"

#define MyMJGameWidgetMainTypeEventVisual    (0x00)
#define MyMJGameWidgetMainTypeIncidentVisual (0x01)
#define MyMJGameWidgetKeyCalculation(mainType, subType) (((mainType & 0xff) << 8) | (((uint8)subType) & 0xff))

int32 UMyMJGameInRoomPlayerInfoWidgetBaseCpp::showAttenderWeave(float dur, MyMJGameEventVisualTypeCpp weaveVisualType)
{
    const UMyMJGameInRoomVisualCfgCpp* pVisualCfg = AMyMJGameRoomLevelScriptActorCpp::helperGetVisualCfg(this, true);
    const FMyMJGameTrivalDataCpp* pTrivalData = AMyMJGameRoomLevelScriptActorCpp::helperGetMJGameTrivalData(this, true);

    int32 idxAttender = m_cRuntimeMeta.m_iIdxAttender;
    MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> widgetClass;
    widgetClass = pTrivalData->m_aAttenderDatas[idxAttender].m_cStyleSettings.m_cEvent.getWeaveWidgetByType(weaveVisualType);
    if (!IsValid(widgetClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("weave visual type have not set its corresponding widget class, using default one. visual type: %s, meta: %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameEventVisualTypeCpp"), (uint8)weaveVisualType), *m_cRuntimeMeta.ToString());
        widgetClass = pVisualCfg->m_cUICfg.m_cDefaultInRoomViewRoleStyle.m_cEvent.getWeaveWidgetByType(weaveVisualType);
    }

    MY_VERIFY(IsValid(widgetClass));

    int32 key;
    CalcKey_Macro_KeyAnd4IdxsMap(key, 0, 0, MyMJGameWidgetMainTypeEventVisual, (uint8)weaveVisualType);

    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* pW = getManagedWidget(key, widgetClass, true);
    
    IMyWidgetBasicOperationInterfaceCpp::Execute_canvasSetChildPosi(this, pW, FVector2D(0.5, 0.5));
    pW->SetVisibility(ESlateVisibility::Visible);

    FWidgetTransform wt;
    pW->SetRenderTransform(wt);

    IMyCardGameScreenPositionRelatedWidgetInterfaceCpp::Execute_restartMainAnimation(pW, m_cRuntimeMeta.m_iIdxScreenPosition, dur, m_cRuntimeMeta.m_cPosiCommonActionShowPoint - m_cRuntimeMeta.m_cPosiSelf, m_cRuntimeMeta.m_cPosiUICenter - m_cRuntimeMeta.m_cPosiSelf);

    return 0;
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

        if (GetClass()->ImplementsInterface(UMyWidgetBasicOperationInterfaceCpp::StaticClass()))
        {
            IMyWidgetBasicOperationInterfaceCpp::Execute_canvasAddChild(this, pW);
            pW->SetVisibility(ESlateVisibility::Hidden);
        }
        else {
            MY_VERIFY(false);
        }

    }

    if (createIfNotExist) {
        MY_VERIFY(pW);
    }

    return pW;
};




int32 UMyMJGameInRoomUIMainWidgetBaseCpp::showAttenderWeave_Implementation(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVsualType)
{
    UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = getInRoomPlayerInfoWidgetByIdxAttender(idxAttender, true);
    return pW->showAttenderWeave(dur, weaveVsualType);
};

int32 UMyMJGameInRoomUIMainWidgetBaseCpp::changeDeskPositionOfIdxScreenPosition0(int32 idxDeskPositionOfIdxScreenPosition0)
{
    MY_VERIFY(idxDeskPositionOfIdxScreenPosition0 >= 0 && idxDeskPositionOfIdxScreenPosition0 < 4);

    refillCachedData(idxDeskPositionOfIdxScreenPosition0);

    return 0;
};

void UMyMJGameInRoomUIMainWidgetBaseCpp::refillCachedData(int32 idxDeskPositionOfCamera)
{
    const UMyMJGameInRoomVisualCfgCpp* pVisualCfg = AMyMJGameRoomLevelScriptActorCpp::helperGetVisualCfg(this, true);
    const FMyMJGameTrivalDataCpp* pTrivalData = AMyMJGameRoomLevelScriptActorCpp::helperGetMJGameTrivalData(this, true);

    if (idxDeskPositionOfCamera < 0) {
        idxDeskPositionOfCamera = m_cCachedData.m_idxDeskPositionOfCamera;
    }

    m_cCachedData.reset();

    m_cCachedData.m_idxDeskPositionOfCamera = idxDeskPositionOfCamera;

    //Todo: support 2, 3, 4 in game config
    int32 attenderNumber = pTrivalData->m_cGameCfg.m_iAttenderNumber;
    MY_VERIFY(attenderNumber >= 2 && attenderNumber <= 4);
    m_cCachedData.m_iAttenderNumber = attenderNumber;

    IMyWidgetBasicOperationInterfaceCpp::Execute_getLocalSize(this, m_cCachedData.m_cMainUILocalSize);

    m_cCachedData.m_aPlayerInfoWidgetsOnScreen.Reset();
    for (int32 i = 0; i < 4; i++) {
        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = IMyMJGameInRoomUIMainInterfaceCpp::Execute_getInRoomPlayerInfoWidgetByScreenPosition(this, i);
        if (!IsValid(pW)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get player info widget, screen position %d, result %p."), i, pW);
            MY_VERIFY(false);
        }
        m_cCachedData.m_aPlayerInfoWidgetsOnScreen.Emplace(pW);

        FMyPlayerInfoWidgetRuntimeMetaCpp& cMeta = pW->getRuntimeMetaRef();
        cMeta.reset();
        cMeta.m_iIdxAttender = -1; //no corresponding attender, set it later
        cMeta.m_iIdxScreenPosition = i;
        
        UMyRenderUtilsLibrary::getCenterPointPositionForWidgetInCanvasWithPointAnchor(m_cCachedData.m_cMainUILocalSize, pW, cMeta.m_cPosiSelf);
        cMeta.m_cPosiUICenter = m_cCachedData.m_cMainUILocalSize / 2;

        const FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp& widgetPoint = pVisualCfg->m_cUICfg.m_aInRoomUIScreenPositionCfgs[cMeta.m_iIdxScreenPosition].m_cCommonEventShowPoint;
        cMeta.m_cPosiCommonActionShowPoint = (cMeta.m_cPosiSelf - cMeta.m_cPosiUICenter) * widgetPoint.m_fShowPosiFromCenterToBorderPercent + cMeta.m_cPosiUICenter + m_cCachedData.m_cMainUILocalSize * widgetPoint.m_cExtraOffsetScreenPercent;
    }

    //set the used and unused ones
    for (int32 i = 0; i < attenderNumber; i++) {
        int32 idxScreenPosition = m_cCachedData.idxAttenderToIdxScreenPosition(i);
        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = m_cCachedData.m_aPlayerInfoWidgetsOnScreen[idxScreenPosition];

        FMyPlayerInfoWidgetRuntimeMetaCpp& cMeta = pW->getRuntimeMetaRef();

        cMeta.m_iIdxAttender = i;

        pW->SetVisibility(ESlateVisibility::Visible);
    }

    for (int32 i = 0; i < 4; i++) {
        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = m_cCachedData.m_aPlayerInfoWidgetsOnScreen[i];

        FMyPlayerInfoWidgetRuntimeMetaCpp& cMeta = pW->getRuntimeMetaRef();
        if (cMeta.m_iIdxAttender >= 0) {
            continue;
        }

        pW->SetVisibility(ESlateVisibility::Hidden);
    }

    m_cCachedData.m_bValid = true;
}