// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.generated.h"

/**
 * 
 */
UCLASS()
class MYONLINECARDGAME_API AMyMJGameRoomLevelScriptActorCpp : public ALevelScriptActor
{
	GENERATED_BODY()
	
	
public:
    AMyMJGameRoomLevelScriptActorCpp()
    {
        m_pRoomActor = NULL;
        m_pCoreDataSource = NULL;
        m_pTrivalDataSource = NULL;
    };

    virtual ~AMyMJGameRoomLevelScriptActorCpp()
    {

    };

    bool checkSettings() const;

protected:

    friend class AMyMJGameRoomViewerPawnCpp;

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        checkSettings();
    };

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "room actor"))
    AMyMJGameRoomCpp* m_pRoomActor;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "core data source"))
    AMyMJGameCoreDataSourceCpp *m_pCoreDataSource;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "trival data source"))
    AMyMJGameTrivalDataSourceCpp *m_pTrivalDataSource;
};
