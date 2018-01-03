// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameDeskVisualData.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

#include "MyMJGameRoom.h"

//#include "Runtime/CoreUObject/Public/Templates/SubclassOf.h"

#define MY_VISUAL_PROCESSOR_SUB_THREAD_LOOP_TIME_MS (16)
#define MY_VISUAL_PROCESSOR_CMD_IN_BUFFER_SIZE (8)
#define MY_VISUAL_PROCESSOR_CMD_OUT_BUFFER_SIZE (8)
#define MY_VISUAL_PROCESSOR_DATA_IN_BUFFER_SIZE (128)
#define MY_VISUAL_PROCESSOR_DATA_OUT_BUFFER_SIZE (128)

#define MyMJGameProgressStayBehindMaxTimeMs (5000)
#define MyMJGameProgressJumpAheadDataMaxTimeMs (2000)
#define MyMJGameProgressPredictMaxTimeMs (3000)

#define MyMJGameVisualStateCatchUpMinTimeToStayMs (500)

int32 FMyMJGameDeskVisualPointCfgCacheCpp::getCardVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const
{

    int32 errCode = helperGetVisualPointCfgByIdxs(0, idxAttender, (int32)eSlot, m_mCardVisualPointCache, visualPoint);
    if (errCode != 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getCardVisualPointCfgByIdxAttenderAndSlot() failed, idxAttender %d, eSlot %d, errorCode: %d."), idxAttender, (int32)eSlot, errCode);
    }

    return errCode;
};

void  FMyMJGameDeskVisualPointCfgCacheCpp::setCardVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, const FMyMJGameDeskVisualPointCfgCpp &visualPoint)
{
    helperSetVisualPointCfgByIdxs(0, idxAttender, (int32)eSlot, m_mCardVisualPointCache, visualPoint);
};

int32 FMyMJGameDeskVisualPointCfgCacheCpp::getTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const
{
    int32 errCode = helperGetVisualPointCfgByIdxs((int32)eElemType, subIdx0, subIdx1, m_mTrivalVisualPointCache, visualPoint);
    if (errCode != 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getTrivalVisualPointCfgByIdxAttenderAndSlot() failed, eElemType %d, subIdx0 %d, subIdx0 %d, errorCode: %d."), (int32)eElemType, subIdx0, subIdx1, errCode);
    }

    return errCode;
};

void  FMyMJGameDeskVisualPointCfgCacheCpp::setTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, const FMyMJGameDeskVisualPointCfgCpp &visualPoint)
{
    helperSetVisualPointCfgByIdxs((int32)eElemType, subIdx0, subIdx1, m_mTrivalVisualPointCache, visualPoint);
};

int32 FMyMJGameDeskVisualPointCfgCacheCpp::helperGetVisualPointCfgByIdxs(int32 idx0, int32 idx1, int32 idx2, const TMap<int32, FMyMJGameDeskVisualPointCfgCpp>& cTargetMap, FMyMJGameDeskVisualPointCfgCpp &visualPoint)
{
    int32 key;
    CalcKey_Macro_KeyAnd4IdxsMap(key, 0, idx0, idx1, idx2);

    const FMyMJGameDeskVisualPointCfgCpp *pV = cTargetMap.Find(key);
    if (pV) {
        visualPoint = *pV;
        return 0;
    }

    return -1;
};

void  FMyMJGameDeskVisualPointCfgCacheCpp::helperSetVisualPointCfgByIdxs(int32 idx0, int32 idx1, int32 idx2, TMap<int32, FMyMJGameDeskVisualPointCfgCpp>& cTargetMap, const FMyMJGameDeskVisualPointCfgCpp &visualPoint)
{
    int32 key;
    CalcKey_Macro_KeyAnd4IdxsMap(key, 0, idx0, idx1, idx2);

    FMyMJGameDeskVisualPointCfgCpp *pV = &cTargetMap.FindOrAdd(key);
    MY_VERIFY(pV);
    *pV = visualPoint;
};


void FMyMJGameDeskVisualDataCpp::applyDelta(struct FMyMJGameDeskVisualDataDeltaCpp& cDelta, uint32 uiNewServerWorldTime_ms)
{
    if (cDelta.m_apNewCoreData.Num() > 0) {
        m_cCoreData = cDelta.m_apNewCoreData[0];
    }
    if (cDelta.m_apNewCoreDataDirtyRecord.Num() > 0) {
        MY_VERIFY(cDelta.m_apNewCoreData.Num() > 0)
    }
    if (cDelta.m_apEventJustApplied.Num() > 0) {
        MY_VERIFY(cDelta.m_apNewCoreData.Num() > 0)
    }

    for (auto& Elem : cDelta.m_mNewActorDataIdCards)
    {
        int32 idCard = Elem.Key;
        FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        if (idCard >= m_cActorData.m_aCards.Num())
        {
            m_cActorData.m_aCards.AddDefaulted((idCard - m_cActorData.m_aCards.Num()) + 1);
            MY_VERIFY(idCard == (m_cActorData.m_aCards.Num() - 1));
        }

        m_cActorData.m_aCards[idCard] = cInfoAndResult;

    }

    for (auto& Elem : cDelta.m_mNewActorDataIdDices)
    {
        int32 idDice = Elem.Key;
        FMyMJGameDiceVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        if (idDice >= m_cActorData.m_aDices.Num())
        {
            m_cActorData.m_aDices.AddDefaulted((idDice - m_cActorData.m_aDices.Num()) + 1);
            MY_VERIFY(idDice == (m_cActorData.m_aDices.Num() - 1));
        }

        m_cActorData.m_aDices[idDice] = cInfoAndResult;

    }

    setTime(uiNewServerWorldTime_ms);
}


FMyMJGameDeskProcessorRunnableCpp::FMyMJGameDeskProcessorRunnableCpp() : FMyRunnableBaseCpp(MY_VISUAL_PROCESSOR_SUB_THREAD_LOOP_TIME_MS),
    m_cCmdIn(TEXT("Processor Cmd In"), MY_VISUAL_PROCESSOR_CMD_IN_BUFFER_SIZE), m_cCmdOut(TEXT("Processor Cmd Out"), MY_VISUAL_PROCESSOR_CMD_OUT_BUFFER_SIZE),
    m_cDataIn(TEXT("Processor Data In"), MY_VISUAL_PROCESSOR_DATA_IN_BUFFER_SIZE), m_cDataOut(TEXT("Processor Data Out"), MY_VISUAL_PROCESSOR_DATA_OUT_BUFFER_SIZE)
{
    m_pSubThreadData = NULL;
    m_pSubThreadDataCoreDataDirtyRecordSincePrevSent = NULL;
    m_pSubThreadAccessor = NULL;

    m_pSubThreadSentLabel = NULL;
    m_bSubThreadHelperSkippedEventBefore = false;

};

FMyMJGameDeskProcessorRunnableCpp::~FMyMJGameDeskProcessorRunnableCpp()
{
};

//return the new key, not take effect now, may need some loop to check subthread report 
uint32 FMyMJGameDeskProcessorRunnableCpp::mainThreadCmdUpdateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache)
{
    m_cMainThreadWaitingToSendCfgCache = cCfgCache;
    m_cMainThreadWaitingToSendCfgCache.m_uiStateKey = m_cMainThreadSentLabel.m_uiCfgStateKey + 1;
    if (m_cMainThreadWaitingToSendCfgCache.m_uiStateKey == MyUIntIdDefaultInvalidValue) {
        //spin over can't happen since buffer size is far smaller, you just let consume don't miss a thing it is OK
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("need reboot!"));
        m_cMainThreadWaitingToSendCfgCache.m_uiStateKey++;
    }

    return m_cMainThreadWaitingToSendCfgCache.m_uiStateKey;
};


bool FMyMJGameDeskProcessorRunnableCpp::mainThreadTryFeedData(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater, bool *pOutHaveFeedData)
{
    if (pOutRetryLater) {
        *pOutRetryLater = false;
    }

    if (pOutHaveFeedData) {
        *pOutHaveFeedData = false;
    }

    if (m_cMainThreadSentLabel.m_uiCfgStateKey == MyUIntIdDefaultInvalidValue) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("visual cfg not updated, check you visual layer code!"));
        return false;
    }

    if (m_cMainThreadReceivedLabel.m_uiCfgStateKey != m_cMainThreadSentLabel.m_uiCfgStateKey) {

        if (pOutRetryLater) {
            *pOutRetryLater = true;
        }
        return false;

    }

    bool bHaveFeedData = false;
    int32 iRet = mainThreadTryFeedEvents(pSeq, &bHaveFeedData);

    uint32 uiServerWorldTime_ms = pSeq->getServerWorldTime_ms();
    //see if we need to update timestamp
    while (iRet == MyTryFeedEventRetAllProcessed && uiServerWorldTime_ms > m_cMainThreadSentLabel.m_uiServerWorldTime_ms) {


        FMyMJGameDeskProcessorDataInputCpp* pIn = m_cDataIn.getItemForProduce();
        if (pIn == NULL) {
            break;
        }

        pIn->reset();
        pIn->m_uiNewServerWorldTime_ms = uiServerWorldTime_ms;

        pIn->verifyValid();
        m_cDataIn.putInProducedItem(pIn);

        m_cMainThreadSentLabel.m_uiServerWorldTime_ms = uiServerWorldTime_ms;

        bHaveFeedData = true;

        break;
    }

    if (pOutHaveFeedData) {
        *pOutHaveFeedData = bHaveFeedData;
    }


    return iRet == MyTryFeedEventRetNeedSyncBase;
};

