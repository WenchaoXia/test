// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameVisualElems.h"


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


    //return errocode. to make it easy to use, list every maintype's api to tip the parameter meanings
    int32 getCardVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyArrangePointCfgWorld3DCpp &visualPoint) const;
    void  setCardVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, const FMyArrangePointCfgWorld3DCpp &visualPoint);

    //return errocode.
    int32 getAttenderVisualPointCfg(int32 idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp eSubtype, FMyArrangePointCfgWorld3DCpp &visualPoint) const;
    void  setAttenderVisualPointCfg(int32 idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp eSubtype, const FMyArrangePointCfgWorld3DCpp &visualPoint);

    //return errocode.
    int32 getTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyArrangePointCfgWorld3DCpp &visualPoint) const;
    void  setTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, const FMyArrangePointCfgWorld3DCpp &visualPoint);

protected:

    static int32 helperGetVisualPointCfgByIdxs(int32 idx0, int32 idx1, int32 idx2, const TMap<int32, FMyArrangePointCfgWorld3DCpp>& cTargetMap, FMyArrangePointCfgWorld3DCpp &visualPoint);
    static void  helperSetVisualPointCfgByIdxs(int32 idx0, int32 idx1, int32 idx2, TMap<int32, FMyArrangePointCfgWorld3DCpp>& cTargetMap, const FMyArrangePointCfgWorld3DCpp &visualPoint);

    //by split to two domains, it runs a bit faster at runtime
    //card cfg
    TMap<int32, FMyArrangePointCfgWorld3DCpp> m_mCardVisualPointCache;

    //other cfg except card
    TMap<int32, FMyArrangePointCfgWorld3DCpp> m_mTrivalVisualPointCache;
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

    FMyModelInfoBoxWorld3DCpp m_cCardModelInfo;
    FMyCardGameDiceModelInfoCpp m_cDiceModelInfo;
};

USTRUCT(BlueprintType)
struct FMyMJGameDeskVisualCfgCacheCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualCfgCacheCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskVisualCfgCacheCpp()
    {

    };

    inline void reset()
    {
        m_bValid = false;
        m_uiStateKey = MyUIntIdDefaultInvalidValue;
        m_cPointCfg.clear();
        m_cModelInfo.clear();
    };

    MyErrorCodeCommonPartCpp helperGetMyArrangePointCfgForCard(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyArrangePointCfgWorld3DCpp& outVisualPointCfg) const
    {

        int32 retCode = m_cPointCfg.getCardVisualPointCfgByIdxAttenderAndSlot(idxAttender, eSlot, outVisualPointCfg);
        if (retCode != 0) {
            return MyErrorCodeCommonPartCpp::ObjectNull;
        }

        return MyErrorCodeCommonPartCpp::NoError;
    };

    bool m_bValid;
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
        reset();
    };

        virtual ~FMyMJGameDeskVisualDataCpp()
    {

    };

        inline
    void reset()
    {
        m_cCfgCache.reset();
        m_cCoreData.reset();
        m_cActorData.reset();

        m_uiServerWorldTime_ms = 0;
    };

    void applyDelta(struct FMyMJGameDeskVisualDataDeltaCpp& cDelta, uint32 uiNewServerWorldTime_ms);
    
    inline
    void setCfg(const FMyMJGameDeskVisualCfgCacheCpp &cCfgCache)
    {
        m_cCfgCache = cCfgCache;
    }

    inline
    const FMyMJGameDeskVisualCfgCacheCpp& getCfgRefConst() const
    {
        return m_cCfgCache;
    };

    inline
    void setTime(uint32 uiNewServerWorldTime_ms)
    {
        MY_VERIFY(uiNewServerWorldTime_ms >= m_uiServerWorldTime_ms);
        m_uiServerWorldTime_ms = uiNewServerWorldTime_ms;
    };

    inline
    uint32 getTime() const
    {
        return m_uiServerWorldTime_ms;
    };

    inline
    const FMyMJDataStructWithTimeStampBaseCpp& getCoreDataRefConst() const
    {
        return m_cCoreData;
    };

    //Warning: only use it inide process to assemble dirty data
    inline 
    FMyMJDataStructWithTimeStampBaseCpp& getCoreDataRef()
    {
        return m_cCoreData;
    };

    inline
    const FMyMJGameDeskVisualActorDatasCpp& getActorDataRefConst() const
    {
        return m_cActorData;
    };

