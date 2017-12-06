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


//#include "Runtime/CoreUObject/Public/Templates/SubclassOf.h"

#define MY_VISUAL_PROCESS_SUB_THREAD_LOOP_TIME_MS (16)
#define MY_VISUAL_PROCESS_IN_BUFFER_SIZE (256)
#define MY_VISUAL_PROCESS_OUT_BUFFER_SIZE (256)

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


FMyMJGameDeskVisualCoreDataRunnableCpp::FMyMJGameDeskVisualCoreDataRunnableCpp() : FMyRunnableBaseCpp(MY_VISUAL_PROCESS_SUB_THREAD_LOOP_TIME_MS), m_cInRawCoreData(MY_VISUAL_PROCESS_IN_BUFFER_SIZE), m_cOutCalcuatedVisualData(MY_VISUAL_PROCESS_OUT_BUFFER_SIZE)
{
};

FMyMJGameDeskVisualCoreDataRunnableCpp::~FMyMJGameDeskVisualCoreDataRunnableCpp()
{
};

bool FMyMJGameDeskVisualCoreDataRunnableCpp::initBeforRun()
{
    if (!FMyRunnableBaseCpp::initBeforRun()) {
        return false;
    }

    return true;
};

void FMyMJGameDeskVisualCoreDataRunnableCpp::loopInRun()
{
    while (1)
    {
        FMyMJGameDeskVisualCoreDataProcessorInputCpp *pInItem = m_cInRawCoreData.getItemForConsume();
        if (pInItem == NULL) {
            break;
        }

        if (pInItem->m_apNewCfgCache.Num() > 0) {
            FMyMJGameDeskVisualCfgCacheCpp& cCfgCache = pInItem->m_apNewCfgCache[0];
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("processor: got new cfgCache %d."), cCfgCache.m_uiStateKey);
        }
        else if (pInItem->m_apFull.Num() > 0) {
            FMyMJDataStructWithTimeStampBaseCpp& cFull = pInItem->m_apFull[0];

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("processor: %s"),
                *cFull.genDebugMsg());
        }
        else if (pInItem->m_apDelta.Num() > 0) {
            FMyMJEventWithTimeStampBaseCpp& cDelta = pInItem->m_apDelta[0];

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("processor: %s"),
                *cDelta.genDebugMsg());
        }
        else {

        }

        m_cInRawCoreData.putInConsumedItem(pInItem);
    }
}

void FMyMJGameDeskVisualCoreDataRunnableCpp::exitAfterRun()
{
    FMyRunnableBaseCpp::exitAfterRun();
};

int32 FMyMJGameDeskVisualCoreDataRunnableCpp::updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& newCfg)
{
    m_cInRawCoreData.getItemForProduce();

    FMyMJGameDeskVisualCoreDataProcessorInputCpp* pIn = m_cInRawCoreData.getItemForProduce();
    if (pIn == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("not enough of buffer for input of core processor!"));
        return  -1;
    }

    pIn->reset();
    MY_VERIFY(pIn->m_apNewCfgCache.Emplace() == 0);
    pIn->m_apNewCfgCache[0] = newCfg;

    pIn->verifyValid();
    m_cInRawCoreData.putInProducedItem(pIn);

    m_cLabelLastIn.m_uiStateKey = newCfg.m_uiStateKey;
    MY_VERIFY(m_cLabelLastIn.m_uiStateKey != MyUIntIdDefaultInvalidValue);

    return 0;
};

bool FMyMJGameDeskVisualCoreDataRunnableCpp::tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq)
{
    bool bHaveNewEvent = false;
    int32 iRet = tryFeedEvents(pSeq, &bHaveNewEvent);
    if (bHaveNewEvent) {
        kick();
    }


    uint32 uiServerWorldTime_ms = pSeq->getServerWorldTime_ms();
    //see if we need to update timestamp
    while (iRet == MyTryFeedEventRetAllProcessed && uiServerWorldTime_ms > m_cLabelLastIn.m_uiServerWorldTime_ms) {


        FMyMJGameDeskVisualCoreDataProcessorInputCpp* pIn = m_cInRawCoreData.getItemForProduce();
        if (pIn == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("not enough of buffer for input of core processor!"));
            break;
        }

        pIn->reset();
        pIn->m_uiUpdateServerWorldTime_ms = uiServerWorldTime_ms;

        pIn->verifyValid();
        m_cInRawCoreData.putInProducedItem(pIn);

        m_cLabelLastIn.m_uiServerWorldTime_ms = uiServerWorldTime_ms;

        break;
    }


    return iRet == MyTryFeedEventRetNeedSyncBase;
};

