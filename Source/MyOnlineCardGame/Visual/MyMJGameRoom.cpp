// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.h"

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
#include "Public/Blueprint/WidgetBlueprintLibrary.h"

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

    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        for (uint8 eSubtype = ((uint8)MyMJGameDeskVisualElemAttenderSubtypeCpp::Invalid + 1); eSubtype < (uint8)MyMJGameDeskVisualElemAttenderSubtypeCpp::Max; eSubtype++) {
            ret = retrieveAttenderVisualPointCfg(idxAttender, (MyMJGameDeskVisualElemAttenderSubtypeCpp)eSubtype, temp);
            if (ret != 0) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when retrieving attender cfg from Blueprint idxAttender %d, eSlot %d."), idxAttender, eSubtype);
                return ret;
            }

            cPointCfgCache.setAttenderVisualPointCfg(idxAttender, (MyMJGameDeskVisualElemAttenderSubtypeCpp)eSubtype, temp);
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
    m_pInRoomcfg = NULL;

    m_pCardCDOInGame = NULL;
};

UMyMJGameDeskResManagerCpp::~UMyMJGameDeskResManagerCpp()
{

};

bool UMyMJGameDeskResManagerCpp::checkSettings(bool bCheckDataInGame) const
{
    if (!IsValid(m_pInRoomcfg)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pInRoomcfg is invalid: %p"), m_pInRoomcfg);
        return false;
    }

    if (!m_pInRoomcfg->checkSettings())
    {
        return false;
    }
    
    //if (m_cCfgCardResPath.Path.IsEmpty()) {
    //    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card resource path not specified!"));
    //    return false;
    //}

    if (bCheckDataInGame)
    {
        if (!IsValid(getCardBaseCDOInGame()))
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card base CDO not exist, may be cfg not correct!"));
            return false;
        }
    }

    return true;
};

void UMyMJGameDeskResManagerCpp::reset()
{
    if (IsValid(m_pCardCDOInGame)) {
        m_pCardCDOInGame->K2_DestroyActor();
        m_pCardCDOInGame = NULL;
    }

    for (int32 i = 0; i < m_aCardActors.Num(); i++)
    {
        AMyMJGameCardBaseCpp* pA = m_aCardActors[i];
        if (IsValid(pA)) {
            pA->K2_DestroyActor();
        }
    }
    m_aCardActors.Reset();

    for (int32 i = 0; i < m_aTrivalDancingActors.Num(); i++)
    {
        AMyMJGameTrivalDancingActorBaseCpp* pA = m_aTrivalDancingActors[i];
        if (IsValid(pA)) {
            pA->K2_DestroyActor();
        }
    }
    m_aTrivalDancingActors.Reset();
};

bool UMyMJGameDeskResManagerCpp::OnBeginPlay()
{
    reset();

    if (!IsValid(m_pCardCDOInGame)) {

        if (!checkSettings(false)) {
            return false;
        }

        AActor *parent = Cast<AActor>(GetOuter());
        MY_VERIFY(IsValid(parent));
        UWorld *w = parent->GetWorld();
        MY_VERIFY(IsValid(w));

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        m_pCardCDOInGame = w->SpawnActor<AMyMJGameCardBaseCpp>(m_pInRoomcfg->m_cMainActorClassCfg.m_cCardClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);
        
        /*
        if (!m_pCardCDOInGame->setResPathWithRet(m_cCfgCardResPath))
        {
            m_pCardCDOInGame->Destroy();
            m_pCardCDOInGame = NULL;
            return false;
        }
        */

        MY_VERIFY(IsValid(m_pCardCDOInGame));
        m_pCardCDOInGame->SetActorHiddenInGame(true);

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("card CDO in game created."));
    }

    return true;
};

