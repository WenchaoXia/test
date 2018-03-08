// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoom.h"

#include "MyMJGameRoomLevelScriptActorCpp.h"

#include "MyMJGamePlayerController.h"

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
#define MY_DICE_ACTOR_MAX (10)

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
        if (!IsValid(getCardActorByIdxConst(0)))
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card instance not exist, may be cfg not correct!"));
            return false;
        }

        if (!IsValid(getDiceActorByIdxConst(0)))
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dice instance not exist, may be cfg not correct!"));
            return false;
        }
    }

    return true;
};

void UMyMJGameDeskResManagerCpp::reset()
{
    for (int32 i = 0; i < m_aCardActors.Num(); i++)
    {
        AMyMJGameCardBaseCpp* pA = m_aCardActors[i];
        if (IsValid(pA)) {
            pA->K2_DestroyActor();
        }
    }
    m_aCardActors.Reset();

    for (int32 i = 0; i < m_aDiceActors.Num(); i++)
    {
        AMyMJGameDiceBaseCpp* pA = m_aDiceActors[i];
        if (IsValid(pA)) {
            pA->K2_DestroyActor();
        }
    }
    m_aDiceActors.Reset();

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

    if (!checkSettings(false)) {
        return false;
    }

    return true;
};

int32 UMyMJGameDeskResManagerCpp::prepareForVisual(int32 cardActorNum)
{
    if (UMyCommonUtilsLibrary::helperPrepareActorsInArray<AMyMJGameCardBaseCpp>(this, cardActorNum, getVisualCfg()->m_cMainActorClassCfg.m_cCardClass, m_aCardActors, true) < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to prepare card actor."));
        return -1;
    }

    int32 diceActorNum = 2;
    if (UMyCommonUtilsLibrary::helperPrepareActorsInArray<AMyMJGameDiceBaseCpp>(this, diceActorNum, getVisualCfg()->m_cMainActorClassCfg.m_cDiceClass, m_aDiceActors, true) < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to prepare dice actor."));
        return -2;
    }

    return 0;
};

int32 UMyMJGameDeskResManagerCpp::retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const
{
    cModelInfoCache.clear();

    const AMyMJGameCardBaseCpp* pCDO = getCardActorByIdxConst(0);
    if (!IsValid(pCDO)) {
        return -1;
    }

    int32 ret = pCDO->getModelInfo(cModelInfoCache.m_cCardModelInfo, false);
    if (ret != 0) {
        return ret;
    }

    const AMyMJGameDiceBaseCpp* pDice = getDiceActorByIdxConst(0);
    if (!IsValid(pDice)) {
        return -1;
    }

    ret = pDice->getDiceModelInfo(cModelInfoCache.m_cDiceModelInfo, false);
    if (ret != 0) {
        return ret;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("retrieveCfgCache, pCDO name %s, card box: %s."), *pCDO->GetName(), *cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.ToString());

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

AMyMJGameCardBaseCpp* UMyMJGameDeskResManagerCpp::getCardActorByIdx(int32 idx, bool verifyValid)
{
    MY_VERIFY(idx >= 0);
    MY_VERIFY(idx < MY_CARD_ACTOR_MAX); //we don't allow too much
    if (idx >= m_aCardActors.Num()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("requiring a card not prepared ahead, existing %d, required idx %d."), m_aCardActors.Num(), idx);
        UMyCommonUtilsLibrary::helperPrepareActorsInArray<AMyMJGameCardBaseCpp>(this, idx + 1, getVisualCfg()->m_cMainActorClassCfg.m_cCardClass, m_aCardActors, true);
        //prepareCardActor(idx + 1);
    }

    AMyMJGameCardBaseCpp* pRet = getCardActorByIdxConst(idx);

    if (verifyValid) {
        MY_VERIFY(IsValid(pRet));
    }

    return pRet;
}