int32 FMyMJGameDeskVisualCoreDataRunnableCpp::tryFeedEvents(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutHaveFeedEvent)
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
    MyMJGameRoleTypeCpp eRoleTypeSelf = m_cLabelLastIn.m_eRoleType;
    MY_VERIFY(eRoleTypeData != MyMJGameRoleTypeCpp::Max);

    //early out
    //Todo: check this code carefully since it need server's replication code do the right thing
    if (eRoleTypeData == eRoleTypeSelf && uiServerWorldTime_ms <= m_cLabelLastIn.m_uiServerWorldTime_ms) {
        return MyTryFeedEventRetAllProcessed;
    }

    //For simple, we only target at the event queue, ignore whether the base has newer data
    const UMyMJGameEventCycleBuffer* pEvents = pSeq->getDeltaDataEvents(false);
    if (!IsValid(pEvents)) {
        return MyTryFeedEventRetDataReplicationIncomplete;
    }

    if (pEvents->getCount(NULL) <= 0) {

        return MyTryFeedEventRetAllProcessed;
    }
    
    int32 deltaCount = pEvents->getCount(NULL);

    uint32 idEventDeltaFirst = pEvents->peekRefAt(0).getIdEvent();
    uint32 idEventDeltaLast = pEvents->peekRefAt(deltaCount - 1).getIdEvent();
    uint32 idEventSelf = m_cLabelLastIn.m_uiIdEvent;


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
         
         if ((cFullData.getIdEventApplied() + 1) < idEventFirstToPick) {
             //we don't have a valid base
             return MyTryFeedEventRetNeedSyncBase;
         }

        //OK we have a valid base, calculate how many we can deliver
        idEventFirstToPick = cFullData.getIdEventApplied() + 1;
        eventCountToPick = idEventDeltaLast >= idEventFirstToPick ? (idEventDeltaLast - idEventFirstToPick) + 1 : 0;

        FMyMJGameDeskVisualCoreDataProcessorInputCpp* pIn = m_cInRawCoreData.getItemForProduce();
        if (pIn == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("not enough of buffer for input of core processor, when update full."));
            return MyTryFeedEventRetLackBuffer;
        }

        pIn->reset();
        MY_VERIFY(pIn->m_apFull.Emplace() == 0);
        pIn->m_apFull[0] = cFullData;
        pIn->m_uiUpdateServerWorldTime_ms = uiServerWorldTime_ms;

        m_cInRawCoreData.putInProducedItem(pIn);

        m_cLabelLastIn.updateAfterEventAdded(eRoleTypeData, cFullData.getIdEventApplied(), uiServerWorldTime_ms);

        if (pOutHaveFeedEvent) {
            *pOutHaveFeedEvent = true;
        }
    }


    MY_VERIFY(idEventFirstToPick >= idEventDeltaFirst);

    uint32 idxStart = idEventFirstToPick - idEventDeltaFirst;
    for (uint32 i = 0; i < eventCountToPick; i++) {
        uint32 idx = idxStart + i;
        MY_VERIFY(idx < (uint32)(1 << 31));

        FMyMJGameDeskVisualCoreDataProcessorInputCpp* pIn = m_cInRawCoreData.getItemForProduce();
        if (pIn == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("not enough of buffer for input of core processor, when update delta."));
            return MyTryFeedEventRetLackBuffer;
        }

        const FMyMJEventWithTimeStampBaseCpp& event = pEvents->peekRefAt(idx);

        pIn->reset();
        MY_VERIFY(pIn->m_apDelta.Emplace() == 0);
        pIn->m_apDelta[0] = event;
        pIn->m_uiUpdateServerWorldTime_ms = uiServerWorldTime_ms;

        pIn->verifyValid();
        m_cInRawCoreData.putInProducedItem(pIn);

        m_cLabelLastIn.updateAfterEventAdded(eRoleTypeData, event.getIdEvent(), uiServerWorldTime_ms);

        if (pOutHaveFeedEvent) {
            *pOutHaveFeedEvent = true;
        }
    }

    return MyTryFeedEventRetAllProcessed;
};

