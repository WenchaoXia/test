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

void UMyCardGameUtilsLibrary::helperUpdatePointOnPlayerScreenConstrainedMeta(int32 iIdxAttenderBelongTo,
                                                                            FVector centerMapped,
                                                                            FVector PointMapped,
                                                                            FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta &outMeta)
{
    PointMapped.Z = 0;

    outMeta.m_iIdxAttenderBelongTo = iIdxAttenderBelongTo;
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

void UMyCardGameUtilsLibrary::helperResolvePointOnPlayerScreenConstrainedMeta(const UObject* WorldContextObject, const FVector& pointInWorld, FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta &outMeta)
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

    helperUpdatePointOnPlayerScreenConstrainedMeta(idxAttenderOnScreen, centerMapped, PointMapped, outMeta);
}

void UMyCardGameUtilsLibrary::helperResolveTransformFromPointOnPlayerScreenConstrainedMeta(const UObject* WorldContextObject, const FMyCardGamePointFromCenterOnPlayerScreenConstrainedMeta &meta,
                                                                                            float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                            const FVector2D& targetPosiFixOnScreenPercent,
                                                                                            float targetVOnScreenPercent,
                                                                                            float targetModelHeightInWorld,
                                                                                            FTransform &outTargetTranform)
{
    FVector popPointMapped = meta.m_cScreenCenterMapped + targetPosiFromCenterToBorderOnScreenPercent * meta.m_fCenterToPointUntilBorderLength * meta.m_cDirectionCenterToPointMapped;
    FVector2D popPoint;
    popPoint.X = popPointMapped.X;
    popPoint.Y = popPointMapped.Y;

    popPoint.X += meta.m_cScreenCenterMapped.X * 2 * targetPosiFixOnScreenPercent.X;
    popPoint.Y += meta.m_cScreenCenterMapped.Y * 2 * targetPosiFixOnScreenPercent.Y;

    float vAbsoluteOnScreen = targetVOnScreenPercent * meta.m_cScreenCenterMapped.Y * 2;

    FVector cameraCenterLoc, cameraCenterDir;
    UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByAbsolute(WorldContextObject, popPoint, vAbsoluteOnScreen, targetModelHeightInWorld, outTargetTranform, cameraCenterLoc, cameraCenterDir);
}