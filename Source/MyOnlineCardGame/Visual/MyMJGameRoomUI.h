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
struct FMyInRoomViewRoleEventStyleSettingsCpp
{
    GENERATED_USTRUCT_BODY()
public:

    FMyInRoomViewRoleEventStyleSettingsCpp()
    {

    };

    inline bool checkSettings() const
    {
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeaveChiWidget, TEXT("weaveChiWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeavePengWidget, TEXT("weavePengWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeaveGangWidget, TEXT("weaveGangWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cWeaveBuWidget, TEXT("weaveBuWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cTingWidget, TEXT("tingWidget")))
        {
            return false;
        }
        if (!UMyCommonUtilsLibrary::isSubClassValidAndChild<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>(m_cHuWidget, TEXT("huWidget")))
        {
            return false;
        }
        return true;
    };


    inline TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> getWeaveWidgetByType(MyMJGameEventVisualTypeCpp type) const
    {
        if (type == MyMJGameEventVisualTypeCpp::WeaveChi) {
            return m_cWeaveChiWidget;
        }
        else if (type == MyMJGameEventVisualTypeCpp::WeavePeng) {
            return m_cWeavePengWidget;
        }
        else if (type == MyMJGameEventVisualTypeCpp::WeaveGang) {
            return m_cWeavePengWidget;
        }
        else if (type == MyMJGameEventVisualTypeCpp::WeaveBu) {
            return m_cWeavePengWidget;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave chi Widget"))
    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeaveChiWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave peng Widget"))
    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeavePengWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave gang Widget"))
    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeaveGangWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "weave bu Widget"))
    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cWeaveBuWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "ting Widget"))
    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cTingWidget;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "hu Widget"))
    TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp> m_cHuWidget;
};

USTRUCT()
struct FMyInRoomViewRoleIncidentStyleSettingsCpp
{
    GENERATED_USTRUCT_BODY()

public:

    inline bool checkSettings() const
    {
        return true;
    };

    FMyInRoomViewRoleIncidentStyleSettingsCpp()
    {

    };
};

USTRUCT()
struct FMyInRoomViewRoleStyleSettingsCpp
{
    GENERATED_USTRUCT_BODY()
public:

    FMyInRoomViewRoleStyleSettingsCpp()
    {

    };

    inline bool checkSettings() const
    {
        if (!m_cEvent.checkSettings()) {
            return false;
        }

        if (!m_cIncident.checkSettings()) {
            return false;
        }

        return true;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "event"))
    FMyInRoomViewRoleEventStyleSettingsCpp m_cEvent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "incident"))
    FMyInRoomViewRoleIncidentStyleSettingsCpp m_cIncident;
};

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
    };

    //< 0 means unknown
    int32 m_idxAttenderForIdxPositionInBox0;
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
    };


    bool m_bValid;

    FMyModelInfoWidget2DCpp m_cModelInfo; //contains size info

    UPROPERTY()
    class UCanvasPanel* m_pDynamicAllocationCanvasPanel;

    FMyMJGameInRoomUIMainWidgetCfgCpp m_cCfg;

    //size is always 4, idx is idx attender
    UPROPERTY()
    TArray<UMyMJGameInRoomPlayerInfoWidgetBaseCpp* > m_aPlayerInfoWidgets;
};

UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomUIMainWidgetBaseCpp : public UUserWidget, public IMyContentSizeWidget2DInterfaceCpp, public IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp, public IMyMJGameInRoomUIMainWidgetInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomUIMainWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : UUserWidget(ObjectInitializer)
    {

    };

    virtual ~UMyMJGameInRoomUIMainWidgetBaseCpp()
    {

    };


    virtual FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVisualType) override;
    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPublicChanged(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic, int32 subType) override;
    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPrivateChanged(int32 idxAttender, const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate, int32 subType) override;
    virtual FMyErrorCodeMJGameCpp updateAttenderPositions(float XYRatioOfplayerScreen, const TArray<FVector2D>& projectedPointsInPlayerScreen_unit_absolute) override;


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

        return ret;
    };



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
