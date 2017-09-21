// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MJ/MyMJGameAttender.h"

//#include "GameFramework/Actor.h"
//#include "UnrealNetwork.h"

#include "MyMJGameEventBase.generated.h"

//16ms is precise enough
#define MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(dataTime)   ((dataTime) << 5)
#define MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(worldTime_ms) ((worldTime_ms) >> 5)

#define MY_MJ_GAME_WORLD_TIME_MS_RESOLUTION MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(1)

#define MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(worldTime_ms) (MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(worldTime_ms)))

#define MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(worldTime_ms) (MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(worldTime_ms) == (worldTime_ms))


//every actor can have it's own state about time
USTRUCT()
struct FMyMJServerClientTimeBondCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJServerClientTimeBondCpp()
    {
        clearBond();
    };

    bool rebondTime(uint32 uiServerTime_data_unit, uint32 uiClientTime_data_uint)
    {
        if (uiServerTime_data_unit == 0 && uiClientTime_data_uint == 0) {
            return false;
        }

        m_uiServerTime_data_unit = uiServerTime_data_unit;
        m_uiClientTime_data_uint = uiClientTime_data_uint;

        return true;
    };

    void clearBond()
    {
        m_uiServerTime_data_unit = 0;
        m_uiClientTime_data_uint = 0;
    };

    inline
    bool haveBond() const
    {
        return (m_uiServerTime_data_unit > 0 || m_uiClientTime_data_uint > 0);
    };

    //in most case, we only care about the calculated server time the actor is on
    uint32 getCalculatedServerTime_data_unit(uint32 uiNewClientTime_data_uint) const
    {
        MY_VERIFY(haveBond());

        return uiNewClientTime_data_uint - m_uiClientTime_data_uint + m_uiServerTime_data_unit;
    };

    uint32 m_uiServerTime_data_unit;
    uint32 m_uiClientTime_data_uint;
};

//uint is MS
USTRUCT()
struct FMyMJEventDataDeltaDurCfgBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJEventDataDeltaDurCfgBaseCpp();

    //return uint is ms, resolved
    uint32 helperGetDeltaDur(const FMyMJDataDeltaCpp& delta) const;


    uint32 m_uiGameStarted;

    uint32 m_uiThrowDiceForDistCardsAtStart;

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

USTRUCT(BlueprintType)
struct FMyMJDataStructWithTimeStampBaseCpp : public FMyMJDataStructCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJDataStructWithTimeStampBaseCpp() : Super()
    {
        m_uiTime_data_unit = 0;
    };

    void reset()
    {
        Super::reset();
        m_uiTime_data_unit = 0;
    };

    //unit is ms
    void setTime(uint32 uiTime_ms)
    {
        MY_VERIFY(uiTime_ms > 0);
        if (!MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiTime_ms)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time_ms is not resolved: %d."), uiTime_ms);
            MY_VERIFY(false);
        }
        m_uiTime_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(uiTime_ms);
    };

    //unit is ms
    uint32 getTime() const
    {
        return MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiTime_data_unit);
    };

    void copyWithRoleFromSysKeeperRole2(MyMJGameRoleTypeCpp eTargetRole, FMyMJDataStructWithTimeStampBaseCpp& cTargetData) const
    {
        Super::copyWithRoleFromSysKeeperRole(eTargetRole, cTargetData);
        cTargetData.m_uiTime_data_unit = m_uiTime_data_unit;
    };

