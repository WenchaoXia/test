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

#include "Curves/CurveVector.h"

#define MY_MJ_GAME_ROOM_VISUAL_LOOP_TIME_MS (17)
#define MY_CARD_ACTOR_MAX (1000)

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
    m_pCardCDOInGame = NULL;
};

UMyMJGameDeskResManagerCpp::~UMyMJGameDeskResManagerCpp()
{

};

bool UMyMJGameDeskResManagerCpp::checkSettings() const
{
    if (!IsValid(m_cCfgCardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), m_cCfgCardClass.Get());
        return false;
    }

    if (m_cCfgCardClass->GetClass() == AMyMJGameCardBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("you must specify a sub class of AMyMJGameCardBaseCpp, not it self!"));
        return false;
    }

    return true;
};

bool UMyMJGameDeskResManagerCpp::prepareForPlay()
{
    if (!IsValid(m_pCardCDOInGame)) {

        if (!checkSettings()) {
            return false;
        }

        AActor *parent = Cast<AActor>(GetOuter());
        MY_VERIFY(IsValid(parent));
        UWorld *w = parent->GetWorld();
        MY_VERIFY(IsValid(w));

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        m_pCardCDOInGame = w->SpawnActor<AMyMJGameCardBaseCpp>(m_cCfgCardClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);

        MY_VERIFY(IsValid(m_pCardCDOInGame));
        m_pCardCDOInGame->SetActorHiddenInGame(true);
    }

    return true;
};

int32 UMyMJGameDeskResManagerCpp::retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const
{
    cModelInfoCache.clear();

    const AMyMJGameCardBaseCpp* pCDO = getCardBaseCDOInGame();
    if (!IsValid(pCDO)) {
        return -1;
    }

    int32 ret = pCDO->getModelInfo(cModelInfoCache.m_cCardModelInfo);
    if (ret != 0) {
        return ret;
    }

    if (cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.X < 1 || cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.Y < 1 || cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.Z < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("mode size is too small: %s."), *cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.ToString());
        return -1;
    }

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("retrieveCfgCache, pCDO name %s, card box: %s."), *pCDO->GetName(), *cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.ToString());

    return 0;
};

#define MyArtDirBase (TEXT("/Game/Art"))
#define MyArtDirNameCommon (TEXT("Common"))
#define MyArtDirNameCurves (TEXT("Curves"))
#define MyArtFileNameCurveVectorDefaultLinear (TEXT("CurveVectorDefaultLinear"))

UCurveVector* UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear()
{
    FString fullName = FString(MyArtDirBase) + TEXT("/") + MyArtDirNameCommon + TEXT("/") + MyArtDirNameCurves + TEXT("/") + MyArtFileNameCurveVectorDefaultLinear;

    return UMyMJBPUtilsLibrary::helperTryFindAndLoadAsset<UCurveVector>(NULL, fullName);
};

AMyMJGameCardBaseCpp* UMyMJGameDeskResManagerCpp::getCardActorByIdx(int32 idx)
{
    MY_VERIFY(idx >= 0);
    MY_VERIFY(idx < MY_CARD_ACTOR_MAX); //we don't allow too much
    if (idx >= m_aCards.Num()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("requiring a card not prepared ahead, existing %d, required idx %d."), m_aCards.Num(), idx);
        prepareCardActor(idx + 1);
    }

    MY_VERIFY(idx < m_aCards.Num());
    AMyMJGameCardBaseCpp* pRet = m_aCards[idx];

    MY_VERIFY(IsValid(pRet));
    return pRet;
}

int32 UMyMJGameDeskResManagerCpp::prepareCardActor(int32 count2reach)
{
    MY_VERIFY(count2reach >= 0);
    MY_VERIFY(count2reach < MY_CARD_ACTOR_MAX); //we don't allow too much

    double s0 = FPlatformTime::Seconds();

    //const AMyMJGameCardBaseCpp* pCDO = getCardBaseCDOInGame();
    //if (!IsValid(pCDO)) {
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCDO is not valid %p, possible no card class specified."), pCDO);
        //return -1;
    //}

    if (!IsValid(m_cCfgCardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), m_cCfgCardClass.Get());
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
    int32 iDebugCOunt = 0;
    for (int32 i = l; i < count2reach; i++) {
        //AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(pCDO->StaticClass(), SpawnParams); //Warning: staticClass is not virtual class, so you can't get actual class
        AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(m_cCfgCardClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);

        MY_VERIFY(IsValid(pNewCardActor));
        pNewCardActor->SetActorHiddenInGame(true);
        MY_VERIFY(m_aCards.Emplace(pNewCardActor) == i);
        iDebugCOunt++;
    }

    double s1 = FPlatformTime::Seconds();

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("prepareCardActor %s, to %d, %d created, time used %f."), *m_cCfgCardClass->GetName(), count2reach, iDebugCOunt, s1 - s0);

    return 0;
    //GetWorld()->SpawnActor<AProjectile>(Location, Rotation, SpawnInfo);
}

