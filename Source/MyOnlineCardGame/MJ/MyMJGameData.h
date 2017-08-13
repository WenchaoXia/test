// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MyMJCommonDefines.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.h"

#include "MyMJGamePushersIO.h"

#include "MyMJGameData.generated.h"

//Todo: use UE4's generated delta instead of my own

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

        m_iCfgStackNumKeptFromTail = 0;
    };


    bool isIdxUntakenSlotInKeptFromTailSegment(int32 idx) const
    {
        const FMyMJGameUntakenSlotInfoCpp *pInfo = this;

        int32 keptCount = pInfo->m_iCfgStackNumKeptFromTail;
        int32 idxTail = pInfo->m_iUntakenSlotLengthAtStart;
        int32 totalL = pInfo->m_iUntakenSlotLengthAtStart;

        if (keptCount <= 0) {
            return false;
        }

        MY_VERIFY(totalL > 0);
        MY_VERIFY(idxTail >= 0);

        int32 idxEnd = (idxTail + 1) % totalL; // use < 
        int32 idxStart = (idxEnd - keptCount + totalL) % totalL;

        int32 idxStartFix0, idxStartFix1, idxEndFix0, idxEndFix1;
        if (idxStart < idxEnd) {
            idxStartFix0 = idxStart;
            idxEndFix0 = idxEnd;
            idxStartFix1 = 0;
            idxEndFix1 = 0;
        }
        else {
            MY_VERIFY(idxStart != idxEnd);
            idxStartFix0 = idxStart;
            idxEndFix0 = totalL;
            idxStartFix1 = 0;
            idxEndFix1 = idxEnd;
        }

        return (idxStartFix0 <= idx && idx < idxEndFix0) || (idxStartFix1 <= idx && idx < idxEndFix1);
    }


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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "cfg stack num kept from tail"))
    int32 m_iCfgStackNumKeptFromTail; //Todo: set it when reset game
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
        m_eActionLoopState = MyMJActionLoopStateCpp::Invalid;
        m_cHelperShowedOut2AllCards.clear();
    };

public:

    int64 m_iMsLast;

    MyMJGameRuleTypeCpp m_eRuleType;//also distinguish sub type
    MyMJGameCoreWorkModeCpp m_eWorkMode;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Loop State"))
    MyMJActionLoopStateCpp m_eActionLoopState;

    //used to calculate how many cards left possible hu
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Cards Showed Out to All"))
        FMyMJValueIdMapCpp m_cHelperShowedOut2AllCards;
};

#define FMyMJCoreDataPublicDirectMask0_UpdateActionGroupId (1 << 1)
#define FMyMJCoreDataPublicDirectMask0_UpdateGameState (1 << 3)
//#define FMyMJCoreDataPublicDirectMask0_UpdateHelperLastCardsGivenOutOrWeave (1 << 5)
//#define FMyMJCoreDataPublicDirectMask0_UpdateHelperLastCardTakenInGame (1 << 7) //not needed. automatically set when removing card from untaken slot

//Atomic
//Both used for logic and visualize
//What the fuck is, UE3 network always send all structor data in one packet even some members in it haven't change, thanks to UE4 this changed, and if Atomic specified, it goes UE3's way
USTRUCT(BlueprintType)
struct FMyMJCoreDataPublicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJCoreDataPublicCpp()
    {
        reinit(MyMJGameRuleTypeCpp::Invalid);
    };

    virtual ~FMyMJCoreDataPublicCpp()
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

        m_iGameId = -1;
        m_iPusherIdLast = -1;
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


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game Id"))
        int32 m_iGameId;

    //the last pusher id we got
    UPROPERTY(BlueprintReadWrite, NotReplicated, meta = (DisplayName = "Pusher Id Last"))
        int32 m_iPusherIdLast;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Action Group Id"))
        int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
     MyMJGameStateCpp m_eGameState;

    //0xf means dice 0, 0xf0 means dice 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Dice Number 0"))
    int32 m_iDiceNumberNowMask;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Untaken Slot Info"))
    FMyMJGameUntakenSlotInfoCpp m_cUntakenSlotInfo;

    //When weave, it takes trigger card(if have) or 1st card per weave, value is possible invalid if card is not flip up(or valid if you have revealled it's value before)
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hai Di Card Id"))
    FMyIdValuePair m_cHelperLastCardTakenInGame; //hai di card if id >= 0

    //used to tell what rule this core is fixed to, not game cfg's type can be invalid, which means not started yet
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Rule Type"))
    MyMJGameRuleTypeCpp m_eRuleType;
};

