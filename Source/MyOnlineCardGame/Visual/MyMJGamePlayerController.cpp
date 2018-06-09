// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGamePlayerController.h"

#include "MyMJGameRoomViewerPawnBase.h"
#include "MyMJGameVisualInterfaces.h"
#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGameDeskVisualData.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Public/Blueprint/WidgetBlueprintLibrary.h"

AMyMJGamePlayerControllerCommunicationCpp::AMyMJGamePlayerControllerCommunicationCpp() : Super()
{
    bReplicates = true;
    bAlwaysRelevant = false;
    bOnlyRelevantToOwner = true; //subclass can change it
    bNetLoadOnClient = true;
    NetUpdateFrequency = 5;

    OldPawnMy = NULL;

    m_pExtRoomActor = NULL;
    m_pExtRoomTrivalDataSource = NULL;
    m_pExtRoomCoreDataSourceSeq = NULL;

    m_fHelperFilterLastRepClientRealtTime = 0;

    m_fLastAnswerSyncForMJCoreFullDataWorldRealTime = 0;
    m_bNeedAnswerSyncForMJCoreFullData = false;
    m_fLastAskSyncForMJCoreFullDataWorldRealTime = 0;
    m_bNeedAskSyncForMJCoreFullData = false;
    m_bNeedRetryFeedDataForCore = false;
    m_bUseAsLocalClientDataBridge = false;

    m_eDebugNetmodeAtStart = ENetMode::NM_MAX;
    m_bDebugHaltFeedData = false;

};

AMyMJGamePlayerControllerCommunicationCpp::~AMyMJGamePlayerControllerCommunicationCpp()
{

};

void AMyMJGamePlayerControllerCommunicationCpp::clearInGame()
{
    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }
};

void AMyMJGamePlayerControllerCommunicationCpp::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    if (UMyMJBPUtilsLibrary::haveClientVisualLayer(this) && IsLocalController()) {
        OnRep_Pawn();
    }
}

void AMyMJGamePlayerControllerCommunicationCpp::UnPossess()
{
    Super::UnPossess();

    if (UMyMJBPUtilsLibrary::haveClientVisualLayer(this) && IsLocalController()) {
        OnRep_Pawn();
    }
}

void AMyMJGamePlayerControllerCommunicationCpp::OnRep_Pawn()
{
    APawn* pNewPawn = GetPawn();

    if (!IsLocalController())
    {
        int32 id = UGameplayStatics::GetPlayerControllerID(this);
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not a local player controller: id %d, pc %p."), id, this);
    }

    // Detect when pawn changes, so we can NULL out the controller on the old pawn
    if (pNewPawn != OldPawnMy.Get())
    {
        onPawnChanged(OldPawnMy.Get(), pNewPawn);
        OldPawnMy = pNewPawn;
    }

    Super::OnRep_Pawn();
}

void AMyMJGamePlayerControllerCommunicationCpp::BeginPlay()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyMJGamePlayerControllerCommunicationCpp BeginPlay()"));

    Super::BeginPlay();

    if (UMyMJBPUtilsLibrary::haveServerLogicLayer(this)) {
        //In most case, this equals netmode == DS, LS, standalone

        //warn: we forbid the using local multiple player mode!
        m_bUseAsLocalClientDataBridge = IsLocalPlayerController();

        resetupWithViewRoleAndAuth(m_eDataRoleType, m_bUseAsLocalClientDataBridge);
    }
    else {
        m_bUseAsLocalClientDataBridge = false;
    }

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &AMyMJGamePlayerControllerCommunicationCpp::loop, (float)MyMJGamePlayerControllerLoopTimeMs / 1000);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
        MY_VERIFY(false);
    }

    m_eDebugNetmodeAtStart = GetNetMode();
};

void AMyMJGamePlayerControllerCommunicationCpp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    clearInGame();
};

void AMyMJGamePlayerControllerCommunicationCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyMJGamePlayerControllerCommunicationCpp, m_pExtRoomActor);
    DOREPLIFETIME(AMyMJGamePlayerControllerCommunicationCpp, m_pExtRoomTrivalDataSource);
    DOREPLIFETIME(AMyMJGamePlayerControllerCommunicationCpp, m_pExtRoomCoreDataSourceSeq);
};

