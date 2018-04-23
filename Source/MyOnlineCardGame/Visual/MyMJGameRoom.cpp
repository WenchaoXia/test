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


FMyErrorCodeMJGameCpp AMyMJGameDeskAreaCpp::retrieveCfgCache(FMyMJGameDeskVisualPointCfgCacheCpp& cPointCfgCache) const
{
    cPointCfgCache.clear();

    FMyArrangePointCfgWorld3DCpp temp;
    FMyErrorCodeMJGameCpp ret(true);
    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        for (uint8 eSlot = ((uint8)MyMJCardSlotTypeCpp::InvalidIterateMin + 1); eSlot < (uint8)MyMJCardSlotTypeCpp::InvalidIterateMax; eSlot++) {
            ret = retrieveCardVisualPointCfg(idxAttender, (MyMJCardSlotTypeCpp)eSlot, temp);
            if (ret.hasError()) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when retrieving card cfg from Blueprint idxAttender %d, eSlot %d."), idxAttender, eSlot);
                return ret;
            }

            cPointCfgCache.setCardVisualPointCfgByIdxAttenderAndSlot(idxAttender, (MyMJCardSlotTypeCpp)eSlot, temp);
        }
    }

    for (int32 idxAttender = 0; idxAttender < 4; idxAttender++) {
        for (uint8 eSubtype = ((uint8)MyMJGameDeskVisualElemAttenderSubtypeCpp::Invalid + 1); eSubtype < (uint8)MyMJGameDeskVisualElemAttenderSubtypeCpp::Max; eSubtype++) {
            ret = retrieveAttenderVisualPointCfg(idxAttender, (MyMJGameDeskVisualElemAttenderSubtypeCpp)eSubtype, temp);
            if (ret.hasError()) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when retrieving attender cfg from Blueprint idxAttender %d, eSlot %d."), idxAttender, eSubtype);
                return ret;
            }

            cPointCfgCache.setAttenderVisualPointCfg(idxAttender, (MyMJGameDeskVisualElemAttenderSubtypeCpp)eSubtype, temp);
        }
    }

    MyMJGameDeskVisualElemTypeCpp elemType = MyMJGameDeskVisualElemTypeCpp::Dice;
    ret = retrieveTrivalVisualPointCfg(elemType, 0, 0, temp);
    if (ret.hasError()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got error when retrieving trival cfg from Blueprint elemType %d, subIdx0 %d, subIdx1 %d."), (uint8)elemType, 0, 0);
        return ret;
    }

    cPointCfgCache.setTrivalVisualPointCfgByIdxAttenderAndSlot(elemType, 0, 0, temp);

    return ret;
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
        AMyMJGameCardActorBaseCpp* pA = m_aCardActors[i];
        //if (IsValid(pA)) {
            //pA->K2_DestroyActor();
        //}
        if (!pA->Destroy()) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("card actor failed to destroy."));
        }
    }
    m_aCardActors.Reset();

    for (int32 i = 0; i < m_aDiceActors.Num(); i++)
    {
        AMyMJGameDiceBaseCpp* pA = m_aDiceActors[i];
        //if (IsValid(pA)) {
            //pA->K2_DestroyActor();
        //}
        if (!pA->Destroy()) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("dice actor failed to destroy."));
        }
    }
    m_aDiceActors.Reset();

    for (int32 i = 0; i < m_aTrivalDancingActors.Num(); i++)
    {
        AMyMJGameTrivalDancingActorBaseCpp* pA = m_aTrivalDancingActors[i];
        //if (IsValid(pA)) {
            //pA->K2_DestroyActor();
        //}
        if (!pA->Destroy()) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("trival actor failed to destroy."));
        }
    }
    m_aTrivalDancingActors.Reset();

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("actors belong to room destroyed."));
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
    MY_VERIFY(cardActorNum > 0);
    getCardActorByIdxEnsured(cardActorNum - 1);

    int32 diceActorNum = 2;
    getDiceActorByIdxEnsured(diceActorNum - 1);

    return 0;
};