protected:

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
        m_bHelperSkippedEventBefore = false;
    };

    inline void reset()
    {
        m_apNewCoreData.Reset();
        m_apNewCoreDataDirtyRecordFiltered.Reset();
        m_mNewActorDataIdCards.Reset();
        m_mNewActorDataIdDices.Reset();
        m_apEventJustApplied.Reset();

        m_bHelperSkippedEventBefore = false;
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
    TArray<FMyDirtyRecordWithKeyAnd4IdxsMapCpp> m_apNewCoreDataDirtyRecordFiltered; //the other direty record except card and dice

    TMap<int32, FMyMJGameCardVisualInfoAndResultCpp> m_mNewActorDataIdCards;
    TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp> m_mNewActorDataIdDices;

    TArray<FMyMJEventWithTimeStampBaseCpp> m_apEventJustApplied; //if valid, means this is a event need show, and no previous event skipped, otherwise this is a core data update
    int32 m_bHelperSkippedEventBefore : 1;
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

    inline
    static void helperApplyToDeskVisualData(FMyMJGameDeskVisualDataCpp& cTarget, FMyMJGameDeskProcessorDataOutputCpp& data)
    {
        if (data.m_apNewVisualDataDelta.Num() > 0) {
            cTarget.applyDelta(data.m_apNewVisualDataDelta[0], data.m_uiNewServerWorldTime_ms);
        }
        else {
            cTarget.setTime(data.m_uiNewServerWorldTime_ms);
        }
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

struct FMyMJGameDeskProcessorLabelCpp
{
public:

    FMyMJGameDeskProcessorLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskProcessorLabelCpp()
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

        m_eRoleType = eRole;
        m_uiIdEvent = idEvent;

        updateServerWorldTime(uiServerWorldTime_ms);
    };

    inline
    void updateServerWorldTime(uint32 uiServerWorldTime_ms)
    {
        if (uiServerWorldTime_ms < m_uiServerWorldTime_ms) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw, uiServerWorldTime_ms %u, m_uiServerWorldTime_ms %u."), uiServerWorldTime_ms, m_uiServerWorldTime_ms);
            MY_VERIFY(false);
        }
        m_uiServerWorldTime_ms = uiServerWorldTime_ms;
    };

    uint32 m_uiCfgStateKey;
    MyMJGameRoleTypeCpp m_eRoleType;
    uint32 m_uiIdEvent;
    uint32 m_uiServerWorldTime_ms;
};

struct FMyMJGameDeskProcessorMainThreadSentLabelCpp : public FMyMJGameDeskProcessorLabelCpp
{
public:

};

struct FMyMJGameDeskProcessorMainThreadReceivedLabelCpp : public FMyMJGameDeskProcessorLabelCpp
{
public:

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
        //m_uiCfgStateKey = MyUIntIdDefaultInvalidValue;
        m_cVisualData.reset();
        //m_uiServerWorldTime_ms = 0;
    };

    //Todo:: not all used, optimize it to save some step such as coreData, actor final transform copy.
    FMyMJGameDeskVisualDataCpp m_cVisualData;
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

    //return true if need to sync base
    bool mainThreadTryFeedData(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater, bool *pOutHaveFeedData);

    void mainThreadCmdLoop();
    void mainThreadDataLoop();

    inline
    FMyMJGameDeskProcessorCmdOutputCpp*  mainThreadGetCmdOutputForConsume()
    {
        FMyMJGameDeskProcessorCmdOutputCpp* pRet = NULL;
        m_cCmdOutBufferForExt.Dequeue(pRet);
        return pRet;
    };

    inline
    void mainThreadPutCmdOutputAfterConsume(FMyMJGameDeskProcessorCmdOutputCpp*  item)
    {
        m_cCmdOut.putInConsumedItem(item);
    };

    inline
    FMyMJGameDeskProcessorDataOutputCpp* mainThreadPeekDataOutputForConsume()
    {
        FMyMJGameDeskProcessorDataOutputCpp* pRet = NULL;
        m_cDataOutBufferForExt.Peek(pRet);
        return pRet;
    };

    inline
    FMyMJGameDeskProcessorDataOutputCpp* mainThreadGetDataOutputForConsume()
    {
        FMyMJGameDeskProcessorDataOutputCpp* pRet = NULL;
        m_cDataOutBufferForExt.Dequeue(pRet);
        return pRet;
    };

    inline
    void mainThreadPutDataOutputAfterConsume(FMyMJGameDeskProcessorDataOutputCpp* item)
    {
        m_cDataOut.putInConsumedItem(item);
    };

    inline
    const FMyMJGameDeskProcessorMainThreadReceivedLabelCpp& getMainThreadReceivedLabel() const
    {
        return m_cMainThreadReceivedLabel;
    };

