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
                                                                            FVector centerMapped,
                                                                            FVector PointMapped,
                                                                            FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp &outMeta)
{
    PointMapped.Z = 0;

    outMeta.m_iIdxScreenPositionBelongTo = idxViewPositionBelongTo;
    outMeta.m_cScreenCenterMapped = centerMapped; //Z is 0
    outMeta.m_cScreenCenterMapped.Z = 0;

    FVector& dir = outMeta.m_cDirectionCenterToPointMapped;
    float& lenToPoint = outMeta.m_fCenterToPointLength;
    (PointMapped - centerMapped).ToDirectionAndLength(dir, lenToPoint);

    float xLen = BIG_NUMBER;
    if (!FMath::IsNearlyEqual(dir.X, 0, KINDA_SMALL_NUMBER)) {
        xLen = FMath::Abs(centerMapped.X / dir.X);
    }
    float yLen = BIG_NUMBER;
    if (!FMath::IsNearlyEqual(dir.Y, 0, KINDA_SMALL_NUMBER)) {
        yLen = FMath::Abs(centerMapped.Y / dir.Y);
    }
    outMeta.m_fCenterToPointUntilBorderLength = FMath::Min(xLen, yLen);
}

void UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrainedByPointPercent(int32 idxViewPositionBelongTo,
                                                                                                    FVector centerMapped,
                                                                                                    const FVector2D& pointPercent,
                                                                                                    FMyCardGamePointAndCenterMetaOnPlayerScreenConstrainedCpp& outMeta)
{
    FVector pointMapped;
    pointMapped.X = pointPercent.X * centerMapped.X * 2;
    pointMapped.Y = pointPercent.Y * centerMapped.Y * 2;
    pointMapped.Z = 0;

    UMyCardGameUtilsLibrary::helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(idxViewPositionBelongTo, centerMapped, pointMapped, outMeta);
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

    FVector centerMapped;
    FVector PointMapped;
    centerMapped.X = constrainedScreenSize.X / 2;
    centerMapped.Y = constrainedScreenSize.Y / 2;

    PointMapped.X = projectedPoint.X;
    PointMapped.Y = projectedPoint.Y;

    helperUpdatePointAndCenterMetaOnPlayerScreenConstrained(idxAttenderOnScreen, centerMapped, PointMapped, outMeta);
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