// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MyMJGameVisualCommon.h"
#include "GameFramework/MovementComponent.h"
#include "Components/TimelineComponent.h"
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

    inline
    FString genDebugString() const
    {
        return m_cVisualInfo.genDebugString() + m_cVisualResult.genDebugString();
    };

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

USTRUCT(BlueprintType)
struct FTransformUpdateSequencDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FTransformUpdateSequencDataCpp()
    {
        reset();
    };

    void reset()
    {
        FTransform tempT;
        m_cStart = m_cEnd = tempT;
        m_cLocalRotatorExtra = FRotator::ZeroRotator;

        m_bLocationEnabledCache = false;
        m_bRotatorBasicEnabledCache = false;
        m_bRotatorExtraEnabledCache = false;
        m_bScaleEnabledCache = false;

        m_fTime = 0;
    };

    void helperSetDataBySrcAndDst(const FTransform& cStart, const FTransform& cEnd, float fTime, int32 iLocalRollExtra = 0, int32 iLocalPitchExtra = 0, int32 iLocalYawExtra = 0);

    //Warning:: because Rotator pitch in UE4, can't exceed +- 90D any where, so we can't store rotaion delta, but indirectly store the start and end
    FTransform m_cStart;
    FTransform m_cEnd;

    FRotator m_cLocalRotatorExtra;

    //fast flag to avoid float calc
    bool m_bLocationEnabledCache;
    bool m_bRotatorBasicEnabledCache;
    bool m_bRotatorExtraEnabledCache;
    bool m_bScaleEnabledCache;

    float m_fTime;
};

//UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent), HideCategories = Velocity)
//class ENGINE_API UInterpToMovementComponent : public UMovementComponent

/**
* make the actor move accroding to preset sequences
*
*/
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class MYONLINECARDGAME_API UMyTransformUpdateSequenceMovementComponent : public UMovementComponent
{
    //GENERATED_UCLASS_BODY()
    GENERATED_BODY()

public:
    UMyTransformUpdateSequenceMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    //UMyCurveSeqMovementComponent();

    //return the internal idx new added, < 0 means fail, do NOT use it as peek idx, use getSeqCount() or getSeqLast() to ref the last added one
    //curve's value range is supposed to 0 - 1, out of that is also allowed, 0 means start, 1 means dest, time range is 0 - N. 
    UFUNCTION(BlueprintCallable)
        int32 addSeqToTail(const FTransformUpdateSequencDataCpp& data, UCurveVector* curve);

    //return the number removed
    UFUNCTION(BlueprintCallable)
    int32 removeSeqFromHead(int32 iCount);

    //return the internal idx, < 0 means not exist, not it is slower since have struct data copy step
    UFUNCTION(BlueprintCallable)
    int32 peekSeqAt(int32 idxFromHead, FTransformUpdateSequencDataCpp& outData, UCurveVector*& outCurve) const;

    int32 peekSeqAtCpp(int32 idxFromHead, const FTransformUpdateSequencDataCpp*& poutData, UCurveVector*& outCurve) const;

    //UFUNCTION(BlueprintCallable)
    //const UCurveVector* peekSeqAt2(int32 idxFromHead, FTransformUpdateSequencDataCpp& outData) const;

    UFUNCTION(BlueprintCallable)
    int32 peekSeqLast(FTransformUpdateSequencDataCpp& outData, UCurveVector*& outCurve) const;

    UFUNCTION(BlueprintCallable)
    int32 getSeqCount() const;

    UFUNCTION(BlueprintCallable)
    void clearSeq();

    UFUNCTION(BlueprintCallable)
    static void helperSetDataBySrcAndDst(FTransformUpdateSequencDataCpp& data, const FTransform& cStart, const FTransform& cEnd, float fTime, int32 iLocalRollExtra = 0, int32 iLocalPitchExtra = 0, int32 iLocalYawExtra = 0)
    {
        data.helperSetDataBySrcAndDst(cStart, cEnd, fTime, iLocalRollExtra, iLocalPitchExtra, iLocalYawExtra);
    };

protected:

    //Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
    virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
    //End UActorComponent Interface

    //mainly used for debug and allow switch the implement quickly
    inline void setActivatedMyEncapped(bool bNew, FString reason = TEXT("none"))
    {
        if (IsActive() != bNew) {
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("activate component change %d -> %d, reason %s."), !bNew, bNew, *reason);
            if (bNew) {
                Activate();
            }
            else {
                Deactivate();
            }
        }
    };


    //return true if new play started
    bool tryStartNextSeq(FString sDebugReason);

    void onTimeLineUpdated(FVector vector);
    void onTimeLineFinished();

    UPROPERTY()
    FTimeline m_cTimeLine;

    FOnTimelineVectorStatic m_cTimeLineVectorDelegate;
    FOnTimelineEventStatic  m_cTimeLineFinishEventDelegate;

    UPROPERTY()
    TArray<FTransformUpdateSequencDataCpp> m_aDataItems;

    UPROPERTY()
    FMyCycleBufferMetaDataCpp m_cDataMeta;

    //Warning:: we assume curve value from 0 - 1. if value out of range, that means final interp out of range, maybe desired in some case
    UPROPERTY()
    TArray<UCurveVector *> m_aCurveItems;

    UPROPERTY()
    FMyCycleBufferMetaDataCpp m_cCurveMeta;

    FMyCycleBuffer<FTransformUpdateSequencDataCpp> m_cDataCycleBuffer;
    FMyCycleBuffer<UCurveVector*> m_cCurveCycleBuffer;

    float m_fDebugTimeLineStartWorldTime;
};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    //return error code, 0 means OK
    UFUNCTION(BlueprintPure, meta = (CallableWithoutWorldContext))
    int32 getModelInfo(FMyMJGameActorModelInfoBoxCpp& modelInfo) const;

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

    inline
    UMyTransformUpdateSequenceMovementComponent* getTransformUpdateSequence()
    {
        return m_pTransformUpdateSequence;
    };

    inline const FTransform& getTransform2GoRefConst() const
    {
        return m_cTransform2Go;
    };

    inline void setTransform2Go(const FTransform& cTransform2Go)
    {
        m_cTransform2Go = cTransform2Go;
    };

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
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "card box"))
    class UBoxComponent *m_pCardBox;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "card static mesh"))
    class UStaticMeshComponent *m_pCardStaticMesh;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "transform update sequence"))
    UMyTransformUpdateSequenceMovementComponent* m_pTransformUpdateSequence;

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
    UPROPERTY(BlueprintReadOnly)
    FTransform m_cTransform2Go;

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
