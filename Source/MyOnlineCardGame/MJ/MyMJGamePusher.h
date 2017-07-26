// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/Class.h"
#include "UObject/NoExportTypes.h"

#include "Utils/MyMJUtils.h"
#include "MyMJCardPack.h"

#include "MyMJCommonDefines.h"

#include "MyMJGamePusher.generated.h"

//typedef struct FMyMJGamePusherPointersCpp FMyMJGamePusherPointersCpp;

//Every thing here need to be serialized
//we don't use netserilize, since we want custom serialize code for local file usage, and net-serialize is another code path, which means a double custom code work  

UENUM(BlueprintType)
enum class MyMJGameStateUpdateReasonCpp : uint8
{
    Invalid = 0                             UMETA(DisplayName = "Invalid"),
    NoCardLeft = 10                         UMETA(DisplayName = "NoCardLeft"),
    AttenderHu = 15                         UMETA(DisplayName = "AttenderHu"),
    DismissedByCmd = 50                     UMETA(DisplayName = "DismissedByCmd"),
};

UENUM(BlueprintType)
enum class MyMJGamePusherTypeCpp : uint8
{
    Invalid = 0                         UMETA(DisplayName = "Invalid"),
    PusherBase = 10                     UMETA(DisplayName = "PusherBase"),
    PusherFillInActionChoices = 11      UMETA(DisplayName = "PusherFillInActionChoices"),
    PusherMadeChoiceNotify = 12         UMETA(DisplayName = "PusherMadeChoiceNotify"),
    PusherCountUpdate = 13              UMETA(DisplayName = "PusherCountUpdate"),
    PusherResetGame = 20                UMETA(DisplayName = "PusherResetGame"),
    PusherUpdateCards = 25              UMETA(DisplayName = "PusherUpdateCards"), //mostly used in game end, reveal all card values
    PusherUpdateTing = 30               UMETA(DisplayName = "PusherUpdateTing"),

    ActionBase = 50                     UMETA(DisplayName = "ActionBase"),
    ActionStateUpdate = 51              UMETA(DisplayName = "ActionStateUpdate"),
    ActionNoAct = 60                    UMETA(DisplayName = "ActionNoAct"),
    ActionThrowDices = 71               UMETA(DisplayName = "ActionThrowDices"),
    ActionDistCardsAtStart = 72         UMETA(DisplayName = "ActionDistCardsAtStart"),

    ActionTakeCards = 75                UMETA(DisplayName = "ActionTakeCards"), //May take card normal, or buzhang, or gang
    ActionGiveOutCards = 76             UMETA(DisplayName = "ActionGiveOutCards"),
    ActionWeave = 80                    UMETA(DisplayName = "ActionWeave"),

    ActionHu = 90                       UMETA(DisplayName = "ActionHu"),

    ActionHuBornLocalCS = 150           UMETA(DisplayName = "ActionHuBornLocalCS"),
    ActionZhaNiaoLocalCS = 151          UMETA(DisplayName = "ActionZhaNiaoLocalCS"),

};

#define PriMyMJGameActionStateUpdate 200
#define PriMyMJGameActionNoAct 100
#define PriMyMJGameActionThrowDice 100
#define PriMyMJGameActionDistCardAtStart 100
#define PriMyMJGameActionTakeCards 100
#define PriMyMJGameActionGiveOutCards 100
#define PriMyMJGameActionWeaveShunZiMing 1500
#define PriMyMJGameActionWeaveKeZiMing 1600
#define PriMyMJGameActionWeaveGangMing 1700
#define PriMyMJGameActionWeaveGangAn   1750

#define PriMyMJGameActionHu 2000

#define PriMyMJGameActionHuBornLocalCS  100
#define PriMyMJGameActionZhaNiaoLocalCS 100

#define ActionGenTimeLeft2AutoChooseMsForImportant 4000


USTRUCT(BlueprintType)
struct FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherBaseCpp()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherBase;
        m_iId = 0;
    };

    virtual ~FMyMJGamePusherBaseCpp()
    {};

    //Never call this directly, since UE4 struct forbid pure virtual function, let's use fake function here to test it at runtime
    //return new one alloceted on heap and neccessary transform have done on it, or NULL to tip this role doesn't need to store it
    virtual FMyMJGamePusherBaseCpp* cloneDeep() const
    {
        MY_VERIFY(false);
        return NULL;
    };

    virtual FString genDebugString() const
    {
        //const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
        /*
        const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("MyMJGamePusherTypeCpp"), true);
        if (!enumPtr)
        {
            return FString("Invalid. ");
        }

        return enumPtr->GetEnumNameStringByValue((uint8)m_eType) + ". ";
        */
        return FString::Printf(TEXT("%s, %d."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGamePusherTypeCpp"), (uint8)m_eType), m_iId);
    };

    virtual void onReachedConsumeThread()
    {

    };

    //@outiRoleMask = 0 in return means no value will be updated
    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues)
    {
        outiRoleMask = 0;
    };

    inline
    MyMJGamePusherTypeCpp getType() const
    {
        return m_eType;
    };

    inline int32 getId() const
    {
        return m_iId;
    };


protected:

    friend class FMyMJGamePusherIOComponentFullCpp;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "type"))
    MyMJGamePusherTypeCpp m_eType;

    //pusher id, reset game is always ID 0
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "id"))
    int32 m_iId;
};

