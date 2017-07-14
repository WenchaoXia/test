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

void AMyMJCoreFullCpp::testFullMode(bool bOverWriteSeed, int32 iSeed2OverWrite)
{
    int32 *pSeed = bOverWriteSeed ? &iSeed2OverWrite : NULL;
    while (!tryChangeMode(MyMJGameRuleTypeCpp::LocalCS, bOverWriteSeed, iSeed2OverWrite)) {
        FPlatformProcess::Sleep(0.1);
    }

    startGame();
}


bool AMyMJCoreFullCpp::tryChangeMode(MyMJGameRuleTypeCpp eRuleType, bool bOverWriteSeed, int32 iSeed2OverWrite)
{

    if (m_pCoreFullWithThread.IsValid()) {
        if (m_pCoreFullWithThread->getRuleType() == eRuleType) {
            return true;
        }

        if (!m_pCoreFullWithThread->isInStartState()) {
            m_pCoreFullWithThread.Reset();
            GetWorldTimerManager().ClearTimer(m_cLoopTimerHandle);
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
        if (bOverWriteSeed) {
            iSeed = iSeed2OverWrite;
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

bool AMyMJCoreFullCpp::startGame()
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

    AActor::GetWorldTimerManager().ClearTimer(m_cLoopTimerHandle);
    AActor::GetWorldTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJCoreFullCpp::loop, ((float)My_MJ_GAME_IO_LOOP_TIME_MS) / 1000, true);

    return true;
}

void AMyMJCoreFullCpp::loop()
{
    if (!m_pCoreFullWithThread.IsValid()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreFullWithThread invalid!"));
        return;
    }

    int32 l = m_apNextNodes.Num();
    if (l != (uint8)MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough nodes next were set, only %d got!"), l);
        return;
    }

    for (int32 i = 0; i < l; i++) {
        //FMyMJGameIOGroupCpp *pGroup = &m_pCoreFullWithThread->getIOGourpAll().m_aGroups[i];
        UMyMJIONodeCpp *pNode = m_apNextNodes[i];
        //pNode->pushPushers(pGroup);
        if (IsValid(pNode)) {
            pNode->pullPushersAndCmdRespFromPrevCoreVerified();
        }
    }

}

void AMyMJCoreFullCpp::clearUp()
{
    m_apNextNodes.Reset();
    if (m_pCoreFullWithThread.IsValid()) {
        m_pCoreFullWithThread->Stop();
    }
    GetWorldTimerManager().ClearTimer(m_cLoopTimerHandle);
}

void AMyMJCoreFullCpp::PostInitProperties()
{
    Super::PostInitProperties();

    m_apNextNodes.Reset();

    for (int32 i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {

        UMyMJIONodeCpp *newNode = NewObject<UMyMJIONodeCpp>(this);
        newNode->setPrev(this, (MyMJGameRoleTypeCpp)i);
        m_apNextNodes.Emplace(newNode);
    }
}

//It seems timer automaticallly check object's validation, but for safe I still stop it manually here
void AMyMJCoreFullCpp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    clearUp();
}


void UMyMJIONodeCpp::pullPushersAndCmdRespFromPrevCoreVerified()
{
    bool bHaveNew = false;
    MY_VERIFY(m_pPrevCoreFull);

    if (IsValid(m_pPrevCoreFull)) {
        MY_VERIFY((uint8)m_eRoleType >= 0 && (uint8)m_eRoleType < (uint8)MyMJGameRoleTypeCpp::Max);
        FMyMJGameIOGroupCpp *pGroup = &(m_pPrevCoreFull->getpIOGourpAll()->m_aGroups[(uint8)m_eRoleType]);
        bHaveNew = m_cPusherBuffer.helperTrySyncDataFromCoreIOGroup(pGroup);

        if (bHaveNew) {
            int32 l = m_apNextNodes.Num();
            for (int32 i = 0; i < l; i++) {
                UMyMJIONodeCpp *pNode = m_apNextNodes[i];
                pNode->pullPushersFromPrevNodeVerified();
            }
        }

        TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc>& cmdRespQ = pGroup->getCmdOutputQueue();
        FMyMJGameCmdBaseCpp *pCmd;
        int32 l = m_apNextNodes.Num();
        while (cmdRespQ.Dequeue(pCmd)) {
            TSharedPtr<FMyMJGameCmdBaseCpp> pCmdShared = MakeShareable<FMyMJGameCmdBaseCpp>(pCmd);

            m_cCmdRespDelegate.Broadcast(pCmdShared);
            for (int32 i = 0; i < l; i++) {
                UMyMJIONodeCpp *pNode = m_apNextNodes[i];
                pNode->pulledcmdRespFromPrevNode(pCmdShared);
            }
        }
    }

    /*
    else if (m_pPrevIONode) {
        if (IsValid(m_pPrevIONode)) {
            bHaveNew = m_cPusherBuffer.copyShallowAndLogicOptimized(m_pPrevIONode->m_cPusherBuffer);
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("prev is NULL!"));
    }
    */


}

void UMyMJIONodeCpp::pullPushersFromPrevNodeVerified()
{
    bool bHaveNew = false;
    MY_VERIFY(m_pPrevIONode);

    if (IsValid(m_pPrevIONode)) {
        bHaveNew = m_cPusherBuffer.copyShallowAndLogicOptimized(m_pPrevIONode->m_cPusherBuffer);
    }

    if (bHaveNew) {
        int32 l = m_apNextNodes.Num();
        for (int32 i = 0; i < l; i++) {
            UMyMJIONodeCpp *pNode = m_apNextNodes[i];
            pNode->pullPushersFromPrevNodeVerified();
        }
    }
}

void UMyMJIONodeCpp::pulledcmdRespFromPrevNode(TSharedPtr<FMyMJGameCmdBaseCpp> pCmdShared)
{
    m_cCmdRespDelegate.Broadcast(pCmdShared);
    int32 l = m_apNextNodes.Num();
    for (int32 i = 0; i < l; i++) {
        UMyMJIONodeCpp *pNode = m_apNextNodes[i];
        pNode->pulledcmdRespFromPrevNode(pCmdShared);
    }

}

/*
void AMyMJCoreFullBaseCpp::trySyncDataFromCore(FMyMJGameIOGroupCpp *IOGroup)
{
    //1st handle cmd


    TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>& pusherQueue = IOGroup->getPusherOutputQueue();
    FMyMJGamePusherBaseCpp* pPusherGot = NULL;


    while (pusherQueue.Dequeue(pPusherGot)) {
        MyMJGamePusherTypeCpp eType = pPusherGot->getType();
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("trySyncBufferFromPrev(), got type %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)eType));
        if (eType == MyMJGamePusherTypeCpp::PusherResetGame) {
            m_cPusherBuffer.clear();
        }

        int32 idx = m_cPusherBuffer.giveInLocalThread(MakeShareable<FMyMJGamePusherBaseCpp>(pPusherGot));

        if (idx != pPusherGot->getId()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("trySyncBufferFromPrev(), pusher id not equal: %d, %d"), idx, pPusherGot->getId());
            MY_VERIFY(false);
            break;
        }
    }
}

TSharedPtr<FMyMJGamePusherBaseCpp> AMyMJCoreContactPointBaseCpp::tryPullPusherFromBuffer(int32 iGameId, int32 iPusherId)
{
    if (iPusherId < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid pusherId %d."), iPusherId);
        return NULL;
    }

    int32 l = m_cPusherBuffer.getCount();
    if (l > 0) {
        TSharedPtr<FMyMJGamePusherBaseCpp> pPusherShared0 = m_cPusherBuffer.getSharedPtrAt(0);
        MY_VERIFY(pPusherShared0->getType() == MyMJGamePusherTypeCpp::PusherResetGame);
        FMyMJGamePusherResetGameCpp* pPusherResetGame = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusherShared0.Get());
        if (iGameId == pPusherResetGame->m_iGameId) {
            if (iPusherId < l) {
                return m_cPusherBuffer.getSharedPtrAt(iPusherId);
            }
            else {
                return NULL;
            }
        }
        else {
            return pPusherShared0;
        }
    }

    return NULL;
}
*/

/*
void AMyMJCoreMirrorBaseCpp::tryProgressInMirrorMode()
{
    if (m_pContactPoint == NULL || m_pContactPoint->IsPendingKillOrUnreachable()) {
        return;
    }

    int32 iGameId = 0;
    int32 iPusherId = 0;

    FMyMJGamePusherBaseCpp *pPusher = NULL;
    do
    {
        if (m_pCoreMirror.IsValid()) {
            iGameId = m_pCoreMirror->getGameId();
            iPusherId = m_pCoreMirror->getPusherId();
        }

        pPusher = m_pContactPoint->tryPullPusherFromBuffer(iGameId, iPusherId).Get();
        if (pPusher == NULL) {
            break;
        }

        if (pPusher->getType() == MyMJGamePusherTypeCpp::PusherResetGame) {
            FMyMJGamePusherResetGameCpp* pPusherReset = StaticCast<FMyMJGamePusherResetGameCpp *>(pPusher);
            if (m_pCoreMirror.IsValid() && m_pCoreMirror->getRuleType() == pPusherReset->m_cGameCfg.m_eRuleType) {
                //rule type not changed
            }
            else {
                int32 mirrorSeedId = UMyMJUtilsLibrary::nowAsMsFromTick();
                m_pCoreMirror = MakeShareable<FMyMJGameCoreCpp>(FMyMJGameCOreThreadControlCpp::helperCreateCoreByRuleType(pPusherReset->m_cGameCfg.m_eRuleType, MyMJGameCoreWorkModeCpp::Mirror, mirrorSeedId));
            }
        }

        if (!m_pCoreMirror.IsValid()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got one pusher but coreMirror is NULL!"));
            break;
        }

        m_pCoreMirror->makeProgressByPusher(pPusher);

    } while (pPusher);
}
*/