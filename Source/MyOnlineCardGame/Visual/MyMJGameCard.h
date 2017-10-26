// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MyMJGameCard.generated.h"

#define MyCardStaticMeshMIDParamInBaseColor (TEXT("InBaseColor"))

#define MyCardAssetPartialNameStaticMesh (TEXT("cardBox"))
#define MyCardAssetPartialNameStaticMeshDefaultMI (TEXT("cardBoxMat0_defaultInst"))

#define MyCardAssetPartialNamePrefixValueNormal (TEXT("v%02d"))
#define MyCardAssetPartialNamePrefixValueUnknown (TEXT("vUnknown"))
#define MyCardAssetPartialNamePrefixValueMiss (TEXT("vMiss"))

#define MyCardAssetPartialNameSuffixValueBaseColorTexture (TEXT("_baseColor"))

#define MyMJGameCardBaseCppDefaultShowingValue (0)

//model always and must facing x axis
USTRUCT(BlueprintType)
struct FMyMJGameCardActorModelInfoCpp

{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardActorModelInfoCpp()
    {
        reset();
    };

    void reset()
    {
        m_cBoxExtend = FVector(1);
        m_cCenterPointRelativeLocation = FVector(0);
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "box extend final"))
    FVector m_cBoxExtend;

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
    FVector m_cCenterPointRelativeLocation;

};


USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardVisualInfoCpp()
    {
        reset();
    };

    void reset()
    {
        m_bVisible = false;
        m_eFlipState = MyMJCardFlipStateCpp::Invalid;

        m_iIdxAttender = 0;
        m_eSlot = MyMJCardSlotTypeCpp::Invalid;
        m_iIdxRow = 0;
        m_iIdxColInRow = 0;
        //m_iCountOfColInRow = 0;
        m_iIdxStackInCol = 0;

        m_iRotateX90D = 0;
        m_iRotateX90DBeforeCount = 0;
        m_iColInRowExtraMarginCount = 0;

        m_iCardValue = 0;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "visible"))
        bool m_bVisible;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "flip state"))
        MyMJCardFlipStateCpp m_eFlipState;

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

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Rotate X 90D"))
        int32 m_iRotateX90D;

    //when colInRow Aligment is left mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Rotate X 90D before count"))
        int32 m_iRotateX90DBeforeCount;

    //when colInRow Aligment is left mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin count"))
        int32 m_iColInRowExtraMarginCount;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card value"))
        int32 m_iCardValue;
};

USTRUCT(BlueprintType)
struct FMyMJGameCardVisualStateCpp

{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardVisualStateCpp()
    {
        reset();
    };

    void reset()
    {
        m_bVisible = false;
        m_cTransform = FTransform();
        m_iCardValue = 0;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "visible"))
    bool m_bVisible;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "transform"))
    FTransform m_cTransform;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card value"))
        int32 m_iCardValue;

};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    //AMyMJGameCardBaseCpp(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    void getModelInfo(FMyMJGameCardActorModelInfoCpp& modelInfo) const;

    UFUNCTION(BlueprintSetter)
    void setValueShowing(int32 newValue);

    UFUNCTION(BlueprintGetter)
    int32 getValueShowing() const;

    UFUNCTION(BlueprintSetter)
    void setResPath(const FDirectoryPath& newResPath);

    UFUNCTION(BlueprintGetter)
    const FDirectoryPath& getResPath() const;

protected:

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostInitializeComponents() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    void createComponentsForCDO();
    int32 checkAndLoadCardBasicResources(const FString &inPath);

    //the update rule is: always update if new settings arrive, and always reflect it even fail, never revert values
    int32 updateVisual();
    int32 updateWithCardBasicResources();
    int32 updateWithValue(int32 newValue);

    int32 updateCardStaticMeshMIDParams(class UTexture* InBaseColor);

    //return true if all res loaded
    bool helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture);


    //important values:
    //value showing now
    UPROPERTY(EditAnywhere, BlueprintSetter = setValueShowing, BlueprintGetter = getValueShowing, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;


    //components
    //root scene
    UPROPERTY(VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;

    UPROPERTY(VisibleAnywhere, Instanced, meta = (DisplayName = "card box"))
    class UBoxComponent *m_pCardBox;

    UPROPERTY(VisibleAnywhere, Instanced, meta = (DisplayName = "card static mesh"))
    class UStaticMeshComponent *m_pCardStaticMesh;


    //resouce settings, the child calss should specify them
    //Note: only one of ContentDir or RelativeToGameContentDir need to be specified to resulting relative path, their difference is dialog 
    //UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "card static mesh", ContentDir = "true", RelativeToGameContentDir = "true"))

    //where the card resource is, have special requirement such as mesh, material, texture, etc. example: /Game/Art/Models/MJCard/Type0
    UPROPERTY(EditDefaultsOnly, BlueprintSetter = setResPath, BlueprintGetter = getResPath, meta = (DisplayName = "resource path", ContentDir = "true"))
    FDirectoryPath m_cResPath;

    UPROPERTY(BlueprintReadOnly)
    class UStaticMesh *m_pResMesh;

    UPROPERTY(BlueprintReadOnly)
    class UMaterialInstance *m_pResMI;

};


USTRUCT(BlueprintType)
struct FMyMJCardVisualInfoPackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardVisualInfoPackCpp()
    {
    };

    virtual ~FMyMJCardVisualInfoPackCpp()
    {

    };

    inline int32 getCount() const
    {
        return m_aCardVisualInfos.Num();
    }

    void resize(int32 l)
    {
        int32 toAdd = l - m_aCardVisualInfos.Num();
        if (toAdd > 0) {
            m_aCardVisualInfos.AddDefaulted(toAdd); //Todo: use add zeroed for optimization
        }
        else if (toAdd < 0) {
            m_aCardVisualInfos.RemoveAt(m_aCardVisualInfos.Num() + toAdd, -toAdd);
        }
    };

    inline FMyMJGameCardVisualInfoCpp* getByIdx(int32 idx, bool bVerifyValid) {
        MY_VERIFY(idx >= 0);
        FMyMJGameCardVisualInfoCpp* ret = NULL;
        if (idx < m_aCardVisualInfos.Num()) {
            ret = &m_aCardVisualInfos[idx];
        }

        if (ret == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("idx out of range: %d / %d."), idx, getCount());
        }

        if (bVerifyValid) {
            MY_VERIFY(ret);
        }

        return ret;
    };

protected:

    friend struct FMyMJDataAccessorCpp;

    UPROPERTY(meta = (DisplayName = "card visual infos"))
        TArray<FMyMJGameCardVisualInfoCpp> m_aCardVisualInfos;

};