void AMyMJGamePlayerControllerCommunicationCpp::onPawnChanged(APawn* oldPawn, APawn* newPawn)
{
    if (IsValid(oldPawn))
    {
        //we can't assume OldPawnMy->Controller == this since that property may be replicated already before
        /*
        if (OldPawnMy->Controller == this)
        {
        IMyPawnInterfaceCpp* pI = Cast<IMyPawnInterfaceCpp>(OldPawnMy.Get());
        if (pI) {
        pI->OnUnPossessedByLocalPlayerController(this);
        }
        }
        else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("oldpawn's controller is not this, unexpected, old's %p, this %p."), OldPawnMy->Controller, this);
        }
        */

        IMyPawnInterfaceCpp* pI = Cast<IMyPawnInterfaceCpp>(oldPawn);
        if (pI) {
            pI->OnUnPossessedByLocalPlayerController(this);
        }
    }

    if (IsValid(newPawn))
    {
        IMyPawnInterfaceCpp* pI = Cast<IMyPawnInterfaceCpp>(newPawn);
        if (pI) {
            pI->OnPossessedByLocalPlayerController(this);
        }
    }
}

bool AMyMJGamePlayerControllerCommunicationCpp::resetupWithViewRoleAndAuth(MyMJGameRoleTypeCpp eRoleType, bool bUseAsLocalClientDataBridge)
{
    MY_VERIFY(HasAuthority());

    AMyMJGameRoomRootActorCpp *pRS = AMyMJGameRoomLevelScriptActorCpp::helperGetRoomRootActor(this);

    /*
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return false;
    }

    ULevel *pL = world->PersistentLevel;
    if (!IsValid(pL)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("persisten level is invalid, %p."), pL);
        return false;
    }

    ALevelScriptActor* pLSA = pL->GetLevelScriptActor();

    if (!IsValid(pLSA)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ALevelScriptActoris invalid, %p."), pLSA);
        return false;
    }

    AMyMJGameRoomLevelScriptActorCpp *pLSASub = Cast<AMyMJGameRoomLevelScriptActorCpp>(pLSA);
    */

    if (!IsValid(pRS)) {
        return false;
    }

    m_pExtRoomActor = pRS->m_pRoomActor;
    if (!IsValid(m_pExtRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("room is invalid, not cfged?, %p."), m_pExtRoomActor);
        return false;
    }

    m_pExtRoomTrivalDataSource = pRS->m_pTrivalDataSource;
    if (!IsValid(m_pExtRoomTrivalDataSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomTrivalDataSource is invalid, not cfged?, %p."), m_pExtRoomTrivalDataSource);
        return false;
    }

    //at last, update the sequence
    if ((uint8)eRoleType >= (uint8)MyMJGameRoleTypeCpp::Max) {
        m_pExtRoomCoreDataSourceSeq = NULL;
        return true;
    }

    //return false; //test

    AMyMJGameCoreDataSourceCpp* pSource = pRS->m_pCoreDataSource;
    if (!IsValid(pSource)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data source is invalid, not cfged?, %p."), pSource);
        return false;
    }

    m_pExtRoomCoreDataSourceSeq = pSource->getMJDataAll()->getDataByRoleType(eRoleType, false);
    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomCoreDataSourceSeq is invalid, not cfged?, %p, role %d."), m_pExtRoomCoreDataSourceSeq, (uint8)eRoleType);
        MY_VERIFY(false);
    }

    if (bUseAsLocalClientDataBridge) {
        m_pExtRoomCoreDataSourceSeq->m_cReplicateDelegate.Clear();
        m_pExtRoomCoreDataSourceSeq->m_cReplicateDelegate.AddUObject(this, &AMyMJGamePlayerControllerCommunicationCpp::tryFeedDataToConsumerWithFilter);
    }

    return true;
};

void AMyMJGamePlayerControllerCommunicationCpp::loop()
{
    ENetMode mode = GetNetMode();
    if (mode != m_eDebugNetmodeAtStart) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("net mode change detected: %d -> %d"), (uint8)m_eDebugNetmodeAtStart, (uint8)mode);
    }

    //network traffic handle
    if (UMyMJBPUtilsLibrary::haveServerNetworkLayer(this)) {
        //we have network traffic to handle as network server
        loopOfSyncForMJCoreFullDataOnNetworkServer();
    }
    else if (UMyMJBPUtilsLibrary::haveClientNetworkLayer(this))
    {
        loopOfSyncForMJCoreFullDataOnNetworkClient();
    }
};

