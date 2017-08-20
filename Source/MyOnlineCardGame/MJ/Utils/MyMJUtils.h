// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Utils/CommonUtils/MyCardUtils.h"


#include "MyMJUtils.generated.h"

#define MY_MJCARD_ID_FAKE -1

UENUM(BlueprintType)
enum class MyMJCardValueTypeCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    WangZi = 1 UMETA(DisplayName = "WangZi"), //Xushu card, [1, 9]
    BingZi = 2 UMETA(DisplayName = "BingZi"), // [11, 19]
    TiaoZi = 3 UMETA(DisplayName = "TiaoZi"), // [21, 29]
    Feng = 4 UMETA(DisplayName = "Feng"), //Word Card:  Dong Nan Xi Bei, [31, 34] 
    Jian = 5 UMETA(DisplayName = "Jian"), //Zhong Fa Bai, [41, 43]
    Hua = 6 UMETA(DisplayName = "Hua"), //Hua Card: Chun Xia Qiu Dong, Mei Lan Zhu Ju, [51, 58]
    Max = 7 UMETA(DisplayName = "Max"), //

    //following are extended, not basic types, don't use it in card logic unless statis
    ExtNum258 = 10 UMETA(DisplayName = "ExtNum258"),
    ExtNum19 = 11 UMETA(DisplayName = "ExtNum19"),
    ExtNumLit = 12 UMETA(DisplayName = "ExtNumLit"),
    ExtNumMid = 13 UMETA(DisplayName = "ExtNumMid"),
    ExtNumBig = 14 UMETA(DisplayName = "ExtNumBig"),
    ExtNumLessThan5 = 15 UMETA(DisplayName = "ExtNumLessThan5"),
    ExtNumBiggerThan5 = 16 UMETA(DisplayName = "ExtNumBiggerThan5"),
    ExtNumDouble = 17 UMETA(DisplayName = "ExtNumDouble"),
    ExtBuDao = 18 UMETA(DisplayName = "ExtBuDao"),
    ExtLv = 19 UMETA(DisplayName = "ExtLv"),
    ExtMax = 20 UMETA(DisplayName = "ExtMax")
};

UENUM()
enum class MyMJChiTypeCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    ChiLeft = 1 UMETA(DisplayName = "ChiLeft"),
    ChiMid = 2 UMETA(DisplayName = "ChiMid"),
    ChiRight = 3 UMETA(DisplayName = "ChiRight"),
    Max = 4 UMETA(DisplayName = "Max")
};

UENUM(BlueprintType)
enum class MyMJWeaveTypeCpp : uint8
{
    Invalid = 0 UMETA(DisplayName = "Invalid"),
    DuiZi = 1 UMETA(DisplayName = "DuiZi(Jiang)"), //DuiZi is always an

    ShunZiAn = 5 UMETA(DisplayName = "ShunZiAn"), //means in hand
    ShunZiMing = 6 UMETA(DisplayName = "ShunZiMing"), //means chi

    KeZiAn = 10    UMETA(DisplayName = "KeZiAn"), //means in hand
    KeZiMing = 11  UMETA(DisplayName = "KeZiMing(Peng)"), //means peng

    GangAn = 15 UMETA(DisplayName = "GangAn"), //means ganged out, but dark (all from hand). Only one exception: when parsing hand card, this means four cards not formed a weave showed out
    GangMing = 16 UMETA(DisplayName = "GangMing"), //means ganged out, at least one card is from other, of peng before 

    SpecialLong = 30 UMETA(DisplayName = "SpecialLong"),
    SpecialUnWeavedCards = 31 UMETA(DisplayName = "SpecialUnWeavedCards") //used to present the trival card when showing the special weaved cards
    //MTE_LAST = 0x80000000 UMETA(Hidden)
};

/* the hu card type is like mutex, you can only hu with one type one time.*/
UENUM()
enum class MyMJHuCardTypeCpp : uint8
{
    Invalid = 0                      UMETA(DisplayName = "Invalid"),

    //Standarded Hu
    CommonHu = 1                     UMETA(DisplayName = "CommonHu"),
    Special7Dui = 2                  UMETA(DisplayName = "Special7Dui"),
    Special13Yao = 3                 UMETA(DisplayName = "Special13Yao"),
    Special13BuKao = 4               UMETA(DisplayName = "Special13BuKao"),
    SpecialZuHeLong = 5              UMETA(DisplayName = "SpecialZuHeLong"),

    //Localied Hu
    LocalCSAllJiang258 = 100         UMETA(DisplayName = "LocalCSAllJiang258"),

    //Born local
    LocalCSBornNoJiang258 = 150      UMETA(DisplayName = "LocalCSBornNoJiang258"),
    LocalCSBornLackNumCardType = 151 UMETA(DisplayName = "LocalCSBornLackNumCardType"),
    LocalCSBorn2KeZi = 152           UMETA(DisplayName = "LocalCSBorn2KeZi"),
    LocalCSBornAnGangInHand = 153            UMETA(DisplayName = "LocalCSBornAnGangInHand")
    //MTE_LAST = 0x80000000 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class MyMJCardFlipStateCpp : uint8
{
    Invalid = 0                    UMETA(DisplayName = "Invalid"),

    Down = 1                       UMETA(DisplayName = "Down"),
    Stand = 2                      UMETA(DisplayName = "Stand"),
    Up = 3                        UMETA(DisplayName = "Up")

};

UENUM(BlueprintType)
enum class MyMJCardSlotTypeCpp : uint8
{
    Invalid = 0                    UMETA(DisplayName = "Invalid"),

    //Keeper = 1                       UMETA(DisplayName = "Keeper"),
    Untaken = 2                      UMETA(DisplayName = "Untaken"),
    JustTaken = 3                      UMETA(DisplayName = "JustTaken"),
    InHand = 4                        UMETA(DisplayName = "InHand"),
    GivenOut = 5                        UMETA(DisplayName = "GivenOut"),
    Weaved = 6                        UMETA(DisplayName = "Weaved"),
    WinSymbol = 7                        UMETA(DisplayName = "WinSymbol")
};

//Todo, to support memset as init, use 0 as default for all to make it run faster
USTRUCT(BlueprintType)
struct FMyMJCardPosiCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJCardPosiCpp() {
        reset();
    };

    void reset() {
        m_iIdxAttender = -1;
        m_eSlot = MyMJCardSlotTypeCpp::Invalid;
        resetMinorData();
    };

    inline void resetMinorData() {
        m_iIdxInSlot0 = -1;
        m_iIdxInSlot1 = -1;
    };

    inline
    bool equal(const FMyMJCardPosiCpp &other) const
    {
        return m_iIdxAttender == other.m_iIdxAttender && m_eSlot == other.m_eSlot && m_iIdxInSlot0 == other.m_iIdxInSlot0 && m_iIdxInSlot1 == other.m_iIdxInSlot1;
    };

    //Note: this may not work as expect for sub class, if you use base ponnter*: for example, class A, class B : public A, A* pA0 = new B(), *pA ==
    //to make it work good, we need to use virtual functon as equal(), and compare double time by this->equal(other) && other.equal(*this)
    //But for simple, we don't handle that case now, but keep in mind what should happen when programming, esp for pointer use case.
    bool operator==(const FMyMJCardPosiCpp& other) const
    {
        if (this == &other) {
            return true;
        }
        return equal(other);
    };

    bool operator!=(const FMyMJCardPosiCpp& other) const
    {
        return !(*this == other);
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx attender"))
        int32 m_iIdxAttender;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card slot"))
        MyMJCardSlotTypeCpp m_eSlot;

    // < 0 means invalid
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx in slot 0"))
        int32 m_iIdxInSlot0;

    // < 0 means invalid
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "idx in slot 1"))
        int32 m_iIdxInSlot1;

};

USTRUCT(BlueprintType)
struct FMyMJCardInfoCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJCardInfoCpp()
    {
        reset();
    };

    void reset() {
        m_iId = -1;

        m_eFlipState = MyMJCardFlipStateCpp::Invalid;
        m_cPosi.reset();
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "id"))
        int32 m_iId; // >= 0 means valid

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "flip state"))
        MyMJCardFlipStateCpp m_eFlipState;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "position"))
        FMyMJCardPosiCpp m_cPosi;
};

USTRUCT(BlueprintType)
struct FMyMJCardCpp : public FMyMJCardInfoCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJCardCpp() : FMyMJCardInfoCpp()
    {
        m_iValue = 0;
    };

    void reset()
    {
        Super::reset();
        m_iValue = 0;
    };

    void fill(const FMyMJCardInfoCpp &info)
    {
        *StaticCast<FMyMJCardInfoCpp *>(this) = info;
    };

    void convert2IdValuePair(FMyIdValuePair &out) const
    {
        out.m_iId = m_iId;
        out.m_iValue = m_iValue;
    };

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "value"))
    int32 m_iValue;
};


