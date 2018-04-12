// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CardGameUtils/MyCardGameElems.h"

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

    IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject();

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
struct FMyPlayerInfoWidgetRuntimeMetaCpp
{
    GENERATED_USTRUCT_BODY()

    FMyPlayerInfoWidgetRuntimeMetaCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_iIdxAttender = -1;
        m_iIdxScreenPosition = 0;

        m_cPosiSelf = FVector2D::ZeroVector;
        m_cPosiUICenter = FVector2D::ZeroVector;

    };

    inline FString ToString() const
    {
        return FString::Printf(TEXT("(idxAttender %d, m_iIdxScreenPosition %d, posiSelf %s, posiUICenter %s, m_cPosiCommonActionShowPoint %s)"), m_iIdxAttender, m_iIdxScreenPosition, *m_cPosiSelf.ToString(), *m_cPosiUICenter.ToString(), *m_cPosiCommonActionShowPoint.ToString());
    };

    int32 m_iIdxAttender;
    int32 m_iIdxScreenPosition; //not always continues, may be 0,2; 0,1,3;, 0,1,2,3

    FVector2D m_cPosiSelf;
    FVector2D m_cPosiUICenter;
    FVector2D m_cPosiCommonActionShowPoint;


};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomPlayerInfoWidgetBaseCpp : public UUserWidget, public IMySizeWidget2DInterfaceCpp, public IMyWidgetBasicOperationInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomPlayerInfoWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : UUserWidget(ObjectInitializer)
    {

    };

    virtual ~UMyMJGameInRoomPlayerInfoWidgetBaseCpp()
    {

    };

    inline FMyPlayerInfoWidgetRuntimeMetaCpp& getRuntimeMetaRef()
    {
        return m_cRuntimeMeta;
    };

    FMyErrorCodeMJGameCpp showAttenderWeave(float dur, MyMJGameEventVisualTypeCpp weaveVisualType);


protected:

    IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject()
    IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject()

    //if @createIfNotExist, always exist
    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp * getManagedWidget(int32 key, TSubclassOf<UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp>& widgetClass, bool createIfNotExist = true);

    FMyPlayerInfoWidgetRuntimeMetaCpp m_cRuntimeMeta;

    TMap<int32, UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp* > m_mManagedWidgets;
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
        m_idxDeskPositionOfCamera = 0;

        m_aPlayerInfoWidgetsOnScreen.Reset();
        m_iAttenderNumber = 0;

        m_cMainUILocalSize = FVector2D::ZeroVector;
    };

    inline int32 idxAttenderToIdxScreenPosition(int32 idxAttender)
    {
        int32 IdxDeskPosition = UMyCardGameUtilsLibrary::idxAttenderToIdxDeskPosition(idxAttender, m_iAttenderNumber);
        return UMyCardGameUtilsLibrary::idxDeskPositionToIdxScreenPosition(IdxDeskPosition, m_idxDeskPositionOfCamera);
    };

    inline int32 IdxScreenPositionToIdxAttender(int32 idxScreenPosition)
    {
        int32 IdxDeskPosition = UMyCardGameUtilsLibrary::IdxScreenPositionToIdxDeskPosition(idxScreenPosition, m_idxDeskPositionOfCamera);
        return UMyCardGameUtilsLibrary::idxDeskPositionToIdxAttender(IdxDeskPosition, m_iAttenderNumber);
    };


    bool m_bValid;
    int32 m_idxDeskPositionOfCamera;

    //size is always 4, idx is screen position
    UPROPERTY()
    TArray<UMyMJGameInRoomPlayerInfoWidgetBaseCpp* > m_aPlayerInfoWidgetsOnScreen;

    int32 m_iAttenderNumber;

    FVector2D m_cMainUILocalSize;
};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomUIMainWidgetBaseCpp : public UUserWidget, public IMyMJGameInRoomUIMainWidgetInterfaceCpp, public IMySizeWidget2DInterfaceCpp, public IMyWidgetBasicOperationInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomUIMainWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : UUserWidget(ObjectInitializer)
    {

    };

    virtual ~UMyMJGameInRoomUIMainWidgetBaseCpp()
    {

    };

    
    virtual FMyErrorCodeMJGameCpp changeDeskPositionOfIdxScreenPosition0(int32 idxDeskPositionOfIdxScreenPosition0) override;


    inline void invalidCachedData()
    {
        m_cCachedData.m_bValid = false;
    };

    inline UMyMJGameInRoomPlayerInfoWidgetBaseCpp* getInRoomPlayerInfoWidgetByScreenPosition(int32 idxScreenPosition, bool verifyValid = true)
    {
        if (!m_cCachedData.m_bValid) {
            refillCachedData();
        }

        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pRet = m_cCachedData.m_aPlayerInfoWidgetsOnScreen[idxScreenPosition];
        if (verifyValid) {
            MY_VERIFY(IsValid(pRet));
        }
        return pRet;
    };

    inline UMyMJGameInRoomPlayerInfoWidgetBaseCpp* getInRoomPlayerInfoWidgetByIdxAttender(int32 idxAttender, bool verifyValid = true)
    {
        if (!m_cCachedData.m_bValid) {
            refillCachedData();
        }

        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* pRet = m_cCachedData.m_aPlayerInfoWidgetsOnScreen[m_cCachedData.idxAttenderToIdxScreenPosition(idxAttender)];
        if (verifyValid) {
            MY_VERIFY(IsValid(pRet));
        }
        return pRet;
    }; 


protected:

    IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject();
    IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject()

    virtual void OnWidgetRebuilt() override
    {
        Super::OnWidgetRebuilt();

        invalidCachedData();
    };


    virtual FMyErrorCodeMJGameCpp showImportantGameStateUpdated_Implementation(float dur, MyMJGameStateCpp newGameState) override
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: showImportantGameStateUpdated only implemented in C++."), *GetClass()->GetName());
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint);
    };

    virtual FMyErrorCodeMJGameCpp showAttenderWeave_Implementation(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVisualType) override;

    virtual FMyErrorCodeMJGameCpp getInRoomPlayerInfoWidgetByScreenPositionEnsured_Implementation(int32 idxScreenPosition, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget) override
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getInRoomPlayerInfoWidgetByScreenPositionEnsured only implemented in C++."), *GetClass()->GetName());
        MY_VERIFY(false);
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint);
    };

    inline UMyMJGameInRoomPlayerInfoWidgetBaseCpp* getInRoomPlayerInfoWidgetByScreenPositionEnsured(int32 i)
    {
        UMyMJGameInRoomPlayerInfoWidgetBaseCpp* ret = NULL;
        IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_getInRoomPlayerInfoWidgetByScreenPositionEnsured(this, i, ret);

        MY_VERIFY(ret);
        return ret;
    };



    //if @idxDeskPositionOfCamera < 0, it will use old settings, such as resetted value as 0
    UFUNCTION(BlueprintCallable)
    void refillCachedData(int32 idxDeskPositionOfCamera = -1);

    FMyMJGameInRoomUIMainWidgetCachedDataCpp m_cCachedData;
};
