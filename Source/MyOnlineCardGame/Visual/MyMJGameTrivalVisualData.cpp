// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameTrivalVisualData.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "UnrealNetwork.h"

void AMyMJGameTrivalDataSourceCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyMJGameTrivalDataSourceCpp, m_cData);
};