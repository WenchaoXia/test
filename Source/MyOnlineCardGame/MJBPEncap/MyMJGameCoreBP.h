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

//Note once created, subthread would start working
class FMyMJGameCoreRunnableCpp : public FMyRunnableBaseCpp
{
public:
    FMyMJGameCoreRunnableCpp();
    virtual ~FMyMJGameCoreRunnableCpp();

    void initData(MyMJGameRuleTypeCpp eRuleType, int32 iSeed, int32 iTrivalConfigMask)
    {
        MY_VERIFY(isReadyForMainThreadClean()); //ready for clean, equal to ready for init
        
        m_eRuleType = eRuleType;
        m_iSeed.Set(iSeed);
        m_iTrivalConfigMask = iTrivalConfigMask;

        MY_VERIFY(m_eRuleType != MyMJGameRuleTypeCpp::Invalid);
    };

    inline MyMJGameRuleTypeCpp getRuleType()
    {
        return m_eRuleType;
    };

    inline FMyMJGameIOGroupAllCpp& getIOGourpAll()
    {
        return m_cIOGourpAll;
    };


    virtual FString anyThreadGenName() const override
    {
        return TEXT("FMyMJGameCoreRunnableCpp");
    };

    virtual bool subThreadIsReady() const override
    {
        return m_eRuleType != MyMJGameRuleTypeCpp::Invalid;
    };

protected:

    virtual bool subThreadInitBeforRun() override
    {
        if (!FMyRunnableBaseCpp::subThreadInitBeforRun()) {
            return false;
        }

        int32 testV = 1 + 3 > 2;
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("thread started, subThreadInitBeforRun(), testV %d"), testV);

        FMyMJGameCoreCpp *pCore = UMyMJBPUtilsLibrary::helperCreateCoreByRuleType(m_eRuleType, m_iSeed.GetValue(), m_iTrivalConfigMask);

        if (m_ppCore) {
            delete(m_ppCore);
            m_ppCore = NULL;
        }

        m_ppCore = new TSharedPtr<FMyMJGameCoreCpp>();
        *m_ppCore = MakeShareable<FMyMJGameCoreCpp>(pCore);
        pCore->initFullMode(*m_ppCore, &m_cIOGourpAll);

        return true;
    };

    virtual void subThreadLoopInRun() override
    {
        (*m_ppCore)->tryProgressInFullMode();
    };

    virtual void subThreadExitAfterRun() override
    {
        FMyRunnableBaseCpp::subThreadExitAfterRun();

        if (m_ppCore) {
            delete(m_ppCore);
            m_ppCore = NULL;
        }
    };

    //logic related, owned by this class, which means parent thread
    MyMJGameRuleTypeCpp m_eRuleType;
    FThreadSafeCounter m_iSeed;
    FMyMJGameIOGroupAllCpp m_cIOGourpAll;

    int32 m_iTrivalConfigMask;

    //completely managed in subthread
    TSharedPtr<FMyMJGameCoreCpp>* m_ppCore;
};


#define MyNeedRebootReason_LackIdEvent (1)
#define MyNeedRebootReason_LackServerWorldTimeMs (2)

//#define MY_EXPECTED_MJ_PAWN_NUM ((uint8)MyMJGameRoleTypeCpp::Max)

//This level focus on logic, the data source, exist both on server and client, but only server will handle data and generate output
UCLASS()
class MYONLINECARDGAME_API AMyMJGameCoreDataSourceCpp : public AActor
{
    GENERATED_BODY()
public:

    AMyMJGameCoreDataSourceCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual ~AMyMJGameCoreDataSourceCpp();

    //can't be called on deconstructor
    void clearInGame();

    //test functions start:

    UFUNCTION(BlueprintCallable)
    void doTestChange();

    UFUNCTION(BlueprintCallable)
    FString genDebugMsg() const;

    UFUNCTION(BlueprintCallable)
        void verifyEvents() const;

    //end


    //game control functions start:

    UFUNCTION(BlueprintCallable)
    void startGameCoreTestInSubThread(bool showCoreLog, bool showDataLog, bool bAttenderRandomSelectHighPriActionFirst);

