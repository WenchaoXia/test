// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoom.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"

#define MY_MJ_GAME_ROOM_VISUAL_LOOP_TIME_MS (17)

int32 AMyMJGameDeskAreaCpp::retrieveCfgCache(FMyMJGameDeskVisualPointCfgCacheCpp& cPointCfgCache) const
{
    cPointCfgCache.clear();

    FMyMJGameDeskVisualPointCfgCpp temp;
    int32 ret;
    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        for (uint8 eSlot = ((uint8)MyMJCardSlotTypeCpp::InvalidIterateMin + 1); eSlot < (uint8)MyMJCardSlotTypeCpp::InvalidIterateMax; eSlot++) {
            ret = retrieveCardVisualPointCfg(idxAttender, (MyMJCardSlotTypeCpp)eSlot, temp);
            if (ret != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when retrieving card cfg from Blueprint idxAttender %d, eSlot %d."), idxAttender, eSlot);
                return ret;
            }

            cPointCfgCache.setCardVisualPointCfgByIdxAttenderAndSlot(idxAttender, (MyMJCardSlotTypeCpp)eSlot, temp);
        }
    }

    MyMJGameDeskVisualElemTypeCpp elemType = MyMJGameDeskVisualElemTypeCpp::Dice;
    ret = retrieveTrivalVisualPointCfg(elemType, 0, 0, temp);
    if (ret != 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when retrieving trival cfg from Blueprint elemType %d, subIdx0 %d, subIdx1 %d."), (uint8)elemType, 0, 0);
        return ret;
    }

    cPointCfgCache.setTrivalVisualPointCfgByIdxAttenderAndSlot(elemType, 0, 0, temp);

    return 0;
};

UMyMJGameDeskResManagerCpp::UMyMJGameDeskResManagerCpp() : Super()
{
};

UMyMJGameDeskResManagerCpp::~UMyMJGameDeskResManagerCpp()
{

};

bool UMyMJGameDeskResManagerCpp::checkSettings() const
{
    const AMyMJGameCardBaseCpp* pCDO = getCardCDO();
    if (IsValid(pCDO)) {
        return true;
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCDO is not valid %p, possible no card class specified."), pCDO);
        return false;
    }
};

int32 UMyMJGameDeskResManagerCpp::retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const
{
    cModelInfoCache.clear();

    const AMyMJGameCardBaseCpp* pCDO = getCardCDO();
    if (!IsValid(pCDO)) {
        return -1;
    }

    pCDO->getModelInfo(cModelInfoCache.m_cCardModelInfo);

    if (cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.X < 1 || cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.Y < 1 || cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.Z < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("mode size is too small: %s."), *cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.ToString());
        return -1;
    }

    return 0;
};

int32 UMyMJGameDeskResManagerCpp::prepareCardActor(int32 count2reach)
{
    MY_VERIFY(count2reach >= 0);

    const AMyMJGameCardBaseCpp* pCDO = getCardCDO();
    if (!IsValid(pCDO)) {
        return -1;
    }

    int32 l = m_aCards.Num();
    for (int32 i = (l - 1); i >= count2reach; i--) {
        AMyMJGameCardBaseCpp* pPoped = m_aCards.Pop();
        pPoped->K2_DestroyActor();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor *parent = Cast<AActor>(GetOuter());
    MY_VERIFY(IsValid(parent));
    UWorld *w = parent->GetWorld();
    MY_VERIFY(IsValid(w));

    l = m_aCards.Num();
    for (int32 i = l; i < count2reach; i++) {
        AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(pCDO->StaticClass(), SpawnParams);
        MY_VERIFY(IsValid(pNewCardActor));
        pNewCardActor->SetActorHiddenInGame(true);
        MY_VERIFY(m_aCards.Emplace(pNewCardActor) == i);
    }

    return 0;
    //GetWorld()->SpawnActor<AProjectile>(Location, Rotation, SpawnInfo);
}

const AMyMJGameCardBaseCpp* UMyMJGameDeskResManagerCpp::getCardCDO() const
{
    if (!IsValid(m_cCfgCardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), m_cCfgCardClass.Get());
        return NULL;
    }

    AMyMJGameCardBaseCpp *pCDO = m_cCfgCardClass->GetDefaultObject<AMyMJGameCardBaseCpp>();
    if (!IsValid(pCDO)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card pCDO is invalid: %p"), pCDO);
        return NULL;
    }

    return pCDO;
}

