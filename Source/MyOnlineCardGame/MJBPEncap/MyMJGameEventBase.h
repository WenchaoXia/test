// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MJ/MyMJGameAttender.h"

//#include "GameFramework/Actor.h"
//#include "UnrealNetwork.h"

#include "MyMJGameEventBase.generated.h"

//although 16ms is precise enough, but it may cause extra delay up to 16ms, let's remove it in data source by use precion as 1ms

/*
#define MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(dataTime)   ((dataTime) << 5)
#define MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(worldTime_ms) ((worldTime_ms) >> 5)

#define MY_MJ_GAME_WORLD_TIME_MS_RESOLUTION MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(1)

#define MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(worldTime_ms) (MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(worldTime_ms)))

#define MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(worldTime_ms) (MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(worldTime_ms) == (worldTime_ms))
*/

//every actor can have it's own state about time, we define a common struct to calculate it, the time can be either game world time, or platform real time
USTRUCT()
struct FMyMJServerClientTimeBondCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJServerClientTimeBondCpp()
    {
        clearBond();
    };

    bool rebondTime(uint32 uiServerTime_ms, uint32 uiClientTime_ms)
    {
        if (uiServerTime_ms == 0 && uiClientTime_ms == 0) {
            return false;
        }

        m_uiServerTime_ms = uiServerTime_ms;
        m_uiClientTime_ms = uiClientTime_ms;

        return true;
    };

    void clearBond()
    {
        m_uiServerTime_ms = 0;
        m_uiClientTime_ms = 0;
    };

    inline
    bool haveBond() const
    {
        return (m_uiServerTime_ms > 0 || m_uiClientTime_ms > 0);
    };


    inline
    uint32 getCalculatedServerTime_ms(uint32 uiNewClientTime_ms) const
    {
        MY_VERIFY(haveBond());
        if (uiNewClientTime_ms > m_uiClientTime_ms) {
            return uiNewClientTime_ms - m_uiClientTime_ms + m_uiServerTime_ms;
        }
        else {
            return m_uiClientTime_ms - uiNewClientTime_ms + m_uiServerTime_ms;
        }
    };

    inline
    const uint32& getServerTime_ms_RefConst() const
    {
        return m_uiServerTime_ms;
    };

    inline
    const uint32& getClientTime_ms_RefConst() const
    {
        return m_uiClientTime_ms;
    };

protected:

    uint32 m_uiServerTime_ms;
    uint32 m_uiClientTime_ms;
};

UENUM()
enum class MyMJGameCoreRelatedEventMainTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    CorePusherResult = 1     UMETA(DisplayName = "CorePusherResult"),
    Trival = 2    UMETA(DisplayName = "Trival"),
};


//uint is MS
USTRUCT()
struct FMyMJCoreRelatedEventCorePusherCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreRelatedEventCorePusherCfgCpp();

    //return uint is ms, resolved
    uint32 helperGetDeltaDur(const FMyMJDataDeltaCpp& delta) const;

    uint32 m_uiGameStarted;

    uint32 m_uiThrowDices;

    uint32 m_uiDistCardsDone;

    uint32 m_uiHuBornLocalCS;

    uint32 m_uiTakeCards;

    uint32 m_uiGiveCards;

    uint32 m_uiWeaveChi;

    uint32 m_uiWeavePeng;

    uint32 m_uiWeaveGang;

    uint32 m_uiWeaveGangBuZhangLocalCS;

    uint32 m_uiHu;

    uint32 m_uiZhaNiaoLocalCS;

    uint32 m_uiGameEnded;
};


USTRUCT()
struct FMyMJGameCoreRelatedEventTrivalCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCoreRelatedEventTrivalCpp()
    {
    };

    virtual ~FMyMJGameCoreRelatedEventTrivalCpp()
    {

    };

    FString genDebugMsg() const
    {
        return FString::Printf(TEXT("trival"));
    };


protected:

};

USTRUCT(BlueprintType)
struct FMyMJDataStructWithTimeStampBaseCpp : public FMyMJDataStructCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJDataStructWithTimeStampBaseCpp() : Super()
    {
        m_uiIdEventApplied = MyUIntIdDefaultInvalidValue;
        m_uiTime_ms = 0;

        //m_cAccessor.setupDataExt(this);
    };

    virtual ~FMyMJDataStructWithTimeStampBaseCpp()
    {

    };

    void reset()
    {
        Super::reset();
        m_uiIdEventApplied = MyUIntIdDefaultInvalidValue;
        m_uiTime_ms = 0;
    };


    //unit is ms
    inline
    uint32 getIdEventApplied() const
    {
        return m_uiIdEventApplied;
    };

    inline
    void setTime_ms(uint32 uiTime_ms)
    {
        MY_VERIFY(uiTime_ms > 0);
        m_uiTime_ms = uiTime_ms;
    };

    //unit is ms
    inline
    uint32 getTime_ms() const
    {
        return m_uiTime_ms;
    };

    void applyEvent(FMyMJDataAccessorCpp& cAccessor, const struct FMyMJEventWithTimeStampBaseCpp& cEvent);

    void copyWithRoleFromSysKeeperRole2(MyMJGameRoleTypeCpp eTargetRole, FMyMJDataStructWithTimeStampBaseCpp& cTargetData) const
    {
        Super::copyWithRoleFromSysKeeperRole(eTargetRole, cTargetData);
        cTargetData.m_uiIdEventApplied = m_uiIdEventApplied;
        cTargetData.m_uiTime_ms = m_uiTime_ms;
    };

    FString genDebugMsg() const
    {
        return FString::Printf(TEXT("full %d, %s: gameid %d, pusherIdLast %d."), m_uiIdEventApplied, *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(m_uiTime_ms), getCoreDataRefConst().m_iGameId, getCoreDataRefConst().m_iPusherIdLast);
    };

protected:

    UPROPERTY(meta = (DisplayName = "id of event applied"))
    uint32 m_uiIdEventApplied;

    UPROPERTY(meta = (DisplayName = "server world time stamp"))
    uint32 m_uiTime_ms;

    //FMyMJDataAccessorCpp m_cAccessor;
};


#define MyMJEventContinuityYes (1)
#define MyMJEventContinuityNoPrevBaseEmpty (2)
#define MyMJEventContinuityNoEventSkipped (3)