//A custom struct which we will implement serialize (default UE4 did not support TArray<struct *>)
//FIFO,like a queue, can transfer over thread, but never used it by multiple thread at one time
//two mode at runtime: canProduceAcrossThread() distinguish them
//two content mode: full sequence or segment
USTRUCT(BlueprintType)
struct FMyMJGamePusherPointersCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherPointersCpp()
    {
        //m_iCount = 0;
        m_iTestCount = 0;
        m_bSegmentClearTarget = false;

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("create  FMyMJGamePusherPointersCpp  %p."), this);
    };

    virtual ~FMyMJGamePusherPointersCpp()
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("destroy FMyMJGamePusherPointersCpp  %p, %d."), this, m_iTestCount);
        clear();
    };

    //Copy everything, this make it safe whenm another thread consume it
    void copyDeep(const FMyMJGamePusherPointersCpp *pOther);

    FMyMJGamePusherPointersCpp& operator = (const FMyMJGamePusherPointersCpp& rhs)
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGamePusherPointersCpp::operator = before , %p, %p; %d, %d."), this, &rhs, m_iTestCount, rhs.m_iTestCount);
        if (this == &rhs) {
            return *this;
        }

        copyDeep(&rhs);
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGamePusherPointersCpp::operator = after, %p, %d."), this, m_iTestCount);
        return *this;
    };

    bool operator== (const FMyMJGamePusherPointersCpp& rhs) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("FMyMJGamePusherPointersCpp::operator == , %p, %p; %d, %d."), this, &rhs, m_iTestCount, rhs.m_iTestCount);

        if (this == &rhs) {
            return true;
        }

        int32 l1 = this->m_aPushersSharedPtr.Num();
        int32 l2 = rhs.m_aPushersSharedPtr.Num();
        bool bCheck = false;
        if (l1 > 0 && l2 > 0) {
            bCheck = this->getGameIdVerified() == rhs.getGameIdVerified();
        }
        else {
            bCheck = l1 == 0;
        }
        return (bCheck && l1 == l2 && this->m_iTestCount == rhs.m_iTestCount && this->m_bSegmentClearTarget == rhs.m_bSegmentClearTarget);// && this->m_iCount == rhs.m_iCount);
    };

    void clear()
    {
        if (m_aPushersSharedPtr.Num() <= 0) {
            //not managed by SharedPtr
            int32 l = m_aPushers.Num();
            for (int32 i = 0; i < l; i++) {
                delete(m_aPushers[i]);
                m_aPushers[i] = NULL;
            }
        }

        m_aPushers.Empty();
        m_aPushersSharedPtr.Empty();

        //m_iCount = 0;
        m_iTestCount = 0;
    };

    void prepareForConsume()
    {
        if (m_aPushers.Num() > 0) {
            int32 l = m_aPushers.Num();
            for (int32 i = 0; i < l; i++) {
                m_aPushersSharedPtr.Emplace(m_aPushers[i]);
            }

            m_aPushers.Empty();
        }
    };

    inline bool canProduceAcrossThread() const
    {
        return (m_aPushersSharedPtr.Num() <= 0);
    }

    inline bool canProduceInLocalThreadSafely() const
    {
        return (m_aPushers.Num() <= 0);
    }

    //if empty, also return true
    bool isStartsAsFullSequence() const;

    //@pPusher must be created on heap, and its ownership will be transfered to this, can only be called on producer thread if consumer thread != producer thread
    //Can only be called before consume
    void give(FMyMJGamePusherBaseCpp *pPusher)
    {
        MY_VERIFY(canProduceAcrossThread());
        m_aPushers.Emplace(pPusher);

        //m_iCount++;
    };

    int32 insertInLocalThread(const TSharedPtr<FMyMJGamePusherBaseCpp> pPusher)
    {
        MY_VERIFY(canProduceInLocalThreadSafely());
        return m_aPushersSharedPtr.Emplace(pPusher);

        //m_iCount++;
    };

    inline
    int32 giveInLocalThread(const FMyMJGamePusherBaseCpp *pPusher)
    {
        return insertInLocalThread(MakeShareable<FMyMJGamePusherBaseCpp>((FMyMJGamePusherBaseCpp *)pPusher));
    };

    //Possible not inserted, return < 0 means skipped, usually used in Full Sequence Mode
    //@iGameId < 0 means not checking it
    int32 insertInLocalThreadWithLogicChecked(int32 iGameId, const TSharedPtr<FMyMJGamePusherBaseCpp> pPusher);

    //can only be called on consumer thread if consumer thread != producer thread, otherwise random crash
    TSharedPtr<FMyMJGamePusherBaseCpp> getSharedPtrAt(int32 idx)
    {
        prepareForConsume();

        MY_VERIFY(idx >= 0 && idx < m_aPushersSharedPtr.Num());

        return m_aPushersSharedPtr[idx];
    };

    const TSharedPtr<FMyMJGamePusherBaseCpp> getSharedPtrAtConst(int32 idx) const
    {
        MY_VERIFY(canProduceInLocalThreadSafely());
        MY_VERIFY(idx >= 0 && idx < m_aPushersSharedPtr.Num());

        return m_aPushersSharedPtr[idx];
    };

    /*
    FMyMJGamePusherBaseCpp* take()
    {
        FMyMJGamePusherBaseCpp *pRet = NULL;
        if (m_aPushers.Num() > 0) {
            pRet = m_aPushers[0];
            m_aPushers.RemoveAt(0);
        }
        else {

        }

        return pRet;
    };
    */

    inline int32 getCount() const
    {
        return m_aPushers.Num() + m_aPushersSharedPtr.Num();
    };


    //Warn: returned value is still onwned by this class, never store it or make sharedPtr on it
    //returned value can't be modified, its pointer can't be modified neither 
    inline const FMyMJGamePusherBaseCpp* const peekAt(int32 idx) const
    {
        MY_VERIFY(idx < getCount());

        if (m_aPushers.Num() > 0) {
            return m_aPushers[idx];
        }

        return m_aPushersSharedPtr[idx].Get();
    };

    FString genDebugString() const
    {
        int32 l = getCount();
        FString str = FString::Printf(TEXT("count: %d, %d."), l, m_iTestCount);
        for (int32 i = 0; i < l; i++) {
            str += FString::Printf(TEXT(" idx %d: "), i) + peekAt(i)->genDebugString();
        }

        return str;
    };

    int32 getGameIdVerified() const;

    //@pOutPusherIdLast, its value < 0 means not filled with valid pusher sequence
    void getGameIdAndPusherIdLast(int32 *pOutGameId, int32 *pOutPusherIdLast) const;

    //Simple check if it is the same head, so we can append new
    bool isSamePusherSequenceSimple(const FMyMJGamePusherPointersCpp &other) const;

    //simple copy other's data and may simple append data since the game logic ensure if head match it would be OK
    //return whether updated
    bool copyShallowAndLogicOptimized(const FMyMJGamePusherPointersCpp &other, bool bKeepDataIfOtherIsShorterWithSameSequence);


    //return whether updated
    bool helperFillAsSegmentFromQueue(TQueue<FMyMJGamePusherBaseCpp *, EQueueMode::Spsc>& pusherQueue);

    //@iGameIdOfSegment < 0 means not checked about gameId, always check game logic and append
    bool helperTryFillDataFromSegment(int32 iGameIdOfSegment, const FMyMJGamePusherPointersCpp &segment, bool bVerifyAllInserted);

    bool helperGenDeltaSegment(int32 iGameIdBase, int32 iPusherIdBase, int32 &outGameId, FMyMJGamePusherPointersCpp &outDeltaSegment);

    TSharedPtr<FMyMJGamePusherBaseCpp> helperTryPullPusher(int32 iGameId, int32 iPusherId);

    bool trySerializeWithTag(FArchive &Ar);

    bool Serialize(FArchive& Ar);
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
    bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms);

    //bool ExportTextItem(FString& ValueStr, FMyMJGamePusherPointersCpp const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;
    //bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* OwnerObject, FOutputDevice* ErrorText);
    bool SerializeFromMismatchedTag(FPropertyTag const& Tag, FArchive& Ar);

