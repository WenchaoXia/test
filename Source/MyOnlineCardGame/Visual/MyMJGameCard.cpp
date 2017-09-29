// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCard.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"

#define MyCardStaticMeshMIDParamInBaseColor (TEXT("InBaseColor"))

#define MyCardAssetPartialNameStaticMesh (TEXT("cardBox"))
#define MyCardAssetPartialNameStaticMeshDefaultMI (TEXT("cardBoxMat0_defaultInst"))

#define MyCardAssetPartialNamePrefixValueNormal (TEXT("v%02d"))
#define MyCardAssetPartialNamePrefixValueUnknown (TEXT("vUnknown"))
#define MyCardAssetPartialNamePrefixValueMiss (TEXT("vMiss"))

#define MyCardAssetPartialNameSuffixValueBaseColorTexture (TEXT("_baseColor"))

AMyMJGameCardBaseCpp::AMyMJGameCardBaseCpp() : Super()
{
    bNetLoadOnClient = true;

    m_pCardBox = NULL;
    m_pCardStaticMesh = NULL;

    m_pResCardStaticMeshMITarget = NULL;
    //m_pResCardStaticMeshMIDTarget = NULL;
    m_pResCardStaticMeshMIDParamInBaseColorTarget = NULL;

    m_iValueShowing = 0;
    m_sModelAssetPath.Reset();
    
    createAndInitComponents();
}

AMyMJGameCardBaseCpp::~AMyMJGameCardBaseCpp()
{

};

void AMyMJGameCardBaseCpp::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnConstruction %p, %p, %p"), m_pResCardStaticMeshMITarget, m_pResCardStaticMeshMIDTarget, m_pResCardStaticMeshMIDParamInBaseColorTarget);

    /*
    FString matDefaultInstFullPathName = m_sModelAssetPath + TEXT("cardBoxMat0_defaultInst");
    UMaterialInstance *pMatInstAsset = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (IsValid(pMatInstAsset)) {
        UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(pMatInstAsset, m_pCardStaticMesh);
        m_pCardStaticMesh->SetMaterial(0, pMID);
    }
    */

    //UMaterialInterface* pMat = m_pCardStaticMesh->GetMaterial(0);
    //pMat->GetFullName();
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("mat %s"), pMat ? *pMat->GetFullName() : TEXT("NULL"));

    /*
    if (m_pResCardStaticMeshMIDTarget) {
        m_pCardStaticMesh->SetMaterial(0, m_pResCardStaticMeshMIDTarget);
        m_pResCardStaticMeshMIDTarget = NULL;
    }
    */
    
    if (IsValid(m_pResCardStaticMeshMITarget)) {
        UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(m_pResCardStaticMeshMITarget, m_pCardStaticMesh);
        m_pCardStaticMesh->SetMaterial(0, pMID);
        m_pResCardStaticMeshMITarget = NULL;
    }

    if (IsValid(m_pResCardStaticMeshMIDParamInBaseColorTarget)) {
        updateCardStaticMeshMIDParams(m_pResCardStaticMeshMIDParamInBaseColorTarget);
        m_pResCardStaticMeshMIDParamInBaseColorTarget = NULL;
    }
}

void AMyMJGameCardBaseCpp::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("PostInitializeComponents"));

}

void AMyMJGameCardBaseCpp::createAndInitComponents()
{
    //create defaults
    if (!IsValid(m_pCardBox)) {

        UBoxComponent* pBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootBox"));
        //UBoxComponent* pBoxComponent = NewObject<UBoxComponent>(this);
        MY_VERIFY(IsValid(pBoxComponent));

        //pBoxComponent->Rename(TEXT("RootBox"));
        RootComponent = pBoxComponent;

        //pBoxComponent->InitBoxExtent(boxSizeFix);
        pBoxComponent->SetCollisionProfileName(TEXT("CollistionProfileBox"));

        //pBoxComponent->RegisterComponent();
        m_pCardBox = pBoxComponent;
    }

    if (!IsValid(m_pCardStaticMesh)) {

        UStaticMeshComponent* pStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardStaticMesh"));
        //UStaticMeshComponent* pStaticMeshComponent = NewObject<UStaticMeshComponent>(this);
        MY_VERIFY(IsValid(pStaticMeshComponent));

        //pStaticMeshComponent->Rename(TEXT("CardStaticMesh"));
        pStaticMeshComponent->SetupAttachment(m_pCardBox);

        //pStaticMeshComponent->RegisterComponent();
        m_pCardStaticMesh = pStaticMeshComponent;
    }

    //set defaults
    changeVisualModelTypeInternal(TEXT("/Game/Art/Models/MJCard/Type0/"), true);

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("init, m_pCardBox %p, m_pCardStaticMesh %p."), m_pCardBox, m_pCardStaticMesh);

}

int32 AMyMJGameCardBaseCpp::updateCardStaticMeshMIDParams(class UTexture* InBaseColor)
{
    UMaterialInterface* pMat = m_pCardStaticMesh->GetMaterial(0);
    if (!IsValid(pMat)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid material on mesh."));
        return -21;
    }

    UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(pMat);
    if (!DynamicMaterial)
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("old material is not MID, it is: %s."), *pMat->GetClass()->GetFullName());
        //DynamicMaterial = m_pCardStaticMesh->CreateAndSetMaterialInstanceDynamic(0);    //always use dynamic material
    }
    if (!DynamicMaterial)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cast to dynamic material instance fail: %s."), *pMat->GetClass()->GetFullName());
        return -30;
    }

    DynamicMaterial->SetTextureParameterValue(MyCardStaticMeshMIDParamInBaseColor, InBaseColor);

    return 0;
};

