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

UMyMJGameDeskDynamicResManagerCpp::UMyMJGameDeskDynamicResManagerCpp() : Super()
{
};

UMyMJGameDeskDynamicResManagerCpp::~UMyMJGameDeskDynamicResManagerCpp()
{

};

bool UMyMJGameDeskDynamicResManagerCpp::checkSettings() const
{
    const AMyMJGameCardBaseCpp* pCDO = getCardCDO();
    if (IsValid(pCDO)) {
        return true;
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCDO is not valid %p, possible no card class specified."), pCDO);
        return false;
    }
};


int32 UMyMJGameDeskDynamicResManagerCpp::prepareCardActor(int32 count2reach)
{
    MY_VERIFY(count2reach >= 0);

    const AMyMJGameCardBaseCpp* pCDO = getCardCDO();
    if (!IsValid(pCDO)) {
        return -1;
    }

    int32 l = m_aCards.Num();
    for (int32 i = (l - 1); i >= count2reach; i--) {
        AMyMJGameCardBaseCpp* pPoped = m_aCards.Pop();
        pPoped->K2_DestroyActor();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor *parent = Cast<AActor>(GetOuter());
    MY_VERIFY(IsValid(parent));
    UWorld *w = parent->GetWorld();
    MY_VERIFY(IsValid(w));
    
    l = m_aCards.Num();
    for (int32 i = l; i < count2reach; i++) {
        AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(pCDO->StaticClass(), SpawnParams);
        MY_VERIFY(IsValid(pNewCardActor));
        pNewCardActor->SetActorHiddenInGame(true);
        MY_VERIFY(m_aCards.Emplace(pNewCardActor) == i);
    }

    return 0;
    //GetWorld()->SpawnActor<AProjectile>(Location, Rotation, SpawnInfo);
}

const AMyMJGameCardBaseCpp* UMyMJGameDeskDynamicResManagerCpp::getCardCDO() const
{
    if (IsValid(m_cCfgCardClass)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_cCardClass is invalid: %p"), m_cCfgCardClass.Get());
        return NULL;
    }

    AMyMJGameCardBaseCpp *pCDO = m_cCfgCardClass->GetDefaultObject<AMyMJGameCardBaseCpp>();
    if (!IsValid(pCDO)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card pCDO is invalid: %p"), pCDO);
        return NULL;
    }

    return pCDO;
}

AMyMJGameDeskSuiteCpp::AMyMJGameDeskSuiteCpp() : Super()
{
    m_pResManager = CreateDefaultSubobject<UMyMJGameDeskDynamicResManagerCpp>(TEXT("res manager"));
    m_pCoreWithVisual = CreateDefaultSubobject<UMyMJGameCoreWithVisualCpp>(TEXT("core with visual"));

    //m_pDataHistoryBuffer = CreateDefaultSubobject<UMyMJDataSequencePerRoleCpp>(TEXT("buffer"));
    //m_pTestObj = CreateDefaultSubobject<UMyTestObject>(TEXT("test obj"));
    //if (!IsValid(m_pRootScene)) {
        //USceneComponent* pRootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
        //MY_VERIFY(IsValid(pRootSceneComponent));

        //RootComponent = pRootSceneComponent;
        //m_pRootScene = pRootSceneComponent;
    //}

    m_pDeskAreaActor = NULL;
    //m_pMJCore = NULL;

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("0 this %p, m_pCoreWithVisual %p, m_pDataHistoryBuffer %p, m_pTestObj %p."), this, m_pCoreWithVisual, m_pDataHistoryBuffer, m_pTestObj);
};

AMyMJGameDeskSuiteCpp::~AMyMJGameDeskSuiteCpp()
{
}

void AMyMJGameDeskSuiteCpp::PostInitializeComponents()
{
    Super::PostInitializeComponents();
};

void AMyMJGameDeskSuiteCpp::BeginPlay()
{
    Super::BeginPlay();
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("1 this %p, m_pCoreWithVisual %p, m_pDataHistoryBuffer %p, m_pTestObj %p."), this, m_pCoreWithVisual, m_pDataHistoryBuffer, m_pTestObj);
    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("0 m_pResManager %p, m_pCoreWithVisual %p, m_pTestObj %p."), m_pResManager, m_pCoreWithVisual, m_pTestObj);
};