AMyMJGameDiceBaseCpp* UMyMJGameDeskResManagerCpp::getDiceActorByIdx(int32 idx, bool verifyValid)
{
    MY_VERIFY(idx >= 0);
    MY_VERIFY(idx < MY_DICE_ACTOR_MAX); //we don't allow too much
    if (idx >= m_aDiceActors.Num()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("requiring a dice not prepared ahead, existing %d, required idx %d."), m_aDiceActors.Num(), idx);
        UMyCommonUtilsLibrary::helperPrepareActorsInArray<AMyMJGameDiceBaseCpp>(this, idx + 1, getVisualCfg()->m_cMainActorClassCfg.m_cDiceClass, m_aDiceActors, true);
    }

    AMyMJGameDiceBaseCpp* pRet = getDiceActorByIdxConst(idx);

    if (verifyValid) {
        MY_VERIFY(IsValid(pRet));
    }

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
    cameraData.m_cStaticData.m_pMoveCurve = UMyCommonUtilsLibrary::getCurveVectorFromSettings(pVisualCfg->m_cCameraCfg.m_cAttenderCameraMoveCurve);

    UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(FTransform(), pVisualCfg->m_cCameraCfg.m_cAttenderCameraRelativeTransformAsAttender0, cameraData.m_cDynamicData.m_cMyTransformOfZRotation);
    cameraData.m_cDynamicData.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane = FRotator::ClampAxis(cameraData.m_cDynamicData.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane - 90 * (int32)roleType);
    cameraData.m_cDynamicData.m_fFOV = pVisualCfg->m_cCameraCfg.m_fAttenderCameraFOV;

};