USTRUCT(BlueprintType)
struct FMyMJCardValuePackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardValuePackCpp()
    {
        reset(0);
    };

    virtual ~FMyMJCardValuePackCpp()
    {

    };

    void reset(int32 iCardNum)
    {
        MY_VERIFY(iCardNum >= 0);
        m_aCardValues.Reset(iCardNum);
        m_aCardValues.AddZeroed(iCardNum);
    };

    //this will reveal all card values
    void resetAndRevealAll(const TArray<int32> &aShuffledValues)
    {
        m_aCardValues = aShuffledValues;
    };

    inline
        int32 getLength() const
    {
        return m_aCardValues.Num();
    };

    inline
        int32 getByIdx(int32 idx, bool bVerifyValueValid = false) const
    {
        MY_VERIFY(idx >= 0 && idx < m_aCardValues.Num());
        int32 v = m_aCardValues[idx];
        if (bVerifyValueValid) {
            MY_VERIFY(v > 0);
        }
        return v;
    };

    inline
        int32 &getByIdxRef(int32 idx)
    {
        MY_VERIFY(idx >= 0 && idx < m_aCardValues.Num());
        return m_aCardValues[idx];
    };

    //this function will check values, and assert if error happens
    void helperVerifyValues() const;

    void tryRevealCardValue(int32 id, int32 value);

    inline void tryRevealCardValue(const FMyIdValuePair &idValuePair)
    {
        tryRevealCardValue(idValuePair.m_iId, idValuePair.m_iValue);
    };

    inline void tryRevealCardValueByIdValuePairs(const TArray<FMyIdValuePair> &aIdValuePairs)
    {
        int32 l = aIdValuePairs.Num();
        for (int32 i = 0; i < l; i++) {
            tryRevealCardValue(aIdValuePairs[i]);
        }
    };


    void getValuesByIds(const TArray<int32> &aIds, TArray<int32> &outaValues, bool bVerifyValueValid = false) const;

    //will fiill in with card value
    void helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs, bool bVerifyValueValid = false) const;

    void helperResolveValue(FMyIdValuePair &pair, bool bVerifyValueValid = false) const;

    void helperResolveValues(TArray<FMyIdValuePair> &aPairs, bool bVerifyValueValid = false) const;



protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card values"))
        TArray<int32> m_aCardValues;
};

UENUM(Blueprintable, Meta = (Bitflags))
enum class EMyMJWeaveReserved0Mask : uint8
{
    LocalCSGangBuZhang = 0x01, //1 means BuZhang, 0 means GangYao
};

//Id should be unique always

/*
USTRUCT(BlueprintType)
struct FMyMJWeaveCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJWeaveCpp()
    {
        reset();
    };

    void reset() {
        m_eType = MyMJWeaveTypeCpp::Invalid;
        m_aIdValues.Reset();

        m_iIdTriggerCard = -1;
        m_iIdxAttenderTriggerCardSrc = -1;
        m_eTypeConsumed = MyMJWeaveTypeCpp::Invalid;
        m_iReserved0 = 0;
    };

    void addCard(int32 id, int32 value)
    {
        int32 idx = m_aIdValues.Emplace();
        m_aIdValues[idx].m_iId = id;
        m_aIdValues[idx].m_iValue = value;
    };

    void addCard(const FMyIdValuePair &idValue)
    {
        addCard(idValue.m_iId, idValue.m_iValue);
    };

    void addCard(const FMyMJCardCpp &card)
    {
        addCard(card.m_iId, card.m_iValue);
    };

    void appendIdsValues(TArray<int32> aIds, TArray<int32> aValues)
    {
        int32 l = aIds.Num();
        MY_VERIFY(l == aValues.Num());

        for (int32 i = 0; i < l; i++) {
            addCard(aIds[i], aValues[i]);
        }
    };

    inline
    void appendIdVakuePairs(const TArray<FMyIdValuePair> &aIdValuePairs)
    {
        m_aIdValues.Append(aIdValuePairs);
    };

    inline
    void appendIdVakuePairsWithLimit(const TArray<FMyIdValuePair> &aIdValuePairs, int32 maxLimit)
    {
        MY_VERIFY(maxLimit > 0);
        int32 count = aIdValuePairs.Num();
        if (count <= maxLimit) {
            m_aIdValues.Append(aIdValuePairs);
        }
        else {
            for (int32 i = 0; i < maxLimit; i++) {
                m_aIdValues.Emplace(aIdValuePairs[i]);
            }
        }
    };

    bool removeById(int32 id)
    {
        int32 idxFound = -1;
        int32 l2 = m_aIdValues.Num();
        for (int32 j = 0; j < l2; j++) {
            if (m_aIdValues[j].m_iId == id) {
                MY_VERIFY(idxFound < 0); //for debug purpose, don't break but check the whether it is unique
                idxFound = j;
                //break;
            }
        }

        if (idxFound >= 0) {
            m_aIdValues.RemoveAt(idxFound);
            return true;
        }
        else {
            return false;
        }
    };

    inline MyMJWeaveTypeCpp& getTypeRef()
    {
        return m_eType;
    };

    inline MyMJWeaveTypeCpp getType() const
    {
        return m_eType;
    };

    inline const TArray<FMyIdValuePair>& getIdValuesRef() const
    {
        return m_aIdValues;
    };

    inline const void getIds(TArray<int32>& outIds) const
    {
        outIds.Reset();
        int32 l = m_aIdValues.Num();
        for (int32 i = 0; i < l; i++) {
            outIds.Emplace(m_aIdValues[i].m_iId);
        }
    };

    inline int32& getIdTriggerCardRef()
    {
        return m_iIdTriggerCard;
    };

    inline int32& getIdxAttenderTriggerCardSrcRef()
    {
        return m_iIdxAttenderTriggerCardSrc;
    };

    inline MyMJWeaveTypeCpp getTypeConsumed() const
    {
        return m_eTypeConsumed;
    };

    inline int32 getReserved0() const
    {
        return m_iReserved0;
    };

    inline int32& getReserved0Ref()
    {
        return m_iReserved0;
    };

    inline int32 getCount() const
    {
        return m_aIdValues.Num();
    };

    int32 getMidValue() const
    {
        int32 l = m_aIdValues.Num();
        MY_VERIFY(l > 0);

        if (getType() == MyMJWeaveTypeCpp::ShunZiAn || getType() == MyMJWeaveTypeCpp::ShunZiMing) {
            int vall = 0;
            for (int32 i = 0; i < l; i++) {
                vall += m_aIdValues[i].m_iValue;
            }

            return vall / l;
        }
        else {
            return m_aIdValues[0].m_iValue;
        }
    };

    //return trigger card , or 1st card if not exist
    int32 getRepresentCardId() const
    {
        if (m_iIdTriggerCard >= 0) {
            return m_iIdTriggerCard;
        }

        MY_VERIFY(m_aIdValues.Num() > 0);
        return  m_aIdValues[0].m_iId;

    };

    void initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp eType, int32 iIdTriggerCard, int32 iIdxAttenderTriggerCardSrc, MyMJWeaveTypeCpp eTypeConsumed, int32 iReserved0)
    {
        m_eType = eType;
        m_iIdTriggerCard = iIdTriggerCard;
        m_iIdxAttenderTriggerCardSrc = iIdxAttenderTriggerCardSrc;
        m_eTypeConsumed = eTypeConsumed;
        m_iReserved0 = iReserved0;
    };

    void buildUnweavedInstanceFromValueIdMapCpp(const FMyValueIdMapCpp &inMap)
    {
        reset();
        m_eType = MyMJWeaveTypeCpp::SpecialUnWeavedCards;
        inMap.collectAllWithValue(m_aIdValues);
    };

    inline
    void eraseValues()
    {
        int32 l = m_aIdValues.Num();
        for (int32 i = 0; i < l; i++) {
            m_aIdValues[i].m_iValue = 0;
        }
    };

    FString genDebugString() const;

protected:

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "type"))
    MyMJWeaveTypeCpp m_eType;

    //values can be 0(invalid) when it is not public action
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Id Value Array"))
    TArray<FMyIdValuePair> m_aIdValues;

    //< 0 means no trigger, and trigger can be in @m_aIdValues or not, most case it is in
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Id Trigger Card"))
    int32 m_iIdTriggerCard;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Idx Attender Trigger Card Src"))
    int32 m_iIdxAttenderTriggerCardSrc;

    //in MJ game, you can find the consumed one by check type equal and mid value equal
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Type Of Previous Weave Consumed"))
    MyMJWeaveTypeCpp m_eTypeConsumed;

    //in some local MJ rule, use it to distinguish weave sub type.
    //in the case of CSMJ, when type is GangAn or GangMing, 0 means it is a gangyao, 1 means a buzhang 
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "reserved0", Bitmask, BitmaskEnum = "EMyMJWeaveReserved0Mask"))
    int32 m_iReserved0;
};
*/


//Id should be unique always
USTRUCT(BlueprintType)
struct FMyMJWeaveCpp
{
    GENERATED_USTRUCT_BODY()

        FMyMJWeaveCpp()
    {
        reset();
    };

    void reset() {
        m_eType = MyMJWeaveTypeCpp::Invalid;
        m_aIds.Reset();

        m_iIdTriggerCard = -1;
        m_iIdxAttenderTriggerCardSrc = -1;
        m_eTypeConsumed = MyMJWeaveTypeCpp::Invalid;
        m_iReserved0 = 0;
    };

    inline
    void addCard(int32 id)
    {
        int32 idx = m_aIds.AddUnique(id);
        MY_VERIFY(id >= 0);
    };

    inline
    void appendIds(TArray<int32> aIds)
    {
        int32 l = aIds.Num();
        for (int32 i = 0; i < l; i++) {
            addCard(aIds[i]);
        }
    };

    inline
        void appendIdVakuePairs(const TArray<FMyIdValuePair> &aIdValuePairs)
    {
        int32 l = aIdValuePairs.Num();
        for (int32 i = 0; i < l; i++) {
            addCard(aIdValuePairs[i].m_iId);
        }
    };

    inline
        void appendIdVakuePairsWithLimit(const TArray<FMyIdValuePair> &aIdValuePairs, int32 maxLimit)
    {
        MY_VERIFY(maxLimit > 0);
        int32 count = aIdValuePairs.Num();
        if (count > maxLimit) {
            count = maxLimit;
        }

        for (int32 i = 0; i < count; i++) {
            addCard(aIdValuePairs[i].m_iId);
        }

    };

