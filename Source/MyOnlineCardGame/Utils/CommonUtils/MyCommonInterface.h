// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonDefines.h"

#include "MyCommonInterface.generated.h"

//If a interface function = NULL, it means C++ child class must implement it.
//If a interface function have default return error code, it means child class can skip implemention and call it ignoring error.
//Warning:: Any function that maybe implemented by blueprint, can't be directly called, but using I*interface::Execute_*()

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

    virtual MyErrorCodeCommonPartCpp getModelInfo(struct FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const = NULL;

    //Never fail, core dump otherwise
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterTransformCpp*& outUpdater) = NULL;

    inline struct FMyWithCurveUpdaterTransformCpp& getMyWithCurveUpdaterTransformRef()
    {
        struct FMyWithCurveUpdaterTransformCpp* pRet = NULL;
        getMyWithCurveUpdaterTransformEnsured(pRet);
        return *pRet;
    };

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
    virtual MyErrorCodeCommonPartCpp getMyId(int32& outMyId) const;

    virtual MyErrorCodeCommonPartCpp setMyId(int32 myId);

    //since C++ compiler can't recognize this function if same name, so use different name
    //return -1 if not implemented
    inline int32 getMyId2() const
    {
        int32 ret = -1;
        getMyId(ret);
        return ret;
    };

protected:

};


UINTERFACE()
class UMyPawnInterfaceCpp : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IMyPawnInterfaceCpp
{
    GENERATED_IINTERFACE_BODY()

public:

    virtual MyErrorCodeCommonPartCpp OnPossessedByLocalPlayerController(APlayerController* newController) = NULL;
    virtual MyErrorCodeCommonPartCpp OnUnPossessedByLocalPlayerController(APlayerController* oldController) = NULL;

protected:

};