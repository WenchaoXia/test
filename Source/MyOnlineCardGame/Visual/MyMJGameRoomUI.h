// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyMJGameVisualInterfaces.h"

#include "MyMJGameRoomUI.generated.h"


/*
UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameCardWidgetBaseCpp : public UUserWidget, public IMyWidgetBasicOperationInterfaceCpp, public IMyIdInterfaceCpp, public IMyCardGameValueRelatedObjectInterfaceCpp
{
    GENERATED_BODY()

public:

    UMyMJGameCardWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_iValueShowing = -1;
        m_cResPath.Path.Reset();
        m_iMyId = -1;
    };

    virtual ~UMyMJGameCardWidgetBaseCpp()
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

    IMyWidgetBasicOperationInterfaceCpp_DefaultImplementationForUObject();

    // < 0 means invalid, not set
    UPROPERTY(EditAnywhere, BlueprintSetter = setValueShowing2, BlueprintGetter = getValueShowing2, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;

    //where the card resource is, example: /Game/Art/Models/MJCard/Type0
    UPROPERTY(EditDefaultsOnly, BlueprintSetter = setResourcePath2, BlueprintGetter = getResourcePath2, meta = (DisplayName = "resource path", ContentDir = "true"))
    FDirectoryPath m_cResPath;

    UPROPERTY(BlueprintReadOnly)
    class UTexture2D *m_pCardMainTexture;

    int32 m_iMyId;
};
*/


USTRUCT()
struct FMyPlayerInfoBasicCpp
{
    GENERATED_USTRUCT_BODY()
public:

    UPROPERTY(EditAnywhere)
    FString m_sName;

    UPROPERTY(EditAnywhere)
    FString m_sHeadImageUrl;

    UPROPERTY(EditAnywhere)
    bool m_bMale;
};

USTRUCT()
struct FMyPlayerStateBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    // < 0 means offline
    UPROPERTY(EditAnywhere)
    int32 m_iPing;
};

USTRUCT()
struct FMyMJGamePlayerDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(EditAnywhere)
    FMyPlayerInfoBasicCpp m_cPlayerInfo;

    UPROPERTY(EditAnywhere)
    FMyPlayerStateBasicCpp m_cPlayerState;

    UPROPERTY(EditAnywhere)
    FMyInRoomViewRoleStyleSettingsCpp m_cStyleSettings;

    UPROPERTY(EditAnywhere)
    MyMJGameRoleTypeCpp m_eRoleType;
};

USTRUCT()
struct FMyPlayerInfoWidgetRuntimeDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyPlayerInfoWidgetRuntimeDataCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_iIdxAttender = -1;
        m_iIdxPositionInBox = -1;

        m_cLocationSelf = FVector2D::ZeroVector;
        m_cLocationUICenter = FVector2D::ZeroVector;
        m_cLocationCommonActionShowPoint = FVector2D::ZeroVector;

    };

    inline FString ToString() const
    {
        return FString::Printf(TEXT("(idxAttender %d, m_iIdxPositionInBox %d, m_cLocationSelf %s, m_cLocationUICenter %s, m_cLocationCommonActionShowPoint %s)"), m_iIdxAttender, m_iIdxPositionInBox, *m_cLocationSelf.ToString(), *m_cLocationUICenter.ToString(), *m_cLocationCommonActionShowPoint.ToString());
    };

    int32 m_iIdxAttender;
    int32 m_iIdxPositionInBox; //not always continues, may be 0,2; 0,1,3;, 0,1,2,3

    FVector2D m_cLocationSelf;
    FVector2D m_cLocationUICenter;
    FVector2D m_cLocationCommonActionShowPoint;
};

