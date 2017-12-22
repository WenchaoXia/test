// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameEventBase.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"

#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"

#include "MyMJGamePlayerControllerBase.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

FMyMJCoreRelatedEventCorePusherCfgCpp::FMyMJCoreRelatedEventCorePusherCfgCpp()
{
    m_uiGameStarted = 1000;

    m_uiThrowDices = 500;

    m_uiDistCardsDone = 0;

    m_uiHuBornLocalCS = 1000;

    m_uiTakeCards = 300;

    m_uiGiveCards = 300;

    m_uiWeaveChi = 500;

    m_uiWeavePeng = 500;

    m_uiWeaveGang = 500;

    m_uiWeaveGangBuZhangLocalCS = 500;

    m_uiHu = 1000;

    m_uiZhaNiaoLocalCS = 1000;

    m_uiGameEnded = 500;
};

uint32 FMyMJCoreRelatedEventCorePusherCfgCpp::helperGetDeltaDur(const FMyMJDataDeltaCpp& delta) const
{
    uint32 ret = 0;
    MyMJGamePusherTypeCpp ePusherType = delta.getType();

    if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
        ret = m_uiTakeCards;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
        ret = m_uiGiveCards;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
        MY_VERIFY(delta.m_aRoleDataAttender.Num() == 1);
        MY_VERIFY(delta.m_aRoleDataAttender[0].m_aDataPublic.Num() == 1);
        MY_VERIFY(delta.m_aRoleDataAttender[0].m_aDataPublic[0].m_aWeave2Add.Num() == 1);
        const FMyMJWeaveCpp& cWeave = delta.m_aRoleDataAttender[0].m_aDataPublic[0].m_aWeave2Add[0];
        MyMJWeaveTypeCpp eWeaveType = cWeave.getType();

        if (eWeaveType == MyMJWeaveTypeCpp::ShunZiMing) {
            ret = m_uiWeaveChi;
        }
        else if (eWeaveType == MyMJWeaveTypeCpp::KeZiMing) {
            ret = m_uiWeavePeng;
        }
        else if (eWeaveType == MyMJWeaveTypeCpp::GangAn || eWeaveType == MyMJWeaveTypeCpp::GangMing) {
            if (cWeave.getGangBuZhangLocalCS()) {
                ret = m_uiWeaveGangBuZhangLocalCS;
            }
            else {
                ret = m_uiWeaveGang;
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Invalid weave type %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJWeaveTypeCpp"), (uint8)eWeaveType));
            MY_VERIFY(false);
        }
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {
        ret = m_uiHu;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {
        ret = m_uiZhaNiaoLocalCS;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionStateUpdate) {
        if (delta.m_aCoreData.Num() > 0) {
            MyMJGameStateCpp eGameState = delta.m_aCoreData[0].m_eGameState;
            if (eGameState == MyMJGameStateCpp::GameStarted) {
                ret = m_uiGameStarted;
            }
            else if (eGameState == MyMJGameStateCpp::GameEnd) {
                ret = m_uiGameEnded;
            }
        }
    }

    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {
        ret = m_uiThrowDices;
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {
        ret = m_uiHuBornLocalCS;
    }

    return (ret);
}

void FMyMJDataStructWithTimeStampBaseCpp::applyEvent(FMyMJDataAccessorCpp& cAccessor, const struct FMyMJEventWithTimeStampBaseCpp& cEvent, FMyDirtyRecordWithKeyAnd4IdxsMapCpp *pDirtyRecord)
{
    MY_VERIFY(cAccessor.isSetupped());

    uint32 uiEndTime = cEvent.getEndTime_ms();
    MY_VERIFY(uiEndTime > 0);

    uint32 idLast = getIdEventApplied();
    if (idLast != MyUIntIdDefaultInvalidValue) {
        MY_VERIFY((idLast + 1) == cEvent.getIdEvent());
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Role %d's squashing Events, base %d : %d."), (uint8)m_eRole, m_cBase.getCoreDataPublicRefConst().m_iGameId, m_cBase.getCoreDataPublicRefConst().m_iPusherIdLast);

    if (cEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult) {
        const FMyMJGamePusherResultCpp* pPusherResult = cEvent.getPusherResult(true);
        cAccessor.applyPusherResult(*pPusherResult, pDirtyRecord);
    }
    else if (cEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::Trival) {
        //Todo: add code to handle trival event
    }
    else {
        MY_VERIFY(false);
    }
    
    m_uiIdEventApplied = cEvent.getIdEvent();
    MY_VERIFY(m_uiIdEventApplied != MyUIntIdDefaultInvalidValue);

    setTime_ms(uiEndTime);
};


void UMyMJGameEventCycleBuffer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_iTest);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_cEventArrayData);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_cEventArrayMeta);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_uiTimeRangeStart_ms);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_uiTimeRangeEnd_ms);

    //DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_iHelperIdxLastCorePusherEvent);
    //DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_iHelperIdxLastTrivalEvent);

};



