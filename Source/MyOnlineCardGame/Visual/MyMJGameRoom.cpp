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


void FMyMJGameInGamePlayerScreenCfgCpp::fillDefaultData()
{
    for (int32 i = 0; i < 4; i++) {
        m_aAttenderAreas[i].reset();
        m_aAttenderAreas[i].m_cCardShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.1;
        m_aAttenderAreas[i].m_cCardShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.3;
        m_aAttenderAreas[i].m_cCardShowPoint.m_cExtraOffsetScreenPercent.Y = -0.05;

        m_aAttenderAreas[i].m_cCommonActionShowPoint.m_fShowPosiFromCenterToBorderPercent = 0.6;
        m_aAttenderAreas[i].m_cCommonActionShowPoint.m_fTargetVLengthOnScreenScreenPercent = 0.4;
        m_aAttenderAreas[i].m_cCommonActionShowPoint.m_cExtraOffsetScreenPercent.Y = -0.05;
    }
};


UMyMJGameDeskResManagerCpp::UMyMJGameDeskResManagerCpp() : Super()
{
    m_pCardCDOInGame = NULL;
};

UMyMJGameDeskResManagerCpp::~UMyMJGameDeskResManagerCpp()
{

};

bool UMyMJGameDeskResManagerCpp::checkSettings(bool bCheckDataInGame) const
{
    if (!IsValid(m_cCfgCardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), m_cCfgCardClass.Get());
        return false;
    }

    if (m_cCfgCardClass->GetClass() == AMyMJGameCardBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("you must specify a sub class of AMyMJGameCardBaseCpp, not it self!"));
        return false;
    }
    
    //if (m_cCfgCardResPath.Path.IsEmpty()) {
    //    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card resource path not specified!"));
    //    return false;
    //}

    for (int32 i = ((int32)MyMJGameTrivalDancingTypeCpp::Invalid + 1); i < (int32)MyMJGameTrivalDancingTypeCpp::Max; i++)
    {
        MyMJGameTrivalDancingTypeCpp eType = (MyMJGameTrivalDancingTypeCpp)i;
        const TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp>* pSubClass = m_mCfgTrivalDancingClasses.Find(eType);
        if (pSubClass == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("subclass not specified for type: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameTrivalDancingTypeCpp"), (uint8)eType));
            return false;
        }

        if (!IsValid(*pSubClass)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("subclass not valid for type: %s, pointer %p."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameTrivalDancingTypeCpp"), (uint8)eType), pSubClass);
            return false;
        }

        if ((*pSubClass)->GetClass() == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("you must specify a sub class of AMyMJGameTrivalDancingActorBaseCpp, not it self, type: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameTrivalDancingTypeCpp"), (uint8)eType));
            return false;
        }
    }


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
        m_pCardCDOInGame = w->SpawnActor<AMyMJGameCardBaseCpp>(m_cCfgCardClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);
        
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
    m_cInGamePlayerScreenCfg.fillDefaultData();

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

#define MyArtDirBase (TEXT("/Game/Art"))
#define MyArtDirNameCommon (TEXT("Common"))
#define MyArtDirNameCurves (TEXT("Curves"))
#define MyArtFileNameCurveVectorDefaultLinear (TEXT("CurveVectorDefaultLinear"))

UCurveVector* UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear()
{
    FString fullName = FString(MyArtDirBase) + TEXT("/") + MyArtDirNameCommon + TEXT("/") + MyArtDirNameCurves + TEXT("/") + MyArtFileNameCurveVectorDefaultLinear;

    return UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UCurveVector>(NULL, fullName);
};


