// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
//#include "CoreUObject.h"

//#include "UObject/Object.h"
//#include "UObject/NoExportTypes.h"

//#include "Queue.h"
#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"

#include "MyMJGameAttenderBP.h"

#include "MJLocalCS/MyMJGameCoreLocalCS.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "MyMJGameCoreBP.generated.h"

class FMyMJGameCoreThreadControlCpp : public FMyThreadControlCpp
{
public:
    FMyMJGameCoreThreadControlCpp(MyMJGameRuleTypeCpp eRuleType, int32 iSeed, int32 iTrivalConfigMask) : FMyThreadControlCpp(MY_MJ_GAME_CORE_FULL_LOOP_TIME_MS)
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
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("thread started, beginInRun()"));

        FMyMJGameCoreCpp *pCore = UMyMJBPUtilsLibrary::helperCreateCoreByRuleType(m_eRuleType, MyMJGameCoreWorkModeCpp::Full, m_iSeed.GetValue(), m_iTrivalConfigMask);

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

    TSharedPtr<FMyMJGameCoreCpp> *m_ppCoreInRun;

    int32 m_iTrivalConfigMask;
};

typedef class UMyMJCoreFullCpp UMyMJCoreFullCpp;

DECLARE_MULTICAST_DELEGATE(FMJGamePusherUpdatedMultcastDelegate);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FMJGamePusherSegmentMultcastDelegate, int32, const FMyMJGamePusherPointersCpp&);
DECLARE_MULTICAST_DELEGATE_OneParam(FMJGameCmdSegmentMultcastDelegate, const FMyMJGameCmdPointersCpp&);

UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API UMyMJPusherBufferCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJPusherBufferCpp() : Super()
    {
        m_pConnectedCoreFull = NULL;
    };

    void trySyncDataFromCoreFull();

    //holds the orignial pushers, which contains all info to restore and present the core game state
    FMyMJGamePusherPointersCpp m_cPusherBuffer;

    //if set, it means it will pull data from the core at runtime periodly, otherwise it works as save-load mode used for replay
    UPROPERTY()
    UMyMJCoreFullCpp *m_pConnectedCoreFull;

    FMJGamePusherUpdatedMultcastDelegate m_cPusherUpdatedMultcastDelegate;
};

UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API UMyMJIONodeCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJIONodeCpp() : Super()
    {
        clearUp();
    }

    virtual ~UMyMJIONodeCpp()
    {
        clearUp();
    }

    void clearUp()
    {
        m_eRoleType = MyMJGameRoleTypeCpp::Max;
    };

    void onCmdUpdated(const FMyMJGameCmdPointersCpp &cSegment)
    {
        m_cCmdUpdatedDelegate.Broadcast(cSegment);
    };

    FMJGameCmdSegmentMultcastDelegate  m_cCmdUpdatedDelegate;

    //only used when contacting with core full
    UPROPERTY()
    MyMJGameRoleTypeCpp m_eRoleType;
};


/*
* this only exist on server, in network env
*/
//notplaceable
UCLASS(BlueprintType, notplaceable, meta = (ShortTooltip = "The full game core with sub thread, should only exist on server"))
class MYONLINECARDGAME_API UMyMJCoreFullCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJCoreFullCpp() : Super() {

        m_iSeed2OverWrite = 0;
        m_pPusherBuffer = NULL;
        //m_apNextNodes.Reset();
        m_pCoreFullWithThread = NULL;
    };

    //test with a full mode core created and run in sub thread
    UFUNCTION(BlueprintCallable, Category = "UMyMJCoreFullCpp")
    void testGameCoreInSubThread(bool showCoreLog);

    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask);
    bool startGame();

    void loop();

    void clearUp();

    inline TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* getPusherQueue()
    {
        if (m_pCoreFullWithThread.IsValid()) {
            return &m_pCoreFullWithThread->getIOGourpAll().m_cPusherQueue;
        }

        return NULL;
    };

    inline TArray<UMyMJIONodeCpp*>& getIONodes()
    {
        return m_apNextNodes;
    };

    inline UMyMJPusherBufferCpp *getpPusherBuffer()
    {
        return m_pPusherBuffer;
    }

protected:
    virtual void PostInitProperties() override;

    
    //It seems timer automaticallly check object's validation, but for safe I still stop it manually here
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    //if not 0, it will be used as seed of random
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

    UPROPERTY()
    UMyMJPusherBufferCpp *m_pPusherBuffer;

    UPROPERTY()
    TArray<UMyMJIONodeCpp*> m_apNextNodes;


    TSharedPtr<FMyMJGameCoreThreadControlCpp> m_pCoreFullWithThread;
    FTimerHandle m_cLoopTimerHandle;

};

#define MY_EXPECTED_MJ_PAWN_NUM ((uint8)MyMJGameRoleTypeCpp::Max)

//This level focus on logic, it may work either connected to full mode core or NULL, since for graphic and BP usage, we didn't need the full mode core
UCLASS(Abstract)
class MYONLINECARDGAME_API AMyMJCoreMirrorCpp : public AInfo
{
    GENERATED_BODY()
public:

    AMyMJCoreMirrorCpp() : Super()
    {
        m_pPusherBuffer = NULL;
        m_iPusherApplyState = 0;
        m_iSeed2OverWrite = 0;
        m_pCoreMirror = NULL;
    }

