// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyMJCommonDefines.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"

#include "MyMJGameData.generated.h"


USTRUCT(BlueprintType)
struct FMyMJGameUntakenSlotInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameUntakenSlotInfoCpp()
    {
        reset();
    };

    virtual ~FMyMJGameUntakenSlotInfoCpp()
    {

    };

    void reset()
    {
        m_iIdxUntakenSlotHeadNow = -1;
        m_iIdxUntakenSlotTailNow = -1;
        m_iIdxUntakenSlotTailAtStart = -1;
        m_iUntakenSlotLengthAtStart = 0;

        m_iUntakenSlotCardsLeftNumTotal = 0;
        m_iUntakenSlotCardsLeftNumKeptFromTail = 0;
        m_iUntakenSlotCardsLeftNumNormalFromHead = 0;
    };

    inline
        int32 getCardNumCanBeTakenNormally() const
    {
        return m_iUntakenSlotCardsLeftNumNormalFromHead;
    };

    inline
        int32 getCardNumCanBeTakenAll() const
    {
        return m_iUntakenSlotCardsLeftNumTotal;
    };



    //reflect the untaken state on desktop, not how mnay/or where attender can take card

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx untaken slot head now"))
    int32 m_iIdxUntakenSlotHeadNow;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx untaken slot tail now"))
    int32 m_iIdxUntakenSlotTailNow;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx untaken slot tail at start"))
    int32 m_iIdxUntakenSlotTailAtStart;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot length at start"))
    int32 m_iUntakenSlotLengthAtStart; //Note this never change in game

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot cards left num total"))
    int32 m_iUntakenSlotCardsLeftNumTotal; //simply reflect how many cards present on dest as untaken, doesn't mean how many you can take it which should be decided by rule

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot cards left num kept from tail"))
    int32 m_iUntakenSlotCardsLeftNumKeptFromTail;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "untaken slot cards left num normal from head"))
    int32 m_iUntakenSlotCardsLeftNumNormalFromHead;

};

//Only Used for logic, not for visulzied
USTRUCT()
struct FMyMJCoreDataLogicOnlyCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJCoreDataLogicOnlyCpp()
    {
        reset();
        m_iMsLast = 0;
        m_eRuleType = MyMJGameRuleTypeCpp::Invalid;
        m_eWorkMode = MyMJGameCoreWorkModeCpp::Invalid;
    };

    void reset()
    {
        m_iGameId = -1;
        m_iPusherIdLast = -1;

        m_eActionLoopState = MyMJActionLoopStateCpp::Invalid;
        m_cHelperShowedOut2AllCards.clear();
    };

public:

    int64 m_iMsLast;

    MyMJGameRuleTypeCpp m_eRuleType;//also distinguish sub type
    MyMJGameCoreWorkModeCpp m_eWorkMode;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Id"))
    int32 m_iGameId;

    //the last pusher id we got
    UPROPERTY(BlueprintReadWrite, NotReplicated, meta = (DisplayName = "Pusher Id Last"))
    int32 m_iPusherIdLast;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Loop State"))
    MyMJActionLoopStateCpp m_eActionLoopState;

    //used to calculate how many cards left possible hu
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Cards Showed Out to All"))
        FMyMJValueIdMapCpp m_cHelperShowedOut2AllCards;
};

