// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Utils/MyMJUtils.h"

#include "MyMJGameData.h"

//#include "MyMJGameAttender.generated.h"


class FMyMJGameAttenderCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameAttenderCpp(MyMJGameCoreWorkModeCpp eWorkMode)
    {
        m_pDataForFullMode = NULL;
        m_pDataPublicForMirrorMode = NULL;
        m_pDataPrivateForMirrorMode = NULL;

        m_pCore = NULL;
        m_eWorkMode = eWorkMode;

        //reset(false); //may core dump since not setupped, for simple just don't reset here, and we need to check the code manually when adding member, and most member should goto data member which doesn't need to init manually
    };

    virtual ~FMyMJGameAttenderCpp()
    {

    };

    virtual void initFullMode(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx)
    {
        m_pCore = pCore;

        MY_VERIFY(!m_pDataForFullMode.IsValid());

        m_cDataLogic.setup(idx);

        m_pDataForFullMode = MakeShareable<FMyMJRoleDataForFullModeCpp>(new FMyMJRoleDataForFullModeCpp());
        m_pDataForFullMode->m_cDataAttenderPublic.setup(idx);
        //m_pDataForFullMode->m_cDataPrivateDirect.setup(idx);

        reset(false);
    };

    virtual void initMirrorMode(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx, UMyMJRoleDataAttenderPublicForMirrorModeCpp *pDataPublic, UMyMJRoleDataAttenderPrivateForMirrorModeCpp *pDataPrivate)
    {
        m_pCore = pCore;

        m_cDataLogic.setup(idx);

        MY_VERIFY(!m_pDataPublicForMirrorMode.IsValid());
        MY_VERIFY(!m_pDataPrivateForMirrorMode.IsValid());

        MY_VERIFY(pDataPublic);

        m_pDataPublicForMirrorMode = pDataPublic;
        m_pDataPrivateForMirrorMode = pDataPrivate;

        m_pDataPublicForMirrorMode->m_cDataAttenderPublic.setup(idx);
        //m_pDataPrivateForMirrorMode->m_cDataPrivateDirect.setup(idx);

        reset(false);
    };

    virtual void reset(bool bIsRealAttender)
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            if (m_pDataForFullMode.IsValid()) {
                m_pDataForFullMode->m_cDataAttenderPublic.reset();
                m_pDataForFullMode->m_cDataAttenderPrivate.reset();
            }
            else {
                //if code reach here, it should be calling constructor, in which case members is not created yet
                MY_VERIFY(!m_pCore.IsValid()); //not inited yet
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            if (m_pDataPublicForMirrorMode.IsValid()) {
                m_pDataPublicForMirrorMode->m_cDataAttenderPublic.reset();
            }

            if (m_pDataPrivateForMirrorMode.IsValid()) {
                m_pDataPrivateForMirrorMode->m_cDataAttenderPrivate.reset();
            }
        }
        else {
            MY_VERIFY(false);
        }
    };

    virtual void genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO) = NULL;
    //end

 

    inline
    int32 getIdx()
    {
        return  m_cDataLogic.m_iIdx;
    };

    inline
    FMyMJGameCoreCpp *getpCore()
    {
        TSharedPtr<FMyMJGameCoreCpp> pCore = m_pCore.Pin();
        if (pCore.IsValid()) {
            return pCore.Get();
        }
        else {
            return NULL;
        }
    };

    inline FMyMJRoleDataAttenderPublicCpp* getDataPublicDirect()
    {
        const FMyMJRoleDataAttenderPublicCpp *p = getDataPublicDirectConst();
        return const_cast<FMyMJRoleDataAttenderPublicCpp *>(p);
    };

    const FMyMJRoleDataAttenderPublicCpp* getDataPublicDirectConst() const
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataPublicForMirrorMode.IsValid()) {
                return &m_pDataPublicForMirrorMode->m_cDataAttenderPublic;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataPublicForMirrorMode Invalid!"));
                return NULL;
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            MY_VERIFY(m_pDataForFullMode.IsValid());
            return &m_pDataForFullMode->m_cDataAttenderPublic;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    FMyMJRoleDataAttenderPrivateCpp* getDataPrivateDirect()
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataPrivateForMirrorMode.IsValid()) {
                return &m_pDataPrivateForMirrorMode->m_cDataAttenderPrivate;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataPrivateForMirrorMode Invalid!"));
                return NULL;
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            MY_VERIFY(m_pDataForFullMode.IsValid());
            return &m_pDataForFullMode->m_cDataAttenderPrivate;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    bool getIsRealAttender() const
    {
        const FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirectConst();
        MY_VERIFY(pDPubD);
        return UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender);
        //return pDPubD->m_bIsRealAttender;
    };

    void setIsRealAttender(bool bV)
    {
        const FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirect();
        MY_VERIFY(pDPubD);
        UMyMJUtilsLibrary::setBoolValueToBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender, bV);
    };

    bool getIsStillInGame() const
    {
        const FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirectConst();
        MY_VERIFY(pDPubD);
        return UMyMJUtilsLibrary::getBoolValueFromBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame);
        //return pDPubD->m_bIsStillInGame;
    };

    void setIsStillInGame(bool bV)
    {
        const FMyMJRoleDataAttenderPublicCpp *pDPubD = getDataPublicDirect();
        MY_VERIFY(pDPubD);
        UMyMJUtilsLibrary::setBoolValueToBitMask(pDPubD->m_iMask0, FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame, bV);
    };

    FMyMJGameActionContainorCpp *getActionContainor()
    {

        return &m_cDataLogic.m_cActionContainor;
    };

    //Don't call following directly, instead, call pCore->moveCardFromOldPosi()
    void removeCard(int32 id);
    void insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot);

protected:

    void recalcMinorPosiOfCardsInShowedOutWeaves();


    TSharedPtr<FMyMJRoleDataForFullModeCpp> m_pDataForFullMode;


    TWeakObjectPtr<UMyMJRoleDataAttenderPublicForMirrorModeCpp>  m_pDataPublicForMirrorMode;


    TWeakObjectPtr<UMyMJRoleDataAttenderPrivateForMirrorModeCpp> m_pDataPrivateForMirrorMode;

    FMyMJAttenderDataLogicOnlyCpp m_cDataLogic;

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;

    MyMJGameCoreWorkModeCpp m_eWorkMode;
};