    bool removeById(int32 id)
    {
        int removedNum = m_aIds.Remove(id);
        return (removedNum > 0);
    };

    inline MyMJWeaveTypeCpp& getTypeRef()
    {
        return m_eType;
    };

    inline MyMJWeaveTypeCpp getType() const
    {
        return m_eType;
    };

    inline const TArray<int32>& getIdsRefConst() const
    {
        return m_aIds;
    };


    inline int32& getIdTriggerCardRef()
    {
        return m_iIdTriggerCard;
    };

    inline int32& getIdxAttenderTriggerCardSrcRef()
    {
        return m_iIdxAttenderTriggerCardSrc;
    };

    inline MyMJWeaveTypeCpp getTypeConsumed() const
    {
        return m_eTypeConsumed;
    };

    inline int32 getReserved0() const
    {
        return m_iReserved0;
    };

    inline int32& getReserved0Ref()
    {
        return m_iReserved0;
    };

    inline int32 getCount() const
    {
        return m_aIds.Num();
    };

    int32 getMidValue(const FMyMJCardValuePackCpp &inValuePack) const
    {
        int32 l = m_aIds.Num();
        MY_VERIFY(l > 0);

        int32 v;
        if (getType() == MyMJWeaveTypeCpp::ShunZiAn || getType() == MyMJWeaveTypeCpp::ShunZiMing) {
            int vall = 0;
            for (int32 i = 0; i < l; i++) {
                v = inValuePack.getByIdx(m_aIds[i], true);
                vall += v;
            }

            return vall / l;
        }
        else {
            v = inValuePack.getByIdx(m_aIds[0], true);
            return v;
        }
    };

    inline
    void getIdValues(const FMyMJCardValuePackCpp &inValuePack, TArray<FMyIdValuePair>& outaIdValues, bool bVerifyValueValid = false) const
    {
        inValuePack.helperIds2IdValuePairs(m_aIds, outaIdValues, bVerifyValueValid);
    };


    //return trigger card , or 1st card if not exist
    int32 getRepresentCardId() const
    {
        if (m_iIdTriggerCard >= 0) {
            return m_iIdTriggerCard;
        }

        MY_VERIFY(m_aIds.Num() > 0);
        return m_aIds[0];

    };

    void initWitIdValuesAlreadyInited(MyMJWeaveTypeCpp eType, int32 iIdTriggerCard, int32 iIdxAttenderTriggerCardSrc, MyMJWeaveTypeCpp eTypeConsumed, int32 iReserved0)
    {
        m_eType = eType;
        m_iIdTriggerCard = iIdTriggerCard;
        m_iIdxAttenderTriggerCardSrc = iIdxAttenderTriggerCardSrc;
        m_eTypeConsumed = eTypeConsumed;
        m_iReserved0 = iReserved0;
    };

    void buildUnweavedInstanceFromValueIdMapCpp(const FMyValueIdMapCpp &inMap)
    {
        reset();
        m_eType = MyMJWeaveTypeCpp::SpecialUnWeavedCards;
        inMap.collectAll(m_aIds);
    };

    FString genDebugString() const;

protected:

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "type"))
        MyMJWeaveTypeCpp m_eType;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Id Array"))
        TArray<int32> m_aIds;

    //< 0 means no trigger, and trigger can be in @m_aIds or not, most take care of it
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Id Trigger Card"))
        int32 m_iIdTriggerCard;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Idx Attender Trigger Card Src"))
        int32 m_iIdxAttenderTriggerCardSrc;

    //in MJ game, you can find the consumed one by check type equal and mid value equal
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Type Of Previous Weave Consumed"))
        MyMJWeaveTypeCpp m_eTypeConsumed;

    //in some local MJ rule, use it to distinguish weave sub type.
    //in the case of CSMJ, when type is GangAn or GangMing, 0 means it is a gangyao, 1 means a buzhang 
    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "reserved0", Bitmask, BitmaskEnum = "EMyMJWeaveReserved0Mask"))
        int32 m_iReserved0;
};


/* multiple hu score type can exist at same type.*/
UENUM(BlueprintType)
enum class MyMJHuScoreTypeCpp : uint8
{
    Invalid = 0                    UMETA(DisplayName = "Invalid"),

    //88 fan
    DaSiXi = 1                     UMETA(DisplayName = "DaSiXi"),
    DaSanYuan = 2                  UMETA(DisplayName = "DaSanYuan"),
    LvYiSe = 3                     UMETA(DisplayName = "LvYiSe"),
    JiuLianBaoDeng = 4             UMETA(DisplayName = "JiuLianBaoDeng"),
    SiGang = 5                     UMETA(DisplayName = "SiGang"),
    LianQiDui = 6                  UMETA(DisplayName = "LianQiDui"),
    ShiSanYao = 7                  UMETA(DisplayName = "ShiSanYao"),

    //64 fan
    QingYaoJiu = 10                UMETA(DisplayName = "QingYaoJiu"),
    XiaoSiXi = 11                  UMETA(DisplayName = "XiaoSiXi"),
    XiaoSanYuan = 12               UMETA(DisplayName = "XiaoSanYuan"),
    ZiYiSe = 13                    UMETA(DisplayName = "ZiYiSe"),
    SiAnKe = 14                    UMETA(DisplayName = "SiAnKe"),
    YiSeShuangLongHui = 15         UMETA(DisplayName = "YiSeShuangLongHui"),

    //48 fan
    YiSeSiTongShun = 20            UMETA(DisplayName = "YiSeSiTongShun"),
    YiSeSiJieGao = 21              UMETA(DisplayName = "YiSeSiJieGao"),

    //32 fan
    YiSeSiBuGao = 30               UMETA(DisplayName = "YiSeSiBuGao"),
    SanGang = 31                   UMETA(DisplayName = "SanGang"),
    HunYaoJiu = 32                 UMETA(DisplayName = "HunYaoJiu"),

    //24 fan
    QiDui = 40                     UMETA(DisplayName = "QiDui"),
    QiXingBuKao = 41               UMETA(DisplayName = "QiXingBuKao"),
    QuanShuangKe = 42              UMETA(DisplayName = "QuanShuangKe"),
    QingYiSe = 43                  UMETA(DisplayName = "QingYiSe"),
    YiSeSanTongShun = 44           UMETA(DisplayName = "YiSeSanTongShun"),
    YiSeSanJieGao = 45             UMETA(DisplayName = "YiSeSanJieGao"),
    QuanDa = 46                    UMETA(DisplayName = "QuanDa"),
    QuanZhong = 47                 UMETA(DisplayName = "QuanZhong"),
    QuanXiao = 48                  UMETA(DisplayName = "QuanXiao"),

    //16 fan
    QingLong = 50                  UMETA(DisplayName = "QingLong"),
    SanSeShuangLongHui = 51        UMETA(DisplayName = "SanSeShuangLongHui"),
    YiSeSanBuGao = 52              UMETA(DisplayName = "YiSeSanBuGao"),
    QuanDaiWu = 53                 UMETA(DisplayName = "QuanDaiWu"),
    SanTongKe = 54                 UMETA(DisplayName = "SanTongKe"),
    SanAnKe = 55                   UMETA(DisplayName = "SanAnKe"),

    //12 fan
    QuanBuKao = 60                 UMETA(DisplayName = "QuanBuKao"),
    ZuHeLong = 61                  UMETA(DisplayName = "ZuHeLong"),
    DaYuWu = 62                    UMETA(DisplayName = "DaYuWu"),
    XiaoYuWu = 63                  UMETA(DisplayName = "XiaoYuWu"),
    SanFengKe = 64                 UMETA(DisplayName = "SanFengKe"),

    //8 fan
    HuaLong = 70                   UMETA(DisplayName = "HuaLong"),
    TuiBuDao = 71                  UMETA(DisplayName = "TuiBuDao"),
    SanSeSanTongShun = 72          UMETA(DisplayName = "SanSeSanTongShun"),
    SanSeSanJieGao = 73            UMETA(DisplayName = "SanSeSanJieGao"),
    WuFanHu = 74                   UMETA(DisplayName = "WuFanHu"),
    MiaoShouHuiChun = 75           UMETA(DisplayName = "MiaoShouHuiChun"),
    HaiDiLaoYue = 76               UMETA(DisplayName = "HaiDiLaoYue"),
    GangShangKaiHua = 77           UMETA(DisplayName = "GangShangKaiHua"),
    QiangGangHu= 78                UMETA(DisplayName = "QiangGangHu"),

    //6 fan
    PengPengHu = 80                UMETA(DisplayName = "PengPengHu"),
    HunYiSe = 81                   UMETA(DisplayName = "HunYiSe"),
    SanSeSanBuGao = 82             UMETA(DisplayName = "SanSeSanBuGao"),
    WuMenQi = 83                   UMETA(DisplayName = "WuMenQi"),
    QuanQiuRen = 84                UMETA(DisplayName = "QuanQiuRen"),
    ShuangAnGang = 85              UMETA(DisplayName = "ShuangAnGang"),
    ShuangJianKe = 86              UMETA(DisplayName = "ShuangJianKe"),

    //4 fan
    QuanDaiYao = 90                UMETA(DisplayName = "QuanDaiYao"),
    BuQiuRen = 91                  UMETA(DisplayName = "BuQiuRen"),
    ShuangMingGang = 92            UMETA(DisplayName = "ShuangMingGang"),
    HuJueZhang = 93                UMETA(DisplayName = "HuJueZhang"),