int32 UMyMJGameDeskResManagerCpp::prepareForVisual(int32 cardActorNum)
{
    //Test only

    return prepareCardActor(cardActorNum);
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

const UMyMJGameInRoomVisualCfgType* UMyMJGameDeskResManagerCpp::helperGetVisualCfg(const UObject* WorldContextObject, bool verifyValid)
{
    const UMyMJGameInRoomVisualCfgType* ret = NULL;

    while (1)
    {
        AMyMJGameRoomCpp* pRoomActor = AMyMJGameRoomRootActorCpp::helperGetRoomActor(WorldContextObject, verifyValid);

        if (pRoomActor == NULL) {
            break;
        }

        ret = pRoomActor->getResManagerVerified()->getVisualCfg(verifyValid);
        if (!IsValid(ret)) {
            ret = NULL;
            break;
        }

        break;
    }

    if (verifyValid) {
        if (!IsValid(ret)) {
            MY_VERIFY(false);
        }
    }

    return ret;
};

AMyMJGameCardBaseCpp* UMyMJGameDeskResManagerCpp::getCardActorByIdx(int32 idx)
{
    MY_VERIFY(idx >= 0);
    MY_VERIFY(idx < MY_CARD_ACTOR_MAX); //we don't allow too much
    if (idx >= m_aCardActors.Num()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("requiring a card not prepared ahead, existing %d, required idx %d."), m_aCardActors.Num(), idx);
        prepareCardActor(idx + 1);
    }

    MY_VERIFY(idx < m_aCardActors.Num());
    AMyMJGameCardBaseCpp* pRet = m_aCardActors[idx];

    MY_VERIFY(IsValid(pRet));
    return pRet;
}

AMyMJGameTrivalDancingActorBaseCpp* UMyMJGameDeskResManagerCpp::getTrivalDancingActorByClass(TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> classType, bool freeActorOnly)
{
    MY_VERIFY(IsValid(classType));

    if (classType == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class must be a child class of AMyMJGameTrivalDancingActorBaseCpp!"));
        MY_VERIFY(false);
    }

    bool bNeedRecycle = false;
    AMyMJGameTrivalDancingActorBaseCpp* ret = NULL;
    int32 iDebugExitingThisType = 0;

    int32 l = m_aTrivalDancingActors.Num();
    for (int32 i = 0; i < l; i++) {
        AMyMJGameTrivalDancingActorBaseCpp* pA = m_aTrivalDancingActors[i];
        if (!IsValid(pA)) {
            bNeedRecycle = true;
            continue;
        }

        if (pA->GetClass() != classType) {
            continue;
        }

        iDebugExitingThisType++;

        bool bIsFree = (pA->getTransformUpdateSequence()->getSeqCount() <= 0);

        if (freeActorOnly && !bIsFree) {
            continue;
        }

        ret = pA;
        break;
    }

    if (bNeedRecycle) {
        while (1) {
            
            bool bFoundInvalidItem = false;

            int32 l = m_aTrivalDancingActors.Num();
            for (int32 i = 0; i < l; i++) {
                AMyMJGameTrivalDancingActorBaseCpp* pA = m_aTrivalDancingActors[i];
                if (!IsValid(pA)) {
                    m_aTrivalDancingActors.RemoveAt(i);
                    bFoundInvalidItem = true;
                    break;
                }
            }

            if (!bFoundInvalidItem) {
                break;
            }

        }
    }

    if (ret) {
        return ret;
    }

    //not found yet, we need to spawn new one

    if (iDebugExitingThisType >= 3) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("%d dancing actors already exist, it is supposed to be small numer, type %s."), iDebugExitingThisType, *classType->GetName());
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor *parent = Cast<AActor>(GetOuter());
    MY_VERIFY(IsValid(parent));
    UWorld *w = parent->GetWorld();
    MY_VERIFY(IsValid(w));

    AMyMJGameTrivalDancingActorBaseCpp *pNewActor = w->SpawnActor<AMyMJGameTrivalDancingActorBaseCpp>(classType, FVector(0, 0, -50), FRotator(0, 0, 0), SpawnParams);
    //pNewCardActor->setResPathWithRet(m_cCfgCardResPath);

    MY_VERIFY(IsValid(pNewActor));
    pNewActor->SetActorHiddenInGame(true);
    m_aTrivalDancingActors.Emplace(pNewActor);

    return pNewActor;
}


