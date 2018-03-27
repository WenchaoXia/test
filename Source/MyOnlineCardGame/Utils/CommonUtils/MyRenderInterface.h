// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyRenderInterface.generated.h"


//BlueprintNativeEvent is better than BlueprintImplementableEvent since it gives C++ code a nice way to report error that
//BP not implemented it

UINTERFACE()
class UMyWidgetBasicOperationInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

//by default, the API's card actors param passed in, are attender related, except those have tipped not only attender's 
class IMyWidgetBasicOperationInterfaceCpp
{
    GENERATED_BODY()

public:

    //Must be implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    int32 canvasAddChild(UWidget *childWidget);

    //The child's center point should be set on parent's @centerPosiInParentPecent(from 0 to 1), the rect of parent is defined by parent itself and can be different
    //Must be implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    int32 canvasSetChildPosi(UWidget *childWidget, FVector2D centerPosiInParentPecent);

    //return the widget size at DESIGN time, like 1920 * 1080
    //Must be implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    int32 getLocalSize(FVector2D& localSize) const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    int32 setLocalSize(FVector2D& localSize, bool keepRatioByWidth, bool keepRatioByHeight);
};

#define IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject() \
int32 canvasAddChild_Implementation(UWidget *childWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvasAddChild only implemented in C++."), *GetClass()->GetName()); \
    return 0; \
}; \
int32 canvasSetChildPosi_Implementation(UWidget *childWidget, FVector2D centerPosiInParentPecent) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvasSetChildPosi only implemented in C++."), *GetClass()->GetName()); \
    return 0; \
}; \
int32 getLocalSize_Implementation(FVector2D &localSize) const override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getLocalSize only implemented in C++."), *GetClass()->GetName()); \
    return 0; \
}; \
int32 setLocalSize_Implementation(FVector2D& localSize, bool keepRatioByWidth, bool keepRatioByHeight) \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getLocalSize only implemented in C++."), *GetClass()->GetName()); \
    return 0; \
};