/*
UMyMJGameDeskSuiteCpp::UMyMJGameDeskSuiteCpp() : Super()
{
    m_pResManager = CreateDefaultSubobject<UMyMJGameDeskResManagerCpp>(TEXT("desk res manager"));
    m_pCoreWithVisual = CreateDefaultSubobject<UMyMJGameCoreWithVisualCpp>(TEXT("core with visual"));

    //m_pDataHistoryBuffer = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("buffer"));
    //m_pTestObj = CreateDefaultSubobject<UMyTestObject>(TEXT("test obj"));
    //if (!IsValid(m_pRootScene)) {
        //USceneComponent* pRootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
        //MY_VERIFY(IsValid(pRootSceneComponent));

        //RootComponent = pRootSceneComponent;
        //m_pRootScene = pRootSceneComponent;
    //}

    m_pDeskAreaActor = NULL;
    //m_pMJCore = NULL;

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("0 this %p, m_pCoreWithVisual %p, m_pDataHistoryBuffer %p, m_pTestObj %p."), this, m_pCoreWithVisual, m_pDataHistoryBuffer, m_pTestObj);
};



UMyMJGameDeskSuiteCpp::~UMyMJGameDeskSuiteCpp()
{
}

void UMyMJGameDeskSuiteCpp::BeginPlay()
{
    Super::BeginPlay();
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("1 this %p, m_pCoreWithVisual %p, m_pDataHistoryBuffer %p, m_pTestObj %p."), this, m_pCoreWithVisual, m_pDataHistoryBuffer, m_pTestObj);
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("0 m_pResManager %p, m_pCoreWithVisual %p, m_pTestObj %p."), m_pResManager, m_pCoreWithVisual, m_pTestObj);

    if (!checkSettings()) {
        return;
    }

};
*/

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
bool UMyMJGameDeskSuiteCpp::checkSettings() const
{
    MY_VERIFY(IsValid(m_pResManager));
    if (false == m_pResManager->checkSettings()) {
        return false;
    }

    MY_VERIFY(IsValid(m_pCoreWithVisual));
    if (false == m_pCoreWithVisual->checkSettings()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreWithVisual have incorrect settings."));
        return false;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDeskAreaActor %p is not valid."), m_pDeskAreaActor);
        return false;
    }

    return true;
}

int32 UMyMJGameDeskSuiteCpp::helperGetColCountPerRowForDefaultAligment(int32 idxAtttender, MyMJCardSlotTypeCpp eSlot, int32& outCount) const
{
    outCount = 1;
    FMyMJGameDeskVisualPointCfgCpp cVisualPointCfg;
    if (0 != m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlot(idxAtttender, eSlot, cVisualPointCfg)) {
        return -1;
    };
    FMyMJGameActorModelInfoBoxCpp cCardModelInfo;
    if (0 != m_pResManager->getCardModelInfoUnscaled(cCardModelInfo)) {
        return -2;
    }

    if (cCardModelInfo.m_cBoxExtend.Y < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fixing cCardModelInfo.m_cBoxExtend.Y since it is %f before."), cCardModelInfo.m_cBoxExtend.Y);
        cCardModelInfo.m_cBoxExtend.Y = 1;
    }

    int32 retCount = FMath::CeilToInt(cVisualPointCfg.m_cAreaBoxExtendFinal.Y / cCardModelInfo.m_cBoxExtend.Y);
    if (retCount <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fixing retCount it is %d before."), retCount);
        retCount = 1;
    }

    outCount = retCount;

    return 0;
}
*/

