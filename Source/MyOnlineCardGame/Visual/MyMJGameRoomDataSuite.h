// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyMJGameVisualCard.h"

#include "MyMJGameDeskVisualData.h"
#include "MyMJGameTrivalVisualData.h"

#include "MyMJGameRoomDataSuite.generated.h"

//A data suit contains all data needed for visualization in a specified range
//the design is: since our game needs to visualize historic actions, unlike fps, so we must process frame by frame, or one period one time, likey, so we keep history queue for important datas
UCLASS()
class UMyMJGameRoomDataSuite : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJGameRoomDataSuite();

    virtual ~UMyMJGameRoomDataSuite();

    inline UMyMJGameDeskVisualDataObjCpp* getDeskDataObjVerified()
    {
        MY_VERIFY(IsValid(m_pDeskDataObj));
        return m_pDeskDataObj;
    };

protected:

    //contains all data need accurate history
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (DisplayName = "desk data obj"))
        UMyMJGameDeskVisualDataObjCpp* m_pDeskDataObj;

    //contains not important data, which means it's history may miss some event
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (DisplayName = "trival data obj"))
        UMyMJGameTrivalVisualDataObjCpp *m_pTrivalDataObj;
};