protected:

    UPROPERTY(meta = (DisplayName = "state server world time stamp"))
    uint32 m_uiTime_data_unit;
};

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
        m_aPusherResult.Reset();
        m_uiStartTime_data_unit = 0;
        m_uiDuration_data_unit = 0;

        m_iIdxDebug = 0;
    };

    //unit is ms
    uint32 getEndTime() const
    {
        return MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiStartTime_data_unit + m_uiDuration_data_unit);
    }

    void setStartTime(uint32 uiStartTime_resolved_ms)
    {
        MY_VERIFY(uiStartTime_resolved_ms > 0);
        MY_VERIFY(MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiStartTime_resolved_ms));
        m_uiStartTime_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(uiStartTime_resolved_ms);
    };

    //unit is ms, resolved
    uint32 getStartTime() const
    {
        return MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiStartTime_data_unit);
    };

    void setDuration(uint32 uiDuration_resolved_ms)
    {
        MY_VERIFY(MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiDuration_resolved_ms));
        m_uiDuration_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(uiDuration_resolved_ms);
    };

    //unit is ms, resolved
    uint32 getDuration() const
    {
        return MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiDuration_data_unit);
    };

    void setPusherResult(const FMyMJGamePusherResultCpp& pusherResult)
    {
        MY_VERIFY(m_aPusherResult.Num() <= 0);
        int32 idx = m_aPusherResult.Emplace();
        m_aPusherResult[idx] = pusherResult;
    };

    const FMyMJGamePusherResultCpp* getPusherResult(bool bAssertValid) const
    {
        if (m_aPusherResult.Num() > 0) {
            return &m_aPusherResult[0];
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
        cTargetData.m_aPusherResult.Reset();
        int l = m_aPusherResult.Num();
        MY_VERIFY(l <= 1);
        if (l > 0) {
            cTargetData.m_aPusherResult.Emplace();
            m_aPusherResult[0].copyWithRoleFromSysKeeperRole(eTargetRole, cTargetData.m_aPusherResult[0]);
        }


        cTargetData.m_uiStartTime_data_unit = m_uiStartTime_data_unit;
        cTargetData.m_uiDuration_data_unit  = m_uiDuration_data_unit;
    };

    // Optional: debug string used with LogNetFastTArray logging
    FString GetDebugString()
    {
        return FString::Printf(TEXT("FMyMJEventWithTimeStampBaseCpp idxDebug %d."), m_iIdxDebug);
    };

    bool canBeAppendedToPrev(int32 iGameIdPrev, int32 iPusherIdLastPrev, bool bIsGameEndOrNotStartedPrev) const
    {


        int32 iGameIdFirst = -1;
        int32 iPusherIdFirst = -1;
        const FMyMJEventWithTimeStampBaseCpp* pItemFirst = this;

        iGameIdFirst = pItemFirst->getPusherResult(true)->getGameId();
        iPusherIdFirst = pItemFirst->getPusherResult(true)->getPusherIdLast();

        bool bRet;
        if (iGameIdPrev < 0) {
            bRet = pItemFirst->getPusherResult(true)->m_aResultBase.Num() > 0;
        }
        else {
            if (iGameIdPrev == iGameIdFirst) {
                bRet = (iPusherIdLastPrev + 1) == iPusherIdFirst;
            }
            else {
                bRet = bIsGameEndOrNotStartedPrev && pItemFirst->getPusherResult(true)->m_aResultBase.Num() > 0;
            }
        }

        if (!bRet) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("canBeAppendedToPrev false: iGameIdPrev %d, iPusherIdLastPrev %d, bIsGameEndOrNotStartedPrev %d, iGameIdFirst %d, iPusherIdFirst %d."),
                iGameIdPrev, iPusherIdLastPrev, bIsGameEndOrNotStartedPrev, iGameIdFirst, iPusherIdFirst);
        }

        return bRet;
    }

    UPROPERTY()
        int32 m_iIdxDebug;

    FString genDebugMsg() const
    {
        uint32 uiStartMs = MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiStartTime_data_unit);
        uint32 uiDurMs = MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiDuration_data_unit);
        FString resultStr = TEXT("null");
        if (m_aPusherResult.Num() > 0) {
            resultStr = m_aPusherResult[0].genDebugMsg();
        }
        return FString::Printf(TEXT("%d.%03d, %d.%03d, %s"), uiStartMs/1000, uiStartMs%1000, uiDurMs/1000, uiDurMs%1000, *resultStr);

    };

protected:

    //actually union, if valid, Num() == 1 and it is a delta pusher
    //TArray<FMyMJGamePusherResultCpp>  m_aPusherResult;
    UPROPERTY()
    TArray<FMyMJGamePusherResultCpp>  m_aPusherResult;

    //timestamp is from server world time
    UPROPERTY(meta = (DisplayName = "state server world time stamp"))
        uint32 m_uiStartTime_data_unit;

    //
    UPROPERTY(meta = (DisplayName = "time wait for animation"))
        uint32 m_uiDuration_data_unit;


};