/*
void AMyMJGameDeskSuiteCpp::helperCalcCardTransform(const FMyMJGameCardVisualInfoCpp& cCardVisualInfo, FTransform &outTransform)
{

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
        m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlotInternal(idxAttender, eSlot, errCode, cVisualPointCenter);
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
        int32 iXRotate90D = cCardVisualInfo.m_iRotateX90D;
        int32 iXRotate90DBeforeCount = cCardVisualInfo.m_iRotateX90DBeforeCount;
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

};
*/

bool AMyMJGameDeskSuiteCpp::checkSettings() const
{
    MY_VERIFY(IsValid(m_pResManager));
    if (false == m_pResManager->checkSettings()) {
        return false;
    }

    MY_VERIFY(IsValid(m_pCoreWithVisual));
    if (false == m_pCoreWithVisual->checkSettings()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pCoreWithVisual have incorrect settings."));
        return false;
    }

    if (!IsValid(m_pDeskAreaActor)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pDeskAreaActor %p is not valid."), m_pDeskAreaActor);
        return false;
    }




    return true;
}

int32 AMyMJGameDeskSuiteCpp::helperGetColCountPerRowForDefaultAligment(int32 idxAtttender, MyMJCardSlotTypeCpp eSlot, int32& outCount) const
{
    outCount = 1;
    FMyMJGameDeskVisualPointCfgCpp cVisualPointCfg;
    if (0 != m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlot(idxAtttender, eSlot, cVisualPointCfg)) {
        return -1;
    };
    FMyMJGameCardActorModelInfoCpp cCardModelInfo;
    if (0 != m_pResManager->getCardModelInfoUnscaled(cCardModelInfo)) {
        return -2;
    }

    if (cCardModelInfo.m_cBoxExtend.Y < 1) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fixing cCardModelInfo.m_cBoxExtend.Y since it is %f before."), cCardModelInfo.m_cBoxExtend.Y);
        cCardModelInfo.m_cBoxExtend.Y = 1;
    }

    int32 retCount = FMath::CeilToInt(cVisualPointCfg.m_cAreaBoxExtendFinal.Y / cCardModelInfo.m_cBoxExtend.Y);
    if (retCount <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fixing retCount it is %d before."), retCount);
        retCount = 1;
    }

    outCount = retCount;

    return 0;
}