int32 FMyMJGameDeskProcessorRunnableCpp::mainThreadTryFeedEvents(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutHaveFeedEvent)
{
    MY_VERIFY(IsValid(pSeq));

    if (pOutHaveFeedEvent) {
        *pOutHaveFeedEvent = false;
    }

    uint32 uiServerWorldTime_ms = pSeq->getServerWorldTime_ms();

    if (uiServerWorldTime_ms <= 0) {
        uint32 idEventLast = 0;
        pSeq->getFullAndDeltaLastData(&idEventLast, NULL);
        if (idEventLast > 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got data that serverWorldTime is Zero but idEventLast %d, check your server code!"), idEventLast);
            MY_VERIFY(false);
        }

        return MyTryFeedEventRetAllProcessed;
    }

    MyMJGameRoleTypeCpp eRoleTypeData = pSeq->getRole();
    MyMJGameRoleTypeCpp eRoleTypeSelf = m_cMainThreadSentLabel.m_eRoleType;
    MY_VERIFY(eRoleTypeData != MyMJGameRoleTypeCpp::Max);

    //early out
    //Todo: check this code carefully since it need server's replication code do the right thing
    if (eRoleTypeData == eRoleTypeSelf && uiServerWorldTime_ms <= m_cMainThreadSentLabel.m_uiServerWorldTime_ms) {
        return MyTryFeedEventRetAllProcessed;
    }

    //For simple, we only target at the event queue, ignore whether the base has newer data
    const UMyMJGameEventCycleBuffer* pEvents = pSeq->getDeltaDataEvents(false);
    if (!IsValid(pEvents)) {
        return MyTryFeedEventRetDataReplicationIncomplete;
    }

    if (pEvents->getCount() <= 0) {

        return MyTryFeedEventRetAllProcessed;
    }

    int32 deltaCount = pEvents->getCount();

    uint32 idEventDeltaFirst = pEvents->peekRefAt(0).getIdEvent();
    uint32 idEventDeltaLast = pEvents->peekRefAt(deltaCount - 1).getIdEvent();
    uint32 idEventSelf = m_cMainThreadSentLabel.m_uiIdEvent;


    bool bNeedFullData = false;
    uint32 idEventFirstToPick = 0, eventCountToPick = 0;
    if (idEventSelf == MyUIntIdDefaultInvalidValue) {
        MY_VERIFY(eRoleTypeSelf == MyMJGameRoleTypeCpp::Max);
        //We haven't got any data before
        if (idEventDeltaFirst == (MyUIntIdDefaultInvalidValue + 1)) {
            //but delta have just next one

        }
        else {
            bNeedFullData = true;
        }

        idEventFirstToPick = idEventDeltaFirst;
        eventCountToPick = deltaCount;
    }
    else {

        //check whether role changed
        if (eRoleTypeSelf != MyMJGameRoleTypeCpp::Max && eRoleTypeSelf != eRoleTypeData) {
            bNeedFullData = true;
        }

        //we have got data before, seek next one
        uint32 idEventNext = idEventSelf + 1;
        if (idEventNext < idEventDeltaFirst) {
            //we have data skipped

            bNeedFullData = true;
            idEventFirstToPick = idEventDeltaFirst;
            eventCountToPick = deltaCount;
        }
        else {
            //note it is possible idxStartOfDeltaToSend > idxEndOfDeltaToSend, means no new data
            idEventFirstToPick = idEventNext;
            eventCountToPick = idEventDeltaLast >= idEventFirstToPick ? (idEventDeltaLast - idEventFirstToPick) + 1 : 0;
        }
    }

    if (eventCountToPick <= 0) {
        //no new data
        return MyTryFeedEventRetAllProcessed;
    }

    if (bNeedFullData) {
        //check if we have a valid base matching it

        const FMyMJDataStructWithTimeStampBaseCpp& cFullData = pSeq->getFullData();

        //debug
        /*
        int32 err = cFullData.checkPrivateDataInExpect();
        if (err != 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("full data check fail, err %d."), err);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("full data check OK."));
        }
        */

        if ((cFullData.getIdEventApplied() + 1) < idEventFirstToPick) {
            //we don't have a valid base
            return MyTryFeedEventRetNeedSyncBase;
        }

        //OK we have a valid base, calculate how many we can deliver
        idEventFirstToPick = cFullData.getIdEventApplied() + 1;
        eventCountToPick = idEventDeltaLast >= idEventFirstToPick ? (idEventDeltaLast - idEventFirstToPick) + 1 : 0;

        FMyMJGameDeskProcessorDataInputCpp* pIn = m_cDataIn.getItemForProduce();
        if (pIn == NULL) {
            return MyTryFeedEventRetLackBuffer;
        }

        pIn->reset();
        MY_VERIFY(pIn->m_apNewFullData.Emplace() == 0);
        pIn->m_apNewFullData[0] = cFullData;
        pIn->m_uiNewServerWorldTime_ms = uiServerWorldTime_ms;

        pIn->verifyValid();
        m_cDataIn.putInProducedItem(pIn);

        MY_VERIFY(cFullData.getTime_ms() > 0);
        m_cMainThreadSentLabel.updateAfterEventAdded(eRoleTypeData, cFullData.getIdEventApplied(), uiServerWorldTime_ms);

        if (pOutHaveFeedEvent) {
            *pOutHaveFeedEvent = true;
        }
    }


    MY_VERIFY(idEventFirstToPick >= idEventDeltaFirst);

    uint32 idxStart = idEventFirstToPick - idEventDeltaFirst;
    for (uint32 i = 0; i < eventCountToPick; i++) {
        uint32 idx = idxStart + i;
        MY_VERIFY(idx < (uint32)(1 << 31));

        FMyMJGameDeskProcessorDataInputCpp* pIn = m_cDataIn.getItemForProduce();
        if (pIn == NULL) {
            return MyTryFeedEventRetLackBuffer;
        }

        const FMyMJEventWithTimeStampBaseCpp& event = pEvents->peekRefAt(idx);

        //debug
        /*
        const FMyMJGamePusherResultCpp* pPusherResult = event.getPusherResult(false);
        if (pPusherResult) {
            if (pPusherResult->m_aResultBase.Num() > 0) {
                int32 err = pPusherResult->m_aResultBase[0].checkPrivateDataInExpect();
                if (err != 0) {
                    UE_MY_LOG(LogMyUtilsInstance, Error,   TEXT("event with base check fail, err %d."), err);
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("event with base check OK."));
                }
            }
            else {
                MY_VERIFY(pPusherResult->m_aResultDelta.Num() == 1);
                const FMyMJDataDeltaCpp& delta = pPusherResult->m_aResultDelta[0];
                if (delta.m_aRoleDataPrivate.Num() > 0) {
                    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("debug: giveing delta event with revealing values, event %s, %s."), *event.genDebugMsg(),
                    *UMyMJUtilsLibrary::formatStrIdValuePairs(delta.m_aRoleDataPrivate[0].m_aIdValuePairs2Reveal));
                }
            }
        }
        */

        pIn->reset();
        MY_VERIFY(pIn->m_apNewDeltaEvent.Emplace() == 0);
        pIn->m_apNewDeltaEvent[0] = event;
        pIn->m_uiNewServerWorldTime_ms = uiServerWorldTime_ms;

        pIn->verifyValid();
        m_cDataIn.putInProducedItem(pIn);

        m_cMainThreadSentLabel.updateAfterEventAdded(eRoleTypeData, event.getIdEvent(), uiServerWorldTime_ms);

        if (pOutHaveFeedEvent) {
            *pOutHaveFeedEvent = true;
        }
    }

    return MyTryFeedEventRetAllProcessed;
};


void FMyMJGameDeskProcessorRunnableCpp::mainThreadCmdLoop()
{
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdInputCpp>&  cCmdIn = m_cCmdIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdOutputCpp>& cCmdOut = m_cCmdOut;

    //receive
    while (1)
    {
        FMyMJGameDeskProcessorCmdOutputCpp *pOut = cCmdOut.getItemForConsume();
        if (pOut == NULL) {
            break;
        }

        if (pOut->m_apNewCfgCache.Num() > 0) {
            m_cMainThreadReceivedLabel.m_uiCfgStateKey = pOut->m_apNewCfgCache[0].m_uiStateKey;
        }

        //cCmdOut.putInConsumedItem(pOut);
        m_cCmdOutBufferForExt.Enqueue(pOut);
    }

    //send
    while (1)
    {
        if (m_cMainThreadSentLabel.m_uiCfgStateKey != m_cMainThreadWaitingToSendCfgCache.m_uiStateKey) {

            FMyMJGameDeskProcessorCmdInputCpp *pIn = cCmdIn.getItemForProduce();
            if (pIn == NULL) {
                break;
            }

            pIn->reset();
            pIn->m_apNewCfgCache.Emplace();
            pIn->m_apNewCfgCache[0] = m_cMainThreadWaitingToSendCfgCache;

            pIn->verifyValid();
            cCmdIn.putInProducedItem(pIn);

            m_cMainThreadSentLabel.m_uiCfgStateKey = m_cMainThreadWaitingToSendCfgCache.m_uiStateKey;
        }

        break;
    }
};

void FMyMJGameDeskProcessorRunnableCpp::mainThreadDataLoop()
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("processor: mainThreadDataLoop"));
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataOutputCpp>& cDataOut = m_cDataOut;

    //receive
    while (1)
    {
        FMyMJGameDeskProcessorDataOutputCpp *pOut = cDataOut.getItemForConsume();
        if (pOut == NULL) {
            break;
        }

        bool bUpdatedLabel = false;
        if (pOut->m_apNewVisualDataDelta.Num() > 0) {
            FMyMJGameDeskVisualDataDeltaCpp& delta = pOut->m_apNewVisualDataDelta[0];
            if (delta.m_apNewCoreData.Num() > 0) {
                FMyMJDataStructWithTimeStampBaseCpp& cFull = delta.m_apNewCoreData[0];
                m_cMainThreadReceivedLabel.updateAfterEventAdded(cFull.getRole(), cFull.getIdEventApplied(), pOut->m_uiNewServerWorldTime_ms);
                bUpdatedLabel = true;
            }
        }

        if (!bUpdatedLabel) {
            m_cMainThreadReceivedLabel.updateServerWorldTime(pOut->m_uiNewServerWorldTime_ms);
        }

        //cDataOut.putInConsumedItem(pOut);
        m_cDataOutBufferForExt.Enqueue(pOut);
    }
};


void FMyMJGameDeskProcessorRunnableCpp::subThreadCmdLoop()
{
    //recieve
    while (1)
    {
        FMyMJGameDeskProcessorCmdInputCpp *pIn = m_cCmdIn.getItemForConsume();
        if (pIn == NULL) {
            break;
        }

        if (pIn->m_apNewCfgCache.Num() > 0) {
            m_pSubThreadData->setCfg(pIn->m_apNewCfgCache[0]);
        }

        m_cCmdIn.putInConsumedItem(pIn);
    }

    //send
    while (1)
    {
        if (m_pSubThreadSentLabel->m_cVisualData.getCfgRefConst().m_uiStateKey != m_pSubThreadData->getCfgRefConst().m_uiStateKey) {
            FMyMJGameDeskProcessorCmdOutputCpp *pOut = m_cCmdOut.getItemForProduce();
            if (pOut == NULL) {
                break;
            }

            pOut->reset();
            pOut->m_apNewCfgCache.Emplace();
            pOut->m_apNewCfgCache[0] = m_pSubThreadData->getCfgRefConst();

            pOut->verifyValid();
            m_cCmdOut.putInProducedItem(pOut);

            m_pSubThreadSentLabel->m_cVisualData.setCfg(m_pSubThreadData->getCfgRefConst());
        }

        break;
    }
};


#define iNeedOutPutNone (0)
#define iNeedOutPutPending (1)
#define iNeedOutPutForCfg (11)

