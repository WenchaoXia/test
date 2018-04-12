// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCardGameElems.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "engine/StaticMesh.h"
#include "engine/Texture.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Kismet/KismetMathLibrary.h"

#define MyCardGameAsset_Common_Value_Normal_PartialNamePrefix (TEXT("v%02d"))
#define MyCardGameAsset_Common_Value_Unknown_PartialNamePrefix (TEXT("vUnknown"))
//#define MyCardGameAsset_Common_PartialNamePrefix_Value_Miss (TEXT("vMiss"))

#define MyCardGameAsset_CardWidget_Image_MainTexture_PartialNameSuffix (TEXT("_mainTexture"))

#define MyCardGameAsset_CardActor_ModelRes_RelativePath (TEXT("Res"))

#define MyCardGameAsset_CardActor_StaticMesh_MID_Param_BaseColor (TEXT("InBaseColor"))
#define MyCardGameAsset_CardActor_StaticMesh_MID_BaseColorTexture_PartialNameSuffix (TEXT("_baseColor"))

#define MyCardGameAsset_CardActor_StaticMesh_Name (TEXT("cardBox"))
#define MyCardGameAsset_CardActor_StaticMesh_DefaultMI_Name (TEXT("cardBoxMat0_defaultInst"))


/*
void FMyArrangeCoordinateWorld3DCpp::helperResolveTransform(const FMyArrangePointCfgWorld3DCpp& cVisualPointCfg,
                                                                 const FMyModelInfoBoxWorld3DCpp& cModelInfo,
                                                                 const FMyArrangeCoordinateWorld3DCpp& cVisualInfo,
                                                                 FTransform& outTransform)
{

    const FTransform& cTransFormCenter = cVisualPointCfg.m_cCenterPointTransform;
    const FVector& cAreaBoxExtend = cVisualPointCfg.m_cAreaBoxExtendFinal;
    MyCardGameVerticalAlignmentCpp eRowAlignment = cVisualPointCfg.m_eRowAlignment;
    //int32 iRowMaxNum = cVisualPointCfg.m_iRowMaxNum;
    MyCardGameHorizontalAlignmentCpp eColInRowAlignment = cVisualPointCfg.m_eColInRowAlignment;
    //int32 iColInRowMaxNum = cVisualPointCfg.m_iColInRowMaxNum;
    float fColInRowExtraMarginAbs = cVisualPointCfg.m_fColInRowExtraMarginPercent * cModelInfo.m_cBoxExtend.Y * 2;
    if (fColInRowExtraMarginAbs < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("fColInRowExtraMarginAbs is negative: %f, forceing to default."), fColInRowExtraMarginAbs);
        fColInRowExtraMarginAbs = 0;
    }

    if (eRowAlignment == MyCardGameVerticalAlignmentCpp::Invalid || eRowAlignment == MyCardGameVerticalAlignmentCpp::Mid) { //we don't support mid allignment now
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("eRowAlignment row alignment %d, forceing to default!."), (uint8)eRowAlignment);
        eRowAlignment = MyCardGameVerticalAlignmentCpp::Bottom;
    }
    //if (iRowMaxNum <= 0) {
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iRowMaxNum %d, forceing to default!."), iRowMaxNum);
    //iRowMaxNum = 1;
    //}

    if (eColInRowAlignment == MyCardGameHorizontalAlignmentCpp::Mid) {
        //trick for middle alignment since col idx have been adjusted before
        eColInRowAlignment = MyCardGameHorizontalAlignmentCpp::Left;
    }

    if (eColInRowAlignment == MyCardGameHorizontalAlignmentCpp::Invalid) { //we don't support mid allignment now
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eColInRowAlignment alignment %d, forceing to default!."), (uint8)eColInRowAlignment);
        eColInRowAlignment = MyCardGameHorizontalAlignmentCpp::Left;
    }
    //if (iColInRowMaxNum <= 0) {
    //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid iColInRowMaxNum %d, forceing to default!."), iColInRowMaxNum);
    //iColInRowMaxNum = 1;
    //}

    int32 idxRow = cVisualInfo.m_iIdxRow;
    int32 idxColInRow = cVisualInfo.m_cCol.m_iIdxElem;
    if (cVisualInfo.m_iHelperIdxColInRowReal >= 0) {
        idxColInRow = cVisualInfo.m_iHelperIdxColInRowReal;
    }
    int32 idxStackInCol = cVisualInfo.m_cStack.m_iIdxElem;
    MyBoxLikeFlipStateCpp eFlipState = cVisualInfo.m_eFlipState;
    int32 iXRotate90D = cVisualInfo.m_iRotateX90D;
    int32 iXRotate90DBeforeCount = cVisualInfo.m_iRotateX90DBeforeCount;
    int32 iColInRowExtraMarginCount = cVisualInfo.m_iColInRowExtraMarginCount;

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


    if (eFlipState == MyBoxLikeFlipStateCpp::Invalid) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("invalid eflipstate!"));
        eFlipState = MyBoxLikeFlipStateCpp::Stand;
    }

    if (cModelInfo.m_cBoxExtend.IsZero()) {
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
    FVector perColOffsetL2R = rightV * cModelInfo.m_cBoxExtend.Y * 2; //left to right
    FVector perColOffsetX90DL2R = rightV * cModelInfo.m_cBoxExtend.Z * 2;
    FVector perStackOffsetB2T(0); //bottom to top

    FRotator localRotatorRelative2StackPoint(0);
    //FVector localRotateRelativePivot(0); //remove this since it is not important

    FVector localPointOfAttaching2StackPointRelativeLocation(0); //which will combine to stack point after rotate
    FVector localPointOfAttaching2StackPointRelativeLocationFixL2R(0);
    if (eFlipState == MyBoxLikeFlipStateCpp::Stand) {
        perRowOffsetB2T = forwardV * cModelInfo.m_cBoxExtend.X * 2;
        perStackOffsetB2T = upV * cModelInfo.m_cBoxExtend.Z * 2;

        localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
        localPointOfAttaching2StackPointRelativeLocation.Z -= cModelInfo.m_cBoxExtend.Z;

        localRotatorRelative2StackPoint.Yaw = 180;

    }
    else if (eFlipState == MyBoxLikeFlipStateCpp::Up) {
        perRowOffsetB2T = forwardV * cModelInfo.m_cBoxExtend.Z * 2;
        perStackOffsetB2T = upV * cModelInfo.m_cBoxExtend.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cModelInfo.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cModelInfo.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = 90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z -= cModelInfo.m_cBoxExtend.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z -= 1 * cModelInfo.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.X -= 1 * cModelInfo.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Pitch = 90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else if (eFlipState == MyBoxLikeFlipStateCpp::Down) {
        perRowOffsetB2T = forwardV * cModelInfo.m_cBoxExtend.Z * 2;
        perStackOffsetB2T = upV * cModelInfo.m_cBoxExtend.X * 2;

        //for simple, if 90D, it always align to bottom line
        if (iXRotate90D > 0) {
            localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cModelInfo.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y += (cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Roll = 90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else if (iXRotate90D < 0) {
            localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cModelInfo.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.Y -= (cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X);
            localPointOfAttaching2StackPointRelativeLocationFixL2R.Z = -(-cModelInfo.m_cBoxExtend.Y + cModelInfo.m_cBoxExtend.X);

            localRotatorRelative2StackPoint.Roll = -90;
            localRotatorRelative2StackPoint.Pitch = -90;
        }
        else {

            localPointOfAttaching2StackPointRelativeLocation = cModelInfo.m_cCenterPointRelativeLocation;
            localPointOfAttaching2StackPointRelativeLocation.Z += cModelInfo.m_cBoxExtend.Z;
            localPointOfAttaching2StackPointRelativeLocation.Z += 1 * cModelInfo.m_cBoxExtend.X;
            localPointOfAttaching2StackPointRelativeLocation.X += 1 * cModelInfo.m_cBoxExtend.X;

            localRotatorRelative2StackPoint.Pitch = -90;
            localRotatorRelative2StackPoint.Yaw = 180;

        }
    }
    else {
        MY_VERIFY(false);
    }

    FVector rowPoint(0);
    if (eRowAlignment == MyCardGameVerticalAlignmentCpp::Top) {
        rowPoint = forwardV * cAreaBoxExtend.X - perRowOffsetB2T * idxRow;
    }
    else if (eRowAlignment == MyCardGameVerticalAlignmentCpp::Bottom) {
        rowPoint = -forwardV * cAreaBoxExtend.X + perRowOffsetB2T * idxRow;
    }
    else {
        MY_VERIFY(false);
    }

    FVector colPoint(0);
    FVector colOffsetL2R = perColOffsetL2R * colNormalCountForLocCalcBefore + perColOffsetX90DL2R * col90DCountForLocCalcBefore + rightV * (fColInRowExtraMarginAbs * iColInRowExtraMarginCount);
    if (eColInRowAlignment == MyCardGameHorizontalAlignmentCpp::Left) {
        colPoint = rowPoint - rightV * cAreaBoxExtend.Y + colOffsetL2R;
        localPointOfAttaching2StackPointRelativeLocation += localPointOfAttaching2StackPointRelativeLocationFixL2R;
    }
    else if (eColInRowAlignment == MyCardGameHorizontalAlignmentCpp::Right) {
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

    //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("relative2VisualPointTransform %s, cTransFormCenter %s, outTransform %s."),
    //*UKismetStringLibrary::Conv_TransformToString(relative2VisualPointTransform),
    //*UKismetStringLibrary::Conv_TransformToString(cTransFormCenter),
    //*UKismetStringLibrary::Conv_TransformToString(outTransform));

    //outLocationWorld = outTransform.GetLocation();
    //outRotatorWorld = outTransform.GetRotation().Rotator();

    return;
}
*/