    //2 fan
    JianKe = 100                   UMETA(DisplayName = "JianKe"),
    QuanFengKe = 101               UMETA(DisplayName = "QuanFengKe"),
    MenFengKe = 102                UMETA(DisplayName = "MenFengKe"),
    MenQianQing = 103              UMETA(DisplayName = "MenQianQing"),
    PingHu = 104                   UMETA(DisplayName = "PingHu"),
    SiGuiYi = 105                  UMETA(DisplayName = "SiGuiYi"),
    ShuangTongKe = 106             UMETA(DisplayName = "ShuangTongKe"),
    ShuangAnKe = 107               UMETA(DisplayName = "ShuangAnKe"),
    AnGang = 108                   UMETA(DisplayName = "AnGang"),
    DuanYao = 109                  UMETA(DisplayName = "DuanYao"),

    //1 fan
    YiBanGao = 120                 UMETA(DisplayName = "YiBanGao"),
    XiXiangFeng = 121              UMETA(DisplayName = "XiXiangFeng"),
    LianLiu = 122                  UMETA(DisplayName = "LianLiu"),
    LaoShaoFu = 123                UMETA(DisplayName = "LaoShaoFu"),
    YaoJiuKe = 124                 UMETA(DisplayName = "YaoJiuKe"),
    MingGang = 125                 UMETA(DisplayName = "MingGang"),
    QueYiMen = 126                 UMETA(DisplayName = "QueYiMen"),
    WuZi = 127                     UMETA(DisplayName = "WuZi"),
    BianZhang = 128                UMETA(DisplayName = "BianZhang"),
    KanZhang = 129                 UMETA(DisplayName = "KanZhang"),
    DanDiaoJiang = 130             UMETA(DisplayName = "DanDiaoJiang"),
    ZiMo = 131                     UMETA(DisplayName = "ZiMo"),
    HuaPai = 132                   UMETA(DisplayName = "HuaPai"),

    //local types, first, define some may reuse by many local types
    LocalTianHu = 150              UMETA(DisplayName = "LocalTianHu"),
    LocalDiHu = 151                UMETA(DisplayName = "LocalDiHu"),
    LocalHaiDiPao = 152            UMETA(DisplayName = "LocalHaiDiPao"),
    LocalGangShangPao = 153        UMETA(DisplayName = "LocalGangShangPao"),

    //local type
    LocalCSBornNoJiang258 = 160      UMETA(DisplayName = "LocalCSBornNoJiang258"),
    LocalCSBornLackNumCardType = 161 UMETA(DisplayName = "LocalCSBornLackNumCardType"),
    LocalCSBorn2KeZi = 162           UMETA(DisplayName = "LocalCSBorn2KeZi"),
    LocalCSBornAnGangInHand = 163    UMETA(DisplayName = "LocalCSBornAnGangInHand"),

    LocalCSAllJiang258 = 165           UMETA(DisplayName = "LocalCSAllJiang258"),
    LocalCSHaoHuaQiDui = 166           UMETA(DisplayName = "LocalCSHaoHuaQiDui"),
    LocalCSShuangHaoHuaQiDui = 167     UMETA(DisplayName = "LocalCSShuangHaoHuaQiDui"),


    //MTE_LAST = 0x80000000 UMETA(Hidden)
};

USTRUCT()
struct FMyMJHuCommonCfg
{
    GENERATED_USTRUCT_BODY()

    FMyMJHuCommonCfg() {

    };

    void reset()
    {

    };
};

//Here we use int, to avoid mem allocation
USTRUCT()
struct FMyMJHuScoreAttrCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJHuScoreAttrCpp()
    {
        m_eType = MyMJHuScoreTypeCpp::Invalid;
        m_iScorePerAttender = 0;

    };

    /* type */
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Type"))
    MyMJHuScoreTypeCpp m_eType;

    /* the score every attender must pay when ZiMo */
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Score Per Attender"))
    int32 m_iScorePerAttender;

};

USTRUCT()
struct FMyMJHuScoreResultItemCpp : public FMyMJHuScoreAttrCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJHuScoreResultItemCpp() : Super()
    {
        m_iCount = 0;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Count"))
    int32 m_iCount;
};

/*
USTRUCT(BlueprintType)
struct FMyMJHuScoreResultCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJHuScoreResultCpp()
    {
        reset();
    };

    void reset()
    {
        m_aItems.Reset();
        m_iTotalScorePerAttender = 0;
    };

    int32 append(TArray<FMyMJHuScoreResultItemCpp>& newItems)
    {
        int32 l = newItems.Num();
        for (int32 i = 0; i < l; i++) {
            m_iTotalScorePerAttender += newItems[i].m_iScorePerAttender * newItems[i].m_iCount;
        }

        m_aItems.Append(newItems);

        return m_iTotalScorePerAttender;
    };

    int32 append(FMyMJHuScoreResultCpp* pOther)
    {
        return append(pOther->m_aItems);
    };

    inline
    int32 getTotalScore()
    {
        return m_iTotalScorePerAttender;
    };

protected:

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Items"))
    TArray<FMyMJHuScoreResultItemCpp> m_aItems;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Total Score Per Attender"))
    int32 m_iTotalScorePerAttender;
};
*/
/*
USTRUCT()
struct FMyMJHuScoreResultBaseCpp final
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJHuScoreResultBaseCpp() {
        reset();
    };
    virtual ~FMyMJHuScoreResultBaseCpp() {};

    TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreResultItemCpp *> m_mScoreTypeItemMap;
    TArray<FMyMJHuScoreResultItemCpp> m_aItems;
};
*/

USTRUCT()
struct FMyMJHuScoreResultBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJHuScoreResultBaseCpp() {
        reset();
    };
    virtual ~FMyMJHuScoreResultBaseCpp() {};

    inline
        bool isScoreLessThan(const struct FMyMJHuScoreResultBaseCpp &other) const
    {
        return getScorePerAttenderTotal() < other.getScorePerAttenderTotal();
    };

    inline
        int32 getScorePerAttenderTotal() const
    {
        return m_iScorePerAttenderTotal;
    };


    //inline const TArray<FMyMJHuScoreResultItemCpp>& getItems() const
    //{
    //    return m_aItems;
    //}

    void reset()
    {
        //m_eHuCardType = MyMJHuCardTypeCpp::Invalid;
        //m_aItems.Reset();
        m_mScoreTypeItemMap.Reset();
        m_iScorePerAttenderTotal = 0;

        //m_aWeavesShowedOut.Reset();
        //m_aWeavesInHand.Reset();


    };

    inline bool addType(const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> &scoreSettings,
                        const TSet<MyMJHuScoreTypeCpp> *pBaned,
                        MyMJHuScoreTypeCpp type)
    {
        return addTypeWithCount(scoreSettings, pBaned, type, 1);
    };

    bool addTypeWithCount(const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> &scoreSettings,
                          const TSet<MyMJHuScoreTypeCpp> *pBaned,
                          MyMJHuScoreTypeCpp type, int32 c)
    {
        if (pBaned && pBaned->Find(type) != NULL) {
            return false;
        }

        const FMyMJHuScoreAttrCpp* pSetting = scoreSettings.Find(type);
        if (pSetting == NULL) {
            return false;
        }

        MY_VERIFY(pSetting->m_eType == type);

        addItemInternal(type, pSetting->m_iScorePerAttender, c);

        return true;

    };

    bool removeType(MyMJHuScoreTypeCpp type)
    {
        //FMyMJHuScoreResultItemCpp **ppItem = m_mScoreTypeItemMap.Find(type);
        //if (ppItem == NULL) {
            //return false;
        //}

        FMyMJHuScoreResultItemCpp *pItem = m_mScoreTypeItemMap.Find(type);
        if (pItem == NULL) {
            return false;
        }

        m_iScorePerAttenderTotal -= pItem->m_iScorePerAttender * pItem->m_iCount;

        MY_VERIFY(m_mScoreTypeItemMap.Remove(type) == 1);

        //int32 l = m_aItems.Num();
        //int32 idxFound = -1;
        //for (int32 i = 0; i < l; i++) {
        //    if (m_aItems[i].m_eType == type) {
        //        idxFound = i;
        //        break;
        //    }
       // }

        //MY_VERIFY(idxFound >= 0);
       // m_aItems.RemoveAt(idxFound);

        return true;
    };

    inline
    FMyMJHuScoreResultItemCpp* findByType(MyMJHuScoreTypeCpp type)
    {
        //FMyMJHuScoreResultItemCpp **ppItem = m_mScoreTypeItemMap.Find(type);
        //if (ppItem) {
        //    return *ppItem;
        //}
        //else {
        //    return NULL;
        //}

        return m_mScoreTypeItemMap.Find(type);
    };

    int32 append(const TArray<FMyMJHuScoreResultItemCpp>& newItems)
    {
        int32 l = newItems.Num();
        for (int32 i = 0; i < l; i++) {
            const FMyMJHuScoreResultItemCpp *pItem = &newItems[i];
            addItemInternal(pItem->m_eType, pItem->m_iScorePerAttender, pItem->m_iCount);
        }

        return m_iScorePerAttenderTotal;
    };

    /*
    FMyMJHuScoreResultBaseCpp& operator = (const FMyMJHuScoreResultBaseCpp& rhs)
    {
        if (this == &rhs) {
            return *this;
        }

        copyDeep(&rhs);
        return *this;
    };

    void copyDeep(const FMyMJHuScoreResultBaseCpp *pOther)
    {
        m_aWeavesShowedOut = pOther->m_aWeavesShowedOut;
        m_aWeavesInHand = pOther->m_aWeavesInHand;

        m_iScorePerAttenderTotal = pOther->m_iScorePerAttenderTotal;
        m_aItems = pOther->m_aItems;

        buildMap();
    };

    void buildMap()
    {
        int32 l = m_aItems.Num();
        m_mScoreTypeItemMap.Reset();
        for (int32 i = 0; i < l; i++) {

            FMyMJHuScoreResultItemCpp *pItem = &m_aItems[i];

            MY_VERIFY(m_mScoreTypeItemMap.Find(pItem->m_eType) == NULL);
            FMyMJHuScoreResultItemCpp *&pNewAddedRef = m_mScoreTypeItemMap.Add(pItem->m_eType);
            pNewAddedRef = pItem;
        }
    };
    */

    //MyMJHuCardTypeCpp m_eHuCardType;

    FString genDebugString() const;