    virtual UMyMJCoreDataForMirrorModeCpp* getpCoreData()
    {
        MY_VERIFY(0 && "You must override this");
        return NULL;
    };

    virtual TArray<AMyMJAttenderPawnBPCpp *>& getAttenderPawnsRef()
    {
        MY_VERIFY(0 && "You must override this");
        return *(TArray<AMyMJAttenderPawnBPCpp *>*)NULL;
    };

    UFUNCTION(BlueprintCallable)
    void connectToCoreFull(UMyMJCoreFullCpp *pCoreFull)
    {
        TArray<AMyMJAttenderPawnBPCpp *>& aAttenderPawns = getAttenderPawnsRef();

        MY_VERIFY(pCoreFull);
        MY_VERIFY(IsValid(pCoreFull));
        TArray<UMyMJIONodeCpp*> &apIONodes = pCoreFull->getIONodes();
        int32 l0, l1;
        l0 = apIONodes.Num();
        l1 = aAttenderPawns.Num();

        if (l0 == (uint8)MyMJGameRoleTypeCpp::Max && l1 <= l0 ) {
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("incorrect state when setup: IONodes %d, attenderPawns %d"), l0, l1);
            MY_VERIFY(false);
        }

        for (int32 i = 0; i < l1; i++) {
            aAttenderPawns[i]->setup(i, apIONodes[i]);
        }

        if (l1 > (uint8)MyMJGameRoleTypeCpp::Max) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("l1 is greater than expected, now %d, pls check!"), l1);
        }

        if (IsValid(m_pPusherBuffer)) {
            m_pPusherBuffer->m_cPusherUpdatedMultcastDelegate.RemoveAll(this);
            m_pPusherBuffer = NULL;
        }

        m_pPusherBuffer = pCoreFull->getpPusherBuffer();

        m_pPusherBuffer->m_cPusherUpdatedMultcastDelegate.AddUObject(this, &AMyMJCoreMirrorCpp::loop);

        //we have mutlicast delegate once setupped with IO Node to trigger action, but in first we may miss some, so set a timer to do loop action once
        UWorld *world = GetWorld();

        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
            world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJCoreMirrorCpp::loop, ((float)MY_MJ_GAME_CORE_MIRROR_LOOP_TIME_MS) / (float)1000, false);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        }

    };

    inline bool checkLevelSettings()
    {
        int32 l = getAttenderPawnsRef().Num();

        if (l != MY_EXPECTED_MJ_PAWN_NUM) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("mirror actor have not set attneder pawns correctly, only %d were set, expect %d, check you level!"), l, MY_EXPECTED_MJ_PAWN_NUM);
            return false;
        }

        return true;
    };

    void loop();

    virtual bool getbHaltForGraphic() const
    {
        MY_VERIFY(0 && "You must override this");
        return false;
    };

    //return true if want to pause progress
    virtual bool prePusherApplyForGraphic(FMyMJGamePusherBaseCpp *pPusher)
    {
        MY_VERIFY(0 && "You must override this");
        return false;
    };

    //return true if want to pause progresss
    virtual bool postPusherApplyForGraphic(FMyMJGamePusherBaseCpp *pPusher)
    {
        MY_VERIFY(0 && "You must override this");
        return false;
    };




    //setttings:

    //if not NULL< the instance have connected to a full core
    UPROPERTY()
    UMyMJPusherBufferCpp* m_pPusherBuffer;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

protected:

    FMyMJGamePusherBaseCpp* tryCheckAndGetNextPusher();


    int32 m_iPusherApplyState; //0 init, 1 pre called, 2 post called

    TSharedPtr<FMyMJGameCoreCpp> m_pCoreMirror;

    FTimerHandle m_cLoopTimerHandle;

};


//This level focus on BP and Graphic
UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyMJCoreBaseForBpCpp : public AMyMJCoreMirrorCpp
{
    GENERATED_BODY()

public:

    AMyMJCoreBaseForBpCpp() : Super()
    {
        m_pData = NULL;
        //m_aAttenderPawns.Reset();
        m_bHaltForGraphic = false;
    }


    //virtual void PostInitProperties() override;
    virtual void PostInitializeComponents() override
    {
        Super::PostInitializeComponents();

        MY_VERIFY(m_aAttenderPawns.Num() <= 0);
        MY_VERIFY(m_pData == NULL);

        m_pData = NewObject<UMyMJCoreDataForMirrorModeCpp>(this);
    };


    virtual UMyMJCoreDataForMirrorModeCpp* getpCoreData() override
    {
        return m_pData;
    };

    virtual TArray<AMyMJAttenderPawnBPCpp *>& getAttenderPawnsRef() override
    {
        return m_aAttenderPawns;
    };

    virtual bool getbHaltForGraphic() const override
    {
        return m_bHaltForGraphic;
    };

    //return true if want to pause progress
    virtual bool prePusherApplyForGraphic(FMyMJGamePusherBaseCpp *pPusher) override;

    //return true if want to pause progresss
    virtual bool postPusherApplyForGraphic(FMyMJGamePusherBaseCpp *pPusher) override;



    UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly)
    bool postPusherApplyResetGame(const FMyMJGamePusherResetGameCpp &pusher);

protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data"))
    UMyMJCoreDataForMirrorModeCpp *m_pData;

    //the level should prepare this data
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "attender pawns"))
    TArray<AMyMJAttenderPawnBPCpp *> m_aAttenderPawns;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "halt for graphic"))
    bool m_bHaltForGraphic;
};