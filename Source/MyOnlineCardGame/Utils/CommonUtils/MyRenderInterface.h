// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtilsLibrary.h"

#include "Runtime/UMG/Public/Components/CanvasPanel.h"

#include "MyRenderInterface.generated.h"

//BlueprintNativeEvent is better than BlueprintImplementableEvent since it gives C++ code a nice way to report error when not implemented by BP.
//Any "inner" function implemention should NOT call any "cached" function to avoid dead lock.
//Any implemention SHOULD generate log if it meets error.



UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyWithCurveUpdaterTransformWorld3DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyWithCurveUpdaterTransformWorld3DInterfaceCpp
{
    GENERATED_BODY()

public:

    //May fail, supposed to be fast, may come from cache and refresh it, not consider actor scale now
    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getModelInfoForUpdater(FMyModelInfoWorld3DCpp& modelInfo)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfoForUpdater() not implemented"));
        MY_VERIFY(false);
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    }

    //Never fail
    virtual struct FMyWithCurveUpdaterTransformWorld3DCpp& getMyWithCurveUpdaterTransformRef()
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyWithCurveUpdaterTransformRef() not implemented"));
        MY_VERIFY(false);
        return *(struct FMyWithCurveUpdaterTransformWorld3DCpp *)(NULL);
    };

    //Never fail
    inline FMyModelInfoWorld3DCpp getModelInfoForUpdaterEnsured()
    {
        FMyModelInfoWorld3DCpp ret;
        MY_VERIFY(getModelInfoForUpdater(ret) == MyErrorCodeCommonPartCpp::NoError);
        return ret;
    };


protected:

};


UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyWithCurveUpdaterTransformWidget2DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyWithCurveUpdaterTransformWidget2DInterfaceCpp
{
    GENERATED_BODY()

public:

    //May fail, supposed to be fast, may come from cache and refresh it
    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getModelInfoForUpdater(FMyModelInfoWidget2DCpp& modelInfo)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfoForUpdater() not implemented"));
        MY_VERIFY(false);
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    }

    //Never fail
    virtual struct FMyWithCurveUpdaterTransformWidget2DCpp& getMyWithCurveUpdaterTransformRef()
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyWithCurveUpdaterTransformRef() not implemented"));
        MY_VERIFY(false);
        return *(struct FMyWithCurveUpdaterTransformWidget2DCpp *)(NULL);
    };

    //Never fail
    inline FMyModelInfoWidget2DCpp getModelInfoForUpdaterEnsured()
    {
        FMyModelInfoWidget2DCpp ret;
        MY_VERIFY(getModelInfoForUpdater(ret) == MyErrorCodeCommonPartCpp::NoError);
        return ret;
    };


protected:

};

/*
//widget 2D model info is different with actor 3D, it is acutally decided by layout, che child class itself just need to fill in the model size, tell what modelinfo it wants to have, and parent canvas instance make it comply
//unit is pixel at DESIGN time, like 1920 x 1080
UINTERFACE()
class UMyModelInfoWidget2DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyModelInfoWidget2DInterfaceCpp
{
    GENERATED_BODY()


protected:

    struct FMyModelInfoWidget2DInterfaceCachedDataCpp
    {

    public:

        FMyModelInfoWidget2DInterfaceCachedDataCpp()
        {
            reset();
        };

        virtual ~FMyModelInfoWidget2DInterfaceCachedDataCpp()
        {

        };

        inline void reset()
        {
            m_bValid = false;
            m_cModelInfo.reset();
        };


        bool m_bValid;
        FMyModelInfoWidget2DCpp m_cModelInfo;
    };

public:

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getContentSize(FVector2D& contentSize);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp setContentSize(const FVector2D& contentSize);

    //Optional implementable by Blueprint
    //Will try get from cache, or direct from BP if not implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getModelInfoFromCache(FMyModelInfoWidget2DCpp& modelInfo, bool verifyValid);

    //Optional implementable by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setModelInfoByExtendXWithRatioLocked(float extendX);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setModelInfo(const FMyModelInfoWidget2DCpp& modelInfo);

protected:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoWidget2DCpp& modelInfo) const;

};

#define IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject_Cpp() \
protected:

#define IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached() \
protected: \
virtual MyErrorCodeCommonPartCpp getModelInfoFromCache_Implementation(FMyModelInfoWidget2DCpp& modelInfo, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = IMyModelInfoWidget2DInterfaceCpp::Execute_getModelInfo(this, modelInfo); \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getModelInfo return error: %d."), *GetClass()->GetName(), (uint8)ret); \
        modelInfo.reset(); \
        if (verifyValid) { \
            MY_VERIFY(false); \
        } \
    } \
    return ret; \
};

#define IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other() \
protected: \
virtual MyErrorCodeCommonPartCpp setModelInfoByExtendXWithRatioLocked_Implementation(float extendX) override \
{ \
    FMyModelInfoWidget2DCpp modelInfo; \
    MyErrorCodeCommonPartCpp ret = IMyModelInfoWidget2DInterfaceCpp::Execute_getModelInfoFromCache(this, modelInfo, false); \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getModelInfoFromCache return error: %d."), *GetClass()->GetName(), (uint8)ret); \
        return ret; \
    } \
    \
    float scale = extendX / modelInfo.getBox2DRefConst().m_cBoxExtend.X; \
    modelInfo = modelInfo * scale; \
    \
    return IMyModelInfoWidget2DInterfaceCpp::Execute_setModelInfo(this, modelInfo); \
}; \
virtual MyErrorCodeCommonPartCpp setModelInfo_Implementation(const FMyModelInfoWidget2DCpp& modelInfo) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setModelInfo only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getModelInfo_Implementation(FMyModelInfoWidget2DCpp& modelInfo) const override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getModelInfo only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};

#define IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject() \
IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject_Cpp() \
IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached() \
IMyModelInfoWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other()

//##classMemberName.m_bValid must exist, MyErrorCodeCommonPartCpp refillCachedData() must exist
#define IMyModelInfoWidget2DInterfaceCpp_getModelInfoFromCache_Style0(classMemberName, subMemberName) \
protected: \
virtual MyErrorCodeCommonPartCpp getModelInfoFromCache_Implementation(FMyModelInfoWidget2DCpp& modelInfo, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError; \
    if (!##classMemberName.m_bValid) { \
        ret = refillCachedData(); \
    } \
    \
    modelInfo = ##classMemberName.##subMemberName; \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getModelInfoFromCache() return error: %d."), *GetClass()->GetName(), (uint8)ret); \
        if (verifyValid) { \
            MY_VERIFY(false); \
        } \
    } \
    return ret; \
};
*/



