// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/MyMJUtils.h"
#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"

//#include "MyMJGameResManager.generated.h"

class FMyMJGameResManager
{
public:
    FMyMJGameResManager(int32 seed);

    virtual ~FMyMJGameResManager()
    {

    };

    FRandomStream&
    getRandomStreamRef()
    {
        return m_cRandomStream;
    };


    inline int32 getSeed() const
    {
        return m_iSeed;
    };

protected:

    FRandomStream m_cRandomStream;
    int32 m_iSeed;
};
