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

UENUM(BlueprintType)
enum class MyMJGameCoreWorkModeCpp : uint8
{
    Full = 0     UMETA(DisplayName = "Full"), //Full Function mode
    Mirror = 1   UMETA(DisplayName = "Mirror"), //Mirror mode, doesn't produce any thing, just consume the pushers
                                                //MirrorAlone = 2      UMETA(DisplayName = "MirrorAlone")
};

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


USTRUCT(BlueprintType)
struct FMyMJAttenderDataPublicDirectForBPCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJAttenderDataPublicDirectForBPCpp()
    {
        m_iIdx = -1;
        m_iTurn = -1;
        m_bIsRealAttender = false;
        m_bIsStillInGame = false; //some game may put one player into observe state, this is the flag
    };

    virtual ~FMyMJAttenderDataPublicDirectForBPCpp()
    {

    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx Attender"))
    int32 m_iIdx; //it is the position

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards just taken"))
    TArray<int32> m_aIdJustTakenCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards just given out"))
    TArray<int32> m_aIdGivenOutCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "weaves"))
    TArray<FMyMJWeaveCpp> m_aShowedOutWeaves; //weaves showed out

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards win symbol"))
    TArray<int32> m_aIdWinSymbolCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards win symbol"))
    FMyMJGameUntakenSlotSubSegmentInfoCpp m_cUntakenSlotSubSegmentInfo;

    //FMyMJGameActionContainorCpp m_cActionContainor;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final"))
    FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "turn"))
    int32 m_iTurn;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "is real attender"))
    bool m_bIsRealAttender;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "is still in game"))
    bool m_bIsStillInGame; //some game may put one player into observe state, this is the flag
};

USTRUCT(BlueprintType)
struct FMyMJAttenderDataPrivateDirectForBPCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJAttenderDataPrivateDirectForBPCpp()
    {

    };

    virtual ~FMyMJAttenderDataPrivateDirectForBPCpp()
    {

    };

    void reset(int32 iCardNum)
    {
        m_cHuScoreResultTingGroup.reset();
        m_cCardValuePack.reset(iCardNum);
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hand card Map"))
    FMyMJValueIdMapCpp m_cHandCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor for choices"))
    FMyMJGameActionContainorCpp m_cActionContainor;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    FMyMJHuScoreResultTingGroupCpp  m_cHuScoreResultTingGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Card Value Pack"))
    FMyMJCardValuePackCpp m_cCardValuePack;

};

UCLASS(BlueprintType, Blueprintable)
class UMyMJAttenderDataPublicForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:
    FMyMJAttenderDataPublicDirectForBPCpp m_cDataPublicDirect;
};

UCLASS(BlueprintType, Blueprintable)
class UMyMJAttenderDataPrivateForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:
    FMyMJAttenderDataPrivateDirectForBPCpp m_cDataPrivateDirect;
};

USTRUCT()
struct FMyMJAttenderDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJAttenderDataPublicDirectForBPCpp m_cDataPublicDirect;
    FMyMJAttenderDataPrivateDirectForBPCpp m_cDataPrivateDirect;
};


class FMyMJGameAttenderCpp
{
    //GENERATED_USTRUCT_BODY()

public:
    FMyMJGameAttenderCpp(MyMJGameCoreWorkModeCpp eWorkMode) : m_cActionContainor()
    {
        m_pCore = NULL;
        m_iIdx = -1;

        m_eWorkMode = eWorkMode;

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
        m_cActionContainor.setup(idx);
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
    void setIsRealAttender(bool b)
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

    FMyMJAttenderDataPublicDirectForBPCpp* getDataPublicDirect()
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            if (IsValid(m_pDataPublicForMirrorMode)) {
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
            if (IsValid(m_pDataPrivateForMirrorMode)) {
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

    inline FMyMJGameActionContainorCpp *getActionContainor()
    {
        return &m_cActionContainor;
    };

    //Don't call following directly, instead, call pCore->moveCardFromOldPosi()
    void removeCard(int32 id);
    void insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot);

protected:

    void recalcMinorPosiOfCardsInShowedOutWeaves();

    TSharedPtr<FMyMJAttenderDataForFullModeCpp> m_pDataForFullMode;

    UPROPERTY()
    UMyMJAttenderDataPublicForMirrorModeCpp  *m_pDataPublicForMirrorMode;

    UPROPERTY()
    UMyMJAttenderDataPrivateForMirrorModeCpp *m_pDataPrivateForMirrorMode;


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

    MyMJGameCoreWorkModeCpp m_eWorkMode;
};