MyMJGameTrivalDancingTypeCpp UMyMJGameDeskResManagerCpp::helperGetWeaveDancingTypeFromWeave(MyMJGameRuleTypeCpp ruleType, const FMyMJWeaveCpp& weave)
{
    if (ruleType == MyMJGameRuleTypeCpp::LocalCS)
    {
        MyMJWeaveTypeCpp eWeaveType = weave.getType();

        if (eWeaveType == MyMJWeaveTypeCpp::ShunZiAn || eWeaveType == MyMJWeaveTypeCpp::ShunZiMing) {
            return MyMJGameTrivalDancingTypeCpp::Chi;
        }
        else if (eWeaveType == MyMJWeaveTypeCpp::KeZiAn || eWeaveType == MyMJWeaveTypeCpp::KeZiMing) {
            return MyMJGameTrivalDancingTypeCpp::Peng;
        }
        else if (eWeaveType == MyMJWeaveTypeCpp::GangAn || eWeaveType == MyMJWeaveTypeCpp::GangMing) {
            bool bIsBuZhang = weave.getGangBuZhangLocalCS();
            if (bIsBuZhang) {
                return MyMJGameTrivalDancingTypeCpp::Bu;
            }
            else {
                return MyMJGameTrivalDancingTypeCpp::Gang;
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected weave type: %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJWeaveTypeCpp"), (uint8)eWeaveType));
            return MyMJGameTrivalDancingTypeCpp::Invalid;
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("meet unsupported rule type: %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)ruleType));
        return MyMJGameTrivalDancingTypeCpp::Invalid;
    }
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

AMyMJGameTrivalDancingActorBaseCpp* UMyMJGameDeskResManagerCpp::getTrivalDancingActorByType(MyMJGameTrivalDancingTypeCpp type, bool freeActorOnly)
{
    const TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp>* pSubClass = m_mCfgTrivalDancingClasses.Find(type);
    if (pSubClass == NULL || !IsValid(*pSubClass) || (*pSubClass)->GetClass() == AMyMJGameTrivalDancingActorBaseCpp::StaticClass()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("subclass not correct for type: %s"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameTrivalDancingTypeCpp"), (uint8)type));
        MY_VERIFY(false);
        return NULL;
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

        if (pA->GetClass() != (*pSubClass)->GetClass()) {
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
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("%d dancing actors already exist, it is supposed to be small numer, type %s."), iDebugExitingThisType, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameTrivalDancingTypeCpp"), (uint8)type));
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor *parent = Cast<AActor>(GetOuter());
    MY_VERIFY(IsValid(parent));
    UWorld *w = parent->GetWorld();
    MY_VERIFY(IsValid(w));

    AMyMJGameTrivalDancingActorBaseCpp *pNewActor = w->SpawnActor<AMyMJGameTrivalDancingActorBaseCpp>(*pSubClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);
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
        AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(m_cCfgCardClass, FVector(0, 0, 50), FRotator(0, 0, 0), SpawnParams);
        //pNewCardActor->setResPathWithRet(m_cCfgCardResPath);

        MY_VERIFY(IsValid(pNewCardActor));
        pNewCardActor->SetActorHiddenInGame(true);
        MY_VERIFY(m_aCardActors.Emplace(pNewCardActor) == i);
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
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGameRoomCpp BeginPlay()"));

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
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("updateVisualData, mIdCardMap.Num() %d, role %d, pusherIdLast %d."), mIdCardMap.Num(), (uint8)cCoreData.getRole(), cCoreData.getCoreDataPublicRefConst().m_iPusherIdLast);

    float fDur = 1.f;
    if (uiSuggestedDur_ms > 0) {
        fDur = (float)uiSuggestedDur_ms / 1000;
    }
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
        pSeq->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        pCardActor->addTargetToGoHistory(cInfoAndResult);
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("actor %03d updated: %s."), idCard, *cInfoAndResult.genDebugString());
    }

    //Todo: handle other dirty data
};

void AMyMJGameRoomCpp::tipEventApplied(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                                        const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                        const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardChanged,
                                        const FMyMJEventWithTimeStampBaseCpp& cEvent)
{
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
                    showVisualTakeCards(idxAttender, aCardActors, (float)cEvent.getDuration_ms() / 1000, cCfgCache.m_cModelInfo.m_cCardModelInfo, cVisualPointForAttender);
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
                    showVisualGiveOutCards(idxAttender, attenderDataPublic, aCardActorsGiveOutForAttender, aCardActorsOtherForAttender, (float)cEvent.getDuration_ms() / 1000, cCfgCache.m_cModelInfo.m_cCardModelInfo, cVisualPointForAttender);
                }
            }
            else if (ePusherType == MyMJGamePusherTypeCpp::ActionWeave) {
 
                TArray<AMyMJGameCardBaseCpp*> aCardActorsWeavedForAttender;

                const FMyMJRoleDataAttenderPublicDeltaCpp* pAttenderPublicDelta = cDelta.getRoleDataAttenderPublicDeltaConst(idxAttender);
                MY_VERIFY(pAttenderPublicDelta);

                MY_VERIFY(pAttenderPublicDelta->m_aWeave2Add.Num() > 0);
                const FMyMJWeaveCpp& cWeave = pAttenderPublicDelta->m_aWeave2Add[0];
                const TArray<int32>& aIds = cWeave.getIdsRefConst();
                int32 l = aIds.Num();
                for (int32 i = 0; i < l; i++)
                {
                    AMyMJGameCardBaseCpp* pCardActor = m_pResManager->getCardActorByIdx(aIds[i]);
                    aCardActorsWeavedForAttender.Emplace(pCardActor);
                }


                if (bGotPointerCfgForAttender) {
                    MyMJGameRuleTypeCpp eRuleType = cCoreData.getCoreDataPublicRefConst().m_cGameCfg.m_eRuleType;
                    showVisualWeave(idxAttender, eRuleType, cWeave, aCardActorsWeavedForAttender, (float)cEvent.getDuration_ms() / 1000, cCfgCache.m_cModelInfo.m_cCardModelInfo, cVisualPointForAttender);
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


void AMyMJGameRoomCpp::showVisualTakeCards(int32 idxAttender, const TArray<AMyMJGameCardBaseCpp*>& cardActors, float totalDur, const FMyMJGameActorModelInfoBoxCpp& cardModelInfo, const FMyMJGameDeskVisualPointCfgCpp &visualPointForAttender)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperResolvePointOnPlayerScreenConstrainedMeta(this, visualPointForAttender.m_cCenterPointWorldTransform.GetLocation(), attenderOnScreenMeta);

    bool bMidLocOverride = false;
    FVector MidLocOverride;
    if (attenderOnScreenMeta.m_iIdxAttenderBelongTo == 0) {
        const FMyMJGameInGamePlayerScreenCfgCpp& cScreenCfg = m_pResManager->getInGamePlayerScreenCfgRefConst();
        int32 idxAttenderOnScreen = attenderOnScreenMeta.m_iIdxAttenderBelongTo;
        float fCenterToBorderPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_fShowPosiFromCenterToBorderPercent;
        float fVLengthOnScreenPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_fTargetVLengthOnScreenScreenPercent;
        const FVector2D& posiFix = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_cExtraOffsetScreenPercent;
        FTransform popTrans;
        UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(this, attenderOnScreenMeta, fCenterToBorderPercent, posiFix, fVLengthOnScreenPercent, cardModelInfo.m_cBoxExtend.Z * 2, popTrans);

        bMidLocOverride = true;
        MidLocOverride = popTrans.GetLocation();
    }

    float fUp2Add = cardModelInfo.m_cBoxExtend.Z * 2 * 1;
    
    float fDur0 = totalDur * 0.4;
    float fDur1 = totalDur * 0.4;
    float fDur2 = totalDur - fDur0 - fDur1;

    int32 l = cardActors.Num();

    for (int32 i = 0; i < l; i++) {
        AMyMJGameCardBaseCpp* pCardActor = cardActors[i];
        UMyTransformUpdateSequenceMovementComponent *pSeqComp = pCardActor->getTransformUpdateSequence();

        const FTransform& cT2Go = pCardActor->getTargetToGoHistory(0)->m_cVisualResult.m_cTransform;

        FVector cMidLoc = UKismetMathLibrary::GetUpVector(cT2Go.GetRotation().Rotator()) * fUp2Add + cT2Go.GetLocation();
        FTransform cMidTransform = cT2Go;
        cMidTransform.SetLocation(cMidLoc);

        if (bMidLocOverride) {
            cMidTransform.SetLocation(MidLocOverride);
        }

        pSeqComp->clearSeq();

        FTransformUpdateSequencDataCpp data;
        data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cMidTransform, fDur0);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        data.helperSetDataBySrcAndDst(cMidTransform, cMidTransform, fDur1);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        data.helperSetDataBySrcAndDst(cMidTransform, cT2Go, fDur2);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

    }
}

void AMyMJGameRoomCpp::showVisualGiveOutCards(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& attenderDataPublic, const TArray<AMyMJGameCardBaseCpp*>& cardActorsGiveOut, const TArray<AMyMJGameCardBaseCpp*>& cardActorsOtherMoving, float totalDur, const FMyMJGameActorModelInfoBoxCpp& cardModelInfo, const FMyMJGameDeskVisualPointCfgCpp &visualPointForAttender)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperResolvePointOnPlayerScreenConstrainedMeta(this, visualPointForAttender.m_cCenterPointWorldTransform.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGamePlayerScreenCfgCpp& cScreenCfg = m_pResManager->getInGamePlayerScreenCfgRefConst();
    int32 idxAttenderOnScreen = attenderOnScreenMeta.m_iIdxAttenderBelongTo;
    float fCenterToBorderPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_fShowPosiFromCenterToBorderPercent;
    float fVLengthOnScreenPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_fTargetVLengthOnScreenScreenPercent;
    const FVector2D& posiFix = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_cExtraOffsetScreenPercent;

    FTransform popTrans;
    UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(this, attenderOnScreenMeta, fCenterToBorderPercent, posiFix, fVLengthOnScreenPercent, cardModelInfo.m_cBoxExtend.Z * 2, popTrans);

    float fDur0 = totalDur * 0.1;
    float fDur1 = totalDur * 0.3;
    float fDur2 = totalDur * 0.4;
    float fDur3 = totalDur - fDur0 - fDur1 - fDur2;

    int32 l = cardActorsGiveOut.Num();

    for (int32 i = 0; i < l; i++) {
        AMyMJGameCardBaseCpp* pCardActor = cardActorsGiveOut[i];
        UMyTransformUpdateSequenceMovementComponent *pSeqComp = pCardActor->getTransformUpdateSequence();

        const FTransform& cT2Go = pCardActor->getTargetToGoHistory(0)->m_cVisualResult.m_cTransform;

        FTransform cMidTransform0, cMidTransform1;
        FTransformUpdateSequencDataCpp data;

        pSeqComp->clearSeq();

        cMidTransform0 = pCardActor->GetTransform();
        cMidTransform0.SetLocation(popTrans.GetLocation());
        data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cMidTransform0, fDur0);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        cMidTransform1 = cMidTransform0;
        cMidTransform1.SetRotation(cT2Go.GetRotation());
        data.helperSetDataBySrcAndDst(cMidTransform0, cMidTransform1, fDur1);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        data.helperSetDataBySrcAndDst(cMidTransform1, cMidTransform1, fDur2);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        data.helperSetDataBySrcAndDst(cMidTransform1, cT2Go, fDur3);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

    }


    l = cardActorsOtherMoving.Num();
    if (l >= 1) {

        fDur0 = totalDur * 0.4;
        fDur1 = totalDur * 0.1;
        fDur2 = totalDur * 0.4;
        fDur3 = totalDur - fDur0 - fDur1 - fDur2;

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
        TArray<AMyMJGameCardBaseCpp *> aActors0;
        for (int32 i = 0; i < l; i++) {
            AMyMJGameCardBaseCpp* pCardActor = cardActorsOtherMoving[i];

            const FMyMJGameCardVisualInfoAndResultCpp* oldTarget = pCardActor->getTargetToGoHistory(1, false);
            if (oldTarget == NULL) {
                continue;
            }

            if (oldTarget->m_cVisualInfo.m_eSlot == MyMJCardSlotTypeCpp::JustTaken &&
                pCardActor->getTargetToGoHistory(0)->m_cVisualInfo.m_iIdxColInRow < (attenderDataPublic.m_aIdHandCards.Num() - 1))
            {
                aActors0.Emplace(pCardActor);
            }
        }

        //Warn: scene related, it assume the hand card is in the left of give out card position, and assume the attender pointer cfg was facing the attender
        //TArray<AMyMJGameCardBaseCpp *>* paActos = aActors0.Num() < aActors1.Num() ? &aActors0 : &aActors1;
        TArray<AMyMJGameCardBaseCpp *>* paActos = &aActors0;

        float fUp2Add = cardModelInfo.m_cBoxExtend.Z * 2 * 1;
        l = paActos->Num();
        for (int32 i = 0; i < l; i++) {
            AMyMJGameCardBaseCpp* pCardActor = (*paActos)[i];

            UMyTransformUpdateSequenceMovementComponent *pSeqComp = pCardActor->getTransformUpdateSequence();

            const FTransform& cT2Go = pCardActor->getTargetToGoHistory(0)->m_cVisualResult.m_cTransform;

            FTransform cMidTransform0, cMidTransform1;
            FTransformUpdateSequencDataCpp data;

            pSeqComp->clearSeq();

            data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), pCardActor->GetTransform(), fDur0);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

            cMidTransform0 = pCardActor->GetTransform();
            FVector cMidLoc = UKismetMathLibrary::GetUpVector(cMidTransform0.GetRotation().Rotator()) * fUp2Add + cMidTransform0.GetLocation();
            cMidTransform0.SetLocation(cMidLoc);
            data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cMidTransform0, fDur1);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

            cMidTransform1 = cT2Go;
            cMidLoc = UKismetMathLibrary::GetUpVector(cMidTransform1.GetRotation().Rotator()) * fUp2Add + cMidTransform1.GetLocation();
            cMidTransform1.SetLocation(cMidLoc);
            data.helperSetDataBySrcAndDst(cMidTransform0, cMidTransform1, fDur2);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

            data.helperSetDataBySrcAndDst(cMidTransform1, cT2Go, fDur3);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        }
    }
}

