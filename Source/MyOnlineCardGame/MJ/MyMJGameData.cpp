// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameData.h"

#include "UnrealNetwork.h"
#include "Engine/ActorChannel.h"

void UMyMJCoreDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJCoreDataForMirrorModeCpp, m_cDataPubicDirect);
};

void UMyMJRoleDataAttenderPublicForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataAttenderPublicForMirrorModeCpp, m_cDataAttenderPublic);
};

void UMyMJRoleDataAttenderPrivateForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataAttenderPrivateForMirrorModeCpp, m_cDataAttenderPrivate);
};

void UMyMJRoleDataPrivateForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataPrivateForMirrorModeCpp, m_cDataPrivate);
};

void UMyMJRoleDataForMirrorModeCpp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMyMJRoleDataForMirrorModeCpp, m_pDataAttenderPublic);
    DOREPLIFETIME(UMyMJRoleDataForMirrorModeCpp, m_pDataAttenderPrivate);
    DOREPLIFETIME(UMyMJRoleDataForMirrorModeCpp, m_pDataPrivate);
};

/*
bool UMyMJRoleDataForMirrorModeCpp::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("UMyMJRoleDataForMirrorModeCpp::ReplicateSubobjects."));

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
    DOREPLIFETIME(UMyMJDataForMirrorModeCpp, m_aRoleDatas);
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

    int l = m_aRoleDatas.Num();
    for (int i = 0; i < l; i++) {
        //m_aAttenderDatas[i]->ReplicateSubobjects(Channel, Bunch, RepFlags);
        WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i], *Bunch, *RepFlags);

        if (m_aRoleDatas[i]->m_pDataAttenderPublic) {
            WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i]->m_pDataAttenderPublic, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("role attender public is NULL."));
        }

        if (m_aRoleDatas[i]->m_pDataAttenderPrivate) {
            WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i]->m_pDataAttenderPrivate, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("role attender private is NULL."));
        }

        if (m_aRoleDatas[i]->m_pDataPrivate) {
            WroteSomething |= Channel->ReplicateSubobject(m_aRoleDatas[i]->m_pDataPrivate, *Bunch, *RepFlags);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("role private is NULL."));
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

    m_aRoleDatas.Reset();
    for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
        UMyMJRoleDataForMirrorModeCpp *pRoleData;
        if (bBuildingDefault) {
            pRoleData = CreateDefaultSubobject<UMyMJRoleDataForMirrorModeCpp>(*FString::Printf(TEXT("UMyMJRoleDataForMirrorModeCpp_%d"), i));
        }
        else {
            pRoleData = NewObject<UMyMJRoleDataForMirrorModeCpp>(this);
        }

        pRoleData->createSubObjects(false, i < 4, bHavePrivate);
        pRoleData->resetup(i);

        int32 idx = m_aRoleDatas.Emplace(pRoleData);

        MY_VERIFY(idx == i);
    }
};


void FMyMJDataAccessorCpp::applyDelta(const FMyMJDataDeltaCpp &delta)
{
    FMyMJCoreDataPublicCpp &coreDataSelf = getCoreDataRef();

    if (!(coreDataSelf.m_iGameId == delta.m_iGameId && coreDataSelf.m_iPusherIdLast == delta.getId())) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to apply delta since id mismatch: game id [%d, %d], push id [%d, %d]."), coreDataSelf.m_iGameId, delta.m_iGameId, coreDataSelf.m_iPusherIdLast, delta.getId());
        return;
    }

    if (delta.m_aCoreData.Num() > 0) {
        MY_VERIFY(delta.m_aCoreData.Num() == 1);
        const FMyMJCoreDataDeltaCpp& coreDataDelta = delta.m_aCoreData[0];

        int32 l = coreDataDelta.m_aCardInfos2Update.Num();
        for (int32 i = 0; i < l; i++) {
            const FMyMJCardInfoCpp& cardInfoTarget = coreDataDelta.m_aCardInfos2Update[i];
            FMyMJCardInfoCpp& cardInfoSelf = coreDataSelf.m_cCardInfoPack.getRefByIdx(cardInfoTarget.m_iId);

            if (cardInfoSelf.m_cPosi != cardInfoTarget.m_cPosi) {
                //for safety, any different exist, we make movement happen
            }

        }

    }

};