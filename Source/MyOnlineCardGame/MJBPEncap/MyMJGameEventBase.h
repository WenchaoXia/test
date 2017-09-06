// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MJ/MyMJGameAttender.h"

//#include "GameFramework/Actor.h"
//#include "UnrealNetwork.h"

#include "MyMJGameEventBase.generated.h"

//16ms is precise enough
#define MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(dataTime)   (dataTime << 4)
#define MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(worldTime_ms) (worldTime_ms >> 4)

#define MY_MJ_GAME_WORLD_TIME_MS_RESOLUTION MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(1)

#define MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(worldTime_ms) (MY_MJ_GAME_DATA_TIME_TO_WORLD_TIME_MS(MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(worldTime_ms)))

#define MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(worldTime_ms) (MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(worldTime_ms) == worldTime_ms)

//uint is MS
USTRUCT()
struct FMyMJEventDataDeltaDurCfgBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJEventDataDeltaDurCfgBaseCpp()
    {
        m_uiGameStarted = 1000;

        m_uiThrowDiceForDistCardsAtStart = 500;

        m_uiDistCardsDone = 1000;

        m_uiHuBornLocalCS = 1000;

        m_uiTakeCards = 300;

        m_uiGiveCards = 300;

        m_uiWeaveChi = 500;

        m_uiWeavePeng = 500;

        m_uiWeaveGang = 500;

        m_uiWeaveGangBuZhangLocalCS = 500;

        m_uiHu = 1000;

        m_uiZhaNiaoLocalCS = 1000;
    };

    //return uint is ms, resolved
    uint32 helperGetDeltaDur(const FMyMJDataDeltaCpp& delta) const
    {
        return 0;
    }

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
};

USTRUCT()
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
        MY_VERIFY(MY_MJ_GAME_WORLD_TIME_MS_IS_RESOLVED(uiTime_ms));
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
USTRUCT()
struct FMyMJEventWithTimeStampBaseCpp : public FFastArraySerializerItem
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJEventWithTimeStampBaseCpp() : Super()
    {
        m_uiStartTime_data_unit = 0;
        m_uiDuration_data_unit = 0;

        m_iIdxDebug = 0;
    };

    virtual ~FMyMJEventWithTimeStampBaseCpp()
    {

    };

    uint32 getEndTime() const
    {
        return m_uiStartTime_data_unit + m_uiDuration_data_unit;
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

    UPROPERTY()
        int32 m_iIdxDebug;

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

//Todo: make it template
//Different game types have different datas, for a turn based one, history is important
//it is like a queue with limited size, but don't need allocate memory, thread unsafe
USTRUCT()
struct FMyMJGameEventCycleBuffer
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameEventCycleBuffer()
    {
        resize(128);
    };

    virtual ~FMyMJGameEventCycleBuffer()
    {


    };

    void resize(int32 iNewSizeMax)
    {
        m_iSizeMax = iNewSizeMax;
        m_cEventArray.Items.AddZeroed(m_iSizeMax);
        MY_VERIFY(m_iSizeMax > 0);

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
    const FMyMJEventWithTimeStampBaseCpp& peekRefAt(int32 idxFromHead, int32 *pOutIdxInArrayDebug = NULL) const
    {
        MY_VERIFY(idxFromHead < m_iCount && idxFromHead >= 0);
        int32 idxFound = idxFromHead + idxFromHead;
        if (pOutIdxInArrayDebug) {
            *pOutIdxInArrayDebug = idxFound;
        }
        const FMyMJEventWithTimeStampBaseCpp& ret = m_cEventArray.Items[idxFound];

        //extra check
        uint32 time_ms = ret.getEndTime();
        if (time_ms <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem at %d have timestamp %d, not valid! internal state: m_idxHead %d, m_iCount %d."), idxFromHead, time_ms, m_idxHead, m_iCount);
            //MY_VERIFY(false);
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

    void verifyData() const
    {
        uint32 timestamp_end = 0;
        int32 l = getCount(NULL);
        for (int32 i = 0; i < l; i++) {
            int32 idxInArray = -1;
            const FMyMJEventWithTimeStampBaseCpp& elem = peekRefAt(i, &idxInArray);

            if (elem.m_iIdxDebug != idxInArray) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("elem's id not equal, elem id %d, real id %d, idx from head %d."), elem.m_iIdxDebug, idxInArray, i);
                MY_VERIFY(false);
            }

            uint32 te = elem.getEndTime();
            if (te > 0) {
                if (te < timestamp_end) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time stamp screw, prev time %d, elem time %d, idx from head %d."), timestamp_end, te, i);
                    MY_VERIFY(false);
                }

                timestamp_end = te;
            }
        }
    };

    void addItemFromOther(const FMyMJEventWithTimeStampBaseCpp& itemOther)
    {
        uint32 startTime = itemOther.getStartTime();
        uint32 dur = itemOther.getDuration();
        FMyMJEventWithTimeStampBaseCpp& newAdded = addToTailWhenNotFull(startTime, dur, startTime > 0);
        newAdded = itemOther;
    };

protected:

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

        FMyMJEventWithTimeStampBaseCpp& newItem = m_cEventArray.Items[idxNew];
        newItem.m_iIdxDebug = idxNew;

        m_cEventArray.MarkItemDirty(newItem);

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

    UPROPERTY()
    FMyMJGameEventArray m_cEventArray;

    UPROPERTY()
    int32 m_iSizeMax;

    //only valid when count > 0
    UPROPERTY()
    int32 m_idxHead;

    UPROPERTY()
    int32 m_iCount;

    UPROPERTY()
    uint32 m_uiLastEventEndTime_data_unit;
};


