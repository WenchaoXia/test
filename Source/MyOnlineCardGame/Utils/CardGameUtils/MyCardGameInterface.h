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
    int32 restartMainAnimation(int32 idxScreenPosition, float time, FVector2D offsetShowPoint, FVector2D offsetScreenCenter);
};