void AMyMJGameRoomCpp::startVisual()
{
    bool bStarted = false;

    if (0 != m_pResManager->prepareForVisual((27 + 3 + 2) * 4))
    {
        return;
    }

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

MyMJGameRuleTypeCpp AMyMJGameRoomCpp::helperGetRuleTypeNow() const
{
    return getRoomDataSuiteVerified()->getDeskDataObjVerified()->getVisualDataRefConst().getCoreDataRefConst().getCoreDataPublicRefConst().m_cGameCfg.m_eRuleType;
};

void AMyMJGameRoomCpp::updateVisualData(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                        const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                        const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                                        const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                                        const TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices,
                                        bool bIsFullBaseReset,
                                        uint32 uiFullBaseResetDur_ms)
{
    const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardMap = mNewActorDataIdCards;
    //cCoreData.checkPrivateDataInExpect();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("updateVisualData, mIdCardMap.Num() %d, role %d, pusherIdLast %d, uiSuggestedDur_ms %u."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast, uiSuggestedDur_ms);

    float fDur;
    UCurveVector *pCurve = NULL;
    if (bIsFullBaseReset) {
        fDur = (float)uiFullBaseResetDur_ms / 1000;
        pCurve = UMyCommonUtilsLibrary::getCurveVectorFromSettings(m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cClientCommonUpdate.m_cMoveCurve);
        //Todo: can use different curve for base reset
    }
    else {
        fDur = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cClientCommonUpdate.m_fTotalTime;
        pCurve = UMyCommonUtilsLibrary::getCurveVectorFromSettings(m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cClientCommonUpdate.m_cMoveCurve);
    }

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("updateVisualData, mIdCardMap.Num() %d, role %d, pusherIdLast %d, bIsFullBaseReset %d, uiFullBaseResetDur_ms %u."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast, bIsFullBaseReset, uiFullBaseResetDur_ms);

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
        pSeq->addSeqToTail(data, pCurve);

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
    if (cEvent.getPusherResult(false)) {
        if (cEvent.getPusherResult(false)->m_aResultDelta.Num() > 0) {
            eventStr = cEvent.getPusherResult(false)->m_aResultDelta[0].genDebugString();
        }
        else {
            eventStr = TEXT("full base update");
        }
    }

    float dur = (float)cEvent.getDuration_ms() / 1000;

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("tipEventApplied: %s, dur %u, role %d, checking [%d:%d]"), *eventStr, cEvent.getDuration_ms(), (uint8)cCoreData.getRole(), checkId, checkValue);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("tipEventApplied: %s, dur %f, maintype %d."), *eventStr, dur, (uint8)cEvent.getMainType());

    //let's sum what happend, and inform BP layer
    if (cEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::CorePusherResult) {
        const FMyMJGamePusherResultCpp* pPusherResult = cEvent.getPusherResult(true);
        if (pPusherResult->m_aResultBase.Num() > 0) {

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
                    IMyMJGameInRoomDeskInterface::Execute_showAttenderTakeCards(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointWorldTransform,
                                                                                aCardActors);
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

                    IMyMJGameInRoomDeskInterface::Execute_showAttenderGiveOutCards(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointWorldTransform,
                                                                                    attenderDataPublic.m_aIdHandCards.Num(),
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

                MyMJGameRuleTypeCpp eRuleType = helperGetRuleTypeNow();
                MyMJGameWeaveVisualTypeCpp eWeaveVisualType = UMyMJBPUtilsLibrary::helperGetWeaveVisualTypeFromWeave(eRuleType, cWeave);

                //first the bone, then the skin
                if (bGotPointerCfgForAttender) {
                    IMyMJGameInRoomDeskInterface::Execute_showAttenderWeave(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointWorldTransform,
                                                                            eWeaveVisualType, cWeave,
                                                                            aCardActorsWeaved, aCardActorsOther);
                    //showVisualWeave((float)cEvent.getDuration_ms() / 1000, cVisualPointForAttender.m_cCenterPointWorldTransform, eRuleType, cWeave, aCardActorsWeaved, aCardActorsOther);
                }

                UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(this, false);
                if (IsValid(pUI)) {
                    IMyMJGameInRoomUIMainInterface::Execute_showAttenderWeave(pUI, dur, idxAttender, eWeaveVisualType);
                }
                //if (pUI->GetClass()->ImplementsInterface(UMyMJGameInRoomUIMainInterface::StaticClass()))
                //C++ already implemented it
                //{
                //    IMyMJGameInRoomUIMainInterface::Execute_changeViewPosition(pUI, pExtra->m_iIdxAttenderTarget);
                //}
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionHu) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionHuBornLocalCS) {

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionStateUpdate) {
                const FMyMJCoreDataDeltaCpp* pCoreDataDelta = cDelta.getCoreDataDeltaConst();
                MY_VERIFY(pCoreDataDelta);

                UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(this, false);
                if (IsValid(pUI)) {
                    IMyMJGameInRoomUIMainInterface::Execute_showImportantGameStateUpdated(pUI, dur, pCoreDataDelta->m_eGameState);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {

                const FMyMJCoreDataDeltaCpp* pCoreDataDelta = cDelta.getCoreDataDeltaConst();
                MY_VERIFY(pCoreDataDelta);

                int32 diceNumerNowMaskUpdateReason = UMyMJUtilsLibrary::getIntValueFromBitMask(pCoreDataDelta->m_iDiceNumberNowMask, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_BitLen);
                MY_VERIFY(diceNumerNowMaskUpdateReason != FMyMJCoreDataPublicDirectDiceNumberNowMask_UpdateReason_Invalid);

                int32 number0, number1;
                UMyMJBPUtilsLibrary::helperGetDiceNumbersFromMask(pCoreDataDelta->m_iDiceNumberNowMask, number0, number1);

                TArray<AMyMJGameDiceBaseCpp *> aDices;
                aDices.Emplace(m_pResManager->getDiceActorByIdx(0));
                aDices.Emplace(m_pResManager->getDiceActorByIdx(1));

                int32 iSeed = cCoreData.getCoreDataPublicRefConst().m_iSeed;
                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("got seed as %d."), iSeed);
                int32 uniqueId = iSeed + (cCoreData.getCoreDataPublicRefConst().m_iGameId << 9) + cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast;

                bool bGotPointerCfgForDice = false;
                FMyMJGameDeskVisualPointCfgCpp diceVisualPointCfg;
                if (0 == cCfgCache.m_cPointCfg.getTrivalVisualPointCfgByIdxAttenderAndSlot(MyMJGameDeskVisualElemTypeCpp::Dice, 0, 0, diceVisualPointCfg)) {
                    bGotPointerCfgForDice = true;
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("failed to get dice visual point cfg."));
                }

                //first the bone, then the skin
                if (bGotPointerCfgForAttender && bGotPointerCfgForDice) {
                    IMyMJGameInRoomDeskInterface::Execute_showAttenderThrowDices(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointWorldTransform,
                                                                                 diceVisualPointCfg, cCfgCache.m_cModelInfo.m_cDiceModelInfo,
                                                                                 number0, number1, iSeed, uniqueId, aDices);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
                const FMyMJCoreDataDeltaCpp* pCoreDataDelta = cDelta.getCoreDataDeltaConst();
                MY_VERIFY(pCoreDataDelta);

                TArray<AMyMJGameCardBaseCpp*> aCardActorsDistributedForAttender, aCardActorsOtherForAttender;

                for (auto& Elem : mIdCardChanged)
                {
                    int32 idCard = Elem.Key;
                    const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

                    if (cInfoAndResult.m_cVisualInfo.m_iIdxAttender != idxAttender)
                    {
                        continue;
                    };

                    AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);
                    const FMyMJGameCardVisualInfoAndResultCpp* pPrevInfoAndResult = pCardActor->getTargetToGoHistory(1, false);
                    if (pPrevInfoAndResult && pPrevInfoAndResult->m_cVisualInfo.m_eSlot == MyMJCardSlotTypeCpp::Untaken) {
                        aCardActorsDistributedForAttender.Emplace(pCardActor);
                    }
                    else {
                        aCardActorsOtherForAttender.Emplace(pCardActor);
                    }
                }

                bool bIsLastDistribution = pCoreDataDelta->m_bUpdateGameState && pCoreDataDelta->m_eGameState == MyMJGameStateCpp::CardsDistributed;

                if (bGotPointerCfgForAttender) {
                    IMyMJGameInRoomDeskInterface::Execute_showAttenderCardsDistribute(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointWorldTransform,
                                                                                      cCoreData.getRoleDataAttenderPublicRefConst(idxAttender).m_aIdHandCards, bIsLastDistribution,
                                                                                      aCardActorsDistributedForAttender, aCardActorsOtherForAttender);
                }
                /*
                if (pCoreDataDelta->m_bUpdateGameState && pCoreDataDelta->m_eGameState == MyMJGameStateCpp::CardsDistributed)
                {
                    FMyMJGameDeskVisualPointCfgCpp aVisualPointCfgs[4];
                    TArray<AMyMJGameCardBaseCpp*> aCards[4];

                    for (int32 tempIdxAttender = 0; tempIdxAttender < 4; tempIdxAttender++) {
                        const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublic = cCoreData.getRoleDataAttenderPublicRefConst(tempIdxAttender);
                        
                        for (int32 i = 0; i < roleDataAttenderPublic.m_aIdHandCards.Num(); i++)
                        {
                            int32 idCard = roleDataAttenderPublic.m_aIdHandCards[i];
                            AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);
                            aCards[tempIdxAttender].Emplace(pCardActor);
                        }
        
                        if (0 != cCfgCache.m_cPointCfg.getAttenderVisualPointCfg(tempIdxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp::OnDeskLocation, aVisualPointCfgs[tempIdxAttender])) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get visual point cfg for attender %d."), tempIdxAttender);
                            aCards[tempIdxAttender].Reset();
                        }

                        if (aCards[tempIdxAttender].Num() > 0) {
                            IMyMJGameInRoomDeskInterface::Execute_showCardsDistributed(this, dur, tempIdxAttender, aVisualPointCfgs[tempIdxAttender].m_cCenterPointWorldTransform,
                                                                                        aCards[tempIdxAttender]);
                        }
                    }
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Dist card event unexpected, only final action should trigger. m_bUpdateGameState %d, %d."), pCoreDataDelta->m_bUpdateGameState, (uint8)pCoreDataDelta->m_eGameState);
                }
                */
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


int32 AMyMJGameRoomCpp::showAttenderThrowDices_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                              const FMyMJGameDeskVisualPointCfgCpp& diceVisualPointCfg, const FMyMJDiceModelInfoBoxCpp& diceModelInfo,
                                                              int32 number0, int32 number1, int32 seed, int32 uniqueId,
                                                              const TArray<class AMyMJGameDiceBaseCpp *>& aDices)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    FRandomStream RS;
    RS.Initialize(uniqueId);

    float radiusMax = diceVisualPointCfg.m_cAreaBoxExtendFinal.Size2D();

    float radius0 = RS.FRandRange(0.1, 0.9) * radiusMax, radius1 = RS.FRandRange(0.1, 0.9) * radiusMax;;
    float degree0 = RS.FRandRange(0, 359);
    float degree1 = RS.FRandRange(-120, 120) + degree0 + 180;

    FVector localLoc0 = UKismetMathLibrary::RotateAngleAxis(FVector(radius0, 0, 0), degree0, FVector(0, 0, 1));
    FVector localLoc1 = UKismetMathLibrary::RotateAngleAxis(FVector(radius1, 0, 0), degree1, FVector(0, 0, 1));

    FRotator localRot0 = diceModelInfo.m_cExtra.getLocalRotatorForDiceValueRefConst(number0);
    FRotator localRot1 = diceModelInfo.m_cExtra.getLocalRotatorForDiceValueRefConst(number1);

    localRot0.Yaw = RS.FRandRange(0, 359);
    localRot1.Yaw = RS.FRandRange(0, 359);

    FTransform localT[2], finalT[2];
    localT[0].SetLocation(localLoc0);
    localT[0].SetRotation(localRot0.Quaternion());
    localT[1].SetLocation(localLoc1);
    localT[1].SetRotation(localRot1.Quaternion());

    finalT[0] = localT[0] * diceVisualPointCfg.m_cCenterPointWorldTransform;
    finalT[1] = localT[1] * diceVisualPointCfg.m_cCenterPointWorldTransform;

    MY_VERIFY(aDices.Num() == 2);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("dice visual point: %s"), *diceVisualPointCfg.m_cCenterPointWorldTransform.ToString());
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("radiusMax: %f, %s, localT[0]: %s, localT[1]: %s."), radiusMax, *diceVisualPointCfg.m_cAreaBoxExtendFinal.ToString(), *localT[0].ToString(), *localT[1].ToString());

    TArray<IMyTransformUpdateSequenceInterface*> aBase;
    for (int32 i = 0; i < 2; i++) {
        aDices[i]->SetActorHiddenInGame(false);
        UMyTransformUpdateSequenceMovementComponent* pSeq = aDices[i]->getTransformUpdateSequence();
        pSeq->setHelperTransformFinal(finalT[i]);
        aBase.Emplace(aDices[i]);

        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("finalT: %s"), *finalT[i].ToString());
    }

    const FMyMJGameEventPusherThrowDicesVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cThrowDices;
    
    //Modify it
    TArray<FMyActorTransformUpdateAnimationStepCpp> aSteps = eventCfg.m_aDiceSteps;
    FMyActorTransformUpdateAnimationStepCpp* pMoveOnScreenStep = NULL;
    FMyActorTransformUpdateAnimationStepCpp* pRotateStep = NULL;
    for (int32 i = 0; i < aSteps.Num(); i++) {
        if (aSteps[i].m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen) {
            //first rotation to final, modify it

            pMoveOnScreenStep = &aSteps[i];
            break;
        }
    }
    for (int32 i = 0; i < aSteps.Num(); i++) {
        if (aSteps[i].m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::FinalRotation) {
            //first rotation to final, modify it

            pRotateStep = &aSteps[i];
            break;
        }
    }

    for (int32 i = 0; i < 2; i++) {

        if (pMoveOnScreenStep) {
            FVector centerP = (finalT[1].GetLocation() + finalT[0].GetLocation()) / 2;
            FVector offset = (finalT[i].GetLocation() - centerP);
            offset.Normalize();
            pMoveOnScreenStep->m_cLocationOffsetPercent = offset * 1; // o.m_cBasic.m_cBoxExtend;
        }

        if (pRotateStep) {
            int32 extraCycle = RS.RandRange(1, 4);
            int32 rollAxis = RS.RandRange(0, 2);
            if (rollAxis == 0) {
                pRotateStep->m_cRotationUpdateExtraCycles = FIntVector(extraCycle, 0, 0);
            }
            else if (rollAxis == 1) {
                pRotateStep->m_cRotationUpdateExtraCycles = FIntVector(0, extraCycle, 0);
            }
            else {
                pRotateStep->m_cRotationUpdateExtraCycles = FIntVector(0, 0, extraCycle);
            }

            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("extraCycle %d, rollAxis %d."), extraCycle, rollAxis);
        }

        aBase.Reset();
        aBase.Emplace(aDices[i]);

        //Todo: add location offset
        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, dur, attenderOnScreenMeta, areaCfg.m_cDiceShowPoint, aSteps, 0, aBase, TEXT("throw dices"), true);
    }

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("showAttenderThrowDices_Implementation."));

    return 0;
}