void AMyMJGameDeskSuiteCpp::helperTryUpdateCardVisualInfoPack()
{
    if (!checkSettings()) {
        return;
    }

    FMyMJDataAtOneMomentCpp& cDataNow = m_pCoreWithVisual->getDataNowRef();
    FMyMJDataAccessorCpp& cAccessor = cDataNow.getAccessorRef();
    const TArray<int32>& aHelperAttenderSlotDirtyMasks = cAccessor.getHelperAttenderSlotDirtyMasksConst(true);
    if (aHelperAttenderSlotDirtyMasks[0] == 0 && aHelperAttenderSlotDirtyMasks[1] == 0 && aHelperAttenderSlotDirtyMasks[2] == 0 && aHelperAttenderSlotDirtyMasks[3] == 0) {
        return;
    }

    FMyMJDataStructWithTimeStampBaseCpp& cBase = cDataNow.getBaseRef();

    const FMyMJCardInfoPackCpp& cCardInfoPack = cBase.getCoreDataRefConst().m_cCardInfoPack;
    const FMyMJCardValuePackCpp& ccardValuePack = cBase.getRoleDataPrivateRefConst((uint8)cDataNow.getRole()).m_cCardValuePack;

    FMyMJCardVisualInfoPackCpp& cCardVisualInfoPack = m_cCardVisualInfoPack;

    if (cCardInfoPack.getLength() != ccardValuePack.getLength()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pack length not equal: info pack %d, value pack %d, accces role type %d."), cCardInfoPack.getLength(), ccardValuePack.getLength(), (uint8)cDataNow.getRole());
        MY_VERIFY(false);
    }

    cCardVisualInfoPack.resize(cCardInfoPack.getLength());

    for (int32 idxAtttender = 0; idxAtttender < 4; idxAtttender++) {
        if (aHelperAttenderSlotDirtyMasks[idxAtttender] == 0) {
            continue;
        }

        const FMyMJRoleDataAttenderPublicCpp& attenderPublic = cBase.getRoleDataAttenderPublicRefConst(idxAtttender);

        //todo: there can be short cut that some case we don't need update
        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::Untaken) == true) {

            const TArray<FMyIdCollectionCpp>& aUntakenCardStacks = cBase.getCoreDataRefConst().m_aUntakenCardStacks;
            const FMyMJGameUntakenSlotSubSegmentInfoCpp& cSubSegmengInfo = attenderPublic.m_cUntakenSlotSubSegmentInfo;

            int32 idxUntakenStackEnd = cSubSegmengInfo.m_iIdxStart + cSubSegmengInfo.m_iLength;

            MY_VERIFY(cSubSegmengInfo.m_iIdxStart >= 0 && cSubSegmengInfo.m_iIdxStart < aUntakenCardStacks.Num());
            MY_VERIFY(idxUntakenStackEnd >= 0 && idxUntakenStackEnd <= aUntakenCardStacks.Num());
            for (int32 idxUntakenStack = cSubSegmengInfo.m_iIdxStart; idxUntakenStack < idxUntakenStackEnd; idxUntakenStack++) {
                const FMyIdCollectionCpp& stack = aUntakenCardStacks[idxUntakenStack];
                for (int32 i = 0; i < stack.m_aIds.Num(); i++) {
                    int32 cardId = stack.m_aIds[i];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_bVisible = true;
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::Untaken);

                    pCardVisualInfo->m_iIdxRow = 0;
                    pCardVisualInfo->m_iIdxColInRow = cCardInfo.m_cPosi.m_iIdxInSlot0;
                    pCardVisualInfo->m_iIdxStackInCol = cCardInfo.m_cPosi.m_iIdxInSlot1;

                    pCardVisualInfo->m_iCardValue = cardValue;
                }
            }

        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::JustTaken) == true) {

            const TArray<int32>& aIdHandCards = attenderPublic.m_aIdHandCards;
            const TArray<int32>& aIdJustTakenCards = attenderPublic.m_aIdJustTakenCards;
            int32 l0 = aIdHandCards.Num();
            int32 l1 = aIdJustTakenCards.Num();

            for (int32 i = 0; i < l1; i++) {
                int32 cardId = aIdJustTakenCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::JustTaken);

                pCardVisualInfo->m_iIdxRow = 0;
                pCardVisualInfo->m_iIdxColInRow = l0 + i;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iColInRowExtraMarginCount = 1;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::InHand) == true) {

            const TArray<int32>& aIdHandCards = attenderPublic.m_aIdHandCards;
            int32 l0 = aIdHandCards.Num();

            FMyMJValueIdMapCpp m_cSortCards;
            m_cSortCards.changeKeepOrder(true, false);
            for (int32 i = 0; i < l0; i++) {
                int32 cardId = aIdHandCards[i];
                int32 cardValue = ccardValuePack.getByIdx(cardId);
                m_cSortCards.insert(cardId, cardValue);
            }
            TArray<FMyIdValuePair> aPairs;
            m_cSortCards.collectAllWithValue(aPairs);
            MY_VERIFY(aPairs.Num() == l0);

            for (int32 i = 0; i < l0; i++) {
                int32 cardId = aPairs[i].m_iId;
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = aPairs[i].m_iValue;

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::InHand);

                pCardVisualInfo->m_iIdxRow = 0;
                pCardVisualInfo->m_iIdxColInRow = i;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::GivenOut) == true) {

            int32 iColPerRow;
            if (0 != helperGetColCountPerRowForDefaultAligment(idxAtttender, MyMJCardSlotTypeCpp::GivenOut, iColPerRow)) {
                continue;
            }

            const TArray<int32>& aIdGivenOutCards = attenderPublic.m_aIdGivenOutCards;
            int32 l = aIdGivenOutCards.Num();

            for (int32 i = 0; i < l; i++) {
                int32 cardId = aIdGivenOutCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::GivenOut);

                pCardVisualInfo->m_iIdxRow = i / iColPerRow;
                pCardVisualInfo->m_iIdxColInRow = i % iColPerRow;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::Weaved) == true) {

            FMyMJGameDeskVisualPointCfgCpp cVisualPointCfg;
            if (0 != m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlot(idxAtttender, MyMJCardSlotTypeCpp::Weaved, cVisualPointCfg)) {
                continue;
            };

            const TArray<FMyMJWeaveCpp>& aWeaves = attenderPublic.m_aShowedOutWeaves;
            int32 l = aWeaves.Num();

            int32 cardArrangedCount = 0;
            int32 cardRotatedX90DCount = 0;
            int32 weaveArrangedCount = 0;
            for (int32 idxWeave = 0; idxWeave < l; idxWeave++) {
                const FMyMJWeaveCpp& cWeave = aWeaves[idxWeave];
                const TArray<int32>& aIds = cWeave.getIdsRefConst();
                int32 l0 = aIds.Num();

                int32 triggerCardX90D = 0;
                if (cWeave.getIdxAttenderTriggerCardSrc() >= 0) {
                    if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAtttender + 1) % 4)) {
                        //aIds.
                        triggerCardX90D = 1;
                    }
                    else if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAtttender + 2) % 4)) {
                        triggerCardX90D = 1;
                    }
                    else if (cWeave.getIdxAttenderTriggerCardSrc() == ((idxAtttender + 3) % 4)) {
                        triggerCardX90D = -1;
                    }
                    else {
                    }
                }

                int32 workingIdx = 0;
                if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Left) {
                }
                else if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
                    workingIdx = l0 - 1;
                }
                else {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("unexpected visualPointCfg: idxAtttender %d, slot weave, m_eColInRowAlignment %d."), idxAtttender, (uint8)cVisualPointCfg.m_eColInRowAlignment);
                    continue;
                }

                while (1)
                {
                    int32 cardId = aIds[workingIdx];
                    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                    int32 cardValue = ccardValuePack.getByIdx(cardId);

                    FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                    pCardVisualInfo->reset();
                    pCardVisualInfo->m_bVisible = true;
                    pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                    pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                    pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                    if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                    }
                    MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::Weaved);

                    pCardVisualInfo->m_iIdxRow = 0;
                    pCardVisualInfo->m_iIdxColInRow = cardArrangedCount;
                    pCardVisualInfo->m_iIdxStackInCol = 0;

                    pCardVisualInfo->m_iRotateX90DBeforeCount = cardRotatedX90DCount;
                    pCardVisualInfo->m_iColInRowExtraMarginCount = weaveArrangedCount;
                    if (triggerCardX90D != 0 && cardId == cWeave.getIdTriggerCard()) {
                        pCardVisualInfo->m_iRotateX90D = triggerCardX90D;
                        cardRotatedX90DCount++;
                    }

                    pCardVisualInfo->m_iCardValue = cardValue;
                    cardArrangedCount++;

                    if (cVisualPointCfg.m_eColInRowAlignment == MyMJGameHorizontalAlignmentCpp::Right) {
                        workingIdx--;
                        if (workingIdx < 0) {
                            break;
                        }
                    }
                    else {
                        workingIdx++;
                        if (workingIdx >= l0) {
                            break;
                        }
                    }
                }
                weaveArrangedCount++;

            }

        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::WinSymbol) == true) {

            int32 iColPerRow;
            if (0 != helperGetColCountPerRowForDefaultAligment(idxAtttender, MyMJCardSlotTypeCpp::WinSymbol, iColPerRow)) {
                continue;
            }

            const TArray<int32>& aIdWinSymbolCards = attenderPublic.m_aIdWinSymbolCards;
            int32 l = aIdWinSymbolCards.Num();

            for (int32 i = 0; i < l; i++) {
                int32 cardId = aIdWinSymbolCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::WinSymbol);

                pCardVisualInfo->m_iIdxRow = i / iColPerRow;
                pCardVisualInfo->m_iIdxColInRow = i % iColPerRow;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }

        if (GetMyHelperAttenderSlotDirtyMasks(aHelperAttenderSlotDirtyMasks, idxAtttender, MyMJCardSlotTypeCpp::ShownOnDesktop) == true) {

            const TArray<int32>& aIdShownOnDesktopCards = attenderPublic.m_aIdShownOnDesktopCards;
            int32 l = aIdShownOnDesktopCards.Num();

            for (int32 i = 0; i < l; i++) {
                int32 cardId = aIdShownOnDesktopCards[i];
                const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(cardId);
                int32 cardValue = ccardValuePack.getByIdx(cardId);

                FMyMJGameCardVisualInfoCpp* pCardVisualInfo = cCardVisualInfoPack.getByIdx(cardId, true);
                pCardVisualInfo->reset();
                pCardVisualInfo->m_bVisible = true;
                pCardVisualInfo->m_eFlipState = cCardInfo.m_eFlipState;
                pCardVisualInfo->m_iIdxAttender = cCardInfo.m_cPosi.m_iIdxAttender;
                pCardVisualInfo->m_eSlot = cCardInfo.m_cPosi.m_eSlot;
                if (pCardVisualInfo->m_iIdxAttender != idxAtttender) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("card [%d: %d] have inconsistent posi: idxAtttender %d, .m_cPosi.m_iIdxAttender."), cardId, cardValue, idxAtttender, cCardInfo.m_cPosi.m_iIdxAttender);
                }
                MY_VERIFY(pCardVisualInfo->m_eSlot == MyMJCardSlotTypeCpp::ShownOnDesktop);

                pCardVisualInfo->m_iIdxRow = 0;
                pCardVisualInfo->m_iIdxColInRow = i;
                pCardVisualInfo->m_iIdxStackInCol = 0;

                pCardVisualInfo->m_iCardValue = cardValue;
            }
        }
    }

}