AMyCardGameCardActorBaseCpp::AMyCardGameCardActorBaseCpp() : Super()
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = uc->GetDefaultObject();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyCardGameCardActorBaseCpp, %s, this: %p, uc %s, cdo %p."), *m_cResPath.Path, this, *uc->GetFullName(), CDO);

    m_iValueShowing = MyCardGameValueUnknown;
    m_iValueUpdatedBefore = MyCardGameValueUnknown - 1;

    m_cResPath.Path.Reset();
    m_pResMesh = NULL;
    m_pResMI = NULL;

    m_iMyId = MyIDUnknown;
}

AMyCardGameCardActorBaseCpp::~AMyCardGameCardActorBaseCpp()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~AMyCardGameCardActorBaseCpp, this: %p."), this);
};

/* it has priority over default constructor, and we don't need it yet */

/*
AMyCardGameCardActorBaseCpp::AMyCardGameCardActorBaseCpp(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
// Initialize CDO properties here.
UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("AMyCardGameCardActorBaseCpp construct 2, %s"), *m_sModelAssetPath);
}
*/

void AMyCardGameCardActorBaseCpp::OnConstruction(const FTransform& Transform)
{
    //UClass* uc = this->GetClass();
    //UObject* CDO = uc->GetDefaultObject();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnConstruction, this %p, cdo %p."), this, CDO);
    Super::OnConstruction(Transform);

    updateVisual(true);
}

