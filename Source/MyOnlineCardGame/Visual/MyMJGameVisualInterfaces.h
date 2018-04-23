// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "MyMJGameVisualCommon.h"

#include "MyMJGameVisualInterfaces.generated.h"

//It seems UE4 have a bug that all function must have return value to let blueprint implement it
//All API here return error code, and 0 is returned if OK, if implementable by blueprint


UINTERFACE()
class UMyMJGameInRoomDeskInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

//by default, the API's card actors param passed in, are attender related, except those have tipped not only attender's 
class IMyMJGameInRoomDeskInterfaceCpp
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderThrowDices(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                 int32 diceVisualStateKey,
                                                 const TArray<class AMyMJGameDiceBaseCpp *>& aDices);


    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderCardsDistribute(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                      const TArray<int32>& aIdsHandCards, bool isLastDistribution,
                                      const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsDistributed,
                                      const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving);

    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderTakeCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                 const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsTaken);
    

    //@handCardCount is used to tip how many cards left, and the implemention can use it to detect if this card is injecting(jump into the cards stacks)
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderGiveOutCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                    int32 handCardsCount,
                                    const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsGivenOut,
                                    const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving);


    //All card actors passed in, are attender related
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                            MyMJGameEventVisualTypeCpp weaveVisualType, const struct FMyMJWeaveCpp& weave,
                            const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving);
};


UINTERFACE(NotBlueprintable)
class UMyMJGameInRoomUIMainWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomUIMainWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    //some important event happend, like game start or end
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showImportantGameStateUpdated(float dur, MyMJGameStateCpp newGameState);

    //return error code, 0 means OK
    virtual FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVisualType) = NULL;


    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPublicChanged(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic, int32 subType) = NULL;

    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPrivateChanged(int32 idxAttender, const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate, int32 subType) = NULL;

    //max is always 4
    virtual FMyErrorCodeMJGameCpp updateAttenderPositions(float XYRatioOfplayerScreen, const TArray<FVector2D>& projectedPointsInPlayerScreen_unit_absolute) = NULL;
    //{
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("changeDeskPositionOfIdxScreenPosition0 must be override!"));
        //MY_VERIFY(false);
        //return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass);
    //};

protected:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getInRoomPlayerInfoWidgetByIdxAttender(int32 idxAttender, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getCfg(FMyMJGameInRoomUIMainWidgetCfgCpp& outCfg);
};


#define IMyMJGameInRoomUIMainWidgetInterfaceCpp_DefaultImplementationForUObject_Bp() \
protected: \
virtual FMyErrorCodeMJGameCpp showImportantGameStateUpdated_Implementation(float dur, MyMJGameStateCpp newGameState) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: showImportantGameStateUpdated only implemented in C++."), *GetClass()->GetName()); \
    return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint); \
}; \
\
virtual MyErrorCodeCommonPartCpp getInRoomPlayerInfoWidgetByIdxAttender_Implementation(int32 idxAttender, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getInRoomPlayerInfoWidgetByIdxAttender only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getCfg_Implementation(FMyMJGameInRoomUIMainWidgetCfgCpp& outCfg) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getCfg only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected:
