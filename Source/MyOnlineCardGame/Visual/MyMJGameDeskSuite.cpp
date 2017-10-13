// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameDeskSuite.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"


//#include "Runtime/CoreUObject/Public/Templates/SubclassOf.h"

bool UMyMJGameDeskDynamicResManagerCpp::checkSettings() const
{
    if (!IsValid(m_cCardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), m_cCardClass.Get());
        return false;
    }

    AMyMJGameCardBaseCpp *pCDO = m_cCardClass->GetDefaultObject<AMyMJGameCardBaseCpp>();
    if (!IsValid(pCDO)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card pCDO is invalid: %p"), pCDO);
        return false;
    }

    return true;
};


void AMyMJGameDeskSuiteCpp::helperCalcCardTransform(const FMyMJGameCardVisualInfoCpp& cCardVisualInfo, FTransform &outTransform)
{
    /*
    int32 idxAttender = cCardVisualInfo.m_iIdxAttender;
    if (!(idxAttender >= 0 && idxAttender < 4))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idxAttender out of range: %d."), idxAttender);
        return;
    }

    MyMJCardSlotTypeCpp eSlot = cCardVisualInfo.m_eSlot;

    if (eSlot == MyMJCardSlotTypeCpp::Invalid) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eSlot is invalid"));
        return;
    }
    else if (eSlot == MyMJCardSlotTypeCpp::ShownOnDesktop) {

    }
    else {

        MY_VERIFY(IsValid(m_pDeskAreaActor));

        FMyMJGameDeskVisualPointCfgCpp cVisualPointCenter;
        int32 errCode;
        m_pDeskAreaActor->getVisualPointByIdxAttenderAndSlotInternal(idxAttender, eSlot, errCode, cVisualPointCenter);
        MY_VERIFY(errCode == 0);

        const FTransform& cTransFormCenter = cVisualPointCenter.m_cCenterPointTransform;
        const FVector& cAreaBoxExtend = cVisualPointCenter.m_cAreaBoxExtend;
        MyMJGameVerticalAlignmentCpp eRowAlignment = cVisualPointCenter.m_eRowAlignment;
        //int32 iRowMaxNum = cVisualPointCenter.m_iRowMaxNum;
        MyMJGameHorizontalAlignmentCpp eColInRowAlignment = cVisualPointCenter.m_eColInRowAlignment;
        //int32 iColInRowMaxNum = cVisualPointCenter.m_iColInRowMaxNum;
        float fColInRowExtraMarginAbs = cVisualPointCenter.m_fColInRowExtraMarginAbs;
        if (fColInRowExtraMarginAbs < 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fColInRowExtraMarginAbs is negative: %f, forceing to default."), fColInRowExtraMarginAbs);
            fColInRowExtraMarginAbs = 0;
        }

        if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Invalid || eRowAlignment == MyMJGameVerticalAlignmentCpp::Mid) { //we don't support mid allignment now
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRowAlignment row alignment %d, forceing to default!."), (uint8)eRowAlignment);
            eRowAlignment = MyMJGameVerticalAlignmentCpp::Bottom;
        }
        //if (iRowMaxNum <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iRowMaxNum %d, forceing to default!."), iRowMaxNum);
            //iRowMaxNum = 1;
        //}
        if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Invalid || eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid) { //we don't support mid allignment now
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eColInRowAlignment alignment %d, forceing to default!."), (uint8)eColInRowAlignment);
            eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Left;
        }
        //if (iColInRowMaxNum <= 0) {
            //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iColInRowMaxNum %d, forceing to default!."), iColInRowMaxNum);
            //iColInRowMaxNum = 1;
        //}

        const FVector cardBoxExtend;

        int32 idxRow = cCardVisualInfo.m_iIdxRow;
        int32 idxColInRow = cCardVisualInfo.m_iIdxColInRow;
        int32 idxStackInCol = cCardVisualInfo.m_iIdxStackInCol;
        MyMJCardFlipStateCpp eFlipState = cCardVisualInfo.m_eFlipState;
        int32 iXRotate90D = cCardVisualInfo.m_iXRotate90D;
        int32 iXRotate90DBeforeCount = cCardVisualInfo.m_iXRotate90DBeforeCount;
        int32 iColInRowExtraMarginCount = cCardVisualInfo.m_iColInRowExtraMarginCount;

        MY_VERIFY(idxRow >= 0);
        MY_VERIFY(idxColInRow >= 0);
        //MY_VERIFY(CountOfColInRow > 0);
        MY_VERIFY(idxStackInCol >= 0);

        if (iXRotate90DBeforeCount < 0 || iXRotate90DBeforeCount > idxColInRow) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("illegal iXRotate90DBeforeCount: %d but idxColInRow: %d!"), iXRotate90DBeforeCount, idxColInRow);
            iXRotate90DBeforeCount = 0;
        }

        if (iColInRowExtraMarginCount < 0) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("iColInRowExtraMarginCount is negative: %d, forceing to default."), iColInRowExtraMarginCount);
            iColInRowExtraMarginCount = 0;
        }

        //not really count, but only for location calc
        int32 colNormalCountForLocCalcBefore = idxColInRow - iXRotate90DBeforeCount;
        int32 col90DCountForLocCalcBefore = iXRotate90DBeforeCount;

        //prefix if this
        //if ((iXRotate90D % 2) == 1) {
            //colNormalCountForLocCalcBefore--;
            //col90DCountForLocCalcBefore++;
        //}


        if (eFlipState == MyMJCardFlipStateCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eflipstate!"));
            eFlipState = MyMJCardFlipStateCpp::Stand;
        }

        if (cardBoxExtend.IsZero()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cardBoxExtend is zero!"));
        }

        //start the calc
        //our model assume one slot's flip state is by somekind same, that is, they all stand, or all laydown
        FVector locationWorldFinal;
        FRotator rotaterWorldFinal;
        FRotator rotaterLocalDelta;

        FQuat rotater = cTransFormCenter.GetRotation();
        FVector forwardV = rotater.GetForwardVector();
        FVector rightV = rotater.GetRightVector();
        FVector upV = rotater.GetUpVector();

        FVector perRowOffsetB2T; //bottom to top
        FVector perColOffsetL2R = -rightV * cardBoxExtend.Y * 2; //left to right
        FVector perColOffsetX90DL2R = -rightV * cardBoxExtend.Z * 2;
        FVector perStackOffsetB2T; //bottom to top

        FVector extraLocOffset;
        FVector extraLocOffsetB2T;
        FVector extraLocOffsetL2R;
        if (eFlipState == MyMJCardFlipStateCpp::Stand) {
            perRowOffsetB2T = -forwardV * cardBoxExtend.X * 2;
            perStackOffsetB2T = upV * cardBoxExtend.Z * 2;


        }
        else if (eFlipState == MyMJCardFlipStateCpp::Up) {
            perRowOffsetB2T = -forwardV * cardBoxExtend.Z * 2;
            perStackOffsetB2T = upV * cardBoxExtend.X * 2;

            rotaterLocalDelta.Yaw = 90;

            //note: when top to down, there may be alignment issue, here we don't solve it now
            if (iXRotate90D > 0) {
                extraLocOffsetB2T = -forwardV * cardBoxExtend.Y;
                extraLocOffsetL2R = -rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = 90;
            }
            else if (iXRotate90D < 0) {
                extraLocOffsetB2T = -forwardV * cardBoxExtend.Y;
                extraLocOffsetL2R = rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = -90;
            }
        }
        else if (eFlipState == MyMJCardFlipStateCpp::Down) {
            perRowOffsetB2T = -forwardV * cardBoxExtend.Z * 2;
            perStackOffsetB2T = upV * cardBoxExtend.X * 2;

            //note: when top to down, there may be alignment issue, here we don't solve it now
            if (iXRotate90D > 0) {
                extraLocOffsetB2T = forwardV * (cardBoxExtend.Y + 2 * cardBoxExtend.Z - 2 * cardBoxExtend.Y);
                extraLocOffsetL2R = rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = 90;
            }
            else if (iXRotate90D < 0) {
                extraLocOffsetB2T = forwardV * (cardBoxExtend.Y + 2 * cardBoxExtend.Z - 2 * cardBoxExtend.Y);
                extraLocOffsetL2R = -rightV * cardBoxExtend.Z;

                rotaterLocalDelta.Roll = -90;
            }


            //cTransFormCenterFix0.SetLocation(cTransFormCenter.GetLocation() - perRowOffsetB2T + perStackOffsetB2T);
            extraLocOffset += -perRowOffsetB2T + perStackOffsetB2T;
        }
        else {
            MY_VERIFY(false);
        }

        //rowBase + perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + fColInRowExtraMarginAbs * iColInRowExtraMarginCount;
        FVector rowCenter;
        if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Top) {
            rowCenter = cTransFormCenter.GetLocation() - forwardV * cAreaBoxExtend.X - perRowOffsetB2T * idxRow - extraLocOffsetB2T;
        }
        else if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Bottom) {
            rowCenter = cTransFormCenter.GetLocation() + forwardV * cAreaBoxExtend.X + perRowOffsetB2T * idxRow + extraLocOffsetB2T;
        }
        else {
            MY_VERIFY(false);
        }
        
        FVector colPivot;
        FVector colOffsetL2R = perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + fColInRowExtraMarginAbs * iColInRowExtraMarginCount + extraLocOffsetL2R;
        if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Left) {
            colPivot = rowCenter - rightV * cAreaBoxExtend.Y + colOffsetL2R;

        }
        else if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
            colPivot = rowCenter + rightV * cAreaBoxExtend.Y - colOffsetL2R;
        }
        else {
            MY_VERIFY(false);
        }
        colPivot += extraLocOffset;

        locationWorldFinal = colPivot + perStackOffsetB2T * idxStackInCol;
        rotaterWorldFinal = UKismetMathLibrary::ComposeRotators(rotaterLocalDelta, FRotator(cTransFormCenter.GetRotation()));

    }
    */
};

