// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameVisualCard.h"
#include "Utils/CommonUtils/MyCardUtils.h"


#include "MyMJGameDeskVisualData.generated.h"


USTRUCT(BlueprintType)
struct FMyMJGameDeskVisualPointCfgCacheCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualPointCfgCacheCpp()
    {

    };

    virtual ~FMyMJGameDeskVisualPointCfgCacheCpp()
    {

    };

    void clear()
    {
        m_mCardVisualPointCache.Reset();
        m_mTrivalVisualPointCache.Reset();
    };


    int32 getCardVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const;
    void  setCardVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, const FMyMJGameDeskVisualPointCfgCpp &visualPoint);

    int32 getTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const;
    void  setTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, const FMyMJGameDeskVisualPointCfgCpp &visualPoint);

protected:

    static int32 helperGetVisualPointCfgByIdxs(int32 idx0, int32 idx1, int32 idx2, const TMap<int32, FMyMJGameDeskVisualPointCfgCpp>& cTargetMap, FMyMJGameDeskVisualPointCfgCpp &visualPoint);
    static void  helperSetVisualPointCfgByIdxs(int32 idx0, int32 idx1, int32 idx2, TMap<int32, FMyMJGameDeskVisualPointCfgCpp>& cTargetMap, const FMyMJGameDeskVisualPointCfgCpp &visualPoint);

    //by split to two domains, it runs a bit faster at runtime
    //card cfg
    TMap<int32, FMyMJGameDeskVisualPointCfgCpp> m_mCardVisualPointCache;

    //other cfg except card
    TMap<int32, FMyMJGameDeskVisualPointCfgCpp> m_mTrivalVisualPointCache;
};

USTRUCT(BlueprintType)
struct FMyMJGameDeskVisualActorModelInfoCacheCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualActorModelInfoCacheCpp()
    {

    };

    virtual ~FMyMJGameDeskVisualActorModelInfoCacheCpp()
    {

    };

    void clear()
    {
        m_cCardModelInfo.reset();
        m_cDiceModelInfo.reset();
    };

    FMyMJGameActorModelInfoBoxCpp m_cCardModelInfo;
    FMyMJGameActorModelInfoBoxCpp m_cDiceModelInfo;
};

USTRUCT(BlueprintType)
struct FMyMJGameDeskVisualCfgCacheCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualCfgCacheCpp()
    {
        m_uiStateKey = MyUIntIdDefaultInvalidValue;
    };

    virtual ~FMyMJGameDeskVisualCfgCacheCpp()
    {

    };

    void clear()
    {
        m_cPointCfg.clear();
        m_cModelInfo.clear();
    };

    uint32 m_uiStateKey; //Represent the state, used to compare if it is changed.
    FMyMJGameDeskVisualPointCfgCacheCpp m_cPointCfg;
    FMyMJGameDeskVisualActorModelInfoCacheCpp m_cModelInfo;
};

USTRUCT()
struct FMyMJGameDeskVisualActorDatasCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualActorDatasCpp()
    {

    };

    virtual ~FMyMJGameDeskVisualActorDatasCpp()
    {

    };

    TArray<FMyMJGameCardVisualInfoAndStateCpp> m_aCards;
    TArray<FMyMJGameActorVisualStateBaseCpp> m_aDices;
};

USTRUCT()
struct FMyMJGameDeskVisualDataDirtyRecordCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameDeskVisualDataDirtyRecordCpp()
    {
        m_uiPrevServerTime_ms_unresolved = MyUIntIdDefaultInvalidValue;
        m_uiPrevIdEvent = MyUIntIdDefaultInvalidValue;
    };

    virtual ~FMyMJGameDeskVisualDataDirtyRecordCpp()
    {

    };

    void resetExceptPrevInfo()
    {
        m_cCoreDataDirtyRecord.reset();
        m_cVisualActorDataDirtyRecord.reset();
        m_aImportantEventsApplied.Reset();
    };

    //time segment data
    uint32 m_uiPrevServerTime_ms_unresolved; //Todo: make timestamp unique to act as unique state label
    uint32 m_uiPrevIdEvent;

    FMyDirtyRecordWithKeyAnd4IdxsMapCpp m_cCoreDataDirtyRecord;
    FMyDirtyRecordWithKeyAnd4IdxsMapCpp m_cVisualActorDataDirtyRecord;
    TArray<FMyMJEventWithTimeStampBaseCpp> m_aImportantEventsApplied;
};

