// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJBPUtils.h"

#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

void AMyTestActorBaseCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

};

bool AMyTestActorBaseCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    const TSet<UActorComponent*>& aC = GetReplicatedComponents();
    for (UActorComponent* ActorComp : aC)
    {
        FString n = ActorComp->GetFName().ToString();
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("comopnent name %s."), *n);
    };

    bool bRet = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("ReplicateSubobjects done."));

    return bRet;
};



void AMyTestActorBaseCpp::createMJData()
{

};

void AMyTestActorBaseCpp::ClientRPCFunction0_Implementation(float v1)
{
    FPlatformProcess::Sleep(v1);
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ClientRPCFunction0_Implementation %f"), v1);
}

bool AMyTestActorBaseCpp::ServerRPCFunction0_Validate(float v1)
{
    return true;
}

void AMyTestActorBaseCpp::ServerRPCFunction0_Implementation(float v1)
{
    FPlatformProcess::Sleep(v1);
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ServerRPCFunction0_Implementation %f"), v1);
}

void AMyTestActorBaseCpp::MulticastRPCFunction0_Implementation(float v1)
{
    FPlatformProcess::Sleep(v1);
    UWorld *world = GetWorld();

    uint8 netMode = GEngine->GetNetMode(world);
    float timeSeconds = world-> GetTimeSeconds();

    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("MulticastRPCFunction0_Implementation in %f, netMode %d, timeSeconds %f"), v1, netMode, timeSeconds);
}

void AMyTestActorBaseCpp::testMulticastRPCFunction0(float v1)
{
    UWorld *world = GetWorld();

    uint8 netMode = GEngine->GetNetMode(world);
    float timeSeconds = world->GetTimeSeconds();

    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("before testMulticastRPCFunction0 in %f, netMode %d, timeSeconds %f"), v1, netMode, timeSeconds);
    MulticastRPCFunction0(v1);

    timeSeconds = world->GetTimeSeconds();
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("after testMulticastRPCFunction0 in %f, netMode %d, timeSeconds %f"), v1, netMode, timeSeconds);
}

/*
void AMyTestActorBaseCpp::genPusherPointers(FMyMJGamePusherPointersCpp &pusherPointers)
{
    FMyMJGamePusherPointersCpp &cPushersIn = pusherPointers;

    cPushersIn.clear();

    FMyMJGamePusherMadeChoiceNotifyCpp *pPusherNotify = new FMyMJGamePusherMadeChoiceNotifyCpp();
    FMyMJGamePusherCountUpdateCpp *pPusherUpdate = new FMyMJGamePusherCountUpdateCpp();

    TArray<int32> aSel;
    aSel.Emplace(1);
    pPusherNotify->init(4, 3, 2, aSel);

    cPushersIn.giveInLocalThread(pPusherNotify);

    pPusherUpdate->m_bActionGroupIncrease = true;
    cPushersIn.giveInLocalThread(pPusherUpdate);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("genPusherPointers : %s"), *cPushersIn.genDebugString());
}


void AMyTestActorBaseCpp::testRPCWithPusherPointers_Implementation(const FMyMJGamePusherPointersCpp &pusherPointers)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("testRPCWithPusherPointers_Implementation called : %p, %s"), &pusherPointers, *pusherPointers.genDebugString());
}

void AMyTestActorBaseCpp::testRPCWithPusherNotify_Implementation(const FMyMJGamePusherMadeChoiceNotifyCpp &pusherNotify)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("testRPCWithPusherNotify called : %s"), *pusherNotify.genDebugString());
}

void AMyTestActorBaseCpp::testRPCWithPusherFillIn_Implementation(const FMyMJGamePusherFillInActionChoicesCpp &pusherFillIn)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("testRPCWithPusherFillIn called : %s"), *pusherFillIn.genDebugString());
}
*/