/*
void UMyMJGameDeskSuiteCpp::helperTryUpdateCardVisualInfoPack()
{
    if (!checkSettings()) {
        return;
    }
    */
    /*
    FMyMJDataAtOneMomentCpp& cDataNow = m_pCoreWithVisual->getDataNowRef();
    FMyMJDataAccessorCpp& cAccessor = cDataNow.getAccessorRef();
    const TArray<int32>& aHelperAttenderSlotDirtyMasks = cAccessor.getHelperAttenderSlotDirtyMasksConst(true);
    if (aHelperAttenderSlotDirtyMasks[0] == 0 && aHelperAttenderSlotDirtyMasks[1] == 0 && aHelperAttenderSlotDirtyMasks[2] == 0 && aHelperAttenderSlotDirtyMasks[3] == 0) {
        return;
    }

    FMyMJDataStructWithTimeStampBaseCpp& cBase = cDataNow.getBaseRef();

    const FMyMJCardInfoPackCpp& cCardInfoPack = cBase.getCoreDataRefConst().m_cCardInfoPack;
    const FMyMJCardValuePackCpp& ccardValuePack = cBase.getRoleDataPrivateRefConst((uint8)cDataNow.getRole()).m_cCardValuePack;

    FMyMJCardVisualInfoPackCpp& cCardVisualInfoPack = m_cCardVisualInfoPack;

    if (cCardInfoPack.getLength() != ccardValuePack.getLength()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pack length not equal: info pack %d, value pack %d, accces role type %d."), cCardInfoPack.getLength(), ccardValuePack.getLength(), (uint8)cDataNow.getRole());
        MY_VERIFY(false);
    }

    cCardVisualInfoPack.resize(cCardInfoPack.getLength());

    for (int32 idxAtttender = 0; idxAtttender < 4; idxAtttender++) {
        if (aHelperAttenderSlotDirtyMasks[idxAtttender] == 0) {
            continue;
        }

        const FMyMJRoleDataAttenderPublicCpp& attenderPublic = cBase.getRoleDataAttenderPublicRefConst(idxAtttender);

        //todo: there can be short cut that some case we don't need update
        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::Untaken) == true) {

            const TArray<FMyIdCollectionCpp>& aUntakenCardStacks = cBase.getCoreDataRefConst().m_aUntakenCardStacks;
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

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_bVisible = true;
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::Untaken);

                    pCardVisualInfo->m_iIdxRow = 0;
                    pCardVisualInfo->m_iIdxColInRow = cCardInfo.m_cPosi.m_iIdxInSlot0;
                    pCardVisualInfo->m_iIdxStackInCol = cCardInfo.m_cPosi.m_iIdxInSlot1;

                    pCardVisualInfo->m_iCardValue = cardValue;
                }
            }

        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::JustTaken) == true) {

            const TArray<int32>& aIdHandCards = attenderPublic.m_aIdHandCards;
            const TArray<int32>& aIdJustTakenCards = attenderPublic.m_aIdJustTakenCards;
            int32 l0 = aIdHandCards.Num();
            int32 l1 = aIdJustTakenCards.Num();

            for (int32 i = 0; i < l1; i++) {
                int32 cardId = aIdJustTakenCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::JustTaken);

                pCardVisualInfo->m_iIdxRow = 0;
                pCardVisualInfo->m_iIdxColInRow = l0 + i;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iColInRowExtraMarginCount = 1;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::InHand) == true) {

            const TArray<int32>& aIdHandCards = attenderPublic.m_aIdHandCards;
            int32 l0 = aIdHandCards.Num();

            FMyMJValueIdMapCpp m_cSortCards;
            m_cSortCards.changeKeepOrder(true, false);
            for (int32 i = 0; i < l0; i++) {
                int32 cardId = aIdHandCards[i];
                int32 cardValue = ccardValuePack.getByIdx(cardId);
                m_cSortCards.insert(cardId, cardValue);
            }
            TArray<FMyIdValuePair> aPairs;
            m_cSortCards.collectAllWithValue(aPairs);
            MY_VERIFY(aPairs.Num() == l0);

            for (int32 i = 0; i < l0; i++) {
                int32 cardId = aPairs[i].m_iId;
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = aPairs[i].m_iValue;

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::InHand);

                pCardVisualInfo->m_iIdxRow = 0;
                pCardVisualInfo->m_iIdxColInRow = i;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::GivenOut) == true) {

            int32 iColPerRow;
            if (0 != helperGetColCountPerRowForDefaultAligment(idxAtttender, MyMJCardSlotTypeCpp::GivenOut, iColPerRow)) {
                continue;
            }

            const TArray<int32>& aIdGivenOutCards = attenderPublic.m_aIdGivenOutCards;
            int32 l = aIdGivenOutCards.Num();

            for (int32 i = 0; i < l; i++) {
                int32 cardId = aIdGivenOutCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::GivenOut);

                pCardVisualInfo->m_iIdxRow = i / iColPerRow;
                pCardVisualInfo->m_iIdxColInRow = i % iColPerRow;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::Weaved) == true) {

            FMyMJGameDeskVisualPointCfgCpp cVisualPointCfg;
            if (0 != m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlot(idxAtttender, MyMJCardSlotTypeCpp::Weaved, cVisualPointCfg)) {
                continue;
            };

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
                    if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAtttender + 1) % 4)) {
                        //aIds.
                        triggerCardX90D = 1;
                    }
                    else if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAtttender + 2) % 4)) {
                        triggerCardX90D = 1;
                    }
                    else if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAtttender + 3) % 4)) {
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
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected visualPointCfg: idxAtttender %d, slot weave, m_eColInRowAlignment %d."), idxAtttender, (uint8)cVisualPointCfg.m_eColInRowAlignment);
                    continue;
                }

                while (1)
                {
                    int32 cardId = aIds[workingIdx];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_bVisible = true;
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
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

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::WinSymbol) == true) {

            int32 iColPerRow;
            if (0 != helperGetColCountPerRowForDefaultAligment(idxAtttender, MyMJCardSlotTypeCpp::WinSymbol, iColPerRow)) {
                continue;
            }

            const TArray<int32>& aIdWinSymbolCards = attenderPublic.m_aIdWinSymbolCards;
            int32 l = aIdWinSymbolCards.Num();

            for (int32 i = 0; i < l; i++) {
                int32 cardId = aIdWinSymbolCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::WinSymbol);

                pCardVisualInfo->m_iIdxRow = i / iColPerRow;
                pCardVisualInfo->m_iIdxColInRow = i % iColPerRow;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::ShownOnDesktop) == true) {

            const TArray<int32>& aIdShownOnDesktopCards = attenderPublic.m_aIdShownOnDesktopCards;
            int32 l = aIdShownOnDesktopCards.Num();

            for (int32 i = 0; i < l; i++) {
                int32 cardId = aIdShownOnDesktopCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::ShownOnDesktop);

                pCardVisualInfo->m_iIdxRow = 0;
                pCardVisualInfo->m_iIdxColInRow = i;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }
    }

    */

