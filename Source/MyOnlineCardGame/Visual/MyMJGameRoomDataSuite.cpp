// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomDataSuite.h"

UMyMJGameRoomDataSuite::UMyMJGameRoomDataSuite()
{
    m_pDeskDataObj = CreateDefaultSubobject<UMyMJGameDeskVisualDataObjCpp>(TEXT("desk data obj"));
    m_pTrivalDataObj = CreateDefaultSubobject<UMyMJGameTrivalVisualDataObjCpp>(TEXT("trival data obj"));
};

UMyMJGameRoomDataSuite::~UMyMJGameRoomDataSuite()
{

};