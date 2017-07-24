// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCoreBP.h"

#include "TimerManager.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"
//#include "Kismet/KismetNodeHelperLibrary.h"


//#include "CoreMinimal.h"
//#include "Stats/Stats.h"
//#include "UObject/ObjectMacros.h"
//#include "UObject/UObjectBaseUtility.h"
//#include "UObject/Object.h"
//#include "InputCoreTypes.h"
//#include "Templates/SubclassOf.h"
//#include "UObject/CoreNet.h"
//#include "Engine/EngineTypes.h"
//#include "Engine/EngineBaseTypes.h"
//#include "ComponentInstanceDataCache.h"
//#include "Components/ChildActorComponent.h"
//#include "RenderCommandFence.h"
//#include "Misc/ITransaction.h"
//#include "Engine/Level.h"

void UMyMJPusherBufferCpp::trySyncDataFromCoreFull()
{
    MY_VERIFY(m_pConnectedCoreFull);
    if (!IsValid(m_pConnectedCoreFull)) {
        return;
    }
    
    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>* pPusherQueue = m_pConnectedCoreFull->getPusherQueue();
    MY_VERIFY(pPusherQueue);

    FMyMJGamePusherPointersCpp cPusherSegment;
    if (cPusherSegment.helperFillAsSegmentFromQueue(*pPusherQueue)) {
        m_cPusherBuffer.helperTryFillDataFromSegment(-1, cPusherSegment, true);
        m_cPusherUpdatedMultcastDelegate.Broadcast();
    }

};

void UMyMJCoreFullCpp::testFullMode()
{

    while (!tryChangeMode(MyMJGameRuleTypeCpp::LocalCS)) {
        FPlatformProcess::Sleep(0.1);
    }

    startGame();
}


bool UMyMJCoreFullCpp::tryChangeMode(MyMJGameRuleTypeCpp eRuleType)
{

    if (m_pCoreFullWithThread.IsValid()) {
        if (m_pCoreFullWithThread->getRuleType() == eRuleType) {
            return true;
        }

        if (!m_pCoreFullWithThread->isInStartState()) {
            m_pCoreFullWithThread.Reset();
            UWorld *world = GetWorld();
            if (IsValid(world)) {
                world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid!"));
            }
        }
        else {
            m_pCoreFullWithThread->Stop();
            return false;
        }
    }

    //recheck
    if (m_pCoreFullWithThread.IsValid()) {
        return false;
    }
    else {
        if (eRuleType == MyMJGameRuleTypeCpp::Invalid) {
            return true;
        }

        //want to create one new
        int32 iSeed;
        if (m_iSeed2OverWrite != 0) {
            iSeed = m_iSeed2OverWrite;
        }
        else {
            iSeed = UMyMJUtilsLibrary::nowAsMsFromTick();
        }
        //iSeed = 530820412;

        FMyMJGameCOreThreadControlCpp *pCoreFull = new FMyMJGameCOreThreadControlCpp(eRuleType, iSeed);
        m_pCoreFullWithThread = MakeShareable<FMyMJGameCOreThreadControlCpp>(pCoreFull);

        return true;
    }


}

bool UMyMJCoreFullCpp::startGame()
{
    if (!m_pCoreFullWithThread.IsValid() || m_pCoreFullWithThread->getRuleType() == MyMJGameRuleTypeCpp::Invalid) {
        return false;
    }
    FMyMJGameCmdRestartGameCpp *pCmdReset = new FMyMJGameCmdRestartGameCpp();
    pCmdReset->m_iAttenderRandomSelectMask = 0x0f;
    UMyMJUtilsLocalCSLibrary::genDefaultCfg(pCmdReset->m_cGameCfg);
    m_pCoreFullWithThread->getIOGourpAll().m_aGroups[(uint8)MyMJGameRoleTypeCpp::SysKeeper].getCmdInputQueue().Enqueue(pCmdReset);

    m_pCoreFullWithThread->kick();

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("startGame() called"));

    UWorld *world = GetWorld();
    
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &UMyMJCoreFullCpp::loop, ((float)My_MJ_GAME_IO_LOOP_TIME_MS) / (float)1000, true);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid!"));
        return false;
    }

    return true;
}

