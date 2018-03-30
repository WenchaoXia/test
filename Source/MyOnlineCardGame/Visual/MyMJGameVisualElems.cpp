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

AMyMoveWithSeqActorBaseCpp::AMyMoveWithSeqActorBaseCpp() : Super()
{
    bNetLoadOnClient = true;

    m_bFakeUpdateSettings = false;

    createComponentsForCDO();
}

AMyMoveWithSeqActorBaseCpp::~AMyMoveWithSeqActorBaseCpp()
{

}


//Todo:: verify its correctness when root scene scaled
MyErrorCodeCommonPartCpp AMyMoveWithSeqActorBaseCpp::getModelInfo(struct FMyActorModelInfoBoxCpp& modelInfo, bool verify) const
{
    //ignore the root scene/actor's scale, but calc from the box

    //FVector actorScale3D = GetActorScale3D();
    //m_pMainBox->GetScaledBoxExtent()
    modelInfo.m_cBoxExtend = m_pMainBox->GetUnscaledBoxExtent() *  m_pMainBox->GetComponentScale();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("name %s, box scale %s."), *GetName(), *m_pMainBox->GetComponentScale().ToString());
    modelInfo.m_cCenterPointRelativeLocation = m_pMainBox->RelativeLocation;// * actorScale3D;

    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
    if (modelInfo.m_cBoxExtend.Size() < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class %s: model size is too small: %s."), *this->GetClass()->GetName(), *modelInfo.m_cBoxExtend.ToString());
        ret = MyErrorCodeCommonPartCpp::ModelSizeIncorrect;
    }

    if (verify) {
        MY_VERIFY(ret == MyErrorCodeCommonPartCpp::NoError);
    }

    return ret;
}

MyErrorCodeCommonPartCpp AMyMoveWithSeqActorBaseCpp::getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterTransformCpp*& outUpdater)
{
    MY_VERIFY(IsValid(m_pMyTransformUpdaterComponent));

    outUpdater = &m_pMyTransformUpdaterComponent->getMyWithCurveUpdaterTransformRef();

    return MyErrorCodeCommonPartCpp::NoError;
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

    m_pMyTransformUpdaterComponent = CreateDefaultSubobject<UMyTransformUpdaterComponent>(TEXT("transform updater component"));

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_pMainBox created as 0x%p, this 0x%p."), m_pMainBox, this);

}


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


MyErrorCodeCommonPartCpp AMyMoveWithSeqActorBaseCpp::updateSettings()
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

    return MyErrorCodeCommonPartCpp::NoError;
}


void AMyMoveWithSeqActorBaseCpp::helperTestAnimationStep(float time, FString debugStr, const TArray<AMyMoveWithSeqActorBaseCpp*>& actors)
{
    FMyTransformUpdateAnimationMetaCpp meta;
    FMyActorTransformUpdateAnimationStepCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = time;
    meta.m_cModelBoxExtend = FVector(20, 30, 60);

    stepData.m_fTimePercent = 1;
    stepData.m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::OffsetFromPrevLocation;
    stepData.m_cLocationOffsetPercent = FVector(0, 0, 1);

    TArray<FMyWithCurveUpdaterTransformCpp *> updaterSortedGroup;

    for (int32 i = 0; i < actors.Num(); i++)
    {
        FMyWithCurveUpdaterTransformCpp* pUpdater = &actors[i]->getMyWithCurveUpdaterTransformRef();
        FTransform targetT;
        targetT.SetLocation(FVector(0, 0, 100));
        pUpdater->setHelperTransformOrigin(actors[i]->GetActorTransform());
        pUpdater->setHelperTransformFinal(targetT);
        updaterSortedGroup.Emplace(pUpdater);
    }

    UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStep(meta, stepData, updaterSortedGroup);
}

/*
void AMyMoveWithSeqActorBaseCpp::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnConstruction m_pMainBox 0x%p, this 0x%p."), m_pMainBox, this);
}
*/

AMyMJGameCardBaseCpp::AMyMJGameCardBaseCpp() : Super(), m_cTargetToGoHistory(TEXT("card TargetToGoHistory"), NULL, NULL, 2)
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = uc->GetDefaultObject();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCardBaseCpp, %s, this: %p, uc %s, cdo %p."), *m_cResPath.Path, this, *uc->GetFullName(), CDO);

    m_iValueShowing = MyMJGameCardBaseCppDefaultShowingValue;
    m_iValueUpdatedBefore = MyMJGameCardBaseCppDefaultShowingValue - 1;

    m_cResPath.Path.Reset();
    m_pResMesh = NULL;
    m_pResMI = NULL;

    m_iMyId = -1;
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

    updateVisual(true);
}