int32 UMyMJGameDeskResManagerCpp::prepareCardActor(int32 count2reach)
{
    MY_VERIFY(count2reach >= 0);
    MY_VERIFY(count2reach < MY_CARD_ACTOR_MAX); //we don't allow too much

    const TSubclassOf<AMyMJGameCardBaseCpp>& cardClass = getVisualCfg()->m_cMainActorClassCfg.m_cCardClass;

    double s0 = FPlatformTime::Seconds();

    //const AMyMJGameCardBaseCpp* pCDO = getCardBaseCDOInGame();
    //if (!IsValid(pCDO)) {
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCDO is not valid %p, possible no card class specified."), pCDO);
        //return -1;
    //}

    if (!IsValid(cardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), cardClass.Get());
        return -1;
    }

    int32 l = m_aCardActors.Num();
    for (int32 i = (l - 1); i >= count2reach; i--) {
        AMyMJGameCardBaseCpp* pPoped = m_aCardActors.Pop();
        pPoped->K2_DestroyActor();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor *parent = Cast<AActor>(GetOuter());
    MY_VERIFY(IsValid(parent));
    UWorld *w = parent->GetWorld();
    MY_VERIFY(IsValid(w));

    l = m_aCardActors.Num();
    int32 iDebugCOunt = 0;
    for (int32 i = l; i < count2reach; i++) {
        //AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(pCDO->StaticClass(), SpawnParams); //Warning: staticClass is not virtual class, so you can't get actual class
        AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(cardClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);
        //pNewCardActor->setResPathWithRet(m_cCfgCardResPath);

        MY_VERIFY(IsValid(pNewCardActor));
        pNewCardActor->SetActorHiddenInGame(true);
        MY_VERIFY(m_aCardActors.Emplace(pNewCardActor) == i);
        iDebugCOunt++;
    }

    double s1 = FPlatformTime::Seconds();

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("prepareCardActor %s, to %d, %d created, time used %f."), *cardClass->GetName(), count2reach, iDebugCOunt, s1 - s0);

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

bool AMyMJGameRoomCpp::checkSettings() const
{
    if (!m_pResManager->checkSettings(true)) {
        return false;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDeskAreaActor is not valid! 0x%p."), m_pDeskAreaActor);
        return false;
    }

    return true;
};

void AMyMJGameRoomCpp::getCameraData(MyMJGameRoleTypeCpp roleType, FMyCardGameCameraDataCpp& cameraData) const
{
    cameraData.reset();

    const UMyMJGameInRoomVisualCfgType* pVisualCfg = getResManagerVerified()->getVisualCfg(false);
    if (pVisualCfg == NULL) {
        return;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDeskAreaActor is not valid! 0x%p."), m_pDeskAreaActor);
        return;
    }

    cameraData.m_cStaticData.m_cCenterPoint = m_pDeskAreaActor->GetActorTransform();
    cameraData.m_cStaticData.m_fMoveTime = pVisualCfg->m_cCameraCfg.m_fAttenderCameraMoveTime;
    cameraData.m_cStaticData.m_pMoveCurve = pVisualCfg->m_cCameraCfg.m_pAttenderCameraMoveCurve;

    UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(FTransform(), pVisualCfg->m_cCameraCfg.m_cAttenderCameraRelativeTransformAsAttender0, cameraData.m_cDynamicData.m_cMyTransformOfZRotation);
    cameraData.m_cDynamicData.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane = FRotator::ClampAxis(cameraData.m_cDynamicData.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane - 90 * (int32)roleType);
    cameraData.m_cDynamicData.m_fFOV = pVisualCfg->m_cCameraCfg.m_fAttenderCameraFOV;

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

    m_pResManager->prepareForVisual((27 + 3 + 2) * 4);
};