void FMyMJGameDeskProcessorRunnableCpp::subThreadDataLoop()
{
    //receive
    while (1)
    {
        //you got new data, always process it!
        FMyMJGameDeskProcessorDataInputCpp *pInData = m_cDataIn.getItemForConsume();
        if (pInData == NULL) {
            break;
        }

        bool bNeedOutPut = false;
        FMyMJEventWithTimeStampBaseCpp* pImportantEventJustApplied = NULL;

        if (pInData->m_apNewFullData.Num() > 0) {
            FMyMJDataStructWithTimeStampBaseCpp& cFullState = pInData->m_apNewFullData[0];

            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("processor: %s"), *cFullState.genDebugMsg());

            m_pSubThreadData->getCoreDataRef() = cFullState;
            m_pSubThreadAccessor->helperSetCoreDataDirtyRecordAllDirty(*m_pSubThreadDataCoreDataDirtyRecordSincePrevSent);

            bNeedOutPut = true;
        }
        else if (pInData->m_apNewDeltaEvent.Num() > 0) {
            FMyMJEventWithTimeStampBaseCpp& cDeltaEvent = pInData->m_apNewDeltaEvent[0];

            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("processor: %s"), *cDeltaEvent.genDebugMsg());

            m_pSubThreadData->getCoreDataRef().applyEvent(*m_pSubThreadAccessor, cDeltaEvent, m_pSubThreadDataCoreDataDirtyRecordSincePrevSent);

            if (cDeltaEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult) {
            }
            else if (cDeltaEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::Trival) {
            }
            else {
                MY_VERIFY(false);  
            }

            //two type of delta need gen out, one is have dur, other is core base set
            if (cDeltaEvent.getDuration_ms() > 0) {
                bNeedOutPut = true;
                pImportantEventJustApplied = &cDeltaEvent;
            }
            else if (cDeltaEvent.getPusherResult(false) != NULL && cDeltaEvent.getPusherResult(false)->m_aResultBase.Num() > 0) {
                bNeedOutPut = true;
            }
        }
        else {

        }

        if (pInData->m_uiNewServerWorldTime_ms < m_pSubThreadData->getTime()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw, pInData->m_uiNewServerWorldTime_ms %u, m_pSubThreadData->m_uiServerWorldTime_ms %u."), pInData->m_uiNewServerWorldTime_ms, m_pSubThreadData->getTime());
            MY_VERIFY(false);
        }
        m_pSubThreadData->setTime(pInData->m_uiNewServerWorldTime_ms);

        if (bNeedOutPut) {
            int32 iErrorCode = subThreadTryGenOutput(m_bSubThreadHelperSkippedEventBefore? NULL : pImportantEventJustApplied);
            if (iErrorCode != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("failed to gen out for important thing, mark it now, iErrorCode %d."), iErrorCode);
                m_bSubThreadHelperSkippedEventBefore = true;
            }
        }

        m_cDataIn.putInConsumedItem(pInData);
    }


    //send
    subThreadTryGenOutput(NULL);
};

int32 FMyMJGameDeskProcessorRunnableCpp::subThreadTryGenOutput(FMyMJEventWithTimeStampBaseCpp* pEventJustApplied)
{
    if (m_pSubThreadSentLabel->m_cVisualData.getTime() >= m_pSubThreadData->getTime() && m_pSubThreadDataCoreDataDirtyRecordSincePrevSent->isEmpty() &&
        m_pSubThreadData->getCoreDataRefConst().getIdEventApplied() <= m_pSubThreadSentLabel->m_cVisualData.getCoreDataRefConst().getIdEventApplied()) {
        //Time not moved forward and important data is not dirty
        return -1;
    }

    FMyMJGameDeskProcessorDataOutputCpp *pOut = m_cDataOut.getItemForProduce();
    if (pOut == NULL) {
        return -2;
    }

    MY_VERIFY(m_pSubThreadSentLabel->m_cVisualData.getCfgRefConst().m_uiStateKey == m_pSubThreadData->getCfgRefConst().m_uiStateKey);
    MY_VERIFY(m_pSubThreadData->getCoreDataRefConst().getIdEventApplied() >= m_pSubThreadSentLabel->m_cVisualData.getCoreDataRefConst().getIdEventApplied());

    pOut->reset();

    if (m_pSubThreadData->getCoreDataRefConst().getIdEventApplied() > m_pSubThreadSentLabel->m_cVisualData.getCoreDataRefConst().getIdEventApplied()) {

        //core data changed
        pOut->m_apNewVisualDataDelta.Emplace();
        MY_VERIFY(pOut->m_apNewVisualDataDelta.Num() == 1);
        FMyMJGameDeskVisualDataDeltaCpp* pOutDataDelta = &pOut->m_apNewVisualDataDelta[0];

        pOutDataDelta->m_apNewCoreData.Emplace();
        MY_VERIFY(pOutDataDelta->m_apNewCoreData.Num() == 1);
        pOutDataDelta->m_apNewCoreData[0] = m_pSubThreadData->getCoreDataRefConst();

        pOutDataDelta->m_apNewCoreDataDirtyRecord.Emplace();
        MY_VERIFY(pOutDataDelta->m_apNewCoreDataDirtyRecord.Num() == 1);
        pOutDataDelta->m_apNewCoreDataDirtyRecord[0] = *m_pSubThreadDataCoreDataDirtyRecordSincePrevSent;

        
        TMap<int32, FMyMJGameCardVisualInfoCpp> mIdCardVisualInfoAccumulatedChanges;
        TMap<int32, int32> mIdDiceValueAccumulatedChanges;

        helperResolveVisualInfoChanges(m_pSubThreadData->getCfgRefConst(), m_pSubThreadData->getCoreDataRefConst(), *m_pSubThreadDataCoreDataDirtyRecordSincePrevSent, m_pSubThreadSentLabel->m_cVisualData.getActorDataRefConst(),
                                       mIdCardVisualInfoAccumulatedChanges, mIdDiceValueAccumulatedChanges);

        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("delta gen in progress, dirty %d, idCard map num %d, %d, idDice map num %d, %d."), !pOutDataDelta->m_apNewCoreDataDirtyRecord[0].isEmpty(),
                 //mIdCardVisualInfoAccumulatedChanges.Num(), pOutDataDelta->m_mNewActorDataIdCards.Num(), mIdDiceValueAccumulatedChanges.Num(), pOutDataDelta->m_mNewActorDataIdDices.Num());

        helperResolveVisualResultChanges(m_pSubThreadData->getCfgRefConst(), mIdCardVisualInfoAccumulatedChanges, mIdDiceValueAccumulatedChanges, pOutDataDelta->m_mNewActorDataIdCards, pOutDataDelta->m_mNewActorDataIdDices);

        if (pEventJustApplied) {
            MY_VERIFY(!m_bSubThreadHelperSkippedEventBefore);
            pOutDataDelta->m_apEventJustApplied.Emplace();
            MY_VERIFY(pOutDataDelta->m_apEventJustApplied.Num() == 1);
            pOutDataDelta->m_apEventJustApplied[0] = *pEventJustApplied;
        }
        pOutDataDelta->m_bHelperSkippedEventBefore = m_bSubThreadHelperSkippedEventBefore;

        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("delta output gened, dirty %d, idCard map num %d, %d, idDice map num %d, %d."), !pOutDataDelta->m_apNewCoreDataDirtyRecord[0].isEmpty(),
            //mIdCardVisualInfoAccumulatedChanges.Num(), pOutDataDelta->m_mNewActorDataIdCards.Num(), mIdDiceValueAccumulatedChanges.Num(), pOutDataDelta->m_mNewActorDataIdDices.Num());

        //clear up subthread records
        m_pSubThreadDataCoreDataDirtyRecordSincePrevSent->reset();
        m_bSubThreadHelperSkippedEventBefore = false;

    }

    pOut->m_uiNewServerWorldTime_ms = m_pSubThreadData->getTime();

    //update sent label
    FMyMJGameDeskProcessorDataOutputCpp::helperApplyToDeskVisualData(m_pSubThreadSentLabel->m_cVisualData, *pOut);

    pOut->verifyValid();
    m_cDataOut.putInProducedItem(pOut);

    return 0;

};

bool FMyMJGameDeskProcessorRunnableCpp::subThreadInitBeforRun()
{
    if (!FMyRunnableBaseCpp::subThreadInitBeforRun()) {
        return false;
    }

    m_pSubThreadData = new FMyMJGameDeskVisualDataCpp();
    m_pSubThreadDataCoreDataDirtyRecordSincePrevSent = new FMyDirtyRecordWithKeyAnd4IdxsMapCpp();
    m_pSubThreadAccessor = new FMyMJDataAccessorCpp();

    m_pSubThreadAccessor->setupDataExt(&m_pSubThreadData->getCoreDataRef());

    m_pSubThreadSentLabel = new FMyMJGameDeskProcessorSubThreadSentLabelCpp();

    m_bSubThreadHelperSkippedEventBefore = false;
    return true;
};

void FMyMJGameDeskProcessorRunnableCpp::subThreadLoopInRun()
{
    subThreadCmdLoop();
    subThreadDataLoop();

}

void FMyMJGameDeskProcessorRunnableCpp::subThreadExitAfterRun()
{
    FMyRunnableBaseCpp::subThreadExitAfterRun();

    delete(m_pSubThreadData);
    m_pSubThreadData = NULL;
    delete(m_pSubThreadDataCoreDataDirtyRecordSincePrevSent);
    m_pSubThreadDataCoreDataDirtyRecordSincePrevSent = NULL;
    delete(m_pSubThreadAccessor);
    m_pSubThreadAccessor = NULL;
    delete(m_pSubThreadSentLabel);
    m_pSubThreadSentLabel = NULL;

};

