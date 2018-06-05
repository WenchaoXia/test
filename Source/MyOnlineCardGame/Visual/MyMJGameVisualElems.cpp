// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameVisualElems.h"

#include "Kismet/GameplayStatics.h"

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


void FMyMJGameCardVisualInfoAndResultCpp::helperResolveCardVisualResultChanges(const FMyArrangePointCfgWorld3DCpp& cVisualPointCfg,
                                                                               const FMyModelInfoBoxWorld3DCpp& cCardModelInfo,
                                                                               const TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoKnownChanges,
                                                                               TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultAccumulatedChanges)
{
    FMyArrangePointResolvedMetaWorld3DCpp meta = UMyRenderUtilsLibrary::genMyArrangePointResolvedMetaWorld3D_World3D(cVisualPointCfg, cCardModelInfo);

    for (auto& Elem : mIdCardVisualInfoKnownChanges)
    {
        int32 idCard = Elem.Key;
        const FMyMJGameCardVisualInfoCpp& cInfo = Elem.Value;


        FMyMJGameCardVisualInfoAndResultCpp& cNewInfoAndResult = mOutIdCardVisualInfoAndResultAccumulatedChanges.Emplace(idCard);
        cNewInfoAndResult.m_cVisualInfo = cInfo;

        UMyRenderUtilsLibrary::helperBoxModelResolveTransformWorld3D(meta, cInfo, cNewInfoAndResult.m_cVisualResult.m_cTransform);

        //post handle

        //Todo: do more about visible check
        cNewInfoAndResult.m_cVisualResult.m_bVisible = true;

    }
}


/*
//a step data support controlled roll
struct FMyWithCurveUpdateStepDataTransformAndRollCpp : public FMyWithCurveUpdateStepDataTransformWorld3DCpp
{

public:

    FMyWithCurveUpdateStepDataTransformAndRollCpp() : FMyWithCurveUpdateStepDataTransformWorld3DCpp()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformAndRollCpp");
        reset(true);
    };

    virtual ~FMyWithCurveUpdateStepDataTransformAndRollCpp()
    {
    };

    inline void reset(bool resetSubClassDataOnly = false)
    {
        if (!resetSubClassDataOnly) {
            FMyWithCurveUpdateStepDataTransformWorld3DCpp::reset();
        }

        totalTime = 1;
        g = 0.98;
        rollAxis = false; //whether x or y

        FTransform t;
        start = end = t;
    };

    virtual FMyWithCurveUpdateStepDataBasicCpp* createOnHeap() override
    {
        return new FMyWithCurveUpdateStepDataTransformAndRollCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataTransformAndRollCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataTransformAndRollCpp *>(&other);
        *this = *pOther;
    };

    float totalTime;
    float g;
    bool rollAxis; //whether x or y


    FTransform start;
    FTransform end;
};
*/


MyErrorCodeCommonPartCpp AMyMJGameCardActorBaseCpp::setSelected(bool selected)
{
    if (m_bSelected == selected) {
        return MyErrorCodeCommonPartCpp::NoError;
    }

    m_bSelected = selected;


    if (m_bSelected) {

        bool selectable;
        getIsSelectable(selectable);
        if (!selectable) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: card is not selectable now, can't be set selected."), *GetName());
            return MyErrorCodeCommonPartCpp::InternalError;
        }

        FTransform t;
        const FTransform* pTUnselected = NULL;
        if (m_pMyTransformUpdaterComponent->m_bHelperTransformUpdated) {
            pTUnselected = &m_pMyTransformUpdaterComponent->m_cHelperTransformUpdated;
        }
        else {
            t = GetActorTransform();
            pTUnselected = &t;
        }

        m_cTransformBeforeSelection = *pTUnselected;

        FMyModelInfoWorld3DCpp cModelInfo = getModelInfoForUpdaterEnsured();

        FVector newLocation = pTUnselected->GetLocation();
        newLocation.Z += cModelInfo.getBox3DRefConst().m_cBoxExtend.Z * 2 * m_fSelectedZOffsetPercent;

        FTransform cTransformBeginSelection = *pTUnselected;
        cTransformBeginSelection.SetLocation(newLocation);

        tryMoveTransformForSelection(cTransformBeginSelection, 0.05);

    }
    else {

        tryMoveTransformForSelection(m_cTransformBeforeSelection, 0.1);

        /*
        if (m_pMyTransformUpdaterComponent->m_bHelperTransformUpdated) {
            //If updated, always back to the latest position in case of it is updated after selection
            tryMoveTransformForSelection(m_pMyTransformUpdaterComponent->m_cHelperTransformUpdated, 0.5);
        }
        else {
            tryMoveTransformForSelection(m_cTransformBeforeSelection, 0.5);
        }
        */

    }

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardActorBaseCpp::getSelected(bool &selected) const
{
    selected = m_bSelected;
    return MyErrorCodeCommonPartCpp::NoError;
}


