// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameEventBase.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"

#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"

void UMyMJDataSequencePerRoleCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_cBase);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_cEventsApplyingAndApplied);
};

uint32 UMyMJDataSequencePerRoleCpp::getLastEventEndTime() const
{

    uint32 uiTimeBase = m_cBase.getTime();
    uint32 uiTimeDeltas = m_cEventsApplyingAndApplied.getLastEventEndTime();

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

void UMyMJDataSequencePerRoleCpp::addPusherResult(const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_resolved_ms)
{
    MY_VERIFY(isReadyToGiveNextPusherResult(uiServerWorldTime_resolved_ms));


    const FMyMJGamePusherResultCpp* pPusherResult = &cPusherResult;

    /*
    if (pPusherResult->m_aResultBase.Num() > 0) {
    //This means a game state reset
    MY_VERIFY(pPusherResult->m_aResultBase.Num() ==  1);
    m_cAccessor.applyBase(pPusherResult->m_aResultBase[0]);
    m_pBase->m_uiStateServerWorldTimeStamp_10ms = uiStateServerWorldTimeStamp_10ms;
    m_cEventsApplyingAndApplied.clear();
    }
    */

    const FMyMJEventDataDeltaDurCfgBaseCpp inEventDeltaDurCfg;
    uint32 dur_resolved_ms = 0;

    //only delta may have dur
    if (pPusherResult->m_aResultDelta.Num() > 0) {
        MY_VERIFY(pPusherResult->m_aResultDelta.Num() == 1);

        const FMyMJDataDeltaCpp& resultDelta = pPusherResult->m_aResultDelta[0];
        dur_resolved_ms = inEventDeltaDurCfg.helperGetDeltaDur(resultDelta);
    }

    FMyMJEventWithTimeStampBaseCpp& cInserted = m_cEventsApplyingAndApplied.addToTailWhenNotFull(uiServerWorldTime_resolved_ms, dur_resolved_ms, pPusherResult->m_aResultBase.Num() > 0);
    cInserted.setPusherResult(cPusherResult);

    //OK, let's check if we need to update base
    trySquashBaseAndEvents(uiServerWorldTime_resolved_ms);

};

void UMyMJDataSequencePerRoleCpp::mergeDataFromOther(const UMyMJDataSequencePerRoleCpp& other)
{
    //we never merge

    //you can append data, when two condition met: 1. same id and pusher, 2. old game ended and new reset comming 

    int32 iGameIdLastSelf = getGameIdLast();
    int32 iPusherIdLastSelf = getPusherIdLast();
    int32 lother = other.m_cEventsApplyingAndApplied.getCount(NULL);
    
    bool bSetBase = false;
    int32 idxNextPusherOther = 0;

    if (iGameIdLastSelf < 0) {
        //we don't have anything, copy all
        bSetBase = true;
        MY_VERIFY(m_cEventsApplyingAndApplied.getCount(NULL) == 0);
        m_cEventsApplyingAndApplied.clear();

    }
    else {
        MY_VERIFY(iPusherIdLastSelf >= 0);

        if (lother > 0) {
            const FMyMJEventWithTimeStampBaseCpp& itemFirstOther = other.m_cEventsApplyingAndApplied.peekRefAt(0);
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
                    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("merging data will reset everything since we can't catch up to target, lastSelf %d:%d, firstOther %d:%d."), iGameIdLastSelf, iPusherIdLastSelf, iGameIdFirstOther, iPusherIdFirstOther);
                }
            }
            else {
                if (isGameEndForLastState()) {
                    //we can gracefully append data
                }
                else {
                    //fuck, other is too fast, we can't catch up
                    bSetBase = true;
                    idxNextPusherOther = 0;
                    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("merging data will reset everything since we can't catch up to target, lastSelf %d:%d, firstOther %d:%d."), iGameIdLastSelf, iPusherIdLastSelf, iGameIdFirstOther, iPusherIdFirstOther);
                }
            }

        }
    }

    if (bSetBase) {
        m_cBase = other.m_cBase;
    }

    //let's try add data
    other.m_cEventsApplyingAndApplied.verifyData();
    for (int32 i = idxNextPusherOther; i < lother; i++) {
        bool bIsFull = false;
        int32 lSelf = m_cEventsApplyingAndApplied.getCount(&bIsFull);
        if (bIsFull) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("buffer is full, only %d/%d merged, lenth self is %d."), i - idxNextPusherOther, lother - idxNextPusherOther, lSelf);
            break;
        }
        const FMyMJEventWithTimeStampBaseCpp& itemOther = other.m_cEventsApplyingAndApplied.peekRefAt(i);
        m_cEventsApplyingAndApplied.addItemFromOther(itemOther);
    }
}

