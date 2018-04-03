// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCardGameUtilsLibrary.h"

#include "Blueprint/UserWidget.h"

#include "MyCardGameElems.generated.h"

//value >= 0 means valid
#define MyCardGameValueUnknown (-1)

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
class MYONLINECARDGAME_API UMyCardGameCardWidgetBaseCpp : public UUserWidget, public IMyWidgetBasicOperationInterfaceCpp, public IMyIdInterfaceCpp, public IMyCardGameValueRelatedObjectInterfaceCpp
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
    UPROPERTY(EditDefaultsOnly, BlueprintSetter = setResPath2, BlueprintGetter = getResPath2, meta = (DisplayName = "resource path", ContentDir = "true"))
        FDirectoryPath m_cResPath;

    int32 m_iMyId;

    UPROPERTY(BlueprintReadOnly)
        class UTexture *m_pCardMainTexture;

    FMyCardGameCardWidgetCachedDataCpp m_cCachedData;

};