protected:

    //Only one member could be not full at one time, m_aPushers.Num() > 0 tips it is in produce mode,  m_aPushersSharedPtr.Num() > 0 tips it is in consume mode
    TArray<FMyMJGamePusherBaseCpp*> m_aPushers; //We don't use shared pointer unless it reaches consumer, and this class "owns" it
                                                //Because TSharedPtr is not thread safe, we assume one thread only consume it, and when it is used, it will not transfer over thread any more

    TArray<TSharedPtr<FMyMJGamePusherBaseCpp>> m_aPushersSharedPtr;

    //UPROPERTY()
    //int32 m_iCount; //also use it to trigger serilaize

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "test Count"))
    int32 m_iTestCount; //also use it to trigger serilaize

    UPROPERTY()
    int32 m_bSegmentClearTarget;
};

template<>
struct TStructOpsTypeTraits<FMyMJGamePusherPointersCpp> : public TStructOpsTypeTraitsBase2<FMyMJGamePusherPointersCpp>
{
    enum
    {
        //WithExportTextItem = true, // struct has an ExportTextItem function used to serialize its state into a string.
        //WithImportTextItem = true, // struct has an ImportTextItem function used to deserialize a string into an object of that class.
        WithSerializer = true,
        WithNetSerializer = true,
        //WithNetDeltaSerializer = true,
        WithIdenticalViaEquality = true,
        //WithSerializeFromMismatchedTag = true,
    };
};


USTRUCT(BlueprintType)
struct FMyMJGamePusherFillInActionChoicesCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherFillInActionChoicesCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherFillInActionChoices;

        m_iActionGroupId = 0;
        m_iIdxAttender = -1;
    };

    virtual ~FMyMJGamePusherFillInActionChoicesCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override;

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iActionGroupId: %d, m_iIdxAttender: %d."), m_iActionGroupId, m_iIdxAttender);
        str += m_cActionChoices.genDebugString();

        return str;
    };

    virtual void onReachedConsumeThread() override
    {
        m_cActionChoices.prepareForConsume();
    };

    void init(int32 iActionGroupId, int32 idxAttender)
    {
        m_iActionGroupId = iActionGroupId;
        m_iIdxAttender = idxAttender;
    };

    inline
    int32 getIdxAttender()
    {
        MY_VERIFY(m_iIdxAttender >= 0 && m_iIdxAttender < 4);
        return m_iIdxAttender;
    };

public:

    //Warn, this struct used FMyMJGamePusherPointersCpp, so use custom code for deep copy instead of operator =, so you need always modify the code when when add memebers
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "action Choices"))
    FMyMJGamePusherPointersCpp m_cActionChoices;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "action Group Id"))
    int32 m_iActionGroupId;

protected:

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx Attender"))
    int32 m_iIdxAttender;

};


USTRUCT(BlueprintType)
struct FMyMJGamePusherMadeChoiceNotifyCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherMadeChoiceNotifyCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherMadeChoiceNotify;
        m_iActionGroupId = 0;

        m_iSelection = 0;
        //m_iSelectionSub = 0;

        m_iIdxAttender = -1;
    };

    virtual ~FMyMJGamePusherMadeChoiceNotifyCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override;

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iActionGroupId: %d, m_iIdxAttender: %d, m_iSelection %d, m_aSubSelections.Num() %d."), m_iActionGroupId, m_iIdxAttender, m_iSelection, m_aSubSelections.Num());
        return str;
    };
     
    void init(int32 idxAttender, int32 iActionGroupId, int32 iSelection, TArray<int32> &aSubSelections)
    {
        m_iIdxAttender = idxAttender;
        m_iActionGroupId = iActionGroupId;

        m_iSelection = iSelection;
        m_aSubSelections = aSubSelections;
        //m_iSelectionSub = iSelectionSub;
    };

    inline
    int32& getIdxAttenderRef()
    {
        return m_iIdxAttender;
    };

public:
    // < 0 means unknown, the system have made it mask
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "selection"))
    int32 m_iSelection;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "sub Selections"))
    TArray<int32> m_aSubSelections;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "action Group Id"))
    int32 m_iActionGroupId;


protected:

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx Attender"))
    int32 m_iIdxAttender;
};


USTRUCT(BlueprintType)
struct FMyMJGamePusherCountUpdateCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherCountUpdateCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherCountUpdate;
        m_bActionGroupIncrease = false;
    };

    virtual ~FMyMJGamePusherCountUpdateCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGamePusherCountUpdateCpp *pRet = NULL;
        pRet = new FMyMJGamePusherCountUpdateCpp();

        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_bActionGroupIncrease: %d."), m_bActionGroupIncrease);
        return str;
    };

    UPROPERTY()
    bool m_bActionGroupIncrease;

};