USTRUCT()
struct FMyMJGameEventArray : public FFastArraySerializer
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventArray() : Super()
    {

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


DECLARE_MULTICAST_DELEGATE_OneParam(FMyMJGameEventCycleBufferReplicatedDelegate, int32);

//Todo: make it template
//Different game types have different datas, for a turn based one, history is important
//it is like a queue with limited size, but don't need allocate memory, thread unsafe
//It seems fast tarray replication, must be directly touched to a uobject!
UCLASS()
class UMyMJGameEventCycleBuffer : public UObject
{
    GENERATED_BODY()

public:

    UMyMJGameEventCycleBuffer()
    {
        m_iTest = 0;
        resize(64);
    };

    virtual ~UMyMJGameEventCycleBuffer()
    {


    };

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    void resize(int32 iNewSizeMax)
    {
        m_iSizeMax = iNewSizeMax;
        MY_VERIFY(m_iSizeMax > 0);

        //m_cEventArray.Items.AddZeroed(m_iSizeMax);

        clear();
    };

    void clear()
    {
        m_idxHead = -1;
        m_iCount = 0;
        setLastEventEndTime(0);
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
    };

    int32 getCount(bool *pOutIsFull) const
    {
        if (pOutIsFull) {
            *pOutIsFull = m_iCount >= m_iSizeMax;
        }

        return m_iCount;
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
        uint32 time_ms = ret.getEndTime();
        if (time_ms <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem at %d have timestamp %d, not valid! internal state: m_idxHead %d, m_iCount %d."), idxFromHead, time_ms, m_idxHead, m_iCount);
            //MY_VERIFY(false);
        }

        if (bVerify) {
            if (ret.m_iIdxDebug != idxFound) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem idx not equal, idxFromHead %d, idxFound %d, m_iIdxDebug %d."), idxFromHead, idxFound, ret.m_iIdxDebug);
                MY_VERIFY(false);
            }
        }

        return ret;
    };

    const FMyMJEventWithTimeStampBaseCpp* getLast() const
    {
        int32 l = getCount(NULL);
        if (l > 0) {
            return &peekRefAt(l - 1);
        }
        else {
            return NULL;
        }
    };


    //only event that have duration records time, otherwise consdier them should be applied instantly
    //all calc here must be resolved to resolution
    //will assert if buffer is full
    FMyMJEventWithTimeStampBaseCpp& addToTailWhenNotFull(uint32 uiTimeStamp_resolved_ms, uint32 uiDur_resolved_ms, bool bForceRecordTime)
    {
        MY_VERIFY(MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiTimeStamp_resolved_ms));
        MY_VERIFY(MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiDur_resolved_ms));

        //int32 idx = m_aEvents.Emplace();
        bool bOverritten = addToTail(NULL);
        MY_VERIFY(!bOverritten);
        FMyMJEventWithTimeStampBaseCpp* pRet = const_cast<FMyMJEventWithTimeStampBaseCpp *>(getLast());
        MY_VERIFY(pRet);
        FMyMJEventWithTimeStampBaseCpp& ret = *pRet;;

        if (bForceRecordTime) {
            ret.setStartTime(uiTimeStamp_resolved_ms);
            setLastEventEndTime(uiTimeStamp_resolved_ms);
        }

        if (uiDur_resolved_ms > 0) {
            ret.setStartTime(uiTimeStamp_resolved_ms);
            ret.setDuration(uiDur_resolved_ms);
            //m_uiLastEventEndTime_10ms = uiTimeStamp_10ms + uiDur_10ms;

            setLastEventEndTime(uiTimeStamp_resolved_ms + uiDur_resolved_ms);

        }
        else {
        }


        return ret;
    };

    //unit is ms, resolved
    uint32 getLastEventEndTime() const
    {
        return MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(m_uiLastEventEndTime_data_unit);
    };

    //return errorCode
    int32 verifyData(bool bShowLog) const
    {
        uint32 timestamp_end = 0;
        int32 l = getCount(NULL);
        int32 iError = 0;
        for (int32 i = 0; i < l; i++) {
            int32 idxInArray = -1;
            const FMyMJEventWithTimeStampBaseCpp& elem = peekRefAt(i, &idxInArray, false);

            if (elem.m_iIdxDebug != idxInArray) {
                //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem's id not equal, elem id %d, real id %d, idx from head %d."), elem.m_iIdxDebug, idxInArray, i);
                //MY_VERIFY(false);

                iError = 1;
                break;
            }

            uint32 te = elem.getEndTime();
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
                const FMyMJGamePusherResultCpp* pResult = elem.getPusherResult(false);
                int32 iGameId = -1;
                int32 iPusherId = -1;
                if (pResult) {
                    iGameId = pResult->getGameId();
                    iPusherId = pResult->getPusherIdLast();
                }
                ret += FString::Printf(TEXT("[%d,%d,%d,%d,%d,%d:%d], "), i, idxInArray, elem.m_iIdxDebug, elem.getStartTime(), elem.getDuration(), iGameId, iPusherId);
            }
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("l %d, iError %d: %s"), l, iError, *ret);
            //MY_VERIFY(false);
        }

        return iError;
    };

    void addItemFromOther(const FMyMJEventWithTimeStampBaseCpp& itemOther)
    {
        uint32 startTime = itemOther.getStartTime();
        uint32 dur = itemOther.getDuration();
        FMyMJEventWithTimeStampBaseCpp& newAdded = addToTailWhenNotFull(startTime, dur, startTime > 0);
        newAdded = itemOther;
    };

    FMyMJGameEventCycleBufferReplicatedDelegate m_cUpdateNotifier;

    UPROPERTY(ReplicatedUsing = OnRep_Data)
    int32 m_iTest;

protected:

    UFUNCTION()
    void OnRep_Data()
    {
        m_cUpdateNotifier.Broadcast(0);
    };

    //return whether buffer overflow and old data is overwriten
    bool addToTail(const FMyMJEventWithTimeStampBaseCpp* itemToSetAs)
    {
        int32 idxNew = -1;
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
  
        m_iCount++;
        if (m_iCount > m_iSizeMax) {
            m_iCount = m_iSizeMax;
            m_idxHead = (idxNew + 1) % m_iSizeMax;
            return true;
        }

        return false;
    };

    void setLastEventEndTime(uint32 uiTimeStamp_resolved_ms)
    {
        MY_VERIFY(MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiTimeStamp_resolved_ms));
        m_uiLastEventEndTime_data_unit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(uiTimeStamp_resolved_ms);
    }

    UPROPERTY(ReplicatedUsing = OnRep_Data)
    FMyMJGameEventArray m_cEventArray;

    UPROPERTY(ReplicatedUsing = OnRep_Data)
    int32 m_iSizeMax;

    //only valid when count > 0
    UPROPERTY(ReplicatedUsing = OnRep_Data)
    int32 m_idxHead;

    UPROPERTY(ReplicatedUsing = OnRep_Data)
    int32 m_iCount;

    UPROPERTY(ReplicatedUsing = OnRep_Data)
    uint32 m_uiLastEventEndTime_data_unit;
};