protected:

    inline void addItemInternal(MyMJHuScoreTypeCpp type, int32 iScorePerAttender, int32 count)
    {
        /*
        FMyMJHuScoreResultItemCpp **ppItem = m_mScoreTypeItemMap.Find(type);
        FMyMJHuScoreResultItemCpp *pItem;
        if (ppItem == NULL) {
            pItem = m_mScoreTypeItemMap.Add(type);
        }
        else {
            pItem = *ppItem;
        }
        */
        MY_VERIFY(type != MyMJHuScoreTypeCpp::Invalid);

        FMyMJHuScoreResultItemCpp *pItem = &m_mScoreTypeItemMap.FindOrAdd(type);

        if (pItem->m_eType != MyMJHuScoreTypeCpp::Invalid) {
            if (pItem->m_eType != type) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("addItemInternal type not equal: %d,  %d"), (uint8)pItem->m_eType, (uint8)type);
                MY_VERIFY(false);
            }
        }
        else {
            pItem->m_eType = type;
        }

        if (pItem->m_iScorePerAttender > 0) {
            MY_VERIFY(pItem->m_iScorePerAttender == iScorePerAttender);
        }
        else {
            pItem->m_iScorePerAttender = iScorePerAttender;
        }
        pItem->m_iCount += count;

        m_iScorePerAttenderTotal += pItem->m_iScorePerAttender * count;
    };

    UPROPERTY()
    TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreResultItemCpp> m_mScoreTypeItemMap;
    //TArray<FMyMJHuScoreResultItemCpp> m_aItems;

    UPROPERTY()
    int32 m_iScorePerAttenderTotal;
};

USTRUCT()
struct FMyMJHuScoreResultFinalCpp : public FMyMJHuScoreResultBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJHuScoreResultFinalCpp() : Super() {
        reset();
    };
    virtual ~FMyMJHuScoreResultFinalCpp() {};

    void reset()
    {
        Super::reset();
        m_cIdValueTriggerCard.reset(true);

    };

    FMyIdValuePair m_cIdValueTriggerCard;
};

USTRUCT()
struct FMyMJHuScoreResultFinalGroupCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJHuScoreResultFinalGroupCpp() {
        reset();
    };
    virtual ~FMyMJHuScoreResultFinalGroupCpp() {};

    void reset()
    {
        m_iIdxAttenderWin = -1;
        m_iIdxAttenderLoseOnlyOne = -1;
        m_aWeavesShowedOut.Reset();
        m_aWeavesInHand.Reset();
        m_aScoreResults.Reset();
    };

    FString genDebugString() const
    {
        int32 l = m_aScoreResults.Num();
        FString ret = FString::Printf(TEXT(" idxWin: %d, idxLoseOnlyOne %d, scoreResultNum %d. "), m_iIdxAttenderWin, m_iIdxAttenderLoseOnlyOne, l);

        for (int32 i = 0; i < l; i++) {
            ret += m_aScoreResults[i].genDebugString();
            //m_aScoreResults[i].
        }

        return ret;
    };

    UPROPERTY()
    int32 m_iIdxAttenderWin;

    UPROPERTY()
    int32 m_iIdxAttenderLoseOnlyOne; // < 0 means every one, not a pao

    UPROPERTY()
    TArray<FMyMJWeaveCpp> m_aWeavesShowedOut;

    //By default, we just form a 'unweaved' for hand cards, otherwise is handweaves
    //trigger card is excluded if it exist
    UPROPERTY()
    TArray<FMyMJWeaveCpp> m_aWeavesInHand;

    UPROPERTY()
    TArray<FMyMJHuScoreResultFinalCpp> m_aScoreResults;
};

USTRUCT()
struct FMyMJHuScoreResultTingCpp : public FMyMJHuScoreResultBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJHuScoreResultTingCpp() : Super() {
        reset();
    };
    virtual ~FMyMJHuScoreResultTingCpp() {};

    void reset()
    {
        Super::reset();
        m_bBanZiMo = false;
        m_bBanPao = false;

        //m_iIdTriggerCard = -1;
        m_iValueTriggerCard = 0;
        m_iTriggerCardLeftOnDesktop = -1;
    };

    inline bool isRealTing() const
    {
        return (m_iTriggerCardLeftOnDesktop != 0) && (!(m_bBanZiMo && m_bBanPao));
    };

    inline bool isRealTingWhenZiMo() const
    {
        return (m_iTriggerCardLeftOnDesktop != 0) && (!m_bBanZiMo);
    };

    inline bool isRealTingWhenPao() const
    {
        return (m_iTriggerCardLeftOnDesktop != 0) && (!m_bBanPao);
    };


    UPROPERTY()
    bool m_bBanZiMo;

    UPROPERTY()
    bool m_bBanPao;

    //int32 m_iIdTriggerCard;

    //what value can trigger the hu
    UPROPERTY()
    int32 m_iValueTriggerCard;

    //< 0 means not set
    UPROPERTY()
    int32 m_iTriggerCardLeftOnDesktop;
};

USTRUCT()
struct FMyMJHuScoreResultTingGroupCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJHuScoreResultTingGroupCpp() {
        reset();
    };
    virtual ~FMyMJHuScoreResultTingGroupCpp() {};

    void reset()
    {
        m_mValueTingMap.Reset();
        m_aValuesHandCardWhenChecking.Reset();
    };

    inline int32 getCount() const
    {
        return m_mValueTingMap.Num();
    };

    FString genDebugString() const
    {
        int32 l = m_mValueTingMap.Num();
        FString ret = FString::Printf(TEXT(" ting count %d, values count %d. "), getCount(), m_aValuesHandCardWhenChecking.Num());

        for (auto It = m_mValueTingMap.CreateConstIterator(); It; ++It)
        {
            int32 cardValue = It.Key();
            const FMyMJHuScoreResultTingCpp* pMapElem = &It.Value();

            ret += FString::Printf(TEXT(" card Value %d, score: %s."), cardValue, *pMapElem->genDebugString());

        }

        return ret;
    };


    bool isRealTing(bool *pbIsRealTingWhenZiMo, bool *pbIsRealTingWhenPao)
    {
        bool bIsRealTing = false;
        bool bIsRealTingWhenZiMo = false;
        bool bIsRealTingWhenPao = false;
        for (auto It = m_mValueTingMap.CreateConstIterator(); It; ++It)
        {
            const FMyMJHuScoreResultTingCpp* pMapElem = &It.Value();

            MY_VERIFY(!(pMapElem->m_bBanPao && pMapElem->m_bBanZiMo));

            if (pMapElem->isRealTing()) {
                bIsRealTing = true;
            }

            if (pMapElem->isRealTingWhenZiMo()) {
                bIsRealTingWhenZiMo = true;
            }

            if (pMapElem->isRealTingWhenPao()) {
                bIsRealTingWhenPao = true;
            }
        }

        MY_VERIFY(bIsRealTing == (bIsRealTingWhenZiMo || bIsRealTingWhenPao));

        if (pbIsRealTingWhenZiMo) {
            *pbIsRealTingWhenZiMo = bIsRealTingWhenZiMo;
        }

        if (pbIsRealTingWhenPao) {
            *pbIsRealTingWhenPao = bIsRealTingWhenPao;
        }

        return bIsRealTing;

    };

    UPROPERTY()
    TMap<int32, FMyMJHuScoreResultTingCpp> m_mValueTingMap;

    UPROPERTY()
    TArray<int32> m_aValuesHandCardWhenChecking; //helper that identify in what condition this Ting is tested, since when ever attender weave, hand card changes, so this can identify condition is same if it is equal
};


USTRUCT()
struct FMyMJHuCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJHuCfgCpp()
    {

    };

    void reset()
    {
        m_cHuCommonCfg.reset();
        m_mHuScoreAttrsCfg.Reset();
    };

    inline
    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp>& getHuScoreAttrsCfgRef() const
    {
        return m_mHuScoreAttrsCfg;
    };

    inline
    TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp>& getHuScoreAttrsCfgRef()
    {
        return m_mHuScoreAttrsCfg;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Hu Common Cfg"))
    FMyMJHuCommonCfg m_cHuCommonCfg; //the common cfg

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Hu Card Types Cfg"))
    //TArray<MyMJHuCardTypeCpp> m_aHuCardTypesCfg; //The allowed hu type


protected:

    //Warn: onece set and used, you can't add/delete/mod(it's eType) anyMore
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Hu Score Attrs Cfg"))
    //TArray<FMyMJHuScoreAttrCpp> m_aHuScoreAttrsCfg;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Hu Score Attrs Cfg"))
    TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> m_mHuScoreAttrsCfg; //always pointer to
};


USTRUCT(BlueprintType)
struct FMyMJWeaveArrayCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJWeaveArrayCpp()
    {};

    //UFUNCTION(BlueprintCallable, Category = "My MJ Utils")
    void reset() {
        m_aWeaves.Empty();
    };

    /* the Weaves */
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Weave Array"))
    TArray<FMyMJWeaveCpp> m_aWeaves;
};

