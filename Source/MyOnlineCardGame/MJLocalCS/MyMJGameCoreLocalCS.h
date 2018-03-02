// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyMJGameAttenderLocalCS.h"
#include "MJ/MyMJGameCore.h"
//#include "MyMJGameCoreLocalCS.generated.h"

//always full mode
class FMyMJGameCoreLocalCSCpp : public FMyMJGameCoreCpp
{
public:

    FMyMJGameCoreLocalCSCpp(int32 iSeed) : FMyMJGameCoreCpp(iSeed)
    {
        m_cDataLogic.m_eRuleType = MyMJGameRuleTypeCpp::LocalCS;
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

    virtual FMyMJGameAttenderCpp* createAttender() override
    {
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGameCoreLocalCSCpp createAndInitAttender."));
        FMyMJGameAttenderLocalCSCpp *pRet = new FMyMJGameAttenderLocalCSCpp();
        return StaticCast<FMyMJGameAttenderCpp *>(pRet);
    };

    virtual void genActionChoices() override;
    virtual bool prevApplyPusherResult(const FMyMJGamePusherResultCpp &pusherResult) override;

    virtual FMyMJGamePusherResultCpp* genPusherResultAsSysKeeper(const FMyMJGamePusherBaseCpp &pusher) override;
    virtual void applyPusher(const FMyMJGamePusherBaseCpp &pusher) override;
    virtual void handleCmd(MyMJGameRoleTypeCpp eRoleTypeOfCmdSrc, FMyMJGameCmdBaseCpp &cmd) override;


    static void genBaseFromPusherResetGame(FMyMJGameResManager& RM, const FMyMJGamePusherResetGameCpp &pusherReset, FMyMJDataStructCpp &outBase);

    //don't store the pPusher or pAction, it is intended to use in local thread, and managed by other component, so it is only for sure valid in the call stack path

    void applyPusherResetGame(const FMyMJGamePusherResetGameCpp &pusher);


};
