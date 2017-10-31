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


//#define MY_EXPECTED_MJ_PAWN_NUM ((uint8)MyMJGameRoleTypeCpp::Max)

//This level focus on logic, the data source, exist both on server and client, but only server will handle data and generate output
UCLASS()
class MYONLINECARDGAME_API AMyMJGameCoreDataSourceCpp : public AActor
{
    GENERATED_BODY()
public:

    AMyMJGameCoreDataSourceCpp() : Super()
    {
        m_iTest0 = 0;
        m_iSeed2OverWrite = 0;
        m_pCoreFullWithThread = NULL;
        m_pMJDataAll = NULL;

        bReplicates = true;
        bAlwaysRelevant = true;
        bNetLoadOnClient = true;
        NetUpdateFrequency = 10;

        m_uiLastReplicateClientTimeMs = 0;
    };

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

    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask);
    
    bool startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst);

    void clearUp();

    void coreDataPullLoop();

    //end
   
    inline UMyMJDataAllCpp* getMJDataAll()
    {
        return m_pMJDataAll;
    };
    
    inline const UMyMJDataAllCpp* getMJDataAllConst()
    {
        return m_pMJDataAll;
    };


    FMyMJDataSeqReplicatedDelegate m_cReplicateFilteredDelegate;

    UPROPERTY(Replicated)
        int32 m_iTest0;

protected:

    //virtual void PostInitProperties() override;
    //virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    //virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    bool getCoreFullPartEnabled() const;
    void setCoreFullPartEnabled(bool bEnabled);

    UFUNCTION()
    void OnRep_MJDataAll();

    void OnRep_MJDataAllContent(MyMJGameRoleTypeCpp eRole);

    //if not 0, it will be used as seed of random
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

    TSharedPtr<FMyMJGameCoreThreadControlCpp> m_pCoreFullWithThread;

    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "data of storage"))
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MJDataAll, meta = (DisplayName = "data of all roles"))
    UMyMJDataAllCpp* m_pMJDataAll;

    FTimerHandle m_cToCoreFullLoopTimerHandle;

    uint32 m_uiLastReplicateClientTimeMs;
};

UCLASS()
class MYONLINECARDGAME_API UMyTestObject : public UObject
{
    GENERATED_BODY()

public:
    UMyTestObject() : Super()
    {

    };
};

UENUM()
enum class MyMJCoreBaseForBpVisualModeCpp : uint8
{
    Normal = 1                              UMETA(DisplayName = "Normal"),
    CatchUp = 2                             UMETA(DisplayName = "CatchUp"),
};

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

    UFUNCTION(BlueprintCallable)
        void test0();

protected:

    //virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;

    //bool getGameCoreDataSourceEnabled() const;
    //void setGameCoreDataSourceEnabled(bool bEnabled);

    void onDataSeqReplicated(MyMJGameRoleTypeCpp eRole);
    bool tryAppendDataToHistoryBuffer();


    //UFUNCTION(BlueprintImplementableEvent)
    void onEventAppliedWithDur(const FMyMJEventWithTimeStampBaseCpp& newEvent) {};

    UFUNCTION(BlueprintCallable)
    void changeViewRole(MyMJGameRoleTypeCpp eRoleType)
    {
        MY_VERIFY(IsValid(m_pDataHistoryBuffer0));

        m_cDataNow.reinit(eRoleType);
        m_pDataHistoryBuffer0->reinit(eRoleType);
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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "data source"))
    AMyMJGameCoreDataSourceCpp* m_pDataSource;

    //UPROPERTY()
    //UMyTestObject* m_pTestObj;

    //this is the visual data
    UPROPERTY()
    UMyMJDataSequencePerRoleCpp* m_pDataHistoryBuffer0;

    //only for test
    //UPROPERTY()
    //UMyMJDataSequencePerRoleCpp* m_pDataHistoryBuffer2;

    //this is the current state used for visualize
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data now"))
    FMyMJDataAtOneMomentCpp m_cDataNow;

    MyMJCoreBaseForBpVisualModeCpp m_eVisualMode;
};