//if not specified in member name, they are the target state
USTRUCT(BlueprintType)
struct FMyMJCoreDataDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCoreDataDeltaCpp()
    {
        m_iActionGroupId = 0;
        m_eGameState = MyMJGameStateCpp::Invalid;
        m_iMask0 = 0;
    };

    //if not empty, update them
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card infos"))
    TArray<FMyMJCardInfoCpp> m_aCardInfos2Update;

    //update to, target state
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Action Group Id"))
    int32 m_iActionGroupId;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Game State"))
    MyMJGameStateCpp m_eGameState;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Last Cards GivenOut Or Weave"))
    //TArray<FMyIdValuePair> m_aHelperLastCardsGivenOutOrWeave; //When weave, it takes trigger card(if have) or 1st card per weave

    //used for bit bool and bit tip updating as delta
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
};

USTRUCT()
struct FMyMJCoreDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCoreDataPublicCpp m_cDataPubicDirect;
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
    FMyMJCoreDataPublicCpp m_cDataPubicDirect;
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

#define FMyMJRoleDataAttenderPublicCpp_Mask0_IsRealAttender       (1 << 0)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsRealAttender (1 << 1)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_IsStillInGame        (1 << 2)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateIsStillInGame  (1 << 3)


#define FMyMJRoleDataAttenderPublicCpp_Mask0_GangYaoedLocalCS        (1 << 16)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateGangYaoedLocalCS  (1 << 17)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_BanPaoHuLocalCS         (1 << 18)
#define FMyMJRoleDataAttenderPublicCpp_Mask0_UpdateBanPaoHuLocalCS   (1 << 19)


USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPublicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPublicCpp()
    {
        setup(-1);
    };

    virtual ~FMyMJRoleDataAttenderPublicCpp()
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

        m_iMask0 = 0;
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

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final Local"))
    FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroupLocal;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final"))
    FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroup;

    //used for bit bool and bit tip updating as delta
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
};

USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPublicDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPublicDeltaCpp()
    {
        m_iMask0 = 0;
    };

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "weave to add"))
    TArray<FMyMJWeaveCpp> m_aWeave2Add;

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final Local"))
    TArray<FMyMJHuScoreResultFinalGroupCpp> m_aHuScoreResultFinalGroupLocal;

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Final"))
    TArray<FMyMJHuScoreResultFinalGroupCpp> m_aHuScoreResultFinalGroup;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "mask0"))
    int32 m_iMask0;
};


USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPrivateCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPrivateCpp()
    {
        reset();
    };

    virtual ~FMyMJRoleDataAttenderPrivateCpp()
    {

    };

    void reset()
    {
        m_cHuScoreResultTingGroup.reset();
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "int test"))
    int32 m_iTest;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    FMyMJHuScoreResultTingGroupCpp  m_cHuScoreResultTingGroup;

};


USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderPrivateDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderPrivateDeltaCpp()
    {

    };

    //if Num() > 0, it must equal to 1
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Hu Score Ting"))
    TArray<FMyMJHuScoreResultTingGroupCpp>  m_aHuScoreResultTingGroup;

};

USTRUCT(BlueprintType)
struct FMyMJRoleDataPrivateCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataPrivateCpp()
    {
        reset();
    };

    void reset()
    {
        m_cCardValuePack.reset(0);
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Card Value Pack"))
        FMyMJCardValuePackCpp m_cCardValuePack;
};

USTRUCT(BlueprintType)
struct FMyMJRoleDataPrivateDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataPrivateDeltaCpp()
    {
        m_eRoleType = MyMJGameRoleTypeCpp::Observer;
    };


    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card id values to reveal"))
    TArray<FMyIdValuePair> m_aIdValuePairs2Reveal;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role type it belong to"))
    MyMJGameRoleTypeCpp m_eRoleType;

    //Controls which role should have @m_aDataPrivate when clone/mute for different roles
    //when a role test return false, @m_aDataPrivate will be removed afer clone for that role
    int32 m_iRoleMaskForDataPrivateClone;



};