//mainly keeps a history, this is the logic core need to replicate
UCLASS()
class MYONLINECARDGAME_API UMyMJDataSequencePerRoleCpp : public UObject
{
    GENERATED_BODY()

public:
    UMyMJDataSequencePerRoleCpp()
    {
        m_iRepKeyOfState = 1;

        m_cAccessor.setupTempMode(&m_cBase);
        reinit(MyMJGameRoleTypeCpp::Max);
    };

    void reinit(MyMJGameRoleTypeCpp eRole)
    {
        m_eRole = eRole;
        clear();
        markDirtyForRep();
    };

    void clear()
    {
        m_cBase.reset();
        m_cEventsApplyingAndApplied.clear();

        m_iHelperGameIdLast = -1;
        m_iHelperPusherIdLast = -1;
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

    //return < 0 means empty, no valid data inside
    int32 getGameIdLast() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = m_cEventsApplyingAndApplied.getLast();

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
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = m_cEventsApplyingAndApplied.getLast();

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

    bool isGameEndForLastState() const
    {
        const FMyMJEventWithTimeStampBaseCpp* pElemLast = m_cEventsApplyingAndApplied.getLast();

        if (pElemLast)
        {
            const FMyMJGamePusherResultCpp* pResult = pElemLast->getPusherResult(true);
            if (pResult->m_aResultDelta.Num() > 0 && pResult->m_aResultDelta[0].getType() == MyMJGamePusherTypeCpp::ActionStateUpdate &&
                pResult->m_aResultDelta[0].m_aCoreData.Num() > 0 && pResult->m_aResultDelta[0].m_aCoreData[0].m_eGameState == MyMJGameStateCpp::GameEnd)
            {
                return true;
            }
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
    void addPusherResult(const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_resolved_ms);

    //enable merging is good, let client have more chance to buffer data
    void mergeDataFromOther(const UMyMJDataSequencePerRoleCpp& other);

    void doTestChange()
    {
        m_cBase.setTime(m_cBase.getTime() + MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(1000));
        if (m_cEventsApplyingAndApplied.getCount(NULL) > 0) {
            m_cEventsApplyingAndApplied.clear();
        }
        else {
            int32 tBase = 0;
            for (int32 i = 0; i < 10; i++) {
                uint32 timeStart = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(tBase);
                uint32 timeDur = MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(1500);
                tBase += timeDur;
                m_cEventsApplyingAndApplied.addToTailWhenNotFull(timeStart, timeDur, false);
            }
        }
    };

    FString genDebugMsg()
    {
        int32 l = m_cEventsApplyingAndApplied.getCount(NULL);
        FString ret = FString::Printf(TEXT("role %d, base time %d, event count %d."), (uint8)m_eRole, m_cBase.getTime(), l);
        for (int32 i = 0; i < l; i++) {
            const FMyMJEventWithTimeStampBaseCpp& e = m_cEventsApplyingAndApplied.peekRefAt(i);
            ret += FString::Printf(TEXT(" event %d, time start %d, dur %d."), i, e.getStartTime(), e.getDuration());
        }

        return ret;
    };

    //following is the core data, representing

    //driven by @m_cEventsApplyingAndApplied
    UPROPERTY(Replicated)
    FMyMJDataStructWithTimeStampBaseCpp m_cBase;

    UPROPERTY(Replicated)
    FMyMJGameEventCycleBuffer m_cEventsApplyingAndApplied;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role"))
    MyMJGameRoleTypeCpp m_eRole;

protected:

    //make the state move forward
    void trySquashBaseAndEvents(uint32 uiServerWorldTime_resolved_ms);

    //The base state of deltas(time of start, not end), since the delta is one way to apply(can not revert)
    //we never store more than one base, this means when base change, all of its previouse delta data will be cleared!
    //UMyMJDataForMirrorModeCpp *m_pBase;
    FMyMJDataAccessorCpp m_cAccessor;

    int32 m_iRepKeyOfState;

    int32 m_iHelperGameIdLast;
    int32 m_iHelperPusherIdLast;
};

/*
USTRUCT()
struct FMyMJDataAtOneMomentCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataAtOneMomentCpp()
    {
        m_eRole = MyMJGameRoleTypeCpp::Max;
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

    void copyWithRoleFromSysKeeperRole(FMyMJDataAtOneMomentCpp& cTargetData) const
    {
        MY_VERIFY(m_eRole == MyMJGameRoleTypeCpp::SysKeeper);
        MY_VERIFY(cTargetData.m_eRole != MyMJGameRoleTypeCpp::SysKeeper && cTargetData.m_eRole != MyMJGameRoleTypeCpp::Max);

        m_cBase.copyWithRoleFromSysKeeperRole2(cTargetData.m_eRole, cTargetData.m_cBase);

        cTargetData.m_aEventApplying.Reset();
        int32 l = m_aEventApplying.Num();
        MY_VERIFY(l <= 1);
        if (l > 0) {
            cTargetData.m_aEventApplying.Emplace();
            m_aEventApplying[0].copyWithRoleFromSysKeeperRole2(cTargetData.m_eRole, cTargetData.m_aEventApplying[0]);
        }
    };

    //unit is ms, resolved
    uint32 getLastEventEndTime() const;
    bool isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_resolved_ms) const;



    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data base"))
    FMyMJDataStructWithTimeStampBaseCpp m_cBase;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data delta applying"))
    TArray<FMyMJEventWithTimeStampBaseCpp> m_aEventApplying;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role"))
        MyMJGameRoleTypeCpp m_eRole;

};


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
        m_iRepObjIdBase = 200;
        m_iRepKeyOfState = 1;
    };

    virtual void PostInitProperties() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    void doTestChange()
    {
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

        markDirtyForRep();
    };

    FString genDebugMsg()
    {

        int32 l = m_aDatas.Num();
        FString ret = FString::Printf(TEXT("m_aDatas l: %d."), l);
        for (int32 i = 0; i < l; i++) {
            ret += FString::Printf(TEXT(" elem %d: "), i);
            if (m_aDatas[i]) {
                ret += m_aDatas[i]->genDebugMsg();
            }
            else {
                ret += FString::Printf(TEXT(" Null."), l);
            }
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
            m_aDatas[i]->markDirtyForRep();
        }
        markDirtyForRep();
    };

    inline
    bool isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_resolved_ms) const
    {
        MY_VERIFY(m_aDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
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
                m_aDatas[i]->addPusherResult(cPusherResult, uiServerWorldTime_resolved_ms);
            }
            else {
                MyMJGameRoleTypeCpp eRole = (MyMJGameRoleTypeCpp)i;
                MY_VERIFY(m_aDatas[i]->m_eRole == eRole);

                cPusherResult.copyWithRoleFromSysKeeperRole(eRole, cPusherNew);
                m_aDatas[i]->addPusherResult(cPusherNew, uiServerWorldTime_resolved_ms);
            }
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

    void tryAddDataToHistoryBuffer(MyMJGameRoleTypeCpp eRoleTypeWanted);

protected:

    void createSubObjects()
    {
        for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            UMyMJDataSequencePerRoleCpp *pNew = NewObject<UMyMJDataSequencePerRoleCpp>(this);
            m_aDatas.Emplace(pNew);
            pNew->m_eRole = (MyMJGameRoleTypeCpp)i;
        }
    };

    //basically we have two ways to sync, one is property Replication, one is RPC, here is implemention of 1st method

    //Memory is cheap, let's clone each
    UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "datas"))
    TArray<UMyMJDataSequencePerRoleCpp *> m_aDatas;

    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "datas"))
    //TArray<UMyMJDataAtOneMomentPerRoleCpp *> m_aDatas;

    UPROPERTY()
    UMyMJDataSequencePerRoleCpp* m_pDataHistoryBuffer;

    //replication helper, let's assume range is 100
    int32 m_iRepObjIdBase;
    int32 m_iRepKeyOfState;
};