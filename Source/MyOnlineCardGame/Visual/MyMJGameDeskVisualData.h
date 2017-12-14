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

    //return errorCode, 0 means OK
    int32 helperGetColCountPerRow(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, int32& outCount) const
    {
        outCount = 1;
        FMyMJGameDeskVisualPointCfgCpp cVisualPointCfg;

        int32 retCode =m_cPointCfg.getCardVisualPointCfgByIdxAttenderAndSlot(idxAttender, eSlot, cVisualPointCfg);
        if (retCode != 0) {
            return retCode;
        }

        const FMyMJGameActorModelInfoBoxCpp &cCardModelInfo = m_cModelInfo.m_cCardModelInfo;

        if (cCardModelInfo.m_cBoxExtend.Y < 1) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cCardModelInfo.m_cBoxExtend.Y too small: %f."), cCardModelInfo.m_cBoxExtend.Y);
            return -2;
        }

        int32 retCount = FMath::CeilToInt(cVisualPointCfg.m_cAreaBoxExtendFinal.Y / cCardModelInfo.m_cBoxExtend.Y);
        if (retCount <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fixing retCount it is %d before."), retCount);
            retCount = 1;
        }

        outCount = retCount;

        return 0;
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

    inline
    void reset()
    {
        m_aCards.Reset();
        m_aDices.Reset();
    };

    TArray<FMyMJGameCardVisualInfoAndResultCpp> m_aCards;
    TArray<FMyMJGameDiceVisualInfoAndResultCpp> m_aDices;
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

        inline
    void reset()
    {
        m_cCfgCache.clear();
        m_cCoreData.reset();
        m_cActorData.reset();

        m_uiServerWorldTime_ms = 0;
    };

    FMyMJGameDeskVisualCfgCacheCpp m_cCfgCache;
    FMyMJDataStructWithTimeStampBaseCpp m_cCoreData;
    FMyMJGameDeskVisualActorDatasCpp m_cActorData;

    uint32 m_uiServerWorldTime_ms; //different with coreData time, it is not the state's supposed time, but the playing time
};

USTRUCT()
struct FMyMJGameDeskVisualDataDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualDataDeltaCpp()
    {
    };

    inline void reset()
    {
        m_apNewCoreData.Reset();
        m_apNewCoreDataDirtyRecord.Reset();
        m_mNewActorDataIdCards.Reset();
        m_mNewActorDataIdDices.Reset();
        m_apEventJustApplied.Reset();

    };

    /*
    inline
        FMyMJDataStructWithTimeStampBaseCpp& getNewCoreDataCreateIfNotExist()
    {
        if (m_apNewCoreData.Num() == 0) {
            m_apNewCoreData.Emplace();
        }
        return m_apNewCoreData[0];
    };

    inline
    FMyDirtyRecordWithKeyAnd4IdxsMapCpp& getNewCoreDataDirtyRecordCreateIfNotExist()
    {
        if (m_apNewCoreDataDirtyRecord.Num() == 0) {
            m_apNewCoreDataDirtyRecord.Emplace();
        }
        return m_apNewCoreDataDirtyRecord[0];
    };
    */

    TArray<FMyMJDataStructWithTimeStampBaseCpp> m_apNewCoreData;
    TArray<FMyDirtyRecordWithKeyAnd4IdxsMapCpp> m_apNewCoreDataDirtyRecord;

    TMap<int32, FMyMJGameCardVisualInfoAndResultCpp> m_mNewActorDataIdCards;
    TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp> m_mNewActorDataIdDices;

    TArray<FMyMJEventWithTimeStampBaseCpp> m_apEventJustApplied;
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
        m_apNewVisualDataDelta.Reset();
        m_uiNewServerWorldTime_ms = 0;
    };

    inline
    void verifyValid() const
    {
       if (m_apNewVisualDataDelta.Num() > 0) {
            MY_VERIFY(m_apNewVisualDataDelta.Num() == 1);
        }
        else {
        }

        MY_VERIFY(m_uiNewServerWorldTime_ms > 0);

    };

    //core and visual data, need timestamp
    TArray<FMyMJGameDeskVisualDataDeltaCpp> m_apNewVisualDataDelta;

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


struct FMyMJGameDeskProcessorMainThreadSentLabelCpp
{
public:

    FMyMJGameDeskProcessorMainThreadSentLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskProcessorMainThreadSentLabelCpp()
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

struct FMyMJGameDeskProcessorMainThreadReceivedLabelCpp
{
public:

    FMyMJGameDeskProcessorMainThreadReceivedLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskProcessorMainThreadReceivedLabelCpp()
    {

    };

    inline
    void reset()
    {
        m_uiCfgStateKey = MyUIntIdDefaultInvalidValue;
    };

    uint32 m_uiCfgStateKey;
};


struct FMyMJGameDeskProcessorSubThreadSentLabelCpp
{
public:

    FMyMJGameDeskProcessorSubThreadSentLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskProcessorSubThreadSentLabelCpp()
    {

    };

    inline
        void reset()
    {
        m_uiCfgStateKey = MyUIntIdDefaultInvalidValue;
        m_cVisualData.reset();
        m_uiServerWorldTime_ms = 0;
    };


    uint32 m_uiCfgStateKey;
    
    FMyMJGameDeskVisualDataCpp m_cVisualData;
    //FMyMJGameDeskVisualDataDeltaCpp m_cVisualDataDelta;

    uint32 m_uiServerWorldTime_ms; //note that it is different with m_cVisualData's core data timestamp, which represent that state's supposed time stamp, but here it means real time

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


    //make it public for test purpose
    static void helperResolveCardTransform(const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCfg,
                                            const FMyMJGameActorModelInfoBoxCpp& cCardModelInfo,
                                            const FMyMJGameCardVisualInfoCpp& cCardVisualInfo,
                                            FTransform& outTransform);


protected:

    int32 mainThreadTryFeedEvents(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutHaveFeedEvent);


    virtual bool subThreadInitBeforRun() override;

    virtual void subThreadLoopInRun() override;

    virtual void subThreadExitAfterRun() override;

    void subThreadCmdLoop();
    void subThreadDataLoop();

    void subThreadTryGenOutput(FMyMJEventWithTimeStampBaseCpp* pEventJustApplied);

    //Keywork accumulated means it does NOT reset inside, and the param can contain prev worked data before calling
    static void helperResolveVisualInfoChanges(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                               const FMyMJDataStructWithTimeStampBaseCpp& cNextCoreData,
                                               const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cNextCoreDataDirtyRecordSincePrev,
                                               const FMyMJGameDeskVisualActorDatasCpp& cPrevActorData,
                                               TMap<int32, FMyMJGameCardVisualInfoCpp>& mOutIdCardVisualInfoAccumulatedChanges,
                                               TMap<int32, int32>& mOutIdDiceValueAccumulatedChanges);

    //@mIdCardVisualInfoAccumulatedChanges, @mIdDiceValueAccumulatedChanges may be sorted inside, the out will be cleaned inside and only contain data correspond to input 
    static void helperResolveVisualResultChanges(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                                 TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoAccumulatedChanges,
                                                 TMap<int32, int32>& mIdDiceValueAccumulatedChanges,
                                                 TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultChanges,
                                                 TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mOutIdDiceVisualInfoAndResultChanges);

    //do the final work with resolved same point cfg and card model in prev step
    static void helperResolveCardVisualResultChanges(const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCfg,
                                                     const FMyMJGameActorModelInfoBoxCpp& cCardModelInfo,
                                                     const TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoKnownChanges,
                                                     TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultAccumulatedChanges);


    FMyMJGameDeskVisualCfgCacheCpp m_cMainThreadWaitingToSendCfgCache;

    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdInputCpp>  m_cCmdIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdOutputCpp> m_cCmdOut;

    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataInputCpp>  m_cDataIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataOutputCpp> m_cDataOut;

    //record
    FMyMJGameDeskProcessorMainThreadSentLabelCpp     m_cMainThreadSentLabel;
    FMyMJGameDeskProcessorMainThreadReceivedLabelCpp m_cMainThreadReceivedLabel;

    //used only inside processor
    FMyMJGameDeskVisualDataCpp          *m_pSubThreadData;
    FMyDirtyRecordWithKeyAnd4IdxsMapCpp *m_pSubThreadDataCoreDataDirtyRecordSincePrevSent;
    //FMyMJGameDeskVisualCfgCacheCpp *m_pSubThreadCfgCache;
    //FMyMJGameDeskVisualDataDeltaCpp     *m_pSubThreadDataDeltaSincePrevSent;

    FMyMJDataAccessorCpp* m_pSubThreadAccessor;

    FMyMJGameDeskProcessorSubThreadSentLabelCpp* m_pSubThreadSentLabel;

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
    void loop();


    uint32 updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

    //return true if need to sync base
    bool tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater);

    
    UFUNCTION(BlueprintCallable)
    static void testHelperResolveCardTransform(const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCfg,
                                            const FMyMJGameActorModelInfoBoxCpp& cCardModelInfo,
                                            const FMyMJGameCardVisualInfoCpp& cCardVisualInfo,
                                            FTransform& outTransform)
    {
        FMyMJGameDeskProcessorRunnableCpp::helperResolveCardTransform(cVisualPointCfg, cCardModelInfo, cCardVisualInfo, outTransform);
    };

protected:

    //TSharedPtr<FMyMJGameDeskVisualCoreDataProcessorCpp> m_pDataProcessor;
    TSharedPtr<FMyThreadControlCpp<FMyMJGameDeskProcessorRunnableCpp>> m_pProcessor;


    bool m_bInFullDataSyncState;
};