//represent a important event, which may have visual effect, or make important change to game
USTRUCT(BlueprintType)
struct FMyMJEventWithTimeStampBaseCpp : public FFastArraySerializerItem
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJEventWithTimeStampBaseCpp() : Super()
    {
        reset();
    };

    virtual ~FMyMJEventWithTimeStampBaseCpp()
    {

    };

    void reset()
    {
        m_aCorePusherResult.Reset();
        m_aTrival.Reset();
        m_uiStartTime_ms = 0;
        m_uiDuration_ms = 0;
        m_uiIdEvent = MyUIntIdDefaultInvalidValue;

        m_iIdxDebug = 0;
    };

    //unit is ms
    inline
    uint32 getEndTime_ms() const
    {
        return m_uiStartTime_ms + m_uiDuration_ms;
    }

    inline
    void setStartTime_ms(uint32 uiStartTime_ms)
    {
        MY_VERIFY(uiStartTime_ms > 0);
        m_uiStartTime_ms = uiStartTime_ms;
    };

    //unit is ms, resolved
    inline
    uint32 getStartTime_ms() const
    {
        return m_uiStartTime_ms;
    };

    inline
    void setDuration_ms(uint32 uiDuration_ms)
    {
        m_uiDuration_ms = uiDuration_ms;
    };

    //unit is ms, resolved
    inline
    uint32 getDuration_ms() const
    {
        return m_uiDuration_ms;
    };

    inline
        uint32 getIdEvent() const
    {
        return m_uiIdEvent;
    };

    inline
        void setIdEvent(uint32 id)
    {
        m_uiIdEvent = id;
        MY_VERIFY(m_uiIdEvent != MyUIntIdDefaultInvalidValue && m_uiIdEvent < MyUInt32IdCoreDumpBottomValue);
    };

    void setPusherResult(const FMyMJGamePusherResultCpp& pusherResult)
    {
        MY_VERIFY(m_aCorePusherResult.Num() <= 0);
        MY_VERIFY(m_aTrival.Num() <= 0);

        int32 idx = m_aCorePusherResult.Emplace();
        m_aCorePusherResult[idx] = pusherResult;
    };

    const FMyMJGamePusherResultCpp* getPusherResult(bool bAssertValid) const
    {
        if (m_aCorePusherResult.Num() > 0) {
            return &m_aCorePusherResult[0];
        }
        else {
            if (bAssertValid) {
                MY_VERIFY(false);
            }
            return NULL;
        }
    };

    void setEventTrival(const FMyMJGameCoreRelatedEventTrivalCpp& eventTrival)
    {
        MY_VERIFY(m_aCorePusherResult.Num() <= 0);
        MY_VERIFY(m_aTrival.Num() <= 0);

        int32 idx = m_aTrival.Emplace();
        m_aTrival[idx] = eventTrival;
    };

    const FMyMJGameCoreRelatedEventTrivalCpp* getEventTrival(bool bAssertValid) const
    {
        if (m_aTrival.Num() > 0) {
            return &m_aTrival[0];
        }
        else {
            if (bAssertValid) {
                MY_VERIFY(false);
            }
            return NULL;
        }
    };

    void copyWithRoleFromSysKeeperRole2(MyMJGameRoleTypeCpp eTargetRole, FMyMJEventWithTimeStampBaseCpp& cTargetData) const
    {
        cTargetData.m_aCorePusherResult.Reset();

        int l = m_aCorePusherResult.Num();
        MY_VERIFY(l <= 1);
        if (l > 0) {
            cTargetData.m_aCorePusherResult.Emplace();
            m_aCorePusherResult[0].copyWithRoleFromSysKeeperRole(eTargetRole, cTargetData.m_aCorePusherResult[0]);
        }

        cTargetData.m_aTrival = m_aTrival;

        cTargetData.m_uiStartTime_ms = m_uiStartTime_ms;
        cTargetData.m_uiDuration_ms  = m_uiDuration_ms;
        cTargetData.m_uiIdEvent = m_uiIdEvent;

        cTargetData.m_iIdxDebug = m_iIdxDebug;
    };

    // Optional: debug string used with LogNetFastTArray logging
    FString GetDebugString()
    {
        return FString::Printf(TEXT("FMyMJEventWithTimeStampBaseCpp idxDebug %d."), m_iIdxDebug);
    };

    inline
    int32 checkContinuity(uint32 uiIdEventPrev) const
    {
        //treat it carefully, when id overscrewed, it will be considered as empty
        if (getIdEvent() == MyUIntIdDefaultInvalidValue) {
            return MyMJEventContinuityNoPrevBaseEmpty;
        }
        else if (uiIdEventPrev == (getIdEvent() + 1)) {
            return MyMJEventContinuityYes;
        }

        return MyMJEventContinuityNoEventSkipped;
    };

    /*
    int32 canBeAppendedToPrevAsCorePusher(int32 iGameIdPrev, int32 iPusherIdLastPrev, bool bIsGameEndOrNotStartedPrev, int32 iTrivalIdPrev) const
    {
        int32 ret = 0;
        MyMJGameCoreRelatedEventMainTypeCpp eMainType = getMainType();
        
        if (eMainType == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult)
        {
            int32 iGameIdFirst = -1;
            int32 iPusherIdFirst = -1;
            const FMyMJEventWithTimeStampBaseCpp* pItemFirst = this;

            pItemFirst->getPusherResult(true)->getGameIdAndPusherId(&iGameIdFirst, &iPusherIdFirst);

            if (iGameIdPrev < 0 || bIsGameEndOrNotStartedPrev) {
                ret = pItemFirst->getPusherResult(true)->m_aResultBase.Num() > 0 ? MyMJEventContinuityYes : MyMJEventContinuityNoPusherSkipped;
            }
            else {
                ret = iGameIdPrev == iGameIdFirst && (iPusherIdLastPrev + 1) == iPusherIdFirst ? MyMJEventContinuityYes : MyMJEventContinuityNoPusherSkipped;
            }

            if (ret == MyMJEventContinuityNoPusherSkipped) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("canBeAppendedToPrev pusher skipped: iGameIdPrev %d, iPusherIdLastPrev %d, bIsGameEndOrNotStartedPrev %d, iGameIdFirst %d, iPusherIdFirst %d."),
                    iGameIdPrev, iPusherIdLastPrev, bIsGameEndOrNotStartedPrev, iGameIdFirst, iPusherIdFirst);
            }

        }
        else if (eMainType == MyMJGameCoreRelatedEventMainTypeCpp::Trival)
        {
            int32 iTrivalIdNew = getEventTrival(true)->getId();
            ret = iTrivalIdPrev == MyIntIdDefaultInvalidValue || (iTrivalIdPrev + 1) == iTrivalIdNew ? MyMJEventContinuityYes : MyMJEventContinuityNoTrivalSkipped;

            if (ret == MyMJEventContinuityNoTrivalSkipped) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("canBeAppendedToPrev trival skipped: iTrivalIdPrev %d, iTrivalIdNew %d."),
                    iTrivalIdPrev, iTrivalIdNew);
            }
        }
        else
        {
            MY_VERIFY(false);
        }

        return ret;
        
    }
    */

    FString genDebugMsg() const
    {
        uint32 uiStartMs = m_uiStartTime_ms;
        uint32 uiDurMs = m_uiDuration_ms;
        FString resultStr = TEXT("null");
        if (m_aCorePusherResult.Num() > 0) {
            resultStr = m_aCorePusherResult[0].genDebugMsg();
        }
        else if (m_aTrival.Num() > 0) {
            resultStr = m_aTrival[0].genDebugMsg();
        }

        return FString::Printf(TEXT("delta %d, %s(%s): %s"), m_uiIdEvent, *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(uiStartMs), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(uiDurMs), *resultStr);

    };

    inline MyMJGameCoreRelatedEventMainTypeCpp getMainType() const
    {
        if (m_aCorePusherResult.Num() == 1 && m_aTrival.Num() == 0) {
            return MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult;
        }
        else if (m_aCorePusherResult.Num() == 0 && m_aTrival.Num() == 1) {
            return MyMJGameCoreRelatedEventMainTypeCpp::Trival;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid core related event: %d, %d"), m_aCorePusherResult.Num(), m_aTrival.Num());
            return MyMJGameCoreRelatedEventMainTypeCpp::Invalid;
        }
    };

    UPROPERTY()
    int32 m_iIdxDebug;

