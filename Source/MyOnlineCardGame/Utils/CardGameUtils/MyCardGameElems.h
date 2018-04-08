// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCardGameUtilsLibrary.h"

#include "Blueprint/UserWidget.h"

#include "MyCardGameElems.generated.h"


UENUM()
enum class MyCardGameHorizontalAlignmentCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Left = 1     UMETA(DisplayName = "Left"),
    Mid = 2    UMETA(DisplayName = "Mid"),
    Right = 3    UMETA(DisplayName = "Right")
};

UENUM()
enum class MyCardGameVerticalAlignmentCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Bottom = 1     UMETA(DisplayName = "Bottom"),
    Mid = 2    UMETA(DisplayName = "Mid"),
    Top = 3    UMETA(DisplayName = "Top")
};


//Can change in runtime, and caller should invalidate cache after change
//Use UE4's XYZ coordinate, stack align is not list but always from -Z to +Z
USTRUCT(BlueprintType)
struct FMyCardGameVisualPointCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCardGameVisualPointCfgCpp() : m_cAreaBoxExtendFinal(0)
    {
        m_eRowAlignment = MyCardGameVerticalAlignmentCpp::Invalid;
        //m_iRowMaxNum = 0;
        m_eColInRowAlignment = MyCardGameHorizontalAlignmentCpp::Invalid;
        //m_iColInRowMaxNum = 0;

        m_fColInRowExtraMarginAbs = 0;
        m_iRowMax = 0;
        m_iExtra0 = 0;
    };

    virtual ~FMyCardGameVisualPointCfgCpp()
    {

    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point world transform"))
        FTransform m_cCenterPointWorldTransform;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "area box extend final"))
        FVector m_cAreaBoxExtendFinal;

    //from -X to +X
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row alignment"))
        MyCardGameVerticalAlignmentCpp m_eRowAlignment;

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row max num"))
    //int32 m_iRowMaxNum;

    //two layer of meaning: single card arrange sequence, all card actors gather point
    //from -Y to +Y, actually it contains two aspect: card sequence and actor alignment, left = card seq left + actor align left, for simple we use one property here. Warn: mid = card seq left + actor align mid, and only valid for hand and just taken slot now!
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row alignment"))
        MyCardGameHorizontalAlignmentCpp m_eColInRowAlignment;

    //assumed card num, if exceed the position may be out of range but will not crash the program
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row max num"))
    //int32 m_iColInRowMaxNum;

    //must >= 0
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin abs"))
        float m_fColInRowExtraMarginAbs;

    //if row number exceed this, the actor should stack. If it is 0, means no limit
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row max"))
        int32 m_iRowMax;

    //in case some unstandard cfg is needed, use this 
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "extra 0"))
        int32 m_iExtra0;
};


