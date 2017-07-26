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


protected:

    FRandomStream m_cRandomStream;
};