//Contains all data for core logic visualization, and it can be processed in sub thread
//if in subtread mode, the data may be 1 or 2 frame late, which  means 16-32ms later
USTRUCT()
struct FMyMJGameDeskVisualDataOneMomentCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameDeskVisualDataOneMomentCpp()
    {

    };

    virtual ~FMyMJGameDeskVisualDataOneMomentCpp()
    {

    };

    FMyMJDataStructWithTimeStampBaseCpp m_cCoreData;
    FMyMJGameDeskVisualActorDatasCpp m_cActorData;
};

//this class's data is history critical, not delta should be missed
USTRUCT()
struct FMyMJGameDeskVisualDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualDataCpp()
    {

    };

        virtual ~FMyMJGameDeskVisualDataCpp()
    {

    };

    FMyMJGameDeskVisualCfgCacheCpp m_cCfgCache;
    FMyMJGameDeskVisualDataOneMomentCpp m_cDataOneMoment;
};

USTRUCT()
struct FMyMJGameDeskVisualDataOneMomentDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualDataOneMomentDeltaCpp()
    {

    };

    virtual ~FMyMJGameDeskVisualDataOneMomentDeltaCpp()
    {

    };

    FMyMJGameDeskVisualDataOneMomentCpp m_cNewFull;
    FMyMJGameDeskVisualDataDirtyRecordCpp m_cDirtyRecord;
};

//#define FMyMJGameDeskSuiteCoreDataProcessorSubThreadLoopTimeMs (10)

USTRUCT()
struct FMyMJGameDeskProcessorDataInputCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskProcessorDataInputCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskProcessorDataInputCpp()
    {
        //reset();
    };

    inline
    void reset()
    {
        m_apNewFullData.Reset();
        m_apNewDeltaEvent.Reset();
        m_uiNewServerWorldTime_ms = 0;
    };

    inline
    void verifyValid() const
    {
        if (m_apNewFullData.Num() > 0) {
            MY_VERIFY(m_apNewFullData.Num() == 1);
            MY_VERIFY(m_apNewDeltaEvent.Num() == 0);
        }
        else if (m_apNewDeltaEvent.Num() > 0) {
            MY_VERIFY(m_apNewFullData.Num() == 0);
            MY_VERIFY(m_apNewDeltaEvent.Num() == 1);
        }
        else {
        }

        MY_VERIFY(m_uiNewServerWorldTime_ms > 0);
    };

    //core data, need timestamp
    TArray<FMyMJDataStructWithTimeStampBaseCpp> m_apNewFullData;
    TArray<FMyMJEventWithTimeStampBaseCpp> m_apNewDeltaEvent;
    uint32 m_uiNewServerWorldTime_ms;
};

USTRUCT()
struct FMyMJGameDeskProcessorDataOutputCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskProcessorDataOutputCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskProcessorDataOutputCpp()
    {

    };

    inline
    void reset()
    {
        m_apNewDelta.Reset();
        m_uiNewServerWorldTime_ms = 0;
    };

    inline
    void verifyValid() const
    {
       if (m_apNewDelta.Num() > 0) {
            MY_VERIFY(m_apNewDelta.Num() == 1);
        }
        else {
        }

        MY_VERIFY(m_uiNewServerWorldTime_ms > 0);

    };

    //core and visual data, need timestamp
    TArray<FMyMJGameDeskVisualDataOneMomentDeltaCpp> m_apNewDelta;
    uint32 m_uiNewServerWorldTime_ms;

};

USTRUCT()
struct FMyMJGameDeskProcessorCmdInputCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskProcessorCmdInputCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskProcessorCmdInputCpp()
    {
        //reset();
    };

    void reset()
    {
        m_apNewCfgCache.Reset();
    };

    inline
    void verifyValid() const
    {
        MY_VERIFY(m_apNewCfgCache.Num() == 1);
    };

    TArray<FMyMJGameDeskVisualCfgCacheCpp> m_apNewCfgCache;
};

USTRUCT()
struct FMyMJGameDeskProcessorCmdOutputCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskProcessorCmdOutputCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskProcessorCmdOutputCpp()
    {
        //reset();
    };

    void reset()
    {
        m_apNewCfgCache.Reset();
    };

    inline
        void verifyValid() const
    {
        MY_VERIFY(m_apNewCfgCache.Num() == 1);
    };

    TArray<FMyMJGameDeskVisualCfgCacheCpp> m_apNewCfgCache;
};


struct FMyMJGameDeskProcessorSentLabelCpp
{
public:

    FMyMJGameDeskProcessorSentLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskProcessorSentLabelCpp()
    {

    };