//Also put on cards, this pusher is just like a base state setting the game to
//When in UI, let's first apply data then animate in UI
USTRUCT(BlueprintType)
struct FMyMJGamePusherResetGameCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherResetGameCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherResetGame;

        m_iAttenderBehaviorRandomSelectMask = 0;
    };

    virtual ~FMyMJGamePusherResetGameCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override;


    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iGameId: %d, ruleType %s."), m_iGameId, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameRuleTypeCpp"), (uint8)m_cGameCfg.m_eRuleType));
        return str;
    };

    //we do reset game special in code
    /*
    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = (1 << (uint8)MyMJGameRoleTypeCpp::SysKeeper);
        outaRevealedCardValues = m_aShuffledIdValues;
    };
    */

    //call this only when m_cGameCfg is set
    void init(int32 iGameId, FRandomStream &RS, FMyMJGameCfgCpp &cGameCfg, FMyMJGameRunDataCpp &cGameRunData, int32 iAttenderBehaviorRandomSelectMask);

    UPROPERTY()
    int32 m_iGameId;

    UPROPERTY()
    FMyMJGameCfgCpp m_cGameCfg;

    UPROPERTY()
    FMyMJGameRunDataCpp m_cGameRunData;

    UPROPERTY()
    TArray<int32> m_aShuffledValues;


    UPROPERTY()
    int32 m_iAttenderBehaviorRandomSelectMask;

};

#define MaskAttenderDataResetIdHandCardShowedOutLocalCS 0x01

USTRUCT(BlueprintType)
struct FMyMJGamePusherUpdateCardsCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherUpdateCardsCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherUpdateCards;
        m_iIdxAttender = -1;
        m_eCurrentState = MyMJCardFlipStateCpp::Invalid;
        m_eTargetState = MyMJCardFlipStateCpp::Invalid;
        m_iMaskAttenderDataReset = 0;
    };

    virtual ~FMyMJGamePusherUpdateCardsCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGamePusherUpdateCardsCpp *pRet = NULL;
        pRet = new FMyMJGamePusherUpdateCardsCpp();

        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iIdxAttender: %d, %s->%s, mask 0x%x"), m_iIdxAttender, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCardFlipStateCpp"), (uint8)m_eCurrentState), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCardFlipStateCpp"), (uint8)m_eTargetState), m_iMaskAttenderDataReset);
        int32 l = m_aIdValues.Num();
        for (int32 i = 0; i < l; i++) {
            str += m_aIdValues[i].genDebugStr();
        }
        return str;
    };

    void initWithCardsTargetStateAlreadyInited(int32 idxAttender, MyMJCardFlipStateCpp eCurrentState, MyMJCardFlipStateCpp eTargetState, int32 iMaskAttenderDataReset)
    {
        m_iIdxAttender = idxAttender;
        m_eCurrentState = eCurrentState;
        m_eTargetState = eTargetState;
        m_iMaskAttenderDataReset = iMaskAttenderDataReset;
    };

    //@outiRoleMask return 0 means not update any one's
    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = 0;
        outaRevealedCardValues.Reset();

        if (m_eTargetState == MyMJCardFlipStateCpp::Up) {
            if (m_eCurrentState == MyMJCardFlipStateCpp::Up) {
                return;
            }
            else {
                //we need to show everyone
                outiRoleMask = 0x0f | (1 << (uint8)MyMJGameRoleTypeCpp::Observer);
            }
        }
        else if (m_eTargetState == MyMJCardFlipStateCpp::Stand) {
            if (m_eCurrentState == MyMJCardFlipStateCpp::Down) {
                //we need to show owner and keeper
                outiRoleMask = (1 << m_iIdxAttender) | (1 << (uint8)MyMJGameRoleTypeCpp::Observer);
            }
            else {
                return;
            }
        }
        
        outaRevealedCardValues = m_aIdValues;
    };

    //Possible -1, means update cards only
    UPROPERTY()
    int32 m_iIdxAttender;

    UPROPERTY()
    MyMJCardFlipStateCpp m_eCurrentState;

    UPROPERTY()
    MyMJCardFlipStateCpp m_eTargetState;

    UPROPERTY()
    int32 m_iMaskAttenderDataReset;

    //possible empty when apply
    UPROPERTY()
    TArray<FMyIdValuePair> m_aIdValues;

};

USTRUCT(BlueprintType)
struct FMyMJGamePusherUpdateTingCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherUpdateTingCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::PusherUpdateTing;
        m_iIdxAttender = -1;
    };

    virtual ~FMyMJGamePusherUpdateTingCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        MY_VERIFY(m_iIdxAttender >= 0);

        FMyMJGamePusherUpdateTingCpp *pRet = NULL;
        pRet = new FMyMJGamePusherUpdateTingCpp();

        *pRet = *this;

        return pRet;

    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iIdxAttender: %d. %s"), m_iIdxAttender, *m_cTingGroup.genDebugString());
        return str;
    };

    void initWithTingGroupAlreadyInited(int32 idxAttender)
    {
        m_iIdxAttender = idxAttender;
    };

    UPROPERTY()
    int32 m_iIdxAttender;

    UPROPERTY()
    FMyMJHuScoreResultTingGroupCpp m_cTingGroup;

};


typedef struct FMyMJGameActionUnfiedForBPCpp FMyMJGameActionUnfiedForBPCpp;

