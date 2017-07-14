// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "MJ/MyMJCommonDefines.h"

#include "Utils/MyMJUtils.h"
#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"

#include "MyMJGameAttender.generated.h"

USTRUCT(BlueprintType)
struct FMyMJGameUntakenSlotSubSegmentInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameUntakenSlotSubSegmentInfoCpp()
    {
        reset();
    };

    virtual ~FMyMJGameUntakenSlotSubSegmentInfoCpp()
    {

    };

    void reset()
    {
        m_iIdxStart = -1;
        m_iLength = 0;
    };

    UPROPERTY()
    int32 m_iIdxStart;

    UPROPERTY()
    int32 m_iLength;

};


class FMyMJGameAttenderCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameAttenderCpp() : m_cActionContainor(this)
    {
        m_pCore = NULL;
        m_iIdx = -1;
        reset(false);
    };

    virtual ~FMyMJGameAttenderCpp()
    {

    };

    //following should be implemented by child class
    //start
    virtual void reset(bool bIsRealAttender)
    {
        m_cHandCards.clear();
        m_aIdJustTakenCards.Reset();
        m_aIdGivenOutCards.Reset();
        m_aShowedOutWeaves.Reset();
        m_aIdWinSymbolCards.Reset();

        m_cUntakenSlotSubSegmentInfo.reset();

        m_cActionContainor.resetForNewLoop();

        m_cHuScoreResultFinalGroup.reset();

        //m_iScoreTotal = 0;
        m_iTurn = 0;

        m_bIsRealAttender = bIsRealAttender;
        m_bIsStillInGame = bIsRealAttender;

    };

    virtual void genActionChoices(FMyMJGamePusherIOComponentFullCpp *pPusherIO) = NULL;
    //end


    void init(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx)
    {
        m_pCore = pCore;
        m_iIdx = idx;

        reset(false);
    };

    FMyMJGameActionContainorCpp* getpActionContainor()
    {
        return &m_cActionContainor;
    };

    inline
    bool getISRealAttender() const
    {
        return m_bIsRealAttender;
    };

    inline
    void setISRealAttender(bool b)
    {
        m_bIsRealAttender = b;
    };


    inline
    bool getIsStillInGame() const
    {
        return m_bIsStillInGame;
    };

    inline
    void setIsStillInGame(bool b)
    {
        m_bIsStillInGame = b;
    };


    inline
    FMyMJGameUntakenSlotSubSegmentInfoCpp* getpUntakenSlotSubSegmentInfo()
    {
        return &m_cUntakenSlotSubSegmentInfo;
    };


    inline
    int32 getIdx()
    {
        return m_iIdx;
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

    inline
    TArray<int32> &getIdJustTakenCardsRef()
    {
        return  m_aIdJustTakenCards;;
    };

    inline
    FMyMJHuScoreResultFinalGroupCpp& getHuScoreResultFinalGroupRef()
    {
        return m_cHuScoreResultFinalGroup;
    };

    //Don't call following directly, instead, call pCore->moveCardFromOldPosi()
    void removeCard(int32 id);
    void insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot);

protected:

    void recalcMinorPosiOfCardsInShowedOutWeaves();

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;
    int32 m_iIdx; //it is the position


    FMyMJValueIdMapCpp m_cHandCards;
    TArray<int32> m_aIdJustTakenCards;
    TArray<int32> m_aIdGivenOutCards;
    TArray<FMyMJWeaveCpp> m_aShowedOutWeaves; //weaves showed out
    TArray<int32> m_aIdWinSymbolCards;


    FMyMJGameUntakenSlotSubSegmentInfoCpp m_cUntakenSlotSubSegmentInfo;

    FMyMJGameActionContainorCpp m_cActionContainor;

    FMyMJHuScoreResultTingGroupCpp  m_cHuScoreResultTingGroup;
    FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroup;
    //int32 m_iScoreTotal;

    int32 m_iTurn;

    bool m_bIsRealAttender;
    bool m_bIsStillInGame; //some game may put one player into observe state, this is the flag


};