void AMyMJGameCardBaseCpp::PostInitializeComponents()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostInitializeComponents, this %p, %s, %p, %p, compo: %p."), this, *m_cResPath.Path, m_pResMesh, m_pResMI, m_pMainStaticMesh);
    Super::PostInitializeComponents();

    updateVisual(true);
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

        updateVisual(false);
        //updateWithValue(m_iValueShowing);
    } 
    else {
        PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

        if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyMJGameCardBaseCpp, m_cResPath))
        {
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 2, this %p, %s"), this, *m_cResPath.Path);
            if (MyErrorCodeCommonPartCpp::NoError != checkAndLoadCardBasicResources(m_cResPath.Path)) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Retrying for default settings."));
                m_cResPath.Path = UMyCommonUtilsLibrary::getClassAssetPath(this->GetClass()) + TEXT("/") + MY_MODEL_RES_RELATIVE_PATH;
                checkAndLoadCardBasicResources(m_cResPath.Path);
            }
            updateVisual(true);
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::updateSettings()
{
    MyErrorCodeCommonPartCpp ret = Super::updateSettings();
    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        return ret;
    }

    FVector boxSize = m_pMainBox->GetScaledBoxExtent();
    FVector alignPoint = FVector::ZeroVector;

    alignPoint.X = -boxSize.X;
    alignPoint.Z = -boxSize.Z;

    m_pMainBox->SetRelativeLocation(-alignPoint);

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::checkAndLoadCardBasicResources(const FString &inPath)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
    if (inPath.IsEmpty()) {
        m_pResMesh = nullptr;
        m_pResMI = nullptr;
        m_cResPath.Path.Reset();

        return MyErrorCodeCommonPartCpp::NoError;
    }


    const FString &modelAssetPath = inPath;

    FString meshFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMesh;
    FString matDefaultInstFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMeshDefaultMI;

    UStaticMesh *pMeshAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(NULL, meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load mesh asset from %s."), *meshFullPathName);
        m_pResMesh = nullptr;
        ret = MyErrorCodeCommonPartCpp::AssetLoadFail;
    }
    else {
        m_pResMesh = pMeshAsset;
    }

    UMaterialInstance *pMatInstAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load material default instance asset from %s."), *matDefaultInstFullPathName);
        m_pResMI = nullptr;
        ret = MyErrorCodeCommonPartCpp::AssetLoadFail;
    }
    else {
        m_pResMI = pMatInstAsset;
    }

    m_cResPath.Path = modelAssetPath;

    return ret;
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::updateVisual(bool bForce)
{
    //if (m_cResPath.Path.IsEmpty()) {
        //return -1;
    //}
    MyErrorCodeCommonPartCpp ret, rett;

    ret = updateWithCardBasicResources();

    rett = updateWithValue(bForce);
    MyErrorCodePartJoin(ret, rett);

    return ret;
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::updateWithCardBasicResources()
{
    if (!IsValid(m_pMainBox)) {
        //UClass* uc = this->GetClass();
        //UObject* CDO = uc->GetDefaultObject();

        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMainBox invalid 0x%p, this 0x%p, cdo 0x%p."), m_pMainBox, this, CDO);
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
 
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::updateWithValue(bool bForce)
{
    if (m_iValueUpdatedBefore == m_iValueShowing && bForce == false) {
        return MyErrorCodeCommonPartCpp::NoError;
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

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::updateCardStaticMeshMIDParams(class UTexture* InBaseColor)
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
                return MyErrorCodeCommonPartCpp::NoError;
            }
            else {
                return MyErrorCodeCommonPartCpp::ObjectCastFail;
            }
        }

        class UTexture* baseColorNow = NULL;
        if (DynamicMaterial->GetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, baseColorNow)) {
            if (baseColorNow == InBaseColor) {
                //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("skip SetTextureParameterValue since target is same, this %p."), this);
                return MyErrorCodeCommonPartCpp::NoError;
            }
        }
        
        //if (InBaseColor == nullptr) {
            //test
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("doing test."));
            //m_pMainStaticMesh->SetMaterial(0, nullptr);
            //return 0;
        //}

        DynamicMaterial->SetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, InBaseColor);
        return MyErrorCodeCommonPartCpp::NoError;
    }
    else {
        if (InBaseColor == nullptr) {
            return MyErrorCodeCommonPartCpp::NoError;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("try to set MID param but material is NULL."));
            return MyErrorCodeCommonPartCpp::ObjectNull;
        }
    }

    return MyErrorCodeCommonPartCpp::NoError;
};

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::updateValueShowing(int32 newValueShowing, int32 animationTimeMs)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setValueShowing %d."), newValue);

    //always set immediately

    if (m_iValueShowing == newValueShowing) {
        return MyErrorCodeCommonPartCpp::NoError;
    }
    m_iValueShowing = newValueShowing;

    return updateWithValue(false);
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::getValueShowing(int32& valueShowing) const
{
    valueShowing = m_iValueShowing;

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::setResPath(const FDirectoryPath& newResPath)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setResPath %s."), *newResPath.Path);

    if (m_cResPath.Path == newResPath.Path) {
        return MyErrorCodeCommonPartCpp::NoError;
    }

    FDirectoryPath oldPath = m_cResPath;
    m_cResPath = newResPath;

    MyErrorCodeCommonPartCpp ret;
    ret = checkAndLoadCardBasicResources(m_cResPath.Path);

    if (ret == MyErrorCodeCommonPartCpp::NoError)
    {
    }
    else {
        m_cResPath = oldPath;
    }

    MyErrorCodePartJoin(ret, updateVisual(true));

    return ret;
}

MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::getResPath(FDirectoryPath& resPath) const
{
    resPath = m_cResPath;
    return MyErrorCodeCommonPartCpp::NoError;
}


MyErrorCodeCommonPartCpp AMyMJGameCardBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

    if (ppOutBaseColorTexture) {
        FString baseColorFullPathName = modelAssetPath + TEXT("/") + valuePrefix + MyCardAssetPartialNameSuffixValueBaseColorTexture;
        UTexture* pTBaseColor = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutBaseColorTexture = pTBaseColor;
        }
        else {
            *ppOutBaseColorTexture = NULL;
            ret = MyErrorCodeCommonPartCpp::AssetLoadFail;
        }
    }

    return ret;
};

void AMyMJGameCardBaseCpp::helperMyMJCardsToMyTransformUpdaters(const TArray<AMyMJGameCardBaseCpp*>& aSub, bool bSort, TArray<IMyTransformUpdaterInterfaceCpp*> &aBase)
{
    TArray<AMyMJGameCardBaseCpp*> aTemp;

    const TArray<AMyMJGameCardBaseCpp*>* pSrc = &aSub;

    if (bSort && aSub.Num() > 1)
    {
        aTemp = aSub;
        aTemp.Sort([](AMyMJGameCardBaseCpp& pA, AMyMJGameCardBaseCpp& pB) {
            return pA.getTargetToGoHistory(0, true)->m_cVisualInfo.m_iIdxColInRow < pB.getTargetToGoHistory(0, true)->m_cVisualInfo.m_iIdxColInRow;
        });
        pSrc = &aTemp;
    }

    int32 l = pSrc->Num();
    aBase.Reset();
    for (int32 i = 0; i < l; i++)
    {
        AMyMJGameCardBaseCpp* pA = (*pSrc)[i];
        IMyTransformUpdaterInterfaceCpp* pI = Cast<IMyTransformUpdaterInterfaceCpp>(pA);
        MY_VERIFY(pI != NULL);
        aBase.Emplace(pI);
    }
};


/*
//a step data support controlled roll
struct FMyWithCurveUpdateStepDataTransformAndRollCpp : public FMyWithCurveUpdateStepDataTransformCpp
{

public:

    FMyWithCurveUpdateStepDataTransformAndRollCpp() : FMyWithCurveUpdateStepDataTransformCpp()
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
            FMyWithCurveUpdateStepDataTransformCpp::reset();
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

AMyMJGameDiceBaseCpp::AMyMJGameDiceBaseCpp() : Super()
{
    m_iMyId = -1;
};

AMyMJGameDiceBaseCpp::~AMyMJGameDiceBaseCpp()
{

};

FTransform AMyMJGameDiceBaseCpp::helperCalcFinalTransform(const FMyMJDiceModelInfoBoxCpp& diceModelInfo, const FMyMJGameDeskVisualPointCfgCpp& diceVisualPointCfg, int32 diceVisualStateKey, int32 idxOfDiceRandomArranged, int32 diceTotalNum, int32 value)
{
    MY_VERIFY(idxOfDiceRandomArranged >= 0);
    MY_VERIFY(idxOfDiceRandomArranged < diceTotalNum);

    if (value < 1 || value > 6) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dice value is out of range: %d."), value);
        int32 key = diceVisualStateKey >= 0 ? diceVisualStateKey : -diceVisualStateKey;
        value = key % 6 + 1;
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("value overwrited as: %d."), value);
    }

    FRandomStream RS;
    RS.Initialize(diceVisualStateKey + idxOfDiceRandomArranged);

    float radiusMax = diceVisualPointCfg.m_cAreaBoxExtendFinal.Size2D();

    float radius = RS.FRandRange(0.1, 0.9) * radiusMax;
    float perDiceDegree = 360 / diceTotalNum;
    float perDiceDegreeHalfRandRange = perDiceDegree / 2 * 0.7;

    float degree = perDiceDegree * idxOfDiceRandomArranged + RS.FRandRange(-perDiceDegreeHalfRandRange, perDiceDegreeHalfRandRange);

    FVector localLoc = UKismetMathLibrary::RotateAngleAxis(FVector(radius, 0, 0), degree, FVector(0, 0, 1));
    FRotator localRot = diceModelInfo.m_cExtra.getLocalRotatorForDiceValueRefConst(value);

    localRot.Yaw = RS.FRandRange(0, 359);
    localRot.Yaw = RS.FRandRange(0, 359);

    FTransform localT, finalT;
    localT.SetLocation(localLoc);
    localT.SetRotation(localRot.Quaternion());

    finalT = localT * diceVisualPointCfg.m_cCenterPointWorldTransform;

    return finalT;
};


AMyMJGameTrivalDancingActorBaseCpp::AMyMJGameTrivalDancingActorBaseCpp() : Super()
{
};

AMyMJGameTrivalDancingActorBaseCpp::~AMyMJGameTrivalDancingActorBaseCpp()
{
};