void FMyMJGameDeskProcessorRunnableCpp::helperResolveVisualInfoChanges(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                                                        const FMyMJDataStructWithTimeStampBaseCpp& cNextCoreData,
                                                                        const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cNextCoreDataDirtyRecordSincePrev,
                                                                        const FMyMJGameDeskVisualActorDatasCpp& cPrevActorData,
                                                                        TMap<int32, FMyMJGameCardVisualInfoCpp>& mOutIdCardVisualInfoAccumulatedChanges,
                                                                        TMap<int32, int32>& mOutIdDiceValueAccumulatedChanges)
{
    const FMyMJGameDeskVisualCfgCacheCpp& cInVisualCfgCache = cCfgCache;
    const FMyMJDataStructWithTimeStampBaseCpp& cInBase = cNextCoreData;
    const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cInBaseDirtyRecord = cNextCoreDataDirtyRecordSincePrev;

    if (cInBase.getRole() == MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Early out since base haven't been set yet, maybe since a trival event arrived before."));
        return;
    }
    //MY_VERIFY((uint8)cInBase.getRole() < (uint8)MyMJGameRoleTypeCpp::Max);

    const FMyMJCardInfoPackCpp& cCardInfoPack = cInBase.getCoreDataPublicRefConst().m_cCardInfoPack;
    const FMyMJCardValuePackCpp& ccardValuePack = cInBase.getRoleDataPrivateRefConst().m_cCardValuePack;

    if (cCardInfoPack.getLength() != ccardValuePack.getLength()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pack length not equal: info pack %d, value pack %d, accces role type %d."), cCardInfoPack.getLength(), ccardValuePack.getLength(), (uint8)cInBase.getRole());
        MY_VERIFY(false);
    }

    TMap<int32, FMyMJGameCardVisualInfoCpp> mIdCardVisualInfoTemp;
    TMap<int32, int32> mIdDiceValueTemp;

    const TSet<int32>& sD = cInBaseDirtyRecord.getRecordSet();
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("sd Num %d, in empty %d."), sD.Num(), cNextCoreDataDirtyRecordSincePrev.isEmpty());

    for (auto& Elem : sD)
    {
        int32 v = Elem;

        MyMJGameCoreDataDirtyMainTypeCpp eMainType;
        int32 subIdx0, subIdx1, subIdx2;
        cInBaseDirtyRecord.recordValueToIdxValuesWith3Idxs(v, subIdx0, subIdx1, subIdx2);
        eMainType = MyMJGameCoreDataDirtyMainTypeCpp(subIdx0);

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("process dirty map %d %d %d."), subIdx0, subIdx1, subIdx2);

        if (eMainType == MyMJGameCoreDataDirtyMainTypeCpp::Card) {
            int32 idxAttender = subIdx1;
            MyMJCardSlotTypeCpp eSlot = (MyMJCardSlotTypeCpp)subIdx2;

            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("process dirty card slot %d, %d"), idxAttender, (uint8)eSlot);

            const FMyMJRoleDataAttenderPublicCpp& attenderPublic = cInBase.getRoleDataAttenderPublicRefConst(idxAttender);
            FMyMJGameDeskVisualPointCfgCpp cVisualPointCfg;
            int32 iColPerRow;
            MY_VERIFY(cInVisualCfgCache.helperGetColCountPerRow(idxAttender, eSlot, cVisualPointCfg, iColPerRow) == 0);

            if (eSlot == MyMJCardSlotTypeCpp::Untaken) {

                const TArray<FMyIdCollectionCpp>& aUntakenCardStacks = cInBase.getCoreDataPublicRefConst().m_aUntakenCardStacks;
                const FMyMJGameUntakenSlotSubSegmentInfoCpp& cSubSegmengInfo = attenderPublic.m_cUntakenSlotSubSegmentInfo;

                int32 idxUntakenStackEnd = cSubSegmengInfo.m_iIdxStart + cSubSegmengInfo.m_iLength;

                MY_VERIFY(cSubSegmengInfo.m_iIdxStart >= 0 && cSubSegmengInfo.m_iIdxStart < aUntakenCardStacks.Num());
                MY_VERIFY(idxUntakenStackEnd >= 0 && idxUntakenStackEnd <= aUntakenCardStacks.Num());

                for (int32 idxUntakenStack = cSubSegmengInfo.m_iIdxStart; idxUntakenStack < idxUntakenStackEnd; idxUntakenStack++) {
                    const FMyIdCollectionCpp& stack = aUntakenCardStacks[idxUntakenStack];

                    for (int32 i = 0; i < stack.m_aIds.Num(); i++) {
                        int32 cardId = stack.m_aIds[i];
                        const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                        int32 cardValue = ccardValuePack.getByIdx(cardId);

                        FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);

                        pCardVisualInfo->reset();
                        pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                        pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                        pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                        if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                        }
                        MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::Untaken);

                        pCardVisualInfo->m_iIdxRow = 0;
                        pCardVisualInfo->m_iIdxColInRow = cCardInfo.m_cPosi.m_iIdxInSlot0 - attenderPublic.m_cUntakenSlotSubSegmentInfo.m_iIdxStart;
                        pCardVisualInfo->m_iIdxStackInCol = cCardInfo.m_cPosi.m_iIdxInSlot1;

                        pCardVisualInfo->m_iCardValue = cardValue;

                    }
                }
            }
            else if (eSlot == MyMJCardSlotTypeCpp::JustTaken) {

                const TArray<int32>& aIdHandCards = attenderPublic.m_aIdHandCards;
                const TArray<int32>& aIdJustTakenCards = attenderPublic.m_aIdJustTakenCards;
                int32 l0 = aIdHandCards.Num();
                int32 l1 = aIdJustTakenCards.Num();

                int32 emptyColHalf = (iColPerRow - l0 - l1) / 2;
                for (int32 i = 0; i < l1; i++) {
                    int32 cardId = aIdJustTakenCards[i];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::JustTaken);

                    pCardVisualInfo->m_iIdxRow = 0;
                    pCardVisualInfo->m_iIdxColInRow = l0 + i;
                    pCardVisualInfo->m_iIdxStackInCol = 0;

                    pCardVisualInfo->m_iColInRowExtraMarginCount = 1;

                    pCardVisualInfo->m_iCardValue = cardValue;

                    //trick for mid alignment
                    if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid && (pCardVisualInfo->m_iIdxColInRow + emptyColHalf) >= 0) {
                        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("fixing idx col for Mid alignment %d->%d"), pCardVisualInfo->m_iIdxColInRow, pCardVisualInfo->m_iIdxColInRow + emptyColHalf);
                        pCardVisualInfo->m_iIdxColInRow += emptyColHalf;
                    }
                }
            }
            else if (eSlot == MyMJCardSlotTypeCpp::InHand) {

                const TArray<int32>& aIdHandCards = attenderPublic.m_aIdHandCards;
                const TArray<int32>& aIdJustTakenCards = attenderPublic.m_aIdJustTakenCards;
                int32 l0 = aIdHandCards.Num();
                int32 l1 = aIdJustTakenCards.Num();

                //int32 debugCheckingIdx = 36;
                //int32 debugValue = 0;

                FMyMJValueIdMapCpp m_cSortCards;
                //m_cSortCards.setDebugInfo(1, FString::Printf(TEXT("%d's handcard"), idxAttender));
                m_cSortCards.changeKeepOrder(true, false);

                for (int32 i = 0; i < l0; i++) {
                    int32 cardId = aIdHandCards[i];
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    //if (cardId == debugCheckingIdx) {
                    //    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("step 0: card [%d: %d]."), debugCheckingIdx, cardValue);
                    //    debugValue = cardValue;
                    //}

                    MY_VERIFY(m_cSortCards.insert(cardId, cardValue));
                }
                MY_VERIFY(m_cSortCards.getCount() == l0);
                TArray<FMyIdValuePair> aPairs;
                m_cSortCards.collectAllWithValue(aPairs);
                MY_VERIFY(aPairs.Num() == l0);

                int32 emptyColHalf = (iColPerRow - l0 - l1) / 2;
                for (int32 i = 0; i < l0; i++) {
                    int32 cardId = aPairs[i].m_iId;
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = aPairs[i].m_iValue;

                    //if (cardId == debugCheckingIdx) {
                    //    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("step 1: card [%d: %d]."), debugCheckingIdx, cardValue);
                    //    if (cardValue != debugValue) {
                    //        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("got error!"), debugCheckingIdx, cardValue);
                    //        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_cSortCards: %s."), *m_cSortCards.dump());
                    //        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("aPairs: %s."), *UMyMJUtilsLibrary::formatStrIdValuePairs(aPairs));
                    //    }
                    //}

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::InHand);

                    pCardVisualInfo->m_iIdxRow = 0;
                    pCardVisualInfo->m_iIdxColInRow = i;
                    pCardVisualInfo->m_iIdxStackInCol = 0;

                    pCardVisualInfo->m_iCardValue = cardValue;

                    //trick for mid alignment
                    if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid && (pCardVisualInfo->m_iIdxColInRow + emptyColHalf) >= 0) {
                        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("fixing idx col for Mid alignment %d->%d"), pCardVisualInfo->m_iIdxColInRow, pCardVisualInfo->m_iIdxColInRow + emptyColHalf);
                        pCardVisualInfo->m_iIdxColInRow += emptyColHalf;
                    }
                }
            }
            else if (eSlot == MyMJCardSlotTypeCpp::GivenOut) {

                const TArray<int32>& aIdGivenOutCards = attenderPublic.m_aIdGivenOutCards;
                int32 l = aIdGivenOutCards.Num();

                for (int32 i = 0; i < l; i++) {
                    int32 cardId = aIdGivenOutCards[i];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::GivenOut);

                    pCardVisualInfo->m_iIdxRow = i / iColPerRow;
                    pCardVisualInfo->m_iIdxColInRow = i % iColPerRow;
                    pCardVisualInfo->m_iIdxStackInCol = 0;

                    pCardVisualInfo->m_iCardValue = cardValue;
                }
            }
            else if (eSlot == MyMJCardSlotTypeCpp::Weaved) {

                const TArray<FMyMJWeaveCpp>& aWeaves = attenderPublic.m_aShowedOutWeaves;
                int32 l = aWeaves.Num();

                int32 cardArrangedCount = 0;
                int32 cardRotatedX90DCount = 0;
                int32 weaveArrangedCount = 0;
                for (int32 idxWeave = 0; idxWeave < l; idxWeave++) {
                    const FMyMJWeaveCpp& cWeave = aWeaves[idxWeave];
                    const TArray<int32>& aIds = cWeave.getIdsRefConst();
                    int32 l0 = aIds.Num();

                    int32 triggerCardX90D = 0;
                    if (cWeave.getIdxAttenderTriggerCardSrc() >= 0) {
                        if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAttender + 1) % 4)) {
                            //aIds.
                            triggerCardX90D = 1;
                        }
                        else if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAttender + 2) % 4)) {
                            triggerCardX90D = 1;
                        }
                        else if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAttender + 3) % 4)) {
                            triggerCardX90D = -1;
                        }
                        else {
                        }
                    }

                    int32 workingIdx = 0;
                    if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Left) {
                    }
                    else if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
                        workingIdx = l0 - 1;
                    }
                    else {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected visualPointCfg: idxAttender %d, slot weave, m_eColInRowAlignment %d."), idxAttender, (uint8)cVisualPointCfg.m_eColInRowAlignment);
                        continue;
                    }

                    while (1)
                    {
                        int32 cardId = aIds[workingIdx];
                        const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                        int32 cardValue = ccardValuePack.getByIdx(cardId);

                        FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);
                        pCardVisualInfo->reset();
                        pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                        pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                        pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                        if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                        }
                        MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::Weaved);

                        pCardVisualInfo->m_iIdxRow = 0;
                        pCardVisualInfo->m_iIdxColInRow = cardArrangedCount;
                        pCardVisualInfo->m_iIdxStackInCol = 0;

                        pCardVisualInfo->m_iRotateX90DBeforeCount = cardRotatedX90DCount;
                        pCardVisualInfo->m_iColInRowExtraMarginCount = weaveArrangedCount;
                        if (triggerCardX90D != 0 && cardId == cWeave.getIdTriggerCard()) {
                            pCardVisualInfo->m_iRotateX90D = triggerCardX90D;
                            cardRotatedX90DCount++;
                        }

                        pCardVisualInfo->m_iCardValue = cardValue;
                        cardArrangedCount++;

                        if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
                            workingIdx--;
                            if (workingIdx < 0) {
                                break;
                            }
                        }
                        else {
                            workingIdx++;
                            if (workingIdx >= l0) {
                                break;
                            }
                        }
                    }
                    weaveArrangedCount++;

                }

            }
            else if (eSlot == MyMJCardSlotTypeCpp::WinSymbol) {

                const TArray<int32>& aIdWinSymbolCards = attenderPublic.m_aIdWinSymbolCards;
                int32 l = aIdWinSymbolCards.Num();

                for (int32 i = 0; i < l; i++) {
                    int32 cardId = aIdWinSymbolCards[i];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::WinSymbol);

                    pCardVisualInfo->m_iIdxRow = i / iColPerRow;
                    pCardVisualInfo->m_iIdxColInRow = i % iColPerRow;
                    pCardVisualInfo->m_iIdxStackInCol = 0;

                    pCardVisualInfo->m_iCardValue = cardValue;
                }
            }
            else if (eSlot == MyMJCardSlotTypeCpp::ShownOnDesktop) {

                const TArray<int32>& aIdShownOnDesktopCards = attenderPublic.m_aIdShownOnDesktopCards;
                int32 l = aIdShownOnDesktopCards.Num();

                for (int32 i = 0; i < l; i++) {
                    int32 cardId = aIdShownOnDesktopCards[i];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = &mIdCardVisualInfoTemp.FindOrAdd(cCardInfo.m_iId);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::ShownOnDesktop);

                    pCardVisualInfo->m_iIdxRow = 0;
                    pCardVisualInfo->m_iIdxColInRow = i;
                    pCardVisualInfo->m_iIdxStackInCol = 0;

                    pCardVisualInfo->m_iCardValue = cardValue;
                }
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unsupported slot type: %d."), (uint8)eSlot);
                MY_VERIFY(false);
            }

        }
        else if (eMainType == MyMJGameCoreDataDirtyMainTypeCpp::Dice) {
 
            int32 diceNumerValue0 = UMyMJUtilsLibrary::getIntValueFromBitMask(cInBase.getCoreDataPublicRefConst().m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen);
            int32 diceNumerValue1 = UMyMJUtilsLibrary::getIntValueFromBitMask(cInBase.getCoreDataPublicRefConst().m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen);

            mIdDiceValueTemp.FindOrAdd(0) = diceNumerValue0;
            mIdDiceValueTemp.FindOrAdd(1) = diceNumerValue1;
        }
        else {

            //we allow other ditry map not affect actor data
              
            //MY_VERIFY(false);
        }
    }

    //post update
    int32 lPrev;
    const TArray<FMyMJGameCardVisualInfoAndResultCpp> &aCards = cPrevActorData.m_aCards;
    lPrev = aCards.Num();
    for (auto& Elem : mIdCardVisualInfoTemp)
    {
        int32 idCard = Elem.Key;
        FMyMJGameCardVisualInfoCpp& cInfoNew = Elem.Value;

        bool bIsDirty = false;

        if (idCard < lPrev) {
            //check condition
            const FMyMJGameCardVisualInfoCpp& cInfoOld = aCards[idCard].m_cVisualInfo;
            if (cInfoOld.equal(cInfoNew)) {
                bIsDirty = false;
            }
            else {
                bIsDirty = true;
            }
        }
        else {
            bIsDirty = true;
        }

        if (!bIsDirty) {
            continue;
        }

        mOutIdCardVisualInfoAccumulatedChanges.FindOrAdd(idCard) = cInfoNew;
    }

    const TArray<FMyMJGameDiceVisualInfoAndResultCpp> &aDices = cPrevActorData.m_aDices;
    lPrev = aDices.Num();
    for (auto& Elem : mIdDiceValueTemp)
    {
        int32 id = Elem.Key;
        int32 iValueNew = Elem.Value;

        bool bIsDirty = false;

        if (id < lPrev) {
            //check condition
            int32 iValueOld = aDices[id].m_iVisualInfoValue;
            if (iValueNew == iValueOld) {
                bIsDirty = false;
            }
            else {
                bIsDirty = true;
            }
        }
        else {
            bIsDirty = true;
        }

        if (!bIsDirty) {
            continue;
        }

        mOutIdDiceValueAccumulatedChanges.FindOrAdd(id) = iValueNew;
    }
};