FMyErrorCodeMJGameCpp UMyMJGameDeskResManagerCpp::retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const
{
    FMyErrorCodeMJGameCpp ret(true);

    cModelInfoCache.clear();

    AMyMJGameCardActorBaseCpp* pCDO = getCardActorByIdxConst(0);
    if (!IsValid(pCDO)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card CDO not exist."));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::RuntimeCDONotPrepared);
    }

    FMyModelInfoWorld3DCpp modelInfo;

    ret.m_eCommonPart = pCDO->getModelInfoForUpdater(modelInfo);
    if (ret.hasError()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card CDO getModelInfo error: %s."), *ret.ToString());
        return ret;
    }
    cModelInfoCache.m_cCardModelInfo = modelInfo.getBox3DRefConst();

    AMyMJGameDiceBaseCpp* pDice = getDiceActorByIdxConst(0);
    if (!IsValid(pDice)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dice CDO not exist."));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::RuntimeCDONotPrepared);
    }

    ret = pDice->getDiceModelInfoNotFromCache(cModelInfoCache.m_cDiceModelInfo, false);
    if (ret.hasError()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dice CDO getModelInfo error: %s."), *ret.ToString());
        return ret;
    }

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("retrieveCfgCache, pCDO name %s, card box: %s."), *pCDO->GetName(), *cModelInfoCache.m_cCardModelInfo.m_cBoxExtend.ToString());

    return ret;
};

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

        bool bIsFree = (pA->getMyWithCurveUpdaterTransformRef().getStepsCount() <= 0);

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
    //pNewCardActor->setResourcePathWithRet(m_cCfgCardResPath);

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

void AMyMJGameRoomCpp::getCameraData(int32 idxDeskPosition, FMyCardGameCameraDataCpp& cameraData) const
{
    cameraData.reset();

    const UMyMJGameInRoomVisualCfgCpp* pVisualCfg = getResManagerVerified()->getVisualCfg(false);
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
    cameraData.m_cDynamicData.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane = FRotator::ClampAxis(cameraData.m_cDynamicData.m_cMyTransformOfZRotation.m_cLocation.m_fYawOnXYPlane - 90 * (int32)idxDeskPosition);
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
        FMyMJGameDeskVisualCfgCacheCpp& cCfgCache = m_cCfgCache;
        cCfgCache.reset();
        if (!retrieveCfg(cCfgCache).hasError()) {

            cCfgCache.m_bValid = true;

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
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJGameRoomCpp::loopVisual, ((float)AMyMJGameRoomVisualLoopTimeMs) / (float)1000, true);
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

    m_cCfgCache.reset();

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

FMyErrorCodeMJGameCpp AMyMJGameRoomCpp::retrieveCfg(FMyMJGameDeskVisualCfgCacheCpp& cCfgCache)
{
    cCfgCache.reset();

    if (!IsValid(m_pResManager)) {
        MY_VERIFY(false);
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::UComponentNotExist);
    }

    FMyErrorCodeMJGameCpp ret(true);
    ret = m_pResManager->retrieveCfgCache(cCfgCache.m_cModelInfo);
    if (ret.hasError()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("res manager retrive cfg cache fail: %s"), *ret.ToString());
        return ret;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("desk area not specified"));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::AActorNotExist);
    }

    //get card
    ret = m_pDeskAreaActor->retrieveCfgCache(cCfgCache.m_cPointCfg);
    if (ret.hasError()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("desk area actor retrive cfg cache fail: %s"), *ret.ToString());
        return ret;
    }

    return ret;
};

MyMJGameRuleTypeCpp AMyMJGameRoomCpp::helperGetRuleTypeNow() const
{
    return getRoomDataSuiteVerified()->getDeskDataObjVerified()->getVisualDataRefConst().getCoreDataRefConst().getCoreDataPublicRefConst().m_cGameCfg.m_eRuleType;
};

