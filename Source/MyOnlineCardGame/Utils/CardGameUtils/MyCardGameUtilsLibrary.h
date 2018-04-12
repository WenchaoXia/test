// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCardGameInterface.h"

#include "MyCardGameUtilsLibrary.generated.h"

//3 layer of idx:
//
//idxAttender
//idxDeskPosition
//idxScreenPosition
//
//idxDeskPosition:
// ---------------
// |      2      |
// |3           1|
// |      0      |
// ---------------
//or
// ---------------
// |             |
// |2           1|
// |      0      |
// ---------------
//or
// ---------------
// |      1      |
// |             |
// |      0      |
// ---------------
//
//idxScreenPosition:
// ---------------
// |      2      |
// |3           1|
// |      0      |
// ---------------

//a widget support per-viewer animation
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp : public UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp, public IMyCardGameScreenPositionRelatedWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
    };

    virtual ~UMyUserWidgetWithCurveUpdaterCardGameScreenPositionRelatedCpp()
    {

    };

    //int32 restartMainAnimation_Implementation(int32 idxScreenPosition, float time, FVector2D offsetShowPoint, FVector2D offsetScreenCenter) override { return 0; };

    IMyCardGameScreenPositionRelatedWidgetInterfaceCpp_DefaultEmptyImplementationForUObject();
    //int32 restartMainAnimation_Implementation(int32 idxScreenPosition, float time, FVector2D offsetShowPoint, FVector2D offsetScreenCenter) { return 0; }

    /*
    int32 restartMainAnimation_Implementation(int32 idxScreenPosition, float time, FVector2D offsetShowPoint, FVector2D offsetScreenCenter) override 
    { 
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: restartMainAnimation only implemented in C++."), *GetClass()->GetName()); 
        return 0; 
    };
    */
};


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

        m_iIdxScreenPositionBelongTo = 0;

    };

    int32 m_iIdxScreenPositionBelongTo;
};

//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyCardGameUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static void helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(int32 idxViewPositionBelongTo,
                                                                       const FVector2D& center,
                                                                       const FVector2D& Point,
                                                                       FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta);

    static void helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(int32 idxViewPositionBelongTo,
                                                                                        const FVector2D& center,
                                                                                        const FVector2D& pointPercent,
                                                                                        FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp& outMeta);

    static void helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FVector& pointInWorld, FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta);


    static int32 idxAttenderToIdxDeskPosition(int32 idxAttender, int32 attenderNum);
    static int32 idxDeskPositionToIdxAttender(int32 idxDeskPosition, int32 attenderNum);

    static int32 idxDeskPositionToIdxScreenPosition(int32 idxDeskPosition, int32 idxDeskPositionOfIdxScreenPosition0);
    static int32 IdxScreenPositionToIdxDeskPosition(int32 IdxScreenPosition, int32 idxDeskPositionOfIdxScreenPosition0);
};