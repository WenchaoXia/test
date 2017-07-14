// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Utils/MyMJUtils.h"
#include "MyMJCardPack.h"

#include "MyMJCommonDefines.h"

#include "MyMJGamePushersIO.h"

#include "MyMJGameCmd.generated.h"


UENUM()
enum class MyMJGameCmdType : uint8
{
    Invalid = 0          UMETA(DisplayName = "Invalid"),
    RestartGame = 10     UMETA(DisplayName = "RestartGame")
};


USTRUCT()
struct FMyMJGameCmdBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCmdBaseCpp()
    {
        m_eType = MyMJGameCmdType::Invalid;

        m_eRespErrorCode = MyMJGameErrorCodeCpp::NotHandled;
    };

    virtual ~FMyMJGameCmdBaseCpp()
    {};


    MyMJGameCmdType m_eType;
    //int32 m_iIdxAttender; //see MyMJGameRoleTypeCpp

    MyMJGameErrorCodeCpp m_eRespErrorCode;
};

USTRUCT()
struct FMyMJGameCmdRestartGameCpp : public FMyMJGameCmdBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCmdRestartGameCpp() : Super()
    {
        m_eType = MyMJGameCmdType::RestartGame;
        m_iAttenderRandomSelectMask = 0;
    };

    virtual ~FMyMJGameCmdRestartGameCpp()
    {};


    FMyMJGameCfgCpp     m_cGameCfg;
    FMyMJGameRunDataCpp m_cGameRunData;
    int32 m_iAttenderRandomSelectMask;
};


USTRUCT()
struct FMyMJGameCmdIOCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCmdIOCpp()
    {
        //m_eDataRoleType = MyMJGameRoleTypeCpp::Max;
    };

    virtual ~FMyMJGameCmdIOCpp()
    {

    };

    //MyMJGameRoleTypeCpp m_eDataRoleType;

    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc>* m_pInputQueueRemote;
    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc>* m_pOutputQueueRemote;
};

class FMyMJGameCmdIOComponentCpp
{
public:
    FMyMJGameCmdIOComponentCpp()
    {

    };

    virtual ~FMyMJGameCmdIOComponentCpp()
    {

    };

    void init(TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> **ppInputQueuesRemote, TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> **ppOutputQueuesRemote, int32 iRemoteQueuesCount)
    {
        for (int32 i = 0; i < iRemoteQueuesCount; i++) {
            int32 idx = m_aIOs.Emplace();
            FMyMJGameCmdIOCpp *pIO = &m_aIOs[idx];
            pIO->m_pInputQueueRemote  = ppInputQueuesRemote[i];
            pIO->m_pOutputQueueRemote = ppOutputQueuesRemote[i];
        }
    };

    TArray<FMyMJGameCmdIOCpp> m_aIOs;
};