protected:

    //actually union, if valid, Num() == 1 and it is a delta pusher
    UPROPERTY()
    TArray<FMyMJGamePusherResultCpp>  m_aCorePusherResult;

    UPROPERTY()
    TArray<FMyMJGameCoreRelatedEventTrivalCpp>  m_aTrival;

    //timestamp is from server world time
    UPROPERTY(meta = (DisplayName = "state server world time stamp"))
        uint32 m_uiStartTime_ms;

    //
    UPROPERTY(meta = (DisplayName = "time wait for animation"))
        uint32 m_uiDuration_ms;

    UPROPERTY()
    uint32 m_uiIdEvent;

};

USTRUCT()
struct FMyMJGameEventArray : public FFastArraySerializer
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventArray() : Super()
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FMyMJGameEventArray() called"));
    };

    virtual ~FMyMJGameEventArray()
    {

    };

    //UPROPERTY()
    //TArray<FMyMJEventWithTimeStampBaseCpp> m_aEvents;

    UPROPERTY()
    TArray<FMyMJEventWithTimeStampBaseCpp>	Items;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FMyMJEventWithTimeStampBaseCpp, FMyMJGameEventArray>(Items, DeltaParms, *this);
    };

};

template<>
struct TStructOpsTypeTraits< FMyMJGameEventArray > : public TStructOpsTypeTraitsBase2< FMyMJGameEventArray >
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};

//extra param is just for test
DECLARE_MULTICAST_DELEGATE_OneParam(FMyMJGameEventCycleBufferReplicatedDelegate, int32);

//Todo: make it template
//Different game types have different datas, for a turn based one, history is important
//it is like a queue with limited size, but don't need allocate memory, thread unsafe
//It seems fast tarray replication, must be directly touched to a uobject!
//range model is [min, max)
UCLASS()
class UMyMJGameEventCycleBuffer : public UObject
{
    GENERATED_BODY()

public:

    UMyMJGameEventCycleBuffer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_iTest = 0;
        resize(64);

