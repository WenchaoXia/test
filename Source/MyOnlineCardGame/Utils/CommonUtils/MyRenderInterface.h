// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtilsLibrary.h"

#include "MyRenderInterface.generated.h"


//BlueprintNativeEvent is better than BlueprintImplementableEvent since it gives C++ code a nice way to report error that
//BP not implemented it

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

    //return the widget size at DESIGN time, like 1920 * 1080
    //Must be implemented
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp getLocalSize(FVector2D& localSize) const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp setLocalSize(FVector2D& localSize, bool keepRatioByWidth, bool keepRatioByHeight);
};

#define IMyWidgetBasicOperationInterfaceCpp_DefaultEmptyImplementationForUObject() \
MyErrorCodeCommonPartCpp canvasAddChild_Implementation(UWidget *childWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvasAddChild only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
MyErrorCodeCommonPartCpp canvasSetChildPosi_Implementation(UWidget *childWidget, FVector2D centerPosiInParentPecent) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: canvasSetChildPosi only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
MyErrorCodeCommonPartCpp getLocalSize_Implementation(FVector2D &localSize) const override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getLocalSize only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
MyErrorCodeCommonPartCpp setLocalSize_Implementation(FVector2D& localSize, bool keepRatioByWidth, bool keepRatioByHeight) \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getLocalSize only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};