typedef class UMyMJDataSequencePerRoleCpp UMyMJDataSequencePerRoleCpp;

DECLARE_MULTICAST_DELEGATE_TwoParams(FMyMJDataSeqReplicatedDelegate, UMyMJDataSequencePerRoleCpp*, int32);

//mainly keeps a history, this is the logic core need to replicate
UCLASS()
class MYONLINECARDGAME_API UMyMJDataSequencePerRoleCpp : public UObject
{
    GENERATED_BODY()

public:
    UMyMJDataSequencePerRoleCpp()
    {
        m_iRepKeyOfState = 1;

        m_pEventsApplyingAndApplied = NULL;

        reinit(MyMJGameRoleTypeCpp::Observer);

    };

    void reinit(MyMJGameRoleTypeCpp eRole)
    {
        m_eRole = eRole;
        m_cAccessor.setupTempMode(&m_cBase, eRole);
        clear();
        markDirtyForRep();
    };

    void clear()
    {
        m_cBase.reset();
        if (IsValid(m_pEventsApplyingAndApplied)) {
            m_pEventsApplyingAndApplied->clear();
        }

        m_iHelperGameIdLast = -1;
        m_iHelperPusherIdLast = -1;
        m_iEventsBasePusherCount = 0;
    };

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual void PostInitProperties() override;

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

    //return < 0 means empty, no valid data inside
    int32 getGameIdLast() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = NULL;
        
        if (IsValid(m_pEventsApplyingAndApplied)) {
            pElemLast = m_pEventsApplyingAndApplied->getLast();
        }