void UMyMJCoreFullCpp::loop()
{
    if (!m_pCoreFullWithThread.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreFullWithThread invalid!"));
        return;
    }

    MY_VERIFY(m_pPusherBuffer);
    if (IsValid(m_pPusherBuffer)) {
        m_pPusherBuffer->trySyncDataFromCoreFull();
    }

    int32 l = m_apNextNodes.Num();
    if (l != (uint8)MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough nodes next were set, only %d got!"), l);
        return;
    }

    FMyMJGameCmdPointersCpp cCmdSegment;
    for (int32 i = 0; i < l; i++) {
        FMyMJGameIOGroupCpp *pGroup = &m_pCoreFullWithThread->getIOGourpAll().m_aGroups[i];
        UMyMJIONodeCpp *pNode = m_apNextNodes[i];
        MY_VERIFY(pNode->m_eRoleType == (MyMJGameRoleTypeCpp)i);
        //pNode->pushPushers(pGroup);
        if (IsValid(pNode)) {

            bool bHaveNew = cCmdSegment.helperFillAsSegmentFromIOGroup(pGroup);
            if (bHaveNew) {
                pNode->onCmdUpdated(cCmdSegment);
            }
        }
    }

}

void UMyMJCoreFullCpp::clearUp()
{
    m_pPusherBuffer = NULL;
    m_apNextNodes.Reset();
    if (m_pCoreFullWithThread.IsValid()) {
        m_pCoreFullWithThread->Stop();
    }

    UWorld *world = GetWorld();

    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }

}

void UMyMJCoreFullCpp::PostInitProperties()
{
    Super::PostInitProperties();

    m_pPusherBuffer = NewObject<UMyMJPusherBufferCpp>(this);
    m_pPusherBuffer->m_pConnectedCoreFull = this;

    m_apNextNodes.Reset();

    for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {

        UMyMJIONodeCpp *newNode = NewObject<UMyMJIONodeCpp>(this);
        m_apNextNodes.Emplace(newNode);
        newNode->m_eRoleType = (MyMJGameRoleTypeCpp)i;
    }
}



FMyMJGamePusherBaseCpp* AMyMJCoreMirrorCpp::tryCheckAndGetNextPusher()
{
    int32 iGameId = -1, iPusherIdLast = -1;

    if (IsValid(m_pPusherBuffer)) {

        if (!m_bHaltForGraphic) {
            if (m_pCoreMirror.IsValid()) {
                m_pCoreMirror->getGameIdAndPusherIdLast(&iGameId, &iPusherIdLast);
            }
            return m_pPusherBuffer->m_cPusherBuffer.helperTryPullPusher(iGameId, iPusherIdLast + 1).Get();
        }
        else {
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pIONodeAsSys invalid!"));
    }

    return NULL;
}

void AMyMJCoreMirrorCpp::loop()
{

    FMyMJGamePusherBaseCpp *pPusher = tryCheckAndGetNextPusher();

    int32 iAttenderMask;
    TArray<FMyIdValuePair> aRevealedCardValues;
    while (pPusher) {
        MyMJGamePusherTypeCpp ePusherType = pPusher->getType();

        //1st, make sure mirror core is correctly set
        if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame) {
            FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
            MyMJGameRuleTypeCpp eDestType = pPusherResetGame->m_cGameCfg.m_eRuleType;
            MY_VERIFY(eDestType != MyMJGameRuleTypeCpp::Invalid);
            MyMJGameRuleTypeCpp eNowType = MyMJGameRuleTypeCpp::Invalid;
            if (m_pCoreMirror.IsValid()) {
                eNowType = m_pCoreMirror->getRuleType();
            }

            if (eNowType != eDestType) {
                int32 iSeed;
                if (m_iSeed2OverWrite != 0) {
                    iSeed = m_iSeed2OverWrite;
                }
                else {
                    iSeed = UMyMJUtilsLibrary::nowAsMsFromTick();
                }

                FMyMJGameCoreCpp *pCore = FMyMJGameCOreThreadControlCpp::helperCreateCoreByRuleType(eDestType, MyMJGameCoreWorkModeCpp::Mirror, iSeed);
                m_pCoreMirror = MakeShareable<FMyMJGameCoreCpp>(pCore);
            }
        }

        if (m_pCoreMirror.IsValid()) {

            //2nd, apply to core in data level
            m_pCoreMirror->makeProgressByPusher(pPusher);
            
            //3rd, set data for graphic
            //3.1 reveal value
            if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame) {
                FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
                int32 l = m_aAttenderPawns.Num();
                for (int32 i = 0; i < l; i++) {
                    m_aAttenderPawns[i]->m_cAttenderDataDirectPrivate.reset(pPusherResetGame->m_aShuffledIdValues.Num());
                }
            }
          
            pPusher->getRevealedCardValues(iAttenderMask, aRevealedCardValues);
            if (iAttenderMask != 0) {
                int32 l = m_aAttenderPawns.Num();
                if (l < (uint8)MyMJGameRoleTypeCpp::Max) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough pawn set! only %d."), l);
                }

                for (int32 i = 0; i < l; i++) {
                    if ((iAttenderMask & (1 << i)) == 0) {
                        continue;
                    }
                    //need update
                    m_aAttenderPawns[i]->m_cAttenderDataDirectPrivate.m_cCardPackForValue.revealCardValueByIdValuePairs(aRevealedCardValues);
                }

            }

            //3.2 set other 


            //4th, try notify for graphic, and this function may update blue print state before call blueprint
            notifyBluePrintPusherApplied(pPusher);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreMirror invalid!"));
            MY_VERIFY(false);
        }


        pPusher = tryCheckAndGetNextPusher();
    }
}