UMyMJDataSequencePerRoleCpp::UMyMJDataSequencePerRoleCpp(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    m_iRepKeyOfState = 1;

    m_eRole = MyMJGameRoleTypeCpp::Max;
    m_iFullDataRecordType = MyMJDataSequencePerRoleFullDataRecordTypeInvalid;
    m_bHelperProduceMode = false;

    m_uiServerWorldTime_ms = 0;

    m_pDeltaDataEvents = NULL;
};

void UMyMJDataSequencePerRoleCpp::createSubObjects(bool bInConstructor)
{
    if (bInConstructor) {
        m_pDeltaDataEvents = CreateDefaultSubobject<UMyMJGameEventCycleBuffer>(TEXT("Events Applying And Applied"));
    }
    else {
        m_pDeltaDataEvents = NewObject<UMyMJGameEventCycleBuffer>(this);
    }
};

void UMyMJDataSequencePerRoleCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UMyMJDataSequencePerRoleCpp, m_cFullData, COND_ReplayOnly);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_pDeltaDataEvents);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_eRole);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_uiServerWorldTime_ms);
};

/*
void UMyMJDataSequencePerRoleCpp::PostInitProperties()
{
    Super::PostInitProperties();

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("UMyMJDataSequencePerRoleCpp PostInitProperties()."));

    if (IsValid(m_pEventsApplyingAndApplied)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pEventsApplyingAndApplied already created."));
        m_pEventsApplyingAndApplied->clear();
    }
    else {
        UObject* outer = GetOuter();

        /*
        if (IsValid(outer)) {
            UActorComponent* comp = dynamic_cast<UActorComponent *>(outer);
            if (IsValid(comp)) {
                AActor *a = comp->GetOwner();
                if (IsValid(a)) {
                    bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(a);
                    if (bHaveLogic) {
                        m_pEventsApplyingAndApplied = NewObject<UMyMJGameEventCycleBuffer>(outer);
                    }
                }
            }

        }
        //

        //Todo: when replicated on client, we don't need to new()

        m_pEventsApplyingAndApplied = NewObject<UMyMJGameEventCycleBuffer>(outer);

        //m_pEventsApplyingAndApplied = NewObject<UMyMJGameEventCycleBuffer>(this);
    }
};
*/

void UMyMJDataSequencePerRoleCpp::getFullAndDeltaLastData(uint32 *pOutLastEventId, uint32 *pOutLastEndTime) const
{

    uint32 id = 0, time = 0;

    uint32 uiIdFull = getFullData().getIdEventApplied();
    uint32 uiTimeFull = getFullData().getTime_ms();

    uint32 uiIdDeltas = 0;
    uint32 uiTimeDeltas = 0;

    const UMyMJGameEventCycleBuffer* pDelta = getDeltaDataEvents(false);
    if (pDelta) {
        const FMyMJEventWithTimeStampBaseCpp* pEventLast = pDelta->peekLast();
        if (pEventLast) {
            uiIdDeltas = pEventLast->getIdEvent();
        }
        uiTimeDeltas = pDelta->getTimeRangeEnd_ms();
    }


    bool bTimeChaos = false, bIdChaos = false;
    if (m_iFullDataRecordType == MyMJDataSequencePerRoleFullDataRecordTypeNone) {
        id = uiIdDeltas;
        time = uiTimeDeltas;
    }
    else {
        if (uiTimeDeltas > 0) {

            //delta exist
            MY_VERIFY(uiIdDeltas > 0);

            if (m_iFullDataRecordType == MyMJDataSequencePerRoleFullDataRecordTypeBottom) {
                if (uiIdDeltas < uiIdFull) {
                    bIdChaos = true;
                }
                if (uiTimeDeltas < uiTimeFull) {
                    bTimeChaos = true;
                }
            }
            else if (m_iFullDataRecordType == MyMJDataSequencePerRoleFullDataRecordTypeTop) {
                if (uiIdDeltas > uiIdFull) {
                    bIdChaos = true;
                }
                if (uiTimeDeltas > uiTimeFull) {
                    bTimeChaos = true;
                }
            }

            id = uiIdDeltas;
            time = uiTimeDeltas;
        }
        else {
            //delta squashed,
            MY_VERIFY(uiIdDeltas == 0);

            id = uiIdFull;
            time = uiTimeFull;
        }
    }

    if (bTimeChaos) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("timestamp chaos: full %d, delta %d, m_iFullDataRecordType %d."), uiTimeFull, uiTimeDeltas, m_iFullDataRecordType);
        MY_VERIFY(false);
    }
    if (bIdChaos) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("event id chaos: full %d, delta %d, m_iFullDataRecordType %d."), uiIdFull, uiIdDeltas, m_iFullDataRecordType);
        MY_VERIFY(false);
    }

    if (pOutLastEventId) {
        *pOutLastEventId = id;
    }

    if (pOutLastEndTime) {
        *pOutLastEndTime = time;
    }
}

