// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MyMJGameVisualCommon.h"

#include "MyMJGameVisualElems.generated.h"

#define MyCardStaticMeshMIDParamInBaseColor (TEXT("InBaseColor"))

#define MyCardAssetPartialNameStaticMesh (TEXT("cardBox"))
#define MyCardAssetPartialNameStaticMeshDefaultMI (TEXT("cardBoxMat0_defaultInst"))

#define MyCardAssetPartialNamePrefixValueNormal (TEXT("v%02d"))
#define MyCardAssetPartialNamePrefixValueUnknown (TEXT("vUnknown"))
#define MyCardAssetPartialNamePrefixValueMiss (TEXT("vMiss"))

#define MyCardAssetPartialNameSuffixValueBaseColorTexture (TEXT("_baseColor"))

#define MyMJGameCardBaseCppDefaultShowingValue (0)


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

    inline
    bool equal(const FMyMJGameCardVisualInfoCpp& other) const
    {
        if (&other == this) {
            return true;
        }

        return m_eFlipState == other.m_eFlipState && m_iIdxAttender == other.m_iIdxAttender && m_eSlot == other.m_eSlot &&
            m_iIdxRow == other.m_iIdxRow && m_iIdxColInRow == other.m_iIdxColInRow && m_iIdxStackInCol == other.m_iIdxStackInCol &&
            m_iRotateX90D == other.m_iRotateX90D && m_iRotateX90DBeforeCount == other.m_iRotateX90DBeforeCount && m_iColInRowExtraMarginCount == other.m_iColInRowExtraMarginCount &&
            m_iCardValue == other.m_iCardValue;
    };

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


//seperate the data from actor, to allow subthread handling in the future
USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoAndResultCpp

{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameCardVisualInfoCpp  m_cVisualInfo;
    FMyMJGameActorVisualResultBaseCpp m_cVisualResult;
};

USTRUCT(BlueprintType)
struct FMyMJGameDiceVisualInfoAndResultCpp

{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameDiceVisualInfoAndResultCpp()
    {
        m_iVisualInfoValue = 0;
    };

    int32  m_iVisualInfoValue;
    FMyMJGameActorVisualResultBaseCpp m_cVisualResult;
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
    void getModelInfo(FMyMJGameActorModelInfoBoxCpp& modelInfo) const;

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
    int32 updateWithValue();

    int32 updateCardStaticMeshMIDParams(class UTexture* InBaseColor);

    //return true if all res loaded
    bool helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture);


    //important values:
    //value showing now
    UPROPERTY(EditAnywhere, BlueprintSetter = setValueShowing, BlueprintGetter = getValueShowing, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;

    int32 m_iValueUpdatedBefore;

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


//all card related visual data is here, not touched to actor, to make it possible subthread handling 
USTRUCT(BlueprintType)
struct FMyMJCardVisualPackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardVisualPackCpp()
    {
    };

    virtual ~FMyMJCardVisualPackCpp()
    {

    };

    inline int32 getCount() const
    {
        return m_aCards.Num();
    }

    void resize(int32 l)
    {
        int32 toAdd = l - m_aCards.Num();
        if (toAdd > 0) {
            m_aCards.AddDefaulted(toAdd); //Todo: use add zeroed for optimization
        }
        else if (toAdd < 0) {
            m_aCards.RemoveAt(m_aCards.Num() + toAdd, -toAdd);
        }
    };

    inline FMyMJGameCardVisualInfoAndResultCpp* getByIdx(int32 idx, bool bVerifyValid) {
        MY_VERIFY(idx >= 0);
        FMyMJGameCardVisualInfoAndResultCpp* ret = NULL;
        if (idx < m_aCards.Num()) {
            ret = &m_aCards[idx];
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

    UPROPERTY(meta = (DisplayName = "cards"))
    TArray<FMyMJGameCardVisualInfoAndResultCpp> m_aCards;

};