void AMyMJGameRoomCpp::stopVisual()
{
    UMyMJGameDeskVisualDataObjCpp* pO = m_pDataSuit->getDeskDataObjVerified();
    pO->stop();

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }

    m_pResManager->reset();
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
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameRoomCpp BeginPlay()"));

    Super::BeginPlay();

    if (!m_pResManager->OnBeginPlay())
    {
        return;
    }

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


void AMyMJGameRoomCpp::updateVisualData(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                        const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                        const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                                        const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                                        const TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices,
                                        uint32 uiSuggestedDur_ms)
{
    const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardMap = mNewActorDataIdCards;
    //cCoreData.checkPrivateDataInExpect();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("updateVisualData, mIdCardMap.Num() %d, role %d, pusherIdLast %d, uiSuggestedDur_ms %u."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast, uiSuggestedDur_ms);

    float fDur;
    if (uiSuggestedDur_ms > 0) {
        fDur = (float)uiSuggestedDur_ms / 1000;
    }
    else {
        fDur = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cResyncUnexpectedIngame.m_fTotalTime;
    }

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("updateVisualData, mIdCardMap.Num() %d, role %d, pusherIdLast %d, uiSuggestedDur_ms %u, fDur %f."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast, uiSuggestedDur_ms, fDur);

    for (auto& Elem : mIdCardMap)
    {
        int32 idCard = Elem.Key;
        const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);

        pCardActor->setValueShowing(cInfoAndResult.m_cVisualInfo.m_iCardValue);
        pCardActor->SetActorHiddenInGame(false);
        //pCardActor->setVisible(false);
        //pCardActor->SetActorTransform(cInfoAndResult.m_cVisualResult.m_cTransform);

        UMyTransformUpdateSequenceMovementComponent* pSeq = pCardActor->getTransformUpdateSequence();
        
        FTransformUpdateSequencDataCpp data;
        data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cInfoAndResult.m_cVisualResult.m_cTransform, fDur);
        pSeq->clearSeq();
        pSeq->addSeqToTail(data, UMyCommonUtilsLibrary::getCurveVectorDefaultLinear());

        pCardActor->addTargetToGoHistory(cInfoAndResult);
        pSeq->setHelperTransformFinal(cInfoAndResult.m_cVisualResult.m_cTransform);

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor %03d updated: %s."), idCard, *cInfoAndResult.genDebugString());
    }

    //Todo: handle other dirty data
};

