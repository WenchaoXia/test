// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCardGameUtilsLibrary.h"

#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

void UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(int32 idxViewPositionBelongTo,
                                                                            const FVector2D& center,
                                                                            const FVector2D& Point,
                                                                            FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta)
{
    outMeta.m_iIdxScreenPositionBelongTo = idxViewPositionBelongTo;
    outMeta.m_cScreenCenter = center;


    FVector2D& dir = outMeta.m_cDirectionCenterToPoint;
    (Point - center).ToDirectionAndLength(dir, outMeta.m_fCenterToPointLength);

    float xLen = BIG_NUMBER;
    if (!FMath::IsNearlyEqual(dir.X, 0, KINDA_SMALL_NUMBER)) {
        xLen = FMath::Abs(center.X / dir.X);
    }
    float yLen = BIG_NUMBER;
    if (!FMath::IsNearlyEqual(dir.Y, 0, KINDA_SMALL_NUMBER)) {
        yLen = FMath::Abs(center.Y / dir.Y);
    }
    outMeta.m_fCenterToPointUntilBorderLength = FMath::Min(xLen, yLen);
}

void UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(int32 idxViewPositionBelongTo,
                                                                                                    const FVector2D& center,
                                                                                                    const FVector2D& pointPercent,
                                                                                                    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp& outMeta)
{
    FVector2D point = pointPercent * center * 2;

    UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(idxViewPositionBelongTo, center, point, outMeta);
};

void UMyCardGameUtilsLibrary::helperPointInWorldToPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FVector& pointInWorld, FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta)
{
    outMeta.reset();

    FVector2D projectedPoint;
    if (!UMyCommonUtilsLibrary::myProjectWorldToScreen(WorldContextObject, pointInWorld, true, projectedPoint)) {
        return;
    }
    FVector2D constrainedScreenSize, fullScreenSize;
    UMyCommonUtilsLibrary::getPlayerScreenSizeAbsolute(WorldContextObject, constrainedScreenSize, fullScreenSize);

    //judge which attender in screen
    float supposedY = constrainedScreenSize.Y / constrainedScreenSize.X * projectedPoint.X;
    float mirroredY = constrainedScreenSize.Y - supposedY;

    int32 idxAttenderOnScreen;
    if (projectedPoint.X < (constrainedScreenSize.X / 2))
    {
        if (projectedPoint.Y < supposedY) {
            idxAttenderOnScreen = 2;
        }
        else if (projectedPoint.Y < mirroredY) {
            idxAttenderOnScreen = 3;
        }
        else {
            idxAttenderOnScreen = 0;
        }
    }
    else {
        if (projectedPoint.Y < mirroredY) {
            idxAttenderOnScreen = 2;
        }
        else if (projectedPoint.Y < supposedY) {
            idxAttenderOnScreen = 1;
        }
        else {
            idxAttenderOnScreen = 0;
        }
    }


    FVector2D center = constrainedScreenSize / 2;
    FVector2D& Point = projectedPoint;

    helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(idxAttenderOnScreen, center, Point, outMeta);
}

int32 UMyCardGameUtilsLibrary::idxAttenderToIdxDeskPosition(int32 idxAttender, int32 attenderNum)
{
    MY_VERIFY(idxAttender >= 0 && idxAttender < attenderNum);

    if (attenderNum == 4) {
        return idxAttender;
    }
    else if (attenderNum == 3) {
        if (idxAttender < 2) {
            return idxAttender;
        }
        else {
            return 3;
        }

    }
    else if (attenderNum == 2) {
        return idxAttender * 2;
    }
    else {
        MY_VERIFY(false);
        return -1;
    }
}

int32 UMyCardGameUtilsLibrary::idxDeskPositionToIdxAttender(int32 idxDeskPosition, int32 attenderNum)
{
    MY_VERIFY(idxDeskPosition >= 0 && idxDeskPosition < 4);

    if (attenderNum == 4) {
        return idxDeskPosition;
    }
    else if (attenderNum == 3) {
        if (idxDeskPosition < 2) {
            return idxDeskPosition;
        }
        else if (idxDeskPosition == 3) {
            return 2;
        }
        else {
            MY_VERIFY(false);
            return -1;
        }
    }
    else if (attenderNum == 2) {
        MY_VERIFY((idxDeskPosition % 2) == 0);
        return idxDeskPosition / 2;
    }
    else {
        MY_VERIFY(false);
        return -1;
    }
}

int32 UMyCardGameUtilsLibrary::idxDeskPositionToIdxScreenPosition(int32 idxDeskPosition, int32 idxDeskPositionOfIdxScreenPosition0)
{
    MY_VERIFY(idxDeskPosition >= 0 && idxDeskPosition < 4);
    MY_VERIFY(idxDeskPositionOfIdxScreenPosition0 >= 0 && idxDeskPositionOfIdxScreenPosition0 < 4);

    return (idxDeskPosition + 4 - idxDeskPositionOfIdxScreenPosition0) % 4;
}

int32 UMyCardGameUtilsLibrary::IdxScreenPositionToIdxDeskPosition(int32 IdxScreenPosition, int32 idxDeskPositionOfIdxScreenPosition0)
{
    MY_VERIFY(IdxScreenPosition >= 0 && IdxScreenPosition < 4);
    MY_VERIFY(idxDeskPositionOfIdxScreenPosition0 >= 0 && idxDeskPositionOfIdxScreenPosition0 < 4);

    return (idxDeskPositionOfIdxScreenPosition0 + IdxScreenPosition) % 4;
}