#define GetKey_CardVisualInfo(info) (((info.m_iIdxAttender & 0xff) << 8) | (((uint8)info.m_eSlot & 0xff) << 0))
#define GetIdxAttender_CardVisualInfoKey(key) ((key >> 8) & 0xff)
#define GetESlot_CardVisualInfoKey(key)       ((MyMJCardSlotTypeCpp)((key >> 0) & 0xff))

void FMyMJGameDeskProcessorRunnableCpp::helperResolveVisualResultChanges(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                                                            TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoAccumulatedChanges,
                                                                            TMap<int32, int32>& mIdDiceValueAccumulatedChanges,
                                                                            TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultChanges,
                                                                            TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mOutIdDiceVisualInfoAndResultChanges)
{
    mOutIdCardVisualInfoAndResultChanges.Reset();
    mOutIdDiceVisualInfoAndResultChanges.Reset();

    mIdCardVisualInfoAccumulatedChanges.ValueSort([](const FMyMJGameCardVisualInfoCpp& A, const FMyMJGameCardVisualInfoCpp& B) {
        int32 keyA = GetKey_CardVisualInfo(A);
        int32 keyB = GetKey_CardVisualInfo(B);
        return keyA < keyB; // sort strings by length
    });

    TMap<int32, FMyMJGameCardVisualInfoCpp> mWorkingIdCardVisualInfo;
    int32 iWorkingIdCardVisualInfoKey = -1;

    FMyMJGameCardVisualInfoCpp cDebugLast;
    for (auto& Elem : mIdCardVisualInfoAccumulatedChanges)
    {
        int32 idCard = Elem.Key;
        const FMyMJGameCardVisualInfoCpp& cInfo = Elem.Value;

        int32 key = GetKey_CardVisualInfo(cInfo);

        if (mWorkingIdCardVisualInfo.Num() == 0) {
            mWorkingIdCardVisualInfo.Emplace(idCard) = cInfo;
            iWorkingIdCardVisualInfoKey = key;
            cDebugLast = cInfo;
        }
        else {
            if (iWorkingIdCardVisualInfoKey != key && iWorkingIdCardVisualInfoKey >= 0) {
                //handle it
                FMyMJGameDeskVisualPointCfgCpp cVisualPoint;
                if (cCfgCache.m_cPointCfg.getCardVisualPointCfgByIdxAttenderAndSlot(GetIdxAttender_CardVisualInfoKey(iWorkingIdCardVisualInfoKey), GetESlot_CardVisualInfoKey(iWorkingIdCardVisualInfoKey), cVisualPoint) != 0)
                {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("error, iWorkingIdCardVisualInfoKey 0x%x, info %d, %d."), iWorkingIdCardVisualInfoKey, cDebugLast.m_iIdxAttender, (uint8)cDebugLast.m_eSlot);
                    MY_VERIFY(false);
                }
                
                
                helperResolveCardVisualResultChanges(cVisualPoint, cCfgCache.m_cModelInfo.m_cCardModelInfo, mWorkingIdCardVisualInfo, mOutIdCardVisualInfoAndResultChanges);

                mWorkingIdCardVisualInfo.Reset();
                iWorkingIdCardVisualInfoKey = -1;
            }

            mWorkingIdCardVisualInfo.Emplace(idCard) = cInfo;
            iWorkingIdCardVisualInfoKey = key;
        }
    }

    if (iWorkingIdCardVisualInfoKey >= 0) {
        //handle it
        MY_VERIFY(mWorkingIdCardVisualInfo.Num() > 0);
        FMyMJGameDeskVisualPointCfgCpp cVisualPoint;
        MY_VERIFY(cCfgCache.m_cPointCfg.getCardVisualPointCfgByIdxAttenderAndSlot(GetIdxAttender_CardVisualInfoKey(iWorkingIdCardVisualInfoKey), GetESlot_CardVisualInfoKey(iWorkingIdCardVisualInfoKey), cVisualPoint) == 0);
        helperResolveCardVisualResultChanges(cVisualPoint, cCfgCache.m_cModelInfo.m_cCardModelInfo, mWorkingIdCardVisualInfo, mOutIdCardVisualInfoAndResultChanges);

        mWorkingIdCardVisualInfo.Reset();
        iWorkingIdCardVisualInfoKey = -1;
    }
};

void FMyMJGameDeskProcessorRunnableCpp::helperResolveCardVisualResultChanges(const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCfg,
                                                                            const FMyMJGameActorModelInfoBoxCpp& cCardModelInfo,
                                                                            const TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoKnownChanges,
                                                                            TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultAccumulatedChanges)
{

    for (auto& Elem : mIdCardVisualInfoKnownChanges)
    {
        int32 idCard = Elem.Key;
        const FMyMJGameCardVisualInfoCpp& cInfo = Elem.Value;


        FMyMJGameCardVisualInfoAndResultCpp& cNewInfoAndResult = mOutIdCardVisualInfoAndResultAccumulatedChanges.Emplace(idCard);
        cNewInfoAndResult.m_cVisualInfo = cInfo;

        helperResolveCardTransform(cVisualPointCfg, cCardModelInfo, cInfo, cNewInfoAndResult.m_cVisualResult.m_cTransform);

        //post handle

        //Todo: do more about visible check
        cNewInfoAndResult.m_cVisualResult.m_bVisible = true;

    }
}