void
UMyMJBPUtilsLibrary::testArrayPointerSerialize(AMyTestParentClass0 *pInParent, AMyTestChildClass0 *pInChild, AMyTestParentClass0 *pOutParent, int32 param)
{
    TSharedPtr<FMyMJGameActionBaseCpp> pChild = MakeShareable<FMyMJGameActionBaseCpp>(new FMyMJGameActionBaseCpp());
    pChild->getIdxAttenderRef() = 5;
    //TArray<FMyMJGamePusherBaseCpp> m_aPushers;

    //ToBinary << a;

    TSharedPtr<FMyMJGamePusherBaseCpp> pParent = MakeShareable<FMyMJGamePusherBaseCpp>(new FMyMJGamePusherBaseCpp());

    FBufferArchive ToBinary;
    FMemoryReader FromBinary = FMemoryReader(ToBinary);

    FMyMJGamePusherBaseCpp *pInst;


    pInst = pChild.Get();

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("toBin: %d, %s"), (uint8)pInst->m_eType, *(pInst->StaticStruct()->GetClass()->GetFullName()));
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("toBin: %d, %s"), (uint8)pInst->getType(), *(pInst->StaticStruct()->GetStructCPPName()));
    ToBinary.Seek(0);
    pInst->StaticStruct()->SerializeBin(ToBinary, pInst);


    TSharedPtr<FMyMJGamePusherBaseCpp> pReverted = MakeShareable<FMyMJGamePusherBaseCpp>(new FMyMJGamePusherBaseCpp());
    pInst = pReverted.Get();

    FromBinary.Seek(0);
    pInst->StaticStruct()->SerializeBin(FromBinary, pInst);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FromBin: %d, %s"), (uint8)pInst->m_eType, *(pInst->StaticStruct()->GetClass()->GetFullName()));
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FromBin: %d, %s"), (uint8)pInst->getType(), *(pInst->StaticStruct()->GetStructCPPName()));

    /*
    AMyTestParentClass0 *pBase;


    pInParent->m_iTest = 2;
    pInChild->m_iTest = 5;
    pInChild->m_iTestChild = 10;

    pBase = pInChild;

    FBufferArchive ToBinary2;
    FMemoryReader FromBinary2 = FMemoryReader(ToBinary2);

    ToBinary2.Seek(0);
    pBase->SerializeScriptProperties(ToBinary2);
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("ToBin2: %d, %s"), pBase->m_iTest, *(pBase->GetClass()->GetFullName()));

    pBase = pOutParent;
    FromBinary2.Seek(0);
    pBase->Serialize(FromBinary2);
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FromBin2: %d, %s"), pBase->m_iTest, *(pBase->GetClass()->GetFullName()));
    */

    //ToBinary << p;
}


void UMyMJBPUtilsLibrary::testPusherSerialize0(int32 param)
{
    FMyMJGamePusherPointersCpp cPushersIn, cPushersOut;

    FMyMJGamePusherMadeChoiceNotifyCpp *pPusherNotify = new FMyMJGamePusherMadeChoiceNotifyCpp();
    FMyMJGamePusherCountUpdateCpp *pPusherUpdate = new FMyMJGamePusherCountUpdateCpp();

    TArray<int32> aSel;
    aSel.Emplace(1);
    pPusherNotify->init(4, 3, 2, aSel);

    cPushersIn.giveInLocalThread(pPusherNotify);

    pPusherUpdate->m_bActionGroupIncrease = true;
    cPushersIn.giveInLocalThread(pPusherUpdate);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("before Ser In : %s"), *cPushersIn.genDebugString());
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("before Ser Out: %s"), *cPushersOut.genDebugString());

    FBufferArchive ToBinary;
    FMemoryReader FromBinary = FMemoryReader(ToBinary);

    ToBinary.Seek(0);
    cPushersIn.trySerializeWithTag(ToBinary);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("serialized size %lld."), ToBinary.TotalSize());

    FromBinary.Seek(0);
    cPushersOut.trySerializeWithTag(FromBinary);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("after Ser: %s"), *cPushersOut.genDebugString());

}