int32 AMyMJGameDeskSuiteCpp::helperCalcCardTransformFromvisualPointCfg(const FMyMJGameCardActorModelInfoCpp& cardModelInfoFinal, const FMyMJGameCardVisualInfoCpp& cardVisualInfoFinal, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FTransform& outTransform)
{

    const FMyMJGameDeskVisualPointCfgCpp& cVisualPointCenter = visualPointCfg;
    const FMyMJGameCardVisualInfoCpp& cCardVisualInfo = cardVisualInfoFinal;

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
    int32 iXRotate90D = cCardVisualInfo.m_iRotateX90D;
    int32 iXRotate90DBeforeCount = cCardVisualInfo.m_iRotateX90DBeforeCount;
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

    if (cardModelInfoFinal.m_cBoxExtend.IsZero()) {
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
    FVector perColOffsetL2R = rightV * cardModelInfoFinal.m_cBoxExtend.Y * 2; //left to right
    FVector perColOffsetX90DL2R = rightV * cardModelInfoFinal.m_cBoxExtend.Z * 2;
    FVector perStackOffsetB2T(0); //bottom to top

    FRotator localRotatorRelative2StackPoint(0);
    //FVector localRotateRelativePivot(0); //remove this since it is not important

    FVector localPointOfAttaching2StackPointRelativeLocation(0); //which will combine to stack point after rotate
    FVector localPointOfAttaching2StackPointRelativeLocationFixL2R(0);
    if (eFlipState == MyMJCardFlipStateCpp::Stand) {
        perRowOffsetB2T = forwardV * cardModelInfoFinal.m_cBoxExtend.X * 2;
        perStackOffsetB2T = upV * cardModelInfoFinal.m_cBoxExtend.Z * 2;

        localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
        localPointOfAttaching2StackPointRelativeLocation.Z -= cardModelInfoFinal.m_cBoxExtend.Z;

        localRotatorRelative2StackPoint.Yaw = 180;

    }
    else if (eFlipState == MyMJCardFlipStateCpp::Up) {
        perRowOffsetB2T = forwardV * cardModelInfoFinal.m_cBoxExtend.Z * 2;
        perStackOffsetB2T = upV * cardModelInfoFinal.m_cBoxExtend.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z -= cardModelInfoFinal.m_cBoxExtend.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z -= 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Pitch = 90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else if (eFlipState == MyMJCardFlipStateCpp::Down) {
        perRowOffsetB2T = forwardV * cardModelInfoFinal.m_cBoxExtend.Z * 2;
        perStackOffsetB2T = upV * cardModelInfoFinal.m_cBoxExtend.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cardModelInfoFinal.m_cBoxExtend.Y + cardModelInfoFinal.m_cBoxExtend.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cardModelInfoFinal.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z += cardModelInfoFinal.m_cBoxExtend.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z += 1 * cardModelInfoFinal.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cardModelInfoFinal.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Pitch = -90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else {
        MY_VERIFY(false);
    }

    FVector rowPoint(0);
    if (eRowAlignment == MyMJGameVerticalAlignmentCpp::Top) {
        rowPoint = forwardV * cAreaBoxExtend.X - perRowOffsetB2T * idxRow;
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

    outTransform = relative2VisualPointTransform * cTransFormCenter;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("relative2VisualPointTransform %s, cTransFormCenter %s, outTransform %s."),
        *UKismetStringLibrary::Conv_TransformToString(relative2VisualPointTransform),
        *UKismetStringLibrary::Conv_TransformToString(cTransFormCenter),
        *UKismetStringLibrary::Conv_TransformToString(outTransform));

    //outLocationWorld = outTransform.GetLocation();
    //outRotatorWorld = outTransform.GetRotation().Rotator();

    return 0;

}

void AMyMJGameDeskSuiteCpp::helperResolveTargetCardVisualState(int32 idxCard, FMyMJGameCardVisualStateCpp& outTargetCardVisualState)
{
    outTargetCardVisualState.reset();

    //if (!checkSettings()) {
        //return;
    //}

    MY_VERIFY(idxCard >= 0);

    const FMyMJDataAtOneMomentCpp& cDataNow = m_pCoreWithVisual->getDataNowRefConst();
    const FMyMJDataStructWithTimeStampBaseCpp& cBase = cDataNow.getBaseRefConst();
    const FMyMJCardInfoPackCpp& cCardInfoPack = cBase.getCoreDataRefConst().m_cCardInfoPack;

    if (idxCard >= cCardInfoPack.getLength()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("idxCard out of range. %d/%d."), idxCard, cCardInfoPack.getLength());
        return;
    }

    const FMyMJCardInfoCpp& cCardInfo = cCardInfoPack.getRefByIdxConst(idxCard);
    if (cCardInfo.m_cPosi.m_iIdxAttender < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("card position is not inited, m_iIdxAttender %d."), cCardInfo.m_cPosi.m_iIdxAttender);
        return;
    }

    FMyMJGameDeskVisualPointCfgCpp cVisualPoint;
    if (0 != m_pDeskAreaActor->getVisualPointCfgByIdxAttenderAndSlot(cCardInfo.m_cPosi.m_iIdxAttender, cCardInfo.m_cPosi.m_eSlot, cVisualPoint)) {
        return;
    }

    FMyMJGameCardVisualInfoCpp* pVisualInfo = m_cCardVisualInfoPack.getByIdx(idxCard, false);
    if (pVisualInfo == NULL) {
        return;
    }

    FMyMJGameCardActorModelInfoCpp cModelInfo;
    if (0 != m_pResManager->getCardModelInfoUnscaled(cModelInfo)) {
        return;
    }

    if (0 != helperCalcCardTransformFromvisualPointCfg(cModelInfo, *pVisualInfo, cVisualPoint, outTargetCardVisualState.m_cTransform)) {
        return;
    }

    outTargetCardVisualState.m_bVisible = pVisualInfo->m_bVisible;
    outTargetCardVisualState.m_iCardValue = pVisualInfo->m_iCardValue;
}