        m_cEventArray.MarkArrayDirty();
    };

    virtual ~UMyMJGameEventCycleBuffer()
    {

    };

    //will trigger a clear()
    void resize(int32 iNewSizeMax)
    {
        m_iSizeMax = iNewSizeMax;
        MY_VERIFY(m_iSizeMax > 0);

        //m_cEventArray.Items.AddZeroed(m_iSizeMax);

        clearInGame();

        //m_cEventArray.Items.Reset(0);
        //m_cEventArray.MarkArrayDirty();
    };

    void clearInGame()
    {
        m_idxHead = MyIntIdDefaultInvalidValue;
        m_iCount = 0;
        m_uiTimeRangeStart_ms = 0;
        m_uiTimeRangeEnd_ms = 0;

        //m_iHelperIdxLastCorePusherEvent = MyIntIdDefaultInvalidValue;
        //m_iHelperIdxLastTrivalEvent = MyIntIdDefaultInvalidValue;
    };

    //This will assert if not enough of items were removed
    void removeFromHead(int32 countToRemove)
    {
        MY_VERIFY(countToRemove > 0);

        if (m_iCount >= countToRemove) {
            m_idxHead = (m_idxHead + countToRemove) % m_iSizeMax;
            m_iCount -= countToRemove;
        }
        else {
            MY_VERIFY(false);
        }

        //m_iHelperIdxLastCorePusherEvent = m_iHelperIdxLastCorePusherEvent < countToRemove ? -1 : m_iHelperIdxLastCorePusherEvent - countToRemove;
        //m_iHelperIdxLastTrivalEvent = m_iHelperIdxLastTrivalEvent < countToRemove ? -1 : m_iHelperIdxLastTrivalEvent - countToRemove;

        if (m_iCount > 0) {
            const FMyMJEventWithTimeStampBaseCpp& event = peekRefAt(0);
            uint32 uiStartTime = event.getStartTime_ms();
            uint32 uiEndTime = event.getStartTime_ms() + event.getDuration_ms();

            //detect error
            if (m_uiTimeRangeStart_ms > uiStartTime) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw: m_uiTimeRangeStart_ms %d, uiStartTime %d."), m_uiTimeRangeStart_ms, uiStartTime);
                MY_VERIFY(false);
            }
            if (m_uiTimeRangeEnd_ms < uiEndTime) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw: m_uiTimeRangeEnd_ms %d, uiEndTime %d."), m_uiTimeRangeEnd_ms, uiEndTime);
                MY_VERIFY(false);
            }

            if (m_uiTimeRangeStart_ms < uiStartTime) {
                m_uiTimeRangeStart_ms = uiStartTime;
            }

        }
        else {
            m_uiTimeRangeStart_ms = 0;
            m_uiTimeRangeEnd_ms = 0;
        }
    };
    
    inline
    int32 getCount(bool *pOutIsFull) const
    {
        if (pOutIsFull) {
            *pOutIsFull = m_iCount >= m_iSizeMax;
        }

        return m_iCount;
    };

    inline
    int32 getSizeMax() const
    {
        return m_iSizeMax;
    };

    inline const FMyMJEventWithTimeStampBaseCpp* peekLast() const
    {
        if (m_iCount > 0) {
            return &peekRefAt(m_iCount - 1);
        }

        return NULL;
    };

    //This will assert if out of range
    const FMyMJEventWithTimeStampBaseCpp& peekRefAt(int32 idxFromHead, int32 *pOutIdxInArrayDebug = NULL, bool bVerify = true) const
    {
        MY_VERIFY(idxFromHead < m_iCount && idxFromHead >= 0);
        int32 idxFound = (m_idxHead + idxFromHead) % m_iSizeMax;
        if (pOutIdxInArrayDebug) {
            *pOutIdxInArrayDebug = idxFound;
        }

        int32 l = m_cEventArray.Items.Num();
        if (idxFound >= l) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idx out of range %d, %d / %d."), idxFromHead, idxFound, l);
            MY_VERIFY(false);
        }
        const FMyMJEventWithTimeStampBaseCpp& ret = m_cEventArray.Items[idxFound];

        //extra check
        //uint32 time_ms = ret.getEndTime_ms();
        //if (time_ms <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem at %d have timestamp %d, not valid! internal state: m_idxHead %d, m_iCount %d."), idxFromHead, time_ms, m_idxHead, m_iCount);
            //MY_VERIFY(false);
        //}

        if (bVerify) {
            if (ret.m_iIdxDebug != idxFound) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem idx not equal, idxFromHead %d, idxFound %d, m_iIdxDebug %d."), idxFromHead, idxFound, ret.m_iIdxDebug);
                MY_VERIFY(false);
            }
        }

        return ret;
    };

    /*
    inline
    const FMyMJEventWithTimeStampBaseCpp* getLastCorePusherEvent() const
    {
        if (m_iHelperIdxLastCorePusherEvent >= 0) {
            return &peekRefAt(m_iHelperIdxLastCorePusherEvent);
        }

        return NULL;
    };

    inline
    const FMyMJEventWithTimeStampBaseCpp* getLastTrivalEvent() const
    {
        if (m_iHelperIdxLastTrivalEvent >= 0) {
            return &peekRefAt(m_iHelperIdxLastTrivalEvent);
        }

        return NULL;
    };
    */

    //will assert if buffer is full
    FMyMJEventWithTimeStampBaseCpp& addToTailWhenNotFull(uint32 uiTimeStamp_ms, uint32 uiDur_ms, MyMJGameCoreRelatedEventMainTypeCpp eMainType)
    {
        MY_VERIFY(uiTimeStamp_ms > 0);

        FMyMJEventWithTimeStampBaseCpp* pRet = NULL;
        bool bOverritten = addToTail(NULL, &pRet);
        MY_VERIFY(!bOverritten);
        MY_VERIFY(pRet);
        FMyMJEventWithTimeStampBaseCpp& ret = *pRet;;

        ret.setStartTime_ms(uiTimeStamp_ms);
        ret.setDuration_ms(uiDur_ms);

        /*
        if (eMainType == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult) {
            const FMyMJEventWithTimeStampBaseCpp* pE = getLastCorePusherEvent();
            if (pE && pE->getMainType() != eMainType) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_iHelperIdxLastCorePusherEvent error detected, idx %d's have unexpected type as %d."), m_iHelperIdxLastCorePusherEvent, (uint8)pE->getMainType());
                MY_VERIFY(false);
            }
            m_iHelperIdxLastCorePusherEvent = getCount(NULL) - 1;
        }
        else if (eMainType == MyMJGameCoreRelatedEventMainTypeCpp::Trival) {
            const FMyMJEventWithTimeStampBaseCpp* pE = getLastTrivalEvent();
            if (pE && pE->getMainType() != eMainType) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_iHelperIdxLastCorePusherEvent error detected, idx %d's have unexpected type as %d."), m_iHelperIdxLastTrivalEvent, (uint8)pE->getMainType());
                MY_VERIFY(false);
            }
            m_iHelperIdxLastTrivalEvent = getCount(NULL) - 1;
        }
        else {
            MY_VERIFY(false);
        }
        */

        uint32 uiStartTime = uiTimeStamp_ms;
        uint32 uiEndTime = uiTimeStamp_ms + uiDur_ms;

        if (m_uiTimeRangeStart_ms == 0 && m_uiTimeRangeEnd_ms == 0) {

            m_uiTimeRangeStart_ms = uiStartTime;
            m_uiTimeRangeEnd_ms = uiEndTime;

        }
        else {

            //detect error
            if (m_uiTimeRangeStart_ms > uiStartTime) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw: m_uiTimeRangeStart_ms %d, uiStartTime %d."), m_uiTimeRangeStart_ms, uiStartTime);
                MY_VERIFY(false);
            }

            if (m_uiTimeRangeEnd_ms < uiEndTime) {
                m_uiTimeRangeEnd_ms = uiEndTime;
            }
        }

        return ret;
    };

    inline uint32 getTimeRangeStart_ms() const
    {
        return m_uiTimeRangeStart_ms;
    };

    inline uint32 getTimeRangeEnd_ms() const
    {
        return m_uiTimeRangeEnd_ms;
    };

    //return errorCode
    int32 verifyData(bool bShowLog) const
    {
        uint32 timestamp_end = 0;
        int32 l = getCount(NULL);
        int32 iError = 0;
        int32 idEventLast = -1;
        for (int32 i = 0; i < l; i++) {
            int32 idxInArray = -1;
            const FMyMJEventWithTimeStampBaseCpp& elem = peekRefAt(i, &idxInArray, false);

            if (elem.m_iIdxDebug != idxInArray) {
                //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem's id not equal, elem id %d, real id %d, idx from head %d."), elem.m_iIdxDebug, idxInArray, i);
                //MY_VERIFY(false);

                iError = 1;
                break;
            }

            if (idEventLast != -1 && (idEventLast + 1) != elem.getIdEvent()) {
                iError = 1;
                break;
            }
            idEventLast = elem.getIdEvent();


            uint32 te = elem.getEndTime_ms();
            if (te > 0) {
                if (te < timestamp_end) {
                    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time stamp screw, prev time %d, elem time %d, idx from head %d."), timestamp_end, te, i);
                    //MY_VERIFY(false);

                    iError = 2;
                    break;
                }

                timestamp_end = te;
            }
        }

        if (iError != 0 || bShowLog) {

            FString ret;
            for (int32 i = 0; i < l; i++) {
                int32 idxInArray = -1;
                const FMyMJEventWithTimeStampBaseCpp& elem = peekRefAt(i, &idxInArray, false);
                MyMJGameCoreRelatedEventMainTypeCpp eMainType = elem.getMainType();
                int32 idEvent = elem.getIdEvent();
                if (eMainType == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult) {
                    const FMyMJGamePusherResultCpp* pResult = elem.getPusherResult(true);

                    int32 iGameId, iPusherId;
                    pResult->getGameIdAndPusherId(&iGameId, &iPusherId);
                    ret += FString::Printf(TEXT("[p %d,%d,%d,%d,%d,%d, %d:%d], "), i, idxInArray, elem.m_iIdxDebug, elem.getStartTime_ms(), elem.getDuration_ms(), idEvent, iGameId, iPusherId);
                }
                else if (eMainType == MyMJGameCoreRelatedEventMainTypeCpp::Trival) {
                    const FMyMJGameCoreRelatedEventTrivalCpp* pEvent = elem.getEventTrival(true);
                    ret += FString::Printf(TEXT("[t %d,%d,%d,%d,%d,%d], ")       , i, idxInArray, elem.m_iIdxDebug, elem.getStartTime_ms(), elem.getDuration_ms(), idEvent);
                }
                else {
                    MY_VERIFY(false);
                }
            }
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("l %d, iError %d: %s"), l, iError, *ret);
            //MY_VERIFY(false);
        }

        return iError;
    };

    void addItemFromOther(const FMyMJEventWithTimeStampBaseCpp& itemOther)
    {
        uint32 startTime = itemOther.getStartTime_ms();
        uint32 dur = itemOther.getDuration_ms();
        FMyMJEventWithTimeStampBaseCpp& newAdded = addToTailWhenNotFull(startTime, dur, itemOther.getMainType());
        int32 oldIdxDebug = newAdded.m_iIdxDebug;
        newAdded = itemOther;

        if (newAdded.m_iIdxDebug != oldIdxDebug) {
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("newAdded idx changing %d -> %d. "), newAdded.m_iIdxDebug, oldIdxDebug);
            newAdded.m_iIdxDebug = oldIdxDebug;
        }
    };

    FMyMJGameEventCycleBufferReplicatedDelegate m_cUpdateNotifier;

    UPROPERTY(ReplicatedUsing = OnRep_Data)
    int32 m_iTest;