void AMyMJGameRoomCpp::updateVisualData(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                        const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                        const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecordFiltered,
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

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("updateVisualData, mIdCardMap.Num() %d, role %d, pusherIdLast %d, bIsFullBaseReset %d, uiFullBaseResetDur_ms %u."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast, bIsFullBaseReset, uiFullBaseResetDur_ms);

    for (auto& Elem : mIdCardMap)
    {
        int32 idCard = Elem.Key;
        const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdxEnsured(idCard);

        pCardActor->setValueShowing2(cInfoAndResult.m_cVisualInfo.m_iCardValue);
        pCardActor->SetActorHiddenInGame(false);
        //pCardActor->setVisible(false);
        //pCardActor->SetActorTransform(cInfoAndResult.m_cVisualResult.m_cTransform);

        FMyWithCurveUpdaterTransformWorld3DCpp* pUpdater = &pCardActor->getMyWithCurveUpdaterTransformRef();
        pUpdater->setHelperTransformOrigin(pCardActor->GetTransform());
        pUpdater->setHelperTransformFinal(cInfoAndResult.m_cVisualResult.m_cTransform);

        FMyWithCurveUpdateStepDataTransformWorld3DCpp data;
        data.helperSetDataBySrcAndDst(fDur, pCurve, pCardActor->GetTransform(), cInfoAndResult.m_cVisualResult.m_cTransform);
        pUpdater->clearSteps();
        pUpdater->addStepToTail(data);

        pCardActor->addTargetToGoHistory(cInfoAndResult);
   
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor %03d updated: %s."), idCard, *cInfoAndResult.ToString());
    }

    //Todo: handle other dirty data
    for (auto& Elem : mNewActorDataIdDices)
    {
        int32 idDice = Elem.Key;
        const FMyMJGameDiceVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

        AMyMJGameDiceBaseCpp* pDiceActor = m_pResManager->getDiceActorByIdxEnsured(idDice);
        pDiceActor->SetActorHiddenInGame(false);

        FMyWithCurveUpdaterTransformWorld3DCpp* pUpdater = &pDiceActor->getMyWithCurveUpdaterTransformRef();
        pUpdater->setHelperTransformOrigin(pDiceActor->GetTransform());
        pUpdater->setHelperTransformFinal(cInfoAndResult.m_cVisualResult.m_cTransform);

        FMyWithCurveUpdateStepDataTransformWorld3DCpp data;
        data.helperSetDataBySrcAndDst(fDur, pCurve, pDiceActor->GetTransform(), cInfoAndResult.m_cVisualResult.m_cTransform);
        pUpdater->clearSteps();
        pUpdater->addStepToTail(data);

        //pDiceActor->addTargetToGoHistory(cInfoAndResult);

        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("dice %d update transform to: %s."), idDice, *cInfoAndResult.m_cVisualResult.m_cTransform.ToString());
    }

    const TSet<int32>& sD = cCoreDataDirtyRecordFiltered.getRecordSetRefConst();
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("sd Num %d, in empty %d."), sD.Num(), cNextCoreDataDirtyRecordSincePrev.isEmpty());
    for (auto& Elem : sD)
    {
        int32 v = Elem;

        int32 subIdx0, subIdx1, subIdx2;
        cCoreDataDirtyRecordFiltered.recordValueToIdxValuesWith3Idxs(v, subIdx0, subIdx1, subIdx2);
        MyMJGameCoreDataDirtyMainTypeCpp eMainType = MyMJGameCoreDataDirtyMainTypeCpp(subIdx0);

        if (eMainType == MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePublic) {
            int32 idxAttender = subIdx1;
            const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic = cCoreData.getRoleDataAttenderPublicRefConst(idxAttender);

            UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(this, false);
            if (IsValid(pUI)) {
                pUI->showMyMJRoleDataAttenderPublicChanged(idxAttender, dataAttenderPublic, subIdx2);
            }

        }
        else if (eMainType == MyMJGameCoreDataDirtyMainTypeCpp::AttenderStatePrivate) {
            int32 idxAttender = subIdx1;
            const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate = cCoreData.getRoleDataAttenderPrivateRefConst(idxAttender);

            UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(this, false);
            if (IsValid(pUI)) {
                pUI->showMyMJRoleDataAttenderPrivateChanged(idxAttender, dataAttenderPrivate, subIdx2);
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected dirty main type %d."), (int32)eMainType);
        }
    }
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
            eventStr = cEvent.getPusherResult(false)->m_aResultDelta[0].ToString();
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
            FMyArrangePointCfgWorld3DCpp cVisualPointForAttender;
            if (idxAttender >= 0 && idxAttender < 4 && 0 == cCfgCache.m_cPointCfg.getAttenderVisualPointCfg(idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp::OnDeskLocation, cVisualPointForAttender)) {
                bGotPointerCfgForAttender = true;
            }

            if (ePusherType == MyMJGamePusherTypeCpp::ActionTakeCards) {
      
                MY_VERIFY(cDelta.m_aCoreData.Num() > 0);
                TArray<AMyMJGameCardActorBaseCpp*> aCardActors;
                for (int32 i = 0; i < cDelta.m_aCoreData[0].m_aCardInfos2Update.Num(); i++) {
                    const FMyMJCardInfoCpp& cardInfo = cDelta.m_aCoreData[0].m_aCardInfos2Update[i];
                    if (cardInfo.m_cPosi.m_eSlot != MyMJCardSlotTypeCpp::JustTaken) {
                        continue;
                    }
                    AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdxEnsured(cardInfo.m_iId);
                    aCardActors.Emplace(pCardActor);
                }

                if (bGotPointerCfgForAttender) {
                    IMyMJGameInRoomDeskInterfaceCpp::Execute_showAttenderTakeCards(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointTransform,
                                                                                aCardActors);
                }

            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
 
                TArray<AMyMJGameCardActorBaseCpp*> aCardActorsGiveOutForAttender, aCardActorsOtherForAttender;

                for (auto& Elem : mIdCardChanged)
                {
                    int32 idCard = Elem.Key;
                    const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;
                    AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdxEnsured(idCard);

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

                    IMyMJGameInRoomDeskInterfaceCpp::Execute_showAttenderGiveOutCards(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointTransform,
                                                                                    attenderDataPublic.m_aIdHandCards.Num(),
                                                                                    aCardActorsGiveOutForAttender, aCardActorsOtherForAttender);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
 
                TArray<AMyMJGameCardActorBaseCpp*> aCardActorsWeaved, aCardActorsOther;

                const FMyMJRoleDataAttenderPublicDeltaCpp* pAttenderPublicDelta = cDelta.getRoleDataAttenderPublicDeltaConst(idxAttender);
                MY_VERIFY(pAttenderPublicDelta);

                MY_VERIFY(pAttenderPublicDelta->m_aWeave2Add.Num() > 0);
                const FMyMJWeaveCpp& cWeave = pAttenderPublicDelta->m_aWeave2Add[0];
                const TArray<int32>& aIds = cWeave.getIdsRefConst();

                for (auto& Elem : mIdCardChanged)
                {
                    int32 idCard = Elem.Key;
                    const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;
                    AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdxEnsured(idCard);

                    if (aIds.Find(idCard) == INDEX_NONE)
                    {
                        aCardActorsOther.Emplace(pCardActor);
                    }
                    else {
                        aCardActorsWeaved.Emplace(pCardActor);
                    }

                }

                MyMJGameRuleTypeCpp eRuleType = helperGetRuleTypeNow();
                MyMJGameEventVisualTypeCpp eWeaveVisualType = UMyMJBPUtilsLibrary::helperGetEventVisualTypeFromWeave(eRuleType, cWeave);

                //first the bone, then the skin
                if (bGotPointerCfgForAttender) {
                    IMyMJGameInRoomDeskInterfaceCpp::Execute_showAttenderWeave(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointTransform,
                                                                            eWeaveVisualType, cWeave,
                                                                            aCardActorsWeaved, aCardActorsOther);
                    //showVisualWeave((float)cEvent.getDuration_ms() / 1000, cVisualPointForAttender.m_cCenterPointTransform, eRuleType, cWeave, aCardActorsWeaved, aCardActorsOther);
                }

                UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(this, false);
                if (IsValid(pUI)) {
                    pUI->showAttenderWeave(dur, idxAttender, eWeaveVisualType);
                }
                //if (pUI->GetClass()->ImplementsInterface(UMyMJGameInRoomUIMainWidgetInterfaceCpp::StaticClass()))
                //C++ already implemented it
                //{
                //    IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_changeViewPosition(pUI, pExtra->m_iIdxAttenderTarget);
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
                    IMyMJGameInRoomUIMainWidgetInterfaceCpp::Execute_showImportantGameStateUpdated(pUI, dur, pCoreDataDelta->m_eGameState);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionThrowDices) {

                const FMyMJCoreDataDeltaCpp* pCoreDataDelta = cDelta.getCoreDataDeltaConst();
                MY_VERIFY(pCoreDataDelta);

                TArray<AMyMJGameDiceBaseCpp *> aDices;
                aDices.Emplace(m_pResManager->getDiceActorByIdxEnsured(0));
                aDices.Emplace(m_pResManager->getDiceActorByIdxEnsured(1));

                int32 iDiceVisualStateKey = cCoreData.getCoreDataPublicRefConst().m_iDiceVisualStateKey;
                MY_VERIFY(iDiceVisualStateKey == pCoreDataDelta->m_iDiceVisualStateKey)
                if (bGotPointerCfgForAttender) {
                    IMyMJGameInRoomDeskInterfaceCpp::Execute_showAttenderThrowDices(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointTransform,
                                                                                    iDiceVisualStateKey,
                                                                                    aDices);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionDistCardsAtStart) {
                const FMyMJCoreDataDeltaCpp* pCoreDataDelta = cDelta.getCoreDataDeltaConst();
                MY_VERIFY(pCoreDataDelta);

                TArray<AMyMJGameCardActorBaseCpp*> aCardActorsDistributedForAttender, aCardActorsOtherForAttender;

                for (auto& Elem : mIdCardChanged)
                {
                    int32 idCard = Elem.Key;
                    const FMyMJGameCardVisualInfoAndResultCpp& cInfoAndResult = Elem.Value;

                    if (cInfoAndResult.m_cVisualInfo.m_iIdxAttender != idxAttender)
                    {
                        continue;
                    };

                    AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdxEnsured(idCard);
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
                    IMyMJGameInRoomDeskInterfaceCpp::Execute_showAttenderCardsDistribute(this, dur, idxAttender, cVisualPointForAttender.m_cCenterPointTransform,
                                                                                      cCoreData.getRoleDataAttenderPublicRefConst(idxAttender).m_aIdHandCards, bIsLastDistribution,
                                                                                      aCardActorsDistributedForAttender, aCardActorsOtherForAttender);
                }
                /*
                if (pCoreDataDelta->m_bUpdateGameState && pCoreDataDelta->m_eGameState == MyMJGameStateCpp::CardsDistributed)
                {
                    FMyArrangePointCfgWorld3DCpp aVisualPointCfgs[4];
                    TArray<AMyMJGameCardActorBaseCpp*> aCards[4];

                    for (int32 tempIdxAttender = 0; tempIdxAttender < 4; tempIdxAttender++) {
                        const FMyMJRoleDataAttenderPublicCpp& roleDataAttenderPublic = cCoreData.getRoleDataAttenderPublicRefConst(tempIdxAttender);
                        
                        for (int32 i = 0; i < roleDataAttenderPublic.m_aIdHandCards.Num(); i++)
                        {
                            int32 idCard = roleDataAttenderPublic.m_aIdHandCards[i];
                            AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(idCard);
                            aCards[tempIdxAttender].Emplace(pCardActor);
                        }
        
                        if (0 != cCfgCache.m_cPointCfg.getAttenderVisualPointCfg(tempIdxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp::OnDeskLocation, aVisualPointCfgs[tempIdxAttender])) {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get visual point cfg for attender %d."), tempIdxAttender);
                            aCards[tempIdxAttender].Reset();
                        }

                        if (aCards[tempIdxAttender].Num() > 0) {
                            IMyMJGameInRoomDeskInterfaceCpp::Execute_showCardsDistributed(this, dur, tempIdxAttender, aVisualPointCfgs[tempIdxAttender].m_cCenterPointTransform,
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
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got a unhandled delta event with type %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)ePusherType));
            }
        }
    }
    else if (cEvent.getMainType() == MyMJGameCoreRelatedEventMainTypeCpp::Trival) {

    }
};

void AMyMJGameRoomCpp::tipDataSkipped()
{

};


FMyErrorCodeMJGameCpp
AMyMJGameRoomCpp::showAttenderThrowDices_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                              int32 diceVisualStateKey,
                                                              const TArray<class AMyMJGameDiceBaseCpp *>& aDices)
{
    MY_VERIFY(aDices.Num() == 2);

    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxScreenPositionBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxScreenPositionBelongTo, attenderOnScreenMeta.m_cScreenCenter, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    FRandomStream RS;
    RS.Initialize(diceVisualStateKey);

    /*
    FRandomStream RS;
    RS.Initialize(uniqueId);

    float radiusMax = diceVisualPointCfg.m_cAreaBoxExtendFinal.Size2D();

    float radius0 = RS.FRandRange(0.1, 0.9) * radiusMax, radius1 = RS.FRandRange(0.1, 0.9) * radiusMax;;
    float degree0 = RS.FRandRange(0, 359);
    float degree1 = RS.FRandRange(-120, 120) + degree0 + 180;

    FVector localLoc0 = UKismetMathLibrary::RotateAngleAxis(FVector(radius0, 0, 0), degree0, FVector(0, 0, 1));
    FVector localLoc1 = UKismetMathLibrary::RotateAngleAxis(FVector(radius1, 0, 0), degree1, FVector(0, 0, 1));

    FRotator localRot0 = diceModelInfo.getLocalRotatorForDiceValueRefConst(number0);
    FRotator localRot1 = diceModelInfo.getLocalRotatorForDiceValueRefConst(number1);

    localRot0.Yaw = RS.FRandRange(0, 359);
    localRot1.Yaw = RS.FRandRange(0, 359);

    FTransform localT[2], finalT[2];
    localT[0].SetLocation(localLoc0);
    localT[0].SetRotation(localRot0.Quaternion());
    localT[1].SetLocation(localLoc1);
    localT[1].SetRotation(localRot1.Quaternion());

    finalT[0] = localT[0] * diceVisualPointCfg.m_cCenterPointTransform;
    finalT[1] = localT[1] * diceVisualPointCfg.m_cCenterPointTransform;

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("dur %f, dice visual point: %s"), dur, *diceVisualPointCfg.m_cCenterPointTransform.ToString());
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("radiusMax: %f, %s, localT[0]: %s, localT[1]: %s."), radiusMax, *diceVisualPointCfg.m_cAreaBoxExtendFinal.ToString(), *localT[0].ToString(), *localT[1].ToString());
    */

    TArray<const FTransform*> finalT;
    TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
    FVector finalLocationCenter(0, 0, 0);
    for (int32 i = 0; i < 2; i++) {
        finalT.Emplace(&aDices[i]->getMyWithCurveUpdaterTransformRef().getHelperTransformFinalRefConst());
        finalLocationCenter += finalT[i]->GetLocation();
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("dice final T: %s"), *finalT[i]->ToString());
    }
    finalLocationCenter /= 2;

    FMyModelInfoWorld3DCpp diceModelInfo = aDices[0]->getModelInfoForUpdaterEnsured();

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("debugCenterLocation: %s"), *debugCenterLocation.ToString());

    const FMyMJGameEventPusherThrowDicesVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cThrowDices;
    
    //Modify it
    TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> aSteps = eventCfg.m_aDiceSteps;
    FMyWithCurveUpdateStepSettingsTransformWorld3DCpp* pMoveOnScreenStep = NULL;
    FMyWithCurveUpdateStepSettingsTransformWorld3DCpp* pRotateStep = NULL;
    for (int32 i = 0; i < aSteps.Num(); i++) {
        if (aSteps[i].m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::PointOnPlayerScreen) {
            //first rotation to final, modify it

            pMoveOnScreenStep = &aSteps[i];
            break;
        }
    }

    for (int32 i = 0; i < aSteps.Num(); i++) {
        if (aSteps[i].m_eRotationUpdateType == MyWithCurveUpdateStepSettingsRotationType::FinalRotation) {
            //first rotation to final, modify it

            pRotateStep = &aSteps[i];
            break;
        }
    }

    //since we want different rotation for two dices, so can't setup them at one call
    for (int32 i = 0; i < 2; i++) {
        
        if (pMoveOnScreenStep) {
            //set the offset on player screeen, since we are not setup in one group
            FVector offset = (finalT[i]->GetLocation() - finalLocationCenter);
            //offset.Normalize();
            pMoveOnScreenStep->m_cLocationOffsetPercent = offset / diceModelInfo.getBox3DRefConst().m_cBoxExtend.Size(); // o.m_cBasic.m_cBoxExtend;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("dice move on screen step not found."));
        }

        if (pRotateStep) {
            int32 extraCycle = RS.RandRange(1, 5);
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
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("dice rotate step not found."));
        }

        aBase.Reset();
        aBase.Emplace(aDices[i]);

        //Todo: add location offset
        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, dur, attenderOnScreenMeta, areaCfg.m_cDiceShowPoint, aSteps, 0, aBase, TEXT("throw dices"), true);
    }

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("showAttenderThrowDices_Implementation."));

    return FMyErrorCodeMJGameCpp(true);
}