AMyMJGameRoomCpp::AMyMJGameRoomCpp() : Super()
{
    m_pDataSuit = CreateDefaultSubobject<UMyMJGameRoomDataSuite>(TEXT("desk suit"));
    m_pResManager = CreateDefaultSubobject<UMyMJGameDeskResManagerCpp>(TEXT("res manager"));
    m_pDeskAreaActor = NULL;

    m_eVisualState = AMyMJGameRoomVisualStateCpp::Invalid;
    m_uiVisualStateStartClientTime_ms = 0;

    bNetLoadOnClient = true;
};

AMyMJGameRoomCpp::~AMyMJGameRoomCpp()
{

};

void AMyMJGameRoomCpp::startVisual()
{
    bool bStarted = false;
    if (checkSettings()) {
        FMyMJGameDeskVisualCfgCacheCpp cCfgCache;
        if (0 == retrieveCfg(cCfgCache)) {

            UMyMJGameDeskVisualDataObjCpp* pO = m_pDataSuit->getDeskDataObjVerified();

            pO->stop();
            pO->start();
            pO->updateCfgCache(cCfgCache);
            bStarted = true;
        }
    }

    if (!bStarted) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to start visual."));
        return;
    }

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJGameRoomCpp::loopVisual, ((float)MY_MJ_GAME_ROOM_VISUAL_LOOP_TIME_MS) / (float)1000, true);
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("startVisual() OK, and visual loop timer started."));
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
        MY_VERIFY(false);
    }
};

void AMyMJGameRoomCpp::stopVisual()
{
    UMyMJGameDeskVisualDataObjCpp* pO = m_pDataSuit->getDeskDataObjVerified();
    pO->stop();

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }
};

void AMyMJGameRoomCpp::loopVisual()
{
    UMyMJGameDeskVisualDataObjCpp* pO = m_pDataSuit->getDeskDataObjVerified();

    pO->loop();
};

void AMyMJGameRoomCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
};

void AMyMJGameRoomCpp::BeginPlay()
{
    Super::BeginPlay();

    if (UMyMJBPUtilsLibrary::haveClientVisualLayer(this)) {
        startVisual();
    }
};

void AMyMJGameRoomCpp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (UMyMJBPUtilsLibrary::haveClientVisualLayer(this)) {
        stopVisual();
    }
};

bool AMyMJGameRoomCpp::checkSettings() const
{
    if (!m_pResManager->checkSettings()) {
        return false;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDeskAreaActor is not valid! 0x%p."), m_pDeskAreaActor);
        return false;
    }

    return true;
};

