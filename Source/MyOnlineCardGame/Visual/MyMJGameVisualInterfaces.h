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
                                      const TArray<class AMyMJGameCardBaseCpp*>& cardActorsDistributed,
                                      const TArray<class AMyMJGameCardBaseCpp*>& cardActorsOtherMoving);

    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderTakeCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                 const TArray<AMyMJGameCardBaseCpp*>& cardActorsTaken);
    

    //@handCardCount is used to tip how many cards left, and the implemention can use it to detect if this card is injecting(jump into the cards stacks)
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderGiveOutCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                    int32 handCardsCount,
                                    const TArray<AMyMJGameCardBaseCpp*>& cardActorsGivenOut,
                                    const TArray<AMyMJGameCardBaseCpp*>& cardActorsOtherMoving);


    //All card actors passed in, are attender related
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                            MyMJGameEventVisualTypeCpp weaveVisualType, const struct FMyMJWeaveCpp& weave,
                            const TArray<class AMyMJGameCardBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardBaseCpp*>& cardActorsOtherMoving);
};

UINTERFACE(NotBlueprintable)
class UMyMJGameInRoomUIMainWidgetInterfaceCpp : public UInterface
{
    //GENERATED_BODY()
    GENERATED_UINTERFACE_BODY()
};

class IMyMJGameInRoomUIMainWidgetInterfaceCpp
{
    //GENERATED_BODY()
    GENERATED_IINTERFACE_BODY()

public:

    //some important event happend, like game start or end
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showImportantGameStateUpdated(float dur, MyMJGameStateCpp newGameState);

    //return error code, 0 means OK
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVisualType);

    //max is always 4
    virtual FMyErrorCodeMJGameCpp changeDeskPositionOfIdxScreenPosition0(int32 idxDeskPositionOfIdxScreenPosition0)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("changeDeskPositionOfIdxScreenPosition0 must be override!"));
        MY_VERIFY(false);
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass);
    };

protected:

    //never fail, coredump in that case
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp getInRoomPlayerInfoWidgetByScreenPositionEnsured(int32 idxScreenPosition, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget);
};