FMyErrorCodeMJGameCpp
AMyMJGameRoomCpp::showAttenderCardsDistribute_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                                   const TArray<int32>& aIdsHandCards, bool isLastDistribution,
                                                                   const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsDistributed,
                                                                   const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxScreenPositionBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxScreenPositionBelongTo, attenderOnScreenMeta.m_cScreenCenter, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const FMyMJGameEventPusherDistCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cDistCards;

    const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> *pSteps = NULL;

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
        if ((*pSteps)[i].m_eLocationUpdateType == MyWithCurveUpdateStepSettingsLocationType::OffsetFromGroupPoint) {
            bNeedGroupPoint = true;
            break;
        }
    }

    while (bNeedGroupPoint) {
        
        int32 cardDistributedBefore = aIdsHandCards.Num() - cardActorsDistributed.Num();
        MY_VERIFY(cardDistributedBefore >= 0);

        TArray<AMyMJGameCardActorBaseCpp *> aCardsTemp = cardActorsDistributed;

        aCardsTemp.Sort([](AMyMJGameCardActorBaseCpp& cardA, AMyMJGameCardActorBaseCpp& cardB) {
            const FMyMJGameCardVisualInfoAndResultCpp* pOldA = cardA.getTargetToGoHistory(1);
            const FMyMJGameCardVisualInfoAndResultCpp* pOldB = cardB.getTargetToGoHistory(1);

            if (pOldA && pOldB) {
                int32 vA = pOldA->m_cVisualInfo.m_cCol.m_iIdxElem + (pOldA->m_cVisualInfo.m_cStack.m_iIdxElem << 4);
                int32 vB = pOldB->m_cVisualInfo.m_cCol.m_iIdxElem + (pOldB->m_cVisualInfo.m_cStack.m_iIdxElem << 4);

                return vA < vB;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get TargetToGo old places: %p, %p."), pOldA, pOldB);
                return false;
            }
        });

        int32 debugGroupSetCount = 0;
        for (int32 i = 0; i < aIdsHandCards.Num(); i++) {
            AMyMJGameCardActorBaseCpp* pCardActor = m_pResManager->getCardActorByIdxEnsured(aIdsHandCards[i]);
            const FMyMJGameCardVisualInfoAndResultCpp* pTargetToGo = pCardActor->getTargetToGoHistory(0, false);
            if (pTargetToGo == NULL) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to get TargetToGo for card with id %d."), aIdsHandCards[i]);
                continue;
            }

            if (pTargetToGo->m_cVisualInfo.m_cCol.m_iIdxElem >= cardDistributedBefore) {
                int32 idxTemp = pTargetToGo->m_cVisualInfo.m_cCol.m_iIdxElem - cardDistributedBefore;

                if (idxTemp < aCardsTemp.Num()) {
                    aCardsTemp[idxTemp]->getMyWithCurveUpdaterTransformRef().setHelperTransformGroupPoint(pTargetToGo->m_cVisualResult.m_cTransform);
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
        TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
        const TArray<AMyMJGameCardActorBaseCpp*>& aSub = cardActorsDistributed;
        AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(aSub, true, aBase);

        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("cards distribute done focused"), true);
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

        TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
        const TArray<AMyMJGameCardActorBaseCpp*>& aSub = cardActorsOtherMoving;
        AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(aSub, true, aBase);

        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("weave cards other"), true);

    }

    return FMyErrorCodeMJGameCpp(true);
}