void AMyMJGamePlayerControllerCommunicationCpp::loopOfSyncForMJCoreFullDataOnNetworkServer()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    //server
    if (GetRemoteRole() == ENetRole::ROLE_None) {
        //not a network mode
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("remote role not exist, maybe connection lost?"));
        return;
    }

    if (m_bNeedAnswerSyncForMJCoreFullData && (nowRealTime - m_fLastAnswerSyncForMJCoreFullDataWorldRealTime) >= ((float)MyMJGamePlayerControllerAnswerSyncGapTimeMs / 1000)) {
        if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data sequence is invalid, not cfged?, %p."), m_pExtRoomCoreDataSourceSeq);
            return;
        }

        answerSyncForMJCoreFullDataOnClient(m_pExtRoomCoreDataSourceSeq->getRole(), m_pExtRoomCoreDataSourceSeq->getFullData());

        m_bNeedAnswerSyncForMJCoreFullData = false;
        m_fLastAnswerSyncForMJCoreFullDataWorldRealTime = nowRealTime;
    }
};

void AMyMJGamePlayerControllerCommunicationCpp::loopOfSyncForMJCoreFullDataOnNetworkClient()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    if (m_bNeedAskSyncForMJCoreFullData && (nowRealTime - m_fLastAskSyncForMJCoreFullDataWorldRealTime) >= ((float)MyMJGamePlayerControllerAskSyncGapTimeMs / 1000)) {
        if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("core data sequence is invalid, not cfged?, %p."), m_pExtRoomCoreDataSourceSeq);
            return;
        }

        askSyncForMJCoreFullDataOnServer();

        m_bNeedAskSyncForMJCoreFullData = false;
        m_fLastAskSyncForMJCoreFullDataWorldRealTime = nowRealTime;
    }

    if (m_bNeedRetryFeedDataForCore) {
        tryFeedDataToConsumer();
    }
}

void AMyMJGamePlayerControllerCommunicationCpp::setDataRoleTypeWithAuth(MyMJGameRoleTypeCpp eRoleType)
{
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("can't set Role Type without auth."));
        return;
    }

    if (eRoleType >= MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRoleType not valid"));
        return;
    }

    if (m_pExtRoomCoreDataSourceSeq == NULL || m_eDataRoleType != eRoleType) {
        //update the pointer
        resetupWithViewRoleAndAuth(eRoleType, m_bUseAsLocalClientDataBridge);
    }

    m_eDataRoleType = eRoleType;
};

void AMyMJGamePlayerControllerCommunicationCpp::setCmdRoleTypeWithAuth(MyMJGameRoleTypeCpp eRoleType)
{
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("can't set Role Type without auth."));
        return;
    }

    if (eRoleType >= MyMJGameRoleTypeCpp::Max) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRoleType not valid"));
        return;
    }

    m_eCmdRoleType = eRoleType;
}


void AMyMJGamePlayerControllerCommunicationCpp::askSyncForMJCoreFullDataOnServer_Implementation()
{
    //FPlatformProcess::Sleep(v1);
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return;
    }

    markNeedAnswerSyncForMJCoreFullData();
}

bool AMyMJGamePlayerControllerCommunicationCpp::askSyncForMJCoreFullDataOnServer_Validate()
{
    if (!HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return false;
    }

    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtRoomCoreDataSourceSeq not valid, %p"), m_pExtRoomCoreDataSourceSeq);
        return false;
    }

    return true;

}

void AMyMJGamePlayerControllerCommunicationCpp::answerSyncForMJCoreFullDataOnClient_Implementation(MyMJGameRoleTypeCpp eRole, const FMyMJDataStructWithTimeStampBaseCpp& cFullData)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("answerSyncForMJCoreFullDataOnClient_Implementation()!"));

    //FPlatformProcess::Sleep(v1);
    if (HasAuthority()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("authority wrong!"));
        return;
    }

    if (!IsValid(m_pExtRoomCoreDataSourceSeq)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pExtCoreDataSeq not valid, %p"), m_pExtRoomCoreDataSourceSeq);
        return;
    }

    m_pExtRoomCoreDataSourceSeq->tryUpdateFullDataFromExternal(eRole, cFullData);
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("full data updated!"));
    if (tryFeedDataToConsumer()) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("full data updated but still need sync, need to note."));
    }
}


void AMyMJGamePlayerControllerCommunicationCpp::makeSelection_Implementation(const FMyMJGameCmdMakeSelectionCpp& cmdMakeSelection)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("debug makeSelection_Implementation with m_eCmdRoleType %d."), (int32)m_eCmdRoleType);

    AMyMJGameCoreDataSourceCpp* pCoreDataSource = AMyMJGameRoomLevelScriptActorCpp::helperGetRoomRootActor(this, true)->getCoreDataSource(true);


    FMyMJGameCmdMakeSelectionCpp *pCmdMakeSelectionNew = new FMyMJGameCmdMakeSelectionCpp();
    *pCmdMakeSelectionNew = cmdMakeSelection;

    pCoreDataSource->getCmdInputQueueByRole(m_eCmdRoleType, true)->Enqueue(pCmdMakeSelectionNew);
}

