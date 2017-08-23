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
        //resetDatasOwned(); // not good to call virtual function in constructor 
    };

    virtual ~FMyMJGameAttenderLocalCSCpp()
    {

    };

    virtual void resetDatasOwned() override
    {
        FMyMJGameAttenderCpp::resetDatasOwned();

        m_aIdHandCardShowedOutLocalCS.Reset();
    };

    //There are methods to move virtual func binding from runtime to compile time, but to save time I just used virtual func here 
    //Warn: don't store &pPusherIO inside the function
    virtual void genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO) override;

    //return whether now ting 
    bool tryGenAndEnqueueUpdateTingPusher();


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


    //local CS, we can set it here since not related to visualize and only afferct local CS game
    TArray<int32> m_aIdHandCardShowedOutLocalCS;


};