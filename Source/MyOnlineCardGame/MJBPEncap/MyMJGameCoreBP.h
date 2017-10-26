// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
//#include "CoreUObject.h"

//#include "UObject/Object.h"
//#include "UObject/NoExportTypes.h"

//#include "Queue.h"
//#include "GameFramework/Actor.h"
//#include "UnrealNetwork.h"

#include "MyMJGameAttenderBP.h"
#include "MyMJGameEventBase.h"

#include "MJLocalCS/MyMJGameCoreLocalCS.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "MyMJGameCoreBP.generated.h"

class FMyMJGameCoreThreadControlCpp : public FMyThreadControlCpp
{
public:
    FMyMJGameCoreThreadControlCpp(MyMJGameRuleTypeCpp eRuleType, int32 iSeed, int32 iTrivalConfigMask) : FMyThreadControlCpp(MY_MJ_GAME_CORE_FULL_SUB_THREAD_LOOP_TIME_MS)
    {
        m_eRuleType = eRuleType;
        m_iSeed.Set(iSeed);

        m_ppCoreInRun = NULL;
        m_iTrivalConfigMask = iTrivalConfigMask;
        MY_VERIFY(start());

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGameCoreThreadControlCpp create()."));
    };

    virtual ~FMyMJGameCoreThreadControlCpp()
    {
        //clean up ahead, otherwise it may crash since subthread may still using vtable which is cleaned here
        cleanUp();

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("~FMyMJGameCoreThreadControlCpp destroy()."));

    };


    inline MyMJGameRuleTypeCpp getRuleType()
    {
        return m_eRuleType;
    };

    inline FMyMJGameIOGroupAllCpp& getIOGourpAll()
    {
        return m_cIOGourpAll;
    };


protected:

    bool start()
    {
        MY_VERIFY(m_eRuleType != MyMJGameRuleTypeCpp::Invalid);
        return FMyThreadControlCpp::start();
    }

    virtual void beginInRun() override
    {
        int32 testV = 1 + 3 > 2;
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("thread started, beginInRun(), testV %d"), testV);

        FMyMJGameCoreCpp *pCore = UMyMJBPUtilsLibrary::helperCreateCoreByRuleType(m_eRuleType, m_iSeed.GetValue(), m_iTrivalConfigMask);

        if (m_ppCoreInRun) {
            delete(m_ppCoreInRun);
            m_ppCoreInRun = NULL;
        }

        m_ppCoreInRun = new TSharedPtr<FMyMJGameCoreCpp>();
        *m_ppCoreInRun = MakeShareable<FMyMJGameCoreCpp>(pCore);
        pCore->initFullMode(*m_ppCoreInRun, &m_cIOGourpAll);
    };

    virtual void loopInRun() override
    {
        (*m_ppCoreInRun)->tryProgressInFullMode();
    };

    virtual void endInRUn() override
    {
        if (m_ppCoreInRun) {
            delete m_ppCoreInRun;
            m_ppCoreInRun = NULL;
        }
    };

    virtual void Exit() override
    {
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
    };

    //logic related, owned by this class, which means parent thread
    MyMJGameRuleTypeCpp m_eRuleType;
    FThreadSafeCounter m_iSeed;
    FMyMJGameIOGroupAllCpp m_cIOGourpAll;

    int32 m_iTrivalConfigMask;

    TSharedPtr<FMyMJGameCoreCpp> *m_ppCoreInRun;


};


/*
* this only exist on server, in network env
*/
//notplaceable
UCLASS(BlueprintType, notplaceable, meta = (ShortTooltip = "The full game core with sub thread, should only exist on server"))
class MYONLINECARDGAME_API UMyMJGameCoreFullCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJGameCoreFullCpp() : Super() {

        m_iSeed2OverWrite = 0;
        //m_pPusherBuffer = NULL;
        //m_apNextNodes.Reset();
        m_pCoreFullWithThread = NULL;
    };

    //test with a full mode core created and run in sub thread
    UFUNCTION(BlueprintCallable, Category = "UMyMJGameCoreFullCpp")
    void testGameCoreInSubThread(bool showCoreLog, bool bAttenderRandomSelectHighPriActionFirst);

    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask);
    bool startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst);

    void clearUp();

    inline TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc>* getPusherResultQueue()
    {
        if (m_pCoreFullWithThread.IsValid()) {
            return &m_pCoreFullWithThread->getIOGourpAll().m_cPusherResultQueue;
        }

        return NULL;
    };


protected:

    //if not 0, it will be used as seed of random
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

    TSharedPtr<FMyMJGameCoreThreadControlCpp> m_pCoreFullWithThread;

};


