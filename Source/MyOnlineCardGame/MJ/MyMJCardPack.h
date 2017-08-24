// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Utils/MyMJUtils.h"

#include "MyMJCardPack.generated.h"

USTRUCT()
struct FMyMJCardInfoPackCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJCardInfoPackCpp()
    {
        reset(0);
    };

    virtual ~FMyMJCardInfoPackCpp()
    {

    };

    void reset(int32 iCardNum)
    {
        m_aCardInfos.Reset(iCardNum);
        for (int32 i = 0; i < iCardNum; i++) {
            int32 idx = m_aCardInfos.Emplace();
            m_aCardInfos[idx].m_iId = i;
        }

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

    inline
    FMyMJCardInfoCpp& getRefByIdx(int32 idx)
    {
        FMyMJCardInfoCpp *pRet = getByIdx(idx);
        MY_VERIFY(pRet);
        return *pRet;
    };

    inline
    const FMyMJCardInfoCpp& getRefByIdxConst(int32 idx) const
    {
        return *getByIdxConst(idx);
    };



    void helperVerifyInfos() const;

    //return true if a recalc of cards with Id in whole array @aIds append, assert id is unique
    bool helperInsertCardUniqueToIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id);

    //return true if a recalc of cards with Id in whole array @aIds happend, assert id is unique
    bool helperRemoveCardUniqueFromIdArrayWithMinorPosiCalced(TArray<int32> &aIds, int32 id, bool *outbHaveRemovedOne = NULL);

    void helperRecalcMinorPosiOfCardsInIdArray(const TArray<int32> &aIds);

    void helperRecalcMinorPosiOfCardsInShowedOutWeaves(const TArray<FMyMJWeaveCpp> &aShowedOutWeaves);

    //slow, return the idx, < 0  means not found
    static int32 helperFindCardByIdInCardInfos(const TArray<FMyMJCardInfoCpp>& aCardInfos, int32 idCard);

protected:

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card infos"))
    TArray<FMyMJCardInfoCpp> m_aCardInfos;

};