USTRUCT(BlueprintType)
struct FMyMJRoleDataAttenderDeltaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJRoleDataAttenderDeltaCpp()
    {
        m_iIdxAttender = -1;
    };

    //Num > 0 means valid, and num must equal 1 in that case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data public"))
    TArray<FMyMJRoleDataAttenderPublicDeltaCpp> m_aDataPublic;

    //Num > 0 means valid, and num must equal 1 in that case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "data private"))
    TArray<FMyMJRoleDataAttenderPrivateDeltaCpp> m_aDataPrivate;

    //equal to idx role, MyMJRoleTypeCpp
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx attender"))
    int32 m_iIdxAttender;
};

USTRUCT()
struct FMyMJRoleDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:

    void resetup(uint8 idxRole)
    {
        m_cDataAttenderPublic.setup(idxRole);
    };

    //although it is not every role have private data, but ususally it is not used in real time game network, so it is OK to have all for simple
    FMyMJRoleDataAttenderPublicCpp  m_cDataAttenderPublic;
    FMyMJRoleDataAttenderPrivateCpp m_cDataAttenderPrivate;
    FMyMJRoleDataPrivateCpp         m_cDataPrivate;
};


UCLASS(BlueprintType)
class UMyMJRoleDataAttenderPublicForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data attender public"))
    FMyMJRoleDataAttenderPublicCpp m_cDataAttenderPublic;
};

UCLASS(BlueprintType)
class UMyMJRoleDataAttenderPrivateForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data attender private"))
    FMyMJRoleDataAttenderPrivateCpp m_cDataAttenderPrivate;
};

UCLASS(BlueprintType)
class UMyMJRoleDataPrivateForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data private"))
    FMyMJRoleDataPrivateCpp m_cDataPrivate;
};

UCLASS(BlueprintType)
class MYONLINECARDGAME_API UMyMJRoleDataForMirrorModeCpp : public UObject
{
    GENERATED_BODY()

public:

    UMyMJRoleDataForMirrorModeCpp()
    {
        m_pDataAttenderPublic = NULL;
        m_pDataAttenderPrivate = NULL;
        m_pDataPrivate = NULL;
    };

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    //virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;


    void createSubObjects(bool bBuildingDefault, bool bIsAttender, bool bHavePrivate)
    {
        if (bBuildingDefault) {
            if (bIsAttender) {
                m_pDataAttenderPublic = CreateDefaultSubobject<UMyMJRoleDataAttenderPublicForMirrorModeCpp>(TEXT("UMyMJRoleDataAttenderPublicForMirrorModeCpp"));
                if (bHavePrivate) {
                    m_pDataAttenderPrivate = CreateDefaultSubobject<UMyMJRoleDataAttenderPrivateForMirrorModeCpp>(TEXT("UMyMJRoleDataAttenderPublicForMirrorModeCpp"));
                }
            }
            if (bHavePrivate) {
                m_pDataPrivate = CreateDefaultSubobject<UMyMJRoleDataPrivateForMirrorModeCpp>(TEXT("UMyMJRoleDataPrivateForMirrorModeCpp"));
            }
        }
        else {
            if (bIsAttender) {
                m_pDataAttenderPublic = NewObject<UMyMJRoleDataAttenderPublicForMirrorModeCpp>(this);
                if (bHavePrivate) {
                    m_pDataAttenderPrivate = NewObject<UMyMJRoleDataAttenderPrivateForMirrorModeCpp>(this);
                }
            }
            if (bHavePrivate) {
                m_pDataPrivate = NewObject<UMyMJRoleDataPrivateForMirrorModeCpp>(this);
            }
        }
    };

