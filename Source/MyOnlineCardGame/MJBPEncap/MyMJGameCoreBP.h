// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
//#include "CoreUObject.h"

//#include "UObject/Object.h"
//#include "UObject/NoExportTypes.h"

//#include "Queue.h"
#include "GameFramework/Actor.h"
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

typedef class AMyMJCoreFullCpp AMyMJCoreFullCpp;

DECLARE_MULTICAST_DELEGATE_OneParam(FMJGameCmdMultcastDelegate, TSharedPtr<FMyMJGameCmdBaseCpp>);

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
        m_pPrevCoreFull = NULL;
        m_pPrevIONode = NULL;
        m_apNextNodes.Reset();
        m_eRoleType = MyMJGameRoleTypeCpp::Max;
        m_cPusherBuffer.clear();
    };

    inline void setPrev(AMyMJCoreFullCpp* pPrev, MyMJGameRoleTypeCpp eRoleType)
    {
        MY_VERIFY(m_pPrevIONode == NULL);
        MY_VERIFY(eRoleType != MyMJGameRoleTypeCpp::Max);
        m_pPrevCoreFull = pPrev;
        m_eRoleType = eRoleType;
    };

    inline void setPrev(UMyMJIONodeCpp* pPrevIONode)
    {
        MY_VERIFY(m_pPrevCoreFull == NULL);
        m_pPrevIONode = pPrevIONode;
    };

    void pullPushersAndCmdRespFromPrevCoreVerified();

    void pullPushersFromPrevNodeVerified();

    void pulledcmdRespFromPrevNode(TSharedPtr<FMyMJGameCmdBaseCpp> pCmdShared);

    FMJGameCmdMultcastDelegate m_cCmdRespDelegate;

    //following is the buffer, need to replic
    FMyMJGamePusherPointersCpp m_cPusherBuffer;

    //If set, IO/data with it as previous
    UPROPERTY()
    AMyMJCoreFullCpp* m_pPrevCoreFull;

    //only used when contacting with core full
    UPROPERTY()
    MyMJGameRoleTypeCpp m_eRoleType;

    //otherwise
    UPROPERTY()
    UMyMJIONodeCpp* m_pPrevIONode;

    UPROPERTY()
    TArray<UMyMJIONodeCpp*> m_apNextNodes;
};


/*
* this only exist on server, in network env
*/
//notplaceable
UCLASS(BlueprintType, Blueprintable, notplaceable, meta = (ShortTooltip = "The full game core with sub thread, should only exist on server"))
class MYONLINECARDGAME_API AMyMJCoreFullCpp : public AActor
{
    GENERATED_BODY()

public:

    //test with a full mode core created and run in sub thread, and a mirror mode core in game core 
    //UFUNCTION(BlueprintCallable, Category = "AMyMJCoreFullCpp")
    void testFullMode(bool bOverWriteSeed, int32 iSeed2OverWrite);

    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType, bool bOverWriteSeed, int32 iSeed2OverWrite);
    bool startGame();

    void loop();

    void clearUp();

    inline FMyMJGameIOGroupAllCpp* getpIOGourpAll()
    {
        if (m_pCoreFullWithThread.IsValid()) {
            return &m_pCoreFullWithThread->getIOGourpAll();
        }

        return NULL;
    };

protected:
    virtual void PostInitProperties() override;

    //It seems timer automaticallly check object's validation, but for safe I still stop it manually here
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY()
    TArray<UMyMJIONodeCpp*> m_apNextNodes;

    TSharedPtr<FMyMJGameCOreThreadControlCpp> m_pCoreFullWithThread;
    FTimerHandle m_cLoopTimerHandle;
};


UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API UMyMJCoreMirrorCpp : public UObject
{
    GENERATED_BODY()

public:

    void tryProgressInMirrorMode();

    UPROPERTY()
    UMyMJIONodeCpp* m_pPrevIONode;

    TSharedPtr<FMyMJGameCoreCpp> m_pCoreMirror;
};

