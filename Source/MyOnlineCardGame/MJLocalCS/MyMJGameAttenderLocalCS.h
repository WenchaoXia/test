// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MJ/MyMJGameAttender.h"

//#include "MyMJGameAttenderLocalCS.generated.h"


class FMyMJGameAttenderLocalCSCpp : public FMyMJGameAttenderCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameAttenderLocalCSCpp() : FMyMJGameAttenderCpp()
    {
        reset(false);
    };

    virtual ~FMyMJGameAttenderLocalCSCpp()
    {

    };

    inline
        FMyMJHuScoreResultFinalGroupCpp& getHuScoreResultFinalGroupLocalCSBornRef()
    {
        return m_cHuScoreResultFinalGroupLocalCSBorn;
    };


    virtual void reset(bool bIsRealAttender) override
    {
        FMyMJGameAttenderCpp::reset(bIsRealAttender);

        m_aIdHandCardShowedOutLocalCS.Reset();
        m_cHuScoreResultFinalGroupLocalCSBorn.reset();
        m_bGangYaoedLocalCS = false;
        m_bBanPaoHuLocalCS = false;

    };

    //There are methods to move virtual func binding from runtime to compile time, but to save time I just used virtual func here 
    //Warn: don't store &pPusherIO inside the function
    virtual void genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO) override;

    //return whether now ting 
    bool tryGenAndEnqueueUpdateTingPusher();

    //Following Can only be called by MJCore, and they try to do all the work related to attender, such as update ting
    void applyPusherUpdateTing(FMyMJGamePusherUpdateTingCpp *pPusher);

    void applyActionNoAct(FMyMJGameActionNoActCpp *pAction);

    void applyActionHuBornLocalCS(FMyMJGameActionHuBornLocalCSCpp *pAction);

    void applyActionWeave(FMyMJGameActionWeaveCpp *pAction);


    void showOutCardsAfterHu();

    void dataResetByMask(int32 iMaskAttenderDataReset);

    //when attender take card or weave, new turn(tips hand card changed), @bIsWeave == false means just taken cards, other wise just weaved
    void onNewTurn(bool bIsWeave);



protected:

    void genActionAfterGivenOutCards(FMyMJGamePusherFillInActionChoicesCpp *pFillInPusher, bool bIsGang);

    //returned one will be created on heap
    FMyMJGameActionGiveOutCardsCpp* genActionChoiceGiveOutCards(bool bRestrict2SelectCardsJustTaken, bool bIsGang);

    //@pTriggerCards if set, must have all card values set.
    void assembleHuActionAttr(int32 iIdxAttenderLoseOnlyOne, const TArray<FMyIdValuePair> *pTriggerCards, MyMJHuTriggerCardSrcTypeCpp eTriggerCardSrcType, FMyMJHuActionAttrCpp &outHuActionAttr);

    //following function takes trigger card as in, which is the card have taken, or other have give out or gang
    //@pTriggerCard can be NULL
    bool checkGang(const FMyMJCardCpp *pTriggerCard, bool bLimited2TriggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves);

    bool checkPeng(const FMyMJCardCpp &triggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves);

    bool checkChi(const FMyMJCardCpp &triggerCard, TArray<FMyMJGameActionWeaveCpp> &outActionWeaves);


    //local CS
    TArray<int32> m_aIdHandCardShowedOutLocalCS;
    FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroupLocalCSBorn;
    bool m_bGangYaoedLocalCS;
    bool m_bBanPaoHuLocalCS;
};