// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MJ/MyMJGameAttender.h"

#include "MyMJGameAttenderBP.generated.h"

typedef class UMyMJIONodeCpp UMyMJIONodeCpp;

UCLASS()
class MYONLINECARDGAME_API AMyMJAttenderPawnBPCpp : public APawn
{
    GENERATED_BODY()

public:

    AMyMJAttenderPawnBPCpp() : Super()
    {
        //m_pIONode = NULL;
    };

    /*
    void setup(int32 idx, UMyMJIONodeCpp* pIONode)
    {
        m_pIONode = pIONode;
    };
    */

    //virtual void PostInitProperties() override;
    virtual void PostInitializeComponents() override;

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "IO Node"))
    //UMyMJIONodeCpp* m_pIONode;
};