#define MY_EXPECTED_MJ_PAWN_NUM ((uint8)MyMJGameRoleTypeCpp::Max)

//This level focus on logic, it may work either connected to full mode core or NULL, since for graphic and BP usage, we didn't need the full mode core
UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCoreMirrorCpp : public AActor
{
    GENERATED_BODY()
public:

    AMyMJGameCoreMirrorCpp() : Super()
    {
        m_iSeed2OverWrite = 0;
        m_iTest0 = 0;

        bReplicates = true;
        bAlwaysRelevant = true;
        bNetLoadOnClient = true;
        NetUpdateFrequency = 10;

        m_pMJDataAll = NULL;
        m_pCoreFull = NULL;
    }

    //virtual void PostInitProperties() override;
    //virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    //virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    UFUNCTION(BlueprintCallable, Category = "AMyMJGameCoreMirrorCpp")
        void doTestChange()
    {
        if (m_pMJDataAll) {
            m_pMJDataAll->doTestChange();
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMJDataStorage is NULL!"));
        }

        m_iTest0 += 3;
    };

    UFUNCTION(BlueprintCallable, Category = "AMyMJGameCoreMirrorCpp")
        FString genDebugMsg()
    {
        FString ret = FString::Printf(TEXT("m_iTest0 %d. "), m_iTest0);
        if (m_pMJDataAll) {
            ret += m_pMJDataAll->genDebugMsg();
        }
        else {
            ret += TEXT("m_pMJDataStorage is NULL!");
        }

        return ret;
    };

    UFUNCTION(BlueprintCallable, Category = "AMyMJGameCoreMirrorCpp")
    void verifyEvents() const
    {
        if (!IsValid(m_pMJDataAll)) {
            return;
        }
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("verifyEvents() exectuing."));

        for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            UMyMJDataSequencePerRoleCpp*pSeq = m_pMJDataAll->getDataByRoleType((MyMJGameRoleTypeCpp)i, false);
            if (!IsValid(pSeq)) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("skipping role %d's data since not valid."), i);
                continue;
            }
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("verifyEvents() for role %d."), i);
            int32 iError = pSeq->getEventsRef().verifyData(true);
            if (iError != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d's data have error %d."), i, iError);
                //MY_VERIFY(false);
            }
        }

        UMyMJDataSequencePerRoleCpp*pSeq = m_pMJDataAll->m_pDataTest0;
        if (!IsValid(pSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pDataTest0 not valid."));
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("verifyEvents() for m_pDataTest0."));
            int32 iError = pSeq->getEventsRef().verifyData(true);
            if (iError != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataTest0 data have error %d."), iError);
                //MY_VERIFY(false);
            }
        }

        pSeq = m_pMJDataAll->m_pDataTest1;
        if (!IsValid(pSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pDataTest1 not valid."));
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("verifyEvents() for m_pDataTest1."));
            int32 iError = pSeq->getEventsRef().verifyData(true);
            if (iError != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataTest1 data have error %d."), iError);
                //MY_VERIFY(false);
            }
        }
    };

    void toCoreFullLoop();

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
        int32 m_iSeed2OverWrite;

    UPROPERTY(Replicated)
        int32 m_iTest0;

protected:

    inline bool getCoreFullPartEnabled() const
    {
        return IsValid(m_pCoreFull);
    };

    void setCoreFullPartEnabled(bool bEnabled);

    UFUNCTION()
    virtual void OnRep_MJDataAll()
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_MJDataAll() 0, this is base class and shouldn't be called."), m_iTest0);
        for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            UMyMJDataSequencePerRoleCpp* pSeq = m_pMJDataAll->getDataByRoleType((MyMJGameRoleTypeCpp)i);
            MY_VERIFY(IsValid(pSeq));
        }
    }


    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "data of storage"))
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MJDataAll, meta = (DisplayName = "data of all roles"))
    UMyMJDataAllCpp* m_pMJDataAll;

    //exist only when we have logic layer which generate data
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "core full"))
    UMyMJGameCoreFullCpp *m_pCoreFull;

    FTimerHandle m_cToCoreFullLoopTimerHandle;

};


UENUM()
enum class MyMJCoreBaseForBpVisualModeCpp : uint8
{
    Normal = 1                              UMETA(DisplayName = "Normal"),
    CatchUp = 2                             UMETA(DisplayName = "CatchUp"),
};