struct FMyMJWeaveTreeNodeCpp
{
    FMyMJWeaveTreeNodeCpp()
    {
        reset();
    };

    virtual ~FMyMJWeaveTreeNodeCpp() {};

    void reset() {
        m_sWeave.reset();
        m_aChilds.Empty();
    };

public:
    void appendChild(TSharedPtr<FMyMJWeaveTreeNodeCpp> &pChild);
    void appendOtherChilds(FMyMJWeaveTreeNodeCpp *other);
    void convert2WeaveArray(TArray<FMyMJWeaveArrayCpp> &outArray);
    FMyMJWeaveCpp *getWeave();

protected:
    void convert2WeaveArrayIn(TArray<FMyMJWeaveCpp> parentWeaves, TArray<FMyMJWeaveArrayCpp> &outArray);

    FMyMJWeaveCpp m_sWeave;
    TArray<TSharedPtr<FMyMJWeaveTreeNodeCpp>> m_aChilds;
};

USTRUCT()
struct FMyValueCountMapCpp
{
    GENERATED_USTRUCT_BODY()

    FMyValueCountMapCpp() {
        reset();
    };
    virtual ~FMyValueCountMapCpp() {};

    void reset() {
        m_mMap.Empty();
        m_iCount = 0;
    };

public:

    TMap<int32, int32> m_mMap;
    int32 m_iCount;
};

USTRUCT()
struct FMyStatisCountsPerCardValueTypeCpp
{
    GENERATED_USTRUCT_BODY()

    FMyStatisCountsPerCardValueTypeCpp() {
        reset();
    };
    virtual ~FMyStatisCountsPerCardValueTypeCpp() {};

public:

    void reset()
    {
        m_mCountPerCardType.Reset();
        m_iCountTotal = 0;
    };

    //add the count with a value used to judge card type
    void addCountByValue(int32 value, int32 count);

    const int32 getCountByCardValueType(MyMJCardValueTypeCpp type);

    const int32 getTotalCount();

protected:
    void collectTouchedTypesByValue(int32 value, TArray<MyMJCardValueTypeCpp> &outTouchedTypes);

    //For effecience, let's use map instead of array, to avoid allocate unnessary item
    //TArray<int32> m_aCountPerCardType;
    TMap<MyMJCardValueTypeCpp, int32> m_mCountPerCardType;
    int32 m_iCountTotal;
};

//note three case: 1 no card, 2 single card taken or give out, 3 gang show out
UENUM(BlueprintType)
enum class MyMJHuTriggerCardSrcTypeCpp : uint8
{
    None = 0                    UMETA(DisplayName = "None"),

    //Standarded Hu
    CommonInGame = 1                   UMETA(DisplayName = "CommonInGame"), //BuHua is also common
    GangWeaveShowedOut = 2                UMETA(DisplayName = "GangWeaveShowedOut"),
    GangCardTaken = 3                UMETA(DisplayName = "GangCardTaken")

    //MTE_LAST = 0x80000000 UMETA(Hidden)
};

USTRUCT()
struct FMyTriggerDataCpp : public FMyIdValuePair
{
    GENERATED_USTRUCT_BODY()

    FMyTriggerDataCpp() : Super()
    {
        reset();
    };

    void reset() {
        Super::reset(true);
        m_iValueShowedOutCountAfter = -1;
    };

    FMyTriggerDataCpp& operator = (const FMyIdValuePair& rhs)
    {
        if (this == &rhs) {
            return *this;
        }

        *(static_cast<FMyIdValuePair *>(this)) = rhs;

        return *this;
    };

    //including trigger Card, so it is 4 if last card, < 0 means unknown
    UPROPERTY()
    int32 m_iValueShowedOutCountAfter;
};

/*
* describe the hu action
*/
USTRUCT()
struct FMyMJHuActionAttrBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJHuActionAttrBaseCpp() {
        reset();
    };

    void reset() {
        //m_eType = MyMJHuCardTypeCpp::Invalid;
        m_iIdxAttenderWin = -1;
        m_iIdxAttenderLoseOnlyOne = -1;
        m_iQUanFeng = 0;
        m_iMenFeng = -1;

        m_eTrigerCardActionType = MyMJHuTriggerCardSrcTypeCpp::None;

        m_iCardNumCanBeTakenNormally = -1;
        //m_iGameTurn = 0;
        m_iAttenderTurn = 0;

        m_iHuaCount = 0;
        m_iTingCount = 0;
    };

    //some time the action is completed banned from hu, here we can store that info in the attr
    inline bool getMayHu() const
    {
        return (m_iIdxAttenderWin >= 0);
    };

    inline bool geIsPao() const
    {
        return (m_iIdxAttenderLoseOnlyOne >= 0);
    };

    //hu other attender's, if false, it means zimo or tianhu

    //MyMJHuCardTypeCpp m_eType;

    int32 m_iIdxAttenderWin;
    int32 m_iIdxAttenderLoseOnlyOne; // < 0 means every one, not a pao

    int32 m_iQUanFeng;
    int32 m_iMenFeng; //same as zhuang


    MyMJHuTriggerCardSrcTypeCpp m_eTrigerCardActionType;

    int32 m_iCardNumCanBeTakenNormally;
    //int32 m_iGameTurn;
    int32 m_iAttenderTurn;

    int32 m_iHuaCount;
    int32 m_iTingCount;

};

USTRUCT()
struct FMyMJHuActionAttrCpp : public FMyMJHuActionAttrBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJHuActionAttrCpp() : Super()
    {
        reset();
    };

    void reset()
    {
        Super::reset();
        m_aTriggerDatas.Reset();
    };

    //In some rules, one action contain more than one trigger, and we will check each card whether can hu
    TArray<FMyTriggerDataCpp> m_aTriggerDatas;
};

/* Strictly used per weave type, 1 gang != 1 KeZi, != 2 DuiZi, 1 KeZi != 1 DuiZi 
 * SHunZi all use CHiLeft for simple
*/
USTRUCT()
struct FMyMJCardParseResultSimpleCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardParseResultSimpleCpp() {
        reset();
    };

    virtual void reset() {
        m_cCardCounts.reset();
        m_mWeaveCounts.Empty();
    };

    //Gang count as two duizi, KeZi does not count
    int32 getDuiZiOrGangTotal() {
        int32 ret = 0;
        FMyStatisCountsPerCardValueTypeCpp *temp;
        
        temp = m_mWeaveCounts.Find(MyMJWeaveTypeCpp::DuiZi);
        if (temp) {
            ret += temp->getTotalCount();
        }

        temp = m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangAn);
        if (temp) {
            ret += temp->getTotalCount() * 2;
        }

        temp = m_mWeaveCounts.Find(MyMJWeaveTypeCpp::GangMing);
        if (temp) {
            ret += temp->getTotalCount() * 2;
        }

        return ret;
    }


    FMyStatisCountsPerCardValueTypeCpp m_cCardCounts;
    TMap<MyMJWeaveTypeCpp, FMyStatisCountsPerCardValueTypeCpp> m_mWeaveCounts; //Use map instead of array for performance
    //TArray<FMyStatisCountsPerCardValueTypeCpp> m_aWeaveCounts;

};

USTRUCT()
struct FMyMJCardParseResultCpp : public FMyMJCardParseResultSimpleCpp
{
    GENERATED_USTRUCT_BODY()

    FMyMJCardParseResultCpp() : Super() {
        //reset is called in parent
    };

    virtual void reset() override {
        Super::reset();

        m_cChiLit.reset();
        m_cChiMid.reset();
        m_cChiBig.reset();
        m_mShunZiMidValueCountMap.Empty();
        m_mKeZiOrGangValueCountMap.Empty();
        m_iFirstDuiZiValue = -1;
        m_iWeavesCountHaveNumCard5 = 0;
        m_iWeaveCountHaveYaoCard = 0;

        m_mDuiZiOrGangValueCountMap.Empty();
    };

    int32 getShunZiSameGroupCount(int32 minCount);

    int32 getShunZiStep1or2IncreaseMax();
 
    int32 getShunZiModV10SameGroupCountRestrictDifferentType(int32 minCount, bool bMustEqual);

    int32 getKeZiOrGangModV10SameGroupCountRestrictDifferentTypeAndNumCard(int32 minCount, bool bMustEqual);

    int32 getKeZiOrGangStep1IncreaseMaxRestrictNumCard();

    int32 getCountSanSeSanJieGao();

    int32 getCountSanSeSanBuGao();
 
    int32 getLaoShaoFuCountByType(MyMJCardValueTypeCpp type);

    int32 getCountLianLiu();

    int32 getCountYaoJiuKe();

    bool findLian7Dui();

    //return invalid if not found
    MyMJCardValueTypeCpp findQingLong();
  
    //Hard point is every type must >= 1
    bool findHuaLong();

protected:
    static
    void buildModV10CountMapFromValueCountMap(TMap<int32, int32> &inValueCountMap, bool bRestrictNumCardValue, bool bAlwaysCountAsOne, TMap<int32, int32> &outModVCountMap);

    static
    int32 getCountOfValuesFromValueCountMap(TMap<int32, int32> &inMap, int32 minCount, bool bMustEqual, bool bRestrictNumCardValue);
   

    /**
    * Before calling, caller must sort input as:
    *        m_mShunZiMidValueCountMap.KeySort([](int32 A, int32 B) {
    *          return A < B; // sort keys in asc
    *        });
    * this function ignore the count part, only check the value part
    * assume (value % 10) != 0 always
    * @param step must in range [1, 10)
    * @param pInMin optional count the number that have continue values >= *pInMin, if set *pInMin must > 1
    */
    static
    int32 getContinueIncreaseMaxFromValueCountMap(TMap<int32, int32> &inMap, int32 step, bool bRestrictNumCardValueAndBanCrossMod10, int32 *pInMin, int32 *pOutMinCount);
   

