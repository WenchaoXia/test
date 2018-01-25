// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyMJGameVisualElems.h"
#include "Utils/CommonUtils/MyRenderUtilsLibrary.h"

#include "MyMJGameVisualTools.generated.h"

UCLASS(Blueprintable, HideCategories = (Collision, Rendering))
class MYONLINECARDGAME_API AMyMJGameTextureGenSuitBaseCpp : public AMyTextureGenSuitBaseCpp
{
    GENERATED_BODY()

public:

    AMyMJGameTextureGenSuitBaseCpp() : Super()
    {
        m_cOutputPath.Path.Reset();
    };

    virtual ~AMyMJGameTextureGenSuitBaseCpp()
    {

    };

    //return error code
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    int32 genTextureForMJCardValues(FString namePrefix, bool haveWord = false, bool haveHua = false);

    //where the generated texture should be stored, caller's responsibility to clean the dir before work
    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "output path", ContentDir = "true"))
    FDirectoryPath m_cOutputPath;
};