void AMyMJGameRoomCpp::loop()
{
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("AMyMJGameRoomCpp::loop"));

    UWorld* world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world of core is not valid!"));
        return;
    }

    uint32 uiClientTimeNow_ms = world->GetTimeSeconds() * 1000;
    uint32 uiClientTimeLast_ms = m_cGameProgressData.m_cLastBond.getClientTime_ms_RefConst();
    uint32 uiServerTimeLast_ms = m_cGameProgressData.m_cLastBond.getServerTime_ms_RefConst();

    if (uiClientTimeNow_ms < uiClientTimeLast_ms) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("time screw: uiClientTimeNow_ms %d, uiClientTimeLast_ms %d."), uiClientTimeNow_ms, uiClientTimeLast_ms);
        m_cGameProgressData.m_cLastBond.rebondTime(uiServerTimeLast_ms, uiClientTimeNow_ms);
        return;
    }

    uint32 uiDataGotServerTimeMin_ms = 0;
    uint32 uiDataGotServerTimeMax_ms = 0;
    getDataTimeRange(uiDataGotServerTimeMin_ms, uiDataGotServerTimeMax_ms);
    MY_VERIFY(uiDataGotServerTimeMin_ms <= uiDataGotServerTimeMax_ms);

    //by default, we keep serverTime as old
    uint32 uiServerTimeToUpdate_ms = uiServerTimeLast_ms;

    while (1) {
        if (m_eVisualState == AMyMJGameRoomVisualStateCpp::NormalPlay) {
            //try step forward
            uint32 uiServerTimeTryToGo_ms = m_cGameProgressData.m_cLastBond.getCalculatedServerTime_ms(uiClientTimeNow_ms);

            if (uiServerTimeTryToGo_ms < uiDataGotServerTimeMin_ms) {
                //catch up
                changeVisualState(AMyMJGameRoomVisualStateCpp::CatchUp, uiClientTimeNow_ms);
                break;
            }
            if (uiServerTimeTryToGo_ms <= uiDataGotServerTimeMax_ms && (uiDataGotServerTimeMax_ms - uiServerTimeTryToGo_ms) >= MyMJGameProgressLatenceAboveNetworkMax) {
                //catch up
                changeVisualState(AMyMJGameRoomVisualStateCpp::CatchUp, uiClientTimeNow_ms);
                break;
            }

            if (uiServerTimeTryToGo_ms > uiDataGotServerTimeMax_ms) {
                //no new data, wait
                changeVisualState(AMyMJGameRoomVisualStateCpp::WaitingForDataInGame, uiClientTimeNow_ms);
                break;;
            }

            uiServerTimeToUpdate_ms = uiServerTimeTryToGo_ms;
            playGameProgressTo(uiServerTimeToUpdate_ms, false);
        }
        else if (m_eVisualState == AMyMJGameRoomVisualStateCpp::CatchUp) {
            if (uiDataGotServerTimeMax_ms > uiServerTimeLast_ms) {
                uiServerTimeToUpdate_ms = uiDataGotServerTimeMax_ms;
                playGameProgressTo(uiServerTimeToUpdate_ms, true);
            }

            if ((uiClientTimeNow_ms - m_uiVisualStateStartClientTime_ms) >= MyMJGameVisualStateCatchUpMinTimeToStayMs) {
                changeVisualState(AMyMJGameRoomVisualStateCpp::NormalPlay, uiClientTimeNow_ms);
            }
        }
        else if (m_eVisualState == AMyMJGameRoomVisualStateCpp::WaitingForDataInGame) {
            uint32 uiServerTimeTryToGo_ms = m_cGameProgressData.m_cLastBond.getCalculatedServerTime_ms(uiClientTimeNow_ms);
            if (uiServerTimeTryToGo_ms <= uiDataGotServerTimeMax_ms) {
                //change mode and play imediently
                changeVisualState(AMyMJGameRoomVisualStateCpp::NormalPlay, uiClientTimeNow_ms);
                uiServerTimeToUpdate_ms = uiServerTimeTryToGo_ms;
                playGameProgressTo(uiServerTimeToUpdate_ms, false);
                break;;
            }

            //TOdo, more action if too late
            //check if data arrives or 

            if ((uiClientTimeNow_ms - m_uiVisualStateStartClientTime_ms) > 2000) {
                bool bFullDataSync = m_pDataSuit->getDeskDataObjVerified()->getInFullDataSyncState();
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("waiting for desk data %d."), bFullDataSync);
                //toogleWaitingForDataTipPanel();
            }
        }
        else if (m_eVisualState == AMyMJGameRoomVisualStateCpp::WaitingForDataInitSync) {
            //find first avaiable data

            if (uiDataGotServerTimeMin_ms > 0) {
                //we have data, play to it
                uiServerTimeToUpdate_ms = uiDataGotServerTimeMin_ms + AMyMJGameRoomVisualLoopTimeMs * 1; //try forward a bit
                if (uiServerTimeToUpdate_ms > uiDataGotServerTimeMax_ms) {
                    uiServerTimeToUpdate_ms = uiDataGotServerTimeMax_ms;
                }
                playGameProgressTo(uiServerTimeToUpdate_ms, false);
                changeVisualState(AMyMJGameRoomVisualStateCpp::NormalPlay, uiClientTimeNow_ms);
            }
        }
        else if (m_eVisualState == AMyMJGameRoomVisualStateCpp::Invalid) {
            changeVisualState(AMyMJGameRoomVisualStateCpp::WaitingForDataInitSync, uiClientTimeNow_ms);
        }

        break;
    }

    //rebond the game time
    m_cGameProgressData.m_cLastBond.rebondTime(uiServerTimeToUpdate_ms, uiClientTimeNow_ms);
}

void AMyMJGameRoomCpp::playGameProgressTo(uint32 uiServerTime_ms, bool bCatchUp)
{

}

void AMyMJGameRoomCpp::getDataTimeRange(uint32 &out_uiDataGotServerTimeMin_ms, uint32& out_uiDataGotServerTimeMax_ms)
{

}

int32 AMyMJGameRoomCpp::retrieveCfg(FMyMJGameDeskVisualCfgCacheCpp& cCfgCache)
{
    cCfgCache.clear();

    if (!IsValid(m_pResManager)) {
        return -1;
    }

    int32 ret;
    ret = m_pResManager->retrieveCfgCache(cCfgCache.m_cModelInfo);
    if (ret != 0) {
        return ret;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        return -2;
    }

    //get card
    ret = m_pDeskAreaActor->retrieveCfgCache(cCfgCache.m_cPointCfg);
    if (ret != 0) {
        return ret;
    }

    return 0;
};