void AMyMJGameRoomCpp::tipEventApplied(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                        const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                        const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardChanged,
                                        const FMyMJEventWithTimeStampBaseCpp& cEvent)
{
    //return;

    //cCoreData.checkPrivateDataInExpect();
    int32 checkId = 36;
    int32 checkValue = cCoreData.getRoleDataPrivateRefConst().m_cCardValuePack.getByIdx(checkId);

    FString eventStr = TEXT("other");
    if (cEvent.getPusherResult(false) && cEvent.getPusherResult(false)->m_aResultDelta.Num() > 0) {
        eventStr = cEvent.getPusherResult(false)->m_aResultDelta[0].genDebugString();
    }

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("tipEventApplied: %s, dur %u, role %d, checking [%d:%d]"), *eventStr, cEvent.getDuration_ms(), (uint8)cCoreData.getRole(), checkId, checkValue);

    //let's sum what happend, and inform BP layer
    if (cEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult) {
        const FMyMJGamePusherResultCpp* pPusherResult = cEvent.getPusherResult(true);
        if (pPusherResult->m_aResultBase.Num() > 0) {
            //do it later
        }
        else {
            MY_VERIFY(pPusherResult->m_aResultDelta.Num() > 0);

            const FMyMJDataDeltaCpp& cDelta = pPusherResult->m_aResultDelta[0];
            MyMJGamePusherTypeCpp ePusherType = cDelta.getType();

            bool bGotPointerCfgForAttender = false;
            int32 idxAttender = cDelta.m_iIdxAttenderActionInitiator;
            FMyMJGameDeskVisualPointCfgCpp cVisualPointForAttender;
            if (idxAttender >= 0 && idxAttender < 4 && 0 == cCfgCache.m_cPointCfg.getAttenderVisualPointCfg(idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp::OnDeskLocation, cVisualPointForAttender)) {
                bGotPointerCfgForAttender = true;
            }

            if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
      
                MY_VERIFY(cDelta.m_aCoreData.Num() > 0);
                TArray<AMyMJGameCardBaseCpp*> aCardActors;
                for (int32 i = 0; i < cDelta.m_aCoreData[0].m_aCardInfos2Update.Num(); i++) {
                    const FMyMJCardInfoCpp& cardInfo = cDelta.m_aCoreData[0].m_aCardInfos2Update[i];
                    if (cardInfo.m_cPosi.m_eSlot != MyMJCardSlotTypeCpp::JustTaken) {
                        continue;
                    }
                    AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(cardInfo.m_iId);
                    aCardActors.Emplace(pCardActor);
                }

                if (bGotPointerCfgForAttender) {
                    showVisualTakeCards((float)cEvent.getDuration_ms() / 1000, cVisualPointForAttender.m_cCenterPointWorldTransform, aCardActors);
                }

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
 
                TArray<AMyMJGameCardBaseCpp*> aCardActorsGiveOutForAttender, aCardActorsOtherForAttender;

                for (auto& Elem : mIdCardChanged)
                {
                    int32 idCard = Elem.Key;
                    const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;
                    AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);

                    if (cInfoAndResult.m_cVisualInfo.m_iIdxAttender != idxAttender)
                    {
                        continue;
                    };

                    if (cInfoAndResult.m_cVisualInfo.m_eSlot == MyMJCardSlotTypeCpp::GivenOut) {
                        aCardActorsGiveOutForAttender.Emplace(pCardActor);
                    }
                    else {
                        aCardActorsOtherForAttender.Emplace(pCardActor);
                    }
                }

                if (bGotPointerCfgForAttender) {
                    const FMyMJRoleDataAttenderPublicCpp& attenderDataPublic = cCoreData.getRoleDataAttenderPublicRefConst(idxAttender);
                    showVisualGiveOutCards((float)cEvent.getDuration_ms() / 1000, cVisualPointForAttender.m_cCenterPointWorldTransform, attenderDataPublic.m_aIdHandCards.Num(), 
                                           aCardActorsGiveOutForAttender, aCardActorsOtherForAttender);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
 
                TArray<AMyMJGameCardBaseCpp*> aCardActorsWeaved, aCardActorsOther;

                const FMyMJRoleDataAttenderPublicDeltaCpp* pAttenderPublicDelta = cDelta.getRoleDataAttenderPublicDeltaConst(idxAttender);
                MY_VERIFY(pAttenderPublicDelta);

                MY_VERIFY(pAttenderPublicDelta->m_aWeave2Add.Num() > 0);
                const FMyMJWeaveCpp& cWeave = pAttenderPublicDelta->m_aWeave2Add[0];
                const TArray<int32>& aIds = cWeave.getIdsRefConst();

                for (auto& Elem : mIdCardChanged)
                {
                    int32 idCard = Elem.Key;
                    const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;
                    AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);

                    if (aIds.Find(idCard) == INDEX_NONE)
                    {
                        aCardActorsOther.Emplace(pCardActor);
                    }
                    else {
                        aCardActorsWeaved.Emplace(pCardActor);
                    }

                }

                if (bGotPointerCfgForAttender) {
                    MyMJGameRuleTypeCpp eRuleType = cCoreData.getCoreDataPublicRefConst().m_cGameCfg.m_eRuleType;
                    showVisualWeave((float)cEvent.getDuration_ms() / 1000, cVisualPointForAttender.m_cCenterPointWorldTransform, eRuleType, cWeave, aCardActorsWeaved, aCardActorsOther);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionStateUpdate) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {

            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got a unhandled delta event with type %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
            }
        }
    }
    else if (cEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::Trival) {

    }
};

