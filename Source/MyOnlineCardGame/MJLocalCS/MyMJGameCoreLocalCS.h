// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyMJGameAttenderLocalCS.h"
#include "MJ/MyMJGameCore.h"
//#include "MyMJGameCoreLocalCS.generated.h"


class FMyMJGameCoreLocalCSCpp : public FMyMJGameCoreCpp
{
public:

    FMyMJGameCoreLocalCSCpp(MyMJGameCoreWorkModeCpp eWorkMode, int32 iSeed) : FMyMJGameCoreCpp(eWorkMode, iSeed)
    {
        m_eRuleType = MyMJGameRuleTypeCpp::LocalCS;
    };

    virtual ~FMyMJGameCoreLocalCSCpp()
    {};

    TSharedPtr<FMyMJGameAttenderLocalCSCpp> getAttenderByIdx(int32 idxAttender)
    {
        TSharedPtr<FMyMJGameAttenderCpp> ret = FMyMJGameCoreCpp::getAttenderByIdx(idxAttender);
        return StaticCastSharedPtr<FMyMJGameAttenderLocalCSCpp>(ret);
    };

    TSharedPtr<FMyMJGameAttenderLocalCSCpp> getRealAttenderByIdx(int32 idxAttender)
    {
 
        TSharedPtr<FMyMJGameAttenderCpp> ret = FMyMJGameCoreCpp::getRealAttenderByIdx(idxAttender);

        return StaticCastSharedPtr<FMyMJGameAttenderLocalCSCpp>(ret);
    };

protected:

    virtual FMyMJGameAttenderCpp* createAttender(MyMJGameCoreWorkModeCpp eWorkMode) override
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGameCoreLocalCSCpp createAndInitAttender."));
        FMyMJGameAttenderLocalCSCpp *pRet = new FMyMJGameAttenderLocalCSCpp(eWorkMode);
        return StaticCast<FMyMJGameAttenderCpp *>(pRet);
    };

    virtual void applyPusher(FMyMJGamePusherBaseCpp *pPusher) override;
    virtual void handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp *pCmd) override;

    //don't store the pPusher or pAction, it is intended to use in local thread, and managed by other component, so it is only for sure valid in the call stack path
    void applyPusherFillInActionChoices(FMyMJGamePusherFillInActionChoicesCpp *pPusher);
    void applyPusherMadeChoiceNotify(FMyMJGamePusherMadeChoiceNotifyCpp *pPusher);
    void applyPusherCountUpdate(FMyMJGamePusherCountUpdateCpp *pPusher);
    void applyPusherResetGame(FMyMJGamePusherResetGameCpp *pPusher);
    void applyPusherUpdateCards(FMyMJGamePusherUpdateCardsCpp *pPusher);

    void applyActionStateUpdate(FMyMJGameActionStateUpdateCpp *pAction);
    void applyActionThrowDices(FMyMJGameActionThrowDicesCpp *pAction);
    void applyActionDistCardsAtStart(FMyMJGameActionDistCardAtStartCpp *pAction);
    void applyActionTakeCards(FMyMJGameActionTakeCardsCpp *pAction);
    void applyActionGiveOutCards(FMyMJGameActionGiveOutCardsCpp *pAction);

    void applyActionWeave(FMyMJGameActionWeaveCpp *pAction);

    void applyActionHu(FMyMJGameActionHuCpp *pAction);

    void applyActionZhaNiaoLocalCS(FMyMJGameActionZhaNiaoLocalCSCpp *pAction);


};