    static
    int32 getCountOfStep1WithDifferentCardTypes(TMap<int32, int32> *pMap);


public:

    FMyStatisCountsPerCardValueTypeCpp m_cChiLit; //123
    FMyStatisCountsPerCardValueTypeCpp m_cChiMid; //456
    FMyStatisCountsPerCardValueTypeCpp m_cChiBig; //789
    TMap<int32, int32> m_mShunZiMidValueCountMap;
    TMap<int32, int32> m_mKeZiOrGangValueCountMap;

    int32 m_iFirstDuiZiValue;
    int32 m_iWeavesCountHaveNumCard5;
    int32 m_iWeaveCountHaveYaoCard;

    TMap<int32, int32> m_mDuiZiOrGangValueCountMap; //Gang Count as two duizi here, only set when parsing hand cards
};


USTRUCT()
struct FMyMJScoreCalcResultCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJScoreCalcResultCpp() {
        m_pScoreSettings = NULL;
        reset();
    };
    virtual ~FMyMJScoreCalcResultCpp() {};

    void init(const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> &inHuScoreSettings) {
        m_pScoreSettings = &inHuScoreSettings;
    };

    inline
    bool isScoreLessThan(const FMyMJScoreCalcResultCpp &other) const {
        return getScore() < other.getScore();
    };

    inline
    int32 getScore() const {
        return m_iScorePerAttenderTotal;
    };


    void reset() {
        m_mAddedTypeCountMap.Empty();
        m_sBaned.Empty();

        m_iScorePerAttenderTotal = 0;

        m_eHuCardType = MyMJHuCardTypeCpp::Invalid;
        m_aWeavesShowedOut.Empty();
        m_aWeavesInHand.Empty();
        m_iIdxTriggerCard = -1;
    };

    inline
    void addType(MyMJHuScoreTypeCpp type) {
        addTypeWithCount(type, 1);
    };

    inline
    void addTypeWithCount(MyMJHuScoreTypeCpp type, int32 c) {
        if (m_sBaned.Find(type) != NULL) {
            return;
        }

        const FMyMJHuScoreAttrCpp* pSetting = m_pScoreSettings->Find(type);
        if (pSetting == NULL) {
            return;
        }

        int32 &count = m_mAddedTypeCountMap.FindOrAdd(type);
        count += c;

        m_iScorePerAttenderTotal += pSetting->m_iScorePerAttender * c;

    };

    inline
    void banType(MyMJHuScoreTypeCpp type) {
        m_sBaned.Emplace(type);
    };

    inline
    bool isTypeAllowed(MyMJHuScoreTypeCpp type) {
        return (m_sBaned.Find(type) == NULL) && (m_pScoreSettings->Find(type) != NULL);
    };

    inline
    MyMJHuScoreTypeCpp getFirstAddedScoreType()
    {
        for (auto It = m_mAddedTypeCountMap.CreateConstIterator(); It; ++It)
        {
            return It.Key();

        }

        return MyMJHuScoreTypeCpp::Invalid;
    };

    const TMap<MyMJHuScoreTypeCpp, FMyMJHuScoreAttrCpp> *m_pScoreSettings;

    TMap<MyMJHuScoreTypeCpp, int32> m_mAddedTypeCountMap;

    TSet<MyMJHuScoreTypeCpp> m_sBaned;

    int32 m_iScorePerAttenderTotal;

    MyMJHuCardTypeCpp m_eHuCardType;
    TArray<FMyMJWeaveCpp> m_aWeavesShowedOut;
    TArray<FMyMJWeaveCpp> m_aWeavesInHand;
    int32 m_iIdxTriggerCard;

};

/**
 * All API that take things as out, such as TArray, will empty/clear it first then try fill.
 */

USTRUCT(BlueprintType)
struct FMyMJValueIdMapCpp : public FMyValueIdMapCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJValueIdMapCpp() : Super()
    {

    };

    /**
    * @pExt != NULL means checking with extra value not in map, checking value is overritten, and it will NOT be present in output, output only contain ids In Map
    */
    bool checkShunZi(MyMJChiTypeCpp type, int32 checkingValueInMap, const FMyIdValuePair *pExt, TArray<FMyIdValuePair> &outIdValuePairsInMap) const;

    bool checkShunZi(MyMJChiTypeCpp type, int32 checkingValueInMap, const FMyIdValuePair *pExt, TArray<int32> &outIds, TArray<int32> &outValues) const;

    /**
    * check remaining cards can form weave, always require a Jiang, pengpengHu, QingYiSe always not require 258 jiang, assuming extra card have been pushed in if exist
    *
    * @param reqJiang258                    whether jiang is restrict to 258 num card
    * @param allowJiangNot258ForLastDuiZi   only used when reqJiang258 is true, whether jiang can be not 258 if only two card left
    * @param weavesShowedMayPengPengHu      only used when reqJiang258 is true, whether the weaves showed out may allwo pengpengHu
    * @param weaveShowedOutUnifiedType      only used when reqJiang258 is true, whether the weaves showed out have same number card type, if not unified, MyMJCardValueTypeCpp::invalid should be specified
    * @param weaveShowedOutCount            only used when reqJiang258 is true, the count of weaves showed out
    * @param bFindAllCombines               whether list all possible combines in outWeaveCombines, if false only one is listed and code runs faster
    * @param outpParent                     the possible weave combines, there may be multiple ways to combine cards, all will be attached as childs
    * @return true if OK

    bool checkAllCanBeWeavedForHu(bool reqJiang258, bool allowJiangNot258ForLastDuiZi, bool weavesShowedMayPengPengHu, MyMJCardValueTypeCpp weaveShowedOutUnifiedType, int32 weaveShowedOutCount, bool findAllCombines, FMyMJWeaveTreeNodeCpp* outpParent) const;
    */

    /**
    * check remaining cards can form weave, always require a Jiang, pengpengHu, QingYiSe always not require 258 jiang, assuming extra card have been pushed in if exist
    *
    * @param reqJiang258                    whether jiang is restrict to 258 num card
    * @param allowShunZi                    whether allow shunZi
    * @param findAllCombines                whether list all possible combines in outWeaveCombines, if false only one is listed and code runs faster
    * @param outpParent                     the possible weave combines, there may be multiple ways to combine cards, all will be attached as childs
    * @return true if OK
    */
    bool checkAllCanBeWeavedForHu(bool reqJiang258, bool allowShunZi, bool findAllCombines, FMyMJWeaveTreeNodeCpp* outpParent) const;


    MyMJCardValueTypeCpp getUnifiedNumCardType() const;

    /*
    * assume all card pushed in
    * @param bComplexParse if not true, only FMyMJCardParseResultSimpleCpp fields are filled
    */
    void parseCards(bool bComplexParse, FMyMJCardParseResultCpp &outResult) const;

    //Following API didn't take extra parameter, assuming that it has been pushed in if exist
    //It doesn't care whether MenQing, consider it as if allow not menqing
    bool checkSpecialZuHeLong(TArray<FMyMJWeaveArrayCpp> &outWeaveCombines) const;
	
    // @param outQiXingBuKao not used now
    bool checkSpecial13BuKaoIn(bool &outQiXingBuKao) const;


    int32 getCountOfGroupSameCardValue(int32 minCount, bool bAlwaysCountAsOne) const;

    void fillInWeavesAssert7Dui(TArray<FMyMJWeaveCpp> &outWeaves) const;

    void fillInWeavesSpecialUnWeavedCards(TArray<FMyMJWeaveCpp> &outWeaves) const;

protected:
    /**
    * checking remainging card can form a weave tree
    * @param bFindAllWeaves if true, every possible combine will be tried, consumming more time
    *
    * @return true if OK
    */
    bool checkAllCanBeWeavedForHuIn(bool allowJiang, bool reqJiang258, bool allowShunZi, bool bFindAllWeaves, FMyMJWeaveTreeNodeCpp* outpParent) const;
};


USTRUCT()
struct FMyWeavesShowedOutStatisCpp
{
    GENERATED_USTRUCT_BODY()

    FMyWeavesShowedOutStatisCpp() {
        reset();
    };
    virtual ~FMyWeavesShowedOutStatisCpp() {};

    void reset() {
        m_bMenQing = false;
        m_bHaveShunZi = false;
        m_eUnifiedNumCardType = MyMJCardValueTypeCpp::Invalid;
        m_iWeaveCount = 0;
    };

public:

    bool m_bMenQing;
    bool m_bHaveShunZi; //equal to chi

    MyMJCardValueTypeCpp m_eUnifiedNumCardType;

    int32 m_iWeaveCount;
};

//Here, define is limited to card type, and ZuHeLong here require remaining 5 cards must be common weave, so it is different from the meaning in score calc.
//standard hu in game
#define MJHuCardTypePRiCommonHu 200
#define MJHuCardTypePRiSpecial7Dui    400
#define MJHuCardTypePRiSpecial13Yao    410
#define MJHuCardTypePRiSpecial13BuKao    420   //for card type, 13 bu kao != ZuHeLong, in score calc, 13Bukao may also met ZuHeLong
#define MJHuCardTypePRiSpecialZuHeLong    430 //in score calcm 13Bukao can't be met for sure

//local hu in game
#define MJHuCardTypePRiLocalAllJiang258 300

