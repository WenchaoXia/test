// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonDefines.h"

#include "MyCommonInterface.generated.h"

//If a interface function = NULL, it means C++ child class must implement it.
//If a interface function have default return error code, it means child class can skip implemention and call it ignoring error.
//Warning:: Any function that maybe implemented by blueprint, can't be directly called, but using I*interface::Execute_*().
//If error happens, Error log should always be generated in implemention, and any pointer as output should be set to NULL, ref as output should be resetted.


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

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getValueShowing(int32& valueShowing) const;

    //@animationTimeMs if < 0, value should be updated at instance instead of animation, otherwise value will be updated by animation if possible
    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp updateValueShowing(int32 newValueShowing, int32 animationTimeMs);


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
        virtual MyErrorCodeCommonPartCpp getResourcePath(FDirectoryPath& resPath) const;

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp setResourcePath(const FDirectoryPath& newResPath);


};



UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
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
};



UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMySelectableObjectInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMySelectableObjectInterfaceCpp
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp setSelected(bool selected);

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getSelected(bool &selected) const;

    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp setIsSelectable(bool selectable);

    //note if setISSelectable() called before, it is possible got different result from getIsSelectable(), since other condition may be added
    UFUNCTION(BlueprintCallable)
        virtual MyErrorCodeCommonPartCpp getIsSelectable(bool &selectable) const;

protected:

};


//in most cases, one draggable object must be selectable, and it must implement IMySelectableObjectInterfaceCpp
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint = "true"))
class UMyDraggableObjectInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyDraggableObjectInterfaceCpp
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp markBeginDrag();

    UFUNCTION(BlueprintCallable)
    virtual MyErrorCodeCommonPartCpp getDataBeginDrag(FTransform& transform, bool& projectionOK, FVector2D& projectedScreenPosition, float& projectedDistance) const;
};