FMyErrorCodeMJGameCpp
AMyMJGameRoomCpp::showAttenderTakeCards_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                             const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsTaken)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxScreenPositionBelongTo];
    const FMyMJGameEventPusherTakeCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cTakeCards;

    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxScreenPositionBelongTo, attenderOnScreenMeta.m_cScreenCenter, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> *pSteps = &eventCfg.m_aCardsFocusedSteps;
    if (attenderOnScreenMeta.m_iIdxScreenPositionBelongTo == 0 && eventCfg.m_bOverrideCardsFocusedStepsForAttenderAsViewer) {
        pSteps = &eventCfg.m_aOverridedCardsFocusedStepsForAttenderAsViewer;
    }

    if (pSteps->Num() > 0) {
        TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
        const TArray<AMyMJGameCardActorBaseCpp*>& aSub = cardActorsTaken;
        AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(aSub, true, aBase);

        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("take cards focused"), true);
    }

    return FMyErrorCodeMJGameCpp(true);
}

FMyErrorCodeMJGameCpp
AMyMJGameRoomCpp::showAttenderGiveOutCards_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                                int32 handCardsCount,
                                                                const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsGivenOut,
                                                                const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxScreenPositionBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxScreenPositionBelongTo, attenderOnScreenMeta.m_cScreenCenter, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    const FMyMJGameEventPusherGiveOutCardsVisualDataCpp& eventCfg = m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cGiveOutCards;

    const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> *pSteps = &eventCfg.m_aCardsFocusedSteps;

    if (pSteps->Num() > 0) {
        TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
        const TArray<AMyMJGameCardActorBaseCpp*>& aSub = cardActorsGivenOut;
        AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(aSub, true, aBase);

        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("give out cards focused"), true);
    }


    int32 l = cardActorsOtherMoving.Num();
    if (l >= 1) {

        /*
        TArray<AMyMJGameCardActorBaseCpp *> aActors0, aActors1;
        FVector rightVector = UKismetMathLibrary::GetRightVector(visualPointForAttender.m_cCenterPointTransform.GetRotation().Rotator());
        for (int32 i = 0; i < l; i++) {
        AMyMJGameCardActorBaseCpp* pCardActor = cardActorsOtherMoving[i];
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
        TArray<AMyMJGameCardActorBaseCpp *> aActorsInjecting, aActorsOther;
        for (int32 i = 0; i < l; i++) {
            AMyMJGameCardActorBaseCpp* pCardActor = cardActorsOtherMoving[i];

            const FMyMJGameCardVisualInfoAndResultCpp* oldTarget = pCardActor->getTargetToGoHistory(1, false);
            if (oldTarget != NULL) {
                if (oldTarget->m_cVisualInfo.m_eSlot == MyMJCardSlotTypeCpp::JustTaken &&
                    pCardActor->getTargetToGoHistory(0)->m_cVisualInfo.m_cCol.m_iIdxElem < (handCardsCount - 1))
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

        const TArray<AMyMJGameCardActorBaseCpp*>* paSub = &aActorsInjecting;
        if (pSteps->Num() > 0 && paSub->Num() > 0) {
            TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
            AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(*paSub, true, aBase);

            UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("give out cards injecting"), true);
        }

        //handle the remain ones
        pSteps = &eventCfg.m_aCardsOtherSteps;

        paSub = &aActorsOther;
        if (pSteps->Num() > 0) {
            TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
            AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(*paSub, true, aBase);

            UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("give out cards remains"), true);
        }
    }

    return FMyErrorCodeMJGameCpp(true);
}

FMyErrorCodeMJGameCpp
AMyMJGameRoomCpp::showAttenderWeave_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                         MyMJGameEventVisualTypeCpp weaveVsualType, const struct FMyMJWeaveCpp& weave,
                                                         const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(this, visualPointTransformForAttender.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGameAttenderAreaOnPlayerScreenCfgCpp& areaCfg = m_pResManager->getVisualCfg()->m_cPlayerScreenCfg.m_aAttenderAreas[attenderOnScreenMeta.m_iIdxScreenPositionBelongTo];
    if (areaCfg.m_bAttenderPointOnScreenOverride) {
        UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(attenderOnScreenMeta.m_iIdxScreenPositionBelongTo, attenderOnScreenMeta.m_cScreenCenter, areaCfg.m_cAttenderPointOnScreenPercentOverride, attenderOnScreenMeta);
    }

    MyMJGameRuleTypeCpp eRuleType = helperGetRuleTypeNow();

    const FMyMJGameEventPusherWeaveVisualDataCpp* pEventCfg = NULL;
    MyMJGameEventVisualTypeCpp eWeaveVisualType = UMyMJBPUtilsLibrary::helperGetEventVisualTypeFromWeave(eRuleType, weave);
    if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeaveChi) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeaveChi;
    }
    else if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeavePeng) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeavePeng;
    }
    else if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeaveGang) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeaveGang;
    }
    else if (eWeaveVisualType == MyMJGameEventVisualTypeCpp::WeaveBu) {
        pEventCfg = &m_pResManager->getVisualCfg()->m_cEventCfg.m_cPusherCfg.m_cWeaveBu;
    }

    const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp> *pSteps = &pEventCfg->m_aCardsFocusedSteps;

    if (pSteps->Num() > 0) {
        TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
        const TArray<AMyMJGameCardActorBaseCpp*>& aSub = cardActorsWeaved;
        AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(aSub, true, aBase);

        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, dur, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, 0, aBase, TEXT("weave cards focused"), true);

        /*
        for (int32 i = 0; i < aSub.Num(); i++) {
            AMyMJGameCardActorBaseCpp* pA = aSub[i];
            UMyTransformUpdateSequenceMovementComponent* pC = pA->getTransformUpdateSequence();

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("weaved card %s: %s."), *pA->GetName(), *pC->getHelperTransformFinalRefConst().ToString());
        }
        */
    }

    pSteps = &pEventCfg->m_aCardsOtherSteps;
    int32 l = cardActorsOtherMoving.Num();
    if (l >= 1 && pSteps->Num() > 0) {
        float totalDurUnfocused = dur;
        if (pEventCfg->m_fTotalTimeOverridedForCardsUnfocused >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)
        {
            totalDurUnfocused = pEventCfg->m_fTotalTimeOverridedForCardsUnfocused;
        };
        float waitDurUnfocused = pEventCfg->m_fDelayTimeForCardsUnfocused;


        TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
        const TArray<AMyMJGameCardActorBaseCpp*>& aSub = cardActorsOtherMoving;
        AMyMJGameCardActorBaseCpp::helperMyMJGameCardActorBaseToMyTransformUpdaters(aSub, true, aBase);

        UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, totalDurUnfocused, attenderOnScreenMeta, areaCfg.m_cCardShowPoint, *pSteps, waitDurUnfocused, aBase, TEXT("weave cards other"), true);

    }

    while (IsValid(pEventCfg->m_cDancingActor0Class))
    {
        if (pEventCfg->m_cDancingActor0Class == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dancing actor0 specified but not a child of AMyMJGameTrivalDancingActorBaseCpp class, weave visual type %s"), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameEventVisualTypeCpp"), (uint8)eWeaveVisualType));
            break;
        }

        pSteps = &pEventCfg->m_aDancingActor0Steps;
        if (pSteps->Num() > 0) {
            AMyMJGameTrivalDancingActorBaseCpp* pDancing = m_pResManager->getTrivalDancingActorByClass(pEventCfg->m_cDancingActor0Class, true);

            pDancing->SetActorHiddenInGame(false);
            pDancing->getMyTransformUpdaterComponent()->setHideWhenInactivated(true);
            TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> aBase;
            aBase.Emplace(pDancing);

            UMyRenderUtilsLibrary::helperUpdatersSetupStepsForPointTransformWorld3D(this, dur, attenderOnScreenMeta, areaCfg.m_cCommonActionActorShowPoint, *pSteps, 0, aBase, TEXT("weave cards dancing actor 0"), true);
        }

        break;
    }

    return FMyErrorCodeMJGameCpp(true);
}