bool AMyMJGamePlayerControllerCommunicationCpp::makeSelection_Validate(const FMyMJGameCmdMakeSelectionCpp& cmdMakeSelection)
{
    return true;
}


void AMyMJGamePlayerControllerCommunicationCpp::tryFeedDataToConsumerWithFilter()
{
    UWorld *world = GetWorld();
    if (!IsValid(world)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check outer settings!"));
        return;
    }
    float nowRealTime = world->GetRealTimeSeconds();

    if (nowRealTime <= m_fHelperFilterLastRepClientRealtTime) {
        return;
    }

    m_fHelperFilterLastRepClientRealtTime = nowRealTime;

    //ENetMode mode = GetNetMode();
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("OnRep_NewDataArrivedWithFilter, ENetMode %d, NM_Standalone %d"), (uint8)mode, (uint8)ENetMode::NM_Standalone);
    
    tryFeedDataToConsumer();
};


bool AMyMJGamePlayerControllerCommunicationCpp::tryFeedDataToConsumer()
{
    if (m_bDebugHaltFeedData) {
        return false;
    }

    if (!IsValid(m_pExtRoomActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("m_pExtRoomActor is not valid now. %p."), m_pExtRoomActor);
        return false;
    }

    bool bRet = false;
    if (IsValid(m_pExtRoomCoreDataSourceSeq)) {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("feeding data %d."), m_bDebugHaltFeedData);
        bRet = m_pExtRoomActor->getRoomDataSuiteVerified()->getDeskDataObjVerified()->tryFeedData(m_pExtRoomCoreDataSourceSeq, &m_bNeedRetryFeedDataForCore);
    }

    if (bRet) {
        markNeedAskSyncForMJCoreFullData();
    }

    return bRet;
};



void UMyMJGameUIManagerCpp::reset()
{
    changeUIMode(MyMJGameUIModeCpp::Invalid);

    if (IsValid(m_pInRoomUIMain))
    {
        m_pInRoomUIMain->RemoveFromParent();
        m_pInRoomUIMain = NULL;
    }
};

void UMyMJGameUIManagerCpp::changeUIMode(MyMJGameUIModeCpp UIMode)
{
    if (m_eUIMode != UIMode)
    {
        //change old
        if (m_eUIMode == MyMJGameUIModeCpp::InRoomPlay) {
            UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = getInRoomUIMain(false, false);
            if (pUI) {
                pUI->RemoveFromParent();
            }
        }

        if (UIMode == MyMJGameUIModeCpp::InRoomPlay) {
            UMyMJGameInRoomUIMainWidgetBaseCpp* pUI = getInRoomUIMain(true, false);
            if (pUI && !pUI->GetIsVisible()) {
                pUI->AddToPlayerScreen();
            }
        }

        m_eUIMode = UIMode;
    }
};

UMyMJGameInRoomUIMainWidgetBaseCpp* UMyMJGameUIManagerCpp::getInRoomUIMain(bool createIfNotExist, bool verify)
{
    AActor *pOwner = this->GetOwner();
    MY_VERIFY(pOwner != NULL);
    AMyMJGamePlayerControllerCpp *pPC = Cast<AMyMJGamePlayerControllerCpp>(pOwner);
    if (!IsValid(pPC)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("owner is supposed to be AMyMJGamePlayerControllerCpp but it is %s, %p."), *pOwner->GetClass()->GetName(), pOwner);
        MY_VERIFY(false);
    }
    if (!pPC->IsLocalController())
    {
        int32 id = UGameplayStatics::GetPlayerControllerID(pPC);
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("player controller is not local one, not supposed to happen, %p, id %d."), pPC, id);

        if (verify)
        {
            MY_VERIFY(false);
        }

        return NULL;
    }

    while (!IsValid(m_pInRoomUIMain) && createIfNotExist)
    {
        if (!IsValid(pPC->m_pExtRoomActor)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("room is invalid, not cfged?, %p."), pPC->m_pExtRoomActor);
            break;
        }

        const UMyMJGameInRoomVisualCfgCpp* pCfg = pPC->m_pExtRoomActor->getResManagerVerified()->getVisualCfg(false);
        if (pCfg == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cfg object is not specified."));
            break;
        }

        //getVisualCfg()->m_cMainActorClassCfg.checkSettings();
        if (pCfg->m_cMainActorClassCfg.checkSettings())
        {
            const TSubclassOf<UMyMJGameInRoomUIMainWidgetBaseCpp>& widgetClass = pCfg->m_cUICfg.m_cInRoomUIMainWidgetClass;
            m_pInRoomUIMain = Cast<UMyMJGameInRoomUIMainWidgetBaseCpp>(UWidgetBlueprintLibrary::Create(this, widgetClass, pPC)); //no player controller owns it but let this class manage it
            MY_VERIFY(IsValid(m_pInRoomUIMain));
            m_pInRoomUIMain->setDataSourceRoomActor(AMyMJGameRoomLevelScriptActorCpp::helperGetRoomActor(this, true));
        }

        break;
    }

    if (!IsValid(m_pInRoomUIMain))
    {
        if (createIfNotExist) {
            //This is an error case
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pInRoomUIMain is invalid: %p."), m_pInRoomUIMain);
        }
        if (verify) {
            MY_VERIFY(false);
        }
    }

    return m_pInRoomUIMain;
};