bool UMyMJDataSequencePerRoleCpp::isReadyToGiveNextEvent(uint32 uiServerWorldTime_ms) const
{
    if (uiServerWorldTime_ms == 0) {
        return false;
    }

    if (!m_bHelperProduceMode) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("this sequence is not in produce mode."));
        return false;
    }

    uint32 endTime_ms;
    getFullAndDeltaLastData(NULL, &endTime_ms);
    return uiServerWorldTime_ms >= endTime_ms; //use >= to allow apply multiple one time
}

//always success, and the data may squash too fast resulting warning logs, which wll force client to do full sync later
uint32 UMyMJDataSequencePerRoleCpp::addPusherResult(const FMyMJCoreRelatedEventCorePusherCfgCpp &inEventCorePusherCfg, const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_ms)
{
    uint32 idEventLast;
    getFullAndDeltaLastData(&idEventLast, NULL);

    if (!m_bHelperProduceMode) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("this sequence is not in produce mode."));
        MY_VERIFY(false);
        return idEventLast;
    }

    const FMyMJGamePusherResultCpp* pPusherResult = &cPusherResult;

    /*
    if (pPusherResult->m_aResultBase.Num() > 0) {
    //This means a game state reset
    MY_VERIFY(pPusherResult->m_aResultBase.Num() ==  1);
    m_cAccessor.applyBase(pPusherResult->m_aResultBase[0]);
    m_pBase->m_uiStateServerWorldTimeStamp_10ms = uiStateServerWorldTimeStamp_10ms;
    m_pEventsApplyingAndApplied->clear();
    }
    */

    uint32 dur_ms = 0;

    //only delta may have dur
    if (pPusherResult->m_aResultDelta.Num() > 0) {
        MY_VERIFY(pPusherResult->m_aResultDelta.Num() == 1);

        const FMyMJDataDeltaCpp& resultDelta = pPusherResult->m_aResultDelta[0];
        dur_ms = inEventCorePusherCfg.helperGetDeltaDur(resultDelta);
    }

    MY_VERIFY(IsValid(m_pDeltaDataEvents));

    FMyMJEventWithTimeStampBaseCpp& cInserted = m_pDeltaDataEvents->addToTailWhenNotFull(uiServerWorldTime_ms, dur_ms, MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult);

    cInserted.setIdEvent(idEventLast + 1);
    cInserted.setPusherResult(cPusherResult);

    if (m_iFullDataRecordType == MyMJDataSequencePerRoleFullDataRecordTypeTop) {
        m_cFullData.applyEvent(m_cAccessor, cInserted, NULL);
    }

    //OK, let's check if we need to update base
    trySquashBaseAndEvents(uiServerWorldTime_ms);

    return idEventLast + 1;
};

