// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.generated.h"

UCLASS(BlueprintType)
class AMyMJGameRoomRootActorCpp : public AActor
{
    GENERATED_BODY()

public:
    AMyMJGameRoomRootActorCpp() : Super()
    {
        m_pRoomActor = NULL;
        m_pCoreDataSource = NULL;
        m_pTrivalDataSource = NULL;
    };

    virtual ~AMyMJGameRoomRootActorCpp()
    {

    };

    bool checkSettings() const;

    inline AMyMJGameCoreDataSourceCpp* getCoreDataSource(bool verifyValid)
    {
        if (verifyValid) {
            MY_VERIFY(IsValid(m_pCoreDataSource));
        }

        return m_pCoreDataSource;
    };

protected:

    friend class AMyMJGamePlayerControllerCommunicationCpp;
    friend class AMyMJGameRoomLevelScriptActorCpp;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "room actor"))
        AMyMJGameRoomCpp* m_pRoomActor;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "core data source"))
        AMyMJGameCoreDataSourceCpp *m_pCoreDataSource;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "trival data source"))
        AMyMJGameTrivalDataSourceCpp *m_pTrivalDataSource;
};

/**
 * 
 */
UCLASS()
class MYONLINECARDGAME_API AMyMJGameRoomLevelScriptActorCpp : public ALevelScriptActor
{
	GENERATED_BODY()
	
	
public:
    AMyMJGameRoomLevelScriptActorCpp() : Super()
    {
        //m_pRoomSettingsObj = CreateDefaultSubobject<UMyMJGameRoomSettingsObjCpp>(TEXT("room settings obj"));
    };

    virtual ~AMyMJGameRoomLevelScriptActorCpp()
    {

    };

    bool checkSettings() const;

    UFUNCTION(BlueprintCallable, Category = "AMyMJGameRoomLevelScriptActorCpp", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static AMyMJGameRoomLevelScriptActorCpp* helperGetLevel(const UObject* WorldContextObject, bool verifyValid = true);

    UFUNCTION(BlueprintPure, Category = "AMyMJGameRoomLevelScriptActorCpp", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static AMyMJGameRoomRootActorCpp* helperGetRoomRootActor(const UObject* WorldContextObject, bool verifyValid = true);

    UFUNCTION(BlueprintPure, Category = "AMyMJGameRoomLevelScriptActorCpp", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static AMyMJGameRoomCpp* helperGetRoomActor(const UObject* WorldContextObject, bool verifyValid);

    static FMyMJGameTrivalDataCpp* helperGetMJGameTrivalData(const UObject* WorldContextObject, bool verifyValid);

    //slower but can be used for any uobject in the world
    static const UMyMJGameInRoomVisualCfgCpp* helperGetVisualCfg(const UObject* WorldContextObject, bool verifyValid = true);

protected:

    friend class AMyMJGameRoomRootActorCpp;

    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "room root actor"))
    AMyMJGameRoomRootActorCpp* m_pRoomRootActor;
};
