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

#include "MyMJGameCoreBP.generated.h"

class FMyMJGameCOreThreadControlCpp : public FMyThreadControlCpp
{
public:
    FMyMJGameCOreThreadControlCpp(MyMJGameRuleTypeCpp eRuleType, int32 iSeed) : FMyThreadControlCpp(MY_MJ_GAME_CORE_LOOP_TIME_MS)
    {
        m_eRuleType = eRuleType;
        m_iSeed.Set(iSeed);

        m_ppCoreInRun = NULL;

        MY_VERIFY(start());
    };

    virtual ~FMyMJGameCOreThreadControlCpp()
    {
        //clean up ahead, otherwise it may crash since subthread may still using vtable which is cleaned here
        cleanUp();
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~FMyMJGameCoreThreadControlCpp()"));

    };

    //create one instance on heap
    static FMyMJGameCoreCpp* helperCreateCoreByRuleType(MyMJGameRuleTypeCpp eRuleType, MyMJGameCoreWorkModeCpp eWorkMode, int32 iSeed)
    {
        if (eRuleType == MyMJGameRuleTypeCpp::LocalCS) {
            return StaticCast<FMyMJGameCoreCpp *>(new FMyMJGameCoreLocalCSCpp(eWorkMode, iSeed));
        }
        else if (eRuleType == MyMJGameRuleTypeCpp::GuoBiao) {
            MY_VERIFY(false);
        }
        else {
            MY_VERIFY(false);
        }

        return NULL;
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
        FMyMJGameCoreCpp *pCore = helperCreateCoreByRuleType(m_eRuleType, MyMJGameCoreWorkModeCpp::Full, m_iSeed.GetValue());

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

    UMyMJPusherBufferCpp()
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

    UMyMJIONodeCpp()
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
UCLASS(BlueprintType, Blueprintable, notplaceable, meta = (ShortTooltip = "The full game core with sub thread, should only exist on server"))
class MYONLINECARDGAME_API UMyMJCoreFullCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJCoreFullCpp() {
        m_iSeed2OverWrite = 0;
    };

    //test with a full mode core created and run in sub thread, and a mirror mode core in game core 
    //UFUNCTION(BlueprintCallable, Category = "UMyMJCoreFullCpp")
    void testFullMode();

    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType);
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


    TSharedPtr<FMyMJGameCOreThreadControlCpp> m_pCoreFullWithThread;
    FTimerHandle m_cLoopTimerHandle;

};

#define MY_EXPECTED_MJ_PAWN_NUM ((uint8)MyMJGameRoleTypeCpp::Max)

UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyMJCoreMirrorCpp : public AInfo
{
    GENERATED_BODY()

public:

    AMyMJCoreMirrorCpp() : Super()
    {
        m_bHaltForGraphic = false;
        m_iSeed2OverWrite = 0;
    }


    void change2PlayMode(UMyMJCoreFullCpp *pCoreFull)
    {
        bool bRet = resetSettings();
        MY_VERIFY(bRet);

        TArray<UMyMJIONodeCpp*> &apIONodes = pCoreFull->getIONodes();
        int32 l0, l1;
        l0 = apIONodes.Num();
        l1 = m_aAttenderPawns.Num();

        if (l0 == (uint8)MyMJGameRoleTypeCpp::Max && l1 <= l0 ) {
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("incorrect state when setup: IONodes %d, attenderPawns %d"), l0, l1);
            MY_VERIFY(false);
        }

        for (int32 i = 0; i < l1; i++) {
            m_aAttenderPawns[i]->setup(i, apIONodes[i]);
        }

        if (l1 >= (uint8)MyMJGameRoleTypeCpp::SysKeeper) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("l1 is greater than expected, %d, pls check!"), l1);
        }

        m_pPusherBuffer = pCoreFull->getpPusherBuffer();

    };

    void change2RePlayMode();

    bool resetSettings()
    {
        bool bRet = checkLevelSettings();

        m_pPusherBuffer = NULL;

        int32 l = m_aAttenderPawns.Num();
        for (int32 i = 0; i < l; i++) {
            m_aAttenderPawns[i]->resetSettings();
        }

        return bRet;
    };

    inline bool checkLevelSettings()
    {
        int32 l = m_aAttenderPawns.Num();

        if (l != MY_EXPECTED_MJ_PAWN_NUM) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("mirror actor have not set attneder pawns correctly, only %d were set, expect %d, check you level!"), l, MY_EXPECTED_MJ_PAWN_NUM);
            return false;
        }

        return true;
    };

    void onPusherUpdated(int32 iGameIdSegment, const FMyMJGamePusherPointersCpp &cSegment)
    {
        //
        loop();
    };


    void loop();

    void notifyBluePrintPusherApplied(FMyMJGamePusherBaseCpp *pPusher);

    virtual void PostInitProperties() override;

    //Returns whether halt progress for graphic, and m_bHaltForGraphic will be set then
    //UFUNCTION(BlueprintImplementableEvent)
    //bool onVisiblePusherApplied(FMyMJGamePusherBaseCpp *pPusher);

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "core data direct"))
    //FMyMJCoreDataDirectForBPCpp m_cCoreDataDirect;

    //setttings:
    UPROPERTY()
    UMyMJPusherBufferCpp* m_pPusherBuffer;

    //the level should prepare this data
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "attender pawns"))
    TArray<AMyMJAttenderPawnBPCpp *> m_aAttenderPawns;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

protected:

    FMyMJGamePusherBaseCpp* tryCheckAndGetNextPusher();

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "halt for graphic"))
    bool m_bHaltForGraphic;

    TSharedPtr<FMyMJGameCoreCpp> m_pCoreMirror;
};