    //can be called multiple times
    void resetup(uint8 idxRole)
    {
        MY_VERIFY(m_pDataAttenderPublic != NULL);
        m_pDataAttenderPublic->m_cDataAttenderPublic.setup(idxRole);

        /*
        if (m_pDataPrivate) {
            //m_pDataPrivate->m_cDataPrivateDirect.setup(idxRole);
        }
        */
    };

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data attender public"))
    UMyMJRoleDataAttenderPublicForMirrorModeCpp*  m_pDataAttenderPublic;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data attender private"))
    UMyMJRoleDataAttenderPrivateForMirrorModeCpp* m_pDataAttenderPrivate;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "data private"))
    UMyMJRoleDataPrivateForMirrorModeCpp* m_pDataPrivate;

protected:


};


USTRUCT(BlueprintType)
struct FMyMJDataDeltaCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataDeltaCpp() : Super()
    {
        m_iGameId = 0;
        m_iServerWorldTimeStamp_10Ms = 0;
    };

    //Num > 0 means valid, and num must equal 1 in that case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "core data"))
    TArray<FMyMJCoreDataDeltaCpp> m_aCoreData;

    //Num > 0 means valid, and num must equal 1 in that case
    //represent whe attender's update, what he does to make game progress, private data is hidden to other
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role data attender"))
    TArray<FMyMJRoleDataAttenderDeltaCpp> m_aRoleDataAttender;

    //Num > 0 means valid, and num must equal 1 in that case
    //represent the role's update, how he knows the game,  it is different for different role
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "role data private"))
    TArray<FMyMJRoleDataPrivateDeltaCpp> m_aRoleDataPrivate;

    //in a idea env, game id and pusher id is not neccessary, we put it here just to detect if data goes wrong in network case
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "game id"))
    int32 m_iGameId;

    //how many 10ms passed, it = timescond * 100
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "time"))
    int32 m_iServerWorldTimeStamp_10Ms;
};

