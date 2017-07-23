// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameCmd.h"
#include "MyMJGameCore.h"

bool FMyMJGameCmdPointersCpp::helperFillAsSegmentFromIOGroup(FMyMJGameIOGroupCpp *pIOGroup)
{
    clear();
    bool bHaveNew = false;
    TQueue<FMyMJGameCmdBaseCpp *, EQueueMode::Spsc>& cmdRespQ = pIOGroup->getCmdOutputQueue();
    FMyMJGameCmdBaseCpp *pCmd;
    while (cmdRespQ.Dequeue(pCmd)) {
        giveInLocalThread(pCmd);
        bHaveNew = true;
    }

    return bHaveNew;
}

bool FMyMJGameCmdPointersCpp::trySerializeWithTag(FArchive &Ar)
{

    UScriptStruct *pSC = StaticStruct();
    pSC->SerializeTaggedProperties(Ar, (uint8*)this, pSC, NULL);

    int32 l;
    bool bIsLoading;
    if (Ar.IsLoading()) {
        clear();
        bIsLoading = true;
    }
    else {
        if (!Ar.IsSaving()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("FMyMJGamePusherPointersCpp::trySerializeWithTag(): neither save or load"));
            return false;
        }
        l = m_aCmdSharedPtr.Num();
        bIsLoading = false;
    }

    Ar << l;

    for (int32 i = 0; i < l; i++) {
        MyMJGameCmdType eType = MyMJGameCmdType::Invalid;
        FMyMJGameCmdBaseCpp *pCmdBase = NULL;
        if (bIsLoading) {

        }
        else {
            pCmdBase = m_aCmdSharedPtr[i].Get();
            eType = pCmdBase->getType();

        }
        Ar << eType;

        UScriptStruct *pS = NULL;
        if (eType == MyMJGameCmdType::RestartGame) {
            if (bIsLoading) {
                FMyMJGameCmdRestartGameCpp *pCmd0 = new FMyMJGameCmdRestartGameCpp();
                pS = pCmd0->StaticStruct();
                pCmdBase = StaticCast<FMyMJGameCmdBaseCpp *>(pCmd0);
            }
            else {
                FMyMJGameCmdRestartGameCpp *pCmd0 = StaticCast<FMyMJGameCmdRestartGameCpp *>(pCmdBase);
                pS = pCmd0->StaticStruct();
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unsupported type found in cmd serialization, bIsLoading: %d, type: %s."), bIsLoading, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCmdType"), (uint8)eType));
            return false;
        }

        pS->SerializeTaggedProperties(Ar, reinterpret_cast<uint8*>(pCmdBase), pS, NULL);
        if (bIsLoading) {
            if (!pCmdBase) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("program error, data not set,  bIsLoading: %d, type: %s."), bIsLoading, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameCmdType"), (uint8)eType));
                MY_VERIFY(false);
            }
            giveInLocalThread(pCmdBase);
        }
        else {
        }

    }

    return true;
}

bool FMyMJGameCmdPointersCpp::Serialize(FArchive& Ar)
{
    return trySerializeWithTag(Ar);
}

bool FMyMJGameCmdPointersCpp::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    return trySerializeWithTag(Ar);
}