void FMyMJGameDeskProcessorRunnableCpp::helperResolveCardTransform(const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCfg,
                                                                   const FMyMJGameActorModelInfoBoxCpp& cCardModelInfo,
                                                                   const FMyMJGameCardVisualInfoCpp& cCardVisualInfo,
                                                                   FTransform& outTransform)
{
    const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCenter = cVisualPointCfg;
    const FMyMJGameActorModelInfoBoxCpp& cardModelInfoFinal = cCardModelInfo;

    const FTransform& cTransFormCenter = cVisualPointCenter.m_cCenterPointWorldTransform;
    const FVector& cAreaBoxExtend = cVisualPointCenter.m_cAreaBoxExtendFinal;
    MyMJGameVerticalAlignmentCpp eRowAlignment = cVisualPointCenter.m_eRowAlignment;
    //int32 iRowMaxNum = cVisualPointCenter.m_iRowMaxNum;
    MyMJGameHorizontalAlignmentCpp eColInRowAlignment = cVisualPointCenter.m_eColInRowAlignment;
    //int32 iColInRowMaxNum = cVisualPointCenter.m_iColInRowMaxNum;
    float fColInRowExtraMarginAbs = cVisualPointCenter.m_fColInRowExtraMarginAbs;
    if (fColInRowExtraMarginAbs < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fColInRowExtraMarginAbs is negative: %f, forceing to default."), fColInRowExtraMarginAbs);
        fColInRowExtraMarginAbs = 0;
    }

    if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Invalid || eRowAlignment == MyMJGameVerticalAlignmentCpp::Mid) { //we don't support mid allignment now
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRowAlignment row alignment %d, forceing to default!."), (uint8)eRowAlignment);
        eRowAlignment = MyMJGameVerticalAlignmentCpp::Bottom;
    }
    //if (iRowMaxNum <= 0) {
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iRowMaxNum %d, forceing to default!."), iRowMaxNum);
    //iRowMaxNum = 1;
    //}

    if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid) {
        //trick for middle alignment since col idx have been adjusted before
        eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Left;
    }

    if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Invalid) { //we don't support mid allignment now
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eColInRowAlignment alignment %d, forceing to default!."), (uint8)eColInRowAlignment);
        eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Left;
    }
    //if (iColInRowMaxNum <= 0) {
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iColInRowMaxNum %d, forceing to default!."), iColInRowMaxNum);
    //iColInRowMaxNum = 1;
    //}

    int32 idxRow = cCardVisualInfo.m_iIdxRow;
    int32 idxColInRow = cCardVisualInfo.m_iIdxColInRow;
    int32 idxStackInCol = cCardVisualInfo.m_iIdxStackInCol;
    MyMJCardFlipStateCpp eFlipState = cCardVisualInfo.m_eFlipState;
    int32 iXRotate90D = cCardVisualInfo.m_iRotateX90D;
    int32 iXRotate90DBeforeCount = cCardVisualInfo.m_iRotateX90DBeforeCount;
    int32 iColInRowExtraMarginCount = cCardVisualInfo.m_iColInRowExtraMarginCount;

    MY_VERIFY(idxRow >= 0);
    MY_VERIFY(idxColInRow >= 0);
    //MY_VERIFY(CountOfColInRow > 0);
    MY_VERIFY(idxStackInCol >= 0);

    if (iXRotate90DBeforeCount < 0 || iXRotate90DBeforeCount > idxColInRow) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("illegal iXRotate90DBeforeCount: %d but idxColInRow: %d!"), iXRotate90DBeforeCount, idxColInRow);
        iXRotate90DBeforeCount = 0;
    }

    if (iColInRowExtraMarginCount < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("iColInRowExtraMarginCount is negative: %d, forceing to default."), iColInRowExtraMarginCount);
        iColInRowExtraMarginCount = 0;
    }

    //not really count, but only for location calc
    int32 colNormalCountForLocCalcBefore = idxColInRow - iXRotate90DBeforeCount;
    int32 col90DCountForLocCalcBefore = iXRotate90DBeforeCount;

    //prefix if this
    //if ((iXRotate90D % 2) == 1) {
    //colNormalCountForLocCalcBefore--;
    //col90DCountForLocCalcBefore++;
    //}


    if (eFlipState == MyMJCardFlipStateCpp::Invalid) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eflipstate!"));
        eFlipState = MyMJCardFlipStateCpp::Stand;
    }

    if (cardModelInfoFinal.m_cBoxExtend.IsZero()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cardBoxExtend is zero!"));
    }

    //target model:
    //        ----
    //        |  |
    //        |  |
    //        ----
    //
    // --------
    // |      |
    // --------

    //start the calc
    //our model assume one slot's flip state is by somekind same, that is, they all stand, or all laydown
    //in local system, x's negative axis is facing attender, just use a actor to show the nature case

    //API example:
    //FQuat rotater = cTransFormCenter.GetRotation();
    //localRotatorRelative2StackPoint.Quaternion();
    //localRotatorRelative2StackPoint = UKismetMathLibrary::ComposeRotators(localRotatorRelative2StackPoint, FRotator(0, 0, 0));

    FVector forwardV(1, 0, 0);
    FVector rightV(0, 1, 0);
    FVector upV(0, 0, 1);

    FVector perRowOffsetB2T(0); //bottom to top
    FVector perColOffsetL2R = rightV * cardModelInfoFinal.m_cBoxExtend.Y * 2; //left to right
    FVector perColOffsetX90DL2R = rightV * cardModelInfoFinal.m_cBoxExtend.Z * 2;
    FVector perStackOffsetB2T(0); //bottom to top

    FRotator localRotatorRelative2StackPoint(0);
    //FVector localRotateRelativePivot(0); //remove this since it is not important

    FVector localPointOfAttaching2StackPointRelativeLocation(0); //which will combine to stack point after rotate
    FVector localPointOfAttaching2StackPointRelativeLocationFixL2R(0);
    if (eFlipState == MyMJCardFlipStateCpp::Stand) {
        perRowOffsetB2T = forwardV * cardModelInfoFinal.m_cBoxExtend.X * 2;
        perStackOffsetB2T = upV * cardModelInfoFinal.m_cBoxExtend.Z * 2;

        localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
        localPointOfAttaching2StackPointRelativeLocation.Z -= cardModelInfoFinal.m_cBoxExtend.Z;

        localRotatorRelative2StackPoint.Yaw = 180;

    }
    else if (eFlipState == MyMJCardFlipStateCpp::Up) {
        perRowOffsetB2T = forwardV * cardModelInfoFinal.m_cBoxExtend.Z * 2;
        perStackOffsetB2T = upV * cardModelInfoFinal.m_cBoxExtend.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z -= cardModelInfoFinal.m_cBoxExtend.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z -= 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Pitch = 90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else if (eFlipState == MyMJCardFlipStateCpp::Down) {
        perRowOffsetB2T = forwardV * cardModelInfoFinal.m_cBoxExtend.Z * 2;
        perStackOffsetB2T = upV * cardModelInfoFinal.m_cBoxExtend.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z += cardModelInfoFinal.m_cBoxExtend.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z += 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Pitch = -90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else {
        MY_VERIFY(false);
    }

    FVector rowPoint(0);
    if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Top) {
        rowPoint = forwardV * cAreaBoxExtend.X - perRowOffsetB2T * idxRow;
    }
    else if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Bottom) {
        rowPoint = -forwardV * cAreaBoxExtend.X + perRowOffsetB2T * idxRow;
    }
    else {
        MY_VERIFY(false);
    }

    FVector colPoint(0);
    FVector colOffsetL2R = perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + rightV * (fColInRowExtraMarginAbs * iColInRowExtraMarginCount);
    if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Left) {
        colPoint = rowPoint - rightV * cAreaBoxExtend.Y + colOffsetL2R;
        localPointOfAttaching2StackPointRelativeLocation += localPointOfAttaching2StackPointRelativeLocationFixL2R;
    }
    else if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
        colPoint = rowPoint + rightV * cAreaBoxExtend.Y - colOffsetL2R;
        localPointOfAttaching2StackPointRelativeLocation -= localPointOfAttaching2StackPointRelativeLocationFixL2R;
    }
    else {
        MY_VERIFY(false);
    }

    FVector stackPoint = colPoint + perStackOffsetB2T * idxStackInCol;

    //OK we got local stack point, let's calc card's local transform
    //the interesting thing is that, we just need to cacl the align point after rotate
    FVector offset = localRotatorRelative2StackPoint.RotateVector(localPointOfAttaching2StackPointRelativeLocation);
    FTransform relative2VisualPointTransform;
    relative2VisualPointTransform.SetLocation(stackPoint - offset);
    relative2VisualPointTransform.SetRotation(localRotatorRelative2StackPoint.Quaternion());

    outTransform = relative2VisualPointTransform * cTransFormCenter;

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("relative2VisualPointTransform %s, cTransFormCenter %s, outTransform %s."),
        //*UKismetStringLibrary::Conv_TransformToString(relative2VisualPointTransform),
        //*UKismetStringLibrary::Conv_TransformToString(cTransFormCenter),
        //*UKismetStringLibrary::Conv_TransformToString(outTransform));

    //outLocationWorld = outTransform.GetLocation();
    //outRotatorWorld = outTransform.GetRotation().Rotator();

    return;
};

