// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJBPUtils.h"


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
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("MulticastRPCFunction0_Implementation %f"), v1);
}


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

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("before Ser In : %s"), *cPushersIn.genDebugString());
}

void AMyTestActorBaseCpp::testRPCWithPusherPointers_Implementation(const FMyMJGamePusherPointersCpp &pusherPointers)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("called : %s"), *pusherPointers.genDebugString());
}


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