void AMyCardGameCardActorBaseCpp::PostInitializeComponents()
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostInitializeComponents, this %p, %s, %p, %p, compo: %p."), this, *m_cResPath.Path, m_pResMesh, m_pResMI, m_pMainStaticMesh);
    Super::PostInitializeComponents();

    updateVisual(true);
}

#if WITH_EDITOR

void AMyCardGameCardActorBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyCardGameCardActorBaseCpp, m_iValueShowing))
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 1, this %p."), this);

        //this may invlove CDO, so update all, since CDO may have not executed onContruct() nor PostInitializeComponents() before

        updateVisual(false);
        //updateWithValue(m_iValueShowing);
    }
    else {
        PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

        if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyCardGameCardActorBaseCpp, m_cResPath))
        {
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 2, this %p, %s"), this, *m_cResPath.Path);
            if (MyErrorCodeCommonPartCpp::NoError != checkAndLoadCardBasicResources(m_cResPath.Path)) {
                UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Retrying for default settings."));
                m_cResPath.Path = UMyCommonUtilsLibrary::getClassAssetPath(this->GetClass()) + TEXT("/") + MyCardGameAsset_CardActor_ModelRes_RelativePath;
                checkAndLoadCardBasicResources(m_cResPath.Path);
            }
            updateVisual(true);
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::updateSettings()
{
    MyErrorCodeCommonPartCpp ret = Super::updateSettings();
    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        return ret;
    }

    FVector boxSize = m_pMainBox->GetScaledBoxExtent();
    FVector alignPoint = FVector::ZeroVector;

    alignPoint.X = -boxSize.X;
    alignPoint.Z = -boxSize.Z;

    m_pMainBox->SetRelativeLocation(-alignPoint);

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::checkAndLoadCardBasicResources(const FString &inPath)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
    if (inPath.IsEmpty()) {
        m_pResMesh = nullptr;
        m_pResMI = nullptr;
        m_cResPath.Path.Reset();

        return MyErrorCodeCommonPartCpp::NoError;
    }


    const FString &modelAssetPath = inPath;

    FString meshFullPathName = modelAssetPath + TEXT("/") + MyCardGameAsset_CardActor_StaticMesh_Name;
    FString matDefaultInstFullPathName = modelAssetPath + TEXT("/") + MyCardGameAsset_CardActor_StaticMesh_DefaultMI_Name;

    UStaticMesh *pMeshAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UStaticMesh>(NULL, meshFullPathName);
    if (!IsValid(pMeshAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load mesh asset from %s."), *meshFullPathName);
        m_pResMesh = nullptr;
        ret = MyErrorCodeCommonPartCpp::AssetLoadFail;
    }
    else {
        m_pResMesh = pMeshAsset;
    }

    UMaterialInstance *pMatInstAsset = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UMaterialInstance>(NULL, matDefaultInstFullPathName);
    if (!IsValid(pMatInstAsset)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to load material default instance asset from %s."), *matDefaultInstFullPathName);
        m_pResMI = nullptr;
        ret = MyErrorCodeCommonPartCpp::AssetLoadFail;
    }
    else {
        m_pResMI = pMatInstAsset;
    }

    m_cResPath.Path = modelAssetPath;

    return ret;
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::updateVisual(bool bForce)
{
    //if (m_cResPath.Path.IsEmpty()) {
    //return -1;
    //}
    MyErrorCodeCommonPartCpp ret, rett;

    ret = updateWithCardBasicResources();

    rett = updateWithValue(bForce);
    MyErrorCodePartJoin(ret, rett);

    return ret;
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::updateWithCardBasicResources()
{
    if (!IsValid(m_pMainBox)) {
        //UClass* uc = this->GetClass();
        //UObject* CDO = uc->GetDefaultObject();

        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pMainBox invalid 0x%p, this 0x%p, cdo 0x%p."), m_pMainBox, this, CDO);
        MY_VERIFY(false);
    }

    //MY_VERIFY(IsValid(m_pMainBox));
    MY_VERIFY(IsValid(m_pMainStaticMesh));

    UStaticMesh* pMeshNow = m_pMainStaticMesh->GetStaticMesh();
    if (pMeshNow != m_pResMesh) {
        //only update when mesh changes, give BP a chance to adjust them manually
        m_pMainStaticMesh->SetStaticMesh(m_pResMesh);
        updateSettings();
    }


    UMaterialInstanceDynamic* pMIDNow = Cast<UMaterialInstanceDynamic>(m_pMainStaticMesh->GetMaterial(0));
    if (IsValid(m_pResMI)) {
        if (IsValid(pMIDNow) && pMIDNow->Parent == m_pResMI) {
            //equal
        }
        else {
            UMaterialInstanceDynamic* pMID = UMaterialInstanceDynamic::Create(m_pResMI, m_pMainStaticMesh);
            MY_VERIFY(IsValid(pMID));
            m_pMainStaticMesh->SetMaterial(0, pMID);
        }
    }
    else {
        //simple, target is to clear
        if (!IsValid(pMIDNow)) {
            //equal
        }
        else {
            m_pMainStaticMesh->SetMaterial(0, nullptr);
        }
    }

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::updateWithValue(bool bForce)
{
    if (m_iValueUpdatedBefore == m_iValueShowing && bForce == false) {
        return MyErrorCodeCommonPartCpp::NoError;
    }
    m_iValueUpdatedBefore = m_iValueShowing;

    UTexture* pTargetBaseColorTexture = NULL;

    FString vPrefix;
    if (!m_cResPath.Path.IsEmpty()) {

        if (m_iValueShowing > MyCardGameValueUnknown) {
            vPrefix = FString::Printf(MyCardGameAsset_Common_Value_Normal_PartialNamePrefix, m_iValueShowing);
        }
        else if (m_iValueShowing == MyCardGameValueUnknown) {
            vPrefix = MyCardGameAsset_Common_Value_Unknown_PartialNamePrefix;
        }

    }

    if (!vPrefix.IsEmpty()) {
        helperTryLoadCardRes(m_cResPath.Path, vPrefix, &pTargetBaseColorTexture);
    }

    return updateCardStaticMeshMIDParams(pTargetBaseColorTexture);
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::updateCardStaticMeshMIDParams(class UTexture* InBaseColor)
{
    MY_VERIFY(IsValid(m_pMainStaticMesh));

    UMaterialInterface* pMat = m_pMainStaticMesh->GetMaterial(0);
    if (IsValid(pMat)) {
        UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(pMat);
        if (!DynamicMaterial)
        {
            //our design need MID in any case
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("cast to dynamic material instance fail: this %p, %s, %s."), this, *pMat->GetClass()->GetFullName(), *pMat->GetFullName());
            if (InBaseColor == nullptr) {
                return MyErrorCodeCommonPartCpp::NoError;
            }
            else {
                return MyErrorCodeCommonPartCpp::ObjectCastFail;
            }
        }

        class UTexture* baseColorNow = NULL;
        if (DynamicMaterial->GetTextureParameterValue(MyCardGameAsset_CardActor_StaticMesh_MID_Param_BaseColor, baseColorNow)) {
            if (baseColorNow == InBaseColor) {
                //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("skip SetTextureParameterValue since target is same, this %p."), this);
                return MyErrorCodeCommonPartCpp::NoError;
            }
        }

        //if (InBaseColor == nullptr) {
        //test
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("doing test."));
        //m_pMainStaticMesh->SetMaterial(0, nullptr);
        //return 0;
        //}

        DynamicMaterial->SetTextureParameterValue(MyCardGameAsset_CardActor_StaticMesh_MID_Param_BaseColor, InBaseColor);
        return MyErrorCodeCommonPartCpp::NoError;
    }
    else {
        if (InBaseColor == nullptr) {
            return MyErrorCodeCommonPartCpp::NoError;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("try to set MID param but material is NULL."));
            return MyErrorCodeCommonPartCpp::ObjectNull;
        }
    }

    return MyErrorCodeCommonPartCpp::NoError;
};

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::updateValueShowing(int32 newValueShowing, int32 animationTimeMs)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setValueShowing %d."), newValue);

    //always set immediately

    if (m_iValueShowing == newValueShowing) {
        return MyErrorCodeCommonPartCpp::NoError;
    }
    m_iValueShowing = newValueShowing;

    return updateWithValue(false);
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::getValueShowing(int32& valueShowing) const
{
    valueShowing = m_iValueShowing;

    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::setResourcePath(const FDirectoryPath& newResPath)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("setResourcePath %s."), *newResPath.Path);

    if (m_cResPath.Path == newResPath.Path) {
        return MyErrorCodeCommonPartCpp::NoError;
    }

    FDirectoryPath oldPath = m_cResPath;
    m_cResPath = newResPath;

    MyErrorCodeCommonPartCpp ret;
    ret = checkAndLoadCardBasicResources(m_cResPath.Path);

    if (ret == MyErrorCodeCommonPartCpp::NoError)
    {
    }
    else {
        m_cResPath = oldPath;
    }

    MyErrorCodePartJoin(ret, updateVisual(true));

    return ret;
}

MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::getResourcePath(FDirectoryPath& resPath) const
{
    resPath = m_cResPath;
    return MyErrorCodeCommonPartCpp::NoError;
}


MyErrorCodeCommonPartCpp AMyCardGameCardActorBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

    if (ppOutBaseColorTexture) {
        FString baseColorFullPathName = modelAssetPath + TEXT("/") + valuePrefix + MyCardGameAsset_CardActor_StaticMesh_MID_BaseColorTexture_PartialNameSuffix;
        UTexture* pTBaseColor = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutBaseColorTexture = pTBaseColor;
        }
        else {
            *ppOutBaseColorTexture = NULL;
            ret = MyErrorCodeCommonPartCpp::AssetLoadFail;
        }
    }

    return ret;
};



