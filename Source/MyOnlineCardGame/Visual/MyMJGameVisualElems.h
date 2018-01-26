// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/DataAsset.h"

#include "Utils/CardGameUtils/MyCardGameUtilsLibrary.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/MyMJGameEventBase.h"

#include "MyMJGameVisualCommon.h"
#include "MyMJGameVisualInterfaces.h"

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

        m_iHelperIdxColInRowReal = -1;
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
            m_iCardValue == other.m_iCardValue && m_iHelperIdxColInRowReal == other.m_iHelperIdxColInRowReal;
    };


    inline
    FString genDebugString() const
    {
        return FString::Printf(TEXT("[value %d, flip %s, posi %d:%s, vidxs %d:%d:%d:%d]"), m_iCardValue, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCardFlipStateCpp"), (uint8)m_eFlipState), m_iIdxAttender, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCardSlotTypeCpp"), (uint8)m_eSlot),
                                m_iIdxRow, m_iIdxColInRow, m_iIdxStackInCol, m_iHelperIdxColInRowReal);
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

    // >= 0 means valid;
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "helper idx col in row real"))
    int32 m_iHelperIdxColInRowReal;
};


//seperate the data from actor, to allow subthread handling in the future
USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoAndResultCpp

{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        m_cVisualInfo.reset();
        m_cVisualResult.reset();
    };

    inline
    FString genDebugString() const
    {
        return m_cVisualInfo.genDebugString() + m_cVisualResult.genDebugString();
    };

    UPROPERTY(BlueprintReadOnly)
    FMyMJGameCardVisualInfoCpp  m_cVisualInfo;

    UPROPERTY(BlueprintReadOnly)
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
class MYONLINECARDGAME_API AMyMoveWithSeqActorBaseCpp : public AActor, public IMyTransformUpdateSequenceInterface
{
    GENERATED_BODY()

public:

    AMyMoveWithSeqActorBaseCpp();

    virtual ~AMyMoveWithSeqActorBaseCpp();


    virtual int32 getModelInfo(FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const override;
    virtual UMyTransformUpdateSequenceMovementComponent* getTransformUpdateSequence(bool verify = true) override;

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    static void helperTestAnimationStep(float time, FString debugStr, const TArray<AMyMoveWithSeqActorBaseCpp*>& actors);

protected:

    //virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    //return errcode if got wrong, 0 if OK, here it just adjust boxExtend to cover the mesh exactly
    virtual int32 updateSettings();

    //components
    //root scene
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
        class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "main box"))
        class UBoxComponent *m_pMainBox;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "main static mesh"))
        class UStaticMeshComponent *m_pMainStaticMesh;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "transform update sequence"))
        UMyTransformUpdateSequenceMovementComponent* m_pTransformUpdateSequence;

    UPROPERTY(EditDefaultsOnly, Category = "My Helper", meta = (DisplayName = "fake button to update settings"))
    bool m_bFakeUpdateSettings;

private:
    void createComponentsForCDO();
};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AMyMoveWithSeqActorBaseCpp
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    UFUNCTION(BlueprintSetter)
    void setValueShowing(int32 newValue);

    UFUNCTION(BlueprintGetter)
    int32 getValueShowing() const;

    //return true if new path is OK
    UFUNCTION(BlueprintSetter)
    void setResPath(const FDirectoryPath& newResPath);

    bool setResPathWithRet(const FDirectoryPath& newResPath);

    UFUNCTION(BlueprintGetter)
    const FDirectoryPath& getResPath() const;

    //Todo: incomplete yet
    inline void reset()
    {
        m_cTargetToGoHistory.clearInGame();
    };

    inline void addTargetToGoHistory(const FMyMJGameCardVisualInfoAndResultCpp& cTargetToGo)
    {
        if (m_cTargetToGoHistory.isFull()) {
            m_cTargetToGoHistory.removeFromHead(1);
        };

        m_cTargetToGoHistory.addToTail(&cTargetToGo, NULL);
    };

    inline const FMyMJGameCardVisualInfoAndResultCpp* getTargetToGoHistory(int32 idxFromLast, bool bVerifyValid = true) const
    {
        const FMyMJGameCardVisualInfoAndResultCpp* ret;
        int32 idx = m_cTargetToGoHistory.getCount() - 1 - idxFromLast;
        if (idx < m_cTargetToGoHistory.getCount()) {
            ret = m_cTargetToGoHistory.peekAt(idx);
        }
        else {
            ret = NULL;
        }

        if (bVerifyValid)
        {
            MY_VERIFY(ret != NULL);
        }

        return ret;
    };

    static void helperToSeqActors(const TArray<AMyMJGameCardBaseCpp*>& aSub, bool bSort, TArray<IMyTransformUpdateSequenceInterface*> &aBase);

protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostInitializeComponents() override;

    //it make box aligh the bottom end point
    virtual int32 updateSettings() override;

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

    //where this card should go, but allow it not be there now(should move smoothly there)
    FMyCycleBuffer<FMyMJGameCardVisualInfoAndResultCpp> m_cTargetToGoHistory;

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

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameTrivalDancingActorBaseCpp : public AMyMoveWithSeqActorBaseCpp
{
    GENERATED_BODY()

public:

    AMyMJGameTrivalDancingActorBaseCpp();

    virtual ~AMyMJGameTrivalDancingActorBaseCpp();

protected:

};