/*
bool UMyMJDataSequencePerRoleCpp::mergeDataFromOther(const UMyMJDataSequencePerRoleCpp& other)
{
    //you can append data, when two condition met: 1. same id and pusher, 2. old game ended and new reset comming 
    bool bRet = false;

    int32 iGameIdLastSelf = getGameIdLast();
    int32 iPusherIdLastSelf = getPusherIdLast();
    int32 lother = other.m_pEventsApplyingAndApplied->getCount(NULL);
    int32 iTest = other.m_pEventsApplyingAndApplied->m_iTest;
    
    bool bSetBase = false;
    int32 idxNextPusherOther = 0;

    MY_VERIFY(IsValid(m_pEventsApplyingAndApplied));

    bool bRoleChanged = m_cBase.getRole() < MyMJGameRoleTypeCpp::Max && m_cBase.getRole() != other.m_cBase.getRole();

    if (iGameIdLastSelf < 0 || bRoleChanged) {
        //we don't have anything, copy all
        
        MY_VERIFY(m_pEventsApplyingAndApplied->getCount(NULL) == 0);

        bSetBase = true;
        idxNextPusherOther = 0;
        m_pEventsApplyingAndApplied->clear();

    }
    else {
        MY_VERIFY(iPusherIdLastSelf >= 0);

        if (lother > 0) {
            const FMyMJEventWithTimeStampBaseCpp& itemFirstOther = other.m_pEventsApplyingAndApplied->peekRefAt(0);
            int32 iGameIdFirstOther = itemFirstOther.getPusherResult(true)->getGameId();
            int32 iPusherIdFirstOther = itemFirstOther.getPusherResult(true)->getPusherIdLast();

            if (iGameIdLastSelf == iGameIdFirstOther) {
                idxNextPusherOther = iPusherIdLastSelf - iPusherIdFirstOther + 1;
                if (idxNextPusherOther >= 0) {
                    //OK, we can give a try later
                }
                else
                {
                    //fuck, other is too fast, we can't catch up
                    bSetBase = true;
                    idxNextPusherOther = 0;
                    m_pEventsApplyingAndApplied->clear();
                    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("merging data will reset everything since we can't catch up to target, lastSelf %d:%d, firstOther %d:%d."), iGameIdLastSelf, iPusherIdLastSelf, iGameIdFirstOther, iPusherIdFirstOther);
                }
            }
            else {
                if (isGameEndForLastState()) {
                    //we can gracefully append data
                    idxNextPusherOther = 0;
                }
                else {
                    //fuck, other is too fast, we can't catch up
                    bSetBase = true;
                    idxNextPusherOther = 0;
                    m_pEventsApplyingAndApplied->clear();
                    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("merging data will reset everything since we can't catch up to target, lastSelf %d:%d, firstOther %d:%d."), iGameIdLastSelf, iPusherIdLastSelf, iGameIdFirstOther, iPusherIdFirstOther);
                }
            }

        }
    }

    int32 iDebugGameIdBefore = m_cBase.getCoreDataPublicRefConst().m_iGameId;
    int32 iDebugPusherIdBefore = m_cBase.getCoreDataPublicRefConst().m_iPusherIdLast;
    if (bSetBase) {
        m_cBase = other.m_cBase;
        bRet = true;

        int32 iDebugGameIdAfter = m_cBase.getCoreDataPublicRefConst().m_iGameId;
        int32 iDebugPusherIdAfter = m_cBase.getCoreDataPublicRefConst().m_iPusherIdLast;

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("setBase: %d:%d -> %d:%d."), iDebugGameIdBefore, iDebugPusherIdBefore, iDebugGameIdAfter, iDebugPusherIdAfter);
    }

    //let's try add data
    //setTime_data_unit(uiSupposedTime_ms);
    MY_VERIFY(other.m_pEventsApplyingAndApplied->verifyData(false) == 0);
    for (int32 i = idxNextPusherOther; i < lother; i++) {
        bool bIsFull = false;
        int32 lSelf = m_pEventsApplyingAndApplied->getCount(&bIsFull);
        const FMyMJEventWithTimeStampBaseCpp& itemOther = other.m_pEventsApplyingAndApplied->peekRefAt(i);
        if (bIsFull) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("buffer is full, only %d/%d merged, lenth self is %d."), i - idxNextPusherOther, lother - idxNextPusherOther, lSelf);
            //uint32 uiDataUnit = MY_MJ_GAME_WORLD_TIME_MS_TO_DATA_TIME(itemOther.getStartTime());
            //MY_VERIFY(uiDataUnit > 0);
            //setTime_data_unit(uiDataUnit);
            break;
        }
        m_pEventsApplyingAndApplied->addItemFromOther(itemOther);

        bRet = true;
    }


    if (bRet) {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("role %d's data merged %d : %d, lother %d, iTest %d, event count %d, first: %d:%d, last %d:%d."), (uint8)m_eRole, bRet, bSetBase, lother, iTest, m_pEventsApplyingAndApplied->getCount(NULL), getGameIdFirst(), getPusherIdFirst(), getGameIdLast(), getPusherIdLast());
    }

    return bRet;
}
*/