AMyCardGameDiceActorBaseCpp::AMyCardGameDiceActorBaseCpp() : Super()
{
    m_iMyId = MyIDUnknown;
};

AMyCardGameDiceActorBaseCpp::~AMyCardGameDiceActorBaseCpp()
{

};

MyErrorCodeCommonPartCpp AMyCardGameDiceActorBaseCpp::updateSettings()
{
    MyErrorCodeCommonPartCpp ret = Super::updateSettings();
    if (ret != MyErrorCodeCommonPartCpp::NoError) {
        return ret;
    }

    //FVector boxSize = m_pMainBox->GetScaledBoxExtent();
    //FVector alignPoint = boxSize / 2;
    //m_pMainBox->SetRelativeLocation(-alignPoint);

    m_pMainBox->SetRelativeLocation(FVector::ZeroVector);

    return MyErrorCodeCommonPartCpp::NoError;
};


FTransform AMyCardGameDiceActorBaseCpp::helperCalcFinalTransform(const FMyCardGameDiceModelInfoCpp& diceModelInfo, const FMyArrangePointCfgWorld3DCpp& diceVisualPointCfg, int32 diceVisualStateKey, int32 idxOfDiceRandomArranged, int32 diceTotalNum, int32 value)
{
    MY_VERIFY(idxOfDiceRandomArranged >= 0);
    MY_VERIFY(idxOfDiceRandomArranged < diceTotalNum);

    if (value < 1 || value > 6) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("dice value is out of range: %d."), value);
        int32 key = diceVisualStateKey >= 0 ? diceVisualStateKey : -diceVisualStateKey;
        value = key % 6 + 1;
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("value overwrited as: %d."), value);
    }

    FRandomStream RS;
    RS.Initialize(diceVisualStateKey + idxOfDiceRandomArranged);

    float radiusMax = diceVisualPointCfg.m_cAreaBoxExtendFinal.Size2D();

    float radius = RS.FRandRange(0.1, 0.9) * radiusMax;
    float perDiceDegree = 360 / diceTotalNum;
    float perDiceDegreeHalfRandRange = perDiceDegree / 2 * 0.7;

    float degree = perDiceDegree * idxOfDiceRandomArranged + RS.FRandRange(-perDiceDegreeHalfRandRange, perDiceDegreeHalfRandRange);

    FVector localLoc = UKismetMathLibrary::RotateAngleAxis(FVector(radius, 0, 0), degree, FVector(0, 0, 1));
    FRotator localRot = diceModelInfo.getLocalRotatorForDiceValueRefConst(value);

    localRot.Yaw = RS.FRandRange(0, 359);
    localRot.Yaw = RS.FRandRange(0, 359);

    FTransform localT, finalT;
    localT.SetLocation(localLoc);
    localT.SetRotation(localRot.Quaternion());

    finalT = localT * diceVisualPointCfg.m_cCenterPointTransform;

    return finalT;
};



MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::updateValueShowing(int32 newValueShowing, int32 animationTimeMs)
{
    //Currently only support set at instance

    m_iValueShowing = newValueShowing;

    return updateWithValue(false);
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::getValueShowing(int32& valueShowing) const
{
    valueShowing = m_iValueShowing;
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::setResourcePath(const FDirectoryPath& newResPath)
{
    m_cResPath = newResPath;
    updateWithValue(true);
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::getResourcePath(FDirectoryPath& resPath) const
{
    resPath = m_cResPath;  
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::updateWithValue(bool bForce)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

    if (m_iValueUpdatedBefore == m_iValueShowing && bForce == false) {
        return MyErrorCodeCommonPartCpp::NoError;
    }

    m_iValueUpdatedBefore = m_iValueShowing;

    FString vPrefix;
    if (!m_cResPath.Path.IsEmpty()) {

        if (m_iValueShowing > MyCardGameValueUnknown) {
            vPrefix = FString::Printf(MyCardGameAsset_Common_Value_Normal_PartialNamePrefix, m_iValueShowing);
        }
        else if (m_iValueShowing == MyCardGameValueUnknown) {
            vPrefix = MyCardGameAsset_Common_Value_Unknown_PartialNamePrefix;
        }

    }

    m_pCardMainTexture = NULL;
    if (!vPrefix.IsEmpty()) {
        MyErrorCodeCommonPartCpp retT = helperTryLoadCardRes(m_cResPath.Path, vPrefix, &m_pCardMainTexture);
        MyErrorCodePartJoin(ret, retT);
    }

    UMyButton* pCB = getCenterButton(false);
    if (pCB) {
        //assume modify inplace is OK, and target texuture can be NULL here, on purpose or unexpected
        pCB->WidgetStyle.Normal.SetResourceObject(m_pCardMainTexture);
        pCB->WidgetStyle.Pressed.SetResourceObject(m_pCardMainTexture);
        pCB->WidgetStyle.Hovered.SetResourceObject(m_pCardMainTexture);
        pCB->SetStyle(pCB->WidgetStyle);
    }
    else {
        MyErrorCodePartJoin(ret, MyErrorCodeCommonPartCpp::UObjectNotExist);
    }

    return ret;
}


#if WITH_EDITOR

void UMyCardGameCardWidgetBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyCardGameCardWidgetBaseCpp, m_iValueShowing))
    {

        updateWithValue(false);
        //updateWithValue(m_iValueShowing);
    }
    else {
        PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

        if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyCardGameCardWidgetBaseCpp, m_cResPath))
        {
            updateWithValue(true);
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutCardValueMainTexture)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

    if (ppOutCardValueMainTexture) {
        FString baseColorFullPathName = modelAssetPath + TEXT("/") + valuePrefix + MyCardGameAsset_CardWidget_Image_MainTexture_PartialNameSuffix;
        UTexture* pTBaseColor = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutCardValueMainTexture = pTBaseColor;
        }
        else {
            *ppOutCardValueMainTexture = NULL;
            MyErrorCodePartJoin(ret, MyErrorCodeCommonPartCpp::AssetLoadFail);
        }
    }

    return ret;
};