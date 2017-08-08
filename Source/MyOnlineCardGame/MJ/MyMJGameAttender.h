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

        m_pDataForFullMode = MakeShareable<FMyMJAttenderDataForFullModeCpp>(new FMyMJAttenderDataForFullModeCpp());
        m_pDataForFullMode->m_cDataPublicDirect.setup(idx);
        //m_pDataForFullMode->m_cDataPrivateDirect.setup(idx);

        reset(false);
    };

    virtual void initMirrorMode(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx, UMyMJAttenderDataPublicForMirrorModeCpp *pDataPublic, UMyMJAttenderDataPrivateForMirrorModeCpp *pDataPrivate)
    {
        m_pCore = pCore;

        m_cDataLogic.setup(idx);

        MY_VERIFY(!m_pDataPublicForMirrorMode.IsValid());
        MY_VERIFY(!m_pDataPrivateForMirrorMode.IsValid());

        MY_VERIFY(pDataPublic);

        m_pDataPublicForMirrorMode = pDataPublic;
        m_pDataPrivateForMirrorMode = pDataPrivate;

        m_pDataPublicForMirrorMode->m_cDataPublicDirect.setup(idx);
        //m_pDataPrivateForMirrorMode->m_cDataPrivateDirect.setup(idx);

        reset(false);
    };

    virtual void reset(bool bIsRealAttender)
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            if (m_pDataForFullMode.IsValid()) {
                m_pDataForFullMode->m_cDataPublicDirect.reset();
                m_pDataForFullMode->m_cDataPrivateDirect.reset();
            }
            else {
                //if code reach here, it should be calling constructor, in which case members is not created yet
                MY_VERIFY(!m_pCore.IsValid()); //not inited yet
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            if (m_pDataPublicForMirrorMode.IsValid()) {
                m_pDataPublicForMirrorMode->m_cDataPublicDirect.reset();
            }

            if (m_pDataPrivateForMirrorMode.IsValid()) {
                m_pDataPrivateForMirrorMode->m_cDataPrivateDirect.reset();
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

    FMyMJAttenderDataPublicDirectForBPCpp* getDataPublicDirect()
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataPublicForMirrorMode.IsValid()) {
                return &m_pDataPublicForMirrorMode->m_cDataPublicDirect;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataPublicForMirrorMode Invalid!"));
                return NULL;
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            MY_VERIFY(m_pDataForFullMode.IsValid());
            return &m_pDataForFullMode->m_cDataPublicDirect;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    FMyMJAttenderDataPrivateDirectForBPCpp* getDataPrivateDirect()
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            MY_VERIFY(IsInGameThread());
            if (m_pDataPrivateForMirrorMode.IsValid()) {
                return &m_pDataPrivateForMirrorMode->m_cDataPrivateDirect;
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDataPrivateForMirrorMode Invalid!"));
                return NULL;
            }
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            MY_VERIFY(m_pDataForFullMode.IsValid());
            return &m_pDataForFullMode->m_cDataPrivateDirect;
        }
        else {
            MY_VERIFY(false);
            return NULL;
        }
    };

    bool& getIsRealAttenderRef()
    {
        FMyMJAttenderDataPublicDirectForBPCpp *pDPubD = getDataPublicDirect();
        MY_VERIFY(pDPubD);
        return pDPubD->m_bIsRealAttender;
    };

    bool& getIsStillInGameRef()
    {
        FMyMJAttenderDataPublicDirectForBPCpp *pDPubD = getDataPublicDirect();
        MY_VERIFY(pDPubD);
        return pDPubD->m_bIsStillInGame;
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


    TSharedPtr<FMyMJAttenderDataForFullModeCpp> m_pDataForFullMode;


    TWeakObjectPtr<UMyMJAttenderDataPublicForMirrorModeCpp>  m_pDataPublicForMirrorMode;


    TWeakObjectPtr<UMyMJAttenderDataPrivateForMirrorModeCpp> m_pDataPrivateForMirrorMode;

    FMyMJAttenderDataLogicOnlyCpp m_cDataLogic;

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;

    MyMJGameCoreWorkModeCpp m_eWorkMode;
};