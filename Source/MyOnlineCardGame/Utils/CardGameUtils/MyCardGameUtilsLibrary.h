// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utils/CommonUtils/MyCommonUtilsLibrary.h"

#include "MyCardGameUtilsLibrary.generated.h"


struct FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta
{

public:
    FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta()
    {
        reset();
    };

    virtual ~FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta()
    {

    };

    inline void reset()
    {
        m_iIdxAttenderBelongTo = 0;
        m_cScreenPointMapped = m_cDirectionCenterToPointMapped = FVector::ZeroVector;
        m_cScreenCenterMapped = FVector(1, 1, 0);
        m_fCenterToPointLength = m_fCenterToPointUntilBorderLength = 0;
    };

    FVector m_cScreenPointMapped; //Z is 0

    //    2
    // 3 [ ] 1
    //    0
    int32 m_iIdxAttenderBelongTo;
    FVector m_cScreenCenterMapped; //Z is 0
    FVector m_cDirectionCenterToPointMapped; //Z is 0
    float m_fCenterToPointLength;
    float m_fCenterToPointUntilBorderLength;
};

USTRUCT()
struct FMyCardGamePointFromCenterOnPlayerScreenCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCardGamePointFromCenterOnPlayerScreenCfgCpp()
    {
        reset();
    };

    virtual ~FMyCardGamePointFromCenterOnPlayerScreenCfgCpp()
    {

    };

    inline void reset()
    {
        m_fShowPosiFromCenterToBorderPercent = 0;
        m_cExtraOffsetScreenPercent = FVector2D::ZeroVector;
        m_fTargetVLengthOnScreenScreenPercent = 0.1;
    };

    UPROPERTY()
        float m_fShowPosiFromCenterToBorderPercent;

    UPROPERTY()
        FVector2D m_cExtraOffsetScreenPercent;

    UPROPERTY()
        float m_fTargetVLengthOnScreenScreenPercent;
};


//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyCardGameUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static void helperResolvePointOnPlayerScreenConstrainedMeta(const UObject* WorldContextObject, const FVector& pointInWorld, FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta &outMeta);

    static void helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(const UObject* WorldContextObject, const FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta &meta,
                                                                            float targetPosiFromCenterToBorderOnScreenPercent,
                                                                            const FVector2D& targetPosiFixOnScreenPercent,
                                                                            float targetVOnScreenPercent,
                                                                            float targetModelHeightInWorld,
                                                                            FTransform &outTargetTranform);
};