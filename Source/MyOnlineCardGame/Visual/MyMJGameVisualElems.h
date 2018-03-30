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

    inline void reset()
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
    FString ToString() const
    {
        return FString::Printf(TEXT("[value %d, flip %s, posi %d:%s, vidxs %d:%d:%d:%d]"), m_iCardValue, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJCardFlipStateCpp"), (uint8)m_eFlipState), m_iIdxAttender, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJCardSlotTypeCpp"), (uint8)m_eSlot),
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
    FString ToString() const
    {
        return m_cVisualInfo.ToString() + m_cVisualResult.ToString();
    };

    UPROPERTY(BlueprintReadOnly)
    FMyMJGameCardVisualInfoCpp  m_cVisualInfo;

    UPROPERTY(BlueprintReadOnly)
    FMyMJGameActorVisualResultBaseCpp m_cVisualResult;
};

/*
USTRUCT(BlueprintType)
struct FMyMJGameDiceVisualInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDiceVisualInfoCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_eFlipState = MyMJCardFlipStateCpp::Invalid;
    }

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "dice visual state key"))
    int32 m_iDiceVisualStateKey = 0;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "dice value"))
        int32 m_iDiceValue;
}£»
*/

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


//most functions are implemented in C++, so fast and free to call
UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMoveWithSeqActorBaseCpp : public AActor, public IMyTransformUpdaterInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyMoveWithSeqActorBaseCpp();

    virtual ~AMyMoveWithSeqActorBaseCpp();

    virtual MyErrorCodeCommonPartCpp getModelInfo(struct FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const override;
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterTransformCpp*& outUpdater) override;

    inline UMyTransformUpdaterComponent* getMyTransformUpdaterComponent() const
    {
        return m_pMyTransformUpdaterComponent;
    };

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    static void helperTestAnimationStep(float time, FString debugStr, const TArray<AMyMoveWithSeqActorBaseCpp*>& actors);

protected:

    //virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    //return errcode if got wrong, 0 if OK, here it just adjust boxExtend to cover the mesh exactly
    virtual MyErrorCodeCommonPartCpp updateSettings();

    //components
    //root scene
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
        class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "main box"))
        class UBoxComponent *m_pMainBox;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "main static mesh"))
        class UStaticMeshComponent *m_pMainStaticMesh;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "my transform updater component"))
    UMyTransformUpdaterComponent* m_pMyTransformUpdaterComponent;

    UPROPERTY(EditDefaultsOnly, Category = "My Helper", meta = (DisplayName = "fake button to update settings"))
    bool m_bFakeUpdateSettings;

private:
    void createComponentsForCDO();
};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AMyMoveWithSeqActorBaseCpp, public IMyIdInterfaceCpp, public IMyCardGameValueRelatedObjectInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    //Todo: incomplete yet
    inline void reset()
    {
        m_cTargetToGoHistory.clearInGame();
    };


    virtual MyErrorCodeCommonPartCpp updateValueShowing(int32 newValueShowing, int32 animationTimeMs) override;
    virtual MyErrorCodeCommonPartCpp getValueShowing(int32& valueShowing) const override;
    virtual MyErrorCodeCommonPartCpp setResPath(const FDirectoryPath& newResPath) override;
    virtual MyErrorCodeCommonPartCpp getResPath(FDirectoryPath& resPath) const override;

    virtual MyErrorCodeCommonPartCpp getMyId(int32& outMyId) const override
    {
        outMyId = m_iMyId;
        return MyErrorCodeCommonPartCpp();
    };

    virtual MyErrorCodeCommonPartCpp setMyId(int32 myId) override
    {
        m_iMyId = myId;
        return MyErrorCodeCommonPartCpp();
    };


    UFUNCTION(BlueprintSetter)
    void setValueShowing2(int32 newValue)
    {
        updateValueShowing(newValue, 0);
    };

    UFUNCTION(BlueprintGetter)
    int32 getValueShowing2() const
    {
        int32 ret = -1;
        getValueShowing(ret);
        return ret;
    };

    UFUNCTION(BlueprintSetter)
    void setResPath2(const FDirectoryPath& newResPath)
    {
        setResPath(newResPath);
    };

    UFUNCTION(BlueprintGetter)
    const FDirectoryPath getResPath2() const
    {
        FDirectoryPath ret;
        getResPath(ret);

        return ret;
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

    static void helperMyMJCardsToMyTransformUpdaters(const TArray<AMyMJGameCardBaseCpp*>& aSub, bool bSort, TArray<IMyTransformUpdaterInterfaceCpp*> &aBase);

protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostInitializeComponents() override;

    //it make box aligh the bottom end point
    virtual MyErrorCodeCommonPartCpp updateSettings() override;

    MyErrorCodeCommonPartCpp checkAndLoadCardBasicResources(const FString &inPath);

    //the update rule is: always update if new settings arrive, and always reflect it even fail, never revert values
    MyErrorCodeCommonPartCpp updateVisual(bool bForce);
    MyErrorCodeCommonPartCpp updateWithCardBasicResources();
    MyErrorCodeCommonPartCpp updateWithValue(bool bForce);

    MyErrorCodeCommonPartCpp updateCardStaticMeshMIDParams(class UTexture* InBaseColor);

    MyErrorCodeCommonPartCpp helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture);


    //important values:
    //value showing now
    UPROPERTY(EditAnywhere, BlueprintSetter = setValueShowing2, BlueprintGetter = getValueShowing2, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;

    //shadow value to avoid duplicated update
    int32 m_iValueUpdatedBefore;

    //resouce settings, the child calss should specify them
    //Note: only one of ContentDir or RelativeToGameContentDir need to be specified to resulting relative path, their difference is dialog 
    //UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "card static mesh", ContentDir = "true", RelativeToGameContentDir = "true"))

    //where the card resource is, have special requirement such as mesh, material, texture, etc. example: /Game/Art/Models/MJCard/Type0
    UPROPERTY(EditDefaultsOnly, BlueprintSetter = setResPath2, BlueprintGetter = getResPath2, meta = (DisplayName = "resource path", ContentDir = "true"))
    FDirectoryPath m_cResPath;

    UPROPERTY(BlueprintReadOnly)
    class UStaticMesh *m_pResMesh;

    UPROPERTY(BlueprintReadOnly)
    class UMaterialInstance *m_pResMI;

    int32 m_iMyId;

    //where this card should go, but allow it not be there now(should move smoothly there)
    FMyCycleBuffer<FMyMJGameCardVisualInfoAndResultCpp> m_cTargetToGoHistory;
};