int32 AMyMJGameRoomCpp::showAttenderCardsDistribute_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                                   const TArray<int32>& aIdsHandCards, bool isLastDistribution,
                                                                   const TArray<class AMyMJGameCardBaseCpp*>& cardActorsDistributed,
                                                                   const TArray<class AMyMJGameCardBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const FMyMJGameEventPusherDistCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cDistCards;

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSteps = NULL;

    if (isLastDistribution)
    {
        pSteps = &eventCfg.m_aCardsFocusedStepsOverrideForLast;
    }
    else {
        pSteps = &eventCfg.m_aCardsFocusedSteps;
    }


    //special code: OffsetFromGroupPoint support
    bool bNeedGroupPoint = false;
    for (int32 i = 0; i < pSteps->Num(); i++) {
        if ((*pSteps)[i].m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::OffsetFromGroupPoint) {
            bNeedGroupPoint = true;
            break;
        }
    }

    while (bNeedGroupPoint) {
        
        int32 cardDistributedBefore = aIdsHandCards.Num() - cardActorsDistributed.Num();
        MY_VERIFY(cardDistributedBefore >= 0);

        TArray<AMyMJGameCardBaseCpp *> aCardsTemp = cardActorsDistributed;

        aCardsTemp.Sort([](AMyMJGameCardBaseCpp& cardA, AMyMJGameCardBaseCpp& cardB) {
            const FMyMJGameCardVisualInfoAndResultCpp* pOldA = cardA.getTargetToGoHistory(1);
            const FMyMJGameCardVisualInfoAndResultCpp* pOldB = cardB.getTargetToGoHistory(1);

            if (pOldA && pOldB) {
                int32 vA = pOldA->m_cVisualInfo.m_iIdxColInRow + (pOldA->m_cVisualInfo.m_iIdxStackInCol << 4);
                int32 vB = pOldB->m_cVisualInfo.m_iIdxColInRow + (pOldB->m_cVisualInfo.m_iIdxStackInCol << 4);

                return vA < vB;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get TargetToGo old places: %p, %p."), pOldA, pOldB);
                return false;
            }
        });

        int32 debugGroupSetCount = 0;
        for (int32 i = 0; i < aIdsHandCards.Num(); i++) {
            AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(aIdsHandCards[i]);
            const FMyMJGameCardVisualInfoAndResultCpp* pTargetToGo = pCardActor->getTargetToGoHistory(0, false);
            if (pTargetToGo == NULL) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get TargetToGo for card with id %d."), aIdsHandCards[i]);
                continue;
            }

            if (pTargetToGo->m_cVisualInfo.m_iIdxColInRow >= cardDistributedBefore) {
                int32 idxTemp = pTargetToGo->m_cVisualInfo.m_iIdxColInRow - cardDistributedBefore;

                if (idxTemp < aCardsTemp.Num()) {
                    aCardsTemp[idxTemp]->getTransformUpdateSequence(true)->setHelperTransformGroupPoint(pTargetToGo->m_cVisualResult.m_cTransform);
                    debugGroupSetCount++;
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idxTemp out of range %d, id %d"), idxTemp, aIdsHandCards[i]);
                }
            }
        }

        if (debugGroupSetCount != aCardsTemp.Num()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough of group point setted, only %d/%d setted."), debugGroupSetCount, aCardsTemp.Num());
        }

        break;
    }



    if (pSteps->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsDistributed;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("cards distribute done focused"), true);
    }

    pSteps = &eventCfg.m_aCardsOtherSteps;
    int32 l = cardActorsOtherMoving.Num();
    if (l >= 1 && pSteps->Num() > 0) {
        float totalDurUnfocused = dur;
        if (eventCfg.m_fTotalTimeOverridedForCardsUnfocused >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            totalDurUnfocused = eventCfg.m_fTotalTimeOverridedForCardsUnfocused;
        };
        float waitDurUnfocused = eventCfg.m_fDelayTimeForCardsUnfocused;

        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsOtherMoving;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("weave cards other"), true);

    }

    return 0;
}