void UMyMJDataSequencePerRoleCpp::trySquashBaseAndEvents(uint32 uiServerWorldTime_ms)
{
    MY_VERIFY(IsValid(m_pDeltaDataEvents));

    bool bIsFull = m_pDeltaDataEvents->isFull();
    int32 l = m_pDeltaDataEvents->getCount();
    if (l <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cEventsApplyingAndApplied have zero lenth: %d, not supposed to happen."), l);
        MY_VERIFY(false);
        return;
    }

    if (!bIsFull) {
        return;
    }

    uint32 toApplyNum = l / 8;
    if (toApplyNum == 0) {
        toApplyNum = 1;
    }

    squashDeltaDataEvents(toApplyNum);
   
    //we don't allow too much base
    //while (m_iEventsBasePusherCount > 2) {
        //squashEventsToBase(1);
    //}

    //we don't use this method to detect
    /*
    uint32 uiStartTime = m_cBase.getTime_ms();
    MY_VERIFY(uiStartTime > 0);

    if (uiServerWorldTime_ms <= uiStartTime) {
        //valid time stamp, and endTime not passed, yet, it is not allowed to apply yet
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("forcing to squash events before required time passed, maybe buffer is too small!, base time %d, uiServerWorldTime_ms %d."), uiStartTime, uiServerWorldTime_ms);
    }

    if (m_eRole == MyMJGameRoleTypeCpp::SysKeeper) {
        //do more check for syskeeper seq
        uint32 lastEventId = 0;
        getFullAndDeltaLastData(&lastEventId, NULL);
        if (lastEventId > m_uiHelerIdEventConsumed) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("forcing to squash events but unconsumed event have lost, producing is too fast or consuming is too slow, lastEventId %d, m_uiHelerIdEventConsumed %d."), lastEventId, m_uiHelerIdEventConsumed);
        }
    }
    */

};

void UMyMJDataSequencePerRoleCpp::OnRep_ServerWorldTime_ms()
{

    UMyCommonUtilsLibrary::genTimeStrFromTimeMs(m_uiServerWorldTime_ms);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnRep_ServerWorldTime_ms, role %d, time %s."), (uint8)m_eRole, *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(m_uiServerWorldTime_ms));
    m_cReplicateDelegate.Broadcast();
};

/*
uint32 UMyMJDataSequencePerRoleCpp::getLastEventEndTime() const
{
    uint32 uiTimeBase = m_cBase.getTime();
    uint32 uiTimeDeltas = m_pEventsApplyingAndApplied->getLastEventEndTime();

    if (uiTimeDeltas > 0) {
        //if you have delta
        if (uiTimeDeltas < uiTimeBase) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("timestamp chaos: base %d, delta %d."), uiTimeBase, uiTimeDeltas);
            MY_VERIFY(false);
        }
    }

    return uiTimeBase > uiTimeDeltas ? uiTimeBase : uiTimeDeltas;
}

bool UMyMJDataSequencePerRoleCpp::isReadyToGiveNextPusherResult(uint32 uiServerWorldTime_resolved_ms) const
{
    if (uiServerWorldTime_resolved_ms == 0) {
        return false;
    }

    bool bRet = uiServerWorldTime_resolved_ms >= getLastEventEndTime(); //use >= to allow apply multiple one time

    return bRet;
}


void UMyMJDataSequencePerRoleCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_cData);
};

void UMyMJDataAtOneMomentPerRoleCpp::applyPusherResult(const FMyMJGamePusherResultCpp& pusherResult, uint32 uiServerWorldTime_resolved_ms)
{
    MY_VERIFY(m_cData.isReadyToGiveNextPusherResult(uiServerWorldTime_resolved_ms));

    const FMyMJEventDataDeltaDurCfgBaseCpp inEventDeltaDurCfg;

    //1st, apply any pending events
    if (m_cData.m_aEventApplying.Num() > 0) {
        MY_VERIFY(m_cData.m_aEventApplying.Num() == 1);
        m_cAccessor.applyPusherResult(*m_cData.m_aEventApplying[0].getPusherResult(true));
        m_cData.m_cBase.setTime(uiServerWorldTime_resolved_ms);
    }
    m_cData.m_aEventApplying.Reset();
    
    //2nd, handle new pusher result
    uint32 dur_resolved_ms = 0;

    //only delta may have dur
    if (pusherResult.m_aResultDelta.Num() > 0) {
        MY_VERIFY(pusherResult.m_aResultDelta.Num() == 1);

        const FMyMJDataDeltaCpp& resultDelta = pusherResult.m_aResultDelta[0];
        dur_resolved_ms = inEventDeltaDurCfg.helperGetDeltaDur(resultDelta);
    }

    if (dur_resolved_ms == 0) {
        //apply it now
        m_cAccessor.applyPusherResult(pusherResult);
        m_cData.m_cBase.setTime(uiServerWorldTime_resolved_ms);
    }
    else {
        int32 idx = m_cData.m_aEventApplying.Emplace();
        m_cData.m_aEventApplying[idx].setPusherResult(pusherResult);
        m_cData.m_aEventApplying[idx].setStartTime(uiServerWorldTime_resolved_ms);
        m_cData. m_aEventApplying[idx].setDuration(dur_resolved_ms);
    }
}

void UMyMJDataAllCpp::PostInitProperties()
{
    Super::PostInitProperties();
    AActor *AOwner = GetOwner();

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("UMyMJDataAllCpp::PostInitProperties()"));

    if (IsValid(AOwner)) {
        bool bHaveLogic = UMyMJBPUtilsLibrary::haveServerLogicLayer(AOwner);
        //bool bHaveVisual = UMyMJBPUtilsLibrary::haveClientVisualLayer(AOwner);


        if (bHaveLogic)
        {
            createSubObjects();
        }
    }bjectInitializer& ObjectInitializer 
    else {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Owner actor is null, this is only expected for default object creation."));
    }
};
*/

