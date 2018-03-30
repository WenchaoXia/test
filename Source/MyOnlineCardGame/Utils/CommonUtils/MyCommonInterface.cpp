// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCommonInterface.h"
#include "MyCommonUtilsLibrary.h"

UMyTransformUpdaterInterfaceCpp::UMyTransformUpdaterInterfaceCpp(const FObjectInitializer& ObjectInitializer)
{

};


MyErrorCodeCommonPartCpp IMyTransformUpdaterInterfaceCpp::getModelInfo(struct FMyActorModelInfoBoxCpp& modelInfo, bool verify) const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfo() must be implemented!"));
    MY_VERIFY(false);
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};

MyErrorCodeCommonPartCpp IMyTransformUpdaterInterfaceCpp::getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterTransformCpp*& outUpdater)
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyWithCurveUpdaterTransform() must be implemented!"));
    MY_VERIFY(false);
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};


UMyIdInterfaceCpp::UMyIdInterfaceCpp(const FObjectInitializer& ObjectInitializer)
{

};

MyErrorCodeCommonPartCpp IMyIdInterfaceCpp::getMyId(int32& outMyId) const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyId() not overrided by subclass."));
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};

MyErrorCodeCommonPartCpp IMyIdInterfaceCpp::setMyId(int32 myId)
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("setMyId() not overrided by subclass."));
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};


UMyPawnInterfaceCpp::UMyPawnInterfaceCpp(const FObjectInitializer& ObjectInitializer)
{

};