/*
void UMyMJGameDeskSuiteCpp::helperCalcCardTransform(const FMyMJGameCardVisualInfoCpp& cCardVisualInfo, FTransform &outTransform)
{

    int32 idxAttender = cCardVisualInfo.m_iIdxAttender;
    if (!(idxAttender >= 0 && idxAttender < 4))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idxAttender out of range: %d."), idxAttender);
        return;
    }

    MyMJCardSlotTypeCpp eSlot = cCardVisualInfo.m_eSlot;

    if (eSlot == MyMJCardSlotTypeCpp::Invalid) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eSlot is invalid"));
        return;
    }
    else if (eSlot == MyMJCardSlotTypeCpp::ShownOnDesktop) {

    }
    else {

        MY_VERIFY(IsValid(m_pDeskAreaActor));

        FMyMJGameDeskVisualPointCfgCpp cVisualPointCenter;
        int32 errCode;
        m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlotInternal(idxAttender, eSlot, errCode, cVisualPointCenter);
        MY_VERIFY(errCode == 0);

        const FTransform& cTransFormCenter = cVisualPointCenter.m_cCenterPointTransform;
        const FVector& cAreaBoxExtend = cVisualPointCenter.m_cAreaBoxExtend;
        MyMJGameVerticalAlignmentCpp eRowAlignment = cVisualPointCenter.m_eRowAlignment;
        //int32 iRowMaxNum = cVisualPointCenter.m_iRowMaxNum;
        MyMJGameHorizontalAlignmentCpp eColInRowAlignment = cVisualPointCenter.m_eColInRowAlignment;
        //int32 iColInRowMaxNum = cVisualPointCenter.m_iColInRowMaxNum;
        float fColInRowExtraMarginAbs = cVisualPointCenter.m_fColInRowExtraMarginAbs;
        if (fColInRowExtraMarginAbs < 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fColInRowExtraMarginAbs is negative: %f, forceing to default."), fColInRowExtraMarginAbs);
            fColInRowExtraMarginAbs = 0;
        }

        if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Invalid || eRowAlignment == MyMJGameVerticalAlignmentCpp::Mid) { //we don't support mid allignment now
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRowAlignment row alignment %d, forceing to default!."), (uint8)eRowAlignment);
            eRowAlignment = MyMJGameVerticalAlignmentCpp::Bottom;
        }
        //if (iRowMaxNum <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iRowMaxNum %d, forceing to default!."), iRowMaxNum);
            //iRowMaxNum = 1;
        //}
        if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Invalid || eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid) { //we don't support mid allignment now
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eColInRowAlignment alignment %d, forceing to default!."), (uint8)eColInRowAlignment);
            eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Left;
        }
        //if (iColInRowMaxNum <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iColInRowMaxNum %d, forceing to default!."), iColInRowMaxNum);
            //iColInRowMaxNum = 1;
        //}

        const FVector cardBoxExtend;

        int32 idxRow = cCardVisualInfo.m_iIdxRow;
        int32 idxColInRow = cCardVisualInfo.m_iIdxColInRow;
        int32 idxStackInCol = cCardVisualInfo.m_iIdxStackInCol;
        MyMJCardFlipStateCpp eFlipState = cCardVisualInfo.m_eFlipState;
        int32 iXRotate90D = cCardVisualInfo.m_iRotateX90D;
        int32 iXRotate90DBeforeCount = cCardVisualInfo.m_iRotateX90DBeforeCount;
        int32 iColInRowExtraMarginCount = cCardVisualInfo.m_iColInRowExtraMarginCount;

        MY_VERIFY(idxRow >= 0);
        MY_VERIFY(idxColInRow >= 0);
        //MY_VERIFY(CountOfColInRow > 0);
        MY_VERIFY(idxStackInCol >= 0);

        if (iXRotate90DBeforeCount < 0 || iXRotate90DBeforeCount > idxColInRow) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("illegal iXRotate90DBeforeCount: %d but idxColInRow: %d!"), iXRotate90DBeforeCount, idxColInRow);
            iXRotate90DBeforeCount = 0;
        }

        if (iColInRowExtraMarginCount < 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("iColInRowExtraMarginCount is negative: %d, forceing to default."), iColInRowExtraMarginCount);
            iColInRowExtraMarginCount = 0;
        }

        //not really count, but only for location calc
        int32 colNormalCountForLocCalcBefore = idxColInRow - iXRotate90DBeforeCount;
        int32 col90DCountForLocCalcBefore = iXRotate90DBeforeCount;

        //prefix if this
        //if ((iXRotate90D % 2) == 1) {
            //colNormalCountForLocCalcBefore--;
            //col90DCountForLocCalcBefore++;
        //}


        if (eFlipState == MyMJCardFlipStateCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eflipstate!"));
            eFlipState = MyMJCardFlipStateCpp::Stand;
        }

        if (cardBoxExtend.IsZero()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cardBoxExtend is zero!"));
        }

        //start the calc
        //our model assume one slot's flip state is by somekind same, that is, they all stand, or all laydown
        FVector locationWorldFinal;
        FRotator rotaterWorldFinal;
        FRotator rotaterLocalDelta;

        FQuat rotater = cTransFormCenter.GetRotation();
        FVector forwardV = rotater.GetForwardVector();
        FVector rightV = rotater.GetRightVector();
        FVector upV = rotater.GetUpVector();

        FVector perRowOffsetB2T; //bottom to top
        FVector perColOffsetL2R = -rightV * cardBoxExtend.Y * 2; //left to right
        FVector perColOffsetX90DL2R = -rightV * cardBoxExtend.Z * 2;
        FVector perStackOffsetB2T; //bottom to top

        FVector extraLocOffset;
        FVector extraLocOffsetB2T;
        FVector extraLocOffsetL2R;
        if (eFlipState == MyMJCardFlipStateCpp::Stand) {
            perRowOffsetB2T = -forwardV * cardBoxExtend.X * 2;
            perStackOffsetB2T = upV * cardBoxExtend.Z * 2;


        }
        else if (eFlipState == MyMJCardFlipStateCpp::Up) {
            perRowOffsetB2T = -forwardV * cardBoxExtend.Z * 2;
            perStackOffsetB2T = upV * cardBoxExtend.X * 2;

            rotaterLocalDelta.Yaw = 90;

            //note: when top to down, there may be alignment issue, here we don't solve it now
            if (iXRotate90D > 0) {
                extraLocOffsetB2T = -forwardV * cardBoxExtend.Y;
                extraLocOffsetL2R = -rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = 90;
            }
            else if (iXRotate90D < 0) {
                extraLocOffsetB2T = -forwardV * cardBoxExtend.Y;
                extraLocOffsetL2R = rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = -90;
            }
        }
        else if (eFlipState == MyMJCardFlipStateCpp::Down) {
            perRowOffsetB2T = -forwardV * cardBoxExtend.Z * 2;
            perStackOffsetB2T = upV * cardBoxExtend.X * 2;

            //note: when top to down, there may be alignment issue, here we don't solve it now
            if (iXRotate90D > 0) {
                extraLocOffsetB2T = forwardV * (cardBoxExtend.Y + 2 * cardBoxExtend.Z - 2 * cardBoxExtend.Y);
                extraLocOffsetL2R = rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = 90;
            }
            else if (iXRotate90D < 0) {
                extraLocOffsetB2T = forwardV * (cardBoxExtend.Y + 2 * cardBoxExtend.Z - 2 * cardBoxExtend.Y);
                extraLocOffsetL2R = -rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = -90;
            }


            //cTransFormCenterFix0.SetLocation(cTransFormCenter.GetLocation() - perRowOffsetB2T + perStackOffsetB2T);
            extraLocOffset += -perRowOffsetB2T + perStackOffsetB2T;
        }
        else {
            MY_VERIFY(false);
        }

        //rowBase + perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + fColInRowExtraMarginAbs * iColInRowExtraMarginCount;
        FVector rowCenter;
        if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Top) {
            rowCenter = cTransFormCenter.GetLocation() - forwardV * cAreaBoxExtend.X - perRowOffsetB2T * idxRow - extraLocOffsetB2T;
        }
        else if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Bottom) {
            rowCenter = cTransFormCenter.GetLocation() + forwardV * cAreaBoxExtend.X + perRowOffsetB2T * idxRow + extraLocOffsetB2T;
        }
        else {
            MY_VERIFY(false);
        }
        
        FVector colPivot;
        FVector colOffsetL2R = perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + fColInRowExtraMarginAbs * iColInRowExtraMarginCount + extraLocOffsetL2R;
        if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Left) {
            colPivot = rowCenter - rightV * cAreaBoxExtend.Y + colOffsetL2R;

        }
        else if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
            colPivot = rowCenter + rightV * cAreaBoxExtend.Y - colOffsetL2R;
        }
        else {
            MY_VERIFY(false);
        }
        colPivot += extraLocOffset;

        locationWorldFinal = colPivot + perStackOffsetB2T * idxStackInCol;
        rotaterWorldFinal = UKismetMathLibrary::ComposeRotators(rotaterLocalDelta, FRotator(cTransFormCenter.GetRotation()));

    }

};
*/

/*
void UMyMJGameDeskSuiteCpp::helperResolveTargetCardVisualState(int32 idxCard, FMyMJGameActorVisualResultBaseCpp& outTargetCardVisualState)
{

        outTargetCardVisualState.reset();

        //if (!checkSettings()) {
        //return;
        //}

        MY_VERIFY(idxCard >= 0);

        const FMyMJDataAtOneMomentCpp& cDataNow = m_pCoreWithVisual->getDataNowRefConst();
        const FMyMJDataStructWithTimeStampBaseCpp& cBase = cDataNow.getBaseRefConst();
        const FMyMJCardInfoPackCpp& cCardInfoPack = cBase.getCoreDataPublicRefConst().m_cCardInfoPack;

        if (idxCard >= cCardInfoPack.getLength()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idxCard out of range. %d/%d."), idxCard, cCardInfoPack.getLength());
        return;
        }

        const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(idxCard);
        if (cCardInfo.m_cPosi.m_iIdxAttender < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("card position is not inited, m_iIdxAttender %d."), cCardInfo.m_cPosi.m_iIdxAttender);
        return;
        }

        FMyMJGameDeskVisualPointCfgCpp cVisualPoint;
        if (0 != m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlot(cCardInfo.m_cPosi.m_iIdxAttender, cCardInfo.m_cPosi.m_eSlot, cVisualPoint)) {
        return;
        }

        FMyMJGameCardVisualInfoCpp* pVisualInfo = m_cCardVisualInfoPack.getByIdx(idxCard, false);
        if (pVisualInfo == NULL) {
        return;
        }

        FMyMJGameActorModelInfoBoxCpp cModelInfo;
        if (0 != m_pResManager->getCardModelInfoUnscaled(cModelInfo)) {
        return;
        }

        if (0 != helperCalcCardTransformFromvisualPointCfg(cModelInfo, *pVisualInfo, cVisualPoint, outTargetCardVisualState.m_cTransform)) {
        return;
        }

        outTargetCardVisualState.m_bVisible = pVisualInfo->m_bVisible;
        outTargetCardVisualState.m_iCardValue = pVisualInfo->m_iCardValue;

}
*/


/*
int32 UMyMJGameDeskSuiteCpp::helperCalcCardTransformFromvisualPointCfg(const FMyMJGameActorModelInfoBoxCpp& cardModelInfoFinal, const FMyMJGameCardVisualInfoCpp& cardVisualInfoFinal, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FTransform& outTransform)
{

}

*/

UMyMJGameDeskVisualDataObjCpp::UMyMJGameDeskVisualDataObjCpp() : Super()
{
    m_bInFullDataSyncState = false;

    m_eVisualState = EMyMJGameRoomVisualStateCpp::Invalid;
    m_uiVisualStateStartClientTime_ms = 0;
};

UMyMJGameDeskVisualDataObjCpp::~UMyMJGameDeskVisualDataObjCpp()
{
    //deconstructor can't access uobject*, but sharedPtr is OK
    stop();
};

void UMyMJGameDeskVisualDataObjCpp::clearInGame()
{
    stop();
};

void UMyMJGameDeskVisualDataObjCpp::start()
{
    //FMyMJGameDeskVisualCoreDataProcessorCpp *pProcessor = new FMyMJGameDeskVisualCoreDataProcessorCpp();
    //m_pDataProcessor = MakeShareable<FMyMJGameDeskVisualCoreDataProcessorCpp>(pProcessor);
    //m_pDataProcessor->Run();

    if (m_pProcessor.IsValid()) {
        return;
    }

    m_pProcessor = MakeShareable<FMyThreadControlCpp<FMyMJGameDeskProcessorRunnableCpp>>(new FMyThreadControlCpp<FMyMJGameDeskProcessorRunnableCpp>());

    MY_VERIFY(m_pProcessor->create(EThreadPriority::TPri_Normal));
};

void UMyMJGameDeskVisualDataObjCpp::stop()
{
    if (!m_pProcessor.IsValid()) {
        return;
    }

    m_pProcessor.Reset();
};

void UMyMJGameDeskVisualDataObjCpp::loop(uint32 uiClientWorldTimeNow_ms)
{
    if (!m_pProcessor.IsValid()) {
        return;
    }

    cmdLoop();
    dataLoop(uiClientWorldTimeNow_ms);
};


uint32 UMyMJGameDeskVisualDataObjCpp::updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache)
{
    if (!m_pProcessor.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataProcessorWithThread invalid."));
        return -10;
    }

    uint32 ret = m_pProcessor->getRunnableRef().mainThreadCmdUpdateCfgCache(cCfgCache);

    cmdLoop();
    m_pProcessor->kick();

    return ret;


};

bool UMyMJGameDeskVisualDataObjCpp::tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater)
{
    if (pOutRetryLater) {
        *pOutRetryLater = false;
    }

    if (!m_pProcessor.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataProcessor invalid."));
        return false;
    }

    bool bRetryLater = false;
    bool bHaveFeedData = false;

    m_bInFullDataSyncState = m_pProcessor->getRunnableRef().mainThreadTryFeedData(pSeq, &bRetryLater, &bHaveFeedData);
    
    if (bRetryLater) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("visual cfg update not completed, retrying."));
        m_pProcessor->kick();
        cmdLoop();
        m_bInFullDataSyncState = m_pProcessor->getRunnableRef().mainThreadTryFeedData(pSeq, &bRetryLater, &bHaveFeedData);
        if (bRetryLater) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("visual cfg update not completed, need wait."));

            if (pOutRetryLater) {
                *pOutRetryLater = bRetryLater;
            }
            return false;
        }
    }

    if (bHaveFeedData) {
        m_pProcessor->kick();
    }

    if (pOutRetryLater) {
        *pOutRetryLater = bRetryLater;
    }
    return m_bInFullDataSyncState;
};