UMyMJDataAllCpp::UMyMJDataAllCpp(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    m_iTest = 0;

    m_aDatas.Reset();

    m_iRepObjIdBase = 200;
    m_iRepKeyOfState = 1;

    m_bShowDebugLog = false;
    m_fDebugSupposedReplicationUpdateLastRealTime = 0;
    m_uiDebugSupposedReplicationUpdateLastIdEvent = 0;

    //UMyMJDataSequencePerRoleCpp *pNew;
    //pNew = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>((TEXT("mj data 0")));
    //pNew->init((MyMJGameRoleTypeCpp)0);

    //m_pTestSeq = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("test seq c"));
    //m_pTestBuffer = CreateDefaultSubobject<UMyMJGameEventCycleBuffer>(TEXT("test buffer c"));

    //m_pTestC = pNew;
    //m_aDatas.Emplace(pNew);

    //replicated object can't be created in CDO
    //createSubObjects(true, this);

    bReplicates = true;

};

UMyMJDataAllCpp::~UMyMJDataAllCpp()
{

};

void UMyMJDataAllCpp::clearInGame()
{
    int32 l = m_aDatas.Num();
    for (int i = 0; i < l; i++)
    {
        UMyMJDataSequencePerRoleCpp* pSeq = m_aDatas[i];
        if (!IsValid(pSeq)) {
            continue;
        }

        pSeq->clearInGame();
    }

    //markDirtyForRep();

    m_fDebugSupposedReplicationUpdateLastRealTime = 0;
    m_uiDebugSupposedReplicationUpdateLastIdEvent = 0;
};

void UMyMJDataAllCpp::createSubObjects(bool bInConstructor)
{
    m_aDatas.Reset();

    for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {

        UMyMJDataSequencePerRoleCpp *pNew;
        if (bInConstructor) {
            FString name = FString::Printf(TEXT("mj data seq %d"), i);
            pNew = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(FName(*name));
        }
        else {
            pNew = NewObject<UMyMJDataSequencePerRoleCpp>(this);
        }
        pNew->createSubObjects(false); //Uobject constructor can only be valid for himself, so for subobject it is always not in subobject's constructor
        pNew->init((MyMJGameRoleTypeCpp)i);

        m_aDatas.Emplace(pNew);

    }

};

void UMyMJDataAllCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMyMJDataAllCpp, m_aDatas);
    DOREPLIFETIME(UMyMJDataAllCpp, m_iTest);
};


