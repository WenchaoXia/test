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

UINTERFACE(Blueprintable)
class UMyMJGameInRoomDeskInterface : public UInterface
{
    GENERATED_BODY()
};

//by default, the API's card actors param passed in, are attender related, except those have tipped not only attender's 
class IMyMJGameInRoomDeskInterface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent)
    int32 showAttenderThrowDices(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                 const FMyMJGameDeskVisualPointCfgCpp& diceVisualPointCfg, const FMyMJDiceModelInfoBoxCpp& diceModelInfo,
                                 int32 number0, int32 number1, int32 seed, int32 uniqueId,
                                 const TArray<class AMyMJGameDiceBaseCpp *>& aDices);


    UFUNCTION(BlueprintNativeEvent)
    int32 showAttenderCardsDistribute(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                      const TArray<int32>& aIdsHandCards, bool isLastDistribution,
                                      const TArray<class AMyMJGameCardBaseCpp*>& cardActorsDistributed,
                                      const TArray<class AMyMJGameCardBaseCpp*>& cardActorsOtherMoving);

    UFUNCTION(BlueprintNativeEvent)
    int32 showAttenderTakeCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                 const TArray<AMyMJGameCardBaseCpp*>& cardActorsTaken);
    

    //@handCardCount is used to tip how many cards left, and the implemention can use it to detect if this card is injecting(jump into the cards stacks)
    UFUNCTION(BlueprintNativeEvent)
    int32 showAttenderGiveOutCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                    int32 handCardsCount,
                                    const TArray<AMyMJGameCardBaseCpp*>& cardActorsGivenOut,
                                    const TArray<AMyMJGameCardBaseCpp*>& cardActorsOtherMoving);


    //All card actors passed in, are attender related
    UFUNCTION(BlueprintNativeEvent)
    int32 showAttenderWeave(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                            MyMJGameWeaveVisualTypeCpp weaveVsualType, const struct FMyMJWeaveCpp& weave,
                            const TArray<class AMyMJGameCardBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardBaseCpp*>& cardActorsOtherMoving);
};

UINTERFACE(Blueprintable)
class UMyMJGameInRoomUIMainInterface : public UInterface
{
    //GENERATED_BODY()
    GENERATED_UINTERFACE_BODY()
};

class IMyMJGameInRoomUIMainInterface
{
    //GENERATED_BODY()
    GENERATED_IINTERFACE_BODY()

public:

    //some important event happend, like game start or end
    UFUNCTION(BlueprintNativeEvent)
    int32 showImportantGameStateUpdated(float dur, MyMJGameStateCpp newGameState);

    //return error code, 0 means OK
    UFUNCTION(BlueprintNativeEvent)
    int32 showAttenderWeave(float dur, int32 idxAttender, MyMJGameWeaveVisualTypeCpp weaveVsualType);

    UFUNCTION(BlueprintNativeEvent)
    int32 changeViewPosition(int32 idxAttender);

protected:

};

UINTERFACE()
class UMyPawnUIInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IMyPawnUIInterface
{
    GENERATED_IINTERFACE_BODY()

public:

    virtual void OnPossessedByLocalPlayerController(APlayerController* newController) = NULL;
    virtual void OnUnPossessedByLocalPlayerController(APlayerController* oldController) = NULL;

protected:

};