USTRUCT()
struct FMyMJDataForFullModeCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJDataForFullModeCpp()
    {
        for (int i = 0; i < (uint8)MyMJGameRoleTypeCpp::Max; i++) {
            int32 idx = m_aRoleDatas.Emplace();
            FMyMJRoleDataForFullModeCpp *pD = &m_aRoleDatas[idx];
            pD->resetup(i);
        }
    };

    inline FMyMJCoreDataPublicCpp& getCoreDataRef()
    {
        return const_cast<FMyMJCoreDataPublicCpp &>(getCoreDataRefConst());
    };

    inline const FMyMJCoreDataPublicCpp& getCoreDataRefConst() const
    {
        return m_cCoreData;
    };

    inline FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRef(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPublicCpp &>(getRoleDataAttenderPublicRefConst(idxAttender));
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRefConst(int32 idxAttender) const
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

        MY_VERIFY(m_aRoleDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        return m_aRoleDatas[idxAttender].m_cDataAttenderPublic;
    };


    inline FMyMJRoleDataAttenderPrivateCpp& getRoleDataAttenderPrivateRef(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPrivateCpp &>(getRoleDataAttenderPrivateRefConst(idxAttender));
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataAttenderPrivateCpp& getRoleDataAttenderPrivateRefConst(int32 idxAttender) const
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

        MY_VERIFY(m_aRoleDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        return m_aRoleDatas[idxAttender].m_cDataAttenderPrivate;
    };

    inline FMyMJRoleDataPrivateCpp& getRoleDataPrivateRef(int32 idxRole)
    {
        return const_cast<FMyMJRoleDataPrivateCpp &>(getRoleDataPrivateRefConst(idxRole));
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataPrivateCpp& getRoleDataPrivateRefConst(int32 idxRole) const
    {
        MY_VERIFY(idxRole >= 0 && idxRole < (uint8)MyMJGameRoleTypeCpp::Max);

        MY_VERIFY(m_aRoleDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        return m_aRoleDatas[idxRole].m_cDataPrivate;
    };

    FMyMJCoreDataPublicCpp m_cCoreData;
    TArray<FMyMJRoleDataForFullModeCpp> m_aRoleDatas;
};

//all mirror data doens't decide how game progress, just represent the game state
//Note: when using component, all outer must be actor
UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API UMyMJDataForMirrorModeCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    //we don't create members by default, since in client replication will do it, saves a memory allocate operation
    UMyMJDataForMirrorModeCpp()
    {
        m_pCoreData = NULL;
        m_aRoleDatas.Reset();
    };

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    inline FMyMJCoreDataPublicCpp& getCoreDataRef()
    {
        return const_cast<FMyMJCoreDataPublicCpp &>(getCoreDataRefConst());
    };

    //don't store the return, it can be invalid anytime
    inline const FMyMJCoreDataPublicCpp& getCoreDataRefConst() const
    {
        //mirror mode is supposed to work in game thread
        MY_VERIFY(IsInGameThread());
        MY_VERIFY(IsValid(m_pCoreData));
        return m_pCoreData->m_cDataPubicDirect;
    };

    inline FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRef(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPublicCpp &>(getRoleDataAttenderPublicRefConst(idxAttender));
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime
    const FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRefConst(int32 idxAttender) const
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

        //mirror mode is supposed to work in game thread
        MY_VERIFY(IsInGameThread());
        MY_VERIFY(m_aRoleDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        UMyMJRoleDataAttenderPublicForMirrorModeCpp *pD = m_aRoleDatas[idxAttender]->m_pDataAttenderPublic;
        MY_VERIFY(IsValid(pD));
        return pD->m_cDataAttenderPublic;
    };

    //can return NULL
    inline FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivate(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPrivateCpp *>(getRoleDataAttenderPrivateConst(idxAttender));
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime, can return NULL
    const FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivateConst(int32 idxAttender) const
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);

        //mirror mode is supposed to work in game thread
        MY_VERIFY(IsInGameThread());
        MY_VERIFY(m_aRoleDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        UMyMJRoleDataAttenderPrivateForMirrorModeCpp *pD = m_aRoleDatas[idxAttender]->m_pDataAttenderPrivate;
        if (IsValid(pD)) {
            return &pD->m_cDataAttenderPrivate;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("requesting role %d 's attender private data but it is NULL, this is not supposed to happen."), idxAttender);
            return NULL;
        }

    };

    inline FMyMJRoleDataPrivateCpp* getRoleDataPrivate(int32 idxRole)
    {
        return const_cast<FMyMJRoleDataPrivateCpp *>(getRoleDataPrivateConst(idxRole));
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataPrivateCpp* getRoleDataPrivateConst(int32 idxRole) const
    {
        MY_VERIFY(idxRole >= 0 && idxRole < (uint8)MyMJGameRoleTypeCpp::Max);

        MY_VERIFY(m_aRoleDatas.Num() == (uint8)MyMJGameRoleTypeCpp::Max);
        UMyMJRoleDataPrivateForMirrorModeCpp* pD = m_aRoleDatas[idxRole]->m_pDataPrivate;
        if (IsValid(pD)) {
            return &pD->m_cDataPrivate;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("requesting role %d 's private data but it is NULL, this is not supposed to happen."), idxRole);
            return NULL;
        }
    };


    void createSubObjects(bool bBuildingDefault, bool bHavePrivate);

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "core data"))
    UMyMJCoreDataForMirrorModeCpp *m_pCoreData;

    UPROPERTY(BlueprintReadWrite, Replicated, meta = (DisplayName = "role datas"))
    TArray<UMyMJRoleDataForMirrorModeCpp*>  m_aRoleDatas;

protected:


};