bool UMyMJDataAllCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if (getServerWorldTime_ms() <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Cancel object replication since m_uiServerTime_ms_unresolved is 0."));
        return WroteSomething;
    }

    //const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("ReplicateSubobjects thread id: %d"), CurrentThreadId);

    UWorld* w = Channel->GetWorld();
    UNetConnection* c = Channel->Connection;
    APlayerController *pC = NULL;
    if (c) {
        pC = c->GetPlayerController(w);
    }


    int32 l = m_aDatas.Num();

    MyMJGameRoleTypeCpp eClientRole = MyMJGameRoleTypeCpp::Observer;
    if (RepFlags && RepFlags->bReplay) {
        MyMJGameRoleTypeCpp eClientRole = MyMJGameRoleTypeCpp::SysKeeper;
    }
    else {

        if (!IsValid(pC)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ReplicateSubobjects, controller's is not valid, %p."), pC);
            return WroteSomething;
        }

        FString strClass = pC->GetClass()->GetFullName();
        AMyMJGamePlayerControllerBaseCpp* pCBase = Cast<AMyMJGamePlayerControllerBaseCpp>(pC);
        if (!IsValid(pCBase)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ReplicateSubobjects, cast failed, it's class is %s."), *strClass);
            return WroteSomething;
        }

        eClientRole = pCBase->getRoleType();
    }

    if (eClientRole >= MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("ReplicateSubobjects, will not replicate since role is %d."), (uint8)eClientRole);
        return WroteSomething;
    }
    
    /*
    uint32 id = pCMy->GetUniqueID();

    uint32 uiTimeLast = 0;
    uint32 uiTimeNew = getServerWorldTime_ms();
    uint32* puiTImeLast = m_mHelerReplicatePCServerTimeMap.Find(id);
    if (puiTImeLast) {
        uiTimeLast = *puiTImeLast;
    }

    if (uiTimeLast == uiTimeNew) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("uiTimeLast is same as %d, to keep data unique, we will skip this loop."), uiTimeLast);
        return WroteSomething;
    }
    uint32& newTime = m_mHelerReplicatePCServerTimeMap.FindOrAdd(id);
    newTime = uiTimeNew;
    */

    int64 byteNum0 = Bunch->GetNumBytes();
    int64 byteNum1 = Bunch->GetNumBytes();

    //if (Channel->KeyNeedsToReplicate(m_iRepObjIdBase + 10, m_iRepKeyOfState))
    bool bReplicated = false;
    {
        for (int32 i = 0; i < l; i++) 
        {
            UMyMJDataSequencePerRoleCpp* pData = m_aDatas[i];
            //MY_VERIFY(pData);

            //if (Channel->KeyNeedsToReplicate(m_iRepObjIdBase + 11 + i, pData->getRepKeyOfState()))
            if (IsValid(pData))
            {

                MY_VERIFY((uint8)pData->getRole() < (uint8)MyMJGameRoleTypeCpp::Max);
                if (pData->getRole() != eClientRole) {
                    continue;
                }

                //if ((i) != 4) {
                    //int32 l0 = pData->getEventCount();
                    //int32 l1 = pData->getEventsRef().m_iTest;
                    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Replicating %d, %d, %d, %d."), i, l0, l1, m_iTest);
                    WroteSomething |= Channel->ReplicateSubobject(pData, *Bunch, *RepFlags);
                    const UMyMJGameEventCycleBuffer* pB = pData->getDeltaDataEvents(false);
                    if (IsValid(pB)) {
                        UMyMJGameEventCycleBuffer* pB0 = const_cast<UMyMJGameEventCycleBuffer *>(pB);
                        WroteSomething |= Channel->ReplicateSubobject(pB0, *Bunch, *RepFlags);
                        bReplicated = true;
                    }
                //}
            }
        }
    }

    //set helper data to detect producing too fast
    /*
    if (bReplicated) {
        UMyMJDataSequencePerRoleCpp* pData = m_aDatas[(uint8)MyMJGameRoleTypeCpp::SysKeeper];
        if (IsValid(pData))
        {
            uint32 uiIdEventLast = 0;
            const UMyMJGameEventCycleBuffer* pEvents = pData->getDeltaDataEvents();
            if (IsValid(pEvents)) {
                const FMyMJEventWithTimeStampBaseCpp* pLast = pEvents->peekLast();
                if (pLast) {
                    uiIdEventLast = pLast->getIdEvent();
                }
            }

            if (uiIdEventLast > 0) {
                pData->setHelerIdEventConsumed(uiIdEventLast);
            }
        }
    }

    if (IsValid(m_pDataTest0)) {
        WroteSomething |= Channel->ReplicateSubobject(m_pDataTest0, *Bunch, *RepFlags);
        const UMyMJGameEventCycleBuffer* pB = m_pDataTest0->getEvents(false);
        if (IsValid(pB)) {
            UMyMJGameEventCycleBuffer* pB0 = const_cast<UMyMJGameEventCycleBuffer *>(pB);
            WroteSomething |= Channel->ReplicateSubobject(pB0, *Bunch, *RepFlags);
        }
    }

    if (IsValid(m_pDataTest1)) {
        WroteSomething |= Channel->ReplicateSubobject(m_pDataTest1, *Bunch, *RepFlags);
        const UMyMJGameEventCycleBuffer* pB = m_pDataTest1->getEvents(false);
        if (IsValid(pB)) {
            UMyMJGameEventCycleBuffer* pB0 = const_cast<UMyMJGameEventCycleBuffer *>(pB);
            WroteSomething |= Channel->ReplicateSubobject(pB0, *Bunch, *RepFlags);
        }
    }
    */

    int64 byteNum = Bunch->GetNumBytes();
    if (byteNum > 0) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("ReplicateSubobjects, %d bytes written, %d, %d."), (int32)byteNum, (int32)byteNum0, (int32)byteNum1);
        if (byteNum >= 60 * 1000) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ReplicateSubobjects, %d bytes written and it is too big!."), (int32)byteNum);
        }
    }

    return WroteSomething;
};

