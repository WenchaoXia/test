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


void AMyMJGameCardActorBaseCpp::setSelected(bool selected)
{
    if (m_bSelected == selected) {
        return;
    }

    m_bSelected = selected;

    if (m_bSelected) {
        FTransform t;
        const FTransform* pTUnselected = NULL;
        if (m_pMyTransformUpdaterComponent->m_bHelperTransformUpdated) {
            pTUnselected = &m_pMyTransformUpdaterComponent->m_cHelperTransformUpdated;
        }
        else {
            t = GetActorTransform();
            pTUnselected = &t;
        }

        FMyModelInfoWorld3DCpp cModelInfo = getModelInfoForUpdaterEnsured();

        FVector newLocation = pTUnselected->GetLocation();
        newLocation.Z += cModelInfo.getBox3DRefConst().m_cBoxExtend.Z * 2 * m_fSelectedZOffsetPercent;

        FTransform newT = *pTUnselected;
        newT.SetLocation(newLocation);

        SetActorTransform(newT);
        m_cTransformWhenUnselected = *pTUnselected;
    }
    else {

        if (m_pMyTransformUpdaterComponent->m_bHelperTransformUpdated) {
            //If updated, always back to the latest position in case of it is updated after selection
            SetActorTransform(m_pMyTransformUpdaterComponent->m_cHelperTransformUpdated);
        }
        else {
            SetActorTransform(m_cTransformWhenUnselected);
        }

    }

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