void UMyMJGameDeskVisualDataObjCpp::getDataTimeRange(uint32 &uiFirstDataGotServerTime_ms, uint32 &uiLastDataGotServerTime_ms) const
{
    MY_VERIFY(m_pProcessor.IsValid());

    uiFirstDataGotServerTime_ms = 0;
    FMyMJGameDeskProcessorDataOutputCpp *pOut = m_pProcessor->getRunnableRef().mainThreadPeekDataOutputForConsume();
    if (pOut) {
        uiFirstDataGotServerTime_ms = pOut->m_uiNewServerWorldTime_ms;
    }

    uiLastDataGotServerTime_ms = m_pProcessor->getRunnableRef().getMainThreadReceivedLabel().m_uiServerWorldTime_ms;
    if (!pOut) {
        uiLastDataGotServerTime_ms = 0;
    }

    MY_VERIFY(uiFirstDataGotServerTime_ms <= uiLastDataGotServerTime_ms);
}


void UMyMJGameDeskVisualDataObjCpp::cmdLoop()
{
    m_pProcessor->getRunnableRef().mainThreadCmdLoop();

    while (1)
    {
        FMyMJGameDeskProcessorCmdOutputCpp *pOut = m_pProcessor->getRunnableRef().mainThreadGetCmdOutputForConsume();
        if (pOut == NULL) {
            break;
        }

        if (pOut->m_apNewCfgCache.Num() > 0) {
            m_cDeskVisualDataNow.setCfg(pOut->m_apNewCfgCache[0]);
        }

        m_pProcessor->getRunnableRef().mainThreadPutCmdOutputAfterConsume(pOut);
    }

};

void UMyMJGameDeskVisualDataObjCpp::dataLoop(uint32 uiClientWorldTimeNow_ms)
{
    //1st, do thread work to prepare the data
    m_pProcessor->getRunnableRef().mainThreadDataLoop();

    //2nd, consume the data if condition met

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("AMyMJGameRoomCpp::loop"));
    uint32 uiClientTimeNow_ms = uiClientWorldTimeNow_ms;
    uint32 uiClientTimeLast_ms = m_cGameProgressData.m_cLastBond.getClientTime_ms_RefConst();
    uint32 uiServerTimeLast_ms = m_cGameProgressData.m_cLastBond.getServerTime_ms_RefConst();

    if (uiClientTimeNow_ms < uiClientTimeLast_ms) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw: uiClientTimeNow_ms %u, uiClientTimeLast_ms %u  .."), uiClientTimeNow_ms, uiClientTimeLast_ms);
        m_cGameProgressData.m_cLastBond.rebondTime(uiServerTimeLast_ms, uiClientTimeNow_ms);
        return;
    }

    uint32 uiFirstDataGotServerTime_ms = 0, uiLastDataGotServerTime_ms = 0;
    getDataTimeRange(uiFirstDataGotServerTime_ms, uiLastDataGotServerTime_ms);
    bool bHaveData = !(uiFirstDataGotServerTime_ms == 0 && uiLastDataGotServerTime_ms == 0);

    //by default, we keep serverTime as old
    uint32 uiServerTimeToBond_ms = 0;

    while (1) {
        if (m_eVisualState == EMyMJGameRoomVisualStateCpp::NormalPlay) {

            //try step forward
            uint32 uiServerTimeTryToGo_ms = m_cGameProgressData.m_cLastBond.getCalculatedServerTime_ms(uiClientTimeNow_ms);

            if (bHaveData) {

                //we are going to play it or wait for time allowed
                bool bTimeChanged = false;
                if (uiServerTimeTryToGo_ms > uiFirstDataGotServerTime_ms && (uiServerTimeTryToGo_ms - uiFirstDataGotServerTime_ms) >= MyMJGameProgressJumpAheadDataMaxTimeMs) {
                    //too ahead, revert back
                    uiServerTimeTryToGo_ms = uiFirstDataGotServerTime_ms;
                    bTimeChanged = true;
                }

                if (uiServerTimeTryToGo_ms <= uiLastDataGotServerTime_ms && (uiLastDataGotServerTime_ms - uiServerTimeTryToGo_ms) >= MyMJGameProgressStayBehindMaxTimeMs) {
                    //falling behind too match, catch up
                    changeVisualState(EMyMJGameRoomVisualStateCpp::CatchUp, uiClientTimeNow_ms, 2);
                    break;
                }

                playGameProgressTo(uiServerTimeTryToGo_ms, false);
                if (bTimeChanged) {
                    uiServerTimeToBond_ms = uiServerTimeTryToGo_ms;
                }
            }
            else {

                //we have no data, prediction
                if (uiServerTimeTryToGo_ms > m_cGameProgressData.m_uiServerTimeConfirmed_ms && (uiServerTimeTryToGo_ms - m_cGameProgressData.m_uiServerTimeConfirmed_ms) > MyMJGameProgressPredictMaxTimeMs) {
                    changeVisualState(EMyMJGameRoomVisualStateCpp::WaitingForDataInGame, uiClientTimeNow_ms, 2);
                }

                playGameProgressTo(uiServerTimeTryToGo_ms, false);
            }
        }
        else if (m_eVisualState == EMyMJGameRoomVisualStateCpp::CatchUp) {
            if (uiLastDataGotServerTime_ms > uiServerTimeLast_ms) {
                playGameProgressTo(uiLastDataGotServerTime_ms, true);
                uiServerTimeToBond_ms = uiLastDataGotServerTime_ms;
            }

            if ((uiClientTimeNow_ms - m_uiVisualStateStartClientTime_ms) >= MyMJGameVisualStateCatchUpMinTimeToStayMs) {
                changeVisualState(EMyMJGameRoomVisualStateCpp::NormalPlay, uiClientTimeNow_ms, 0);
            }
        }
        else if (m_eVisualState == EMyMJGameRoomVisualStateCpp::WaitingForDataInGame) {

            if (bHaveData) {
                //change mode and play imediently
                playGameProgressTo(uiFirstDataGotServerTime_ms, false);
                uiServerTimeToBond_ms = uiFirstDataGotServerTime_ms;

                changeVisualState(EMyMJGameRoomVisualStateCpp::NormalPlay, uiClientTimeNow_ms, 0);
                break;;
            }
        }
        else if (m_eVisualState == EMyMJGameRoomVisualStateCpp::WaitingForDataInitSync) {
            //find first avaiable data

            if (bHaveData) {
                //we have data, play to it
                uiServerTimeToBond_ms = uiFirstDataGotServerTime_ms + AMyMJGameRoomVisualLoopTimeMs * 1; //try forward a bit
                if (uiServerTimeToBond_ms > uiLastDataGotServerTime_ms) {
                    uiServerTimeToBond_ms = uiLastDataGotServerTime_ms;
                }
                playGameProgressTo(uiServerTimeToBond_ms, false);
                changeVisualState(EMyMJGameRoomVisualStateCpp::NormalPlay, uiClientTimeNow_ms, 0);
            }
        }
        else if (m_eVisualState == EMyMJGameRoomVisualStateCpp::Invalid) {
            changeVisualState(EMyMJGameRoomVisualStateCpp::WaitingForDataInitSync, uiClientTimeNow_ms, 0);
        }

        break;
    }

    //rebond the game time
    if (uiServerTimeToBond_ms) {
        m_cGameProgressData.m_cLastBond.rebondTime(uiServerTimeToBond_ms, uiClientTimeNow_ms);
    }
};

//play all events <= uiServerTime_ms
void UMyMJGameDeskVisualDataObjCpp::playGameProgressTo(uint32 uiServerTime_ms, bool bCatchUp)
{
    MY_VERIFY(m_pProcessor.IsValid());

    uint32 uiDataConsumedTime_ms = 0;

    //receive
    while (1)
    {
        FMyMJGameDeskProcessorDataOutputCpp *pOut = m_pProcessor->getRunnableRef().mainThreadPeekDataOutputForConsume();
        if (pOut == NULL) {
            break;
        }

        if (uiServerTime_ms < pOut->m_uiNewServerWorldTime_ms) {
            //time not reached
            break;
        }

        MY_VERIFY(pOut == m_pProcessor->getRunnableRef().mainThreadGetDataOutputForConsume());

        FMyMJGameDeskProcessorDataOutputCpp::helperApplyToDeskVisualData(m_cDeskVisualDataNow, *pOut);

        if (pOut->m_apNewVisualDataDelta.Num() > 0) {
            FMyMJGameDeskVisualDataDeltaCpp &cDelta = pOut->m_apNewVisualDataDelta[0];
            
            MY_VERIFY(cDelta.m_apNewCoreData.Num() > 0);
            MY_VERIFY(cDelta.m_apNewCoreDataDirtyRecord.Num() > 0);

            if (cDelta.m_apEventJustApplied.Num() > 0) {
                getRoomVerified()->onDeskEventApplied(cDelta.m_apNewCoreData[0], cDelta.m_apNewCoreDataDirtyRecord[0], cDelta.m_mNewActorDataIdCards, cDelta.m_mNewActorDataIdDices, cDelta.m_apEventJustApplied[0]);
            }
            else {
                //updating base
                if (cDelta.m_apNewCoreDataDirtyRecord[0].isEmpty() || cDelta.m_mNewActorDataIdCards.Num() > 0 || cDelta.m_mNewActorDataIdDices.Num() > 0) {
                    getRoomVerified()->onDeskUpdatedWithImportantChange(cDelta.m_apNewCoreData[0], cDelta.m_apNewCoreDataDirtyRecord[0], cDelta.m_mNewActorDataIdCards, cDelta.m_mNewActorDataIdDices);
                }
            }
        }

        uiDataConsumedTime_ms = pOut->m_uiNewServerWorldTime_ms;
        m_pProcessor->getRunnableRef().mainThreadPutDataOutputAfterConsume(pOut);
    }

    if (uiDataConsumedTime_ms > 0) {
        MY_VERIFY(uiDataConsumedTime_ms >= m_cGameProgressData.m_uiServerTimeConfirmed_ms);

        if (uiDataConsumedTime_ms > m_cGameProgressData.m_uiServerTimeConfirmed_ms) {
            //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("confirmed progress: %s -> %s."), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(m_cGameProgressData.m_uiServerTimeConfirmed_ms), *UMyCommonUtilsLibrary::genTimeStrFromTimeMs(uiDataConsumedTime_ms));
            m_cGameProgressData.m_uiServerTimeConfirmed_ms = uiDataConsumedTime_ms;
        }
    }
}

AMyMJGameRoomCpp* UMyMJGameDeskVisualDataObjCpp::getRoomVerified() const
{
    //UObject* pOuter = GetOuter()->GetOuter();
    AMyMJGameRoomCpp* pRet = GetTypedOuter<AMyMJGameRoomCpp>();
    //AMyMJGameRoomCpp* pRet = Cast<AMyMJGameRoomCpp>(pOuter);

    /*
    if (!IsValid(pRet)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ret not valid, pOuter %p, class name %s."), pOuter, pOuter ? *pOuter->GetClass()->GetName() : TEXT("NULL"));
        MY_VERIFY(false);
    }
    */

    MY_VERIFY(IsValid(pRet));
    return pRet;
}