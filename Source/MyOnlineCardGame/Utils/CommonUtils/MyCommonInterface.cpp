// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCommonInterface.h"
#include "MyCommonUtilsLibrary.h"



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


MyErrorCodeCommonPartCpp IMyValueInterfaceCpp::updateValueShowing(int32 newValueShowing, int32 animationTimeMs)
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("updateValueShowing() must be override by subclass."));
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};

MyErrorCodeCommonPartCpp IMyValueInterfaceCpp::getValueShowing(int32& valueShowing) const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getValueShowing() must be override by subclass."));
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};


MyErrorCodeCommonPartCpp IMyResourceInterfaceCpp::setResourcePath(const FDirectoryPath& newResPath)
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("setResourcePath() must be override by subclass."));
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};

MyErrorCodeCommonPartCpp IMyResourceInterfaceCpp::getResourcePath(FDirectoryPath& resPath) const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getResourcePath() must be override by subclass."));
    resPath.Path.Reset();
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass;
};