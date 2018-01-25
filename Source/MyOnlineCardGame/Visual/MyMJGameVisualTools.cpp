// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameVisualTools.h"

int32 AMyMJGameTextureGenSuitBaseCpp::genTextureForMJCardValues(FString namePrefix, bool haveWord, bool haveHua)
{
    if (!checkSettings()) {
        return -1;
    }

    if (m_cOutputPath.Path.IsEmpty()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("output path not specified."));
        return -2;
    }

    if (m_aTargetActors.Num() != 1) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("only one target actor should be specified, but now %d."), m_aTargetActors.Num());
        return -3;
    }

    AActor* pA = m_aTargetActors[0];
    MY_VERIFY(IsValid(pA));

    AMyMJGameCardBaseCpp* pCardActor = Cast<AMyMJGameCardBaseCpp>(pA);
    if (!IsValid(pCardActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("target is not a MJ card child class, class name: %s."), *pA->StaticClass()->GetName());
        return -4;
    }

    int32 ret;
    ret = genPrepare();
    if (0 != ret)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("error happend in prepare, ret: %d."), ret);
        return -10;
    }

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("genTextureForMJCardValues start: path: %s."), *m_cOutputPath.Path);

    FString finalPrefix = m_cOutputPath.Path + TEXT("/") + namePrefix + TEXT("_v");
    FString fullPath;
    int32 value;
    int32 iDebugCount = 0;
    for (int32 i = 0; i < 3; i++)
    {
        for (int32 j = 1; j < 10; j++) {
            value = i * 10 + j;

            pCardActor->setValueShowing(value);
            fullPath = finalPrefix + FString::Printf(TEXT("%02d"), value);

            ret = genDo(fullPath);
            if (ret != 0)
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("error happend in genDo(), value %d, ret: %d."), value, ret);
                return -20;
            }

            iDebugCount++;
        }
    }

    if (haveWord) {
        for (value = 31; value < 35; value++)
        {
            pCardActor->setValueShowing(value);
            fullPath = finalPrefix + FString::Printf(TEXT("%02d"), value);

            ret = genDo(fullPath);
            if (ret != 0)
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("error happend in genDo(), value %d, ret: %d."), value, ret);
                return -21;
            }

            iDebugCount++;
        }

        for (value = 41; value < 44; value++)
        {
            pCardActor->setValueShowing(value);
            fullPath = finalPrefix + FString::Printf(TEXT("%02d"), value);

            ret = genDo(fullPath);
            if (ret != 0)
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("error happend in genDo(), value %d, ret: %d."), value, ret);
                return -22;
            }

            iDebugCount++;
        }
    }

    if (haveHua) {
        for (value = 51; value < 59; value++)
        {
            pCardActor->setValueShowing(value);
            fullPath = finalPrefix + FString::Printf(TEXT("%02d"), value);

            ret = genDo(fullPath);
            if (ret != 0)
            {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("error happend in genDo(), value, ret: %d."), value, ret);
                return -23;
            }

            iDebugCount++;
        }
    }

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("genTextureForMJCardValues end succuessfully: %d texture gened, path: %s."), iDebugCount, *m_cOutputPath.Path);
    return 0;
}