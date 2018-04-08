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


void FMyMJGameCardVisualInfoAndResultCpp::helperResolveCardVisualResultChanges(const FMyCardGameVisualPointCfgCpp& cVisualPointCfg,
                                                                               const FMyModelInfoBox3DCpp& cCardModelInfo,
                                                                               const TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoKnownChanges,
                                                                               TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultAccumulatedChanges)
{

    for (auto& Elem : mIdCardVisualInfoKnownChanges)
    {
        int32 idCard = Elem.Key;
        const FMyMJGameCardVisualInfoCpp& cInfo = Elem.Value;


        FMyMJGameCardVisualInfoAndResultCpp& cNewInfoAndResult = mOutIdCardVisualInfoAndResultAccumulatedChanges.Emplace(idCard);
        cNewInfoAndResult.m_cVisualInfo = cInfo;

        FMyCardGameBoxLikeElemVisualInfoCpp::helperResolveTransform(cVisualPointCfg, cCardModelInfo, cInfo, cNewInfoAndResult.m_cVisualResult.m_cTransform);

        //post handle

        //Todo: do more about visible check
        cNewInfoAndResult.m_cVisualResult.m_bVisible = true;

    }
}


/*
//a step data support controlled roll
struct FMyWithCurveUpdateStepDataTransformAndRollCpp : public FMyWithCurveUpdateStepDataWorldTransformCpp
{

public:

    FMyWithCurveUpdateStepDataTransformAndRollCpp() : FMyWithCurveUpdateStepDataWorldTransformCpp()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformAndRollCpp");
        reset(true);
    };

    virtual ~FMyWithCurveUpdateStepDataTransformAndRollCpp()
    {
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            FMyWithCurveUpdateStepDataWorldTransformCpp::reset();
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


void AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(const TArray<AMyMJGameCardActorBaseCpp*>& aSub, bool bSort, TArray<IMyWithCurveUpdaterTransformInterfaceCpp*> &aBase)
{
    TArray<AMyMJGameCardActorBaseCpp*> aTemp;

    const TArray<AMyMJGameCardActorBaseCpp*>* pSrc = &aSub;

    if (bSort && aSub.Num() > 1)
    {
        aTemp = aSub;
        aTemp.Sort([](AMyMJGameCardActorBaseCpp& pA, AMyMJGameCardActorBaseCpp& pB) {
            return pA.getTargetToGoHistory(0, true)->m_cVisualInfo.m_iIdxColInRow < pB.getTargetToGoHistory(0, true)->m_cVisualInfo.m_iIdxColInRow;
        });
        pSrc = &aTemp;
    }

    int32 l = pSrc->Num();
    aBase.Reset();
    for (int32 i = 0; i < l; i++)
    {
        AMyCardGameCardActorBaseCpp* pA = (*pSrc)[i];
        IMyWithCurveUpdaterTransformInterfaceCpp* pI = Cast<IMyWithCurveUpdaterTransformInterfaceCpp>(pA);
        MY_VERIFY(pI != NULL);
        aBase.Emplace(pI);
    }
};



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