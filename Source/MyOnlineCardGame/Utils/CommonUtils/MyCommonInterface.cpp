// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCommonInterface.h"
#include "MyCommonUtilsLibrary.h"

UMyTransformUpdaterInterfaceCpp::UMyTransformUpdaterInterfaceCpp(const FObjectInitializer& ObjectInitializer)
{

};


int32 IMyTransformUpdaterInterfaceCpp::getModelInfo(FMyActorModelInfoBoxCpp& modelInfo, bool verify) const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfo() must be implemented!"));
    MY_VERIFY(false);
    return -1;
};

FMyWithCurveUpdaterTransformCpp& IMyTransformUpdaterInterfaceCpp::getMyWithCurveUpdaterTransformRef()
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyWithCurveUpdaterTransformRef() must be implemented!"));
    MY_VERIFY(false);
    return *((FMyWithCurveUpdaterTransformCpp *)NULL);
};


UMyIdInterfaceCpp::UMyIdInterfaceCpp(const FObjectInitializer& ObjectInitializer)
{

};

int32 IMyIdInterfaceCpp::getMyId() const
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getMyId() not overrided by subclass."));
    return -1;
};

void IMyIdInterfaceCpp::setMyId(int32 myId)
{
    return;
};