USTRUCT()
struct FMyPlayerInfoWidgetCachedDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyPlayerInfoWidgetCachedDataCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_bValid = false;
        m_pDynamicAllocationCanvasPanel = NULL;
    };


    bool m_bValid;

    UPROPERTY()
    class UCanvasPanel* m_pDynamicAllocationCanvasPanel;
};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomPlayerInfoWidgetBaseCpp : public UMyWithCurveUpdaterTransformWidget2DBoxWidgetBaseCpp, public IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomPlayerInfoWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {

    };

    virtual ~UMyMJGameInRoomPlayerInfoWidgetBaseCpp()
    {

    };

    inline FMyPlayerInfoWidgetRuntimeDataCpp& getRuntimeDataRef()
    {
        return m_cRuntimeData;
    };


    FMyErrorCodeMJGameCpp showAttenderWeave(float dur, MyMJGameEventVisualTypeCpp weaveVisualType);


protected:

    IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other()
    IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached_Style0(m_cCachedData, m_pDynamicAllocationCanvasPanel)

    virtual void OnWidgetRebuilt() override
    {
        Super::OnWidgetRebuilt();

        m_cRuntimeData.reset();
        invalidCachedData();
    };


    //if @createIfNotExist, always exist
    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp * getManagedWidget(int32 key, TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>& widgetClass, bool createIfNotExist = true);

    FMyPlayerInfoWidgetRuntimeDataCpp m_cRuntimeData;


    TMap<int32, UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* > m_mManagedWidgets;

private:

    inline void invalidCachedData() 
    {
        m_cCachedData.reset();
    };

    inline MyErrorCodeCommonPartCpp refillCachedData()
    {
        m_cCachedData.reset();
        MyErrorCodeCommonPartCpp ret = IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp::Execute_getDynamicAllocationRootCanvasPanel(this, m_cCachedData.m_pDynamicAllocationCanvasPanel);
        if (ret == MyErrorCodeCommonPartCpp::NoError) {
            MY_VERIFY(m_cCachedData.m_pDynamicAllocationCanvasPanel);
            m_cCachedData.m_bValid = true;
        }
        else {
            m_cCachedData.reset();
        }

        return ret;
    };

    FMyPlayerInfoWidgetCachedDataCpp m_cCachedData;
};



UCLASS(Abstract, editinlinenew, BlueprintType, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomChoiceSelectWidgetBaseCpp : public UUserWidget, public IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomChoiceSelectWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        reset();
    };

    inline void reset()
    {
        m_iIdxOfSelection = -1;
    };


    UFUNCTION(BlueprintPure)
    int32 getIdxOfSelection() const
    {
        return m_iIdxOfSelection;
    };

    UFUNCTION(BlueprintCallable)
        MyErrorCodeCommonPartCpp updateChoiceDataIgnoreIdxOfSelection(bool haveAnySelection, bool selected, bool InteractiveEnabled)
    {
        MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

        MyErrorCodeCommonPartJoin(ret, IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp::Execute_setChoiceFillState(this, haveAnySelection));
        MyErrorCodeCommonPartJoin(ret, IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp::Execute_setChoiceSelectState(this, selected));
        MyErrorCodeCommonPartJoin(ret, IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp::Execute_setChoiceInteractiveEnableState(this, InteractiveEnabled));

        if (ret != MyErrorCodeCommonPartCpp::NoError) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateChoiceDataIgnoreIdxOfSelection got error: %s."), *GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
        }

        return ret;
    };

    UFUNCTION(BlueprintCallable)
        MyErrorCodeCommonPartCpp updateChoiceData(int32 idxOfSelection, bool selected, bool InteractiveEnabled)
    {
        m_iIdxOfSelection = idxOfSelection;
        return updateChoiceDataIgnoreIdxOfSelection(m_iIdxOfSelection >= 0, selected, InteractiveEnabled);
    };

protected:

    IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp_DefaultImplementationForUObject_Bp()


    int32 m_iIdxOfSelection;
};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomChoiceSelectCommonWidgetBaseCpp : public UMyMJGameInRoomChoiceSelectWidgetBaseCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomChoiceSelectCommonWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        reset();
    };

    inline void reset()
    {
    };


protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    virtual void SynchronizeProperties() override
    {
        Super::SynchronizeProperties();
        tryUpdateCenterButtonStyle();
        return;
    }

    inline void tryUpdateCenterButtonStyle()
    {
        UMyButton* pButton = NULL;
        getCenterButton(pButton);

        if (IsValid(pButton)) {
            pButton->tryUpdateStylesInUnifiedWay(m_cCenterButtonStyleNormal, true);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: failed get center button as %p."), *GetName(), pButton);
        }
    };

    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getCenterButton(UMyButton*& outWidget);

    MyErrorCodeCommonPartCpp getCenterButton_Implementation(UMyButton*& outWidget) 
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getCenterButton only implemented in C++."), *GetClass()->GetName());
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "center button style normal"))
    FSlateBrush m_cCenterButtonStyleNormal;
};

UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomChoiceSelectChiWidgetBaseCpp : public UMyMJGameInRoomChoiceSelectWidgetBaseCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomChoiceSelectChiWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        reset();
    };

    inline void reset()
    {
    };

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp updateChoiceDataChi(const FMyMJGameActionChoiceDataChiCpp& dataChi);

protected:

    MyErrorCodeCommonPartCpp updateChoiceDataChi_Implementation(const FMyMJGameActionChoiceDataChiCpp& dataChi)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateChoiceDataChi only implemented in C++."), *GetClass()->GetName());
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
    };

};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomOperationLvl1ActionPanelWidgetBaseCpp : public UUserWidget, public IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomOperationLvl1ActionPanelWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        reset();
    };

    inline void reset()
    {
    };



protected:

    IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp_DefaultImplementationForUObject_Bp();
        //void acceptClientUserSelect(int32 IdxOfSelection);
};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomOperationLvl2ChiPanelWidgetBaseCpp : public UUserWidget, public IMyMJGameInRoomOperationLvl2ChiPanelWidgetInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomOperationLvl2ChiPanelWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        reset();
    };

    inline void reset()
    {
    };



protected:

    IMyMJGameInRoomOperationLvl2ChiPanelWidgetInterfaceCpp_DefaultImplementationForUObject_Bp();

        //void acceptClientUserSelect(int32 IdxOfSelection);
};

/*
USTRUCT()
struct FMyMJGameInRoomOperationRootPanelWidgetBaseCachedDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameInRoomOperationRootPanelWidgetBaseCachedDataCpp()
    {
        reset();
    };

    virtual ~FMyMJGameInRoomOperationRootPanelWidgetBaseCachedDataCpp()
    {

    };

    inline void reset()
    {
        m_bValid = false;

        m_pLv1ActionPanel = NULL;
        m_pLv2ChiPanel = NULL;
    };


    bool m_bValid;

    //UPROPERTY()
    //class UWidgetSwitcher* m_pWidgetSwitcher;

    UPROPERTY()
        UMyMJGameInRoomOperationLvl1ActionPanelWidgetBaseCpp* m_pLv1ActionPanel;

    UPROPERTY()
        UMyMJGameInRoomOperationLvl2ChiPanelWidgetBaseCpp* m_pLv2ChiPanel;
};
*/


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomOperationRootPanelWidgetBaseCpp : public UUserWidget, public IMyMJGameInRoomOperationRootPanelWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    UMyMJGameInRoomOperationRootPanelWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_iDelayedUIUpdateTimeMs = 0;
    };

    void updateWithActionContainor(MyMJGameRoleTypeCpp eDataRoleType, MyMJGameRuleTypeCpp eRuleType, int32 iGameId, int32 iActionGroupId, int32 idxAttender,
                                   const FMyMJCardValuePackCpp& cardValuePack,
                                   const FMyMJGameActionContainorForBPCpp& actionContainor);

    UFUNCTION(BlueprintCallable)
    void makeSelection(int32 IdxOfSelection);