protected:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_Data()
    {
        m_cUpdateNotifier.Broadcast(0);
    };

    //return whether buffer overflow and old data is overwriten
    bool addToTail(const FMyMJEventWithTimeStampBaseCpp* itemToSetAs, FMyMJEventWithTimeStampBaseCpp** ppOutNewAddedItem)
    {
        int32 idxNew = -1;
        /*
        if (m_iCount > 0) {
            MY_VERIFY(m_idxHead >= 0);
            idxNew = (m_idxHead + m_iCount) % m_iSizeMax;
        }
        else if (m_iCount == 0) {
            idxNew = 0;
            m_idxHead = 0;
        }
        else {
            MY_VERIFY(false);
        }
        */

        if (m_idxHead >= 0) {
            MY_VERIFY(m_idxHead >= 0);
            idxNew = (m_idxHead + m_iCount) % m_iSizeMax;
        }
        else {
            MY_VERIFY(m_iCount == 0);
            idxNew = 0;
            m_idxHead = 0;
        }

        int32 l = m_cEventArray.Items.Num();
        if (idxNew >= l) {
            MY_VERIFY(idxNew == l);
            int32 idx = m_cEventArray.Items.Emplace();
            MY_VERIFY(idx == idxNew);
        }

        FMyMJEventWithTimeStampBaseCpp& newItem = m_cEventArray.Items[idxNew];
        newItem.reset();
        newItem.m_iIdxDebug = idxNew;

        m_cEventArray.MarkItemDirty(newItem);
        //m_cEventArray.MarkArrayDirty();

        if (itemToSetAs) {
            newItem = *itemToSetAs;
        }
  
        if (ppOutNewAddedItem) {
            *ppOutNewAddedItem = &newItem;
        }

        m_iCount++;
        if (m_iCount > m_iSizeMax) {
            m_iCount = m_iSizeMax;
            m_idxHead = (idxNew + 1) % m_iSizeMax;
            return true;
        }

        return false;
    };

    //UPROPERTY(ReplicatedUsing = OnRep_Data)
    UPROPERTY(Replicated)
    FMyMJGameEventArray m_cEventArray;

    UPROPERTY(Replicated)
    int32 m_iSizeMax;

    //only valid when count > 0
    UPROPERTY(Replicated)
    int32 m_idxHead;

    UPROPERTY(Replicated)
    int32 m_iCount;

    UPROPERTY(Replicated)
    uint32 m_uiTimeRangeStart_ms;

    UPROPERTY(Replicated)
    uint32 m_uiTimeRangeEnd_ms;

    //UPROPERTY(ReplicatedUsing = OnRep_Data)
    //int32 m_iHelperIdxLastCorePusherEvent;

    //UPROPERTY(ReplicatedUsing = OnRep_Data)
    //int32 m_iHelperIdxLastTrivalEvent;

    //UPROPERTY(ReplicatedUsing = OnRep_Data)
    //int32 m_iHelperEventsBasePusherCount; //how many base puhser we have inside
};

//typedef class UMyMJDataSequencePerRoleCpp UMyMJDataSequencePerRoleCpp;
#define MyMJDataSequencePerRoleFullDataRecordTypeInvalid (-1)
#define MyMJDataSequencePerRoleFullDataRecordTypeNone (0)
#define MyMJDataSequencePerRoleFullDataRecordTypeBottom (1)
#define MyMJDataSequencePerRoleFullDataRecordTypeTop (2)

//DECLARE_MULTICAST_DELEGATE_TwoParams(FMyMJDataSeqReplicatedDelegate, UMyMJDataSequencePerRoleCpp*, int32);
DECLARE_MULTICAST_DELEGATE(FMyMJDataSeqReplicatedDelegate);

