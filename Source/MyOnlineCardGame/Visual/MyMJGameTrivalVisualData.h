// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameRoomUI.h"

#include "MyMJGameTrivalVisualData.generated.h"

USTRUCT()
struct FMyMJGameTrivalDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameTrivalDataCpp()
    {
        m_aAttenderDatas.AddDefaulted(4);
    };

    UPROPERTY( meta = (DisplayName = "Game Cfg"))
    FMyMJGameCfgCpp m_cGameCfg;

    UPROPERTY( meta = (DisplayName = "Game RunData"))
    FMyMJGameRunDataCpp m_cGameRunData;

    //always 4
    UPROPERTY( meta = (DisplayName = "attender datas"))
    TArray<FMyMJGamePlayerDataCpp> m_aAttenderDatas;
};

/*
USTRUCT(BlueprintType)
struct FMyTestDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTestDataCpp()
    {

        
        for (int i = 0; i < RoleDataAttenderNum; i++) {
        int32 idx = m_aRoleDataAttenders.Emplace();
        FMyMJRoleDataAttenderCpp *pD = &m_aRoleDataAttenders[idx];
        pD->resetup(i);
        }
        

        for (int i = 0; i < 4; i++) {
            int32 idx = m_aTestArray.Emplace();
            m_aTestArray[idx] = i;
        }
    };

    UPROPERTY()
    TArray<int32> m_aTestArray;

    UPROPERTY()
    int32 m_aTestArray2[4];

    UPROPERTY()
    TArray<FMyMJRoleDataAttenderCpp> m_aRoleDataAttenders;

};
*/

DECLARE_MULTICAST_DELEGATE(FMyStructContentDelegate);

UCLASS()
class MYONLINECARDGAME_API AMyMJGameTrivalDataSourceCpp : public AActor
{
    GENERATED_BODY()
public:

    AMyMJGameTrivalDataSourceCpp() : Super()
    {
        bReplicates = true;
        bAlwaysRelevant = true;
        bNetLoadOnClient = true;
        NetUpdateFrequency = 3;
    };

    virtual ~AMyMJGameTrivalDataSourceCpp()
    {

    };

    inline FMyMJGameTrivalDataCpp& getDataRef()
    {
        return m_cData;
    };

    FMyStructContentDelegate m_cReplicateDelegate;

protected:

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_Data()
    {
        m_cReplicateDelegate.Broadcast();
    };

    UPROPERTY(ReplicatedUsing = OnRep_Data, meta = (DisplayName = "data"))
    FMyMJGameTrivalDataCpp m_cData;
};

UCLASS(Blueprintable, NotBlueprintType)
class MYONLINECARDGAME_API UMyMJGameTrivalVisualDataObjCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJGameTrivalVisualDataObjCpp()
    {
        m_pTestBuffer = CreateDefaultSubobject<UMyMJGameEventCycleBuffer>(TEXT("test buffer"));
    };

    virtual ~UMyMJGameTrivalVisualDataObjCpp()
    {

    };

protected:

    //UPROPERTY()
    //FMyTestDataCpp m_aTestData;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "test Buffer"))
        UMyMJGameEventCycleBuffer *m_pTestBuffer;
};