protected:

    IMyMJGameInRoomOperationRootPanelWidgetInterface_DefaultImplementationForUObject_Bp();

    struct FActionChoicesUpdateStateCpp
    {

    public:

        FActionChoicesUpdateStateCpp()
        {
            reset();
        };

        inline void reset()
        {
            m_iGameId = m_iActionGroupId = m_iIdxAttender = -1;
            m_eDataRoleType = MyMJGameRoleTypeCpp::Max;
            m_iIdxOfSelected = -1;
            m_iCountOfChoices = 0;
        };

        inline bool equal(const FActionChoicesUpdateStateCpp& other) const
        {
            return m_iGameId == other.m_iGameId && m_iActionGroupId == other.m_iActionGroupId && m_iIdxAttender == other.m_iIdxAttender && m_eDataRoleType == other.m_eDataRoleType &&
                   m_iIdxOfSelected == other.m_iIdxOfSelected && m_iCountOfChoices == other.m_iCountOfChoices;
        };

        inline FString ToString() const
        {
            return FString::Printf(TEXT("m_iGameId %d, m_iActionGroupId %d, m_iIdxAttender %d, m_eDataRoleType %d, m_iIdxOfSelected %d, m_iCountOfChoices %d."),
                   m_iGameId, m_iActionGroupId, m_iIdxAttender, (uint8)m_eDataRoleType, m_iIdxOfSelected, m_iCountOfChoices);
        };

        int32 m_iGameId;
        int32 m_iActionGroupId;
        int32 m_iIdxAttender;
        MyMJGameRoleTypeCpp m_eDataRoleType;

        int32 m_iIdxOfSelected;
        int32 m_iCountOfChoices;
    };



    struct FMyMJGameActionChoiceDataOtherCpp
    {
    public:

        FMyMJGameActionChoiceDataOtherCpp()
        {
            reset();
        };

        inline void reset()
        {
            m_iIdxOfSelection = -1;
            m_bSelected = false;
            m_bInteractiveEnabled = false;
        };

        inline bool isValid() const
        {
            return m_iIdxOfSelection >= 0;
        };

        int32 m_iIdxOfSelection;
        bool m_bSelected;
        bool m_bInteractiveEnabled;
    };


    struct FUIUpdateDataCpp : public FActionChoicesUpdateStateCpp
    {
    public:

        FUIUpdateDataCpp() : FActionChoicesUpdateStateCpp()
        {
            m_bHaveChiSelected = false;
            m_iCountOfGuoChoices = 0;
        };

        inline void reset()
        {
            FActionChoicesUpdateStateCpp::reset();
            m_aChiDatas.Reset();
            m_cPengData.reset();
            m_cGangData.reset();
            m_cBuData.reset();
            m_cHuData.reset();
            m_cGuoData.reset();

            m_bHaveChiSelected = false;
            m_iCountOfGuoChoices = 0;
        };

        void postProcessForInteractiveFlag()
        {
            bool bCanBeSelectedLater = m_iIdxOfSelected < 0; //not selected yet

            for (int32 i = 0; i < m_aChiDatas.Num(); i++) {
                m_aChiDatas[i].m_bInteractiveEnabled = m_aChiDatas[i].m_iIdxOfSelection >= 0 && bCanBeSelectedLater;
            }
            m_cPengData.m_bInteractiveEnabled = m_cPengData.m_iIdxOfSelection >= 0 && bCanBeSelectedLater;
            m_cGangData.m_bInteractiveEnabled = m_cGangData.m_iIdxOfSelection >= 0 && bCanBeSelectedLater;
            m_cBuData.m_bInteractiveEnabled = m_cBuData.m_iIdxOfSelection >= 0 && bCanBeSelectedLater;
            m_cHuData.m_bInteractiveEnabled = m_cHuData.m_iIdxOfSelection >= 0 && bCanBeSelectedLater;
            m_cGuoData.m_bInteractiveEnabled = m_cGuoData.m_iIdxOfSelection >= 0 && bCanBeSelectedLater;
        };


        TArray<FMyMJGameActionChoiceDataChiCpp> m_aChiDatas;
        FMyMJGameActionChoiceDataOtherCpp m_cPengData;
        FMyMJGameActionChoiceDataOtherCpp m_cGangData;
        FMyMJGameActionChoiceDataOtherCpp m_cBuData;
        FMyMJGameActionChoiceDataOtherCpp m_cHuData;
        FMyMJGameActionChoiceDataOtherCpp m_cGuoData;

        bool m_bHaveChiSelected;
        int32 m_iCountOfGuoChoices;
    };


    void delayedUIUpdate();
    void UIUpdate(const FUIUpdateDataCpp& data);


    FActionChoicesUpdateStateCpp m_cUpdateStateLast;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "delayed UI Update time ms"))
    int32 m_iDelayedUIUpdateTimeMs;

    FTimerHandle m_cDelayedUIUpdateTimerHandle;
    FUIUpdateDataCpp m_cDelayedUIUpdateData;
};