USTRUCT(BlueprintType)
struct FMyMJGameActionBaseCpp : public FMyMJGamePusherBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionBaseCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionBase;
        m_iPriority = 0;
        m_iIdxAttender = 0;
        m_iTimeLeft2AutoChooseMs = 0;
    };

    virtual ~FMyMJGameActionBaseCpp()
    {};


    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iIdxAttender: %d, m_iPriority %d, m_iTimeLeft2AutoChoose %d."), m_iIdxAttender, m_iPriority, m_iTimeLeft2AutoChooseMs); //PRIu64 can't be used now since I didn't find a proper way to include the defines
        return str;
    };

    virtual int32 getRealCountOfSelection() const
    {
        return 1;
    };

    //return 0 if OK, transform this to target form, make sure this is stateless, means regargless how many times it is called, result is same
    virtual int32 makeSubSelection(TArray<int32> &subSelections)
    {
        MY_VERIFY(getRealCountOfSelection() >= 1);
        return 0;
    };

    virtual int32 genRandomSubSelections(FRandomStream &RS, TArray<int32> &outSubSelections)
    {
        return 0;
    };

    //called in full mode collected, before enqueue and apply this give a chance to fill in data
    virtual void resolveActionResult(FMyMJGameAttenderCpp &attender)
    {
        return;
    };

    //return true means this action have a valid represent, @poutActionUnified can be NULL fpr caller to test if this action have a valid unified form
    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified)
    {
        return false;
    };

    inline
    int32 getIdxAttender()
    {
        MY_VERIFY(m_iIdxAttender >= 0 && m_iIdxAttender < 4);
        return m_iIdxAttender;
    };

    inline
    int32 getPriority()
    {
        return m_iPriority;
    };

    inline
    int32& getIdxAttenderRef()
    {
        return m_iIdxAttender;
    };

    inline
    int32 getTimeLeft2AutoChoose()
    {
        return m_iTimeLeft2AutoChooseMs;
    };

    inline
    int32& getTimeLeft2AutoChooseRef()
    {
        return m_iTimeLeft2AutoChooseMs;
    };

    //used to determine priority when collect action, if return true, same action will always have only one picked as result
    virtual bool isAlwaysCheckDistWhenCalcPri() const
    {
        return false;
    };

protected:

    UPROPERTY()
    int32 m_iIdxAttender;

    UPROPERTY()
    int32 m_iPriority;

    //0 means instantly, < 0 means never
    UPROPERTY()
    int32 m_iTimeLeft2AutoChooseMs; //only used in full mode, not need to serialize, < 0 means not enabled
};


UENUM(Blueprintable, Meta = (Bitflags))
enum class EMyMJGameActionReserved0Mask : uint8
{
    PassPaoHu,
};

//Unified action present for graphic and UI
//If m_eType is actionBase, it means s stub to hold place
USTRUCT(BlueprintType)
struct FMyMJGameActionUnfiedForBPCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionUnfiedForBPCpp() : Super()
    {

    };

    virtual ~FMyMJGameActionUnfiedForBPCpp()
    {

    };

    UPROPERTY(BlueprintReadOnly)
    TArray<int32> m_aCardIds;

    UPROPERTY(BlueprintReadOnly)
    FMyMJWeaveCpp m_cWeave;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "reserved0", Bitmask, BitmaskEnum = "EMyMJGameActionReserved0Mask"))
    int32 m_iReserved0;
};


#define MyMJGameActionStateUpdateMaskNotResetHelperLastCardsGivenOutOrWeave 0x01

USTRUCT(BlueprintType)
struct FMyMJGameActionStateUpdateCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionStateUpdateCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionStateUpdate;
        m_iPriority = PriMyMJGameActionStateUpdate;

        m_eStateNext = MyMJGameStateCpp::Invalid; //Means not need to update
        m_iAttenderMaskNext = 0;
        m_bAllowSamePriAction = false;
        m_iIdxAttenderHavePriMax = 0;
        m_eReason = MyMJGameStateUpdateReasonCpp::Invalid;
        m_iMask = 0;
    };

    virtual ~FMyMJGameActionStateUpdateCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionStateUpdateCpp *pRet = NULL;
        pRet = new FMyMJGameActionStateUpdateCpp();

        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {

        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_eStateNext: %s, m_iAttenderMaskNext: %s, m_eReason: %s."),
               *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameStateCpp"), (uint8)m_eStateNext), *(UMyMJUtilsLibrary::formatMaskString(m_iAttenderMaskNext, 4)), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameStateUpdateReasonCpp"), (uint8)m_eReason));

        return str;
    };

    UPROPERTY()
    MyMJGameStateCpp m_eStateNext;

    UPROPERTY()
    int32 m_iAttenderMaskNext;

    UPROPERTY()
    bool m_bAllowSamePriAction;
    
    UPROPERTY()
    int32 m_iIdxAttenderHavePriMax;

    UPROPERTY()
    MyMJGameStateUpdateReasonCpp m_eReason;

    UPROPERTY()
    int32 m_iMask;
};

USTRUCT(BlueprintType)
struct FMyMJGameActionNoActCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionNoActCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionNoAct;
        m_iPriority = PriMyMJGameActionNoAct;

        m_iReserved0 = 0;

    };

    virtual ~FMyMJGameActionNoActCpp()
    {

    };

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionNoActCpp *pRet = NULL;
        pRet = new FMyMJGameActionNoActCpp();

        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {

        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iReserved0: %d."), m_iReserved0);
        return str;
    };

    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified) override
    {
        if (poutActionUnified) {
            FMyMJGameActionBaseCpp* pBase = StaticCast<FMyMJGameActionBaseCpp *>(poutActionUnified);
            *pBase = *this;
            poutActionUnified->m_iReserved0 = m_iReserved0;
        }

        return true;
    };

    inline
    void init(int32 idxAttender, int32 iReserved0, int32 iTimeLeft2AutoChooseMs, bool bForceActionGenTimeLeft2AutoChooseMsZero)
    {
        m_iIdxAttender = idxAttender;
        m_iReserved0 = iReserved0;

        if (!bForceActionGenTimeLeft2AutoChooseMsZero) {
            m_iTimeLeft2AutoChooseMs = iTimeLeft2AutoChooseMs;
        }
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "reserved0", Bitmask, BitmaskEnum = "EMyMJGameActionReserved0Mask"))
    int32 m_iReserved0;
};

