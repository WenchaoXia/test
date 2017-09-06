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
UCLASS(notplaceable, meta = (ShortTooltip = "The full game core with sub thread, should only exist on server"))
class MYONLINECARDGAME_API UMyMJCoreFullCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJCoreFullCpp() : Super() {

        m_iSeed2OverWrite = 0;
        //m_pPusherBuffer = NULL;
        //m_apNextNodes.Reset();
        m_pCoreFullWithThread = NULL;
    };

    //test with a full mode core created and run in sub thread
    UFUNCTION(BlueprintCallable, Category = "UMyMJCoreFullCpp")
    void testGameCoreInSubThread(bool showCoreLog, bool bAttenderRandomSelectHighPriActionFirst);

    bool tryChangeMode(MyMJGameRuleTypeCpp eRuleType, int32 iTrivalConfigMask);
    bool startGame(bool bAttenderRandomSelectDo, bool bAttenderRandomSelectHighPriActionFirst);

    void loop();

    void clearUp();

    inline TQueue<FMyMJGamePusherResultCpp *, EQueueMode::Spsc>* getPusherResultQueue()
    {
        if (m_pCoreFullWithThread.IsValid()) {
            return &m_pCoreFullWithThread->getIOGourpAll().m_cPusherResultQueue;
        }

        return NULL;
    };


protected:
    virtual void PostInitProperties() override;

    
    //It seems timer automaticallly check object's validation, but for safe I still stop it manually here
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    //if not 0, it will be used as seed of random
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

    TSharedPtr<FMyMJGameCoreThreadControlCpp> m_pCoreFullWithThread;
    FTimerHandle m_cLoopTimerHandle;

};

//struct FMyMJGameEventArray : public FFastArraySerializer
//USTRUCT()
//struct FMyMJGameEventArray : public FFastArraySerializerItem
//{
    //GENERATED_USTRUCT_BODY()
//};


#define MY_EXPECTED_MJ_PAWN_NUM ((uint8)MyMJGameRoleTypeCpp::Max)

//This level focus on logic, it may work either connected to full mode core or NULL, since for graphic and BP usage, we didn't need the full mode core
UCLASS(Abstract)
class MYONLINECARDGAME_API AMyMJCoreMirrorCpp : public AInfo
{
    GENERATED_BODY()
public:

    AMyMJCoreMirrorCpp() : Super()
    {
        m_pCoreFull = NULL;
        m_iPusherApplyState = 0;
        m_iSeed2OverWrite = 0;

    }

    //virtual void PostInitProperties() override;
    virtual void PostInitializeComponents() override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;


    UFUNCTION(BlueprintCallable, Category = "AMyMJCoreMirrorCpp")
        void doTestChange()
    {
        if (m_pMJDataAll) {
            m_pMJDataAll->doTestChange();
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMJDataStorage is NULL!"));
        }

    };

    UFUNCTION(BlueprintCallable, Category = "AMyMJCoreMirrorCpp")
        FString genDebugMsg()
    {
        FString ret;
        if (m_pMJDataAll) {
            return m_pMJDataAll->genDebugMsg();
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMJDataStorage is NULL!"));
            return ret;
        }

    };

    UFUNCTION(BlueprintCallable)
    void connectToCoreFull(UMyMJCoreFullCpp *pCoreFull)
    {
        MY_VERIFY(pCoreFull);
        MY_VERIFY(IsValid(pCoreFull));

        MY_VERIFY(checkLevelSetting());

        m_pCoreFull = pCoreFull;

        //we have mutlicast delegate once setupped with IO Node to trigger action, but in first we may miss some, so set a timer to do loop action once
        UWorld *world = GetWorld();

        if (IsValid(world)) {
            world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
            world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJCoreMirrorCpp::toCoreFullLoop, ((float)MY_MJ_GAME_CORE_MIRROR_LOOP_TIME_MS) / (float)1000, true);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
            MY_VERIFY(false);
        }

    };

    void toCoreFullLoop();

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


    bool checkLevelSetting()
    {
        return m_aAttenderPawns.Num() == 4;
    };

    //the level should prepare this data
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "attender pawns"))
        TArray<AMyMJAttenderPawnBPCpp *> m_aAttenderPawns;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

protected:

    UFUNCTION()
    void OnRep_MJDataAll();

    FMyMJGamePusherBaseCpp* tryCheckAndGetNextPusher();

    //UPROPERTY(BlueprintReadOnly, Replicated, meta = (DisplayName = "data of storage"))
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MJDataAll, meta = (DisplayName = "data of all roles"))
        UMyMJDataAllCpp* m_pMJDataAll;

    UPROPERTY()
    UMyMJCoreFullCpp *m_pCoreFull;

    int32 m_iPusherApplyState; //0 init, 1 pre called, 2 post called

    FTimerHandle m_cToCoreFullLoopTimerHandle;

};


//This level focus on BP and Graphic
UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyMJCoreBaseForBpCpp : public AMyMJCoreMirrorCpp
{
    GENERATED_BODY()

public:

    AMyMJCoreBaseForBpCpp() : Super()
    {
        //m_aAttenderPawns.Reset();
        m_bHaltForGraphic = false;
    }



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


    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "halt for graphic"))
    bool m_bHaltForGraphic;
};