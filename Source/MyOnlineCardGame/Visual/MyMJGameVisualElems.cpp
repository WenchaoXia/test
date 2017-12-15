// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameVisualElems.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"


AMyMJGameCardBaseCpp::AMyMJGameCardBaseCpp() : Super()
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = uc->GetDefaultObject();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameCardBaseCpp, %s, this: %p, uc %s, cdo %p."), *m_cResPath.Path, this, *uc->GetFullName(), CDO);

    bNetLoadOnClient = true;


    m_iValueShowing = MyMJGameCardBaseCppDefaultShowingValue;
    m_iValueUpdatedBefore = MyMJGameCardBaseCppDefaultShowingValue - 1;

    m_pRootScene = NULL;
    m_pCardBox = NULL;
    m_pCardStaticMesh = NULL;

    m_cResPath.Path.Reset();
    m_pResMesh = NULL;
    m_pResMI = NULL;

    
    createComponentsForCDO();
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
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostInitializeComponents, this %p, %s, %p, %p, compo: %p."), this, *m_cResPath.Path, m_pResMesh, m_pResMI, m_pCardStaticMesh);
    Super::PostInitializeComponents();

    updateVisual();
}

#if WITH_EDITOR

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
            checkAndLoadCardBasicResources(m_cResPath.Path);
            updateVisual();
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif


void AMyMJGameCardBaseCpp::createComponentsForCDO()
{

    USceneComponent* pRootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    MY_VERIFY(IsValid(pRootSceneComponent));
    RootComponent = pRootSceneComponent;
    m_pRootScene = pRootSceneComponent;


    UBoxComponent* pBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    MY_VERIFY(IsValid(pBoxComponent));
    pBoxComponent->SetupAttachment(m_pRootScene);
    pBoxComponent->SetCollisionProfileName(TEXT("CollistionProfileBox"));
    pBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); //by default disable collision
    m_pCardBox = pBoxComponent;


    UStaticMeshComponent* pStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardStaticMesh"));
    MY_VERIFY(IsValid(pStaticMeshComponent));
    pStaticMeshComponent->SetupAttachment(m_pCardBox);
    pStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    m_pCardStaticMesh = pStaticMeshComponent;

}

int32 AMyMJGameCardBaseCpp::checkAndLoadCardBasicResources(const FString &inPath)
{
    if (inPath.IsEmpty()) {
        m_pResMesh = nullptr;
        m_pResMI = nullptr;
        m_cResPath.Path.Reset();

        return 0;
    }


    const FString &modelAssetPath = inPath;

    FString meshFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMesh;
    FString matDefaultInstFullPathName = modelAssetPath + TEXT("/") + MyCardAssetPartialNameStaticMeshDefaultMI;

    UStaticMesh *pMeshAsset = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(NULL, meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load mesh asset from %s."), *meshFullPathName);
        m_pResMesh = nullptr;
    }
    else {
        m_pResMesh = pMeshAsset;
    }

    UMaterialInstance *pMatInstAsset = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load material default instance asset from %s."), *matDefaultInstFullPathName);
        m_pResMI = nullptr;
    }
    else {
        m_pResMI = pMatInstAsset;
    }

    m_cResPath.Path = modelAssetPath;

    return 0;
}

int32 AMyMJGameCardBaseCpp::updateVisual()
{
    updateWithCardBasicResources();
    updateWithValue();

    return 0;
}

int32 AMyMJGameCardBaseCpp::updateWithCardBasicResources()
{
    MY_VERIFY(IsValid(m_pCardBox));
    MY_VERIFY(IsValid(m_pCardStaticMesh));

    UStaticMesh* pMeshNow = m_pCardStaticMesh->GetStaticMesh();
    if (pMeshNow != m_pResMesh) {

        FVector boxSizeFix(0), boxSizeFixPivotOffset(0), boxOrigin(0);

        if (IsValid(m_pResMesh)) {
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("updating mesh and material for this %p."), this);

            FBox box = m_pResMesh->GetBoundingBox();
            FVector boxSize = box.Max - box.Min;

            boxSizeFix.X = UKismetMathLibrary::FCeil(boxSize.X) / 2;
            boxSizeFix.Y = UKismetMathLibrary::FCeil(boxSize.Y) / 2;
            boxSizeFix.Z = UKismetMathLibrary::FCeil(boxSize.Z) / 2;

            //boxSizeFixPivotOffset = boxSizeFix;
            boxSizeFixPivotOffset.X = -boxSizeFix.X;
            boxSizeFixPivotOffset.Z = -boxSizeFix.Z;

            boxOrigin.X = (box.Min.X + box.Max.X) / 2;
            boxOrigin.Y = (box.Min.Y + box.Max.Y) / 2;
            boxOrigin.Z = (box.Min.Z + box.Max.Z) / 2;

        }
        else {

        }

        m_pCardBox->SetBoxExtent(boxSizeFix);
        m_pCardBox->SetRelativeLocation(-boxSizeFixPivotOffset);

        m_pCardStaticMesh->SetStaticMesh(m_pResMesh);
        m_pCardStaticMesh->SetRelativeLocation(-boxOrigin);
    }


    UMaterialInstanceDynamic* pMIDNow = Cast<UMaterialInstanceDynamic>(m_pCardStaticMesh->GetMaterial(0));
    if (IsValid(m_pResMI)) {
        if (IsValid(pMIDNow) && pMIDNow->Parent == m_pResMI) {
            //equal
        }
        else {
            UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(m_pResMI, m_pCardStaticMesh);
            MY_VERIFY(IsValid(pMID));
            m_pCardStaticMesh->SetMaterial(0, pMID);
        }
    }
    else {
        //simple, target is to clear
        if (!IsValid(pMIDNow)) {
            //equal
        }
        else {
            m_pCardStaticMesh->SetMaterial(0, nullptr);
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
    MY_VERIFY(IsValid(m_pCardStaticMesh));

    UMaterialInterface* pMat = m_pCardStaticMesh->GetMaterial(0);
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
            //m_pCardStaticMesh->SetMaterial(0, nullptr);
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


void AMyMJGameCardBaseCpp::getModelInfo(FMyMJGameActorModelInfoBoxCpp& modelInfo) const
{
    FVector actorScale3D = GetActorScale3D();
    modelInfo.m_cBoxExtend = m_pCardBox->GetScaledBoxExtent() * actorScale3D;
    modelInfo.m_cCenterPointRelativeLocation = m_pCardBox->RelativeLocation * actorScale3D;

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
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setResPath %s."), *newResPath.Path);

    if (m_cResPath.Path == newResPath.Path) {
        return;
    }
    m_cResPath = newResPath;

    checkAndLoadCardBasicResources(m_cResPath.Path);
    updateVisual();
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
        UTexture* pTBaseColor = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
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