        if (pElemLast)
        {
            const FMyMJGamePusherResultCpp* pResult = pElemLast->getPusherResult(true);
            return pResult->getGameId();
        }
        else {
            int32 ret = m_cBase.getCoreDataRefConst().m_iGameId;
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime() > 0); //we presume timestamp must e attached in our user case
            }

            return ret;
        }
    };

    //return < 0 means empty, no valid data inside
    int32 getPusherIdLast() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = NULL;

        if (IsValid(m_pEventsApplyingAndApplied)) {
            pElemLast = m_pEventsApplyingAndApplied->getLast();
        }

        if (pElemLast)
        {
            const FMyMJGamePusherResultCpp* pResult = pElemLast->getPusherResult(true);
            return pResult->getPusherIdLast();
        }
        else {
            int32 ret = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime() > 0); //we presume timestamp must e attached in our user case
            }

            return ret;
        }
    };

    //all data's first id, not first event's id
    int32 getGameIdFirst() const
    {
        if (m_cBase.getCoreDataRefConst().m_iGameId >= 0) {
            return m_cBase.getCoreDataRefConst().m_iGameId;
        }
        else {
            if (IsValid(m_pEventsApplyingAndApplied)) {
                int32 l = m_pEventsApplyingAndApplied->getCount(NULL);
                if (l > 0) {
                    return m_pEventsApplyingAndApplied->peekRefAt(0).getPusherResult(true)->getGameId();
                }
            }

            return -1;

        }
    };

    //all data's first id, not first event's id
    int32 getPusherIdFirst() const
    {
        if (m_cBase.getCoreDataRefConst().m_iPusherIdLast >= 0) {
            return m_cBase.getCoreDataRefConst().m_iPusherIdLast;
        }
        else {
            if (IsValid(m_pEventsApplyingAndApplied)) {
                int32 l = m_pEventsApplyingAndApplied->getCount(NULL);
                if (l > 0) {
                    return m_pEventsApplyingAndApplied->peekRefAt(0).getPusherResult(true)->getPusherIdLast();
                }
            }

            return -1;

        }
    };


    bool isGameEndForLastState() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = NULL;

        if (IsValid(m_pEventsApplyingAndApplied)) {
            pElemLast = m_pEventsApplyingAndApplied->getLast();
        }

        if (pElemLast)
        {
            const FMyMJGamePusherResultCpp* pResult = pElemLast->getPusherResult(true);
            return pResult->isGameEndDelta();
        }
        else {
            int32 ret = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime() > 0); //we presume timestamp must e attached in our user case
                return m_cBase.getCoreDataRefConst().m_eGameState == MyMJGameStateCpp::GameEnd;
            }
        }

        return false;
    };

    //unit is ms, resolved
    uint32 getLastEventEndTime() const;

    bool isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_resolved_ms) const;
    void addPusherResult(const FMyMJEventDataDeltaDurCfgBaseCpp &inEventDeltaDurCfg, const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_resolved_ms);

    //enable merging is good, let client have more chance to buffer data
    //return true if new data merged
    bool mergeDataFromOther(const UMyMJDataSequencePerRoleCpp& other);

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
        if (IsValid(m_pEventsApplyingAndApplied)) {
            m_pEventsApplyingAndApplied->m_iTest++;
        }
    };

    FString genDebugMsg()
    {
        const FMyMJCoreDataPublicCpp& coreData = m_cBase.getCoreDataRefConst();
        //int32 l = m_pEventsApplyingAndApplied->getCount(NULL);
        FString ret = FString::Printf(TEXT("role %d, base time %d [%d:%d:%d:%d], m_pEventsApplyingAndApplied valid %d."), (uint8)m_eRole, m_cBase.getTime(), coreData.m_iGameId, coreData.m_iPusherIdLast, coreData.m_iActionGroupId, (uint8)coreData.m_eGameState, IsValid(m_pEventsApplyingAndApplied));
        if (IsValid(m_pEventsApplyingAndApplied)) {
            int32 l = m_pEventsApplyingAndApplied->getCount(NULL);
            ret += FString::Printf(TEXT("event count %d, event's m_iTest %d."), l, m_pEventsApplyingAndApplied->m_iTest);
        }
        //for (int32 i = 0; i < l; i++) {
            //const FMyMJEventWithTimeStampBaseCpp& e = m_pEventsApplyingAndApplied->peekRefAt(i);
            //ret += FString::Printf(TEXT(" event %d, time start %d, dur %d."), i, e.getStartTime(), e.getDuration());
       // }

        return ret;
    };

    inline const FMyMJDataStructWithTimeStampBaseCpp& getBase() const
    {
        return m_cBase;
    };

    //will assert if out of range
    inline const FMyMJEventWithTimeStampBaseCpp& peekEventRefAt(int32 idx) const
    {
        return m_pEventsApplyingAndApplied->peekRefAt(idx);
    };

    //just like remove, but info goes to base
    void squashEventsToBase(int32 count)
    {
        if (!IsValid(m_pEventsApplyingAndApplied)) {
            return;
        }

        MY_VERIFY(count <= m_pEventsApplyingAndApplied->getCount(NULL));
        for (int32 i = 0; i < count; i++) {
            const FMyMJEventWithTimeStampBaseCpp& cEvent = m_pEventsApplyingAndApplied->peekRefAt(0);
            uint32 uiEndTime = cEvent.getEndTime();

            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's squashing Events, base %d : %d."), (uint8)m_eRole, m_cBase.getCoreDataRefConst().m_iGameId, m_cBase.getCoreDataRefConst().m_iPusherIdLast);

            const FMyMJGamePusherResultCpp* pPusherResult = cEvent.getPusherResult(true);
            m_cAccessor.applyPusherResult(*pPusherResult);

            if (uiEndTime > 0) {
                m_cBase.setTime(uiEndTime);
            }

            if (pPusherResult->m_aResultBase.Num() > 0) {
                m_iEventsBasePusherCount--;
            }

            m_pEventsApplyingAndApplied->removeFromHead(1);
        }

        //m_cBaseTest2 = m_cBase;
    };

    inline
    int32 getEventCount() const
    {
        if (!IsValid(m_pEventsApplyingAndApplied)) {
            MY_VERIFY(false);
        }
        return m_pEventsApplyingAndApplied->getCount(NULL);
    };

    inline
    void resizeEvents(int32 newSize)
    {
        m_cBase.reset();
        if (IsValid(m_pEventsApplyingAndApplied)) {
            m_pEventsApplyingAndApplied->resize(newSize);
        }
        markDirtyForRep();
    }

    inline
    const UMyMJGameEventCycleBuffer& getEventsRef() const
    {
        return *getEvents(true);
    };

    inline 
        const UMyMJGameEventCycleBuffer* getEvents(bool bVerifyValid) const
    {

        if (IsValid(m_pEventsApplyingAndApplied)) {
            return m_pEventsApplyingAndApplied;
        }
        else {
            if (bVerifyValid) {
                MY_VERIFY(false);
            }
            return false;
        }

    };

    FMyMJDataSeqReplicatedDelegate m_cReplicateDelegate;

    UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "role"))
        MyMJGameRoleTypeCpp m_eRole;

protected:

    //make the state move forward
    void trySquashBaseAndEvents(uint32 uiServerWorldTime_resolved_ms);

    UFUNCTION()
        void OnRep_Base()
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's base replicated,  %d : %d."), (uint8)m_eRole, m_cBase.getCoreDataRefConst().m_iGameId, m_cBase.getCoreDataRefConst().m_iPusherIdLast);

        m_cReplicateDelegate.Broadcast(this, 0);
    };

    UFUNCTION()
    void OnRep_EventsPointer()
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's events replicated."), (uint8)m_eRole);
        m_cReplicateDelegate.Broadcast(this, 1);

        if (IsValid(m_pEventsApplyingAndApplied)) {
            m_pEventsApplyingAndApplied->m_cUpdateNotifier.Clear();
            m_pEventsApplyingAndApplied->m_cUpdateNotifier.AddUObject(this, &UMyMJDataSequencePerRoleCpp::OnRep_EventsContent);
        }

    };

    UFUNCTION()
    void OnRep_EventsContent(int32 iExtra)
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's events replicated."), (uint8)m_eRole);
        m_cReplicateDelegate.Broadcast(this, 2);
    };

    //following is the core data, representing

    //driven by @m_cEventsApplyingAndApplied
    UPROPERTY(ReplicatedUsing = OnRep_Base)
        FMyMJDataStructWithTimeStampBaseCpp m_cBase;

    //UPROPERTY(Replicated)
    //FMyMJDataStructWithTimeStampBaseCpp m_cBaseTest2;

    //focused deltas, base is only used when merge or pull
    UPROPERTY(ReplicatedUsing = OnRep_EventsPointer)
    //UPROPERTY()
    UMyMJGameEventCycleBuffer* m_pEventsApplyingAndApplied;



    //The base state of deltas(time of start, not end), since the delta is one way to apply(can not revert)
    //we never store more than one base, this means when base change, all of its previouse delta data will be cleared!
    //UMyMJDataForMirrorModeCpp *m_pBase;
    FMyMJDataAccessorCpp m_cAccessor;

    int32 m_iRepKeyOfState;

    int32 m_iHelperGameIdLast;
    int32 m_iHelperPusherIdLast;
    int32 m_iEventsBasePusherCount;
};

