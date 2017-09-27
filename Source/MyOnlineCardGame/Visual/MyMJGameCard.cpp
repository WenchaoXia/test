// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCard.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"
#include "Kismet/KismetMathLibrary.h"

AMyMJGameCardBaseCpp::AMyMJGameCardBaseCpp()
{
    m_pCardBox = NULL;
    m_pCardStaticMesh = NULL;

    m_sModelAssetPath.Reset();
}

AMyMJGameCardBaseCpp::~AMyMJGameCardBaseCpp()
{

};


///example: /Game/Art/Models/MJCard/Type0/cardBox/
bool AMyMJGameCardBaseCpp::changeModelType(const FString &modelAssetPath)
{
    if (modelAssetPath == m_sModelAssetPath) {
        return !m_sModelAssetPath.IsEmpty();
    }


    //FString meshFullPathName = modelAssetPath + TEXT("cardBox.cardBox");
    //FString matDefaultInstFullPathName = modelAssetPath + TEXT("cardBoxMat0_defaultInst.cardBoxMat0_defaultInst");
    //FString textureMisssFullPathName = modelAssetPath + TEXT("vMiss_baseColor.vMiss_baseColor");

    FString meshFullPathName = modelAssetPath + TEXT("cardBox");
    FString matDefaultInstFullPathName = modelAssetPath + TEXT("cardBoxMat0_defaultInst");
    FString textureMisssFullPathName = modelAssetPath + TEXT("vMiss_baseColor");

    //UClass *pMeshAssetClass = LoadClass<UStaticMesh>(NULL, *meshFullPathName, NULL, LOAD_None, NULL);

    UStaticMesh *pMeshAsset = FindObject<UStaticMesh>(NULL, *meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("find object fail, maybe forgot preload it: %s."), *meshFullPathName);
        pMeshAsset = LoadObject<UStaticMesh>(NULL, *meshFullPathName, NULL, LOAD_None, NULL);
    }
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load: %s."), *meshFullPathName);
        return false;
    }


    UMaterialInstance *pMatInstAsset = FindObject<UMaterialInstance>(NULL, *matDefaultInstFullPathName);
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("find object fail, maybe forgot preload it: %s."), *matDefaultInstFullPathName);
        pMatInstAsset = LoadObject<UMaterialInstance>(NULL, *matDefaultInstFullPathName, NULL, LOAD_None, NULL);
    }
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load: %s."), *matDefaultInstFullPathName);
        return false;
    }


    UTexture *pTextureAsset = LoadObject<UTexture>(NULL, *textureMisssFullPathName, NULL, LOAD_None, NULL);
    if (!IsValid(pTextureAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load: %s."), *textureMisssFullPathName);
        return false;
    }

    FBox box = pMeshAsset->GetBoundingBox();
    FVector boxSize = box.Max - box.Min;
    FVector boxSizeFix;
    boxSizeFix.X = UKismetMathLibrary::FCeil(boxSize.X);
    boxSizeFix.Y = UKismetMathLibrary::FCeil(boxSize.Y);
    boxSizeFix.Z = UKismetMathLibrary::FCeil(boxSize.Z);

    FVector boxOrigin;
    boxOrigin.X = (box.Min.X + box.Max.X) / 2;
    boxOrigin.Y = (box.Min.Y + box.Max.Y) / 2;
    boxOrigin.Z = box.Min.Z;


    //let's clear old components
    if (!IsValid(m_pCardBox)) {

        //UBoxComponent* pBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootBox"));
        UBoxComponent* pBoxComponent = NewObject<UBoxComponent>(this);
        MY_VERIFY(IsValid(pBoxComponent));

        pBoxComponent->Rename(TEXT("RootBox"));
        RootComponent = pBoxComponent;

        pBoxComponent->InitBoxExtent(boxSizeFix);
        pBoxComponent->SetCollisionProfileName(TEXT("Box"));

        pBoxComponent->RegisterComponent();
        m_pCardBox = pBoxComponent;
    }

    if (!IsValid(m_pCardStaticMesh)) {
        //UStaticMeshComponent* pStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("cardMesh"));
        UStaticMeshComponent* pStaticMeshComponent = NewObject<UStaticMeshComponent>(this);
        MY_VERIFY(IsValid(pStaticMeshComponent));

        pStaticMeshComponent->Rename(TEXT("CardStaticMesh"));
        pStaticMeshComponent->SetupAttachment(m_pCardBox);

        pStaticMeshComponent->RegisterComponent();
        m_pCardStaticMesh = pStaticMeshComponent;
    }

    m_pCardBox->SetBoxExtent(boxSizeFix);

    m_pCardStaticMesh->SetStaticMesh(pMeshAsset);
    m_pCardStaticMesh->SetRelativeLocation(-boxOrigin);
    //m_pCardStaticMesh->SetWorldScale3D(FVector(1.0f));

    m_sModelAssetPath = modelAssetPath;

    return true;

};