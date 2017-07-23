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

DECLARE_MULTICAST_DELEGATE_TwoParams(FMJGamePusherSegmentMultcastDelegate, int32, const FMyMJGamePusherPointersCpp&);
DECLARE_MULTICAST_DELEGATE_OneParam(FMJGameCmdSegmentMultcastDelegate, const FMyMJGameCmdPointersCpp&);

UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API UMyMJIONodeCpp : public UObject
{
    GENERATED_BODY()

public:

    /*
    UMyMJIONodeCpp::UMyMJIONodeCpp(const class FObjectInitializer & PCIP) : Super(PCIP)
    {
        bReplicates = true;
    }
    */

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
        m_cPusherBuffer.clear();
    };

    void pullPushersAndCmdRespFromPrevCoreVerified(FMyMJGameIOGroupCpp *pGroup);

    void pullPushersFromPrevNodeVerified();

    void onPusherUpdated(int32 iGameIdSegment, const FMyMJGamePusherPointersCpp &cSegment)
    {
        m_cPusherUpdatedDelegate.Broadcast(iGameIdSegment, cSegment);

    };

    void onCmdUpdated(const FMyMJGameCmdPointersCpp &cSegment)
    {
        m_cCmdUpdatedDelegate.Broadcast(cSegment);
    };


    FMJGamePusherSegmentMultcastDelegate  m_cPusherUpdatedDelegate;
    FMJGameCmdSegmentMultcastDelegate     m_cCmdUpdatedDelegate;

    //following is the buffer, need to replic
    FMyMJGamePusherPointersCpp m_cPusherBuffer;

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

    /*
    inline FMyMJGameIOGroupAllCpp* getpIOGourpAll()
    {
        if (m_pCoreFullWithThread.IsValid()) {
            return &m_pCoreFullWithThread->getIOGourpAll();
        }

        return NULL;
    };
    */

    inline TArray<UMyMJIONodeCpp*>& getIONodes()
    {
        return m_apNextNodes;
    };

protected:
    virtual void PostInitProperties() override;

    
    //It seems timer automaticallly check object's validation, but for safe I still stop it manually here
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    //if not 0, it will be used as seed of random
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "seed overwrite"))
    int32 m_iSeed2OverWrite;

    UPROPERTY()
    TArray<UMyMJIONodeCpp*> m_apNextNodes;

    TSharedPtr<FMyMJGameCOreThreadControlCpp> m_pCoreFullWithThread;
    FTimerHandle m_cLoopTimerHandle;
};


USTRUCT(BlueprintType)
struct FMyMJCoreDataDirectForBPCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreDataDirectForBPCpp()
    {
        m_iCardNumCanBeTakenAll = 0;
        m_iActionGroupId = 0;
        m_eGameState = MyMJGameStateCpp::Invalid;
        m_eActionLoopState = MyMJActionLoopStateCpp::Invalid;
        m_iDiceNumberNow0 = m_iDiceNumberNow1 = 0;
        m_iIdHelperLastCardTakenInGame = -1;
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
    };

    virtual ~FMyMJCoreDataDirectForBPCpp()
    {

    };

    void reset()
    {
        m_iCardNumCanBeTakenAll = 0;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Card Num Left"))
        int32 m_iCardNumCanBeTakenAll;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UnTaken Card Stacks"))
        TArray<FMyIdCollectionCpp> m_aUntakenCardStacks; //Always start from attender 0 to 3

                                                         //Cfg
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Cfg"))
        FMyMJGameCfgCpp m_cGameCfg;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game RunData"))
        FMyMJGameRunDataCpp m_cGameRunData;

    //int32 m_iGameId;
    //int32 m_iPusherId; //the last pusher id we got
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Group Id"))
        int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
        MyMJGameStateCpp m_eGameState;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Loop State"))
        MyMJActionLoopStateCpp m_eActionLoopState;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number 0"))
        int32 m_iDiceNumberNow0;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number 1"))
        int32 m_iDiceNumberNow1;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
        TArray<int32> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hai Di Card Id"))
        int32 m_iIdHelperLastCardTakenInGame;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Rule Type"))
        MyMJGameRuleTypeCpp m_eRuleType;//also distinguish sub type
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

        m_pIONodeAsSys = apIONodes[(uint8)MyMJGameRoleTypeCpp::SysKeeper];
        m_pIONodeAsSys->m_cPusherUpdatedDelegate.Clear();
        m_pIONodeAsSys->m_cPusherUpdatedDelegate.AddUObject(this, &AMyMJCoreMirrorCpp::onPusherUpdated);
    };

    void change2RePlayMode();

    bool resetSettings()
    {
        bool bRet = checkLevelSettings();

        m_pIONodeAsSys = NULL;

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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "core data direct"))
    FMyMJCoreDataDirectForBPCpp m_cCoreDataDirect;

    //setttings:
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "IO Node As Sys"))
    UMyMJIONodeCpp* m_pIONodeAsSys;

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