int32 AMyMJGameRoomCpp::showAttenderTakeCards_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                             const TArray<AMyMJGameCardBaseCpp*>& cardActorsTaken)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    const FMyMJGameEventPusherTakeCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cTakeCards;

    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSteps = &eventCfg.m_aCardsFocusedSteps;
    if (attenderOnScreenMeta.m_iIdxAttenderBelongTo == 0 && eventCfg.m_bOverrideCardsFocusedStepsForAttenderAsViewer) {
        pSteps = &eventCfg.m_aOverridedCardsFocusedStepsForAttenderAsViewer;
    }

    if (pSteps->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsTaken;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("take cards focused"), true);
    }

    return 0;
}

int32 AMyMJGameRoomCpp::showAttenderGiveOutCards_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                                int32 handCardsCount,
                                                                const TArray<AMyMJGameCardBaseCpp*>& cardActorsGivenOut,
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

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSteps = &eventCfg.m_aCardsFocusedSteps;

    if (pSteps->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsGivenOut;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("give out cards focused"), true);
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
                    pCardActor->getTargetToGoHistory(0)->m_cVisualInfo.m_iIdxColInRow < (handCardsCount - 1))
                {
                    aActorsInjecting.Emplace(pCardActor);
                    continue;
                }
            }

            aActorsOther.Emplace(pCardActor);
            continue;
        }

        float totalDurUnfocused = dur;
        if (eventCfg.m_fTotalTimeOverridedForCardsUnfocused >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            totalDurUnfocused = eventCfg.m_fTotalTimeOverridedForCardsUnfocused;
        };
        float waitDurUnfocused = eventCfg.m_fDelayTimeForCardsUnfocused;

        //handle the injected ones
        if (eventCfg.m_aCardsInsertedToHandSlotSteps.Num() > 0) {
            pSteps = &eventCfg.m_aCardsInsertedToHandSlotSteps;
        }
        else
        {
            pSteps = &eventCfg.m_aCardsOtherSteps;
        }

        const TArray<AMyMJGameCardBaseCpp*>* paSub = &aActorsInjecting;
        if (pSteps->Num() > 0 && paSub->Num() > 0) {
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            AMyMJGameCardBaseCpp::helperToSeqActors(*paSub, true, aBase);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("give out cards injecting"), true);
        }

        //handle the remain ones
        pSteps = &eventCfg.m_aCardsOtherSteps;

        paSub = &aActorsOther;
        if (pSteps->Num() > 0) {
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            AMyMJGameCardBaseCpp::helperToSeqActors(*paSub, true, aBase);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("give out cards remains"), true);
        }
    }

    return 0;
}