    //return true if changed successfully, eRuleType == MyMJGameRuleTypeCpp::Invalid means destroy but not create new one
    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask);
    
    bool startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst);
    void stopGame();

    //end
   
    inline UMyMJDataAllCpp* getMJDataAll(bool bVerify = true)
    {
        if (bVerify) {
            MY_VERIFY(IsValid(m_pMJDataAll));
        }
        return m_pMJDataAll;
    };
    
    inline const UMyMJDataAllCpp* getMJDataAllConst()
    {
        return m_pMJDataAll;
    };

    bool getCoreFullPartEnabled() const;
    void setCoreFullPartEnabled(bool bEnabled);

    //FMyMJDataSeqReplicatedDelegate m_cReplicateFilteredDelegate;

    UPROPERTY(Replicated)
        int32 m_iTest0;

protected:

    //virtual void PostInitProperties() override;
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    //virtual void ForceNetUpdate() override;
    virtual void Tick(float DeltaSeconds) override;

    //virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
    virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

    void loop();
    void coreDataPullLoopInner();
    void markDataChanged(uint32 uiServerWorldTime_ms, bool bForceUpdate); //uiServerWorldTime_ms is the key representing the state, and this function encapsule all data notify to clients in any game mode
    //void notifyGameHaveNewProgress(float fWorldRealTime);
    //void fakeReplicationForLocalVisualLayer(float fWorldRealTime, bool bForced);

    void setNeedReboot(bool bNeedReboot, int32 iDebugReason);

    UFUNCTION()
    void OnRep_MJDataAllPointer()
    {
        if (getCoreFullPartEnabled()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("MJData all is replicated but full part enabled, netmode swithced??"));
        }
    };

    //if not 0, it will be used as seed of random
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

    TSharedPtr<FMyThreadControlCpp<FMyMJGameCoreRunnableCpp>> m_pCoreFullWithThread;

    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "data of storage"))
    UPROPERTY(BlueprintReadOnly, Instanced, ReplicatedUsing = OnRep_MJDataAllPointer, meta = (DisplayName = "data of all roles"))
    UMyMJDataAllCpp* m_pMJDataAll;

    //FTimerHandle m_cToCoreFullLoopTimerHandle;
    bool m_bCoreFullPartEnabled;

    float m_fFakeReplicationForLocalVisualLayerLastRealTime;

    ENetMode m_eDebugNetMode;
    //uint32 m_uiLastReplicateClientTimeMs;
};

/*
DECLARE_MULTICAST_DELEGATE(FMyMJGameCoreAllUpdatedDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FMyMJGameCoreEventAppliedDelegate, const FMyMJEventWithTimeStampBaseCpp&);

//This level focus on BP and Graphic
UCLASS()
class MYONLINECARDGAME_API UMyMJGameCoreWithVisualCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJGameCoreWithVisualCpp();
    virtual ~UMyMJGameCoreWithVisualCpp();

    bool checkSettings() const;

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

    //play all events <= uiServerTime_ms
    void playGameProgressTo(uint32 uiServerTimeNew_ms_unresolved, bool bCatchUp);

    FMyMJGameCoreAllUpdatedDelegate m_cBaseAllUpdatedDelegate;
    FMyMJGameCoreEventAppliedDelegate m_cEventAppliedDelegate;

    UFUNCTION(BlueprintCallable)
        void test0();

protected:

    //virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;

    void onDataSeqReplicated(MyMJGameRoleTypeCpp eRole);
    bool tryAppendDataToHistoryBuffer();


    //UFUNCTION(BlueprintCallable)
    //void changeViewRole(MyMJGameRoleTypeCpp eRoleType)
    //{
        //MY_VERIFY(IsValid(m_pDataHistoryBuffer0));

        //m_cDataNow.reinit(eRoleType);
        //m_pDataHistoryBuffer0->reinit(eRoleType);
    //};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "cfg", meta = (DisplayName = "data source"))
    AMyMJGameCoreDataSourceCpp* m_pDataSource;

    //this is the visual data
    UPROPERTY()
    UMyMJDataSequencePerRoleCpp* m_pDataHistoryBuffer0;

    //this is the current state used for visualize
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data now"))
    FMyMJDataAtOneMomentCpp m_cDataNow;

};
*/