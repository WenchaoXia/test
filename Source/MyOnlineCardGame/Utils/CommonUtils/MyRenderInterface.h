// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtilsLibrary.h"

#include "MyRenderInterface.generated.h"



UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyWithCurveUpdaterTransformWorld3DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyWithCurveUpdaterTransformWorld3DInterfaceCpp
{
    GENERATED_BODY()

public:

    //supposed to be fast, may come from cache
    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoWorld3DCpp& modelInfo, bool verify) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfo() not implemented"));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    //Never fail, core dump otherwise
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformWorld3DEnsured(struct FMyWithCurveUpdaterTransformWorld3DCpp*& outUpdater)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyWithCurveUpdaterTransformWorld3DEnsured() not implemented"));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    //Never fail
    inline struct FMyWithCurveUpdaterTransformWorld3DCpp& getMyWithCurveUpdaterTransformWorld3DRef()
    {
        struct FMyWithCurveUpdaterTransformWorld3DCpp* pRet = NULL;
        getMyWithCurveUpdaterTransformWorld3DEnsured(pRet);
        MY_VERIFY(pRet);
        return *pRet;
    }

    //Never fail
    inline FMyModelInfoWorld3DCpp getModelInfo() const
    {
        FMyModelInfoWorld3DCpp ret;
        MY_VERIFY(getModelInfo(ret, true) == MyErrorCodeCommonPartCpp::NoError);
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

    //supposed to be fast, may come from cache
    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoWidget2DCpp& modelInfo, bool verify) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfo() not implemented"));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    //Never fail, core dump otherwise
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformWidget2DEnsured(struct FMyWithCurveUpdaterTransformWidget2DCpp*& outUpdater)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyWithCurveUpdaterTransformWorld3DEnsured() not implemented"));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    //Never fail
    inline struct FMyWithCurveUpdaterTransformWidget2DCpp& getMyWithCurveUpdaterTransformWidget2DRef()
    {
        struct FMyWithCurveUpdaterTransformWidget2DCpp* pRet = NULL;
        getMyWithCurveUpdaterTransformWidget2DEnsured(pRet);
        MY_VERIFY(pRet);
        return *pRet;
    };

    //Never fail
    inline FMyModelInfoWidget2DCpp getModelInfo() const
    {
        FMyModelInfoWidget2DCpp ret;
        MY_VERIFY(getModelInfo(ret, true) == MyErrorCodeCommonPartCpp::NoError);
        return ret;
    };

protected:

};



//BlueprintNativeEvent is better than BlueprintImplementableEvent since it gives C++ code a nice way to report error that
//BP not implemented it
UINTERFACE()
class UMySizeWidget2DInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMySizeWidget2DInterfaceCpp
{
    GENERATED_BODY()


protected:

    struct FMySizeWidget2DInterfaceCachedDataCpp
    {

    public:

        FMySizeWidget2DInterfaceCachedDataCpp()
        {
            reset();
        };

        virtual ~FMySizeWidget2DInterfaceCachedDataCpp()
        {

        };

        inline void reset()
        {
            m_bValid = false;
            m_cLocalSize = FVector2D::ZeroVector;
        };


        bool m_bValid;
        FVector2D m_cLocalSize;
    };

public:

    //Will try get from cache, or direct from BP if not implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getLocalSizeFromCache(FVector2D& localSize);

    //if @keepRatioByWidth is true, it keeps ratio by width, and has higher priority than @keepRatioByHeight
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setLocalSize(const FVector2D& localSize, bool keepRatioByWidth, bool keepRatioByHeight);

protected:

    //return the widget size at DESIGN time, like 1920 * 1080
    //Must be implemented
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getLocalSize(FVector2D& localSize) const;

};

#define IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject_Cpp() \
public: \
virtual MyErrorCodeCommonPartCpp getLocalSizeFromCache_Implementation(FVector2D& localSize) override final \
{ \
    return IMySizeWidget2DInterfaceCpp::Execute_getLocalSize(this, localSize); \
}; \
protected:

#define IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject_Bp() \
public: \
virtual MyErrorCodeCommonPartCpp setLocalSize_Implementation(const FVector2D& localSize, bool keepRatioByWidth, bool keepRatioByHeight) override\
{ \
UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getLocalSize only implemented in C++."), *GetClass()->GetName()); \
return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected: \
virtual MyErrorCodeCommonPartCpp getLocalSize_Implementation(FVector2D &localSize) const override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getLocalSize only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected:

#define IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject() \
IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject_Cpp() \
IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject_Bp()


UINTERFACE()
class UMyWidgetBasicOperationInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyWidgetBasicOperationInterfaceCpp
{
    GENERATED_BODY()

public:

    //Must be implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp canvasAddChild(UWidget *childWidget);

    //The child's center point should be set on parent's @centerPosiInParentPecent(from 0 to 1), the rect of parent is defined by parent itself and can be different
    //Must be implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp canvasSetChildPosi(UWidget *childWidget, FVector2D centerPosiInParentPecent);

};

#define IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject() \
virtual MyErrorCodeCommonPartCpp canvasAddChild_Implementation(UWidget *childWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvasAddChild only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp canvasSetChildPosi_Implementation(UWidget *childWidget, FVector2D centerPosiInParentPecent) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvasSetChildPosi only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};