//local hu when born (dist card done)
#define MJHuCardTypePRiBornLocalNoJiang258 100
#define MJHuCardTypePRiBornLocalLackNumCardType 110
#define MJHuCardTypePRiBornLocal2KeZi 120
#define MJHuCardTypePRiBornLocalAnGangInHand 130


UCLASS()
class UMyMJUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    //GENERATED_UCLASS_BODY()


        //UFUNCTION(BlueprintCallable, Category = "Analytics")
        //static bool StartSession();

public:
    
    //@iMask is the storage, @iTestingBitValue is the value such as 0x02, 0x04, 0x08. multi bit such as 0xff is illegal
    static inline
    bool getBoolValueFromBitMask(int32 iMask, int32 iTestingBitValue)
    {
        return (iMask & iTestingBitValue) > 0;
    };

    static inline
    void setBoolValueToBitMask(int32 &iMask, int32 iTestingBitValue, bool bV)
    {
        if (bV) {
            iMask |= iTestingBitValue;
        }
        else {
            iMask &= (~iTestingBitValue);
        }
    };

    static inline
    void testUpdateFlagAndsetBoolValueToStorageBitMask(int32 &iStorageBitMask, int32 iDeltaBitMask, int32 iUpdateFlagBitValue, int32 iResultBitValue)
    {
        if (getBoolValueFromBitMask(iDeltaBitMask, iUpdateFlagBitValue)) {
            bool bV = UMyMJUtilsLibrary::getBoolValueFromBitMask(iDeltaBitMask, iResultBitValue);
            setBoolValueToBitMask(iStorageBitMask, iResultBitValue, bV);
        }
    };

    static inline
    void setUpdateFlagAndBoolValueToDeltaBitMask(int32 &iDeltaBitMask, int32 iUpdateFlagBitValue, int32 iResultBitValue, bool bV)
    {
        setBoolValueToBitMask(iDeltaBitMask, iUpdateFlagBitValue, true);
        setBoolValueToBitMask(iDeltaBitMask, iResultBitValue, bV);
    };

    //@iBitPosStart must like (n), Not £¨1 << n£©
    static
    int32 getIntValueFromBitMask(int32 iMask, uint32 iBitPosiStart, uint32 iBitLen)
    {
        MY_VERIFY((iBitPosiStart + iBitLen) <= 32);
        MY_VERIFY(iBitLen > 0);

        int32 valueMask = ((1 << iBitLen) - 1);
        int32 valueMaskExpanded = valueMask << iBitPosiStart;

        int32 valueExpanded = iMask & valueMaskExpanded;
        int32 value = valueExpanded >> iBitPosiStart;

        return value;
    };

    static
    void setIntValueToBitMask(int32 &iMask, uint32 iBitPosiStart, uint32 iBitLen, int32 v)
    {
        MY_VERIFY((iBitPosiStart + iBitLen) <= 32);
        MY_VERIFY(iBitLen > 0);

        int32 valueMask = ((1 << iBitLen) - 1);
        int32 valueMaskExpanded = valueMask << iBitPosiStart;

        int32 value = v & valueMask;
        int32 valueExpanded = value << iBitPosiStart;

        iMask &= (~valueMaskExpanded);
        iMask |= valueExpanded;

    };

    /*
    static
    int32 getMidValueWithWeaveType(const TArray<FMyIdValuePair> &aIdValues, MyMJWeaveTypeCpp eWeaveType)
    {
        int32 l = aIdValues.Num();
        MY_VERIFY(l > 0);

        if (eWeaveType == MyMJWeaveTypeCpp::ShunZiAn || eWeaveType == MyMJWeaveTypeCpp::ShunZiMing) {
            int vall = 0;
            for (int32 i = 0; i < l; i++) {
                int32 v = aIdValues[i].m_iValue;
                MY_VERIFY(v > 0); //any time calc value, assert it is a valid one
                vall += v;
            }

            return vall / l;
        }
        else {
            int32 v = aIdValues[0].m_iValue;
            MY_VERIFY(v > 0); //any time calc value, assert it is a valid one
            return v;
        }
    };
    */

    static FString getStringFromEnum(const TCHAR *enumName, uint8 value);

    static int64 nowAsMsFromTick();

    static FString formatStrIdsValues(const TArray<int32> &aIDs, const TArray<int32> &aValues);

    static FString formatStrIdValuePairs(const TArray<FMyIdValuePair> &aIdValues);

    static FString formatMaskString(int32 iMask, uint32 uBitsCount);

    static void convertIdValuePairs2Ids(const TArray<FMyIdValuePair> &aIdValues, TArray<int32> &outaValues);

    //Note if all cards of one weave is removed in @inWeaves, that weave will be deleted
    static bool removeCardByIdInWeaves(TArray<FMyMJWeaveCpp> &inWeaves, int32 id, bool *pOutWeaveDeleted);

    static void resetCardValueInIdValuePairs(TArray<FMyIdValuePair> &m_aIdValuePairs);

    /* only return basic type, not ext */
    UFUNCTION(BlueprintCallable, Category = "MyMJUtilsLibrary")
    static MyMJCardValueTypeCpp getCardValueType(int32 cardValue);

    static bool isCardTypeNumCard(MyMJCardValueTypeCpp type);

    UFUNCTION(BlueprintCallable, Category = "MyMJUtilsLibrary")
    static bool isCardValue258(int32 cardValue);

    UFUNCTION(BlueprintCallable, Category = "MyMJUtilsLibrary")
    static bool isCardValueYao(int32 cardValue);

    UFUNCTION(BlueprintCallable, Category = "MyMJUtilsLibrary")
    static bool isMenQing(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut);

    UFUNCTION(BlueprintCallable, Category = "MyMJUtilsLibrary")
    static void testSleep(float seconds);

    /*
    * @param pOutIdxTriggerWeave the idx found in @inWeaves, if @pInTriggerCardIdx specified, not touched if not found, caller must set its initial value < 0
    */
    static void parseWeaves(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves, const int32 *pInTriggerCardIdx, FMyMJCardParseResultCpp &outResult, int32 *pOutIdxTriggerWeave);

    static int32 getCountOfValueHave4CardsExceptGang(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves);

    static bool isWeavesHaveShunZi(const TArray<FMyMJWeaveCpp> &inWeaves);

    static MyMJCardValueTypeCpp getWeavesUnifiedNumCardType(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves);

    static void getWeavesShowedOutStatis(const FMyMJCardValuePackCpp &inValuePack, const TArray<FMyMJWeaveCpp> &inWeaves, FMyWeavesShowedOutStatisCpp &outResult);


    /**
    * note: PengPenghu, QingYiSe always doesn't require 258Jiang
    * @param inHuCardTypeCfgs assume higher prioroty hu is at tail, conform to this rule may speed up a bit
    * 
    */
    static bool checkHu(const FMyMJCardValuePackCpp &inValuePack,
                        const FMyMJHuCommonCfg &inHuCommonCfg,
                        const TArray<MyMJHuCardTypeCpp> inHuCardTypeCfgs,
                        const TArray<FMyMJWeaveCpp> &inWeavesShowOut,
                        const FMyMJValueIdMapCpp &inHandCardMap,
                        int32 extraId,
                        int32 extraValue,
                        MyMJHuCardTypeCpp &outHuType);

    /*
    * @param outScoreResult make sure it has called init() before
    */
    //static void calcWeaveScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
    //                           const TArray<FMyMJWeaveCpp> &inWeavesInHand,
    //                           const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
    //                           const FMyMJHuActionAttrCpp &inHuActionAttr,
   //                            FMyMJScoreCalcResultCpp &outScoreResult);

    static void calc7DuiScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                              const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                              const FMyMJHuActionAttrCpp &inHuActionAttr,
                              FMyMJScoreCalcResultCpp &outScoreResult);

    static void calc13YaoScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                               const FMyMJHuActionAttrCpp &inHuActionAttr,
                               FMyMJScoreCalcResultCpp &outScoreResult);

    static void calc13BuKaoScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                 const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                                 const FMyMJHuActionAttrCpp &inHuActionAttr,
                                 FMyMJScoreCalcResultCpp &outScoreResult);

    //Note: this function also fill in outScoreResult.m_aWeavesInHand
    static void calcZuHeLongScore(const FMyMJHuCommonCfg &inHuCommonCfg,
                                  const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                  const FMyMJValueIdMapCpp &inCardsValueIdMapInHand,
                                  const FMyMJHuActionAttrCpp &inHuActionAttr,
                                  FMyMJScoreCalcResultCpp &outScoreResult);

    static void calcHuOtherLocalScore(const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                      const FMyMJHuActionAttrCpp &inHuActionAttr,
                                      FMyMJScoreCalcResultCpp &outScoreResult);

    //assume all pushed in, incuding the trigger card
    static void calHuScoreAllCardIn(const FMyMJHuCfgCpp &inHuCfg,
                                    const FMyMJHuActionAttrCpp &inHuActionAttr,
                                    const TArray<FMyMJWeaveCpp> &inWeavesShowedOut,
                                    const FMyMJValueIdMapCpp &inHandCardMap,
                                    FMyMJScoreCalcResultCpp &outScoreResultMax);

    static int32 getIdxOfUntakenSlotHavingCard(const TArray<FMyIdCollectionCpp> &aUntakenCardStacks, int32 idxBase, uint32 delta, bool bReverse);


protected:

    static void calcHuScorePostProcess(const FMyMJHuActionAttrBaseCpp &inHuActionAttrBase, const FMyTriggerDataCpp *pTriggerData, const TArray<FMyMJWeaveCpp> &inWeavesShowedOut, FMyMJScoreCalcResultCpp &outScoreResult);
};