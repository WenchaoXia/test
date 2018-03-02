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

protected:

    int32 showImportantGameStateUpdated_Implementation(float dur, MyMJGameStateCpp newGameState)
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("showImportantGameStateUpdated_Implementation only implemented in C++."));
        return 0;
    };

    virtual int32 showAttenderWeave_Implementation(float dur, int32 idxAttender, MyMJGameWeaveVisualTypeCpp weaveVsualType) override
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("showAttenderWeave_Implementation only implemented in C++."));
        return 0;
    };

    virtual int32 changeViewPosition_Implementation(int32 idxAttender) override
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("changeViewPosition_Implementation only implemented in C++."));
        return 0;
    };

    MyMJGameRuleTypeCpp m_eRuleType;
};
