// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameResManager.h"


FMyMJGameResManager::FMyMJGameResManager(int32 seed)
{
    m_cRandomStream.Initialize(seed);
    m_iSeed = seed;
};