USTRUCT()
struct FMyMJGameInRoomUIMainWidgetRuntimeDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameInRoomUIMainWidgetRuntimeDataCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_idxAttenderForIdxPositionInBox0 = -1;
        m_bCanGiveCmd = false;
        m_bInited = false;
    };

    //< 0 means unknown
    int32 m_idxAttenderForIdxPositionInBox0;

    bool m_bCanGiveCmd;
    bool m_bInited;
};

USTRUCT()
struct FMyMJGameInRoomUIMainWidgetCachedDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameInRoomUIMainWidgetCachedDataCpp()
    {
        reset();
    };

    virtual ~FMyMJGameInRoomUIMainWidgetCachedDataCpp()
    {

    };

    inline void reset()
    {
        m_bValid = false;
        m_cModelInfo.reset();
        m_pDynamicAllocationCanvasPanel = NULL;

        m_cCfg.reset();
        m_aPlayerInfoWidgets.Reset();
        m_pOperationRootPanelWidget = NULL;
    };


    bool m_bValid;

    FMyModelInfoWidget2DCpp m_cModelInfo; //contains size info

    UPROPERTY()
    class UCanvasPanel* m_pDynamicAllocationCanvasPanel;

    FMyMJGameInRoomUIMainWidgetCfgCpp m_cCfg;

    //size is always 4, idx is idx attender
    UPROPERTY()
    TArray<UMyMJGameInRoomPlayerInfoWidgetBaseCpp* > m_aPlayerInfoWidgets;

    UPROPERTY()
    UMyMJGameInRoomOperationRootPanelWidgetBaseCpp* m_pOperationRootPanelWidget;
};


USTRUCT()
struct FMyMJGameInRoomUIMainWidgetDirtyRecordsCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameInRoomUIMainWidgetDirtyRecordsCpp()
    {
        reset();
    };

    inline void reset()
    {
        FMyMJDataAccessorCpp::helperSetCoreDataDirtyRecordDataDirty(m_cCoreDataDirtyRecord);
    };

    FMyDirtyRecordWithKeyAnd4IdxsMapCpp m_cCoreDataDirtyRecord;
};

UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomUIMainWidgetBaseCpp : public UUserWidget, public IMyContentSizeWidget2DInterfaceCpp, public IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp, public IMyMJGameInRoomUIMainWidgetInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomUIMainWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : UUserWidget(ObjectInitializer)
    {
        reset();
    };

    virtual ~UMyMJGameInRoomUIMainWidgetBaseCpp()
    {

    };

    inline void reset()
    {
        m_pDataSourceRoomActor = NULL;
    };


    virtual FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVisualType) override;
    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPublicChanged(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic, int32 subType) override;
    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPrivateChanged(int32 idxAttender, const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate, int32 subType) override;
    virtual FMyErrorCodeMJGameCpp updateAttenderPositions(float XYRatioOfplayerScreen, const TArray<FVector2D>& projectedPointsInPlayerScreen_unit_absolute) override;


    inline void markCoreDataDirty(const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecordFiltered)
    {
        m_cDirtyRecords.m_cCoreDataDirtyRecord.join(cCoreDataDirtyRecordFiltered);
    };

    void updateUI();

    inline void setDataSourceRoomActor(class AMyMJGameRoomCpp* pDataSourceRoomActor)
    {
        m_pDataSourceRoomActor = pDataSourceRoomActor;
    };


    FMyErrorCodeMJGameCpp getCfgRefConstFromCache(const FMyMJGameInRoomUIMainWidgetCfgCpp*& pCfg, bool verifyValid)
    {
        pCfg = NULL;

        FMyErrorCodeMJGameCpp ret(true);
        if (!m_cCachedData.m_bValid) {
            ret = refillCachedData();
        }

        if (ret.hasError()) {
            if (verifyValid) {
                MY_VERIFY(false);
            }
            return ret;
        }

        pCfg = &m_cCachedData.m_cCfg;

        MY_VERIFY(pCfg);

        return ret;
    };