UENUM(BlueprintType)
enum class MyMJGameActionThrowDicesSubTypeCpp : uint8
{
    Invalid = 0                             UMETA(DisplayName = "Invalid"),
    GameStart = 10                          UMETA(DisplayName = "GameStart"),
    GangYaoLocalCS = 50                     UMETA(DisplayName = "GangYaoLocalCS"),
};

USTRUCT(BlueprintType)
struct FMyMJGameActionThrowDicesCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionThrowDicesCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionThrowDices;
        m_iPriority = PriMyMJGameActionThrowDice;
        m_iDiceNumber0 = -1;
        m_iDiceNumber1 = -1;

        m_eSubType = MyMJGameActionThrowDicesSubTypeCpp::Invalid;

    };

    virtual ~FMyMJGameActionThrowDicesCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override;

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_iDiceNumber0: %d, m_iDiceNumber1: %d, subType %s."), m_iDiceNumber0, m_iDiceNumber1, *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJGameActionThrowDicesSubTypeCpp"), (uint8)m_eSubType));
        return str;
    };


    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified) override
    {
        if (m_eSubType == MyMJGameActionThrowDicesSubTypeCpp::GangYaoLocalCS) {
            if (poutActionUnified) {
                FMyMJGameActionBaseCpp* pBase = StaticCast<FMyMJGameActionBaseCpp *>(poutActionUnified);
                *pBase = *this;
            }

            return true;
        }
        else {
            return false;
        }


    };

    void init(MyMJGameActionThrowDicesSubTypeCpp eSubType, int32 idxAttender, FRandomStream &RS, bool bForceActionGenTimeLeft2AutoChooseMsZero);

    void getDiceNumbers(int32 &outDiceNumber0, int32 &outDiceNumber1) const;

    MyMJGameActionThrowDicesSubTypeCpp getSubType() const;


protected:

    UPROPERTY()
    int32 m_iDiceNumber0;

    UPROPERTY()
    int32 m_iDiceNumber1;

    UPROPERTY()
    MyMJGameActionThrowDicesSubTypeCpp m_eSubType;

};


USTRUCT(BlueprintType)
struct FMyMJGameActionDistCardAtStartCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionDistCardAtStartCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionDistCardsAtStart;
        m_iPriority = PriMyMJGameActionDistCardAtStart;

    };

    virtual ~FMyMJGameActionDistCardAtStartCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override;

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_bLastCard: %d. cards: "), m_bLastCard);

        str += UMyMJUtilsLibrary::formatStrIdValuePairs(m_aIdValues);

        return str;
    };

    void init(int32 idxAttender, const TArray<FMyIdValuePair> &aIdValues, bool bLastCard)
    {
        m_iIdxAttender = idxAttender;
        m_aIdValues = aIdValues;
        m_bLastCard = bLastCard;
    };

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = (1 << m_iIdxAttender);
        outaRevealedCardValues = m_aIdValues;
    };

    UPROPERTY()
    TArray<FMyIdValuePair> m_aIdValues;

    UPROPERTY()
    bool m_bLastCard;
};

UENUM(BlueprintType)
enum class MyMJGameCardTakenOrderCpp : uint8
{
    Invalid = 0                      UMETA(DisplayName = "Invalid"),
    Head = 1                         UMETA(DisplayName = "Head"),
    Tail = 2                         UMETA(DisplayName = "Tail"),
    NotFixed = 3                     UMETA(DisplayName = "NotFixed"),
};


USTRUCT(BlueprintType)
struct FMyMJGameActionTakeCardsCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionTakeCardsCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionTakeCards;
        m_iPriority = PriMyMJGameActionTakeCards;

        m_bIsGang = false;
        m_eTakenOrder = MyMJGameCardTakenOrderCpp::Invalid;

    };

    virtual ~FMyMJGameActionTakeCardsCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionTakeCardsCpp *pRet = new FMyMJGameActionTakeCardsCpp();
        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_bIsGang: %d, m_eTakenOrder %d."), m_bIsGang, (uint8)m_eTakenOrder);
        str += UMyMJUtilsLibrary::formatStrIdValuePairs(m_aIdValuePairs);

        return str;
    };

    virtual void resolveActionResult(FMyMJGameAttenderCpp &attender) override
    {
        int32 l = m_aIdValuePairs.Num();
        MY_VERIFY(l > 0);
        for (int32 i = 0; i < l; i++) {
            MY_VERIFY(m_aIdValuePairs[i].m_iId >= 0);
            MY_VERIFY(m_aIdValuePairs[i].m_iValue > 0);
        }
        return;
    }

    void initWithIdValuePairsInited(int32 idxAttender, bool bIsGang, MyMJGameCardTakenOrderCpp eTakenOrder)
    {
        m_iIdxAttender = idxAttender;
        m_bIsGang = bIsGang;
        m_eTakenOrder = eTakenOrder;
    };

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = (1 << m_iIdxAttender);
        outaRevealedCardValues = m_aIdValuePairs;
    };


    UPROPERTY()
    TArray<FMyIdValuePair> m_aIdValuePairs;

    UPROPERTY()
    bool m_bIsGang;

    UPROPERTY()
    MyMJGameCardTakenOrderCpp m_eTakenOrder;

};