//dice doesn't support set value directly, they must be calculated by dice visual point
UCLASS(Blueprintable, Abstract)
class MYONLINECARDGAME_API AMyMJGameDiceBaseCpp : public AMyMoveWithSeqActorBaseCpp, public IMyIdInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyMJGameDiceBaseCpp();

    virtual ~AMyMJGameDiceBaseCpp();

    //Todo: incomplete yet
    inline void reset()
    {
        //m_cTargetToGoHistory.clearInGame();
    };

    virtual MyErrorCodeCommonPartCpp getMyId(int32& outMyId) const override
    {
        outMyId = m_iMyId;
        return MyErrorCodeCommonPartCpp();
    };

    virtual MyErrorCodeCommonPartCpp setMyId(int32 myId) override
    {
        m_iMyId = myId;
        return MyErrorCodeCommonPartCpp();
    };

    inline FMyErrorCodeMJGameCpp getDiceModelInfo(FMyMJDiceModelInfoBoxCpp& data, bool verifyValid = true) const
    {
        FMyErrorCodeMJGameCpp ret;
        while (1) {

            ret.m_eCommonPart = Super::getModelInfo(data.m_cBasic, verifyValid);
            if (ret.hasError()) {
                break;
            }

            ret = getDiceModelInfoExtra(data.m_cExtra);
            if (ret.hasError()) {
                break;
            }

            break;
        }

        if (verifyValid) {
            MY_VERIFY(!ret.hasError());
        }

        return ret;
    };

    //always success, otherwise core dump
    inline const FMyMJDiceModelInfoBoxCpp& getDiceModelInfoRefConstFromCache()
    {

        if (!m_cModelInfoCache.m_bValid) {
            getDiceModelInfo(m_cModelInfoCache.m_cData, true);
            m_cModelInfoCache.m_bValid = true;
        }

        return m_cModelInfoCache.m_cData;
    };

    inline void invalidModelInfoCache()
    {
        m_cModelInfoCache.m_bValid = false;
    };

    /*
    inline void addTargetToGoHistory(const FMyMJGameDiceVisualInfoAndResultCpp& cTargetToGo)
    {
        if (m_cTargetToGoHistory.isFull()) {
            m_cTargetToGoHistory.removeFromHead(1);
        };

        m_cTargetToGoHistory.addToTail(&cTargetToGo, NULL);
    };

    inline const FMyMJGameDiceVisualInfoAndResultCpp* getTargetToGoHistory(int32 idxFromLast, bool bVerifyValid = true) const
    {
        const FMyMJGameDiceVisualInfoAndResultCpp* ret;
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
    */

    //@idxOfDiceRandomArranged is the randome arranged sequece idx, and it may NOT equal to idxOfDice, acctually the shuffered idx in dices array. it must <= diceTotalNum
    static FTransform helperCalcFinalTransform(const FMyMJDiceModelInfoBoxCpp& diceModelInfo, const FMyMJGameDeskVisualPointCfgCpp& diceVisualPointCfg, int32 diceVisualStateKey, int32 idxOfDiceRandomArranged, int32 diceTotalNum, int32 value);

protected:

    struct FMyMJDiceModelInfoBoxCachedCpp
    {

    public:

        FMyMJDiceModelInfoBoxCachedCpp()
        {
            reset();
        };

        virtual ~FMyMJDiceModelInfoBoxCachedCpp()
        {

        };

        inline void reset()
        {
            m_bValid = false;
            m_cData.reset();
        };

        bool m_bValid;
        FMyMJDiceModelInfoBoxCpp m_cData;
    };

    //return errcode, 0 means no error
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    FMyErrorCodeMJGameCpp getDiceModelInfoExtra(FMyMJDiceModelInfoExtraCpp& diceModelInfoExtra) const;

    virtual FMyErrorCodeMJGameCpp getDiceModelInfoExtra_Implementation(FMyMJDiceModelInfoExtraCpp& diceModelInfoExtra) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDiceBaseCpp::getDiceModelInfoExtra() must be overrided by blueprint child class!"));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint);
    };

    FMyMJDiceModelInfoBoxCachedCpp m_cModelInfoCache;

    //FMyCycleBuffer<FMyMJGameDiceVisualInfoAndResultCpp> m_cTargetToGoHistory;

    int32 m_iMyId;
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