void AMyMJGameRoomCpp::tipDataSkipped()
{

};


//static void helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaCpp& meta,
    //const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
    //const TArray<UMyTransformUpdateSequenceMovementComponent *>& actorComponentsSortedGroup);

void AMyMJGameRoomCpp::showVisualTakeCards(float totalDur, const FTransform &visualPointTransformForAttender, const TArray<AMyMJGameCardBaseCpp*>& takenCardActors)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    const FMyMJGameEventPusherTakeCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cTakeCards;

    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSeq = &eventCfg.m_aCardsFocusedSteps;
    if (attenderOnScreenMeta.m_iIdxAttenderBelongTo == 0 && eventCfg.m_bOverrideCardsFocusedStepsForAttenderAsViewer) {
        pSeq = &eventCfg.m_aOverridedCardsFocusedStepsForAttenderAsViewer;
    }

    if (pSeq->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = takenCardActors;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSeq, 0, aBase, TEXT("take cards focused"), true);
    }
}

void AMyMJGameRoomCpp::showVisualGiveOutCards(float totalDur, const FTransform &visualPointTransformForAttender, int32 handCardNum,
                                                const TArray<AMyMJGameCardBaseCpp*>& cardActorsGiveOut,
                                                const TArray<AMyMJGameCardBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const FMyMJGameEventPusherGiveOutCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cGiveOutCards;

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSeq = &eventCfg.m_aCardsFocusedSteps;

    if (pSeq->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsGiveOut;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSeq, 0, aBase, TEXT("give out cards focused"), true);
    }


    int32 l = cardActorsOtherMoving.Num();
    if (l >= 1) {

        /*
        TArray<AMyMJGameCardBaseCpp *> aActors0, aActors1;
        FVector rightVector = UKismetMathLibrary::GetRightVector(visualPointForAttender.m_cCenterPointWorldTransform.GetRotation().Rotator());
        for (int32 i = 0; i < l; i++) {
            AMyMJGameCardBaseCpp* pCardActor = cardActorsOtherMoving[i];
            FVector movingVector = pCardActor->getTransform2GoRefConst().GetLocation() - pCardActor->GetActorLocation();
            float dotResult = FVector::DotProduct(rightVector, movingVector);

            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("dot result %f."), dotResult);
            if (dotResult >= 0) {
                aActors0.Emplace(pCardActor);
            }
            else {
                aActors1.Emplace(pCardActor);
            }
        }
        */

        //find the injecting cards
        //warning: assume arrnge is from 0 to n for hand cards
        TArray<AMyMJGameCardBaseCpp *> aActorsInjecting, aActorsOther;
        for (int32 i = 0; i < l; i++) {
            AMyMJGameCardBaseCpp* pCardActor = cardActorsOtherMoving[i];

            const FMyMJGameCardVisualInfoAndResultCpp* oldTarget = pCardActor->getTargetToGoHistory(1, false);
            if (oldTarget != NULL) {
                if (oldTarget->m_cVisualInfo.m_eSlot == MyMJCardSlotTypeCpp::JustTaken &&
                    pCardActor->getTargetToGoHistory(0)->m_cVisualInfo.m_iIdxColInRow < (handCardNum - 1))
                {
                    aActorsInjecting.Emplace(pCardActor);
                    continue;
                }
            }

            aActorsOther.Emplace(pCardActor);
            continue;
        }

        float totalDurUnfocused = totalDur;
        if (eventCfg.m_fTotalTimeOverridedForCardsUnfocused >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            totalDurUnfocused = eventCfg.m_fTotalTimeOverridedForCardsUnfocused;
        };
        float waitDurUnfocused = eventCfg.m_fDelayTimeForCardsUnfocused;

        //handle the injected ones
        if (eventCfg.m_aCardsInsertedToHandSlotSteps.Num() > 0) {
            pSeq = &eventCfg.m_aCardsInsertedToHandSlotSteps;
        }
        else
        {
            pSeq = &eventCfg.m_aCardsOtherSteps;
        }

        const TArray<AMyMJGameCardBaseCpp*>* paSub = &aActorsInjecting;
        if (pSeq->Num() > 0 && paSub->Num() > 0) {
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            AMyMJGameCardBaseCpp::helperToSeqActors(*paSub, true, aBase);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSeq, waitDurUnfocused, aBase, TEXT("give out cards injecting"), true);
        }

        //handle the remain ones
        pSeq = &eventCfg.m_aCardsOtherSteps;

        paSub = &aActorsOther;
        if (pSeq->Num() > 0) {
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            AMyMJGameCardBaseCpp::helperToSeqActors(*paSub, true, aBase);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSeq, waitDurUnfocused, aBase, TEXT("give out cards remains"), true);
        }
    }
}

