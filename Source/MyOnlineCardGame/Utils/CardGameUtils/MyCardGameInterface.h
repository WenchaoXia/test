// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CommonUtils/MyRenderUtilsLibrary.h"
#include "MyCardGameCommonDefines.h"

#include "MyCardGameInterface.generated.h"


UINTERFACE()
class UMyCardGameScreenPositionRelatedWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyCardGameScreenPositionRelatedWidgetInterfaceCpp
{
    GENERATED_BODY()

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