void UMyMJBPUtilsLibrary::testGameCoreInLocalThread(int32 seed, bool bAttenderRandomSelectHighPriActionFirst)
{
    int32 iSeed;
    if (seed != 0) {
        iSeed = seed;
    }
    else {
        iSeed = UMyMJUtilsLibrary::nowAsMsFromTick();
    }

    FMyMJGameCoreCpp *pCore = helperCreateCoreByRuleType(MyMJGameRuleTypeCpp::LocalCS, iSeed, MyMJGameCoreTrivalConfigMaskForceActionGenTimeLeft2AutoChooseMsZero | MyMJGameCoreTrivalConfigMaskShowPusherLog);

    TSharedPtr<FMyMJGameCoreCpp> p = MakeShareable<FMyMJGameCoreCpp>(pCore);

    FMyMJGameIOGroupAllCpp cIOGourpAll;
    pCore->initFullMode(p, &cIOGourpAll);

    int8 iAttenderRandomMask = 0;
    if (true) {
        iAttenderRandomMask |= MyMJGameActionContainorCpp_RandomMask_DoRandomSelect;
    }
    if (bAttenderRandomSelectHighPriActionFirst) {
        iAttenderRandomMask |= MyMJGameActionContainorCpp_RandomMask_HighPriActionFirst;
    }
    int32 iAttendersAllRandomSelectMask = MyMJGameDup8BitMaskForSingleAttenderTo32BitMaskForAll(iAttenderRandomMask);

    FMyMJGameCmdRestartGameCpp *pCmdReset = new FMyMJGameCmdRestartGameCpp();
    pCmdReset->m_iAttendersAllRandomSelectMask = iAttendersAllRandomSelectMask;
    UMyMJUtilsLocalCSLibrary::genDefaultCfg(pCmdReset->m_cGameCfg);
    cIOGourpAll.m_aGroups[(uint8)MyMJGameRoleTypeCpp::SysKeeper].getCmdInputQueue().Enqueue(pCmdReset);


    pCore->tryProgressInFullMode();

};



float UMyMJBPUtilsLibrary::testGetRealTimeSeconds(AActor *actor)
{
    return actor->GetWorld()->GetRealTimeSeconds();
};


int32 UMyMJBPUtilsLibrary::getEngineNetMode(AActor *actor)
{
    UWorld *world = actor->GetWorld();
    if (world) {
        return GEngine->GetNetMode(world);
    }
    else {
        return -1;
    }
};

bool UMyMJBPUtilsLibrary::haveServerLogicLayer(AActor *actor)
{
    ENetMode mode = actor->GetNetMode();

    return mode == ENetMode::NM_Standalone || mode == ENetMode::NM_ListenServer || mode == ENetMode::NM_DedicatedServer;

    //return actor->HasAuthority();
};

bool UMyMJBPUtilsLibrary::haveServerNetworkLayer(AActor *actor)
{
    ENetMode mode = actor->GetNetMode();

    return mode == ENetMode::NM_ListenServer || mode == ENetMode::NM_DedicatedServer;
}

bool UMyMJBPUtilsLibrary::haveClientVisualLayer(AActor *actor)
{
    ENetMode mode = actor->GetNetMode();

    return mode == ENetMode::NM_Standalone || mode == ENetMode::NM_ListenServer || mode == ENetMode::NM_Client;
};

bool UMyMJBPUtilsLibrary::haveClientNetworkLayer(AActor *actor)
{
    ENetMode mode = actor->GetNetMode();

    return mode == ENetMode::NM_Client;
};