USTRUCT(BlueprintType)
struct FMyMJGameActionGiveOutCardsCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionGiveOutCardsCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionGiveOutCards;
        m_iPriority = PriMyMJGameActionGiveOutCards;

        m_bRestrict2SelectCardsJustTaken = false;
        m_bIsGang = false;

    };

    virtual ~FMyMJGameActionGiveOutCardsCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionGiveOutCardsCpp *pRet = new FMyMJGameActionGiveOutCardsCpp();
        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_bRestrict2SelectCardsJustTaken %d, getRealCountOfSelection()t: %d. selected: %s."), m_bRestrict2SelectCardsJustTaken, getRealCountOfSelection(), *UMyMJUtilsLibrary::formatStrIdValuePairs(m_aIdValuePairsSelected));

        return str;
    };

    virtual int32 getRealCountOfSelection() const override
    {
        if (m_bRestrict2SelectCardsJustTaken) {
            return 1;
        }
        else {

            int32 count0 = m_aOptionIdsHandCard.Num();
            int32 count1 = m_aOptionIdsJustTaken.Num();
            int32 countAll = count0 + count1;
            MY_VERIFY(count1 == 0 || count1 == 1);
            MY_VERIFY(countAll > 0);
            return countAll;
        }
    };

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = 0x0f | (uint8)MyMJGameRoleTypeCpp::Observer;
        outaRevealedCardValues = m_aIdValuePairsSelected;
    };

    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified) override
    {
        if (m_bRestrict2SelectCardsJustTaken) {
            return false;
        }
        else {
            if (poutActionUnified) {

                FMyMJGameActionBaseCpp* pBase = StaticCast<FMyMJGameActionBaseCpp *>(poutActionUnified);
                *pBase = *this;
                poutActionUnified->m_aCardIds = m_aOptionIdsHandCard;
                poutActionUnified->m_aCardIds.Append(m_aOptionIdsJustTaken);
            }
            return true;
        }

    };

    //subSelection here contains card Id
    virtual int32 makeSubSelection(TArray<int32> &subSelections) override;

    //subSelection here contains card Id
    virtual int32 genRandomSubSelections(FRandomStream &RS, TArray<int32> &outSubSelections) override;

    virtual void resolveActionResult(FMyMJGameAttenderCpp &attender) override;

    void init(int32 idxAttender, TArray<int32> &aOptionIdsHandCard, TArray<int32> &aOptionIdsJustTaken, bool bRestrict2SelectCardsJustTaken, bool bIsGang)
    {
        m_iIdxAttender = idxAttender;
        m_aOptionIdsHandCard = aOptionIdsHandCard;
        m_aOptionIdsJustTaken = aOptionIdsJustTaken;

        m_bRestrict2SelectCardsJustTaken = bRestrict2SelectCardsJustTaken;

        if (m_bRestrict2SelectCardsJustTaken) {
            FMyIdValuePair::helperIds2IdValuePairs(m_aOptionIdsJustTaken, m_aIdValuePairsSelected);
        }

        m_bIsGang = bIsGang;
    };

    UPROPERTY()
    TArray<int32> m_aOptionIdsHandCard;

    UPROPERTY()
    TArray<int32> m_aOptionIdsJustTaken;

    UPROPERTY()
    TArray<FMyIdValuePair> m_aIdValuePairsSelected; //value will be resolved before enqueue automatically

    //if false, choose one card, if true, move out all cads just taken
    UPROPERTY()
    bool m_bRestrict2SelectCardsJustTaken;

    UPROPERTY()
    bool m_bIsGang;

};


USTRUCT(BlueprintType)
struct FMyMJGameActionWeaveCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionWeaveCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionWeave;
        m_iPriority = 0;

        m_eTargetFlipState = MyMJCardFlipStateCpp::Up;
    };

    virtual ~FMyMJGameActionWeaveCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionWeaveCpp *pRet = new FMyMJGameActionWeaveCpp();
        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += FString::Printf(TEXT(" m_eTargetFlipState: %s."), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJCardFlipStateCpp"), (uint8)m_eTargetFlipState));
        str += m_cWeave.genDebugString();

        return str;
    };

    virtual bool isAlwaysCheckDistWhenCalcPri() const override
    {
        return true;
    };

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = 0;

        if (m_eTargetFlipState == MyMJCardFlipStateCpp::Up) {
            outiRoleMask = 0x0f | (uint8)MyMJGameRoleTypeCpp::Observer;
            outaRevealedCardValues = m_cWeave.getIdValuesRef();
        }
    };

    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified) override
    {
        if (poutActionUnified) {
            FMyMJGameActionBaseCpp* pBase = StaticCast<FMyMJGameActionBaseCpp *>(poutActionUnified);
            *pBase = *this;
            poutActionUnified->m_cWeave = m_cWeave;
        }
        return true;
    };

    void initWithWeaveAlreadyInited(int32 idxAttender, MyMJCardFlipStateCpp eTargetFlipState)
    {
        m_iIdxAttender = idxAttender;
        m_eTargetFlipState = eTargetFlipState;
        MyMJWeaveTypeCpp eType = m_cWeave.getType();
        if (eType == MyMJWeaveTypeCpp::ShunZiMing) {
            m_iPriority = PriMyMJGameActionWeaveShunZiMing;
        }
        else if (eType == MyMJWeaveTypeCpp::KeZiMing) {
            m_iPriority = PriMyMJGameActionWeaveKeZiMing;
        }
        else if (eType == MyMJWeaveTypeCpp::GangMing) {
            m_iPriority = PriMyMJGameActionWeaveGangMing;
        }
        else if (eType == MyMJWeaveTypeCpp::GangAn) {
            m_iPriority = PriMyMJGameActionWeaveGangAn;
        }
        else {
            MY_VERIFY(false);
        }
    };

    //it is possible ingame hu not ending the game, such as, one attender exit as observer and other continue
    UPROPERTY()
    FMyMJWeaveCpp m_cWeave;

    //In some MJ rule, anGang need to be hide
    UPROPERTY()
    MyMJCardFlipStateCpp m_eTargetFlipState;

};



