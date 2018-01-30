// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MJBPEncap/Utils/MyMJBPUtils.h"

#include "MyMJGameRoomViewerPawnBase.generated.h"


//Our player controller will help do replication work, and only replication help code goes here
UCLASS()
class MYONLINECARDGAME_API AMyMJGameRoomViewerPawnBaseCpp : public APawn
{
	GENERATED_BODY()

public:
    AMyMJGameRoomViewerPawnBaseCpp();
    virtual ~AMyMJGameRoomViewerPawnBaseCpp();


protected:

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
};
