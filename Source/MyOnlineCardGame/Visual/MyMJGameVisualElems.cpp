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
int32 AMyMoveWithSeqActorBaseCpp::getModelInfo(FMyActorModelInfoBoxCpp& modelInfo, bool verify) const
{
    //ignore the root scene/actor's scale, but calc from the box

    //FVector actorScale3D = GetActorScale3D();
    //m_pMainBox->GetScaledBoxExtent()
    modelInfo.m_cBoxExtend = m_pMainBox->GetUnscaledBoxExtent() *  m_pMainBox->GetComponentScale();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("name %s, box scale %s."), *GetName(), *m_pMainBox->GetComponentScale().ToString());
    modelInfo.m_cCenterPointRelativeLocation = m_pMainBox->RelativeLocation;// * actorScale3D;

    int32 ret = 0;
    if (modelInfo.m_cBoxExtend.Size() < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class %s: model size is too small: %s."), *this->GetClass()->GetName(), *modelInfo.m_cBoxExtend.ToString());
        ret = -1;
    }

    if (verify) {
        MY_VERIFY(ret == 0);
    }

    return ret;
}

UMyTransformUpdateSequenceMovementComponent* AMyMoveWithSeqActorBaseCpp::getTransformUpdateSequence(bool verify)
{
    UMyTransformUpdateSequenceMovementComponent* pRet = m_pTransformUpdateSequence;

    if (verify)
    {
        MY_VERIFY(IsValid(pRet));
    }
    return pRet;
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

    m_pTransformUpdateSequence = CreateDefaultSubobject<UMyTransformUpdateSequenceMovementComponent>(TEXT("transform update sequence movement component"));

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


int32 AMyMoveWithSeqActorBaseCpp::updateSettings()
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

    return 0;
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

    TArray<UMyTransformUpdateSequenceMovementComponent *> actorComponentsSortedGroup;

    for (int32 i = 0; i < actors.Num(); i++)
    {
        UMyTransformUpdateSequenceMovementComponent* pComp = actors[i]->getTransformUpdateSequence();
        FTransform targetT;
        targetT.SetLocation(FVector(0, 0, 100));
        pComp->setHelperTransformFinal(targetT);
        actorComponentsSortedGroup.Emplace(pComp);
    }

    UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStep(meta, stepData, actorComponentsSortedGroup);
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

    updateVisual();
}

void AMyMJGameCardBaseCpp::PostInitializeComponents()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostInitializeComponents, this %p, %s, %p, %p, compo: %p."), this, *m_cResPath.Path, m_pResMesh, m_pResMI, m_pMainStaticMesh);
    Super::PostInitializeComponents();

    updateVisual();
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

        updateVisual();
        //updateWithValue(m_iValueShowing);
    } 
    else {
        PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

        if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyMJGameCardBaseCpp, m_cResPath))
        {
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 2, this %p, %s"), this, *m_cResPath.Path);
            if (0 != checkAndLoadCardBasicResources(m_cResPath.Path)) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Retrying for default settings."));
                m_cResPath.Path = UMyCommonUtilsLibrary::getClassAssetPath(this->GetClass()) + TEXT("/") + MY_MODEL_RES_RELATIVE_PATH;
                checkAndLoadCardBasicResources(m_cResPath.Path);
            }
            updateVisual();
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif

int32 AMyMJGameCardBaseCpp::updateSettings()
{
    int32 ret = Super::updateSettings();
    if (ret != 0) {
        return ret;
    }

    FVector boxSize = m_pMainBox->GetScaledBoxExtent();
    FVector alignPoint = FVector::ZeroVector;

    alignPoint.X = -boxSize.X;
    alignPoint.Z = -boxSize.Z;

    m_pMainBox->SetRelativeLocation(-alignPoint);

    return 0;
}

int32 AMyMJGameCardBaseCpp::checkAndLoadCardBasicResources(const FString &inPath)
{
    int32 ret = 0;
    if (inPath.IsEmpty()) {
        m_pResMesh = nullptr;
        m_pResMI = nullptr;
        m_cResPath.Path.Reset();

        return 0;
    }


    const FString &modelAssetPath = inPath;

    FString meshFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMesh;
    FString matDefaultInstFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMeshDefaultMI;

    UStaticMesh *pMeshAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(NULL, meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load mesh asset from %s."), *meshFullPathName);
        m_pResMesh = nullptr;
        ret |= 0x01;
    }
    else {
        m_pResMesh = pMeshAsset;
    }

    UMaterialInstance *pMatInstAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load material default instance asset from %s."), *matDefaultInstFullPathName);
        m_pResMI = nullptr;
        ret |= 0x10;
    }
    else {
        m_pResMI = pMatInstAsset;
    }

    m_cResPath.Path = modelAssetPath;

    return ret;
}

int32 AMyMJGameCardBaseCpp::updateVisual()
{
    //if (m_cResPath.Path.IsEmpty()) {
        //return -1;
    //}

    updateWithCardBasicResources();
    updateWithValue();

    return 0;
}