//mainly keeps a history, this is the logic core need to replicate, and should only be write at producer side(the server)
UCLASS()
class MYONLINECARDGAME_API UMyMJDataSequencePerRoleCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJDataSequencePerRoleCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    void createSubObjects(bool bInConstructor);

    //since base can't be rebuild until we start from sequence 1, so, it should only be called once in the beginning
    void init(MyMJGameRoleTypeCpp eRoleType)
    {
        //Make sure called only once
        MY_VERIFY(m_iFullDataRecordType == MyMJDataSequencePerRoleFullDataRecordTypeInvalid);

        m_cFullData.setRole(eRoleType);
        m_eRole = eRoleType;

        m_cAccessor.setupDataExt(&m_cFullData);

        clearInGame();
    };

    inline bool isEmpty() const
    {
        uint32 idEventLast = 0;
        getFullAndDeltaLastData(&idEventLast, NULL);

        return idEventLast == 0;
    };

    void setFullDataRecordType(int32 iFullDataRecordType)
    {
        if (m_iFullDataRecordType == iFullDataRecordType) {
            return;
        }

        MY_VERIFY(isEmpty());

        m_iFullDataRecordType = iFullDataRecordType;
    };

    void setHelperProduceMode(bool bHelperProduceMode)
    {
        if (m_bHelperProduceMode == bHelperProduceMode) {
            return;
        }

        MY_VERIFY(isEmpty());

        m_bHelperProduceMode = bHelperProduceMode;
    };

    void clearInGame()
    {
        m_cFullData.reset();
        if (IsValid(m_pDeltaDataEvents)) {
            m_pDeltaDataEvents->clearInGame();
        }
        m_uiServerWorldTime_ms = 0;

        m_cReplicateDelegate.Clear();

        markDirtyForRep();
    };

    void markDirtyForRep()
    {
        m_iRepKeyOfState++;
        if (m_iRepKeyOfState <= 0) {
            m_iRepKeyOfState = 1;
        }
    };

    int32 getRepKeyOfState() const
    {
        return m_iRepKeyOfState;
    };

    /*
    //return < 0 means empty, no valid data inside
    void getGameIdAndPusherIdLast(int32 *pOutGameIdLast, int32* pOutPusherIdLast) const
    {
        const FMyMJEventWithTimeStampBaseCpp* pLastCorePusherEvent = NULL;

        if (IsValid(m_pEventsApplyingAndApplied)) {
            pLastCorePusherEvent = m_pEventsApplyingAndApplied->getLastCorePusherEvent();
        }

        if (pLastCorePusherEvent)
        {
            pLastCorePusherEvent->getPusherResult(true)->getGameIdAndPusherId(pOutGameIdLast, pOutPusherIdLast);
        }
        else {

            if (pOutGameIdLast) {
                *pOutGameIdLast = m_cBase.getCoreDataRefConst().m_iGameId;
                if (*pOutGameIdLast > 0) {
                    MY_VERIFY(m_cBase.getTime_ms() > 0);
                }
            }
            if (pOutPusherIdLast) {
                *pOutPusherIdLast = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
            }

        }
    };

    //return < 0 means empty, no valid data inside
    int32 getTrivalIdLast() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pLastTrivalEvent = NULL;

        if (IsValid(m_pEventsApplyingAndApplied)) {
            pLastTrivalEvent = m_pEventsApplyingAndApplied->getLastTrivalEvent();
        }

        if (pLastTrivalEvent)
        {
            return pLastTrivalEvent->getEventTrival(true)->getId();
        }
        else {
            int32 ret = m_cBase.getTrivalIdLast();
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime_ms() > 0); //we presume timestamp must e attached in our user case
            }

            return ret;
        }
    };


    bool isGameEndForLastCoreState() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pLastCorePusherEvent = NULL;

        if (IsValid(m_pEventsApplyingAndApplied)) {
            pLastCorePusherEvent = m_pEventsApplyingAndApplied->getLastCorePusherEvent();
        }

        if (pLastCorePusherEvent)
        {
            const FMyMJGamePusherResultCpp* pResult = pLastCorePusherEvent->getPusherResult(true);
            return pResult->isGameEndDelta();
        }
        else {
            int32 ret = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime_ms() > 0); //we presume timestamp must e attached in our user case
                return m_cBase.getCoreDataRefConst().m_eGameState == MyMJGameStateCpp::GameEnd;
            }
        }

        return false;
    };
    */

    //in most cases, we care about only deltas
    
    //unit is ms, can be called both in client and server, even in client case that subobject replication not completed
    void getFullAndDeltaLastData(uint32 *pOutLastEventId, uint32 *pOutLastEndTime) const;

    bool isReadyToGiveNextEvent(uint32 uiServerWorldTime_ms) const;

    //return the new added event's id
    uint32 addPusherResult(const FMyMJCoreRelatedEventCorePusherCfgCpp &inEventCorePusherCfg, const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_ms);


    void doTestChange()
    {
        /*
        m_cBase.setTime(m_cBase.getTime() + MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(1000));
        if (m_pEventsApplyingAndApplied->getCount(NULL) > 0) {
            m_pEventsApplyingAndApplied->clear();
        }
        else {
            int32 tBase = 0;
            for (int32 i = 0; i < 10; i++) {
                uint32 timeStart = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(tBase);
                uint32 timeDur = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(1500);
                tBase += timeDur;
                m_pEventsApplyingAndApplied->addToTailWhenNotFull(timeStart, timeDur, false);
            }
        }
        */
        if (IsValid(m_pDeltaDataEvents)) {
            m_pDeltaDataEvents->m_iTest++;
        }
    };

    FString genDebugMsg()
    {
        const FMyMJCoreDataPublicCpp& coreData = m_cFullData.getCoreDataRefConst();
        //int32 l = m_pEventsApplyingAndApplied->getCount(NULL);
        FString ret = FString::Printf(TEXT("role %d, base time %d [%d:%d:%d:%d], m_pEventsApplyingAndApplied valid %d."), (uint8)m_cFullData.getRole(), m_cFullData.getTime_ms(), coreData.m_iGameId, coreData.m_iPusherIdLast, coreData.m_iActionGroupId, (uint8)coreData.m_eGameState, IsValid(m_pDeltaDataEvents));
        if (IsValid(m_pDeltaDataEvents)) {
            int32 l = m_pDeltaDataEvents->getCount(NULL);
            ret += FString::Printf(TEXT("event count %d, event's m_iTest %d."), l, m_pDeltaDataEvents->m_iTest);
        }
        //for (int32 i = 0; i < l; i++) {
            //const FMyMJEventWithTimeStampBaseCpp& e = m_pEventsApplyingAndApplied->peekRefAt(i);
            //ret += FString::Printf(TEXT(" event %d, time start %d, dur %d."), i, e.getStartTime(), e.getDuration());
       // }

        return ret;
    };

    inline const FMyMJDataStructWithTimeStampBaseCpp& getFullData() const
    {
        return m_cFullData;
    };

    //warn: may return NULL if not replicated yet
    inline const UMyMJGameEventCycleBuffer* getDeltaDataEvents(bool bVerifyValid = false) const
    {
        if (IsValid(m_pDeltaDataEvents)) {
            return m_pDeltaDataEvents;
        }
        else {
            if (bVerifyValid) {
                MY_VERIFY(false);
            }
            return NULL;
        }
    };

    inline MyMJGameRoleTypeCpp getRole() const
    {
        return m_eRole;
    };

    inline uint32 getServerWorldTime_ms() const
    {
        return m_uiServerWorldTime_ms;
    };

    inline void setServerWorldTime_ms(uint32 uiServerWorldTime_ms)
    {
        m_uiServerWorldTime_ms = uiServerWorldTime_ms;
    };

    inline
    void resizeEvents(int32 newSize)
    {
        MY_VERIFY(m_cFullData.getIdEventApplied() == 0);
        m_cFullData.reset();

        if (IsValid(m_pDeltaDataEvents)) {
            MY_VERIFY(m_pDeltaDataEvents->getCount(NULL) == 0);
            m_pDeltaDataEvents->resize(newSize);
        }
        markDirtyForRep();
    }

    bool tryUpdateFullDataFromExternal(MyMJGameRoleTypeCpp eRole, const FMyMJDataStructWithTimeStampBaseCpp& cNewFullData)
    {
        if (m_bHelperProduceMode) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("this sequence is in produce mode, so it's full data can't be directly set."));
            return false;
        }

        if (m_eRole != eRole) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("role is different, m_eRole %d, eRole %d."), (uint8)m_eRole, (uint8)eRole);
            return false;
        }

        m_cFullData = cNewFullData;
        return true;
    };

    FMyMJDataSeqReplicatedDelegate m_cReplicateDelegate;

protected:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    //virtual void PostInitProperties() override;

    //just like remove, but info keepet to full data if behavior type specified in init
    void squashDeltaDataEvents(int32 count)
    {
        if (!IsValid(m_pDeltaDataEvents)) {
            return;
        }

        MY_VERIFY(count <= m_pDeltaDataEvents->getCount(NULL));

        if (m_iFullDataRecordType == MyMJDataSequencePerRoleFullDataRecordTypeBottom) {
            for (int32 i = 0; i < count; i++) {

                const FMyMJEventWithTimeStampBaseCpp& cEvent = m_pDeltaDataEvents->peekRefAt(i);
                m_cFullData.applyEvent(m_cAccessor, cEvent);

            }
        }

        m_pDeltaDataEvents->removeFromHead(count);

    };

    //make the state move forward
    void trySquashBaseAndEvents(uint32 uiServerWorldTime_ms);


    //UFUNCTION()
    //void OnRep_DeltaDataEventsPointer()
    //{
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's events replicated."), (uint8)m_eRole);
        //m_cReplicateDelegate.Broadcast(this, 1);

        //if (IsValid(m_pDeltaDataEvents)) {
            //m_pDeltaDataEvents->m_cUpdateNotifier.Clear();
            //m_pDeltaDataEvents->m_cUpdateNotifier.AddUObject(this, &UMyMJDataSequencePerRoleCpp::OnRep_DeltaDataEventsContent);
        //}

    //};

    //UFUNCTION()
    //void OnRep_DeltaDataEventsContent(int32 iExtra)
    //{
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's events replicated."), (uint8)m_eRole);
        //m_cReplicateDelegate.Broadcast();
    //};

    //design is simple: timestamp can represent the state's key, that means same key always have same content
    UFUNCTION()
        void OnRep_ServerWorldTime_ms();

    //following is the core data, representing

    UPROPERTY(Replicated)
    FMyMJDataStructWithTimeStampBaseCpp m_cFullData;

    //focused deltas, base is only used when merge or pull
    UPROPERTY(Replicated)
    UMyMJGameEventCycleBuffer* m_pDeltaDataEvents;

    UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "role"))
    MyMJGameRoleTypeCpp m_eRole;

    UPROPERTY(ReplicatedUsing = OnRep_ServerWorldTime_ms)
    uint32 m_uiServerWorldTime_ms;

    FMyMJDataAccessorCpp m_cAccessor;

    bool m_bHelperProduceMode;

    int32 m_iFullDataRecordType;

    int32 m_iRepKeyOfState;
};