void AMyMJGameRoomCpp::showVisualWeave(int32 idxAttender, MyMJGameRuleTypeCpp ruleType, const FMyMJWeaveCpp& weave, TArray<AMyMJGameCardBaseCpp*>& cardActorsWeaved, float totalDur, const FMyMJGameActorModelInfoBoxCpp& cardModelInfo, const FMyMJGameDeskVisualPointCfgCpp &visualPointForAttender)
{
    UMyCommonUtilsLibrary::invalidScreenDataCache();

    FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta attenderOnScreenMeta;
    UMyCardGameUtilsLibrary::helperResolvePointOnPlayerScreenConstrainedMeta(this, visualPointForAttender.m_cCenterPointWorldTransform.GetLocation(), attenderOnScreenMeta);

    const FMyMJGameInGamePlayerScreenCfgCpp& cScreenCfg = m_pResManager->getInGamePlayerScreenCfgRefConst();
    int32 idxAttenderOnScreen = attenderOnScreenMeta.m_iIdxAttenderBelongTo;
    float fCenterToBorderPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_fShowPosiFromCenterToBorderPercent;
    float fVLengthOnScreenPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_fTargetVLengthOnScreenScreenPercent;
    const FVector2D& posiFix = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_cExtraOffsetScreenPercent;

    FTransform popTransForCards;
    UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(this, attenderOnScreenMeta, fCenterToBorderPercent, posiFix, fVLengthOnScreenPercent, cardModelInfo.m_cBoxExtend.Z * 2, popTransForCards);

    float fDur0 = totalDur * 0.1;
    float fDur1 = totalDur * 0.3;
    float fDur2 = totalDur * 0.4;
    float fDur3 = totalDur - fDur0 - fDur1 - fDur2;

    if (cardActorsWeaved.Num() > 0) {
        cardActorsWeaved.Sort([](AMyMJGameCardBaseCpp& pA,AMyMJGameCardBaseCpp& pB) {
            return pA.getTargetToGoHistory(0, true)->m_cVisualInfo.m_iIdxColInRow < pB.getTargetToGoHistory(0, true)->m_cVisualInfo.m_iIdxColInRow;
        });

        int32 l = cardActorsWeaved.Num();
        const AMyMJGameCardBaseCpp* pCardActorCenter = cardActorsWeaved[(l / 2)];
        const FTransform& cT2GoCardActorCenter = pCardActorCenter->getTargetToGoHistory(0)->m_cVisualResult.m_cTransform;

        for (int32 i = 0; i < l; i++) {
            AMyMJGameCardBaseCpp* pCardActor = cardActorsWeaved[i];

            UMyTransformUpdateSequenceMovementComponent *pSeqComp = pCardActor->getTransformUpdateSequence();
            const FTransform& cT2Go = pCardActor->getTargetToGoHistory(0)->m_cVisualResult.m_cTransform;

            FTransform cMidTransform0, cMidTransform1;
            FTransformUpdateSequencDataCpp data;

            pSeqComp->clearSeq();
            
            FVector popLocation;
            if (pCardActor == pCardActorCenter) {
                popLocation = popTransForCards.GetLocation();
            }
            else {
                popLocation = popTransForCards.GetLocation() + (cT2Go.GetLocation() - cT2GoCardActorCenter.GetLocation());
            }
            cMidTransform0.SetLocation(popLocation);

            data.helperSetDataBySrcAndDst(pCardActor->GetTransform(), cMidTransform0, fDur0);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

            cMidTransform1 = cMidTransform0;
            cMidTransform1.SetRotation(cT2Go.GetRotation());
            data.helperSetDataBySrcAndDst(cMidTransform0, cMidTransform1, fDur1);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

            data.helperSetDataBySrcAndDst(cMidTransform1, cMidTransform1, fDur2);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

            data.helperSetDataBySrcAndDst(cMidTransform1, cT2Go, fDur3);
            pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("weave card actor num is zero."));
        MY_VERIFY(false);
    }

    {
        fDur0 = totalDur * 0.2;
        fDur1 = totalDur * 0.6;
        fDur2 = totalDur - fDur0 - fDur1;


        MyMJGameTrivalDancingTypeCpp eDancingType = UMyMJGameDeskResManagerCpp::helperGetWeaveDancingTypeFromWeave(ruleType, weave);
        MY_VERIFY(eDancingType != MyMJGameTrivalDancingTypeCpp::Invalid);

        AMyMJGameTrivalDancingActorBaseCpp* pDancingActor = m_pResManager->getTrivalDancingActorByType(eDancingType, true);
        pDancingActor->SetActorHiddenInGame(false);
        FMyMJGameActorModelInfoBoxCpp dancingActorModelInfo;
        MY_VERIFY(pDancingActor->getModelInfo(dancingActorModelInfo) == 0);

        float fCenterToBorderDancingPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCommonActionShowPoint.m_fShowPosiFromCenterToBorderPercent;
        float fVLengthOnScreenDancingPercent = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCommonActionShowPoint.m_fTargetVLengthOnScreenScreenPercent;
        const FVector2D& posiFixDancing = cScreenCfg.m_aAttenderAreas[idxAttenderOnScreen].m_cCardShowPoint.m_cExtraOffsetScreenPercent;


        FTransform popTransForDancingActor;
        UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(this, attenderOnScreenMeta, fCenterToBorderDancingPercent, posiFixDancing, fVLengthOnScreenDancingPercent, dancingActorModelInfo.m_cBoxExtend.Z * 2, popTransForDancingActor);

        FTransform popTransStartForDancingActor;
        UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(this, attenderOnScreenMeta, fCenterToBorderDancingPercent, posiFixDancing, 0.9, dancingActorModelInfo.m_cBoxExtend.Z * 2, popTransStartForDancingActor);

        FTransform popTransEndForDancingActor;
        UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(this, attenderOnScreenMeta, fCenterToBorderDancingPercent, posiFixDancing, 0.01, dancingActorModelInfo.m_cBoxExtend.Z * 2, popTransEndForDancingActor);

        pDancingActor->SetActorTransform(popTransStartForDancingActor);
        UMyTransformUpdateSequenceMovementComponent *pSeqComp = pDancingActor->getTransformUpdateSequence();

        pSeqComp->clearSeq();

        //FTransform cMidTransform0, cMidTransform1;
        FTransformUpdateSequencDataCpp data;

        data.helperSetDataBySrcAndDst(pDancingActor->GetTransform(), popTransForDancingActor, fDur0);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        data.helperSetDataBySrcAndDst(popTransForDancingActor, popTransForDancingActor, fDur1);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

        data.helperSetDataBySrcAndDst(popTransForDancingActor, popTransEndForDancingActor, fDur2);
        pSeqComp->addSeqToTail(data, UMyMJGameDeskResManagerCpp::getCurveVectorDefaultLinear());

    }

}