void AMyMJGameRoomCpp::showVisualWeave(float totalDur, const FTransform &visualPointTransformForAttender, MyMJGameRuleTypeCpp ruleType, const FMyMJWeaveCpp& weave,
                                        const TArray<AMyMJGameCardBaseCpp*>& cardActorsWeaved,
                                        const TArray<AMyMJGameCardBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const FMyMJGameEventPusherWeaveVisualDataCpp* pEventCfg = NULL;
    MyMJGameWeaveVisualTypeCpp eWeaveVisualType = UMyMJBPUtilsLibrary::helperGetWeaveVisualTypeFromWeave(ruleType, weave);
    if (eWeaveVisualType == MyMJGameWeaveVisualTypeCpp::Chi) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeaveChi;
    }
    else if (eWeaveVisualType == MyMJGameWeaveVisualTypeCpp::Peng) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeavePeng;
    }
    else if (eWeaveVisualType == MyMJGameWeaveVisualTypeCpp::Gang) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeaveGang;
    }
    else if (eWeaveVisualType == MyMJGameWeaveVisualTypeCpp::Bu) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeaveBu;
    }

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSeq = &pEventCfg->m_aCardsFocusedSteps;

    if (pSeq->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsWeaved;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSeq, 0, aBase, TEXT("weave cards focused"), true);
    }

    int32 l = cardActorsOtherMoving.Num();
    if (l >= 1) {
        float totalDurUnfocused = totalDur;
        if (pEventCfg->m_fTotalTimeOverridedForCardsUnfocused >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            totalDurUnfocused = pEventCfg->m_fTotalTimeOverridedForCardsUnfocused;
        };
        float waitDurUnfocused = pEventCfg->m_fDelayTimeForCardsUnfocused;

        pSeq = &pEventCfg->m_aCardsOtherSteps;

        if (pSeq->Num() > 0) {
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsOtherMoving;
            AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSeq, waitDurUnfocused, aBase, TEXT("weave cards other"), true);
        }
    }

    while (IsValid(pEventCfg->m_cDancingActor0Class))
    {
        if (pEventCfg->m_cDancingActor0Class == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dancing actor0 specified but not a child of AMyMJGameTrivalDancingActorBaseCpp class, weave visual type %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameWeaveVisualTypeCpp"), (uint8)eWeaveVisualType));
            break;
        }

        pSeq = &pEventCfg->m_aDancingActor0Steps;
        if (pSeq->Num() > 0) {
            AMyMJGameTrivalDancingActorBaseCpp* pDancing = m_pResManager->getTrivalDancingActorByClass(pEventCfg->m_cDancingActor0Class, true);

            pDancing->SetActorHiddenInGame(false);
            pDancing->getTransformUpdateSequence()->setHideWhenInactived(true);
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            aBase.Emplace(pDancing);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDur, attenderOnScreenMeta, areaCfg.m_cCommonActionShowPoint, *pSeq, 0, aBase, TEXT("weave cards dancing actor 0"), true);
        }

        break;
    }

}