MyErrorCodeCommonPartCpp AMyMJGameCardActorBaseCpp::setIsSelectable(bool selectable)
{
    if (m_bSelectable != selectable) {
        m_bSelectable = selectable;

        if (m_bSelectable == false) {
            setSelected(false);
        }

    }

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardActorBaseCpp::getIsSelectable(bool &selectable) const
{
    //AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp::getMyWithCurveUpdaterTransformRef()->get
    const FMyWithCurveUpdaterTransformWorld3DCpp& updater = getMyWithCurveUpdaterTransformRefConst();

    selectable = m_bSelectable && updater.getStepsCount() == 0;

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardActorBaseCpp::markBeginDrag()
{
    APlayerController *pC = UGameplayStatics::GetPlayerController(this, 0);
    MY_VERIFY(pC != NULL);

    m_bProjectionOKBeginDrag = false;
    m_cTransformBeginDrag = GetActorTransform();
    FVector locationBeginDrag = m_cTransformBeginDrag.GetLocation();
    if (UGameplayStatics::ProjectWorldToScreen(pC, locationBeginDrag, m_cProjectedScreenPositionBeginDrag)) {
        FVector worldPosition, worldDirection;
        if (UGameplayStatics::DeprojectScreenToWorld(pC, m_cProjectedScreenPositionBeginDrag, worldPosition, worldDirection)) {
            m_fProjectedDistanceBeginDrag = (locationBeginDrag - worldPosition).Size();
            m_bProjectionOKBeginDrag = true;

            FVector debugLocationRecal = worldPosition + worldDirection * m_fProjectedDistanceBeginDrag;

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s: debug locationBeginDrag %s, worldPosition %s, worldDirection %s, debugLocationRecal %s."), *GetName(), *locationBeginDrag.ToString(), *worldPosition.ToString(), *worldDirection.ToString(), *debugLocationRecal.ToString());
        }
    }

    if (!m_bProjectionOKBeginDrag) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: markBeginDrag() failed in screen projection."), *GetName());
    }

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardActorBaseCpp::getDataBeginDrag(FTransform& transform, bool& projectionOK, FVector2D& projectedScreenPosition, float& projectedDistance) const
{
    transform = m_cTransformBeginDrag;
    projectionOK = m_bProjectionOKBeginDrag;
    projectedScreenPosition = m_cProjectedScreenPositionBeginDrag;
    projectedDistance = m_fProjectedDistanceBeginDrag;

    bool selected;
    getSelected(selected);
    if (!selected) {
        projectionOK = false;
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: card is not selected, can't getDataBeginSelection()."), *GetName());
        return MyErrorCodeCommonPartCpp::InternalError;
    }

    return MyErrorCodeCommonPartCpp::NoError;
}


void AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(const TArray<AMyMJGameCardActorBaseCpp*>& aSub, bool bSort, TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> &aBase)
{
    TArray<AMyMJGameCardActorBaseCpp*> aTemp;

    const TArray<AMyMJGameCardActorBaseCpp*>* pSrc = &aSub;

    if (bSort && aSub.Num() > 1)
    {
        aTemp = aSub;
        aTemp.Sort([](AMyMJGameCardActorBaseCpp& pA, AMyMJGameCardActorBaseCpp& pB) {
            return pA.getTargetToGoHistory(0, true)->m_cVisualInfo.m_cCol.m_iIdxElem < pB.getTargetToGoHistory(0, true)->m_cVisualInfo.m_cCol.m_iIdxElem;
        });
        pSrc = &aTemp;
    }

    int32 l = pSrc->Num();
    aBase.Reset();
    for (int32 i = 0; i < l; i++)
    {
        AMyCardGameCardActorBaseCpp* pA = (*pSrc)[i];
        IMyWithCurveUpdaterTransformWorld3DInterfaceCpp* pI = Cast<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp>(pA);
        MY_VERIFY(pI != NULL);
        aBase.Emplace(pI);
    }
};

void AMyMJGameCardActorBaseCpp::BeginPlay()
{
    Super::BeginPlay();
    m_bSelected = false;
}

void AMyMJGameCardActorBaseCpp::tryMoveTransformForSelection(const FTransform& targetTransform, float dur)
{
    FMyWithCurveUpdaterTransformWorld3DCpp* pUpdater = &getMyWithCurveUpdaterTransformRef();
    if (pUpdater->getStepsCount() > 0) {
        SetActorTransform(targetTransform);
    }
    else {
        UCurveVector* pCurve = UMyCommonUtilsLibrary::getCurveVectorByType(MyCurveAssetType::DefaultLinear);

        FMyWithCurveUpdateStepDataTransformWorld3DCpp data;
        data.helperSetDataBySrcAndDst(dur, pCurve, GetTransform(), targetTransform);
        pUpdater->clearSteps();
        pUpdater->addStepToTail(data);
    }
}




AMyMJGameDiceBaseCpp::AMyMJGameDiceBaseCpp() : Super()
{
};

AMyMJGameDiceBaseCpp::~AMyMJGameDiceBaseCpp()
{

};



AMyMJGameTrivalDancingActorBaseCpp::AMyMJGameTrivalDancingActorBaseCpp() : Super()
{
};

AMyMJGameTrivalDancingActorBaseCpp::~AMyMJGameTrivalDancingActorBaseCpp()
{
};