/*
}

int32 UMyMJGameDeskSuiteCpp::helperCalcCardTransformFromvisualPointCfg(const FMyMJGameActorModelInfoBoxCpp& cardModelInfoFinal, const FMyMJGameCardVisualInfoCpp& cardVisualInfoFinal, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FTransform& outTransform)
{

    const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCenter = visualPointCfg;
    const FMyMJGameCardVisualInfoCpp& cCardVisualInfo = cardVisualInfoFinal;

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
    if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Invalid || eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid) { //we don't support mid allignment now
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
    FVector colOffsetL2R = perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + fColInRowExtraMarginAbs * iColInRowExtraMarginCount;
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

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("relative2VisualPointTransform %s, cTransFormCenter %s, outTransform %s."),
        *UKismetStringLibrary::Conv_TransformToString(relative2VisualPointTransform),
        *UKismetStringLibrary::Conv_TransformToString(cTransFormCenter),
        *UKismetStringLibrary::Conv_TransformToString(outTransform));

    //outLocationWorld = outTransform.GetLocation();
    //outRotatorWorld = outTransform.GetRotation().Rotator();

    return 0;

}

void UMyMJGameDeskSuiteCpp::helperResolveTargetCardVisualState(int32 idxCard, FMyMJGameActorVisualStateBaseCpp& outTargetCardVisualState)
{
*/
    /*
    outTargetCardVisualState.reset();

    //if (!checkSettings()) {
        //return;
    //}

    MY_VERIFY(idxCard >= 0);

    const FMyMJDataAtOneMomentCpp& cDataNow = m_pCoreWithVisual->getDataNowRefConst();
    const FMyMJDataStructWithTimeStampBaseCpp& cBase = cDataNow.getBaseRefConst();
    const FMyMJCardInfoPackCpp& cCardInfoPack = cBase.getCoreDataRefConst().m_cCardInfoPack;

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
    */
/*
}
*/

UMyMJGameDeskVisualDataObjCpp::UMyMJGameDeskVisualDataObjCpp()
{
    m_bInFullDataSyncState = false;
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

    if (m_pDataProcessorWithThread.IsValid()) {
        return;
    }

    m_pDataProcessorWithThread = MakeShareable<FMyThreadControlCpp<FMyMJGameDeskVisualCoreDataRunnableCpp>>(new FMyThreadControlCpp<FMyMJGameDeskVisualCoreDataRunnableCpp>());

    MY_VERIFY(m_pDataProcessorWithThread->create(EThreadPriority::TPri_Normal));
};

void UMyMJGameDeskVisualDataObjCpp::stop()
{
    if (!m_pDataProcessorWithThread.IsValid()) {
        return;
    }

    m_pDataProcessorWithThread.Reset();
};

int32 UMyMJGameDeskVisualDataObjCpp::updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache)
{
    if (!m_pDataProcessorWithThread.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataProcessorWithThread invalid."));
        return -10;
    }

    return m_pDataProcessorWithThread->getRunnableRef().updateCfgCache(cCfgCache);

};

uint32 UMyMJGameDeskVisualDataObjCpp::getLastInCfgCacheStateKey() const
{
    if (!m_pDataProcessorWithThread.IsValid()) {
        return MyUIntIdDefaultInvalidValue;
    }

    return m_pDataProcessorWithThread->getRunnableRef().getLabelLastInRefConst().m_uiStateKey;
}

bool UMyMJGameDeskVisualDataObjCpp::tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq)
{
    if (!m_pDataProcessorWithThread.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataProcessor invalid."));
        return false;
    }

    m_bInFullDataSyncState = m_pDataProcessorWithThread->getRunnableRef().tryFeedData(pSeq);

    return m_bInFullDataSyncState;
};