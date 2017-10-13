// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MyMJGameCard.generated.h"

USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardVisualInfoCpp()
    {
        m_iIdxAttender = 0;
        m_eSlot = MyMJCardSlotTypeCpp::Invalid;
        m_iIdxRow = 0;
        m_iIdxColInRow = 0;
        //m_iCountOfColInRow = 0;
        m_iIdxStackInCol = 0;
        m_eFlipState = MyMJCardFlipStateCpp::Invalid;
        m_iXRotate90D = 0;
        m_iXRotate90DBeforeCount = 0;
        m_iColInRowExtraMarginCount = 0;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx attender"))
        int32 m_iIdxAttender;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card slot"))
        MyMJCardSlotTypeCpp m_eSlot;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx row"))
    int32 m_iIdxRow;

    //when colInRow Aligment is left or mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx col in row"))
    int32 m_iIdxColInRow;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "count of col in row"))
    //int32 m_iCountOfColInRow;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx stack in col"))
    int32 m_iIdxStackInCol;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "flip state"))
    MyMJCardFlipStateCpp m_eFlipState;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "X Rotate 90D"))
    int32 m_iXRotate90D;

    //when colInRow Aligment is left mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "X Rotate 90D before count"))
    int32 m_iXRotate90DBeforeCount;

    //when colInRow Aligment is left mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin count"))
    int32 m_iColInRowExtraMarginCount;
};

//model always and must facing x axis
USTRUCT(BlueprintType)
struct FMyMJGameCardActorModelInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardActorModelInfoCpp() : m_cBoxExtendFinal(0), m_cCenterPointFinalRelativeLocation(0)
    {
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "box extend final"))
    FVector m_cBoxExtendFinal;

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
    FVector m_cCenterPointFinalRelativeLocation;

};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    //nomatter whether collsion is enabled, it just return the current size
    const class UBoxComponent& getCollisionBoxRef() const;

    //return 0 if no error happens and OK to do visual operations, even mode not changed since it equal to old fasion, otherwise errorcode
    //@modelAssetPath example /Game/Art/Models/MJCard/Type0/cardBox/
    UFUNCTION(BlueprintCallable)
    int32 changeVisualModelType(const FString &modelAssetPath);

    UFUNCTION(BlueprintCallable)
    int32 changeVisualValue(int32 newValue);

    UFUNCTION(BlueprintCallable)
    void getModelInfo(FMyMJGameCardActorModelInfoCpp& modelInfo);


protected:

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostInitializeComponents() override;

    void createAndInitComponents();
    int32 updateCardStaticMeshMIDParams(class UTexture* InBaseColor);

    int32 changeVisualModelTypeInternal(const FString &modelAssetPath, bool bInConstruct);
    int32 changeVisualValueInternal(int32 newValue, bool bInConstruct, bool bIgnoreValueCompare);

    //return true if all res loaded
    bool helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture);


    //component doesn't need uproperty
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card box"))
    class USceneComponent *m_pRootScene;

    class UBoxComponent *m_pCardBox;

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card static mesh"))
    class UStaticMeshComponent *m_pCardStaticMesh;

    UPROPERTY()
    class UMaterialInstance* m_pResCardStaticMeshMITarget;

    //for some reason, MID can't survive across ctor process
    //UPROPERTY()
    //class UMaterialInstanceDynamic* m_pResCardStaticMeshMIDTarget;

    UPROPERTY()
    class UTexture* m_pResCardStaticMeshMIDParamInBaseColorTarget;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "model asset path"))
    FString m_sModelAssetPath;

    int32 m_iError;
};