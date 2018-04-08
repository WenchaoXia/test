// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonDefines.h"

#include "MyCommonInterface.generated.h"

//If a interface function = NULL, it means C++ child class must implement it.
//If a interface function have default return error code, it means child class can skip implemention and call it ignoring error.
//Warning:: Any function that maybe implemented by blueprint, can't be directly called, but using I*interface::Execute_*()

UINTERFACE()
class UMyWithCurveUpdaterTransformInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyWithCurveUpdaterTransformInterfaceCpp
{
    GENERATED_BODY()

public:

    virtual MyErrorCodeCommonPartCpp getModelInfo(struct FMyModelInfoCpp& modelInfo, bool verify = true) const = NULL;

    //Never fail, core dump otherwise
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterBasicCpp*& outUpdater) = NULL;

    //Never fail
    struct FMyWithCurveUpdaterBasicCpp& getMyWithCurveUpdaterTransformRef();

protected:

};


//application level id
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyIdInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyIdInterfaceCpp
{
    GENERATED_BODY()

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


UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyValueInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyValueInterfaceCpp
{
    GENERATED_BODY()

public:

    //@animationTimeMs if < 0, value should be updated at instance instead of animation, otherwise value will be updated by animation if possible
    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp updateValueShowing(int32 newValueShowing, int32 animationTimeMs);

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getValueShowing(int32& valueShowing) const;
};

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyResourceInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyResourceInterfaceCpp
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp setResourcePath(const FDirectoryPath& newResPath);

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getResourcePath(FDirectoryPath& resPath) const;
};


UINTERFACE()
class UMyPawnInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyPawnInterfaceCpp
{
    GENERATED_BODY()

public:

    virtual MyErrorCodeCommonPartCpp OnPossessedByLocalPlayerController(APlayerController* newController) = NULL;
    virtual MyErrorCodeCommonPartCpp OnUnPossessedByLocalPlayerController(APlayerController* oldController) = NULL;

protected:

};