    inline
    void reset()
    {
        m_uiCfgStateKey = MyUIntIdDefaultInvalidValue;
        m_eRoleType = MyMJGameRoleTypeCpp::Max;
        m_uiIdEvent = MyUIntIdDefaultInvalidValue;
        m_uiServerWorldTime_ms = 0;
    };

    inline
    void updateAfterEventAdded(MyMJGameRoleTypeCpp eRole, uint32 idEvent, uint32 uiServerWorldTime_ms)
    {
        MY_VERIFY(eRole != MyMJGameRoleTypeCpp::Max);
        MY_VERIFY(idEvent > m_uiIdEvent);
        MY_VERIFY(uiServerWorldTime_ms >= m_uiServerWorldTime_ms);

        m_eRoleType = eRole;
        m_uiIdEvent = idEvent;
        m_uiServerWorldTime_ms = uiServerWorldTime_ms;
    };

    uint32 m_uiCfgStateKey;
    MyMJGameRoleTypeCpp m_eRoleType;
    uint32 m_uiIdEvent;
    uint32 m_uiServerWorldTime_ms;

};

struct FMyMJGameDeskProcessorReceivedLabelCpp
{
public:

    FMyMJGameDeskProcessorReceivedLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskProcessorReceivedLabelCpp()
    {

    };

    inline
    void reset()
    {
        m_uiCfgStateKey = MyUIntIdDefaultInvalidValue;
    };

    uint32 m_uiCfgStateKey;
};

#define MyTryFeedEventRetAllProcessed (0)
#define MyTryFeedEventRetDataReplicationIncomplete (-1)
#define MyTryFeedEventRetNeedSyncBase (-2)
#define MyTryFeedEventRetLackBuffer (-3)

//can only used inside thread controller
class FMyMJGameDeskProcessorRunnableCpp : public FMyRunnableBaseCpp
{
public:

    FMyMJGameDeskProcessorRunnableCpp();
    virtual ~FMyMJGameDeskProcessorRunnableCpp();

    virtual FString anyThreadGenName() const override
    {
        return TEXT("FMyMJGameDeskVisualCoreDataRunnableCpp");
    };

    virtual bool subThreadIsReady() const override
    {
        return true;
    };

    //return the new key, not take effect now, may need some loop to check subthread report
    uint32 mainThreadCmdUpdateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

    void mainThreadCmdLoop();
    void mainThreadDataLoop();

    //return true if need to sync base
    bool mainThreadDataTryFeed(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater);



protected:

    int32 mainThreadTryFeedEvents(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutHaveFeedEvent);


    virtual bool subThreadInitBeforRun() override;

    virtual void subThreadLoopInRun() override;

    virtual void subThreadExitAfterRun() override;

    void subThreadCmdLoop();
    void subThreadDataLoop();


    void subThreadApplyEvent();

    FMyMJGameDeskVisualCfgCacheCpp m_cMainThreadWaitingToSendCfgCache;

    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdInputCpp>  m_cCmdIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdOutputCpp> m_cCmdOut;

    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataInputCpp>  m_cDataIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataOutputCpp> m_cDataOut;

    //record
    FMyMJGameDeskProcessorSentLabelCpp     m_cMainThreadSentLabel;
    FMyMJGameDeskProcessorReceivedLabelCpp m_cMainThreadReceivedLabel;

    //used only inside processor
    FMyMJGameDeskVisualDataCpp *m_pSubThreadData;
    FMyMJGameDeskVisualDataDirtyRecordCpp *m_pSubThreadDirtyRecord;
    FMyMJDataAccessorCpp* m_pSubThreadAccessor;

    FMyMJGameDeskProcessorSentLabelCpp* m_pSubThreadSentLabel;

};


UCLASS(Blueprintable, NotBlueprintType)
class MYONLINECARDGAME_API UMyMJGameDeskVisualDataObjCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJGameDeskVisualDataObjCpp();
    virtual ~UMyMJGameDeskVisualDataObjCpp();

    void clearInGame();

    inline bool getInFullDataSyncState() const
    {
        return m_bInFullDataSyncState;
    };

    void start();
    void stop();

    uint32 updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

    //return true if need to sync base
    bool tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater);

protected:

    //TSharedPtr<FMyMJGameDeskVisualCoreDataProcessorCpp> m_pDataProcessor;
    TSharedPtr<FMyThreadControlCpp<FMyMJGameDeskProcessorRunnableCpp>> m_pProcessor;


    bool m_bInFullDataSyncState;
};