void UMyMJDataSequencePerRoleCpp::trySquashBaseAndEvents(uint32 uiServerWorldTime_resolved_ms)
{
    bool bIsFull = false;
    int32 l = m_cEventsApplyingAndApplied.getCount(&bIsFull);
    if (l <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cEventsApplyingAndApplied have zero lenth: %d, not supposed to happen."), l);
        MY_VERIFY(false);
        return;
    }

    if (!bIsFull) {
        return;
    }

    uint32 toApplyNum = l / 2;
    MY_VERIFY(toApplyNum > 0);

   
    for (int32 i = 0; i < l; i++) {
        const FMyMJEventWithTimeStampBaseCpp& cEvent = m_cEventsApplyingAndApplied.peekRefAt(0);
        uint32 uiEndTime = cEvent.getEndTime();

        if (uiEndTime > 0 && uiServerWorldTime_resolved_ms < uiEndTime) {
            //valid time stamp, and endTime not passed, yet, it is not allowed to apply yet
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("forcing to squash events before required time passed, maybe buffer is too small."));
        }

        const FMyMJGamePusherResultCpp* pPusherResult = cEvent.getPusherResult(true);
        m_cAccessor.applyPusherResult(*pPusherResult);

        if (uiEndTime > 0) {
            m_cBase.setTime(uiEndTime);
            break;
        }
    }
 

    MY_VERIFY(m_cBase.getTime() > 0);
};

/*
uint32 UMyMJDataSequencePerRoleCpp::getLastEventEndTime() const
{
    uint32 uiTimeBase = m_cBase.getTime();
    uint32 uiTimeDeltas = m_cEventsApplyingAndApplied.getLastEventEndTime();

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
*/

void UMyMJDataAllCpp::PostInitProperties()
{
    Super::PostInitProperties();
    AActor *AOwner = GetOwner();
    if (AOwner) {
        if (AOwner->HasAuthority()) {
            createSubObjects();
        }
        else {
            m_pDataHistoryBuffer = NewObject<UMyMJDataSequencePerRoleCpp>(this);
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("UMyMJDataAllCpp owner is NULL, only supposed to happen in default object."));
    }
};



void UMyMJDataAllCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJDataAllCpp, m_aDatas);
};


bool UMyMJDataAllCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    UWorld* w = Channel->GetWorld();
    UNetConnection* c = Channel->Connection;
    APlayerController *pc = NULL;
    if (c) {
        pc = c->GetPlayerController(w);
    }

    FString pcStr = TEXT("NULL");
    if (pc) {
        pcStr = pc->GetClass()->GetFullName();
    }

    int32 l = m_aDatas.Num();
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("ReplicateSubobjects, datas len: %d, world %p, netconnection %p, pc %p, pc: %s"), l, w, c, pc, *pcStr);

    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if (Channel->KeyNeedsToReplicate(m_iRepObjIdBase + 10, m_iRepKeyOfState))
    {
        for (int32 i = 0; i < l; i++) 
        {
            UMyMJDataSequencePerRoleCpp* pData = m_aDatas[i];
            MY_VERIFY(pData);

            if (Channel->KeyNeedsToReplicate(m_iRepObjIdBase + 11 + i, pData->getRepKeyOfState()))
            {
                if ((i) != 4) {
                    WroteSomething |= Channel->ReplicateSubobject(pData, *Bunch, *RepFlags);
                }
            }
        }
    }

    return WroteSomething;
};

void UMyMJDataAllCpp::tryAddDataToHistoryBuffer(MyMJGameRoleTypeCpp eRoleTypeWanted)
{
    MY_VERIFY((uint8)eRoleTypeWanted < (uint8)MyMJGameRoleTypeCpp::Max);
    MY_VERIFY(m_aDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
    MY_VERIFY(IsValid(m_pDataHistoryBuffer));

    UMyMJDataSequencePerRoleCpp* pNew = m_aDatas[(uint8)eRoleTypeWanted];
    
    if (!IsValid(pNew)) {
        //haven't got that data, need wait
        return;
    }

    MY_VERIFY(pNew->m_eRole == eRoleTypeWanted);

    if (m_pDataHistoryBuffer->m_eRole != pNew->m_eRole) {
        m_pDataHistoryBuffer->reinit(eRoleTypeWanted);
    }

    m_pDataHistoryBuffer->mergeDataFromOther(*pNew);

}