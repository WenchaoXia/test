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
    };

    virtual ~FMyMJGameCmdBaseCpp()
    {};

    virtual FString ToString() const
    {
        FString ret = FString::Printf(TEXT("type %s, ret %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCmdType"), (uint8)m_eType), *m_cRespErrorCode.ToString());
        return ret;
    };

    inline MyMJGameCmdType getType() const
    {
        return m_eType;
    };

    UPROPERTY()
    MyMJGameCmdType m_eType;
    //int32 m_iIdxAttender; //see MyMJGameRoleTypeCpp

    UPROPERTY()
    FMyErrorCodeMJGameCpp m_cRespErrorCode;
};

USTRUCT()
struct FMyMJGameCmdRestartGameCpp : public FMyMJGameCmdBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCmdRestartGameCpp() : Super()
    {
        m_eType = MyMJGameCmdType::RestartGame;
    };

    virtual ~FMyMJGameCmdRestartGameCpp()
    {};

    UPROPERTY()
    FMyMJGameCfgCpp     m_cGameCfg;

    UPROPERTY()
    FMyMJGameRunDataCpp m_cGameRunData;

    //every attender takes 8 bit
    //UPROPERTY()
    //int32 m_iAttendersAllRandomSelectMask;
};


//never use it across thread
USTRUCT()
struct FMyMJGameCmdPointersCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCmdPointersCpp()
    {
        clear();
    };

    virtual ~FMyMJGameCmdPointersCpp()
    {
        clear();
    };

    void clear()
    {
        m_aCmdSharedPtr.Reset();
    };

    FMyMJGameCmdPointersCpp& operator = (const FMyMJGameCmdPointersCpp& rhs)
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGameCmdPointersCpp::operator =  , %p, %p."), this, &rhs);

        this->m_aCmdSharedPtr = rhs.m_aCmdSharedPtr;

        return *this;
    };



    bool operator == (const FMyMJGameCmdPointersCpp& rhs) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGameCmdPointersCpp::operator ==  , %p, %p."), this, &rhs);
        return false;
    };


    inline
    int32 giveInLocalThread(const TSharedPtr<FMyMJGameCmdBaseCpp> pCmd)
    {
        return m_aCmdSharedPtr.Emplace(pCmd);
    };

    //input must be allocated on heap, and this function will take ownership
    inline
    int32 giveInLocalThread(const FMyMJGameCmdBaseCpp *pCmd)
    {
        return giveInLocalThread(MakeShareable<FMyMJGameCmdBaseCpp>((FMyMJGameCmdBaseCpp *)pCmd));
    };

    //can only be called on consumer thread if consumer thread != producer thread, otherwise random crash
    inline
    TSharedPtr<FMyMJGameCmdBaseCpp> getSharedPtrAt(int32 idx)
    {

        return ConstCastSharedPtr<FMyMJGameCmdBaseCpp>(getSharedPtrAtConst(idx));
    };

    inline
    const TSharedPtr<FMyMJGameCmdBaseCpp> getSharedPtrAtConst(int32 idx) const
    {

        MY_VERIFY(idx >= 0 && idx < m_aCmdSharedPtr.Num());

        return m_aCmdSharedPtr[idx];
    };

 

    inline int32 getCount() const
    {
        return m_aCmdSharedPtr.Num();
    };

    FString ToString() const
    {
        int32 l = getCount();
        FString str = FString::Printf(TEXT("count: %d."), l);
        for (int32 i = 0; i < l; i++) {
            str += FString::Printf(TEXT(" idx %d: "), i) + getSharedPtrAtConst(i)->ToString();
        }

        return str;
    };

    bool helperFillAsSegmentFromIOGroup(FMyMJGameIOGroupCpp *pIOGroup);

    bool trySerializeWithTag(FArchive &Ar);

    bool Serialize(FArchive& Ar);
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
    //bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms);


protected:

    TArray<TSharedPtr<FMyMJGameCmdBaseCpp>> m_aCmdSharedPtr;
};

template<>
struct TStructOpsTypeTraits<FMyMJGameCmdPointersCpp> : public TStructOpsTypeTraitsBase2<FMyMJGameCmdPointersCpp>
{
    enum
    {
        //WithExportTextItem = true, // struct has an ExportTextItem function used to serialize its state into a string.
        //WithImportTextItem = true, // struct has an ImportTextItem function used to deserialize a string into an object of that class.
        WithSerializer = true,
        WithNetSerializer = true,
        //WithNetDeltaSerializer = true,
        WithIdenticalViaEquality = true,
        //WithSerializeFromMismatchedTag = true,
    };
};



USTRUCT()
struct FMyMJGameCmdIOCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCmdIOCpp()
    {
        //m_eDataRoleType = MyMJGameRoleTypeCpp::Max;
        m_pInputQueueRemote = NULL;
        m_pOutputQueueRemote = NULL;
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
        m_aIOs.Reset();
    };

    virtual ~FMyMJGameCmdIOComponentCpp()
    {

    };

    void init(TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> **ppInputQueuesRemote, TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc> **ppOutputQueuesRemote, int32 iRemoteQueuesCount)
    {
        MY_VERIFY(m_aIOs.Num() == 0); //Verified it is inited only once

        for (int32 i = 0; i < iRemoteQueuesCount; i++) {
            int32 idx = m_aIOs.Emplace();
            FMyMJGameCmdIOCpp *pIO = &m_aIOs[idx];
            pIO->m_pInputQueueRemote  = ppInputQueuesRemote[i];
            pIO->m_pOutputQueueRemote = ppOutputQueuesRemote[i];
        }
    };

    TArray<FMyMJGameCmdIOCpp> m_aIOs;
};
