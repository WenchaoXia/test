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

//always full mode
class FMyMJGameAttenderCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameAttenderCpp()
    {
        m_pCore = NULL;

        //reset(false); //may core dump since not setupped, for simple just don't reset here, and we need to check the code manually when adding member, and most member should goto data member which doesn't need to init manually
    };

    virtual ~FMyMJGameAttenderCpp()
    {

    };

    virtual void initFullMode(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx)
    {
        m_pCore = pCore;

        m_cDataLogic.setup(idx);

        resetDatasOwned();
    };

    //Only reset data that belong to it, others such as  public attender data will not be reset
    virtual void resetDatasOwned()
    {
        m_cDataLogic.resetForNewGame();
    };

    virtual void genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO) = NULL;
    //end

    inline
    int32 getIdx() const
    {
        int32 idx = m_cDataLogic.m_iIdx;
        MY_VERIFY(idx >= 0 && idx < 4);
        return  idx;
    };

    inline
    FMyMJGameCoreCpp& getCoreRef() const
    {
        return const_cast<FMyMJGameCoreCpp &>(getCoreRefConst());
    };

    inline
    const FMyMJGameCoreCpp& getCoreRefConst() const
    {
        TSharedPtr<FMyMJGameCoreCpp> pCore = m_pCore.Pin();
        if (pCore.IsValid()) {
            return *pCore.Get();
        }
        else {
            MY_VERIFY(false);
            return *(FMyMJGameCoreCpp *)NULL;
        }
    };

    const FMyMJAttenderDataLogicOnlyCpp& getDataLogicRefConst() const
    {
        return m_cDataLogic;
    };

    inline
    void resetForNewLoop(bool bNeed2Collect)
    {
        FMyMJGameActionContainorCpp *pContainor = &m_cDataLogic.m_cActionContainor;
        pContainor->resetForNewLoop();

        pContainor->getNeed2CollectRef() = bNeed2Collect;
    };


    FMyMJGameActionContainorCpp& getActionContainorRef()
    {
        return m_cDataLogic.m_cActionContainor;
    };


    const FMyMJRoleDataAttenderPublicCpp&  getRoleDataAttenderPublicRefConst()  const;
    const FMyMJRoleDataAttenderPrivateCpp& getRoleDataAttenderPrivateRefConst() const;

    bool getIsRealAttender() const;
    bool getIsStillInGame() const;

    //Don't call following directly, instead, call pCore->moveCardFromOldPosi()
    void removeCard(int32 id);
    void insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot);

protected:


    FMyMJAttenderDataLogicOnlyCpp m_cDataLogic;

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;

};