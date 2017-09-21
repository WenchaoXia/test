// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameAttender.h"

#include "MJLocalCS/Utils/MyMJUtilsLocalCS.h"

#include "MyMJGameCore.h"

const FMyMJRoleDataAttenderPublicCpp&  FMyMJGameAttenderCpp::getRoleDataAttenderPublicRefConst()  const
{
    int32 idx = getIdx();
    return getCoreRefConst().getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(idx);
}

const FMyMJRoleDataAttenderPrivateCpp& FMyMJGameAttenderCpp::getRoleDataAttenderPrivateRefConst() const
{
    int32 idx = getIdx();
    const FMyMJRoleDataAttenderPrivateCpp* p = getCoreRefConst().getDataAccessorRefConst().getRoleDataAttenderPrivateConst(idx);
    MY_VERIFY(p);
    return *p;
}

bool FMyMJGameAttenderCpp::getIsRealAttender() const
{
    int32 idx = getIdx();
    //int32 iMask = getCoreRefConst().getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(idx).m_iMask0;

    //return UMyMJUtilsLibrary::getBoolValueFromBitMask(iMask, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender);

    return getCoreRefConst().getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(idx).m_bIsRealAttender;
}

bool FMyMJGameAttenderCpp::getIsStillInGame() const
{
    int32 idx = getIdx();
    //int32 iMask = getCoreRefConst().getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(idx).m_iMask0;

    //return UMyMJUtilsLibrary::getBoolValueFromBitMask(iMask, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame);

    return getCoreRefConst().getDataAccessorRefConst().getRoleDataAttenderPublicRefConst(idx).m_bIsStillInGame;
}

void FMyMJGameAttenderCpp::removeCard(int32 id)
{
    const FMyMJGameCoreCpp *pCore = &getCoreRefConst();
    const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();

    const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(id);
    MyMJCardSlotTypeCpp eType = pCardInfo->m_cPosi.m_eSlot;

    if (eType == MyMJCardSlotTypeCpp::InHand) {

        const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

        int32 value = pCardValuePack->getByIdx(id, true);
        bool bRemoved = m_cDataLogic.m_cHandCards.remove(id, value);
        MY_VERIFY(bRemoved);
    }

}

void FMyMJGameAttenderCpp::insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot)
{
    if (eTargetSlot == MyMJCardSlotTypeCpp::InHand) {

        const FMyMJGameCoreCpp *pCore = &getCoreRefConst();
        const FMyMJCardInfoPackCpp  *pCardInfoPack = &pCore->getCardInfoPackRefConst();
        const FMyMJCardValuePackCpp *pCardValuePack = &pCore->getCardValuePackOfSysKeeperRefConst();

        const FMyMJCardInfoCpp *pCardInfo = pCardInfoPack->getByIdxConst(id);

        int32 value = pCardValuePack->getByIdx(id, true);
        bool bInserted = m_cDataLogic.m_cHandCards.insert(id, value);
        MY_VERIFY(bInserted);
    }

}