// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameEventBase.h"

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"

#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"

#include "MyMJGamePlayerController.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

FMyMJEventDataDeltaDurCfgBaseCpp::FMyMJEventDataDeltaDurCfgBaseCpp()
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

uint32 FMyMJEventDataDeltaDurCfgBaseCpp::helperGetDeltaDur(const FMyMJDataDeltaCpp& delta) const
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

    return MY_MJ_GAME_WORLD_TIME_MS_RESOLVE_WITH_DATA_TIME_RESOLUTION(ret);
}

void UMyMJGameEventCycleBuffer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_iTest);

    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_cEventArray);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_iSizeMax);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_idxHead);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_iCount);
    DOREPLIFETIME(UMyMJGameEventCycleBuffer, m_uiLastEventEndTime_data_unit);

};

void UMyMJDataSequencePerRoleCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_eRole);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_cBase);
    //DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_cBaseTest2);
    DOREPLIFETIME(UMyMJDataSequencePerRoleCpp, m_pEventsApplyingAndApplied);
};

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
        */

        //Todo: when replicated on client, we don't need to new()

        m_pEventsApplyingAndApplied = NewObject<UMyMJGameEventCycleBuffer>(outer);

        //m_pEventsApplyingAndApplied = NewObject<UMyMJGameEventCycleBuffer>(this);
    }
};

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

void UMyMJDataSequencePerRoleCpp::addPusherResult(const FMyMJEventDataDeltaDurCfgBaseCpp &inEventDeltaDurCfg, const FMyMJGamePusherResultCpp& cPusherResult, uint32 uiServerWorldTime_resolved_ms)
{
    MY_VERIFY(isReadyToGiveNextPusherResult(uiServerWorldTime_resolved_ms));


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

    uint32 dur_resolved_ms = 0;

    //only delta may have dur
    if (pPusherResult->m_aResultDelta.Num() > 0) {
        MY_VERIFY(pPusherResult->m_aResultDelta.Num() == 1);

        const FMyMJDataDeltaCpp& resultDelta = pPusherResult->m_aResultDelta[0];
        dur_resolved_ms = inEventDeltaDurCfg.helperGetDeltaDur(resultDelta);
    }

    FMyMJEventWithTimeStampBaseCpp& cInserted = m_pEventsApplyingAndApplied->addToTailWhenNotFull(uiServerWorldTime_resolved_ms, dur_resolved_ms, pPusherResult->m_aResultBase.Num() > 0);
    cInserted.setPusherResult(cPusherResult);

    if (cPusherResult.m_aResultBase.Num() > 0) {
        m_iEventsBasePusherCount++;
    }

    //OK, let's check if we need to update base
    trySquashBaseAndEvents(uiServerWorldTime_resolved_ms);

};

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

    if (iGameIdLastSelf < 0) {
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

    int32 iDebugGameIdBefore = m_cBase.getCoreDataRefConst().m_iGameId;
    int32 iDebugPusherIdBefore = m_cBase.getCoreDataRefConst().m_iPusherIdLast;
    if (bSetBase) {
        m_cBase = other.m_cBase;
        bRet = true;

        int32 iDebugGameIdAfter = m_cBase.getCoreDataRefConst().m_iGameId;
        int32 iDebugPusherIdAfter = m_cBase.getCoreDataRefConst().m_iPusherIdLast;

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("setBase: %d:%d -> %d:%d."), iDebugGameIdBefore, iDebugPusherIdBefore, iDebugGameIdAfter, iDebugPusherIdAfter);
    }

    //let's try add data
    MY_VERIFY(other.m_pEventsApplyingAndApplied->verifyData(false) == 0);
    for (int32 i = idxNextPusherOther; i < lother; i++) {
        bool bIsFull = false;
        int32 lSelf = m_pEventsApplyingAndApplied->getCount(&bIsFull);
        if (bIsFull) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("buffer is full, only %d/%d merged, lenth self is %d."), i - idxNextPusherOther, lother - idxNextPusherOther, lSelf);
            break;
        }
        const FMyMJEventWithTimeStampBaseCpp& itemOther = other.m_pEventsApplyingAndApplied->peekRefAt(i);
        m_pEventsApplyingAndApplied->addItemFromOther(itemOther);

        bRet = true;
    }

    if (bRet) {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("role %d's data merged %d : %d, lother %d, iTest %d, event count %d, first: %d:%d, last %d:%d."), (uint8)m_eRole, bRet, bSetBase, lother, iTest, m_pEventsApplyingAndApplied->getCount(NULL), getGameIdFirst(), getPusherIdFirst(), getGameIdLast(), getPusherIdLast());
    }

    return bRet;
}


