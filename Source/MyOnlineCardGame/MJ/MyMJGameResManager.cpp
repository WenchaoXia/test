// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameResManager.h"


FMyMJGameResManager::FMyMJGameResManager(int32 seed)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("init with seed: %d"), seed);
    m_cRandomStream.Initialize(seed);
};

