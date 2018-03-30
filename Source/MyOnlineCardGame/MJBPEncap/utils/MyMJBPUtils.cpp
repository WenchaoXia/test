// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJBPUtils.h"

#include "Utils/CommonUtils/MyCommonUtilsLibrary.h"

#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

#include "Kismet/GameplayStatics.h"

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

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("genPusherPointers : %s"), *cPushersIn.ToString());
}


void AMyTestActorBaseCpp::testRPCWithPusherPointers_Implementation(const FMyMJGamePusherPointersCpp &pusherPointers)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("testRPCWithPusherPointers_Implementation called : %p, %s"), &pusherPointers, *pusherPointers.ToString());
}

void AMyTestActorBaseCpp::testRPCWithPusherNotify_Implementation(const FMyMJGamePusherMadeChoiceNotifyCpp &pusherNotify)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("testRPCWithPusherNotify called : %s"), *pusherNotify.ToString());
}

void AMyTestActorBaseCpp::testRPCWithPusherFillIn_Implementation(const FMyMJGamePusherFillInActionChoicesCpp &pusherFillIn)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("testRPCWithPusherFillIn called : %s"), *pusherFillIn.ToString());
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

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("before Ser In : %s"), *cPushersIn.ToString());
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("before Ser Out: %s"), *cPushersOut.ToString());

    FBufferArchive ToBinary;
    FMemoryReader FromBinary = FMemoryReader(ToBinary);

    ToBinary.Seek(0);
    cPushersIn.trySerializeWithTag(ToBinary);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("serialized size %lld."), ToBinary.TotalSize());

    FromBinary.Seek(0);
    cPushersOut.trySerializeWithTag(FromBinary);

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("after Ser: %s"), *cPushersOut.ToString());

}

void UMyMJBPUtilsLibrary::testGameCoreInLocalThread(int32 seed, bool bAttenderRandomSelectHighPriActionFirst)
{
    int32 iSeed;
    if (seed != 0) {
        iSeed = seed;
    }
    else {
        iSeed = UMyCommonUtilsLibrary::nowAsMsFromTick();
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

    pMeshAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(outer, fullPathName);

    return true;
}


MyMJGameEventVisualTypeCpp UMyMJBPUtilsLibrary::helperGetEventVisualTypeFromWeave(MyMJGameRuleTypeCpp ruleType, const FMyMJWeaveCpp& weave)
{
    if (ruleType == MyMJGameRuleTypeCpp::LocalCS)
    {
        MyMJWeaveTypeCpp eWeaveType = weave.getType();

        if (eWeaveType == MyMJWeaveTypeCpp::ShunZiAn || eWeaveType == MyMJWeaveTypeCpp::ShunZiMing) {
            return MyMJGameEventVisualTypeCpp::WeaveChi;
        }
        else if (eWeaveType == MyMJWeaveTypeCpp::KeZiAn || eWeaveType == MyMJWeaveTypeCpp::KeZiMing) {
            return MyMJGameEventVisualTypeCpp::WeavePeng;
        }
        else if (eWeaveType == MyMJWeaveTypeCpp::GangAn || eWeaveType == MyMJWeaveTypeCpp::GangMing) {
            bool bIsBuZhang = weave.getGangBuZhangLocalCS();
            if (bIsBuZhang) {
                return MyMJGameEventVisualTypeCpp::WeaveBu;
            }
            else {
                return MyMJGameEventVisualTypeCpp::WeaveGang;
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected weave type: %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJWeaveTypeCpp"), (uint8)eWeaveType));
            return MyMJGameEventVisualTypeCpp::Invalid;
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("meet unsupported rule type: %s."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)ruleType));
        return MyMJGameEventVisualTypeCpp::Invalid;
    }
};

void UMyMJBPUtilsLibrary::helperGetDiceNumbersFromMask(int32 mask, int32 &number0, int32 &number1)
{
    number0 = UMyMJUtilsLibrary::getIntValueFromBitMask(mask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value0_BitLen);
    number1 = UMyMJUtilsLibrary::getIntValueFromBitMask(mask, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitPosiStart, FMyMJCoreDataPublicDirectDiceNumberNowMask_Value1_BitLen);
};