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

    };

    virtual ~FMyMJGameDeskVisualDataDirtyRecordCpp()
    {

    };

    //time segment data
    uint32 m_uiPrevServerTime_ms_unresolved; //Todo: make timestamp unique to act as unique state label
    int32 m_iPrevGameId; //this is the unique state lable, to meausre progress precisely and guarentte no key events missed.
    int32 m_iPrevPushId;

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

    TArray<FMyMJGameDeskVisualCfgCacheCpp> m_apNewCfgCache;
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
    FMyMJGameDeskVisualDataOneMomentCpp m_cDataNow;
    FMyMJGameDeskVisualActorDatasCpp m_cExpectedActorData;
};

//#define FMyMJGameDeskSuiteCoreDataProcessorSubThreadLoopTimeMs (10)

USTRUCT()
struct FMyMJGameDeskVisualCoreDataProcessorInputCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualCoreDataProcessorInputCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskVisualCoreDataProcessorInputCpp()
    {
        //reset();
    };

    inline
    void reset()
    {
        m_apNewCfgCache.Reset();
        m_apFull.Reset();
        m_apDelta.Reset(1);
        m_uiUpdateServerWorldTime_ms = 0;
    };

    inline
    void verifyValid() const
    {
        if (m_apNewCfgCache.Num() > 0) {
            MY_VERIFY(m_apNewCfgCache.Num() == 1);
            MY_VERIFY(m_apFull.Num() == 0);
            MY_VERIFY(m_apDelta.Num() == 0);
        }
        else if (m_apFull.Num() > 0) {
            MY_VERIFY(m_apNewCfgCache.Num() == 0);
            MY_VERIFY(m_apFull.Num() == 1);
            MY_VERIFY(m_apDelta.Num() == 0);

            MY_VERIFY(m_uiUpdateServerWorldTime_ms > 0);
        }
        else if (m_apDelta.Num() > 0) {
            MY_VERIFY(m_apNewCfgCache.Num() == 0);
            MY_VERIFY(m_apFull.Num() == 0);
            MY_VERIFY(m_apDelta.Num() == 1);

            MY_VERIFY(m_uiUpdateServerWorldTime_ms > 0);
        }
        else {
            MY_VERIFY(m_uiUpdateServerWorldTime_ms > 0);
        }
    };

    TArray<FMyMJGameDeskVisualCfgCacheCpp> m_apNewCfgCache;
    TArray<FMyMJDataStructWithTimeStampBaseCpp> m_apFull;
    TArray<FMyMJEventWithTimeStampBaseCpp> m_apDelta;
    uint32 m_uiUpdateServerWorldTime_ms; // > 0 means valid, == 0 means not touching the progress, maybe a config change
};

USTRUCT()
struct FMyMJGameDeskVisualCoreDataProcessorOutputCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualCoreDataProcessorOutputCpp()
    {
        reset();
    };

    virtual ~FMyMJGameDeskVisualCoreDataProcessorOutputCpp()
    {

    };

    inline
    void reset()
    {
        m_apNext.Reset();
        m_apDirtyRecordSincePrev.Reset();
        m_uiUpdateServerWorldTime_ms = 0;
    };

    inline
    void verifyValid() const
    {
        if (m_apNext.Num() > 0) {
            MY_VERIFY(m_apNext.Num() == 1);

            const FMyMJGameDeskVisualDataOneMomentCpp& DataOneMoment = m_apNext[0];
            //check it's type
            if (DataOneMoment.m_apNewCfgCache.Num() > 0) {
                MY_VERIFY(DataOneMoment.m_apNewCfgCache.Num() == 1);

                MY_VERIFY(DataOneMoment.m_cCoreData.getCoreDataPublicRefConst().m_iGameId == MyIntIdDefaultInvalidValue);
                MY_VERIFY(m_apDirtyRecordSincePrev.Num() == 0);
                MY_VERIFY(m_uiUpdateServerWorldTime_ms == 0);
            }
            else {
                
                MY_VERIFY(DataOneMoment.m_cCoreData.getCoreDataPublicRefConst().m_iGameId != MyIntIdDefaultInvalidValue);
                MY_VERIFY(m_apDirtyRecordSincePrev.Num() == 1);
                MY_VERIFY(m_uiUpdateServerWorldTime_ms > 0);
            }
        }
        else if (m_apDirtyRecordSincePrev.Num() > 0) {
            MY_VERIFY(m_apNext.Num() == 1);

            const FMyMJGameDeskVisualDataOneMomentCpp& DataOneMoment = m_apNext[0];
            MY_VERIFY(DataOneMoment.m_apNewCfgCache.Num() == 0);

            MY_VERIFY(DataOneMoment.m_cCoreData.getCoreDataPublicRefConst().m_iGameId != MyIntIdDefaultInvalidValue);
            MY_VERIFY(m_apDirtyRecordSincePrev.Num() == 1);
            MY_VERIFY(m_uiUpdateServerWorldTime_ms > 0);
        }
        else {
            MY_VERIFY(m_uiUpdateServerWorldTime_ms > 0);
        }
    };

    TArray<FMyMJGameDeskVisualDataOneMomentCpp> m_apNext;
    TArray<FMyMJGameDeskVisualDataDirtyRecordCpp> m_apDirtyRecordSincePrev;
    uint32 m_uiUpdateServerWorldTime_ms;
};