USTRUCT(BlueprintType)
struct FMyCardGameBoxLikeElemVisualInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCardGameBoxLikeElemVisualInfoCpp()
    {
        reset();
    };

    //virtual function to allow reset by base pointer
    virtual void reset(bool resetSubClassDataonly = false)
    {
        m_eFlipState = MyCardGameBoxLikeElemFlipStateCpp::Invalid;

        m_iIdxRow = 0;
        m_iIdxColInRow = 0;
        //m_iCountOfColInRow = 0;
        m_iIdxStackInCol = 0;

        m_iRotateX90D = 0;
        m_iRotateX90DBeforeCount = 0;
        m_iColInRowExtraMarginCount = 0;

        m_iHelperIdxColInRowReal = -1;
    };

    inline bool equal(const FMyCardGameBoxLikeElemVisualInfoCpp& other) const
    {
        if (&other == this) {
            return true;
        }

        return m_eFlipState == other.m_eFlipState &&
            m_iIdxRow == other.m_iIdxRow && m_iIdxColInRow == other.m_iIdxColInRow && m_iIdxStackInCol == other.m_iIdxStackInCol &&
            m_iRotateX90D == other.m_iRotateX90D && m_iRotateX90DBeforeCount == other.m_iRotateX90DBeforeCount && m_iColInRowExtraMarginCount == other.m_iColInRowExtraMarginCount &&
            m_iHelperIdxColInRowReal == other.m_iHelperIdxColInRowReal;
    };

    virtual FString ToString() const
    {
        return FString::Printf(TEXT("flip %s, vidxs %d:%d:%d:%d"), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyCardGameBoxLikeElemFlipStateCpp"), (uint8)m_eFlipState),
            m_iIdxRow, m_iIdxColInRow, m_iIdxStackInCol, m_iHelperIdxColInRowReal);
    };

    static void helperResolveTransform(const FMyCardGameVisualPointCfgCpp& cVisualPointCfg,
                                       const FMyModelInfoBox3DCpp& cModelInfo,
                                       const FMyCardGameBoxLikeElemVisualInfoCpp& cVisualInfo,
                                       FTransform& outTransform);



    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "flip state"))
    MyCardGameBoxLikeElemFlipStateCpp m_eFlipState;

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

    // >= 0 means valid;
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "helper idx col in row real"))
        int32 m_iHelperIdxColInRowReal;
};




UCLASS(BlueprintType)
class MYONLINECARDGAME_API AMyCardGameCardActorBaseCpp : public AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp, public IMyIdInterfaceCpp, public IMyValueInterfaceCpp, public IMyResourceInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyCardGameCardActorBaseCpp();

    virtual ~AMyCardGameCardActorBaseCpp();


    virtual MyErrorCodeCommonPartCpp updateValueShowing(int32 newValueShowing, int32 animationTimeMs) override;
    virtual MyErrorCodeCommonPartCpp getValueShowing(int32& valueShowing) const override;
    virtual MyErrorCodeCommonPartCpp setResourcePath(const FDirectoryPath& newResPath) override;
    virtual MyErrorCodeCommonPartCpp getResourcePath(FDirectoryPath& resPath) const override;

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


    static void helperMyMJGameCardActorBaseToMyTransformUpdaters(const TArray<AMyCardGameCardActorBaseCpp*>& aSub, bool bSort, TArray<IMyWithCurveUpdaterTransformInterfaceCpp*> &aBase);

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
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "resource path", ContentDir = "true"))
        FDirectoryPath m_cResPath;

    UPROPERTY(BlueprintReadOnly)
        class UStaticMesh *m_pResMesh;

    UPROPERTY(BlueprintReadOnly)
        class UMaterialInstance *m_pResMI;

    int32 m_iMyId;

};


USTRUCT(BlueprintType)
struct FMyCardGameDiceModelInfoCpp : public FMyModelInfoBox3DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCardGameDiceModelInfoCpp() : Super()
    {
        reset(true);
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        m_aLocalRotatorForDiceValues.Reset();
        for (int32 i = 0; i < 7; i++) {
            m_aLocalRotatorForDiceValues.Emplace(FRotator::ZeroRotator);
        }
    };

    //input is expected as in 1 to 6
    inline const FRotator& getLocalRotatorForDiceValueRefConst(int32 value) const
    {
        MY_VERIFY(value > 0 && value < 7);
        MY_VERIFY(value < m_aLocalRotatorForDiceValues.Num());
        return m_aLocalRotatorForDiceValues[value];
    };


protected:

    //the array's index means the value, and the array size is always 7
    UPROPERTY(BlueprintReadWrite, EditAnywhere, EditFixedSize, meta = (DisplayName = "local rotator for dice values"))
    TArray<FRotator> m_aLocalRotatorForDiceValues;

};

//for simple, dice is not set by value directly, but through world rotation 
UCLASS(BlueprintType)
class MYONLINECARDGAME_API AMyCardGameDiceActorBaseCpp : public AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp, public IMyIdInterfaceCpp
{
    GENERATED_BODY()

public:
    AMyCardGameDiceActorBaseCpp();
    virtual ~AMyCardGameDiceActorBaseCpp();