int32 AMyMJGameRoomCpp::showAttenderWeave_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                         MyMJGameWeaveVisualTypeCpp weaveVsualType, const struct FMyMJWeaveCpp& weave,
                                                         const TArray<class AMyMJGameCardBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxAttenderBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxAttenderBelongTo, attenderOnScreenMeta.m_cScreenCenterMapped, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    MyMJGameRuleTypeCpp eRuleType = helperGetRuleTypeNow();

    const FMyMJGameEventPusherWeaveVisualDataCpp* pEventCfg = NULL;
    MyMJGameWeaveVisualTypeCpp eWeaveVisualType = UMyMJBPUtilsLibrary::helperGetWeaveVisualTypeFromWeave(eRuleType, weave);
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

    const TArray<FMyActorTransformUpdateAnimationStepCpp> *pSteps = &pEventCfg->m_aCardsFocusedSteps;

    if (pSteps->Num() > 0) {
        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsWeaved;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("weave cards focused"), true);

        /*
        for (int32 i = 0; i < aSub.Num(); i++) {
            AMyMJGameCardBaseCpp* pA = aSub[i];
            UMyTransformUpdateSequenceMovementComponent* pC = pA->getTransformUpdateSequence();

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("weaved card %s: %s."), *pA->GetName(), *pC->getHelperTransformFinalRefConst().ToString());
        }
        */
    }

    int32 l = cardActorsOtherMoving.Num();
    if (l >= 1 && pSteps->Num() > 0) {
        float totalDurUnfocused = dur;
        if (pEventCfg->m_fTotalTimeOverridedForCardsUnfocused >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            totalDurUnfocused = pEventCfg->m_fTotalTimeOverridedForCardsUnfocused;
        };
        float waitDurUnfocused = pEventCfg->m_fDelayTimeForCardsUnfocused;

        pSteps = &pEventCfg->m_aCardsOtherSteps;

        TArray<IMyTransformUpdateSequenceInterface*> aBase;
        const TArray<AMyMJGameCardBaseCpp*>& aSub = cardActorsOtherMoving;
        AMyMJGameCardBaseCpp::helperToSeqActors(aSub, true, aBase);

        UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("weave cards other"), true);

    }

    while (IsValid(pEventCfg->m_cDancingActor0Class))
    {
        if (pEventCfg->m_cDancingActor0Class == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dancing actor0 specified but not a child of AMyMJGameTrivalDancingActorBaseCpp class, weave visual type %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameWeaveVisualTypeCpp"), (uint8)eWeaveVisualType));
            break;
        }

        pSteps = &pEventCfg->m_aDancingActor0Steps;
        if (pSteps->Num() > 0) {
            AMyMJGameTrivalDancingActorBaseCpp* pDancing = m_pResManager->getTrivalDancingActorByClass(pEventCfg->m_cDancingActor0Class, true);

            pDancing->SetActorHiddenInGame(false);
            pDancing->getTransformUpdateSequence()->setHideWhenInactived(true);
            TArray<IMyTransformUpdateSequenceInterface*> aBase;
            aBase.Emplace(pDancing);

            UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(this, dur, attenderOnScreenMeta, areaCfg.m_cCommonActionShowPoint, *pSteps, 0, aBase, TEXT("weave cards dancing actor 0"), true);
        }

        break;
    }

    return 0;
}