protected:

    IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other()
    IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_getContentSizeFromCache_Style0(m_cCachedData, m_cModelInfo.getBox2DRefConst().m_cBoxExtend * 2, refillCachedData)
    IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_setContentSizeThroughCache_Style0(invalidCachedData)

    IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other()
    IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached_Style0(m_cCachedData, m_pDynamicAllocationCanvasPanel)

    IMyMJGameInRoomUIMainWidgetInterfaceCpp_DefaultImplementationForUObject_Bp()


    virtual void OnWidgetRebuilt() override
    {
        Super::OnWidgetRebuilt();

        m_cRuntimeData.reset();
        invalidCachedData();

        for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
            UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pW = NULL;
            if (getInRoomPlayerInfoWidgetByIdxAttenderFromCache(idxAttender, pW, false).hasError()) {
                break;
            }

            pW->updateSlotSettingsToComply_MyModelInfoWidget2D();
        }

        m_cDirtyRecords.reset();
    };


    //error will always be logged, and return OK means @outWidget never NULL
    FMyErrorCodeMJGameCpp  getInRoomPlayerInfoWidgetByIdxAttenderFromCache(int32 idxAttender, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget, bool verifyValid)
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

        outWidget = NULL;

        FMyErrorCodeMJGameCpp ret(true);
        if (!m_cCachedData.m_bValid) {
            ret = refillCachedData();
        }

        if (ret.hasError()) {
            if (verifyValid) {
                MY_VERIFY(false);
            }
            return ret;
        }

        MY_VERIFY(m_cCachedData.m_aPlayerInfoWidgets.Num() >= 4);

        outWidget = m_cCachedData.m_aPlayerInfoWidgets[idxAttender];

        MY_VERIFY(IsValid(outWidget));

        return ret;
    };


    FMyErrorCodeMJGameCpp getOperationRootPanelWidgetRefFromCache(UMyMJGameInRoomOperationRootPanelWidgetBaseCpp*& pW, bool verifyValid)
    {
        pW = NULL;

        FMyErrorCodeMJGameCpp ret(true);
        if (!m_cCachedData.m_bValid) {
            ret = refillCachedData();
        }

        if (ret.hasError()) {
            if (verifyValid) {
                MY_VERIFY(false);
            }
            return ret;
        }

        pW = m_cCachedData.m_pOperationRootPanelWidget;

        MY_VERIFY(IsValid(pW));

        return ret;
    };


    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp changeStateCanGiveCmd(bool canGiveCmd);

    MyErrorCodeCommonPartCpp changeStateCanGiveCmd_Implementation(bool canGiveCmd)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: changeStateCanGiveCmd only implemented in C++."), *GetClass()->GetName());
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint;
    };


    UPROPERTY()
    class AMyMJGameRoomCpp* m_pDataSourceRoomActor;


    FMyMJGameInRoomUIMainWidgetDirtyRecordsCpp m_cDirtyRecords;
    FMyMJGameInRoomUIMainWidgetRuntimeDataCpp m_cRuntimeData;

private:

    //just to comply with update function macro
    inline MyErrorCodeCommonPartCpp invalidCachedData()
    {
        m_cCachedData.reset();
        return MyErrorCodeCommonPartCpp::NoError;
    };

    //If error got, always a log will be generated, so caller need NOT log again
    MyErrorCodeCommonPartCpp refillCachedData();

    FMyMJGameInRoomUIMainWidgetCachedDataCpp m_cCachedData;
};