/*
USTRUCT(BlueprintType)
struct FMyMJDataAtOneMomentCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataAtOneMomentCpp()
    {
        //m_eRole = MyMJGameRoleTypeCpp::Max;
        reinit();
    };

    void reinit()
    {
        m_cAccessor.setupDataExt(&m_cBase);
        clear();
    };

    void clear()
    {
        m_cBase.reset();
        m_uiMinServerWorldTimeForNextEvent_ms = 0;
    };

    //return < 0 means empty, no valid data inside
    int32 getGameIdLast() const
    {

        int32 ret = m_cBase.getCoreDataRefConst().m_iGameId;
        if (ret >= 0) {
            MY_VERIFY(m_cBase.getTime_ms() > 0); //we presume timestamp must e attached in our user case
        }

        return ret;
    };

    //return < 0 means empty, no valid data inside
    int32 getPusherIdLast() const
    {

        int32 ret = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
        if (ret >= 0) {
            MY_VERIFY(m_cBase.getTime_ms() > 0); //we presume timestamp must e attached in our user case
        }

        return ret;

    };

    void doTestChange()
    {
        m_cBase.setTime_ms(m_cBase.getTime_ms() + (1000));

    };

    FString genDebugMsg()
    {
        FString ret = FString::Printf(TEXT("role %d, base time %s."), (uint8)getRole(), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(m_cBase.getTime_ms()));

        return ret;
    };


    //unit is ms, resolved, not resolved
    uint32 getEndTime_unresolved_ms() const
    {
        uint32 uiTimeBase = m_cBase.getTime_ms();

        if (m_uiMinServerWorldTimeForNextEvent_ms > 0) {
            //if you have delta
            if (m_uiMinServerWorldTimeForNextEvent_ms < uiTimeBase) {
                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("timestamp chaos: base %s, delta %s."), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(uiTimeBase), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(m_uiMinServerWorldTimeForNextEvent_ms));
                MY_VERIFY(false);
            }
        }

        return uiTimeBase > m_uiMinServerWorldTimeForNextEvent_ms ? uiTimeBase : m_uiMinServerWorldTimeForNextEvent_ms;
    };


    bool isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_ms) const
    {
        bool bRet = uiServerWorldTime_ms >= m_uiMinServerWorldTimeForNextEvent_ms; //use >= to allow apply multiple one time

        return bRet;
    };

    void applyEvent(const FMyMJEventWithTimeStampBaseCpp& cEvent)
    {
       m_cAccessor.applyPusherResult(*cEvent.getPusherResult(true), NULL);
       if (cEvent.getDuration_ms() > 0) {
            uint32 uiEndTime = cEvent.getEndTime_ms();
            m_uiMinServerWorldTimeForNextEvent_ms = uiEndTime;
        }

    };

    void resetWithBase(const FMyMJDataStructWithTimeStampBaseCpp& cBase)
    {
        clear();
        m_cBase = cBase;
    };

    //warning: not checked whether the time passed
    inline
    bool isGameEndOrNotStarted() const
    {
        bool bNotStated = getGameIdLast() < 0;
        bool bGameEnd = m_cBase.getCoreDataRefConst().m_eGameState == MyMJGameStateCpp::GameEnd;
        return bNotStated || bGameEnd;
    };

    inline
    const FMyMJDataStructWithTimeStampBaseCpp& getBaseRefConst() const
    {
        return m_cBase;
    };

    inline
    FMyMJDataStructWithTimeStampBaseCpp& getBaseRef()
    {
        return m_cBase;
    };

    inline MyMJGameRoleTypeCpp getRole() const
    {
        return m_cAccessor.getAccessRoleType();
    };

    inline FMyMJDataAccessorCpp& getAccessorRef()
    {
        return m_cAccessor;
    };

    inline const FMyMJDataAccessorCpp& getAccessorRefConst() const
    {
        return m_cAccessor;
    };


protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data base"))
    FMyMJDataStructWithTimeStampBaseCpp m_cBase;

    //helper to let it wait a while and then allow to apply next event
    uint32 m_uiMinServerWorldTimeForNextEvent_ms;

    FMyMJDataAccessorCpp m_cAccessor;
};
*/

/*
//Different game types have different datas, for a turn based one, history is important
//Todo: make it template
//it is like a queue with limited size, but don't need allocate memory, thread unsafe
USTRUCT()
struct FMyMJDataAtOneMomentCycleBufferCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJDataAtOneMomentCycleBufferCpp()
    {
        m_iSizeOfBuffer = 64;
        m_aDatas.AddZeroed(m_iSizeOfBuffer);

        MY_VERIFY(m_iSizeOfBuffer > 0);

        clear();
    };

    void clear()
    {
        m_idxHead = -1;
        m_iCount = 0;
    };

    void addToTail(const FMyMJDataAtOneMomentCpp& elem)
    {
        m_idxHead = (m_idxHead + 1) % m_iSizeOfBuffer;
        m_aDatas[m_idxHead] = elem;

        m_iCount++;
        if (m_iCount > m_iSizeOfBuffer) {
            m_iCount = m_iSizeOfBuffer;
        }
    };

    //This will assert if none was removed
    void removeFromHead()
    {
        if (m_iCount > 0) {
            m_idxHead = (m_idxHead + 1) % m_iSizeOfBuffer;
            m_iCount--;
        }
        else {
            MY_VERIFY(false);
        }
    };

    int32 getCount() const
    {
        return m_iCount;
    };

    //This will assert if out of range
    const FMyMJDataAtOneMomentCpp& peekRefAt(int32 idxFromHead)
    {
        MY_VERIFY(idxFromHead < m_iCount);
        const FMyMJDataAtOneMomentCpp& ret = m_aDatas[idxFromHead + idxFromHead];

        //extra check
        uint32 time_ms = ret.getLastEventEndTime();
        if (time_ms <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem at %d have timestamp %d, not valid! internal state: m_idxHead %d, m_iCount %d."), idxFromHead, time_ms, m_idxHead, m_iCount);
            MY_VERIFY(false);
        }

        return ret;
    };

protected:
    TArray<FMyMJDataAtOneMomentCpp> m_aDatas;
    int32 m_iSizeOfBuffer;

    int32 m_idxHead;
    int32 m_iCount;
};

//this is the data used to reflect the current state
UCLASS()
class MYONLINECARDGAME_API UMyMJDataAtOneMomentPerRoleCpp : public UObject
{
    GENERATED_BODY()

public:
    UMyMJDataAtOneMomentPerRoleCpp()
    {
        m_cAccessor.setupTempMode(&m_cData.m_cBase);
        m_iRepKeyOfState = 1;
    };

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    void markDirtyForRep()
    {
        m_iRepKeyOfState++;
        if (m_iRepKeyOfState <= 0) {
            m_iRepKeyOfState = 1;
        }
    };

    int32 getRepKeyOfState() const
    {
        return m_iRepKeyOfState;
    };
    
    void applyPusherResult(const FMyMJGamePusherResultCpp& pusherResult, uint32 uiServerWorldTime_resolved_ms);

    UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "data"))
    FMyMJDataAtOneMomentCpp m_cData;

protected:

    FMyMJDataAccessorCpp m_cAccessor;

    int32 m_iRepKeyOfState;
};
*/