int32 AMyMJGameCardBaseCpp::updateWithCardBasicResources()
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
 
    return 0;
}

int32 AMyMJGameCardBaseCpp::updateWithValue()
{
    if (m_iValueUpdatedBefore == m_iValueShowing) {
        return 0;
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

int32 AMyMJGameCardBaseCpp::updateCardStaticMeshMIDParams(class UTexture* InBaseColor)
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
                return 0;
            }
            else {
                return -30;
            }
        }

        class UTexture* baseColorNow = NULL;
        if (DynamicMaterial->GetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, baseColorNow)) {
            if (baseColorNow == InBaseColor) {
                //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("skip SetTextureParameterValue since target is same, this %p."), this);
                return 0;
            }
        }
        
        //if (InBaseColor == nullptr) {
            //test
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("doing test."));
            //m_pMainStaticMesh->SetMaterial(0, nullptr);
            //return 0;
        //}

        DynamicMaterial->SetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, InBaseColor);
        return 0;
    }
    else {
        if (InBaseColor == nullptr) {
            return 0;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("try to set MID param but material is NULL."));
            return -31;
        }
    }
};

void AMyMJGameCardBaseCpp::setValueShowing(int32 newValue)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setValueShowing %d."), newValue);

    if (m_iValueShowing == newValue) {
        return;
    }
    m_iValueShowing = newValue;

    updateWithValue();
}

int32 AMyMJGameCardBaseCpp::getValueShowing() const
{
    return m_iValueShowing;
}

void AMyMJGameCardBaseCpp::setResPath(const FDirectoryPath& newResPath)
{
    setResPathWithRet(newResPath);
}

bool AMyMJGameCardBaseCpp::setResPathWithRet(const FDirectoryPath& newResPath)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setResPath %s."), *newResPath.Path);

    if (m_cResPath.Path == newResPath.Path) {
        return true;
    }

    bool ret = true;
    FDirectoryPath oldPath = m_cResPath;
    m_cResPath = newResPath;

    if (0 == checkAndLoadCardBasicResources(m_cResPath.Path))
    {
    }
    else {
        m_cResPath = oldPath;
        ret = false;
    }

    updateVisual();

    return ret;
}

const FDirectoryPath& AMyMJGameCardBaseCpp::getResPath() const
{
    return m_cResPath;
}

bool AMyMJGameCardBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture)
{
    bool bRet = true;

    if (ppOutBaseColorTexture) {
        FString baseColorFullPathName = modelAssetPath + TEXT("/") + valuePrefix + MyCardAssetPartialNameSuffixValueBaseColorTexture;
        UTexture* pTBaseColor = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutBaseColorTexture = pTBaseColor;
        }
        else {
            *ppOutBaseColorTexture = NULL;
            bRet = false;
        }
    }

    return bRet;
};

void AMyMJGameCardBaseCpp::helperToSeqActors(const TArray<AMyMJGameCardBaseCpp*>& aSub, bool bSort, TArray<IMyTransformUpdateSequenceInterface*> &aBase)
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
        IMyTransformUpdateSequenceInterface* pI = Cast<IMyTransformUpdateSequenceInterface>(pA);
        MY_VERIFY(pI != NULL);
        aBase.Emplace(pI);
    }
};



struct FMyControlledRollExtraDataCpp : public FTransformUpdateSequencExtraDataBaseCpp
{

public:

    FMyControlledRollExtraDataCpp() : FTransformUpdateSequencExtraDataBaseCpp()
    {
    };

    virtual ~FMyControlledRollExtraDataCpp()
    {
    };

    virtual FTransformUpdateSequencExtraDataBaseCpp* createOnHeap() override
    {
        FMyControlledRollExtraDataCpp* ret = new FMyControlledRollExtraDataCpp();
        MY_VERIFY(ret);
        return ret;
    };

    virtual void copyContentFrom(const FTransformUpdateSequencExtraDataBaseCpp& other) override
    {
        const FMyControlledRollExtraDataCpp *pOther = StaticCast<const FMyControlledRollExtraDataCpp *>(&other);
        *this = *pOther;
    };

    float totalTime;
    float g;
    bool rollAxis; //whether x or y


    FTransform start;
    FTransform end;
};

AMyMJGameDiceBaseCpp::AMyMJGameDiceBaseCpp() : Super()
{

};

AMyMJGameDiceBaseCpp::~AMyMJGameDiceBaseCpp()
{

};

void AMyMJGameDiceBaseCpp::onTransformSeqUpdated(const struct FTransformUpdateSequencDataCpp& data, const FVector& vector)
{

};

void AMyMJGameDiceBaseCpp::onTransformSeqFinished(const struct FTransformUpdateSequencDataCpp& data)
{

};


AMyMJGameTrivalDancingActorBaseCpp::AMyMJGameTrivalDancingActorBaseCpp() : Super()
{
};

AMyMJGameTrivalDancingActorBaseCpp::~AMyMJGameTrivalDancingActorBaseCpp()
{
};