void UMyMJGameSelectManagerCpp::addSelectedActor(AActor *pActor)
{
    if (getSelectedActorNumMax() <= 0) {
        return;
    }

    int32 existingIdx;
    if (m_aSelectedActors.Find(pActor, existingIdx)) {
        return;
    }


    IMySelectableObjectInterfaceCpp* pSI = Cast<IMySelectableObjectInterfaceCpp>(pActor);
    if (!pSI) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: does not implement IMySelectableObjectInterfaceCpp."), *pActor->GetName());
    }

    //add it
    m_aSelectedActors.Emplace(pActor);

    if (pSI) {
        pSI->setSelected(true);
    }

    removeOutOfBoundSelectedActors();
};

void UMyMJGameSelectManagerCpp::removeSelectedActor(AActor *pActor)
{
    int32 existingIdx;
    if (!m_aSelectedActors.Find(pActor, existingIdx)) {
        return;
    }

    m_aSelectedActors.RemoveAt(existingIdx);

    IMySelectableObjectInterfaceCpp* pSI = Cast<IMySelectableObjectInterfaceCpp>(pActor);
    if (pSI) {
        pSI->setSelected(false);
    }
};

void UMyMJGameSelectManagerCpp::clearSelectedActors()
{
    int32 l = m_aSelectedActors.Num();

    for (int32 i = 0; i < l; i++) {
        if (IsValid(m_aSelectedActors[i])) {
            IMySelectableObjectInterfaceCpp* pSI = Cast<IMySelectableObjectInterfaceCpp>(m_aSelectedActors[i]);
            if (pSI) {
                pSI->setSelected(false);
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("clear selected actors but actor is invalid: %p."), m_aSelectedActors[i]);
        }
    }

    m_aSelectedActors.Reset();
};

void UMyMJGameSelectManagerCpp::setSelectedActorNumMax(int32 iSelectedActorNumMax, bool clearSelected)
{
    if (clearSelected) {
        clearSelectedActors();
    }

    m_iSelectedActorNumMax = iSelectedActorNumMax;

    removeOutOfBoundSelectedActors();
}

void UMyMJGameSelectManagerCpp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    clearSelectedActors();
};

void UMyMJGameSelectManagerCpp::removeOutOfBoundSelectedActors()
{
    int32 cardNumToRemove = m_aSelectedActors.Num() - m_iSelectedActorNumMax;
    if (cardNumToRemove > 0) {

        for (int32 i = 0; i < cardNumToRemove; i++) {
            IMySelectableObjectInterfaceCpp* pSI = Cast<IMySelectableObjectInterfaceCpp>(m_aSelectedActors[i]);
            if (pSI) {
                pSI->setSelected(false);
            }
        }
        m_aSelectedActors.RemoveAt(0, cardNumToRemove);

    }
};


AMyMJGamePlayerControllerCpp::AMyMJGamePlayerControllerCpp() : Super()
{
    m_pUIManager = CreateDefaultSubobject<UMyMJGameUIManagerCpp>(TEXT("UI manager"));
    m_pSelectManager = CreateDefaultSubobject<UMyMJGameSelectManagerCpp>(TEXT("Select manager"));

    m_iViewRoleWhenNotAttend = -1;


    bEnableTouchEvents = bEnableTouchOverEvents = bEnableClickEvents = bEnableMouseOverEvents = false;

    m_eDragEndActionType = MyMJGamePlayerControllerDragEndActionTypeCpp::Invalid;
};

