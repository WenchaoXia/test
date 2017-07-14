// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.generated.h"

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

    void resetMinorData() {
        m_iIdxInSlot0 = -1;
        m_iIdxInSlot1 = -1;
    };
    
    bool equal(const FMyMJCardPosiCpp &other)
    {
        return m_iIdxAttender == other.m_iIdxAttender && m_eSlot == other.m_eSlot && m_iIdxInSlot0 == other.m_iIdxInSlot0 && m_iIdxInSlot1 == other.m_iIdxInSlot1;
    };

    UPROPERTY()
    int32 m_iIdxAttender;

    UPROPERTY()
    MyMJCardSlotTypeCpp m_eSlot;

    // < 0 means invalid
    UPROPERTY()
    int32 m_iIdxInSlot0;

    // < 0 means invalid
    UPROPERTY()
    int32 m_iIdxInSlot1;

};

USTRUCT()
struct FMyMJCardCpp : public FMyIdValuePair
{
    GENERATED_USTRUCT_BODY()

    FMyMJCardCpp() : Super()
    {
        resetExceptValue();
    };

    void resetExceptValue() {
        FMyIdValuePair::reset(false);

        m_eFlipState = MyMJCardFlipStateCpp::Invalid;
        m_cPosi.reset();
    };

    UPROPERTY()
    MyMJCardFlipStateCpp m_eFlipState;

    UPROPERTY()
    FMyMJCardPosiCpp m_cPosi;
};


USTRUCT()
struct FMyMJCardPackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardPackCpp()
    {
    };

    virtual ~FMyMJCardPackCpp()
    {

    };


    void reset(TArray<int32> m_aShuffledValues);

    void getValuesByIds(const TArray<int32> &aIds, TArray<int32> &outaValues) const;

    void getIdValuePairsByIds(const TArray<int32> &aIds, TArray<FMyIdValuePair> &outaIdValuePairs) const;

    inline
    FMyMJCardCpp *getCardByIdx(int32 idx)
    {
        return const_cast<FMyMJCardCpp *>(getCardByIdxConst(idx));
    };

    const FMyMJCardCpp *getCardByIdxConst(int32 idx) const;


    void revealCardValue(int32 id, int32 value);

    inline void revealCardValue(const FMyIdValuePair &idValuePair)
    {
        revealCardValue(idValuePair.m_iId, idValuePair.m_iValue);
    };

    inline void revealCardValueByIdValuePairs(const TArray<FMyIdValuePair> &aIdValuePairs)
    {
        int32 l = aIdValuePairs.Num();
        for (int32 i = 0; i < l; i++) {
            revealCardValue(aIdValuePairs[i]);
        }
    };

    //void updateCardData(int32 id, int32 value);

    inline
    int32 getLength() {
        return m_aCards.Num();
    };

    //this function will check values, and assert if error happens
    void helperVerifyValues();

    //will fiill in with card value
    void helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs);

    void helperResolveValue(FMyIdValuePair &pair);

    void helperResolveValues(TArray<FMyIdValuePair> &aPairs);


    static void helperInsertCardUniqueToIdArrayWithMinorPosiCalced(FMyMJCardPackCpp &cardPack, TArray<int32> &aIds, int32 id);

    //return true if a recalc of cards with Id in whole array @aIds, happend
    static bool helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(FMyMJCardPackCpp &cardPack, TArray<int32> &aIds, int32 id);

    static void helperRecalcMinorPosiIfCardsInIdArray(FMyMJCardPackCpp &cardPack, const TArray<int32> &aIds);

protected:

    TArray<FMyMJCardCpp> m_aCards;

};