void UMyMJDataAllCpp::addPusherResult(const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_ms, bool *pOutNeedReboot)
{
    int32 l = m_aDatas.Num();
    MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);
    FMyMJGamePusherResultCpp cPusherNew;

    if (m_bShowDebugLog) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("adding pusher result: %sms: %s"), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(uiServerWorldTime_ms), *cPusherResult.genDebugMsg());
    }

    for (int32 i = 0; i < l; i++) {

        if (i == (uint8)MyMJGameRoleTypeCpp::SysKeeper) {
            MY_VERIFY(IsValid(m_aDatas[i]));
            uint32 idEventNew = m_aDatas[i]->addPusherResult(m_cEventCorePusherCfg, cPusherResult, uiServerWorldTime_ms);

            //check event id here, time_ms check should goto actor
            if (idEventNew >= MyUInt32IdWarnBottomValue && idEventNew < (MyUInt32IdWarnBottomValue + 2)) {

                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("uint32 id is too large as %d, require reboot!"), idEventNew);

                if (pOutNeedReboot) {
                    *pOutNeedReboot = true;
                }
            }

            if (idEventNew >= MyUInt32IdCoreDumpBottomValue) {
                MY_VERIFY(false);
            }
        }
        else {
            MyMJGameRoleTypeCpp eRole = (MyMJGameRoleTypeCpp)i;
            if (!IsValid(m_aDatas[i])) {
                continue;
            }
            MY_VERIFY(m_aDatas[i]->getRole() == eRole);

            cPusherResult.copyWithRoleFromSysKeeperRole(eRole, cPusherNew);
            m_aDatas[i]->addPusherResult(m_cEventCorePusherCfg, cPusherNew, uiServerWorldTime_ms);
        }
    }
};

void UMyMJDataAllCpp::updateDebugInfo(float fWorldRealTimeNow, uint32 uiIdEventBefore)
{
    UMyMJDataSequencePerRoleCpp* pSeq = getDataByRoleType(MyMJGameRoleTypeCpp::SysKeeper);
    uint32 uiIdEventNow = 0;
    pSeq->getFullAndDeltaLastData(&uiIdEventNow, NULL);

    int32 bufferSize = pSeq->getDeltaDataEvents(true)->getCountMax();
    int32 eventNumAddedThisTurn = (uiIdEventNow - uiIdEventBefore);
    if (eventNumAddedThisTurn > bufferSize) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("in one turn,    event adding is too fast! %d/%d added."), eventNumAddedThisTurn, bufferSize);
    }

    int32 eventNumAddedAfterPrevUpdate = (uiIdEventNow - m_uiDebugSupposedReplicationUpdateLastIdEvent);
    if (eventNumAddedAfterPrevUpdate > bufferSize) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("in supposed rep, event adding is too fast! %d/%d added."), eventNumAddedAfterPrevUpdate, bufferSize);
    }

    if (fWorldRealTimeNow >= (m_fDebugSupposedReplicationUpdateLastRealTime + (1.f / MyReplicateUpdateFreqMax))) {
        m_fDebugSupposedReplicationUpdateLastRealTime = fWorldRealTimeNow;
        m_uiDebugSupposedReplicationUpdateLastIdEvent = uiIdEventNow;
    }
}