AMyMJGamePlayerControllerCpp::~AMyMJGamePlayerControllerCpp()
{

};

void AMyMJGamePlayerControllerCpp::clearInGame()
{
    Super::clearInGame();

    m_pUIManager->reset();
};

void AMyMJGamePlayerControllerCpp::resetCameraAndUI()
{
    int32 viewRole = (int32)m_eDataRoleType;
    if (viewRole < 0 || viewRole >= 4) {
        if (m_iViewRoleWhenNotAttend >= 0 && m_iViewRoleWhenNotAttend < 4) {
            viewRole = m_iViewRoleWhenNotAttend;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_iViewRoleWhenNotAttend not valid and data role not attender, using default role now. m_eDataRoleType %d, "), (int32)m_eDataRoleType, m_iViewRoleWhenNotAttend);
            viewRole = 0;
        }
    }

    GetPawn();
};

AMyMJGameRoomViewerPawnBaseCpp* AMyMJGamePlayerControllerCpp::helperGetRoomViewerPawn(bool verify)
{
    AMyMJGameRoomViewerPawnBaseCpp* ret = NULL;
    while (1) {
        APawn *pPawn = GetPawn();
        if (!IsValid(pPawn)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("possed pawn is invalid: %p."), pPawn);
            break;
        }

        ret = Cast<AMyMJGameRoomViewerPawnBaseCpp>(pPawn);
        if (!IsValid(ret)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("possed pawn is not a AMyMJGameRoomViewerPawnBaseCpp, class: %s."), *pPawn->GetClass()->GetName());
            ret = NULL;
            break;
        }
        break;
    }

    if (verify) {
        MY_VERIFY(ret);
    }

    return NULL;
}

AMyMJGamePlayerControllerCpp* AMyMJGamePlayerControllerCpp::helperGetLocalController(const UObject* WorldContextObject)
{
    APlayerController *pC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    MY_VERIFY(pC != NULL);

    AMyMJGamePlayerControllerCpp *pRet = Cast<AMyMJGamePlayerControllerCpp>(pC);
    if (!IsValid(pRet)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("player controller class not currect, %p, class %s."), pRet, *pC->GetClass()->GetName());
        MY_VERIFY(false);
    }

    return pRet;
};

UMyMJGameInRoomUIMainWidgetBaseCpp* AMyMJGamePlayerControllerCpp::helperGetInRoomUIMain(const UObject* WorldContextObject, bool verify)
{
    return helperGetLocalController(WorldContextObject)->getUIManagerVerified()->getInRoomUIMain(false, verify);
};

void AMyMJGamePlayerControllerCpp::SetupInputComponent()
{
    Super::SetupInputComponent();

    MY_VERIFY(InputComponent);

    InputComponent->BindTouch(IE_Pressed, this, &AMyMJGamePlayerControllerCpp::OnTouchBegin);
    InputComponent->BindTouch(IE_Released, this, &AMyMJGamePlayerControllerCpp::OnTouchEnd);
    InputComponent->BindTouch(IE_Repeat, this, &AMyMJGamePlayerControllerCpp::OnFingerMove);

    m_cLastTouchDragLocation = FVector2D::ZeroVector;
};


