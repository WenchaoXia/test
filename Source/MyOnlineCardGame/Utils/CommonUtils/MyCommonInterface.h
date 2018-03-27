// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyCommonInterface.generated.h"

UINTERFACE()
class UMyTransformUpdaterInterfaceCpp : public UInterface
{
    //GENERATED_BODY()
    GENERATED_UINTERFACE_BODY()
};

class IMyTransformUpdaterInterfaceCpp
{
    //GENERATED_BODY()
    GENERATED_IINTERFACE_BODY()

public:

    //return error code, 0 means OK
    //UFUNCTION(meta = (CallableWithoutWorldContext))
    virtual int32 getModelInfo(struct FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const;

    //UE4's reflection is not play well with template, so don't use UFunction
    //UFUNCTION(meta = (CallableWithoutWorldContext))
    virtual struct FMyWithCurveUpdaterTransformCpp& getMyWithCurveUpdaterTransformRef();

protected:

};


//application level id
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyIdInterfaceCpp : public UInterface
{
    //GENERATED_BODY()
    GENERATED_UINTERFACE_BODY()
};

class IMyIdInterfaceCpp
{
    //GENERATED_BODY()
    GENERATED_IINTERFACE_BODY()

public:

    //if return < 0, it means incorrect set or not used, check it carefully!
    UFUNCTION(BlueprintCallable)
    virtual int32 getMyId() const;

    virtual void setMyId(int32 myId);

protected:

};