///example: /Game/Art/Models/MJCard/Type0/cardBox/
int32 AMyMJGameCardBaseCpp::changeVisualModelType(const FString &modelAssetPath)
{
    return changeVisualModelTypeInternal(modelAssetPath, false);
};

int32 AMyMJGameCardBaseCpp::changeVisualModelTypeInternal(const FString &modelAssetPath, bool bInConstruct)
{
    if (modelAssetPath == m_sModelAssetPath) {
        if (m_sModelAssetPath.IsEmpty()) {
            return -1;
        }
        else {
            return 0;
        }
    }

    FString meshFullPathName = modelAssetPath + MyCardAssetPartialNameStaticMesh;
    FString matDefaultInstFullPathName = modelAssetPath + MyCardAssetPartialNameStaticMeshDefaultMI;
    //FString textureMisssFullPathName = modelAssetPath + TEXT("vMiss_baseColor");

    UStaticMesh *pMeshAsset = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(NULL, meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        return -2;
    }

    UMaterialInstance *pMatInstAsset = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (!IsValid(pMeshAsset)) {
        return -2;
    }


    FBox box = pMeshAsset->GetBoundingBox();
    FVector boxSize = box.Max - box.Min;
    FVector boxSizeFix;
    boxSizeFix.X = UKismetMathLibrary::FCeil(boxSize.X) / 2;
    boxSizeFix.Y = UKismetMathLibrary::FCeil(boxSize.Y) / 2;
    boxSizeFix.Z = UKismetMathLibrary::FCeil(boxSize.Z) / 2;

    FVector boxOrigin;
    boxOrigin.X = (box.Min.X + box.Max.X) / 2;
    boxOrigin.Y = (box.Min.Y + box.Max.Y) / 2;
    //boxOrigin.Z = box.Min.Z;
    boxOrigin.Z = (box.Min.Z + box.Max.Z) / 2;

    MY_VERIFY(IsValid(m_pCardBox));
    MY_VERIFY(IsValid(m_pCardStaticMesh));

    m_pCardBox->SetBoxExtent(boxSizeFix);

    m_pCardStaticMesh->SetStaticMesh(pMeshAsset);

    m_pCardStaticMesh->SetRelativeLocation(-boxOrigin);
    //m_pCardStaticMesh->SetWorldScale3D(FVector(1.0f));

    //create MID
    //UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(pMatInstAsset, m_pCardStaticMesh);
    if (bInConstruct) {
        //postpond it, since in construct it is invalid to set parameters of visual unit which have not been constructed
        m_pResCardStaticMeshMITarget = pMatInstAsset;
        //m_pResCardStaticMeshMIDTarget = pMID;
    }
    else {
        UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(pMatInstAsset, m_pCardStaticMesh);
        m_pCardStaticMesh->SetMaterial(0, pMID);
    }

    m_sModelAssetPath = modelAssetPath;


    int32 iRet = 0;
    iRet = changeVisualValueInternal(m_iValueShowing, bInConstruct, true);
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("changeVisualModelTypeInternal %p, %p, %p"), m_pResCardStaticMeshMITarget, m_pResCardStaticMeshMIDTarget, m_pResCardStaticMeshMIDParamInBaseColorTarget);
    return iRet;
};

int32 AMyMJGameCardBaseCpp::changeVisualValue(int32 newValue)
{
    return changeVisualValueInternal(newValue, false, false);
};

int32 AMyMJGameCardBaseCpp::changeVisualValueInternal(int32 newValue, bool bInConstruct, bool bIgnoreValueCompare)
{
    if (m_sModelAssetPath.IsEmpty()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_sModelAssetPath is empty."));
        return -10;
    }

    if (UMyMJUtilsLibrary::getCardValueType(newValue) == MyMJCardValueTypeCpp::Invalid && newValue != 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid newVaule %d."), newValue);
        return -11;
    }

    if (m_iValueShowing == newValue && !bIgnoreValueCompare) {
        return -12;
    }


    FString vPrefix;
    if (newValue > 0) {
        vPrefix = FString::Printf(MyCardAssetPartialNamePrefixValueNormal, newValue);
    }
    else if (newValue == 0) {
        vPrefix = MyCardAssetPartialNamePrefixValueUnknown;
    }
    else {
        return -13;
    }

    UTexture* pBaseColorTexture = NULL;

    if (!helperTryLoadCardRes(m_sModelAssetPath, vPrefix, &pBaseColorTexture)) {
        vPrefix = MyCardAssetPartialNamePrefixValueMiss;
        if (!helperTryLoadCardRes(m_sModelAssetPath, vPrefix, &pBaseColorTexture)) {
            return -20;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("warning: using 'miss' texture for value %d."), newValue);
        }
    }

    MY_VERIFY(IsValid(pBaseColorTexture));
    MY_VERIFY(IsValid(m_pCardStaticMesh));

    if (bInConstruct) {
        //postpond it, since in construct it is invalid to set parameters of visual unit which have not been constructed
        m_pResCardStaticMeshMIDParamInBaseColorTarget = pBaseColorTexture;
    }
    else {
        updateCardStaticMeshMIDParams(pBaseColorTexture);
    }

    m_iValueShowing = newValue;

    return 0;
};

bool AMyMJGameCardBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture)
{
    bool bRet = true;

    if (ppOutBaseColorTexture) {
        FString baseColorFullPathName = modelAssetPath + valuePrefix + MyCardAssetPartialNameSuffixValueBaseColorTexture;
        UTexture* pTBaseColor = UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutBaseColorTexture = pTBaseColor;
        }
        else {
            bRet = false;
        }
    }

    return bRet;
}