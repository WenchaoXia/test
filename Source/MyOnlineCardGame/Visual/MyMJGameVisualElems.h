// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/CardGameUtils/MyCardGameUtilsLibrary.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/MyMJGameEventBase.h"
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


USTRUCT()
struct FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp()
    {
        reset();
    };

    virtual ~FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp()
    {

    };

    inline void reset()
    {
        m_cCardShowPoint.reset();
        m_cCommonActionShowPoint.reset();

        m_bAttenderPointOnScreenOverride = false;
        m_cAttenderPointOnScreenPercentOverride = FVector2D::ZeroVector;

    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "card show point"))
    FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp m_cCardShowPoint;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "action show point"))
    FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp m_cCommonActionShowPoint;

    //By default, attender point on screen is calculated at runtime by project desktop point on player screen. If enabled, it will use overrided value directly to helper combine umg
    UPROPERTY(EditAnywhere, meta = (DisplayName = "attender point on screen override"))
    bool m_bAttenderPointOnScreenOverride;

    //it should be the pointer on border, like (0.5, 0), unit is percent
    UPROPERTY(EditAnywhere, meta = (EditCondition = "m_bAttenderPointOnScreenOverride", DisplayName = "attender point on screen percent override as"))
    FVector2D m_cAttenderPointOnScreenPercentOverride;

};

USTRUCT()
struct FMyMJGameInGamePlayerScreenCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameInGamePlayerScreenCfgCpp()
    {
        m_aAttenderAreas.Reset();
        m_aAttenderAreas.AddDefaulted(4);
    };

    virtual ~FMyMJGameInGamePlayerScreenCfgCpp()
    {

    };

    inline void reset()
    {
        for (int32 i = 0; i < 4; i++) {
            m_aAttenderAreas[i].reset();
        }
    };

    //mainly for test
    void fillDefaultData();

    UPROPERTY(EditAnywhere, EditFixedSize, meta = (DisplayName = "Attender Areas"))
    TArray<FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp> m_aAttenderAreas;

};

//MyMJGameCoreRelatedEventMainTypeCpp

USTRUCT()
struct FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyGameEventVisualDataBaseCpp()
    {
        m_fTotalTime = 1.f;
    }

    //unit is second
    UPROPERTY(EditAnywhere, meta = (DisplayName = "total time"))
    float m_fTotalTime;
};

USTRUCT()
struct FMyMJGameGameStartVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    
};

USTRUCT()
struct FMyMJGameThrowDicesVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameDistCardsDoneVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventCardsRelatedVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventCardsRelatedVisualDataCpp() : Super()
    {
        reset();
    };

    inline void reset()
    {
        m_fTotalTime = 1.f;
        m_aCardsFocusedSteps.Reset();
        m_aCardsOtherSteps.Reset();
        m_fDelayTimeForCardsUnfocused = 0;
        m_fTotalTimeOverridedForCardsUnfocused = 0;
    };

    //the cards focused, like given out cards in give out action. If empty it will use the default moving action.
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards focused steps"))
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aCardsFocusedSteps;

    //other cards, like the cards still moving but not the given out ones in give out action. If empty it will use the default moving action
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards Other steps"))
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aCardsOtherSteps;

    //if it > 0.01 and steps for other(or subclass's step for specified cards), a wait sequence will be inserted at head for those cards
    UPROPERTY(EditAnywhere, meta = (DisplayName = "delay time for cards unfocused"))
    float m_fDelayTimeForCardsUnfocused;

    //if it > 0.01, will override the total time set for this event, for cards unfocused
    UPROPERTY(EditAnywhere, meta = (DisplayName = "total time overrided for cards unfocused"))
    float m_fTotalTimeOverridedForCardsUnfocused;
};

USTRUCT()
struct FMyMJGameEventPusherTakeCardsVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventPusherTakeCardsVisualDataCpp() : Super()
    {
        m_bOverrideCardsFocusedStepsForAttenderAsViewer = false;
    };

    //whether act differently for current viewer, that is the attender near camera
    UPROPERTY(EditAnywhere, meta = (DisplayName = "override cards focused steps for attender as viewer"))
    bool m_bOverrideCardsFocusedStepsForAttenderAsViewer;

    //for cards belong to the attender near camera
    UPROPERTY(EditAnywhere, meta = (DisplayName = "overrided cards focused steps for attender as viewer", EditCondition = "m_bOverrideCardsFocusedStepsForAttenderAsViewer"))
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aOverridedCardsFocusedStepsForAttenderAsViewer;
};

USTRUCT()
struct FMyMJGameEventPusherGiveOutCardsVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        Super::reset();
        m_aCardsInsertedToHandSlotSteps.Reset();
    };

    //If empty it will use the default moving action
    UPROPERTY(EditAnywhere, meta = (DisplayName = "cards inserted to hand slot steps"))
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aCardsInsertedToHandSlotSteps;
};

USTRUCT()
struct FMyMJGameEventPusherWeaveVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        Super::reset();
        m_aDancingActor0Steps.Reset();
    }

    inline void copyExceptClass(const FMyMJGameEventPusherWeaveVisualDataCpp& other)
    {
        if (this != &other) {
            TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> oldClass = m_cDancingActor0Class;
            *this = other;
            m_cDancingActor0Class = oldClass;
        }
    };


    bool checkSettings(FString debugStr) const
    {
        const TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> &cClass = m_cDancingActor0Class;
        if (IsValid(cClass)) {
            if (cClass == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: class must be a child class of AMyMJGameTrivalDancingActorBaseCpp!"), *debugStr);
                return false;
            }
        }

        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class is %s."), *cClass->GetName());

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dancing actor0 class"))
    TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> m_cDancingActor0Class;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dancing actor0 steps"))
    TArray<FMyActorTransformUpdateAnimationStepCpp> m_aDancingActor0Steps;
};