//Atomic
//Both used for logic and visualize
//What the fuck is, UE3 network always send all structor data in one packet even some members in it haven't change, thanks to UE4 this changed, and if Atomic specified, it goes UE3's way
USTRUCT(BlueprintType)
struct FMyMJCoreDataPublicDirectCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreDataPublicDirectCpp()
    {
        reinit(MyMJGameRuleTypeCpp::Invalid);
    };

    virtual ~FMyMJCoreDataPublicDirectCpp()
    {

    };

    void reinit(MyMJGameRuleTypeCpp eRuleType)
    {
        m_eRuleType = eRuleType;
        reset();
    };

    void reset()
    {
        //some data does not need to reset since reset pusher will overwrite them all
        m_aUntakenCardStacks.Reset();
        m_cCardInfoPack.reset(0);

        m_cGameCfg.reset();
        m_cGameRunData.reset();


        m_iActionGroupId = -1;
        m_eGameState = MyMJGameStateCpp::Invalid;

        m_iDiceNumberNowMask = 0;

        m_cUntakenSlotInfo.reset();

        m_aHelperLastCardsGivenOutOrWeave.Reset();
        m_cHelperLastCardTakenInGame.reset(true);
    };

    inline bool isInGameState()
    {
        return (!(m_eGameState == MyMJGameStateCpp::Invalid || m_eGameState == MyMJGameStateCpp::GameEnd));
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "UnTaken Card Stacks"))
        TArray<FMyIdCollectionCpp> m_aUntakenCardStacks; //Always start from attender 0 to 3

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card info pack"))
        FMyMJCardInfoPackCpp  m_cCardInfoPack;

    //FMyMJCardValuePackCpp m_cCardValuePack;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Cfg"))
        FMyMJGameCfgCpp m_cGameCfg;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game RunData"))
        FMyMJGameRunDataCpp m_cGameRunData;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Action Group Id"))
        int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
     MyMJGameStateCpp m_eGameState;

    //0xf means dice 0, 0xf0 means dice 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number 0"))
    int32 m_iDiceNumberNowMask;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Untaken Slot Info"))
    FMyMJGameUntakenSlotInfoCpp m_cUntakenSlotInfo;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hai Di Card Id"))
    FMyIdValuePair m_cHelperLastCardTakenInGame; //hai di card if id >= 0

    //used to tell what rule this core is fixed to, not game cfg's type can be invalid, which means not started yet
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Rule Type"))
    MyMJGameRuleTypeCpp m_eRuleType;
};

USTRUCT(BlueprintType)
struct FMyMJCoreDataPublicDirectDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card infos to update"))
    TArray<FMyMJCardInfoCpp> m_aCardInfos2Update;

    //<= 0 means no update
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Action Group Id to update"))
    int32 m_iActionGroupId2Update;

    //invalid means no update
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State to update"))
    MyMJGameStateCpp m_eGameState2Update;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hai Di Card Id"))
    FMyIdValuePair m_cHelperLastCardTakenInGame; //hai di card if id >= 0
};

USTRUCT()
struct FMyMJCoreDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCoreDataPublicDirectCpp m_cDataPubicDirect;
};


UCLASS(NotBlueprintType)
class UMyMJCoreDataForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data public direct"))
    FMyMJCoreDataPublicDirectCpp m_cDataPubicDirect;
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

//Only Used for logic, not for visulzied
USTRUCT()
struct FMyMJAttenderDataLogicOnlyCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJAttenderDataLogicOnlyCpp()
    {
        setup(-1);
    };


    void setup(int32 idxAttender)
    {
        m_iIdx = idxAttender;
        m_cActionContainor.setup(idxAttender);
        m_cActionContainor.reinit(false);
        reset();
    };

    void reset()
    {
        m_iTurn = -1;
        m_cHandCards.clear();
        m_cActionContainor.resetForNewLoop();
    };

    //can be cast to MyMJGameRoleTypeCpp
    int32 m_iIdx;

    UPROPERTY(BlueprintReadWrite, NotReplicated, meta = (DisplayName = "turn"))
        int32 m_iTurn;

    //not need to duplicate, it is only used in core logic, not graphic
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "hand card Map"))
        FMyMJValueIdMapCpp m_cHandCards;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "action containor for choices"))
        FMyMJGameActionContainorCpp m_cActionContainor;
};

