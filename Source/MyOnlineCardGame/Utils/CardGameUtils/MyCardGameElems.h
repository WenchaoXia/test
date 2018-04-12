// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCardGameUtilsLibrary.h"

#include "Blueprint/UserWidget.h"

#include "MyCardGameElems.generated.h"



UCLASS(BlueprintType)
class MYONLINECARDGAME_API AMyCardGameCardActorBaseCpp : public AMyWithCurveUpdaterTransformWorld3DBoxLikeActorBaseCpp, public IMyIdInterfaceCpp, public IMyValueInterfaceCpp, public IMyResourceInterfaceCpp
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


    static void helperMyMJGameCardActorBaseToMyTransformUpdaters(const TArray<AMyCardGameCardActorBaseCpp*>& aSub, bool bSort, TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> &aBase);

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
struct FMyCardGameDiceModelInfoCpp : public FMyModelInfoBoxWorld3DCpp
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, meta = (DisplayName = "local rotator for dice values"))
    TArray<FRotator> m_aLocalRotatorForDiceValues;

};

//for simple, dice is not set by value directly, but through world rotation 
UCLASS(BlueprintType)
class MYONLINECARDGAME_API AMyCardGameDiceActorBaseCpp : public AMyWithCurveUpdaterTransformWorld3DBoxLikeActorBaseCpp, public IMyIdInterfaceCpp
{
    GENERATED_BODY()

public:
    AMyCardGameDiceActorBaseCpp();
    virtual ~AMyCardGameDiceActorBaseCpp();


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
            refillCachedData();
        }

        return m_cModelInfoCache.m_cData;
    };

    inline void invalidCachedData()
    {
        m_cModelInfoCache.m_bValid = false;
    };

    inline void refillCachedData()
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

            FMyModelInfoWorld3DCpp modelInfoAll;

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
    static FTransform helperCalcFinalTransform(const FMyCardGameDiceModelInfoCpp& diceModelInfo, const FMyArrangePointCfgWorld3DCpp& diceVisualPointCfg, int32 diceVisualStateKey, int32 idxOfDiceRandomArranged, int32 diceTotalNum, int32 value);

protected:

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        invalidCachedData();
    };

    //it make box aligh in the center
    virtual MyErrorCodeCommonPartCpp updateSettings() override;

    //@basicInfo is the parent class info already get, BP can direct use it as part of @fullInfo, or overrite it
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
        MyErrorCodeCommonPartCpp getDiceModelInfoFromBP(const FMyModelInfoBoxWorld3DCpp& basicInfo, FMyCardGameDiceModelInfoCpp& fullInfo) const;

    virtual MyErrorCodeCommonPartCpp getDiceModelInfoFromBP_Implementation(const FMyModelInfoBoxWorld3DCpp& basicInfo, FMyCardGameDiceModelInfoCpp& fullInfo) const
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
class MYONLINECARDGAME_API UMyCardGameCardWidgetBaseCpp : public UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp, public IMyWidgetBasicOperationInterfaceCpp, public IMyIdInterfaceCpp, public IMyValueInterfaceCpp, public IMyResourceInterfaceCpp
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

        invalidCachedData();
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

};