//widget 2D model info is different with actor 3D, it is acutally decided by layout, che child class itself just need to fill in the model size, tell what modelinfo it wants to have, and parent canvas instance make it comply
//we can get the content size and gen a compatiable model info 2D and tell widget to layout according to the info
//unit is pixel at DESIGN time, like 1920 x 1080
UINTERFACE()
class UMyContentSizeWidget2DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyContentSizeWidget2DInterfaceCpp
{
    GENERATED_BODY()


public:


    //Optional implementable by Blueprint, recommend NOT
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getContentSizeFromCache(FVector2D& contentSize, bool verifyValid);

    //Optional implementable by Blueprint, recommend NOT
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp setContentSizeThroughCache(const FVector2D& contentSize, bool verifyValid);

protected:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getContentSize(FVector2D& contentSize) const;

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setContentSize(const FVector2D& contentSize);
};


#define IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other() \
protected: \
virtual MyErrorCodeCommonPartCpp getContentSize_Implementation(FVector2D& contentSize) const override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getContentSize only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp setContentSize_Implementation(const FVector2D& contentSize) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setContentSize only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};

#define IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_getContentSizeFromCache_Style0(classMemberName, subMemberName, refillCachedDataFuncName) \
protected: \
virtual MyErrorCodeCommonPartCpp getContentSizeFromCache_Implementation(FVector2D& contentSize, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError; \
    if (!##classMemberName.m_bValid) { \
        ret = ##refillCachedDataFuncName(); \
    } \
    contentSize = ##classMemberName.##subMemberName; \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getContentSizeFromCache() returning error: %s."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret)); \
        if (verifyValid) { \
            MY_VERIFY(false); \
        } \
    } \
    return ret; \
};