#define FMyMJDataAtOneMomentCpp_eventApplyWay_Normal      (0)
#define FMyMJDataAtOneMomentCpp_eventApplyWay_ForApplyNow (1)
#define FMyMJDataAtOneMomentCpp_eventApplyWay_ApplyAhead  (2)

USTRUCT(BlueprintType)
struct FMyMJDataAtOneMomentCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataAtOneMomentCpp()
    {
        //m_eRole = MyMJGameRoleTypeCpp::Max;
        reinit(MyMJGameRoleTypeCpp::Observer);
    };

    void reinit(MyMJGameRoleTypeCpp eRole)
    {
        m_eRole = eRole;
        m_cAccessor.setupTempMode(&m_cBase, eRole);
        clear();
    };

    void clear()
    {
        m_cBase.reset();
        m_aEventApplying.Reset();
        m_bEventApplyingAppliedAhead = false;
    };

    //return < 0 means empty, no valid data inside
    int32 getGameIdLast() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = NULL;
        if (m_aEventApplying.Num() > 0) {
            pElemLast = &m_aEventApplying[0];
        }

        if (pElemLast)
        {
            const FMyMJGamePusherResultCpp* pResult = pElemLast->getPusherResult(true);
            return pResult->getGameId();
        }
        else {
            int32 ret = m_cBase.getCoreDataRefConst().m_iGameId;
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime() > 0); //we presume timestamp must e attached in our user case
            }

            return ret;
        }
    };

    //return < 0 means empty, no valid data inside
    int32 getPusherIdLast() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = NULL;
        if (m_aEventApplying.Num() > 0) {
            pElemLast = &m_aEventApplying[0];
        }

        if (pElemLast)
        {
            const FMyMJGamePusherResultCpp* pResult = pElemLast->getPusherResult(true);
            return pResult->getPusherIdLast();
        }
        else {
            int32 ret = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
            if (ret >= 0) {
                MY_VERIFY(m_cBase.getTime() > 0); //we presume timestamp must e attached in our user case
            }

            return ret;
        }
    };

    void doTestChange()
    {
        m_cBase.setTime(m_cBase.getTime() + MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(1000));
        if (m_aEventApplying.Num() > 0) {
            m_aEventApplying.Reset();
        }
        else {
            int32 idx = m_aEventApplying.Emplace();
            m_aEventApplying[idx].setStartTime(MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(5000));
            m_aEventApplying[idx].setDuration(MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(1500));
        }
    };

    FString genDebugMsg()
    {
        FString ret = FString::Printf(TEXT("role %d, base time %d, event applying Num %d."), (uint8)m_eRole, m_cBase.getTime(), m_aEventApplying.Num());
        if (m_aEventApplying.Num() > 0) {
            ret += FString::Printf(TEXT(" event time start %d, dur %d."), m_aEventApplying[0].getStartTime(), m_aEventApplying[0].getDuration());
        }

        return ret;
    };


    //unit is ms, resolved
    uint32 getLastEventEndTime() const
    {
        uint32 uiTimeBase = m_cBase.getTime();
        uint32 uiTimeDeltas = 0;
        if (m_aEventApplying.Num() > 0) {
            m_aEventApplying[0].getEndTime();
        }

        if (uiTimeDeltas > 0) {
            //if you have delta
            if (uiTimeDeltas < uiTimeBase) {
                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("timestamp chaos: base %d, delta %d."), uiTimeBase, uiTimeDeltas);
                MY_VERIFY(false);
            }
        }

        return uiTimeBase > uiTimeDeltas ? uiTimeBase : uiTimeDeltas;
    };


    bool isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_resolved_ms) const
    {
        if (uiServerWorldTime_resolved_ms == 0) {
            return false;
        }

        bool bRet = uiServerWorldTime_resolved_ms >= getLastEventEndTime(); //use >= to allow apply multiple one time

        return bRet;
    };

    //@applyWay: 0 
    void applyEvent(const FMyMJEventWithTimeStampBaseCpp& cEvent, int32 applyWay)
    {
        prepareFOrNextEvent();

        bool bApplyNow = true;
        if (cEvent.getDuration() > 0) {

            if (applyWay == FMyMJDataAtOneMomentCpp_eventApplyWay_Normal) {
                int32 idx = m_aEventApplying.Emplace();
                MY_VERIFY(idx == 0);
                m_aEventApplying[idx] = cEvent;

                m_bEventApplyingAppliedAhead = false;
                bApplyNow = false;
            }
            else if (applyWay == FMyMJDataAtOneMomentCpp_eventApplyWay_ForApplyNow)
            {

            }
            else if (applyWay == FMyMJDataAtOneMomentCpp_eventApplyWay_ApplyAhead)
            {
                int32 idx = m_aEventApplying.Emplace();
                MY_VERIFY(idx == 0);
                m_aEventApplying[idx] = cEvent;

                m_bEventApplyingAppliedAhead = true;
                bApplyNow = true;
            }
            else {
                MY_VERIFY(false);
            }

        }


        if (bApplyNow) {
            m_cAccessor.applyPusherResult(*cEvent.getPusherResult(true));
            uint32 uiEndTime = cEvent.getEndTime();
            if (uiEndTime > 0) {
                m_cBase.setTime(uiEndTime);
            }
        }

    };

    void resetWithBase(const FMyMJDataStructWithTimeStampBaseCpp& cBase)
    {
        clear();
        m_cBase = cBase;
    };

    inline
    bool isGameEndOrNotStarted() const
    {
        bool bNotStated = getGameIdLast() < 0;
        bool bGameEnd = m_cBase.getCoreDataRefConst().m_eGameState == MyMJGameStateCpp::GameEnd || (m_aEventApplying.Num() > 0 && m_aEventApplying[0].getPusherResult(true)->isGameEndDelta());
        return bNotStated || bGameEnd;
    };

    FMyMJServerClientTimeBondCpp m_cTimeBond;

