// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.generated.h"

USTRUCT(BlueprintType)
struct FMyMJCardInfoPackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardInfoPackCpp()
    {
    };

    virtual ~FMyMJCardInfoPackCpp()
    {

    };

    void reset(int32 iCardNum)
    {
        m_aCardInfos.Reset(iCardNum);
        m_aCardInfos.AddDefaulted(iCardNum);
    };

    inline
    int32 getLength() const
    {
        return m_aCardInfos.Num();
    };

    inline
    FMyMJCardInfoCpp *getByIdx(int32 idx)
    {
        return const_cast<FMyMJCardInfoCpp *>(getByIdxConst(idx));
    };

    const FMyMJCardInfoCpp *getByIdxConst(int32 idx) const;

    void helperVerifyInfos() const;

    //this function will insert @id into @aIds, and update correspond info in self's m_aCardInfos
    void helperInsertCardUniqueToIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id);

    //return true if a recalc of cards with Id in whole array @aIds, happend
    bool helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id);

    void helperRecalcMinorPosiIfCardsInIdArray(const TArray<int32> &aIds);

protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card infos"))
    TArray<FMyMJCardInfoCpp> m_aCardInfos;

};

USTRUCT(BlueprintType)
struct FMyMJCardValuePackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardValuePackCpp()
    {
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

    void reset(const TArray<int32> &aShuffledValues)
    {
        m_aCardValues = aShuffledValues;
    };

    inline
    int32 getLength() const
    {
        return m_aCardValues.Num();
    };

    inline
    int32 getByIdx(int32 idx) const
    {
        MY_VERIFY(idx >= 0 && idx < m_aCardValues.Num());
        return m_aCardValues[idx];
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


    void getValuesByIds(const TArray<int32> &aIds, TArray<int32> &outaValues) const;

    void getIdValuePairsByIds(const TArray<int32> &aIds, TArray<FMyIdValuePair> &outaIdValuePairs) const;

    //will fiill in with card value
    void helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs);

    void helperResolveValue(FMyIdValuePair &pair);

    void helperResolveValues(TArray<FMyIdValuePair> &aPairs);



protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card values"))
    TArray<int32> m_aCardValues;
};