// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyMJGameVisualInterfaces.h"

#include "MyMJGameRoomUI.generated.h"

UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameInRoomUIMainWidgetBaseCpp : public UUserWidget, public IMyMJGameInRoomUIMainInterface
{
    GENERATED_BODY()

public:
    UMyMJGameInRoomUIMainWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : UUserWidget(ObjectInitializer)
    {

    };

    virtual ~UMyMJGameInRoomUIMainWidgetBaseCpp()
    {

    };
};