    //Todo: incomplete yet
    inline void reset()
    {
        invalidModelInfoCache();
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


    //always success, otherwise core dump
    inline const FMyCardGameDiceModelInfoCpp& getDiceModelInfoRefConstFromCache()
    {

        if (!m_cModelInfoCache.m_bValid) {
            refillModelInfoCache();
        }

        return m_cModelInfoCache.m_cData;
    };

    inline void invalidModelInfoCache()
    {
        m_cModelInfoCache.m_bValid = false;
    };

    inline void refillModelInfoCache()
    {
        m_cModelInfoCache.reset();
        getDiceModelInfoNotFromCache(m_cModelInfoCache.m_cData, true);
        m_cModelInfoCache.m_bValid = true;
    };

    //go through cache layer and leave actor untouched, may fail if verify is false
    inline MyErrorCodeCommonPartCpp getDiceModelInfoNotFromCache(FMyCardGameDiceModelInfoCpp& fullInfo, bool verify) const
    {
        MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
        while (1) {

            FMyModelInfoCpp modelInfoAll;

            ret = getModelInfo(modelInfoAll, verify);
            if (ret != MyErrorCodeCommonPartCpp::NoError) {
                break;
            }

            ret = getDiceModelInfoFromBP(modelInfoAll.getBox3DRefConst(), fullInfo);
            if (ret != MyErrorCodeCommonPartCpp::NoError) {
                break;
            }

            break;
        }

        if (verify) {
            MY_VERIFY(ret == MyErrorCodeCommonPartCpp::NoError);
        }

        return ret;
    };

    //@idxOfDiceRandomArranged is the randome arranged sequece idx, and it may NOT equal to idxOfDice, acctually the shuffered idx in dices array. it must <= diceTotalNum
    static FTransform helperCalcFinalTransform(const FMyCardGameDiceModelInfoCpp& diceModelInfo, const FMyCardGameVisualPointCfgCpp& diceVisualPointCfg, int32 diceVisualStateKey, int32 idxOfDiceRandomArranged, int32 diceTotalNum, int32 value);

protected:

    //it make box aligh in the center
    virtual MyErrorCodeCommonPartCpp updateSettings() override;

    //@basicInfo is the parent class info already get, BP can direct use it as part of @fullInfo, or overrite it
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
        MyErrorCodeCommonPartCpp getDiceModelInfoFromBP(const FMyModelInfoBox3DCpp& basicInfo, FMyCardGameDiceModelInfoCpp& fullInfo) const;

    virtual MyErrorCodeCommonPartCpp getDiceModelInfoFromBP_Implementation(const FMyModelInfoBox3DCpp& basicInfo, FMyCardGameDiceModelInfoCpp& fullInfo) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDiceBaseCpp::getDiceModelInfoFromBP() must be overrided by blueprint child class!"));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint;
    };


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
        FMyCardGameDiceModelInfoCpp m_cData;
    };

    FMyMJDiceModelInfoBoxCachedCpp m_cModelInfoCache;

    int32 m_iMyId;
};



USTRUCT()
struct FMyCardGameCardWidgetCachedDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyCardGameCardWidgetCachedDataCpp()
    {
        reset();
    };

    virtual ~FMyCardGameCardWidgetCachedDataCpp()
    {

    };

    inline void reset()
    {
        m_bValid = false;
        m_pCenterButton = NULL;
    };


    bool m_bValid;

    UPROPERTY()
    UMyButton* m_pCenterButton;
};



UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyCardGameCardWidgetBaseCpp : public UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp, public IMyWidgetBasicOperationInterfaceCpp, public IMyIdInterfaceCpp, public IMyValueInterfaceCpp, public IMyResourceInterfaceCpp
{
    GENERATED_BODY()

public:

    UMyCardGameCardWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_iValueShowing = MyCardGameValueUnknown;
        m_iValueUpdatedBefore = m_iValueShowing - 1;

        m_cResPath.Path.Reset();
        m_iMyId = -1;
        m_pCardMainTexture = NULL;
    };

    virtual ~UMyCardGameCardWidgetBaseCpp()
    {

    };


    virtual MyErrorCodeCommonPartCpp updateValueShowing(int32 newValueShowing, int32 animationTimeMs) override;
    virtual MyErrorCodeCommonPartCpp getValueShowing(int32& valueShowing) const override;
    virtual MyErrorCodeCommonPartCpp setResourcePath(const FDirectoryPath& newResPath) override;
    virtual MyErrorCodeCommonPartCpp getResourcePath(FDirectoryPath& resPath) const override;

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
        void setResourcePath2(const FDirectoryPath& newResPath)
    {
        setResourcePath(newResPath);
    };

    UFUNCTION(BlueprintGetter)
        const FDirectoryPath getResourcePath2() const
    {
        FDirectoryPath ret;
        getResourcePath(ret);

        return ret;
    };


protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    virtual void OnWidgetRebuilt() override
    {
        Super::OnWidgetRebuilt();

        updateWithValue(true);
    };


    IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject();

    MyErrorCodeCommonPartCpp updateWithValue(bool bForce);

    MyErrorCodeCommonPartCpp helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutMainCardTexture);

    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getCenterButtonFromBP(UMyButton*& button);

    inline MyErrorCodeCommonPartCpp getCenterButtonFromBP_Implementation(UMyButton*& button)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getCenterButtonFromBP not implemented in BP!"), *GetClass()->GetName());
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint;
    };

    inline void invalidCachedData()
    {
        m_cCachedData.m_bValid = false;
    };

    inline void refillCachedData()
    {
        m_cCachedData.reset();
        MyErrorCodeCommonPartCpp ret = getCenterButtonFromBP(m_cCachedData.m_pCenterButton);

        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getCenterButtonFromBP return fail: %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyErrorCodeCommonPartCpp"), (uint8)ret));
        }
    };

    inline UMyButton* getCenterButton(bool verify = true)
    {
        if (!m_cCachedData.m_bValid) {
            refillCachedData();
        }

        if (!IsValid(m_cCachedData.m_pCenterButton)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("center button invalid: %p."), m_cCachedData.m_pCenterButton);
            if (verify) {
                MY_VERIFY(false);
            }
        }

        return m_cCachedData.m_pCenterButton;
    };


    // < 0 means invalid, not set
    UPROPERTY(EditAnywhere, BlueprintSetter = setValueShowing2, BlueprintGetter = getValueShowing2, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;

    int32 m_iValueUpdatedBefore;

    //where the card resource is, example: /Game/Art/Models/MJCard/Type0
    UPROPERTY(EditDefaultsOnly, BlueprintSetter = setResourcePath2, BlueprintGetter = getResourcePath2, meta = (DisplayName = "resource path", ContentDir = "true"))
        FDirectoryPath m_cResPath;

    int32 m_iMyId;

    UPROPERTY(BlueprintReadOnly)
        class UTexture *m_pCardMainTexture;

    FMyCardGameCardWidgetCachedDataCpp m_cCachedData;

};






UCLASS()
class UMyCardGameElemsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    static void helperResolveBoxLikeElemTransformForBp(const FMyCardGameVisualPointCfgCpp& cVisualPointCfg,
                                                        const FMyModelInfoBox3DCpp& cModelInfo,
                                                        const FMyCardGameBoxLikeElemVisualInfoCpp& cVisualInfo,
                                                        FTransform& outTransform)
    {
        FMyCardGameBoxLikeElemVisualInfoCpp::helperResolveTransform(cVisualPointCfg, cModelInfo, cVisualInfo, outTransform);
    };
};