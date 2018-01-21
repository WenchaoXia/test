// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CommonUtils/MyCommonUtilsLibrary.h"

#include "MyCardGameUtilsLibrary.generated.h"

USTRUCT()
struct FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp : public FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp() : Super()
    {
        reset();
    };

    virtual ~FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp()
    {

    };

    inline void reset()
    {
        Super::reset();

        m_iIdxAttenderBelongTo = 0;

    };

    //    2
    // 3 [ ] 1
    //    0
    int32 m_iIdxAttenderBelongTo;
};

//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyCardGameUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static void helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(int32 iIdxAttenderBelongTo,
                                                                       FVector centerMapped,
                                                                       FVector PointMapped,
                                                                       FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta);

    static void helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(int32 iIdxAttenderBelongTo,
                                                                                        FVector centerMapped,
                                                                                        const FVector2D& pointPercent,
                                                                                        FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp& outMeta);

    static void helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FVector& pointInWorld, FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta);


};