void AMyMJCoreMirrorCpp::notifyBluePrintPusherApplied(FMyMJGamePusherBaseCpp *pPusher)
{
    //let's filter out important pusher and notify blueprint
    MyMJGamePusherTypeCpp ePusherType = pPusher->getType();
    bool bHaltForGraphic = false;


    if (ePusherType == MyMJGamePusherTypeCpp::PusherFillInActionChoices) {
        FMyMJGamePusherFillInActionChoicesCpp *pPusherFillInActionChoices = StaticCast<FMyMJGamePusherFillInActionChoicesCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherMadeChoiceNotify) {
        FMyMJGamePusherMadeChoiceNotifyCpp *pPusherMadeChoiceNotify = StaticCast<FMyMJGamePusherMadeChoiceNotifyCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherResetGame) {

        //reflect data
        m_cCoreDataDirect.m_iCardNumCanBeTakenNormally = m_pCoreMirror->getUntakenSlotInfoRef().getCardNumCanBeTakenNormally();
        m_cCoreDataDirect.m_aUntakenCardStacks = m_pCoreMirror->getUntakenCardStacksRef();
        m_cCoreDataDirect.m_cGameCfg = *m_pCoreMirror->getpGameCfg();
        FMyMJGamePusherResetGameCpp *pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::PusherUpdateTing) {
        FMyMJGamePusherUpdateTingCpp *pPusherUpdateTing = StaticCast<FMyMJGamePusherUpdateTingCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionNoAct) {
        FMyMJGameActionNoActCpp *pActionNoAct = StaticCast<FMyMJGameActionNoActCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {
        FMyMJGameActionThrowDicesCpp *pActionThrowDices = StaticCast<FMyMJGameActionThrowDicesCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
        FMyMJGameActionDistCardAtStartCpp *pActionDistCardAtStart = StaticCast<FMyMJGameActionDistCardAtStartCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
        FMyMJGameActionTakeCardsCpp *pActionTakeCards = StaticCast<FMyMJGameActionTakeCardsCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
        FMyMJGameActionGiveOutCardsCpp *pActionGiveOutCards = StaticCast<FMyMJGameActionGiveOutCardsCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
        FMyMJGameActionWeaveCpp *pActionWeave = StaticCast<FMyMJGameActionWeaveCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {
        FMyMJGameActionHuCpp *pActionHu = StaticCast<FMyMJGameActionHuCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {
        FMyMJGameActionHuBornLocalCSCpp *pActionHuBornLocalCS = StaticCast<FMyMJGameActionHuBornLocalCSCpp *>(pPusher);
    }
    else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {
        FMyMJGameActionZhaNiaoLocalCSCpp *pActionZhaNiaoLocalCS = StaticCast<FMyMJGameActionZhaNiaoLocalCSCpp *>(pPusher);
    }
}

void AMyMJCoreMirrorCpp::PostInitProperties()
{
    Super::PostInitProperties();

    MY_VERIFY(m_aAttenderPawns.Num() <= 0);

    //pAttenderData->m_e
}