bool UMyMJBPUtilsLibrary::testLoadAsset(UObject* outer, FString fullPathName)
{
    UStaticMesh *pMeshAsset = NULL;

    /*
    pMeshAsset = FindObject<UStaticMesh>(outer, *fullPathName);
    if (pMeshAsset) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("FindObject: %s, pendingKill %d."), *fullPathName, pMeshAsset->IsPendingKill());
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to findObject: %s."), *fullPathName);
    }

    pMeshAsset = LoadObject<UStaticMesh>(outer, *fullPathName, NULL, LOAD_None, NULL);
    if (pMeshAsset) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("LoadObject: %s, pendingKill %d."), *fullPathName, pMeshAsset->IsPendingKill());
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to LoadObject: %s."), *fullPathName);
    }
    */

    pMeshAsset = helperTryFindAndLoadAsset<UStaticMesh>(outer, fullPathName);

    return true;
}

void UMyMJBPUtilsLibrary::rotateOriginWithPivot(const FTransform& originCurrentWorldTransform, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultWorldTransform)
{
    //FRotator originCurrentWorldRotator(originCurrentWorldTransform.GetRotation());
    //FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    //FVector originZeroRotateWorldLocation = originCurrentWorldTransform.GetLocation() - (originCurrentWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);

    //originResultWorldTransform.SetLocation(originZeroRotateWorldLocation + (originTargetWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    //originResultWorldTransform.SetRotation(FQuat(originTargetWorldRotator));
    //originResultWorldTransform.SetScale3D(originCurrentWorldTransform.GetScale3D());

    FQuat originCurrentWorldQuat = originCurrentWorldTransform.GetRotation();
    FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    FVector testV = originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("testV: (%f, %f, %f)."), testV.X, testV.Y, testV.Z);
    //return;

    FVector originZeroRotateWorldLocation = originCurrentWorldTransform.GetLocation() - (originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originZeroRotateWorldLocation: (%f, %f, %f)."), originZeroRotateWorldLocation.X, originZeroRotateWorldLocation.Y, originZeroRotateWorldLocation.Z);
    //return;
    
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originTargetWorldRotator: (%f, %f, %f)."),
        originTargetWorldRotator.Roll, originTargetWorldRotator.Pitch, originTargetWorldRotator.Yaw);
    //FQuat originTargetWorldQuat(originTargetWorldRotator);
    FQuat originTargetWorldQuat = originTargetWorldRotator.Quaternion();
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originTargetWorldQuat: (%f, %f, %f, %f)."),
        originTargetWorldQuat.X, originTargetWorldQuat.Y, originTargetWorldQuat.Z, originTargetWorldQuat.W)
    //return;
    
    FRotator originTargetWorldRotator2(originTargetWorldQuat);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("rotator change: (%f, %f, %f) -> (%f, %f, %f)."),
              originTargetWorldRotator.Roll, originTargetWorldRotator.Pitch, originTargetWorldRotator.Yaw,
              originTargetWorldRotator2.Roll, originTargetWorldRotator2.Pitch, originTargetWorldRotator2.Yaw);
    //return;

    FVector locResultRotated = originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("locResultRotated: (%f, %f, %f)."), locResultRotated.X, locResultRotated.Y, locResultRotated.Z);

    originResultWorldTransform.SetLocation(originZeroRotateWorldLocation + (originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    originResultWorldTransform.SetRotation(originTargetWorldQuat);
    originResultWorldTransform.SetScale3D(originCurrentWorldTransform.GetScale3D());
};

FString UMyMJBPUtilsLibrary::getDebugStringFromEWorldType(EWorldType::Type t)
{
    if (t == EWorldType::None)
    {
        return TEXT("None");
    }
    else if (t == EWorldType::Game)
    {
        return TEXT("Game");
    }
    else if (t == EWorldType::Editor)
    {
        return TEXT("Editor");
    }
    else if (t == EWorldType::PIE)
    {
        return TEXT("PIE");
    }
    else if (t == EWorldType::EditorPreview)
    {
        return TEXT("EditorPreview");
    }
    else if (t == EWorldType::GamePreview)
    {
        return TEXT("GamePreview");
    }
    else if (t == EWorldType::Inactive)
    {
        return TEXT("Inactive");
    }
    else
    {
        return TEXT("Invalid");
    }

};

FString UMyMJBPUtilsLibrary::getDebugStringFromENetMode(ENetMode t)
{
    if (t == ENetMode::NM_Standalone)
    {
        return TEXT("NM_Standalone");
    }
    else if (t == ENetMode::NM_DedicatedServer)
    {
        return TEXT("NM_DedicatedServer");
    }
    else if (t == ENetMode::NM_ListenServer)
    {
        return TEXT("NM_ListenServer");
    }
    else if (t == ENetMode::NM_Client)
    {
        return TEXT("NM_Client");
    }
    else if (t == ENetMode::NM_MAX)
    {
        return TEXT("NM_MAX");
    }
    else {
        return TEXT("Invalid");
    }
}

void UMyMJBPUtilsLibrary::MyBpLog(UObject* WorldContextObject, const FString& InString, bool bPrintToScreen, bool bPrintToLog, MyLogVerbosity eV, FLinearColor TextColor, float Duration)
{
    const FString *pStr = &InString;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
   

    FString Prefix;
    if (World)
    {
        Prefix = getDebugStringFromEWorldType(World->WorldType) + TEXT(" ") + getDebugStringFromENetMode(World->GetNetMode()) + TEXT(": ");
        Prefix += InString;
        pStr = &Prefix;
    }

    if (bPrintToLog) {
        //the fuck is that, UE_LOG's log level is a compile time check
        if (eV == MyLogVerbosity::Display) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s"), **pStr);
        }
        else if (eV == MyLogVerbosity::Warning) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("%s"), **pStr);
        }
        else if (eV == MyLogVerbosity::Error) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s"), **pStr);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Log, TEXT("%s"), **pStr);
        }


        APlayerController* PC = (WorldContextObject ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : NULL);
        ULocalPlayer* LocalPlayer = (PC ? Cast<ULocalPlayer>(PC->Player) : NULL);
        if (LocalPlayer && LocalPlayer->ViewportClient && LocalPlayer->ViewportClient->ViewportConsole)
        {
            LocalPlayer->ViewportClient->ViewportConsole->OutputText(*pStr);
        }
    }

    if (bPrintToScreen)
    {
        if (GAreScreenMessagesEnabled)
        {
            if (GConfig && Duration < 0)
            {
                GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
            }

            bool bPrint = true;
//#if (UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test
            bPrint = (uint8)eV >= (uint8)MyLogVerbosity::Display;
//#endif
            if (bPrint) {
                GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), *pStr);
            }
        }
        else
        {
            UE_LOG(LogBlueprint, VeryVerbose, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
        }
    }

};

FString UMyMJBPUtilsLibrary::getClassAssetPath(UClass* pC)
{   
    //two way:
    // 1st is UPackage* pP = Cast<UPackage>(m_cCfgCardClass->GetDefaultObject()->GetOuter()); pP->FileName; result like /Game/CoreBPs/MyMJGameCardBaseBp
    // 2nd is m_cCfgCardClass->GetPathName(NULL), result like /Game/CoreBPs/MyMJGameCardBaseBp.MyMJGameCardBaseBp_C
    FString ret;

    if (!pC) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class is NULL"));
        return ret;
    }

    UObject* o = pC->GetDefaultObject()->GetOuter();
    if (!o) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("outer is NULL"));
        return ret;
    }
    UPackage* pP = Cast<UPackage>(o);
    if (!o) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to cast, class name is %s."), *o->GetClass()->GetName());
        return ret;
    }

    FString assetFullName = pP->FileName.ToString();
    int32 idxLastDelim = -1;
    if (!assetFullName.FindLastChar('/', idxLastDelim)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to find delimitor, orinin str %s."), *assetFullName);
        return ret;
    }

    MY_VERIFY(idxLastDelim >= 0);
    ret = assetFullName.Left(idxLastDelim);

    return ret;
}