USTRUCT()
struct FMyMJDataAccessorCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJDataAccessorCpp()
    {
        m_pDataFullMode = NULL;
        m_pDataMirrorMode = NULL;
        m_eAccessRoleType = MyMJGameRoleTypeCpp::Observer;
        m_eWorkMode = MyMJGameCoreWorkModeCpp::Invalid;
    };

    MyMJGameRoleTypeCpp getAccessRoleType() const
    {
        //todo: make it conditional
        return m_eAccessRoleType;
    };

    void setupFullMode()
    {
        MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Invalid);
        m_eAccessRoleType = MyMJGameRoleTypeCpp::SysKeeper;
        m_eWorkMode = MyMJGameCoreWorkModeCpp::Full;
        m_pDataFullMode = MakeShareable<FMyMJDataForFullModeCpp>(new FMyMJDataForFullModeCpp());
    };

    //Can be called multiple times
    void resetupMirrorMode(MyMJGameRoleTypeCpp eAccessRoleType, UMyMJDataForMirrorModeCpp *pDataMirrorMode)
    {
        MY_VERIFY(m_eWorkMode != MyMJGameCoreWorkModeCpp::Full);
        m_eAccessRoleType = eAccessRoleType;
        m_eWorkMode = MyMJGameCoreWorkModeCpp::Mirror;
        m_pDataMirrorMode = pDataMirrorMode;
        MY_VERIFY(IsValid(m_pDataMirrorMode.Get()));
    };

    inline FMyMJCoreDataPublicCpp& getCoreDataRef()
    {
        return const_cast<FMyMJCoreDataPublicCpp &>(getCoreDataRefConst());
    };

    //don't store the return, it can be invalid anytime
    const FMyMJCoreDataPublicCpp& getCoreDataRefConst() const
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            return m_pDataFullMode->getCoreDataRefConst();
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {

            MY_VERIFY(IsValid(m_pDataMirrorMode.Get()));
            return m_pDataMirrorMode->getCoreDataRefConst();
        }
        else {
            MY_VERIFY(false);
        }

        return *(FMyMJCoreDataPublicCpp *)NULL;
    };

    inline FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRef(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPublicCpp &>(getRoleDataAttenderPublicRefConst(idxAttender));
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime
    const FMyMJRoleDataAttenderPublicCpp& getRoleDataAttenderPublicRefConst(int32 idxAttender) const
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            return m_pDataFullMode->getRoleDataAttenderPublicRefConst(idxAttender);
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {

            MY_VERIFY(IsValid(m_pDataMirrorMode.Get()));
            return m_pDataMirrorMode->getRoleDataAttenderPublicRefConst(idxAttender);
        }
        else {
            MY_VERIFY(false);
        }

        return *(FMyMJRoleDataAttenderPublicCpp *)NULL;
    };

    
    inline FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivate(int32 idxAttender)
    {
        return const_cast<FMyMJRoleDataAttenderPrivateCpp *>(getRoleDataAttenderPrivateConst(idxAttender));
    };

    //@idxAttender equal to idxRole
    //don't store the return, it can be invalid anytime, can return NULL
    const FMyMJRoleDataAttenderPrivateCpp *getRoleDataAttenderPrivateConst(int32 idxAttender) const
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            return &m_pDataFullMode->getRoleDataAttenderPrivateRefConst(idxAttender);
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {

            MY_VERIFY(IsValid(m_pDataMirrorMode.Get()));
            return m_pDataMirrorMode->getRoleDataAttenderPrivateConst(idxAttender);
        }
        else {
            MY_VERIFY(false);
        }

        return NULL;
    };

    inline FMyMJRoleDataPrivateCpp* getRoleDataPrivate(int32 idxRole)
    {
        return const_cast<FMyMJRoleDataPrivateCpp *>(getRoleDataPrivateConst(idxRole));
    };

    //@idxAttender equal to idxRole
    inline const FMyMJRoleDataPrivateCpp* getRoleDataPrivateConst(int32 idxRole) const
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Full) {
            return &m_pDataFullMode->getRoleDataPrivateRefConst(idxRole);
        }
        else if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {

            MY_VERIFY(IsValid(m_pDataMirrorMode.Get()));
            return m_pDataMirrorMode->getRoleDataPrivateConst(idxRole);
        }
        else {
            MY_VERIFY(false);
        }

        return NULL;
    };

    //Note all data will be overwritten, include default values, usually used in full mode, but also possible for mirror as one way to implement replay  
    void applyBase(const FMyMJDataForFullModeCpp &base)
    {
        if (m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror) {
            //now this is not supposed to happen unless low level replay method is used
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("apply base but mode is mirror, check it!"), );
        }

        getCoreDataRef() = base.getCoreDataRefConst();

        int32 l = base.m_aRoleDatas.Num();
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);
        for (int i = 0; i < l; i++) {

            getRoleDataAttenderPublicRef(i) = base.getRoleDataAttenderPublicRefConst(i);

            {
                FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivSelf = getRoleDataAttenderPrivate(i);
                const FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivOther = &base.getRoleDataAttenderPrivateRefConst(i);
                if (pAttenderPrivSelf && pAttenderPrivOther) {
                    *pAttenderPrivSelf = *pAttenderPrivOther;
                }
                else {
                    //it is OK, but in a fine design this should be avoid
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's attender private data is NULL, self %p, dest %p."), i, pAttenderPrivSelf, pAttenderPrivOther);
                }
            }

            {
                FMyMJRoleDataPrivateCpp *pPrivSelf = getRoleDataPrivate(i);
                const FMyMJRoleDataPrivateCpp *pPrivOther = &base.getRoleDataPrivateRefConst(i);
                if (pPrivSelf && pPrivOther) {
                    *pPrivSelf = *pPrivOther;
                }
                else {
                    //it is OK, but in a fine design this should be avoid
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's private data is NULL, self %p, dest %p."), i, pPrivSelf, pPrivOther);
                }
            }
        }

    }


    void applyBase(const UMyMJDataForMirrorModeCpp &base)
    {
        MY_VERIFY(m_eWorkMode == MyMJGameCoreWorkModeCpp::Mirror);

        getCoreDataRef() = base.getCoreDataRefConst();

        int32 l = base.m_aRoleDatas.Num();
        MY_VERIFY(l == (uint8)MyMJGameRoleTypeCpp::Max);
        for (int i = 0; i < l; i++) {

            getRoleDataAttenderPublicRef(i) = base.getRoleDataAttenderPublicRefConst(i);

            {
                FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivSelf = getRoleDataAttenderPrivate(i);
                const FMyMJRoleDataAttenderPrivateCpp *pAttenderPrivOther = base.getRoleDataAttenderPrivateConst(i);
                if (pAttenderPrivSelf && pAttenderPrivOther) {
                    *pAttenderPrivSelf = *pAttenderPrivOther;
                }
                else {
                    //it is OK, but in a fine design this should be avoid
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's attender private data is NULL, self %p, dest %p."), i, pAttenderPrivSelf, pAttenderPrivOther);
                }
            }

            {
                FMyMJRoleDataPrivateCpp *pPrivSelf = getRoleDataPrivate(i);
                const FMyMJRoleDataPrivateCpp *pPrivOther = base.getRoleDataPrivateConst(i);
                if (pPrivSelf && pPrivOther) {
                    *pPrivSelf = *pPrivOther;
                }
                else {
                    //it is OK, but in a fine design this should be avoid
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("role %d 's private data is NULL, self %p, dest %p."), i, pPrivSelf, pPrivOther);
                }
            }
        }
    };

    //the delta myst be applied by two step, and after step 0 it can be visualized, can be used by both full and mirror mode
    //after step 0 we have a chance for visualize, before calling next
    void applyDeltaStep0(const FMyMJDataDeltaCpp &delta);
    void applyDeltaStep1(const FMyMJDataDeltaCpp &delta);

    //direct to apply, don't leave a chance for visualize
    void applyDelta(const FMyMJDataDeltaCpp &delta);

protected:

    FMyMJCardValuePackCpp& getCardValuePackRef()
    {
        int32 iAccessRoleType = (uint8)getAccessRoleType();

        FMyMJRoleDataPrivateCpp* pDRolePriv = getRoleDataPrivate(iAccessRoleType);

        MY_VERIFY(pDRolePriv);

        return pDRolePriv->m_cCardValuePack;
    };

    void moveCardFromOldPosi(int32 id);
    void moveCardToNewPosi(int32 id, int32 idxAttender, MyMJCardSlotTypeCpp eSlotDst);
    void updateUntakenInfoHeadOrTail(bool bUpdateHead, bool bUpdateTail);

    //the really storage we point to
    TSharedPtr<FMyMJDataForFullModeCpp> m_pDataFullMode;
    TWeakObjectPtr<UMyMJDataForMirrorModeCpp> m_pDataMirrorMode;

    MyMJGameRoleTypeCpp m_eAccessRoleType;
    MyMJGameCoreWorkModeCpp m_eWorkMode;
};