USTRUCT()
struct FMyMJGameEventHuVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameGameEndVisualDataCpp : public FMyGameEventVisualDataBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventLocalCSHuBornVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};

USTRUCT()
struct FMyMJGameEventLocalCSZhaNiaoVisualDataCpp : public FMyMJGameEventCardsRelatedVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
};


USTRUCT()
struct FMyMJGameEventPusherCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventPusherCfgCpp()
    {
    };

    void fillDefaultData();

    bool checkSettings() const
    {
        if (!m_cWeaveChi.checkSettings(TEXT("event pusher cfg weave chi")))
        {
            return false;
        }

        if (!m_cWeavePeng.checkSettings(TEXT("event pusher cfg weave peng")))
        {
            return false;
        }

        if (!m_cWeaveGang.checkSettings(TEXT("event pusher cfg weave gang")))
        {
            return false;
        }

        if (!m_cWeaveBu.checkSettings(TEXT("event pusher cfg weave bu")))
        {
            return false;
        }

        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card class is %s."), *cClass->GetName());

        return true;
    };

    //when resync unexpected, the elems animation time. Note it would not occupy time segment on server, so this should be small as a pure client animation time 
    UPROPERTY(EditAnywhere, meta = (DisplayName = "resync unexpected in game"))
    FMyGameEventVisualDataBaseCpp m_cResyncUnexpectedIngame;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "resync normal at start"))
    FMyGameEventVisualDataBaseCpp m_cResyncNormalAtStart;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "game start"))
    FMyMJGameGameStartVisualDataCpp m_cGameStart;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "throw dices"))
    FMyMJGameThrowDicesVisualDataCpp m_cThrowDices;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "dist cards done"))
    FMyMJGameDistCardsDoneVisualDataCpp m_cDistCardsDone;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "take cards"))
    FMyMJGameEventPusherTakeCardsVisualDataCpp m_cTakeCards;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "give out cards"))
    FMyMJGameEventPusherGiveOutCardsVisualDataCpp m_cGiveOutCards;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave chi"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeaveChi;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave peng"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeavePeng;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave gang"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeaveGang;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave bu"))
    FMyMJGameEventPusherWeaveVisualDataCpp m_cWeaveBu;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "hu"))
    FMyMJGameEventHuVisualDataCpp m_cHu;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "game end"))
    FMyMJGameGameEndVisualDataCpp m_cGameEnd;

    UPROPERTY(EditAnywhere, Category = "local cs", meta = (DisplayName = "local CS hu born"))
    FMyMJGameEventLocalCSHuBornVisualDataCpp  m_cLocalCSHuBorn;

    UPROPERTY(EditAnywhere, Category = "local cs", meta = (DisplayName = "local CS zha niao"))
    FMyMJGameEventLocalCSZhaNiaoVisualDataCpp m_cLocalCSZhaNiao;
};

USTRUCT()
struct FMyMJGameEventTrivalCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
 
    inline void fillDefaultData()
    {

    };

    inline bool checkSettings() const
    {
        return true;
    };
};

//event is some thing time critical, important thing can't be missed
USTRUCT()
struct FMyMJGameInRoomEventCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline void fillDefaultData()
    {
        m_cPusherCfg.fillDefaultData();
        m_cTrivalCfg.fillDefaultData();
    };

    inline bool checkSettings() const
    {
        if (!m_cPusherCfg.checkSettings())
        {
            return false;
        }

        if (!m_cTrivalCfg.checkSettings())
        {
            return false;
        }

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "pusher"))
    FMyMJGameEventPusherCfgCpp m_cPusherCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "trival"))
    FMyMJGameEventTrivalCfgCpp m_cTrivalCfg;
};

 //incident is some thing no time critical, not important
USTRUCT()
struct FMyMJGameInRoomIncidentCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

};

USTRUCT()
struct FMyMJGameInRoomMainActorClassCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    bool checkSettings() const;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "card class"))
    TSubclassOf<AMyMJGameCardBaseCpp> m_cCardClass;
};


UCLASS()
class MYONLINECARDGAME_API UMyMJGameInRoomVisualCfgType : public UDataAsset
{
    GENERATED_BODY()

public:

    void fillDefaultData();

    bool checkSettings() const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    static void helperMapToSimplifiedTimeCfg(const FMyMJGameInRoomEventCfgCpp& eventCfg, FMyMJGameEventTimeCfgCpp& outSimplifiedTimeCfg);


    UPROPERTY(EditAnywhere, meta = (DisplayName = "Player Screen Cfg"))
    FMyMJGameInGamePlayerScreenCfgCpp m_cPlayerScreenCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Event Cfg"))
    FMyMJGameInRoomEventCfgCpp m_cEventCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Incident Cfg"))
    FMyMJGameInRoomIncidentCfgCpp m_cIncidentCfg;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Main actor class Cfg"))
    FMyMJGameInRoomMainActorClassCfgCpp m_cMainActorClassCfg;

    //If set to 10, all data will reset to default
    UPROPERTY(EditAnywhere, meta = (DisplayName = "fake reset efault"))
    int32 m_iResetDefault;
};