USTRUCT(BlueprintType)
struct FMyMJAttenderDataPublicDirectForBPCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJAttenderDataPublicDirectForBPCpp()
    {
        setup(-1);
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
        m_aIdHandCards.Reset();
        m_aIdJustTakenCards.Reset();
        m_aIdGivenOutCards.Reset();
        m_aShowedOutWeaves.Reset();
        m_aIdWinSymbolCards.Reset();
        m_cUntakenSlotSubSegmentInfo.reset();
        m_cHuScoreResultFinalGroup.reset();
        m_bIsRealAttender = false;
        m_bIsStillInGame = false;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx Attender"))
        int32 m_iIdxAttender; //it is the position

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id cards in hand"))
    TArray<int32> m_aIdHandCards;

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


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final"))
        FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroup;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "is real attender"))
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
        reset();
    };

    virtual ~FMyMJAttenderDataPrivateDirectForBPCpp()
    {

    };

    void reset()
    {
        m_cHuScoreResultTingGroup.reset();
        m_cCardValuePack.reset(0);
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "int test"))
    int32 m_iTest;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    FMyMJHuScoreResultTingGroupCpp  m_cHuScoreResultTingGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Card Value Pack"))
    FMyMJCardValuePackCpp m_cCardValuePack;

    //Todo: action containor for visualize
};

USTRUCT()
struct FMyMJAttenderDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJAttenderDataPublicDirectForBPCpp m_cDataPublicDirect;
    FMyMJAttenderDataPrivateDirectForBPCpp m_cDataPrivateDirect;
};


UCLASS(BlueprintType)
class UMyMJAttenderDataPublicForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data public direct"))
    FMyMJAttenderDataPublicDirectForBPCpp m_cDataPublicDirect;
};

UCLASS(BlueprintType)
class UMyMJAttenderDataPrivateForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data private direct"))
    FMyMJAttenderDataPrivateDirectForBPCpp m_cDataPrivateDirect;
};

UCLASS(BlueprintType)
class MYONLINECARDGAME_API UMyMJAttenderDataForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJAttenderDataForMirrorModeCpp()
    {
        m_pDataPublic = NULL;
        m_pDataPrivate = NULL;
    };

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    //virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;


    void createSubObjects(bool bBuildingDefault, bool bHavePrivate)
    {
        if (bBuildingDefault) {
            m_pDataPublic = CreateDefaultSubobject<UMyMJAttenderDataPublicForMirrorModeCpp>(TEXT("UMyMJAttenderDataPublicForMirrorModeCpp"));
            if (bHavePrivate) {
                m_pDataPrivate = CreateDefaultSubobject<UMyMJAttenderDataPrivateForMirrorModeCpp>(TEXT("UMyMJAttenderDataPublicForMirrorModeCpp"));
            }
        }
        else {
            m_pDataPublic = NewObject<UMyMJAttenderDataPublicForMirrorModeCpp>(this);
            if (bHavePrivate) {
                m_pDataPrivate = NewObject<UMyMJAttenderDataPrivateForMirrorModeCpp>(this);
            }
        }
    };

    //can be called multiple times
    void resetup(uint8 idxRole)
    {
        MY_VERIFY(m_pDataPublic != NULL);
        m_pDataPublic->m_cDataPublicDirect.setup(idxRole);
        if (m_pDataPrivate) {
            //m_pDataPrivate->m_cDataPrivateDirect.setup(idxRole);
        }
    };

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data public"))
    UMyMJAttenderDataPublicForMirrorModeCpp*  m_pDataPublic;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data private"))
    UMyMJAttenderDataPrivateForMirrorModeCpp* m_pDataPrivate;

protected:


};


//all mirror data doens't decide how game progress, just represent the game state
//Note: when using component, all outer must be actor
UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API UMyMJDataForMirrorModeCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJDataForMirrorModeCpp()
    {
        m_pCoreData = NULL;
        m_aAttenderDatas.Reset();
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    void createSubObjects(bool bBuildingDefault, bool bHavePrivate);

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "core data"))
    UMyMJCoreDataForMirrorModeCpp *m_pCoreData;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "attender datas"))
    TArray<UMyMJAttenderDataForMirrorModeCpp*>  m_aAttenderDatas;

protected:


};