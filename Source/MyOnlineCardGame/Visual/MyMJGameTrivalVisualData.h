// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MJBPEncap/MyMJGameCoreBP.h"

#include "MyMJGameTrivalVisualData.generated.h"

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

protected:

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

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