void AMyMJGamePlayerControllerCpp::OnTouchBegin(ETouchIndex::Type FingerIndex, FVector Location)
{
    if (FingerIndex != ETouchIndex::Touch1)
    {
        //only support one touch now, so mouse can do anything
        return;
    }

    if (bEnableTouchEvents || bEnableTouchOverEvents || bEnableClickEvents || bEnableMouseOverEvents) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: we use custom touch code, so default touch or mouse code path should be disabled for performance, current setting: bEnableTouchEvents %d, bEnableTouchOverEvents %d, bEnableClickEvents %d, bEnableMouseOverEvents %d."), *GetName(), bEnableTouchEvents, bEnableTouchOverEvents, bEnableClickEvents, bEnableMouseOverEvents);
        return;
    }

    UMyMJGameSelectManagerCpp* pSM = getSelectManagerVerified();
    if (pSM->getSelectedActorNumMax() <= 0) {
        return;
    }

    m_cLastTouchDragLocation = FVector2D(Location);

    FHitResult HitResult;
    const bool bHit = GetHitResultAtScreenPosition(m_cLastTouchDragLocation, CurrentClickTraceChannel, true, HitResult);

    UPrimitiveComponent* CurrentComponent = (bHit ? HitResult.Component.Get() : NULL);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("CurrentComponent %p."), CurrentComponent);

    AActor* hittedSelectableActor = NULL;
    while (CurrentComponent) {
        AActor* pA = CurrentComponent->GetOwner();
        if (!IsValid(pA)) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("hittedActor pA not valid: %p."), pA);
            break;
        }

        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("hittedActor pA %s."), *pA->GetName());

        IMySelectableObjectInterfaceCpp* pSI = Cast<IMySelectableObjectInterfaceCpp>(pA);
        if (!pSI) {
            break;
        }

        hittedSelectableActor = pA;
        break;
    }

    if (hittedSelectableActor) {
        while (1) {

            IMySelectableObjectInterfaceCpp* pSI = Cast<IMySelectableObjectInterfaceCpp>(hittedSelectableActor);
            if (!pSI) {
                break;
            }

            bool bSelectable = false;
            bool bSelected = false;
            if (pSI->getIsSelectable(bSelectable) != MyErrorCodeCommonPartCpp::NoError || pSI->getSelected(bSelected) != MyErrorCodeCommonPartCpp::NoError) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: get error when callling getIsSelectable() and getSelected()."), *hittedSelectableActor->GetName());
                break;
            }

            if (!bSelectable) {
                break;
            }

            getSelectManagerVerified()->addSelectedActor(hittedSelectableActor);

            IMyDraggableObjectInterfaceCpp* pDI = Cast<IMyDraggableObjectInterfaceCpp>(hittedSelectableActor);
            if (!pDI) {
                break;
            }

            pDI->markBeginDrag();

            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("hitted actor handled: %s."), *hittedActor->GetName());

            break;
        }

    }
    else {
        getSelectManagerVerified()->clearSelectedActors();
    }
};

void AMyMJGamePlayerControllerCpp::OnTouchEnd(ETouchIndex::Type FingerIndex, FVector Location)
{
    if (FingerIndex == ETouchIndex::Touch1)
    {
        m_cLastTouchDragLocation = FVector2D::ZeroVector;

        int32 idxAttender = (int32)m_eCmdRoleType;
        bool bCanGiveCmd = idxAttender >= 0 && idxAttender < 4;

        if (m_eDragEndActionType == MyMJGamePlayerControllerDragEndActionTypeCpp::GiveOutCards) {

            TArray<AMyMJGameCardActorBaseCpp*> aGiveOutCards;
            bool bHaveCardsGiveOut = false;

            FTransform transform;
            bool projectionOK;
            FVector2D projectedScreenPosition;
            float projectedDistance;

            FMyModelInfoWorld3DCpp cardModelInfo;
            float fDragOutSizeOffsetPercent = 1;
            bool cardModelInfoGot = false;

            const TArray<AActor *>& aSelectedActors = m_pSelectManager->getSelectedActorsRefConst();
            int32 l = aSelectedActors.Num();
            for (int32 i = 0; i < l; i++) {
                AActor* pActor = aSelectedActors[i];
                AMyMJGameCardActorBaseCpp* pCard = Cast<AMyMJGameCardActorBaseCpp>(pActor);
                if (!IsValid(pCard)) {
                    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("want to giveout card but selected actor is not valid card: %p, %s."), pCard, *pActor->GetName());
                    continue;
                }

                MyErrorCodeCommonPartCpp ret = pCard->getDataBeginDrag(transform, projectionOK, projectedScreenPosition, projectedDistance);

                if (MyErrorCodeCommonPartCpp::NoError != ret) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getDataBeginDrag() returned error as %s."),
                              *pCard->GetName(),
                              *UMyCommonUtilsLibrary::Conv_MyErrorCodeCommonPartCpp_String(ret));
                    continue;
                }

                FVector movedLocation = pCard->GetActorLocation() - transform.GetLocation();

                if (!cardModelInfoGot) {
                    cardModelInfo = pCard->getModelInfoForUpdaterEnsured();
                    fDragOutSizeOffsetPercent = pCard->getDragOutSizeOffsetPercent();
                    cardModelInfoGot = true;
                }

                float modelBoxSize = cardModelInfo.getBox3DRefConst().m_cBoxExtend.Size();
                float movedLocationSize = movedLocation.Size();

                if (movedLocationSize >= modelBoxSize * fDragOutSizeOffsetPercent) {
                    bHaveCardsGiveOut = true;
                }

                aGiveOutCards.Emplace(pCard);
            }

            if (bHaveCardsGiveOut) {
                AMyMJGameRoomCpp* pRoom = AMyMJGameRoomLevelScriptActorCpp::helperGetRoomActor(this, false);
                if (IsValid(pRoom)) {
                    const FMyMJGameDeskVisualDataCpp& dataNow = pRoom->getRoomDataSuiteVerified()->getDeskDataObjVerified()->getDataAllRefConst().m_cDeskVisualDataNow;
                    const FMyMJDataStructWithTimeStampBaseCpp& coreData = dataNow.getCoreDataRefConst();
                    const FMyMJRoleDataAttenderPrivateCpp& dataPrivate = coreData.getRoleDataAttenderPrivateRefConst(idxAttender);

                    if (dataPrivate.m_cActionContainor.m_aActionChoices.Num() > 0) {
                        const FMyMJGameActionUnfiedForBPCpp& actionUnified0 = dataPrivate.m_cActionContainor.m_aActionChoices[0];
                        if (actionUnified0.getType() == MyMJGamePusherTypeCpp::ActionGiveOutCards) {
                            FMyMJGameCmdMakeSelectionCpp cmdMakeSelection;
                            cmdMakeSelection.m_iGameId = coreData.getCoreDataPublicRefConst().m_iGameId;
                            cmdMakeSelection.m_iIdxAttender = idxAttender;
                            cmdMakeSelection.m_iActionGroupId = coreData.getCoreDataPublicRefConst().m_iActionGroupId;
                            cmdMakeSelection.m_iSelection = 0;

                            for (int32 idxCard = 0; idxCard < aGiveOutCards.Num(); idxCard++) {
                                int32 cardId = aGiveOutCards[idxCard]->getMyId2();
                                if (cardId >= 0) {
                                    cmdMakeSelection.m_aSubSelections.Emplace(cardId);
                                }
                                else {
                                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("want giveout cards but cardId < 0: %s: %d."), *aGiveOutCards[idxCard]->GetName(), cardId);
                                }
                            }

                            makeSelection(cmdMakeSelection);
                        }
                        else {
                            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("want giveout cards but actionChoices0 is not GiveOutCards, it is %s."), *FMyMJGamePusherBaseCpp::helperPusherTypeToString(actionUnified0.getType()));
                        }
                    }
                    else {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("want giveout cards but actionChoices num is 0"));
                    }

                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pRoom not valid: %p."), pRoom);
                }
            }

        }


    }
};