#define MY_ACTOR_CHANNEL_OBJ_ID_GROUP_RANGE 100

//this tell the the state of game core in logic level
UCLASS()
class MYONLINECARDGAME_API UMyMJDataAllCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJDataAllCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual ~UMyMJDataAllCpp();

    //virtual void PostInitProperties() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    void doTestChange()
    {
        m_iTest += 2;

        int32 l = m_aDatas.Num();
        for (int32 i = 0; i < l; i++) {
            if (IsValid(m_aDatas[i])) {
                m_aDatas[i]->doTestChange();
            }
        }

        markDirtyForRep();
    };

    FString genDebugMsg()
    {
        int32 l = m_aDatas.Num();
        FString ret = FString::Printf(TEXT("m_aDatas l: %d, m_iTest %d. "), l, m_iTest);
        for (int32 i = 0; i < l; i++) {
            ret += FString::Printf(TEXT(" elem %d: "), i);
            if (IsValid(m_aDatas[i])) {
                ret += m_aDatas[i]->genDebugMsg();
            }
            else {
                ret += FString::Printf(TEXT(" Null."), l);
            }
        }
        return ret;

    };

    //Can't be called on deconstructor since it access another UObject*
    void clearInGame();

    void createSubObjects(bool bInConstructor);

    void setSubobjectBehaviors(int32 iFullDataRecordType, bool bHelperProduceMode)
    {
        for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++)
        {
            UMyMJDataSequencePerRoleCpp* pSeq = m_aDatas[i];
            if (!IsValid(pSeq)) {
                continue;
            }

            pSeq->setFullDataRecordType(iFullDataRecordType);
            pSeq->setHelperProduceMode(bHelperProduceMode);
        }
    };

    void markDirtyForRep()
    {
        m_iRepKeyOfState++;
        if (m_iRepKeyOfState <= 0) {
            m_iRepKeyOfState = 1;
        }
    };

    void markAllDirtyForRep()
    {
        int32 l = m_aDatas.Num();
        for (int32 i = 0; i < l; i++) {
            if (IsValid(m_aDatas[i])) {
                m_aDatas[i]->markDirtyForRep();
                MY_VERIFY(m_aDatas[i]->getDeltaDataEvents(true)->verifyData(false) == 0);
            }
        }
        markDirtyForRep();
    };

    inline
    bool isReadyToGiveNextEvent(uint32 uiServerWorldTime_ms) const
    {
        MY_VERIFY(m_aDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        MY_VERIFY(IsValid(m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper]));
        return m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper]->isReadyToGiveNextEvent(uiServerWorldTime_ms);
    };

    void addPusherResult(const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_ms, bool *pOutNeedReboot);

    inline
    UMyMJDataSequencePerRoleCpp* getDataByRoleType(MyMJGameRoleTypeCpp eRoleType, bool bVerify = true)
    {
        return const_cast<UMyMJDataSequencePerRoleCpp *>(getDataByRoleTypeConst(eRoleType, bVerify));
    };

    const UMyMJDataSequencePerRoleCpp* getDataByRoleTypeConst(MyMJGameRoleTypeCpp eRoleType, bool bVerify = true) const
    {
        MY_VERIFY((uint8)eRoleType < (uint8)MyMJGameRoleTypeCpp::Max);
        int32 l = m_aDatas.Num();
        if (l != (uint8)MyMJGameRoleTypeCpp::Max) {
            if (bVerify) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_aDatas.Num() is %d."), l);
                MY_VERIFY(false);
            }
        }

        if ((uint8)eRoleType < l) {
            if (IsValid(m_aDatas[(uint8)eRoleType])) {
                return m_aDatas[(uint8)eRoleType];
            }
        }

        return NULL;
    };

    inline
    void setShowDebugLog(bool bShow)
    {
        m_bShowDebugLog = bShow;
    }

    inline uint32 getServerWorldTime_ms() const
    {
        int32 l = m_aDatas.Num();
        int32 targetIdx = (uint8)MyMJGameRoleTypeCpp::SysKeeper;
        if (targetIdx >= l) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_aDatas's length is not correct: %d."), l);
            return 0;
        }

        UMyMJDataSequencePerRoleCpp* pSeq = m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper];
        if (IsValid(pSeq)) {
            return pSeq->getServerWorldTime_ms();
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("syskeeper's seq is not valid!, %p"), pSeq);
            return 0;
        }
    };

    inline void setServerWorldTime_ms(uint32 uiServerWorldTime_ms)
    {

        int32 l = m_aDatas.Num();
        for (int32 i = 0; i < l; i++) {
            UMyMJDataSequencePerRoleCpp* pSeq = m_aDatas[i];
            if (IsValid(pSeq)) {
                pSeq->setServerWorldTime_ms(uiServerWorldTime_ms);
            }
        }
    };

    void updateDebugInfo(float fWorldRealTimeNow, uint32 uiIdEventBefore);

    UPROPERTY(Replicated)
    int32 m_iTest;

protected:

    UFUNCTION()
    void OnRep_Datas()
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_Datas()"));

        int32 l = m_aDatas.Num();
        for (int32 i = 0; i < l; i++) {
            UMyMJDataSequencePerRoleCpp* pSeq = m_aDatas[i];
            if (!IsValid(pSeq)) {
                continue;
            }

            //pSeq->m_cReplicateDelegate.Clear();
            //pSeq->m_cReplicateDelegate.AddUObject(this, &UMyMJDataAllCpp::onDataSeqReplicated);
        }
    }


    //basically we can make visual and logic unified by ignoring old status, but turn based game, history is also important, so we divide them, logic tells the latest state,
    //visual tells what shows to player now

    //basically we have two ways to sync, one is property Replication, one is RPC, here is implemention of 1st method
    //this is the real logic data
    //Memory is cheap, let's clone each
    //this is the logic data

    //UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "datas"))
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "datas"))
    TArray<UMyMJDataSequencePerRoleCpp *> m_aDatas;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "test Seq"))
    //UMyMJDataSequencePerRoleCpp *m_pTestSeq;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "test Buffer"))
    //UMyMJGameEventCycleBuffer *m_pTestBuffer;

    FMyMJCoreRelatedEventCorePusherCfgCpp m_cEventCorePusherCfg;

    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "datas"))
    //TArray<UMyMJDataAtOneMomentPerRoleCpp *> m_aDatas;



    //replication helper, let's assume range is 100
    int32 m_iRepObjIdBase;
    int32 m_iRepKeyOfState;

    bool m_bShowDebugLog;

    float m_fDebugSupposedReplicationUpdateLastRealTime;
    uint32 m_uiDebugSupposedReplicationUpdateLastIdEvent;
};