#define IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_setContentSizeThroughCache_Style0(updateAfterSettingFuncName) \
protected: \
MyErrorCodeCommonPartCpp setContentSizeThroughCache_Implementation(const FVector2D& contentSize, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = IMyContentSizeWidget2DInterfaceCpp::Execute_setContentSize(this, contentSize); \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setContentSizeThroughCache() returning error: %s."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret)); \
        if (verifyValid) { \
            MY_VERIFY(false); \
        } \
    } \
    \
    MyErrorCodeCommonPartJoin(ret, ##updateAfterSettingFuncName()); \
    \
    return ret; \
};

/*
//##classMemberName.m_bValid must exist, MyErrorCodeCommonPartCpp refillCachedData(), invalidCachedData() must exist
#define IMyContentSizeWidget2DInterfaceCpp_DefaultImplementationForUObject_Cached_Style0(classMemberName, subMemberName) \
protected: \
virtual MyErrorCodeCommonPartCpp getContentSizeFromCache_Implementation(FVector2D& contentSize, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError; \
    if (!##classMemberName.m_bValid) { \
        ret = refillCachedData(); \
    } \
    \
    contentSize = (##classMemberName.##subMemberName); \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getContentSizeFromCache() returning error: %s."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret)); \
        if (verifyValid) { \
            MY_VERIFY(false); \
        } \
    } \
    return ret; \
}; \
virtual MyErrorCodeCommonPartCpp setContentSizeThroughCache_Implementation(const FVector2D& contentSize, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = IMyContentSizeWidget2DInterfaceCpp::Execute_setContentSize(this, contentSize); \
    IMyContentSizeWidget2DInterfaceCpp::Execute_invalidCache_MyContentSizeWidget2DInterface(this); \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setContentSizeThroughCache() returning error: %s."), *GetClass()->GetName(), *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret)); \
        if (verifyValid) { \
            MY_VERIFY(false); \
        } \
    } \
    return ret; \
}; \
virtual void invalidCache_MyContentSizeWidget2DInterface_Implementation() override \
{ \
    invalidCachedData(); \
};
*/


//supposed to work with widget
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyCachedData_MyModelInfoWidget2D_InterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyCachedData_MyModelInfoWidget2D_InterfaceCpp
{
    GENERATED_BODY()

public:

    virtual MyErrorCodeCommonPartCpp getDataByCacheRefConst_MyModelInfoWidget2D(const FMyModelInfoWidget2DCpp*& pModelInfo, bool verifyValid) = 0;

    //supposed to be called after added to parent panel, now only support canvas as parent panel
    virtual MyErrorCodeCommonPartCpp updateSlotSettingsToComply_MyModelInfoWidget2D() = 0;


    //don't override
    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getDataByCache_MyModelInfoWidget2D(FMyModelInfoWidget2DCpp& modelInfo, bool verifyValid)
    {
        const FMyModelInfoWidget2DCpp* pI = NULL;
        MyErrorCodeCommonPartCpp ret = getDataByCacheRefConst_MyModelInfoWidget2D(pI, verifyValid);
        if (ret == MyErrorCodeCommonPartCpp::NoError) {
            MY_VERIFY(pI);
            modelInfo = *pI;
        }
        else {
            modelInfo.reset();
        }

        return ret;
    };

protected:

    struct FMyCachedData_MyModelInfoWidget2D_BaseCpp
    {

    public:

        FMyCachedData_MyModelInfoWidget2D_BaseCpp()
        {
            reset();
        };

        virtual ~FMyCachedData_MyModelInfoWidget2D_BaseCpp()
        {

        };

        inline void reset()
        {
            m_bValid = false;
            m_cModelInfo.reset();
        };


        bool m_bValid;
        FMyModelInfoWidget2DCpp m_cModelInfo;
    };


    virtual void invalidCachedData_MyModelInfoWidget2D() = 0;

    virtual MyErrorCodeCommonPartCpp refillCachedData_MyModelInfoWidget2D() = 0;

};


UINTERFACE()
class UMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp
{
    GENERATED_BODY()

public:

    //Optional implementable by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getDynamicAllocationRootCanvasPanelFromCache(UCanvasPanel*& canvasPanel, bool verifyValid);

protected:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getDynamicAllocationRootCanvasPanel(UCanvasPanel*& canvasPanel);
};

#define IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached() \
protected: \
virtual MyErrorCodeCommonPartCpp getDynamicAllocationRootCanvasPanelFromCache_Implementation(UCanvasPanel*& canvasPanel, bool verifyValid) override \
{ \
    canvasPanel = NULL; \
    MyErrorCodeCommonPartCpp ret = IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp::Execute_getDynamicAllocationRootCanvasPanel(this, canvasPanel); \
    if (ret != MyErrorCodeCommonPartCpp::NoError) { \
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getDynamicAllocationRootCanvasPanel return error: %d."), *GetClass()->GetName(), (uint8)ret); \
    } \
    else { \
        MY_VERIFY(canvasPanel != NULL); \
    } \
    if (verifyValid) { \
        MY_VERIFY(ret == MyErrorCodeCommonPartCpp::NoErro); \
    } \
    return ret; \
};

#define IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other() \
protected: \
virtual MyErrorCodeCommonPartCpp getDynamicAllocationRootCanvasPanel_Implementation(class UCanvasPanel*& canvasPanel) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getDynamicAllocationRootCanvasPanel only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};

#define IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject() \
        IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached() \
        IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Other()


//##classMemberName.m_bValid must exist, MyErrorCodeCommonPartCpp refillCachedData() must exist
#define IMyDynamicAllocationCanvasPannelWidget2DInterfaceCpp_DefaultImplementationForUObject_Bp_Cached_Style0(classMemberName, subMemberName) \
virtual MyErrorCodeCommonPartCpp getDynamicAllocationRootCanvasPanelFromCache_Implementation(class UCanvasPanel*& canvasPanel, bool verifyValid) override \
{ \
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError; \
    if (!##classMemberName.m_bValid) { \
        ret = refillCachedData(); \
    } \
    \
    canvasPanel = ##classMemberName.##subMemberName; \
    \
    if (ret == MyErrorCodeCommonPartCpp::NoError) { \
        MY_VERIFY(canvasPanel); \
    } \
    else { \
        MY_VERIFY(canvasPanel == NULL); \
    } \
    \
    if (verifyValid) { \
        MY_VERIFY(IsValid(canvasPanel)); \
    } \
    \
    return ret; \
};