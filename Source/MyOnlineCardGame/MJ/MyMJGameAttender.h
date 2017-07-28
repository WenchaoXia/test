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
        m_iIdxAttender = -1;
        reset();
    };

    virtual ~FMyMJAttenderDataPublicDirectForBPCpp()
    {

    };

    void setup(int32 idxAttender)
    {
        m_iIdxAttender = idxAttender;
        reset();
    };

    void reset()
    {
        m_aIdJustTakenCards.Reset();
        m_aIdGivenOutCards.Reset();
        m_aShowedOutWeaves.Reset();
        m_aIdWinSymbolCards.Reset();
        m_cUntakenSlotSubSegmentInfo.reset();
        m_cHuScoreResultFinalGroup.reset();
        m_iTurn = -1;
        m_bIsRealAttender = false;
        m_bIsStillInGame = false;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx Attender"))
    int32 m_iIdxAttender; //it is the position

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

    //some game may put one player into observe state, this is the flag
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "is still in game"))
    bool m_bIsStillInGame;
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

    void setup(int32 idxAttender)
    {
        m_cActionContainor.setup(idxAttender);
        m_cActionContainor.reinit(false);
        reset();
    };

    void reset()
    {
        m_cHandCards.clear();
        m_cActionContainor.resetForNewLoop();
        m_cHuScoreResultTingGroup.reset();
        m_cCardValuePack.reset(0);
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
    FMyMJGameAttenderCpp(MyMJGameCoreWorkModeCpp eWorkMode)
    {
        m_pDataForFullMode = NULL;
        m_pDataPublicForMirrorMode = NULL;
        m_pDataPrivateForMirrorMode = NULL;

        m_iIdx = -1;
        m_pCore = NULL;
        m_eWorkMode = eWorkMode;

        //reset(false);
    };

    virtual ~FMyMJGameAttenderCpp()
    {

    };

    virtual void initFullMode(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx)
    {
        m_pCore = pCore;
        m_iIdx = idx;

        MY_VERIFY(!m_pDataForFullMode.IsValid());

        m_pDataForFullMode = MakeShareable<FMyMJAttenderDataForFullModeCpp>(new FMyMJAttenderDataForFullModeCpp());
        m_pDataForFullMode->m_cDataPublicDirect.setup(idx);
        m_pDataForFullMode->m_cDataPrivateDirect.setup(idx);

        reset(false);
    };

    virtual void initMirrorMode(TWeakPtr<FMyMJGameCoreCpp> pCore, int32 idx, UMyMJAttenderDataPublicForMirrorModeCpp *pDataPublic, UMyMJAttenderDataPrivateForMirrorModeCpp *pDataPrivate)
    {
        m_pCore = pCore;
        m_iIdx = idx;

        MY_VERIFY(!m_pDataPublicForMirrorMode.IsValid());
        MY_VERIFY(!m_pDataPrivateForMirrorMode.IsValid());

        m_pDataPublicForMirrorMode = pDataPublic;
        m_pDataPrivateForMirrorMode = pDataPrivate;

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
        FMyMJAttenderDataPrivateDirectForBPCpp *pDPriD = getDataPrivateDirect();
        MY_VERIFY(pDPriD);
        return &pDPriD->m_cActionContainor;
    };

    //Don't call following directly, instead, call pCore->moveCardFromOldPosi()
    void removeCard(int32 id);
    void insertCard(int32 id, MyMJCardSlotTypeCpp eTargetSlot);

protected:

    void recalcMinorPosiOfCardsInShowedOutWeaves();

    TSharedPtr<FMyMJAttenderDataForFullModeCpp> m_pDataForFullMode;


    TWeakObjectPtr<UMyMJAttenderDataPublicForMirrorModeCpp>  m_pDataPublicForMirrorMode;


    TWeakObjectPtr<UMyMJAttenderDataPrivateForMirrorModeCpp> m_pDataPrivateForMirrorMode;

    //can be cast to MyMJGameRoleTypeCpp
    int32 m_iIdx;

    TWeakPtr<FMyMJGameCoreCpp> m_pCore;

    MyMJGameCoreWorkModeCpp m_eWorkMode;
};