//This level focus on BP and Graphic
UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCoreWithVisualCpp : public AMyMJGameCoreMirrorCpp
{
    GENERATED_BODY()

public:

    AMyMJGameCoreWithVisualCpp();
    virtual ~AMyMJGameCoreWithVisualCpp();

    inline
    const FMyMJDataAtOneMomentCpp& getDataNowRefConst() const
    {
        return m_cDataNow;
    };

    inline
    FMyMJDataAtOneMomentCpp& getDataNowRef()
    {
        return m_cDataNow;
    };

protected:

    //virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;

    //for visual, we need mirror data
    inline bool getCoreMirrorPartEnabled() const
    {
        return IsValid(m_pDataHistoryBuffer);
    };

    void setCoreMirrorPartEnabled(bool bEnabled);

    UFUNCTION()
    virtual void OnRep_MJDataAll()
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("OnRep_MJDataAll() 1, m_iTest0 %d."), m_iTest0);
        MY_VERIFY(IsValid(m_pMJDataAll));

        m_pMJDataAll->m_cReplicateDelegate.Clear();
        m_pMJDataAll->m_cReplicateDelegate.AddUObject(this, &AMyMJGameCoreWithVisualCpp::onDataSeqReplicated);
    }

    void onDataSeqReplicated(UMyMJDataSequencePerRoleCpp *pSeq, int32 iExtra)
    {
        const UMyMJGameEventCycleBuffer* pEvents = pSeq->getEvents(false);
        pSeq->getGameIdLast();
        int32 eventsCount = 0;
        if (IsValid(pEvents)) {
            eventsCount = pEvents->getCount(NULL);
        }

        float clientTimeNow = 0;
        UWorld* world = GetWorld();
        if (IsValid(world)) {
            clientTimeNow = world->GetTimeSeconds();
        }

        uint32 clientTimeNowMs = clientTimeNow * 1000;

        //filter out duplicated notify
        if ((clientTimeNowMs - m_uiReplicateClientTimeMs) <= 0) {
            return;
        }
        m_uiReplicateClientTimeMs = clientTimeNowMs;

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("time %.3f: onDataSeqReplicated(), role %d, %d, events valid %d, count %d, last %d:%d."), clientTimeNow, (uint8)pSeq->m_eRole, iExtra, IsValid(pEvents), eventsCount, pSeq->getGameIdLast(), pSeq->getPusherIdLast());
        
        forVisualLoop();
    };

    bool tryAppendData2Buffer();

    //UFUNCTION(BlueprintImplementableEvent)
    void onEventAppliedWithDur(const FMyMJEventWithTimeStampBaseCpp& newEvent) {};

    UFUNCTION(BlueprintCallable)
    void changeViewRole(MyMJGameRoleTypeCpp eRoleType)
    {
        MY_VERIFY(IsValid(m_pDataHistoryBuffer));

        m_cDataNow.reinit(eRoleType);
        m_pDataHistoryBuffer->reinit(eRoleType);
    };

    UFUNCTION(BlueprintCallable)
    void changeVisualMode(MyMJCoreBaseForBpVisualModeCpp eVisualMode)
    {
        MyMJCoreBaseForBpVisualModeCpp eVisualModeOld = m_eVisualMode;
        m_eVisualMode = eVisualMode;

        float clientTimeNow = 0;
        UWorld* world = GetWorld();
        if (IsValid(world)) {
            clientTimeNow = world->GetTimeSeconds();
        }

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("time %.3f: changeVisualMode %s -> %s."), clientTimeNow, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCoreBaseForBpVisualModeCpp"), (uint8)eVisualModeOld), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCoreBaseForBpVisualModeCpp"), (uint8)eVisualMode));
        onVisualModeChanged(eVisualModeOld, eVisualMode);
    };

    //UFUNCTION(BlueprintImplementableEvent)
    void onVisualModeChanged(MyMJCoreBaseForBpVisualModeCpp eVisualModeOld, MyMJCoreBaseForBpVisualModeCpp eVisualMode) {};

    void forVisualLoop();
    void forVisualLoopModeNormal(uint32 clientTimeNow_ms);
    void forVisualLoopModeCatchUp(uint32 clientTimeNow_ms);

    //this is the visual data
    UPROPERTY()
    UMyMJDataSequencePerRoleCpp* m_pDataHistoryBuffer;

    //this is the current state used for visualize
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data now"))
    FMyMJDataAtOneMomentCpp m_cDataNow;


    MyMJCoreBaseForBpVisualModeCpp m_eVisualMode;

    FTimerHandle m_cForVisualLoopTimerHandle;

    uint32 m_uiReplicateClientTimeMs;
};