void AMyMJGamePlayerControllerCpp::OnFingerMove(ETouchIndex::Type FingerIndex, FVector Location)
{
    if ((FingerIndex == ETouchIndex::Touch1) && (!m_cLastTouchDragLocation.IsZero()))
    {
        float TouchDragRotationScale = 1;
        FVector2D const DragDelta = (FVector2D(Location) - m_cLastTouchDragLocation) * TouchDragRotationScale;

        FTransform transformBeginSelection;
        bool projectionOKBeginSelection;
        FVector2D projectedScreenPositionBeginSelection;
        float projectedDistanceBeginSelection;

        const TArray<AActor *>& aSelectedActors = m_pSelectManager->getSelectedActorsRefConst();
        int32 l = aSelectedActors.Num();
        for (int32 i = 0; i < l; i++) {
            AActor* pActor = aSelectedActors[i];

            IMyDraggableObjectInterfaceCpp* pDI = Cast<IMyDraggableObjectInterfaceCpp>(pActor);
            if (!pDI) {
                UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("%s: clicked actor have not implement IMyDraggableObjectInterfaceCpp."), *pActor->GetName());
                continue;
            }

            MyErrorCodeCommonPartCpp ret = pDI->getDataBeginDrag(transformBeginSelection, projectionOKBeginSelection, projectedScreenPositionBeginSelection, projectedDistanceBeginSelection);
            if (ret != MyErrorCodeCommonPartCpp::NoError || projectionOKBeginSelection == false) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: failed get projection data at drag begin: %d, %d."), *pActor->GetName(), (int32)ret, projectionOKBeginSelection);
                continue;
            }

            FVector2D actorScreenPositionNow = projectedScreenPositionBeginSelection + DragDelta;

            FVector worldPosition, worldDirection;
            if (!UGameplayStatics::DeprojectScreenToWorld(this, actorScreenPositionNow, worldPosition, worldDirection)) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: failed deproject back, screen position %s."), *pActor->GetName(), *actorScreenPositionNow.ToString());
                continue;
            }

            FVector actorLocationNow = worldPosition + worldDirection * projectedDistanceBeginSelection;
            pActor->SetActorLocation(actorLocationNow);

            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("selected actor handled: %s."), *pActor->GetName());
        }

        //LastTouchDragLocation = FVector2D(Location);
    }
};