USTRUCT(BlueprintType)
struct FMyMJGameActionHuCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionHuCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionHu;
        m_iPriority = PriMyMJGameActionHu;
        m_bEndGame = true;
    };

    virtual ~FMyMJGameActionHuCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionHuCpp *pRet = new FMyMJGameActionHuCpp();
        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += m_cHuScoreResultFinalGroup.genDebugString();

        return str;
    };

    virtual void resolveActionResult(FMyMJGameAttenderCpp &attender) override;

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = 0x0f | (uint8)MyMJGameRoleTypeCpp::Observer;
        outaRevealedCardValues = m_aRevealingCards;
    };

    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified) override
    {
        if (poutActionUnified) {
            FMyMJGameActionBaseCpp* pBase = StaticCast<FMyMJGameActionBaseCpp *>(poutActionUnified);
            *pBase = *this;
        }
        return true;
    };

    void init(int32 idxAttender, bool bEndGame, FMyMJHuScoreResultFinalGroupCpp &finalGroup)
    {
        m_iIdxAttender = idxAttender;
        m_bEndGame = bEndGame;
        m_cHuScoreResultFinalGroup = finalGroup;

    };

    void initWithFinalGroupAlreadyInited(int32 idxAttender, bool bEndGame)
    {
        m_iIdxAttender = idxAttender;
        m_bEndGame = bEndGame;

        MY_VERIFY(m_cHuScoreResultFinalGroup.m_iIdxAttenderWin >= 0);

    };

    //it is possible ingame hu not ending the game, such as, one attender exit as observer and other continue
    UPROPERTY()
    bool m_bEndGame;

    UPROPERTY()
    FMyMJHuScoreResultFinalGroupCpp m_cHuScoreResultFinalGroup;

    UPROPERTY()
    TArray<FMyIdValuePair> m_aRevealingCards;
};


USTRUCT(BlueprintType)
struct FMyMJGameActionHuBornLocalCSCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionHuBornLocalCSCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionHuBornLocalCS;
        m_iPriority = PriMyMJGameActionHuBornLocalCS;
    };

    virtual ~FMyMJGameActionHuBornLocalCSCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionHuBornLocalCSCpp *pRet = NULL;
        pRet = new FMyMJGameActionHuBornLocalCSCpp();

        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += TEXT(" cards: ");
        str += UMyMJUtilsLibrary::formatStrIdValuePairs(m_aShowOutIdValues);

        int32 l = m_aHuScoreResultItems.Num();
        str += FString::Printf(TEXT(" huScoreItem.Num %d: "), l);
        for (int32 i = 0; i < l; i++) {
            const FMyMJHuScoreResultItemCpp *pItem = &(m_aHuScoreResultItems[i]);
            str += FString::Printf(TEXT("type %s, scorePerAttender %d, count %d"), *UMyMJUtilsLibrary::getStringFromEnum(TEXT("MyMJHuScoreTypeCpp"), (uint8)pItem->m_eType), pItem->m_iScorePerAttender, pItem->m_iCount);
        }

        return str;
    };

    virtual bool genActionUnified(FMyMJGameActionUnfiedForBPCpp *poutActionUnified) override
    {
        if (poutActionUnified) {
            FMyMJGameActionBaseCpp* pBase = StaticCast<FMyMJGameActionBaseCpp *>(poutActionUnified);
            *pBase = *this;
        }
        return true;
    };

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = 0x0f | (uint8)MyMJGameRoleTypeCpp::Observer;
        outaRevealedCardValues = m_aShowOutIdValues;
    };

    inline
    void init(int32 idxAttender, const TArray<FMyMJHuScoreResultItemCpp> &aHuScoreResultItems, const TArray<FMyIdValuePair> &aShowOutIdValues)
    {
        m_iIdxAttender = idxAttender;
        m_aHuScoreResultItems = aHuScoreResultItems;
        m_aShowOutIdValues = aShowOutIdValues;

    };

    UPROPERTY()
    TArray<FMyMJHuScoreResultItemCpp> m_aHuScoreResultItems;

    UPROPERTY()
    TArray<FMyIdValuePair> m_aShowOutIdValues;

};

USTRUCT(BlueprintType)
struct FMyMJGameActionZhaNiaoLocalCSCpp : public FMyMJGameActionBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionZhaNiaoLocalCSCpp() : Super()
    {
        m_eType = MyMJGamePusherTypeCpp::ActionZhaNiaoLocalCS;
        m_iPriority = PriMyMJGameActionZhaNiaoLocalCS;
    };

    virtual ~FMyMJGameActionZhaNiaoLocalCSCpp()
    {};

    virtual FMyMJGamePusherBaseCpp* cloneDeep() const override
    {
        FMyMJGameActionZhaNiaoLocalCSCpp *pRet = NULL;
        pRet = new FMyMJGameActionZhaNiaoLocalCSCpp();

        *pRet = *this;

        return pRet;
    };

    virtual FString genDebugString() const override
    {
        FString str = Super::genDebugString();
        str += TEXT(" cards: ");
        str += UMyMJUtilsLibrary::formatStrIdValuePairs(m_aPickedIdValues);

        return str;
    };

    virtual void getRevealedCardValues(int32 &outiRoleMask, TArray<FMyIdValuePair> &outaRevealedCardValues) override
    {
        outiRoleMask = 0x0f | (uint8)MyMJGameRoleTypeCpp::Observer;
        outaRevealedCardValues = m_aPickedIdValues;
    };

    inline
    void initWithPickedIdValuesInited(int32 idxAttender)
    {
        m_iIdxAttender = idxAttender;
    };


    UPROPERTY()
    TArray<FMyIdValuePair> m_aPickedIdValues;

};

/*
USTRUCT(BlueprintType)
struct FMyMJGameActionUnifiedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameActionUnifiedCpp()
    {
    };

    virtual ~FMyMJGameActionUnifiedCpp()
    {
    };
};

USTRUCT(BlueprintType)
struct FMyMJGamePusherUnifiedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherUnifiedCpp()
    {
    };

    virtual ~FMyMJGamePusherUnifiedCpp()
    {
        //clear();
    };
};

USTRUCT(BlueprintType)
struct FMyMJGamePusherUnitCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGamePusherUnitCpp()
    {
    };

    virtual ~FMyMJGamePusherUnitCpp()
    {
        //clear();
    };

    UPROPERTY()
    TArray<FMyMJGamePusherUnifiedCpp> m_aOptionPusher;

    UPROPERTY()
    TArray<FMyMJGameActionUnifiedCpp> m_aOptionAction;

    UPROPERTY()
    TArray<FMyMJGameActionUnifiedCpp> m_aOptionActionCollection;
};
*/



