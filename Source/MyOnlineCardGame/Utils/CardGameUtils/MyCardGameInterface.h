// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CommonUtils/MyRenderUtilsLibrary.h"

#include "MyCardGameInterface.generated.h"


UINTERFACE()
class UMyCardGameScreenPositionRelatedWidgetInterfaceCpp : public UInterface
{
    //GENERATED_BODY()
    GENERATED_UINTERFACE_BODY()
};

class IMyCardGameScreenPositionRelatedWidgetInterfaceCpp
{
    //GENERATED_BODY()
    GENERATED_IINTERFACE_BODY()

public:

    //when calling this, offset = 0 always means the widget is at viewRole's point
    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp restartMainAnimation(int32 idxScreenPosition, float time, FVector2D offsetShowPoint, FVector2D offsetScreenCenter);
};

#define IMyCardGameScreenPositionRelatedWidgetInterfaceCpp_DefaultEmptyImplementationForUObject() \
MyErrorCodeCommonPartCpp restartMainAnimation_Implementation(int32 idxScreenPosition, float time, FVector2D offsetShowPoint, FVector2D offsetScreenCenter) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: restartMainAnimation only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};

//like MJ card actor and widget, they can be set with values
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyCardGameValueRelatedObjectInterfaceCpp : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IMyCardGameValueRelatedObjectInterfaceCpp
{
    GENERATED_IINTERFACE_BODY()

public:

    //@animationTimeMs if < 0, value should be updated at instance instead of animation, otherwise value will be updated by animation if possible
    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp updateValueShowing(int32 newValueShowing, int32 animationTimeMs)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("updateValueShowing() must be override by subclass."));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp getValueShowing(int32& valueShowing) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getValueShowing() must be override by subclass."));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp setResPath(const FDirectoryPath& newResPath)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("setResPath() must be override by subclass."));
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };

    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp getResPath(FDirectoryPath& resPath) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getResPath() must be override by subclass."));
        resPath.Path.Reset();
        return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
    };
};