int32 AMyMJGameDeskSuiteCpp::helperCalcCardTransformFromvisualPointCfg(const FMyMJGameCardActorModelInfoCpp& cardModelInfo, const FMyMJGameCardVisualInfoCpp& cardVisualInfo, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FVector& outLocationWorld, FRotator& outRotatorWorld)
{

    const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCenter = visualPointCfg;
    const FMyMJGameCardVisualInfoCpp& cCardVisualInfo = cardVisualInfo;

    const FTransform& cTransFormCenter = cVisualPointCenter.m_cCenterPointWorldTransform;
    const FVector& cAreaBoxExtend = cVisualPointCenter.m_cAreaBoxExtendFinal;
    MyMJGameVerticalAlignmentCpp eRowAlignment = cVisualPointCenter.m_eRowAlignment;
    //int32 iRowMaxNum = cVisualPointCenter.m_iRowMaxNum;
    MyMJGameHorizontalAlignmentCpp eColInRowAlignment = cVisualPointCenter.m_eColInRowAlignment;
    //int32 iColInRowMaxNum = cVisualPointCenter.m_iColInRowMaxNum;
    float fColInRowExtraMarginAbs = cVisualPointCenter.m_fColInRowExtraMarginAbs;
    if (fColInRowExtraMarginAbs < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fColInRowExtraMarginAbs is negative: %f, forceing to default."), fColInRowExtraMarginAbs);
        fColInRowExtraMarginAbs = 0;
    }

    if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Invalid || eRowAlignment == MyMJGameVerticalAlignmentCpp::Mid) { //we don't support mid allignment now
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRowAlignment row alignment %d, forceing to default!."), (uint8)eRowAlignment);
        eRowAlignment = MyMJGameVerticalAlignmentCpp::Bottom;
    }
    //if (iRowMaxNum <= 0) {
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iRowMaxNum %d, forceing to default!."), iRowMaxNum);
    //iRowMaxNum = 1;
    //}
    if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Invalid || eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Mid) { //we don't support mid allignment now
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eColInRowAlignment alignment %d, forceing to default!."), (uint8)eColInRowAlignment);
        eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Left;
    }
    //if (iColInRowMaxNum <= 0) {
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iColInRowMaxNum %d, forceing to default!."), iColInRowMaxNum);
    //iColInRowMaxNum = 1;
    //}

    int32 idxRow = cCardVisualInfo.m_iIdxRow;
    int32 idxColInRow = cCardVisualInfo.m_iIdxColInRow;
    int32 idxStackInCol = cCardVisualInfo.m_iIdxStackInCol;
    MyMJCardFlipStateCpp eFlipState = cCardVisualInfo.m_eFlipState;
    int32 iXRotate90D = cCardVisualInfo.m_iXRotate90D;
    int32 iXRotate90DBeforeCount = cCardVisualInfo.m_iXRotate90DBeforeCount;
    int32 iColInRowExtraMarginCount = cCardVisualInfo.m_iColInRowExtraMarginCount;

    MY_VERIFY(idxRow >= 0);
    MY_VERIFY(idxColInRow >= 0);
    //MY_VERIFY(CountOfColInRow > 0);
    MY_VERIFY(idxStackInCol >= 0);

    if (iXRotate90DBeforeCount < 0 || iXRotate90DBeforeCount > idxColInRow) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("illegal iXRotate90DBeforeCount: %d but idxColInRow: %d!"), iXRotate90DBeforeCount, idxColInRow);
        iXRotate90DBeforeCount = 0;
    }

    if (iColInRowExtraMarginCount < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("iColInRowExtraMarginCount is negative: %d, forceing to default."), iColInRowExtraMarginCount);
        iColInRowExtraMarginCount = 0;
    }

    //not really count, but only for location calc
    int32 colNormalCountForLocCalcBefore = idxColInRow - iXRotate90DBeforeCount;
    int32 col90DCountForLocCalcBefore = iXRotate90DBeforeCount;

    //prefix if this
    //if ((iXRotate90D % 2) == 1) {
    //colNormalCountForLocCalcBefore--;
    //col90DCountForLocCalcBefore++;
    //}


    if (eFlipState == MyMJCardFlipStateCpp::Invalid) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eflipstate!"));
        eFlipState = MyMJCardFlipStateCpp::Stand;
    }

    if (cardModelInfo.m_cBoxExtendFinal.IsZero()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cardBoxExtend is zero!"));
    }

    //target model:
    //        ----
    //        |  |
    //        |  |
    //        ----
    //
    // --------
    // |      |
    // --------

    //start the calc
    //our model assume one slot's flip state is by somekind same, that is, they all stand, or all laydown
    //in local system, x's negative axis is facing attender, just use a actor to show the nature case

    //API example:
    //FQuat rotater = cTransFormCenter.GetRotation();
    //localRotatorRelative2StackPoint.Quaternion();
    //localRotatorRelative2StackPoint = UKismetMathLibrary::ComposeRotators(localRotatorRelative2StackPoint, FRotator(0, 0, 0));

    FVector forwardV(1, 0, 0);
    FVector rightV(0, 1, 0);
    FVector upV(0, 0, 1);

    FVector perRowOffsetB2T(0); //bottom to top
    FVector perColOffsetL2R = rightV * cardModelInfo.m_cBoxExtendFinal.Y * 2; //left to right
    FVector perColOffsetX90DL2R = rightV * cardModelInfo.m_cBoxExtendFinal.Z * 2;
    FVector perStackOffsetB2T(0); //bottom to top

    FRotator localRotatorRelative2StackPoint(0);
    //FVector localRotateRelativePivot(0); //remove this since it is not important

    FVector localPointOfAttaching2StackPointRelativeLocation(0); //which will combine to stack point after rotate
    FVector localPointOfAttaching2StackPointRelativeLocationFixL2R(0);
    if (eFlipState == MyMJCardFlipStateCpp::Stand) {
        perRowOffsetB2T = forwardV * cardModelInfo.m_cBoxExtendFinal.X * 2;
        perStackOffsetB2T = upV * cardModelInfo.m_cBoxExtendFinal.Z * 2;

        localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
        localPointOfAttaching2StackPointRelativeLocation.Z -= cardModelInfo.m_cBoxExtendFinal.Z;

        localRotatorRelative2StackPoint.Yaw = 180;

    }
    else if (eFlipState == MyMJCardFlipStateCpp::Up) {
        perRowOffsetB2T = forwardV * cardModelInfo.m_cBoxExtendFinal.Z * 2;
        perStackOffsetB2T = upV * cardModelInfo.m_cBoxExtendFinal.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfo.m_cBoxExtendFinal.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = - cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfo.m_cBoxExtendFinal.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z -= cardModelInfo.m_cBoxExtendFinal.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z -= 1 * cardModelInfo.m_cBoxExtendFinal.X;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfo.m_cBoxExtendFinal.X;

            localRotatorRelative2StackPoint.Pitch = 90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else if (eFlipState == MyMJCardFlipStateCpp::Down) {
        perRowOffsetB2T = forwardV * cardModelInfo.m_cBoxExtendFinal.Z * 2;
        perStackOffsetB2T = upV * cardModelInfo.m_cBoxExtendFinal.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfo.m_cBoxExtendFinal.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfo.m_cBoxExtendFinal.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cardModelInfo.m_cBoxExtendFinal.Y + cardModelInfo.m_cBoxExtendFinal.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cardModelInfo.m_cCenterPointFinalRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z += cardModelInfo.m_cBoxExtendFinal.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z += 1 * cardModelInfo.m_cBoxExtendFinal.X;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfo.m_cBoxExtendFinal.X;

            localRotatorRelative2StackPoint.Pitch = -90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else {
        MY_VERIFY(false);
    }

    FVector rowPoint(0);
    if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Top) {
        rowPoint =  forwardV * cAreaBoxExtend.X - perRowOffsetB2T * idxRow;
    }
    else if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Bottom) {
        rowPoint = -forwardV * cAreaBoxExtend.X + perRowOffsetB2T * idxRow;
    }
    else {
        MY_VERIFY(false);
    }

    FVector colPoint(0);
    FVector colOffsetL2R = perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + fColInRowExtraMarginAbs * iColInRowExtraMarginCount;
    if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Left) {
        colPoint = rowPoint - rightV * cAreaBoxExtend.Y + colOffsetL2R;
        localPointOfAttaching2StackPointRelativeLocation += localPointOfAttaching2StackPointRelativeLocationFixL2R;
    }
    else if (eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
        colPoint = rowPoint + rightV * cAreaBoxExtend.Y - colOffsetL2R;
        localPointOfAttaching2StackPointRelativeLocation -= localPointOfAttaching2StackPointRelativeLocationFixL2R;
    }
    else {
        MY_VERIFY(false);
    }

    FVector stackPoint = colPoint + perStackOffsetB2T * idxStackInCol;

    //OK we got local stack point, let's calc card's local transform
    //the interesting thing is that, we just need to cacl the align point after rotate
    FVector offset = localRotatorRelative2StackPoint.RotateVector(localPointOfAttaching2StackPointRelativeLocation);
    FTransform relative2VisualPointTransform;
    relative2VisualPointTransform.SetLocation(stackPoint - offset);
    relative2VisualPointTransform.SetRotation(localRotatorRelative2StackPoint.Quaternion());

    FTransform outTransform = relative2VisualPointTransform * cTransFormCenter;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("relative2VisualPointTransform %s, cTransFormCenter %s, outTransform %s."),
        *UKismetStringLibrary::Conv_TransformToString(relative2VisualPointTransform),
        *UKismetStringLibrary::Conv_TransformToString(cTransFormCenter),
        *UKismetStringLibrary::Conv_TransformToString(outTransform));

    outLocationWorld = outTransform.GetLocation();
    outRotatorWorld = outTransform.GetRotation().Rotator();

    return 0;
}