protected:

    int32 mainThreadTryFeedEvents(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutHaveFeedEvent);


    virtual bool subThreadInitBeforRun() override;

    virtual void subThreadLoopInRun() override;

    virtual void subThreadExitAfterRun() override;

    void subThreadCmdLoop();
    void subThreadDataLoop();

    //return 0 if generated, otherwise error code
    int32 subThreadTryGenOutput(FMyMJEventWithTimeStampBaseCpp* pEventJustApplied);

    //Keywork accumulated means it does NOT reset inside, and the param can contain prev worked data before calling
    //Assume the point have: col->Y, row->X, stack->Z
    static void helperResolveVisualInfoChanges(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                               const FMyMJDataStructWithTimeStampBaseCpp& cNextCoreData,
                                               const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cNextCoreDataDirtyRecordSincePrev,
                                               const FMyMJGameDeskVisualActorDatasCpp& cPrevActorData,
                                               FMyDirtyRecordWithKeyAnd4IdxsMapCpp& outDirtyRecordFiltered,
                                               TMap<int32, FMyMJGameCardVisualInfoCpp>& mOutIdCardVisualInfoAccumulatedChanges,
                                               bool& bDicesAccumulatedChanges);

    //Todo: resolve dices
    //@mIdCardVisualInfoAccumulatedChanges, @mIdDiceValueAccumulatedChanges may be sorted inside, the out will be cleaned inside and only contain data correspond to input 
    static void helperResolveVisualResultChanges(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                                 const FMyMJDataStructWithTimeStampBaseCpp& cNextCoreData,
                                                 TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoAccumulatedChanges,
                                                 bool bDicesAccumulatedChanges,
                                                 TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultChanges,
                                                 TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mOutIdDiceVisualInfoAndResultChanges);


    FMyMJGameDeskVisualCfgCacheCpp m_cMainThreadWaitingToSendCfgCache;

    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdInputCpp>  m_cCmdIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorCmdOutputCpp> m_cCmdOut;
    TQueue<FMyMJGameDeskProcessorCmdOutputCpp*>  m_cCmdOutBufferForExt; //another layer of buffer, to allow we scan info inside before external usage
    //Todo: use cycle array buffer instead of queue, to save memory allocation step for each item

    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataInputCpp>  m_cDataIn;
    FMyQueueWithLimitBuffer<FMyMJGameDeskProcessorDataOutputCpp> m_cDataOut;
    TQueue<FMyMJGameDeskProcessorDataOutputCpp*> m_cDataOutBufferForExt; //another layer of buffer, to allow we scan info inside before external usage


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

    int32 m_bSubThreadHelperSkippedEventBefore : 1;
};

UENUM()
enum class EMyMJGameRoomVisualStateCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    WaitingForDataInitSync = 1     UMETA(DisplayName = "WaitingForDataInitSync"),
    WaitingForDataInGame = 2     UMETA(DisplayName = "WaitingForDataInGame"),
    NormalPlay = 3    UMETA(DisplayName = "NormalPlay"),
    CatchUp = 4    UMETA(DisplayName = "CatchUp")
};

USTRUCT()
struct FMyGameProgressDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyGameProgressDataCpp()
    {
        reset();
    };

    inline void reset() {
        m_uiServerTimeConfirmed_ms = 0;
        m_cLastBond.clearBond();
    };

    uint32 m_uiServerTimeConfirmed_ms; //how much we played and confirmed from data received, not prediction (out prediction is simply play ahead)
    FMyMJServerClientTimeBondCpp m_cLastBond;
};

UCLASS(Blueprintable)
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
    void loop(uint32 uiClientWorldTimeNow_ms);

    inline const FMyMJGameDeskVisualDataCpp& getVisualDataRefConst() const
    {
        return m_cDeskVisualDataNow;
    };

    uint32 updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

    //return true if need to sync base
    bool tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutRetryLater);

    //first and last will be 0 if all data already consumed
    void getDataTimeRange(uint32 &uiFirstDataGotServerTime_ms, uint32 &uiLastDataGotServerTime_ms) const;
    

protected:

    void cmdLoop();
    void dataLoop(uint32 uiClientWorldTimeNow_ms);

    void changeVisualState(EMyMJGameRoomVisualStateCpp eNewState, uint32 uiClientTimeNow_ms, int32 iDebug)
    {
        if (m_eVisualState != eNewState) {
            m_eVisualState = eNewState;
            m_uiVisualStateStartClientTime_ms = uiClientTimeNow_ms;
        }

    };

    //play all events <= uiServerTime_ms
    void playGameProgressTo(uint32 uiServerTime_ms, bool bCatchUp);

    class AMyMJGameRoomCpp* getRoomVerified() const;


    //TSharedPtr<FMyMJGameDeskVisualCoreDataProcessorCpp> m_pDataProcessor;
    TSharedPtr<FMyThreadControlCpp<FMyMJGameDeskProcessorRunnableCpp>> m_pProcessor;

    bool m_bInFullDataSyncState;


    FMyMJGameDeskVisualDataCpp m_cDeskVisualDataNow;
    FMyGameProgressDataCpp m_cGameProgressData;
    EMyMJGameRoomVisualStateCpp m_eVisualState;
    uint32 m_uiVisualStateStartClientTime_ms;
};