struct FMyMJGameDeskDataUniqueLabelCpp
{
public:

    FMyMJGameDeskDataUniqueLabelCpp()
    {
        reset();
    };

    ~FMyMJGameDeskDataUniqueLabelCpp()
    {

    };

    inline
    void reset()
    {
        m_uiStateKey = MyUIntIdDefaultInvalidValue;
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

    uint32 m_uiStateKey;

    MyMJGameRoleTypeCpp m_eRoleType;
    uint32 m_uiIdEvent;
    uint32 m_uiServerWorldTime_ms;

};

#define MyTryFeedEventRetAllProcessed (0)
#define MyTryFeedEventRetDataReplicationIncomplete (-1)
#define MyTryFeedEventRetNeedSyncBase (-2)
#define MyTryFeedEventRetLackBuffer (-3)


class FMyMJGameDeskVisualCoreDataRunnableCpp : public FMyRunnableBaseCpp
{
public:

    FMyMJGameDeskVisualCoreDataRunnableCpp();
    virtual ~FMyMJGameDeskVisualCoreDataRunnableCpp();

    void reset()
    {

    };

    virtual FString genName() const override
    {
        return TEXT("FMyMJGameDeskVisualCoreDataRunnableCpp");
    };

    virtual bool isReadyForSubThread() const override
    {
        return true;
    };

    inline const FMyMJGameDeskDataUniqueLabelCpp& getLabelLastInRefConst() const
    {
        return m_cLabelLastIn;
    };

    //return true if need to sync base
    bool tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq);

    //return error code
    int32 updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

protected:

    virtual bool initBeforRun() override;

    virtual void loopInRun() override;

    virtual void exitAfterRun() override;

    int32 tryFeedEvents(UMyMJDataSequencePerRoleCpp *pSeq, bool *pOutHaveFeedEvent);

    FMyQueueWithLimitBuffer<FMyMJGameDeskVisualCoreDataProcessorInputCpp>  m_cInRawCoreData;
    FMyQueueWithLimitBuffer<FMyMJGameDeskVisualCoreDataProcessorOutputCpp> m_cOutCalcuatedVisualData;

    //input record
    FMyMJGameDeskDataUniqueLabelCpp m_cLabelLastIn;
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

    int32 updateCfgCache(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);
    uint32 getLastInCfgCacheStateKey() const;


    //return true if need to sync base
    bool tryFeedData(UMyMJDataSequencePerRoleCpp *pSeq);



protected:

    //TSharedPtr<FMyMJGameDeskVisualCoreDataProcessorCpp> m_pDataProcessor;
    TSharedPtr<FMyThreadControlCpp<FMyMJGameDeskVisualCoreDataRunnableCpp>> m_pDataProcessorWithThread;


    bool m_bInFullDataSyncState;
};
