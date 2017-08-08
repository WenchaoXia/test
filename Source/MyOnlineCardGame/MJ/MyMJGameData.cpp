// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameData.h"

#include "UnrealNetwork.h"
#include "Engine/ActorChannel.h"

void UMyMJCoreDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJCoreDataForMirrorModeCpp, m_cDataPubicDirect);
};

void UMyMJAttenderDataPublicForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJAttenderDataPublicForMirrorModeCpp, m_cDataPublicDirect);
};

void UMyMJAttenderDataPrivateForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJAttenderDataPrivateForMirrorModeCpp, m_cDataPrivateDirect);
};

void UMyMJAttenderDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJAttenderDataForMirrorModeCpp, m_pDataPublic);
    DOREPLIFETIME(UMyMJAttenderDataForMirrorModeCpp, m_pDataPrivate);
};

/*
bool UMyMJAttenderDataForMirrorModeCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJAttenderDataForMirrorModeCpp::ReplicateSubobjects."));

    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);



    if (m_pDataPublic != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(m_pDataPublic, *Bunch, *RepFlags);
    }

    if (m_pDataPrivate != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(m_pDataPrivate, *Bunch, *RepFlags);
    }

    return WroteSomething;

};
*/

void UMyMJDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJDataForMirrorModeCpp, m_pCoreData);
    DOREPLIFETIME(UMyMJDataForMirrorModeCpp, m_aAttenderDatas);
};

//Component replicate subobjects, just take care of child subobject, it's own property will be taken care by caller
bool UMyMJDataForMirrorModeCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{

    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJDataForMirrorModeCpp::ReplicateSubobjects 0."));

    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);


    if (m_pCoreData != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(m_pCoreData, *Bunch, *RepFlags);
    }

    int l = m_aAttenderDatas.Num();
    for (int i = 0; i < l; i++) {
        //m_aAttenderDatas[i]->ReplicateSubobjects(Channel, Bunch, RepFlags);
        WroteSomething |= Channel->ReplicateSubobject(m_aAttenderDatas[i], *Bunch, *RepFlags);

        if (m_aAttenderDatas[i]->m_pDataPublic) {
            WroteSomething |= Channel->ReplicateSubobject(m_aAttenderDatas[i]->m_pDataPublic, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("public is NULL."));
        }

        if (m_aAttenderDatas[i]->m_pDataPrivate) {
            WroteSomething |= Channel->ReplicateSubobject(m_aAttenderDatas[i]->m_pDataPrivate, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("private is NULL."));
        }
    }


    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJDataForMirrorModeCpp::ReplicateSubobjects 1, %d"), WroteSomething);

    return WroteSomething;
};


void UMyMJDataForMirrorModeCpp::createSubObjects(bool bBuildingDefault, bool bHavePrivate)
{
    if (bBuildingDefault) {
        m_pCoreData = CreateDefaultSubobject<UMyMJCoreDataForMirrorModeCpp>(TEXT("UMyMJCoreDataForMirrorModeCpp"));
    }
    else {
        m_pCoreData = NewObject<UMyMJCoreDataForMirrorModeCpp>(this);
    }

    m_aAttenderDatas.Reset();
    for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
        UMyMJAttenderDataForMirrorModeCpp *pAttenderData;
        if (bBuildingDefault) {
            pAttenderData = CreateDefaultSubobject<UMyMJAttenderDataForMirrorModeCpp>(*FString::Printf(TEXT("UMyMJAttenderDataForMirrorModeCpp_%d"), i));
        }
        else {
            pAttenderData = NewObject<UMyMJAttenderDataForMirrorModeCpp>(this);
        }

        pAttenderData->createSubObjects(false, bHavePrivate);
        pAttenderData->resetup(i);

        int32 idx = m_aAttenderDatas.Emplace(pAttenderData);

        MY_VERIFY(idx == i);
    }
};