protected:

    void prepareFOrNextEvent()
    {
        if (!m_bEventApplyingAppliedAhead && m_aEventApplying.Num() > 0) {
            MY_VERIFY(m_aEventApplying.Num() == 1);

            const FMyMJEventWithTimeStampBaseCpp& cEvent = m_aEventApplying[0];
            m_cAccessor.applyPusherResult(*cEvent.getPusherResult(true));

            uint32 uiEndTime = cEvent.getEndTime();
            if (uiEndTime > 0) {
                m_cBase.setTime(uiEndTime);
            }

            m_aEventApplying.Reset();
            m_bEventApplyingAppliedAhead = false;
        }
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data base"))
    FMyMJDataStructWithTimeStampBaseCpp m_cBase;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data delta applying"))
    TArray<FMyMJEventWithTimeStampBaseCpp> m_aEventApplying;

    //It is possible we apply the event before timestamp, to let visualization easior
    bool m_bEventApplyingAppliedAhead;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role"))
    MyMJGameRoleTypeCpp m_eRole;



    FMyMJDataAccessorCpp m_cAccessor;
};

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

    //we don't create members by default, since in client replication will do it, saves a memory allocate operation
    UMyMJDataAllCpp()
    {
        m_iTest = 0;
        m_iRepObjIdBase = 200;
        m_iRepKeyOfState = 1;

        m_pDataTest0 = m_pDataTest1 = NULL;
    };

    virtual void PostInitProperties() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    void doTestChange()
    {
        /*
        int32 l = m_aDatas.Num();
        for (int32 i = 0; i < l; i++) {
            if (m_aDatas[i]) {
                m_aDatas[i]->doTestChange();
                //test purpse:
                if (i != 2) {
                    m_aDatas[i]->markDirtyForRep();
                }
            }
        }
        */

        m_iTest += 2;

        int32 l = m_aDatas.Num();
        for (int32 i = 0; i < l; i++) {
            if (IsValid(m_aDatas[i])) {
                m_aDatas[i]->doTestChange();
            }
        }

        if (IsValid(m_pDataTest0)) {
            m_pDataTest0->doTestChange();
        }

        if (IsValid(m_pDataTest1)) {
            m_pDataTest1->doTestChange();
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

        ret += FString::Printf(TEXT(" m_pDataTest0 %d: "), IsValid(m_pDataTest0));
        if (IsValid(m_pDataTest0)) {
            ret += m_pDataTest0->genDebugMsg();
        }

        ret += FString::Printf(TEXT(" m_pDataTest1 %d: "), IsValid(m_pDataTest1));
        if (IsValid(m_pDataTest1)) {
            ret += m_pDataTest1->genDebugMsg();
        }

        return ret;

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
                MY_VERIFY(m_aDatas[i]->getEventsRef().verifyData(false) == 0);
            }
        }
        markDirtyForRep();
    };

    inline
    bool isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_resolved_ms) const
    {
        MY_VERIFY(m_aDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        MY_VERIFY(IsValid(m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper]));
        return m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper]->isReadyToGiveNextPusherResult(uiServerWorldTime_resolved_ms);
    };

    //void applyPusherResult(const FMyMJGamePusherResultCpp& pusherResult, uint32 uiServerWorldTime_resolved_ms)

    //inline
    //void givePusherResult(FMyMJGamePusherResultCpp **ppPusherResult, uint32 uiServerWorldTime_resolved_ms)

    void addPusherResult(const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_resolved_ms)
    {
        int32 l = m_aDatas.Num();
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);
        FMyMJGamePusherResultCpp cPusherNew;
        for (int32 i = 0; i < l; i++) {
            if (i == (uint8)MyMJGameRoleTypeCpp::SysKeeper) {
                MY_VERIFY(IsValid(m_aDatas[i]));
                m_aDatas[i]->addPusherResult(m_cEventDeltaDurCfg, cPusherResult, uiServerWorldTime_resolved_ms);
            }
            else {
                MyMJGameRoleTypeCpp eRole = (MyMJGameRoleTypeCpp)i;
                if (!IsValid(m_aDatas[i])) {
                    continue;
                }
                MY_VERIFY(m_aDatas[i]->m_eRole == eRole);

                cPusherResult.copyWithRoleFromSysKeeperRole(eRole, cPusherNew);
                m_aDatas[i]->addPusherResult(m_cEventDeltaDurCfg, cPusherNew, uiServerWorldTime_resolved_ms);
            }
        }

        if (IsValid(m_pDataTest0)) {
            m_pDataTest0->addPusherResult(m_cEventDeltaDurCfg, cPusherResult, uiServerWorldTime_resolved_ms);
        }

        if (IsValid(m_pDataTest1)) {
            m_pDataTest1->addPusherResult(m_cEventDeltaDurCfg, cPusherResult, uiServerWorldTime_resolved_ms);
        }
    };

    /*
    void cloneDataForAllRoles()
    {
        int32 l = m_aDatas.Num();
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);

        for (int32 i = 0; i < l; i++) {
            if (i == (uint8)MyMJGameRoleTypeCpp::SysKeeper) {
                continue;
            }
            m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper]->m_cData.copyWithRoleFromSysKeeperRole(m_aDatas[i]->m_cData);
        }
    };
    */

    UMyMJDataSequencePerRoleCpp* getDataByRoleType(MyMJGameRoleTypeCpp eRoleType, bool bVerify = true)
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

    FMyMJDataSeqReplicatedDelegate m_cReplicateDelegate;

    UPROPERTY(Replicated)
    int32 m_iTest;

    //UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "dataTest"))
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "dataTest0"))
    UMyMJDataSequencePerRoleCpp *m_pDataTest0;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "dataTest1"))
    UMyMJDataSequencePerRoleCpp *m_pDataTest1;

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

            pSeq->m_cReplicateDelegate.Clear();
            pSeq->m_cReplicateDelegate.AddUObject(this, &UMyMJDataAllCpp::onDataSeqReplicated);
        }

        if (IsValid(m_pDataTest0)) {
            m_pDataTest0->m_cReplicateDelegate.Clear();
            m_pDataTest0->m_cReplicateDelegate.AddUObject(this, &UMyMJDataAllCpp::onDataSeqReplicated);
        }

        if (IsValid(m_pDataTest1)) {
            m_pDataTest1->m_cReplicateDelegate.Clear();
            m_pDataTest1->m_cReplicateDelegate.AddUObject(this, &UMyMJDataAllCpp::onDataSeqReplicated);
        }
    }

    void onDataSeqReplicated(UMyMJDataSequencePerRoleCpp *pSeq, int32 iExtra)
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("onDataSeqReplicated(), role %d."), (uint8)pSeq->m_eRole);
        m_cReplicateDelegate.Broadcast(pSeq, iExtra);
    };

    void createSubObjects()
    {
        m_aDatas.Reset();
        for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            if (i == (uint8)MyMJGameRoleTypeCpp::SysKeeper || i == 5) {
                UMyMJDataSequencePerRoleCpp *pNew = NewObject<UMyMJDataSequencePerRoleCpp>(this);
                pNew->reinit((MyMJGameRoleTypeCpp)i);
                m_aDatas.Emplace(pNew);
            }
            else {
                m_aDatas.Emplace((UMyMJDataSequencePerRoleCpp *)NULL);
            }
        }

        //m_pDataTest0 = NewObject<UMyMJDataSequencePerRoleCpp>(this);
        //m_pDataTest0->reinit(MyMJGameRoleTypeCpp::SysKeeper);

        //m_pDataTest1 = NewObject<UMyMJDataSequencePerRoleCpp>(this);
        //m_pDataTest1->reinit(MyMJGameRoleTypeCpp::SysKeeper);
    };

    //basically we can make visual and logic unified by ignoring old status, but turn based game, history is also important, so we divide them, logic tells the latest state,
    //visual tells what shows to player now

    //basically we have two ways to sync, one is property Replication, one is RPC, here is implemention of 1st method
    //this is the real logic data
    //Memory is cheap, let's clone each
    //this is the logic data

    //UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "datas"))
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Datas, meta = (DisplayName = "datas"))
    TArray<UMyMJDataSequencePerRoleCpp *> m_aDatas;



    FMyMJEventDataDeltaDurCfgBaseCpp m_cEventDeltaDurCfg;

    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "datas"))
    //TArray<UMyMJDataAtOneMomentPerRoleCpp *> m_aDatas;



    //replication helper, let's assume range is 100
    int32 m_iRepObjIdBase;
    int32 m_iRepKeyOfState;
};