void UMyMJDataSequencePerRoleCpp::trySquashBaseAndEvents(uint32 uiServerWorldTime_resolved_ms)
{
    bool bIsFull = false;
    int32 l = m_pEventsApplyingAndApplied->getCount(&bIsFull);
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

    squashEventsToBase(toApplyNum);
   
    //size control
    while (m_iEventsBasePusherCount > 2) {
        squashEventsToBase(1);
    }

 
    uint32 uiEndTime = m_cBase.getTime();
    if (uiEndTime > 0 && uiServerWorldTime_resolved_ms < uiEndTime) {
        //valid time stamp, and endTime not passed, yet, it is not allowed to apply yet
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("forcing to squash events before required time passed, maybe buffer is too small."));
    }

    MY_VERIFY(uiEndTime > 0);
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
*/

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
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Owner actor is null, this is only expected for default object creation."));
    }
};



void UMyMJDataAllCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMyMJDataAllCpp, m_aDatas);
    //DOREPLIFETIME(UMyMJDataAllCpp, m_pDataTest0);
    //DOREPLIFETIME(UMyMJDataAllCpp, m_pDataTest1);
    DOREPLIFETIME(UMyMJDataAllCpp, m_iTest);
};


bool UMyMJDataAllCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    UWorld* w = Channel->GetWorld();
    UNetConnection* c = Channel->Connection;
    APlayerController *pc = NULL;
    if (c) {
        pc = c->GetPlayerController(w);
    }

    int32 l = m_aDatas.Num();

    AMyMJGamePlayerControllerCpp* pCMy = Cast<AMyMJGamePlayerControllerCpp>(pc);

    if (!IsValid(pCMy)) {
        FString pcStr = TEXT("NULL");
        if (pc) {
            pcStr = pc->GetClass()->GetFullName();
        }

        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ReplicateSubobjects, pcMy is NULL, the class is not correct, datas len: %d, world %p, netconnection %p, pc %p, pc: %s."), l, w, c, pc, *pcStr);
        return false;
    }

    MyMJGameRoleTypeCpp eClientRole = pCMy->m_eRoleType;


    int64 byteNum0 = Bunch->GetNumBytes();
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
    int64 byteNum1 = Bunch->GetNumBytes();

    //if (Channel->KeyNeedsToReplicate(m_iRepObjIdBase + 10, m_iRepKeyOfState))
    {
        for (int32 i = 0; i < l; i++) 
        {
            UMyMJDataSequencePerRoleCpp* pData = m_aDatas[i];
            //MY_VERIFY(pData);

            //if (Channel->KeyNeedsToReplicate(m_iRepObjIdBase + 11 + i, pData->getRepKeyOfState()))
            if (IsValid(pData))
            {
                MY_VERIFY((uint8)pData->m_eRole < (uint8)MyMJGameRoleTypeCpp::Max);
                if (pData->m_eRole != eClientRole) {
                    continue;
                }

                //if ((i) != 4) {
                    //int32 l0 = pData->getEventCount();
                    //int32 l1 = pData->getEventsRef().m_iTest;
                    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Replicating %d, %d, %d, %d."), i, l0, l1, m_iTest);
                    WroteSomething |= Channel->ReplicateSubobject(pData, *Bunch, *RepFlags);
                    const UMyMJGameEventCycleBuffer* pB = pData->getEvents(false);
                    if (IsValid(pB)) {
                        UMyMJGameEventCycleBuffer* pB0 = const_cast<UMyMJGameEventCycleBuffer *>(pB);
                        WroteSomething |= Channel->ReplicateSubobject(pB0, *Bunch, *RepFlags);
                    }
                //}
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

    int64 byteNum = Bunch->GetNumBytes();
    if (byteNum > 0) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("ReplicateSubobjects, %d bytes written, %d, %d."), (int32)byteNum, (int32)byteNum0, (int32)byteNum1);
        if (byteNum >= 60 * 1000) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ReplicateSubobjects, %d bytes written and it is too big!."), (int32)byteNum);
        }
    }

    return WroteSomething;
};