AMyMJGameRoomCpp::AMyMJGameRoomCpp() : Super()
{
    m_pDataSuit = CreateDefaultSubobject<UMyMJGameRoomDataSuite>(TEXT("desk suit"));
    m_pResManager = CreateDefaultSubobject<UMyMJGameDeskResManagerCpp>(TEXT("res manager"));
    m_pDeskAreaActor = NULL;

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

    m_pResManager->prepareCardActor((27 + 3 + 2) * 4);
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
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        MY_VERIFY(false);
    }

    float timeNow = world->GetTimeSeconds();
    uint32 timeNowMs = timeNow * 1000;

    UMyMJGameDeskVisualDataObjCpp* pO = m_pDataSuit->getDeskDataObjVerified();

    pO->loop(timeNowMs);
};

void AMyMJGameRoomCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
};

void AMyMJGameRoomCpp::BeginPlay()
{
    Super::BeginPlay();

    m_pResManager->prepareForPlay();

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


void AMyMJGameRoomCpp::onDeskUpdatedWithImportantChange(FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                                        FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                                                        TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                                                        TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices)
{
    TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardMap = mNewActorDataIdCards;
    //cCoreData.checkPrivateDataInExpect();
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("onDeskUpdatedWithImportantChange, mIdCardMap.Num() %d, role %d, pusherIdLast %d."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast);

    for (auto& Elem : mIdCardMap)
    {
        int32 idCard = Elem.Key;
        FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);

        pCardActor->setValueShowing(cInfoAndResult.m_cVisualInfo.m_iCardValue);
        pCardActor->SetActorHiddenInGame(false);
        //pCardActor->setVisible(false);
        //pCardActor->SetActorTransform(cInfoAndResult.m_cVisualResult.m_cTransform);

        UMyTransformUpdateSequenceMovementComponent* pSeq = pCardActor->getTransformUpdateSequence();
        
        FTransformUpdateSequencDataCpp data;
        data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cInfoAndResult.m_cVisualResult.m_cTransform, 1.0f);
        pSeq->clearSeq();
        pSeq->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor %03d updated: %s."), idCard, *cInfoAndResult.genDebugString());
    }
};

void AMyMJGameRoomCpp::onDeskEventApplied(FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                            FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                                            TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                                            TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices,
                                            FMyMJEventWithTimeStampBaseCpp& cEvent)
{
    TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardMap = mNewActorDataIdCards;
    //cCoreData.checkPrivateDataInExpect();
    int32 checkId = 36;
    int32 checkValue = cCoreData.getRoleDataPrivateRefConst().m_cCardValuePack.getByIdx(checkId);

    FString eventStr = TEXT("other");
    if (cEvent.getPusherResult(false) && cEvent.getPusherResult(false)->m_aResultDelta.Num() > 0) {
        eventStr = cEvent.getPusherResult(false)->m_aResultDelta[0].genDebugString();
    }

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("onDeskEventApplied: %s, mIdCardMap.Num() %d, dur %u, role %d, checking [%d:%d]"), *eventStr, mIdCardMap.Num(), cEvent.getDuration_ms(), (uint8)cCoreData.getRole(), checkId, checkValue);

    for (auto& Elem : mIdCardMap)
    {
        int32 idCard = Elem.Key;
        FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);

        pCardActor->setValueShowing(cInfoAndResult.m_cVisualInfo.m_iCardValue);
        pCardActor->SetActorHiddenInGame(false);
        //pCardActor->SetActorTransform(cInfoAndResult.m_cVisualResult.m_cTransform);

        UMyTransformUpdateSequenceMovementComponent* pSeq = pCardActor->getTransformUpdateSequence();

        FTransformUpdateSequencDataCpp data;
        data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cInfoAndResult.m_cVisualResult.m_cTransform, (float)cEvent.getDuration_ms() / 1000);
        pSeq->clearSeq();
        pSeq->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor %03d updated: %s."), idCard, *cInfoAndResult.genDebugString());
    }


};