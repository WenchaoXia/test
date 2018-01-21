// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCommonUtilsLibrary.h"

#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

int32 UMyCommonUtilsLibrary::getEngineNetMode(AActor *actor)
{
    UWorld *world = actor->GetWorld();
    if (world) {
        return GEngine->GetNetMode(world);
    }
    else {
        return -1;
    }
};


void UMyCommonUtilsLibrary::rotateOriginWithPivot(const FTransform& originCurrentWorldTransform, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultWorldTransform)
{
    //FRotator originCurrentWorldRotator(originCurrentWorldTransform.GetRotation());
    //FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    //FVector originZeroRotateWorldLocation = originCurrentWorldTransform.GetLocation() - (originCurrentWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);

    //originResultWorldTransform.SetLocation(originZeroRotateWorldLocation + (originTargetWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    //originResultWorldTransform.SetRotation(FQuat(originTargetWorldRotator));
    //originResultWorldTransform.SetScale3D(originCurrentWorldTransform.GetScale3D());

    FQuat originCurrentWorldQuat = originCurrentWorldTransform.GetRotation();
    FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    FVector testV = originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("testV: (%f, %f, %f)."), testV.X, testV.Y, testV.Z);
    //return;

    FVector originZeroRotateWorldLocation = originCurrentWorldTransform.GetLocation() - (originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originZeroRotateWorldLocation: (%f, %f, %f)."), originZeroRotateWorldLocation.X, originZeroRotateWorldLocation.Y, originZeroRotateWorldLocation.Z);
    //return;

    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originTargetWorldRotator: (%f, %f, %f)."),
        originTargetWorldRotator.Roll, originTargetWorldRotator.Pitch, originTargetWorldRotator.Yaw);
    //FQuat originTargetWorldQuat(originTargetWorldRotator);
    FQuat originTargetWorldQuat = originTargetWorldRotator.Quaternion();
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originTargetWorldQuat: (%f, %f, %f, %f)."),
        originTargetWorldQuat.X, originTargetWorldQuat.Y, originTargetWorldQuat.Z, originTargetWorldQuat.W)
        //return;

        FRotator originTargetWorldRotator2(originTargetWorldQuat);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("rotator change: (%f, %f, %f) -> (%f, %f, %f)."),
        originTargetWorldRotator.Roll, originTargetWorldRotator.Pitch, originTargetWorldRotator.Yaw,
        originTargetWorldRotator2.Roll, originTargetWorldRotator2.Pitch, originTargetWorldRotator2.Yaw);
    //return;

    FVector locResultRotated = originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("locResultRotated: (%f, %f, %f)."), locResultRotated.X, locResultRotated.Y, locResultRotated.Z);

    originResultWorldTransform.SetLocation(originZeroRotateWorldLocation + (originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    originResultWorldTransform.SetRotation(originTargetWorldQuat);
    originResultWorldTransform.SetScale3D(originCurrentWorldTransform.GetScale3D());
};

FString UMyCommonUtilsLibrary::getDebugStringFromEWorldType(EWorldType::Type t)
{
    if (t == EWorldType::None)
    {
        return TEXT("None");
    }
    else if (t == EWorldType::Game)
    {
        return TEXT("Game");
    }
    else if (t == EWorldType::Editor)
    {
        return TEXT("Editor");
    }
    else if (t == EWorldType::PIE)
    {
        return TEXT("PIE");
    }
    else if (t == EWorldType::EditorPreview)
    {
        return TEXT("EditorPreview");
    }
    else if (t == EWorldType::GamePreview)
    {
        return TEXT("GamePreview");
    }
    else if (t == EWorldType::Inactive)
    {
        return TEXT("Inactive");
    }
    else
    {
        return TEXT("Invalid");
    }

};

FString UMyCommonUtilsLibrary::getDebugStringFromENetMode(ENetMode t)
{
    if (t == ENetMode::NM_Standalone)
    {
        return TEXT("NM_Standalone");
    }
    else if (t == ENetMode::NM_DedicatedServer)
    {
        return TEXT("NM_DedicatedServer");
    }
    else if (t == ENetMode::NM_ListenServer)
    {
        return TEXT("NM_ListenServer");
    }
    else if (t == ENetMode::NM_Client)
    {
        return TEXT("NM_Client");
    }
    else if (t == ENetMode::NM_MAX)
    {
        return TEXT("NM_MAX");
    }
    else {
        return TEXT("Invalid");
    }
}

void UMyCommonUtilsLibrary::MyBpLog(UObject* WorldContextObject, const FString& InString, bool bPrintToScreen, bool bPrintToLog, MyLogVerbosity eV, FLinearColor TextColor, float Duration)
{
    const FString *pStr = &InString;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);


    FString Prefix;
    if (World)
    {
        Prefix = getDebugStringFromEWorldType(World->WorldType) + TEXT(" ") + getDebugStringFromENetMode(World->GetNetMode()) + TEXT(": ");
        Prefix += InString;
        pStr = &Prefix;
    }

    if (bPrintToLog) {
        //the fuck is that, UE_LOG's log level is a compile time check
        if (eV == MyLogVerbosity::Display) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("%s"), **pStr);
        }
        else if (eV == MyLogVerbosity::Warning) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("%s"), **pStr);
        }
        else if (eV == MyLogVerbosity::Error) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s"), **pStr);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Log, TEXT("%s"), **pStr);
        }


        APlayerController* PC = (WorldContextObject ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : NULL);
        ULocalPlayer* LocalPlayer = (PC ? Cast<ULocalPlayer>(PC->Player) : NULL);
        if (LocalPlayer && LocalPlayer->ViewportClient && LocalPlayer->ViewportClient->ViewportConsole)
        {
            LocalPlayer->ViewportClient->ViewportConsole->OutputText(*pStr);
        }
    }

    if (bPrintToScreen)
    {
        if (GAreScreenMessagesEnabled)
        {
            if (GConfig && Duration < 0)
            {
                GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
            }

            bool bPrint = true;
            //#if (UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test
            bPrint = (uint8)eV >= (uint8)MyLogVerbosity::Display;
            //#endif
            if (bPrint) {
                GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), *pStr);
            }
        }
        else
        {
            UE_LOG(LogBlueprint, VeryVerbose, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
        }
    }

};

FString UMyCommonUtilsLibrary::getClassAssetPath(UClass* pC)
{
    //two way:
    // 1st is UPackage* pP = Cast<UPackage>(m_cCfgCardClass->GetDefaultObject()->GetOuter()); pP->FileName; result like /Game/CoreBPs/MyMJGameCardBaseBp
    // 2nd is m_cCfgCardClass->GetPathName(NULL), result like /Game/CoreBPs/MyMJGameCardBaseBp.MyMJGameCardBaseBp_C
    FString ret;

    if (!pC) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("class is NULL"));
        return ret;
    }

    UObject* o = pC->GetDefaultObject()->GetOuter();
    if (!o) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("outer is NULL"));
        return ret;
    }
    UPackage* pP = Cast<UPackage>(o);
    if (!o) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to cast, class name is %s."), *o->GetClass()->GetName());
        return ret;
    }

    FString assetFullName = pP->FileName.ToString();
    int32 idxLastDelim = -1;
    if (!assetFullName.FindLastChar('/', idxLastDelim)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to find delimitor, orinin str %s."), *assetFullName);
        return ret;
    }

    MY_VERIFY(idxLastDelim >= 0);
    ret = assetFullName.Left(idxLastDelim);

    return ret;

    //UWidgetLayoutLibrary
}

struct FMyScreenDataCache
{
public:
    FSceneViewProjectionData m_cSceneViewProjectionData;
    FMatrix m_cViewProjMatrix;
    FMatrix m_cInvViewProjMatrix;

};

static FMyScreenDataCache g_sMyScreenDataCache;
static bool g_bMyScreenDataCacheValid;

void UMyCommonUtilsLibrary::invalidScreenDataCache()
{
    g_bMyScreenDataCacheValid = false;
}

void UMyCommonUtilsLibrary::refillScreenDataCache(const UObject* WorldContextObject)
{
    g_bMyScreenDataCacheValid = false;
    APlayerController* Player = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
    if (LP && LP->ViewportClient)
    {
        // get the projection data
        if (LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ g_sMyScreenDataCache.m_cSceneViewProjectionData))
        {
            g_sMyScreenDataCache.m_cViewProjMatrix = g_sMyScreenDataCache.m_cSceneViewProjectionData.ComputeViewProjectionMatrix();
            g_sMyScreenDataCache.m_cInvViewProjMatrix = g_sMyScreenDataCache.m_cViewProjMatrix.InverseFast();
            g_bMyScreenDataCacheValid = true;

            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("GetConstrainedViewRect(): %s; %s"),
             //*g_sMyScreenDataCache.m_cSceneViewProjectionData.GetViewRect().ToString(),
             //*g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().ToString());
            /*
            FMatrix const ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
            const bool bResult = FSceneView::ProjectWorldToScreen(WorldPosition, ProjectionData.GetConstrainedViewRect(), ViewProjectionMatrix, ScreenPosition);

            if (bPlayerViewportRelative)
            {
            ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
            }

            return bResult;
            */
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed to GetProjectionData()."));
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("ViewPort related pointer invalid: %p, %p, %p."), Player, LP, LP ? LP->ViewportClient : NULL);
    }
}

void UMyCommonUtilsLibrary::playerScreenFullPosiAbsoluteToPlayerScreenConstrainedPosiAbsolute(const UObject* WorldContextObject, const FVector2D& FullPosiAbsolute, FVector2D& ConstrainedPosiAbsolute)
{
    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        ConstrainedPosiAbsolute = FVector2D::ZeroVector;
        return;
    }

    ConstrainedPosiAbsolute = FullPosiAbsolute - FVector2D(g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().Min);
}

void UMyCommonUtilsLibrary::playerScreenConstrainedPosiAbsoluteToPlayerScreenFullPosiAbsolute(const UObject* WorldContextObject, const FVector2D& ConstrainedPosiAbsolute, FVector2D& FullPosiAbsolute)
{
    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        FullPosiAbsolute = FVector2D::ZeroVector;
        return;
    }

    FullPosiAbsolute = ConstrainedPosiAbsolute + FVector2D(g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().Min);
}

void UMyCommonUtilsLibrary::playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(const UObject* WorldContextObject, const FVector2D& ConstrainedPosiPercent, FVector2D& ConstrainedPosiAbsolute)
{
    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        ConstrainedPosiAbsolute = FVector2D::ZeroVector;
        return;
    }

    const FIntRect& r = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect();
    ConstrainedPosiAbsolute.X = ConstrainedPosiPercent.X * r.Width();
    ConstrainedPosiAbsolute.Y = ConstrainedPosiPercent.Y * r.Height();
}

void UMyCommonUtilsLibrary::getPlayerScreenSizeAbsolute(const UObject* WorldContextObject, FVector2D& ConstrainedSize, FVector2D& FullSize)
{
    ConstrainedSize = FullSize = FVector2D(100, 100);

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return;
    }

    ConstrainedSize = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect().Size();
    FullSize = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetViewRect().Size();
};

bool UMyCommonUtilsLibrary::myProjectWorldToScreen(const UObject* WorldContextObject, const FVector& WorldPosition, bool ShouldOutScreenPosiAbsoluteConstrained, FVector2D& OutScreenPosiAbsolute)
{
    OutScreenPosiAbsolute = FVector2D::ZeroVector;

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return false;
    }

    const FSceneViewProjectionData& ProjectionData = g_sMyScreenDataCache.m_cSceneViewProjectionData;
    const FMatrix& ViewProjectionMatrix = g_sMyScreenDataCache.m_cViewProjMatrix;

    FVector2D posi;
    const bool bResult = FSceneView::ProjectWorldToScreen(WorldPosition, ProjectionData.GetConstrainedViewRect(), ViewProjectionMatrix, posi);

    if (bResult) {
        if (ShouldOutScreenPosiAbsoluteConstrained) {
            playerScreenFullPosiAbsoluteToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, posi, OutScreenPosiAbsolute);
        }
        else {
            OutScreenPosiAbsolute = posi;
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("failed in ProjectWorldToScreen()."));
    }

    return bResult;

};

bool UMyCommonUtilsLibrary::myDeprojectScreenToWorld(const UObject* WorldContextObject, const FVector2D& PosiAbsolute, bool IsPosiConstrained, FVector& WorldPosition, FVector& WorldDirection)
{
    WorldPosition = FVector::ZeroVector;
    WorldDirection = FVector::ZeroVector;

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return false;
    }

    FVector2D ScreenPosition = PosiAbsolute;
    if (IsPosiConstrained) {
        playerScreenConstrainedPosiAbsoluteToPlayerScreenFullPosiAbsolute(WorldContextObject, PosiAbsolute, ScreenPosition);
    }

    const FSceneViewProjectionData& ProjectionData = g_sMyScreenDataCache.m_cSceneViewProjectionData;
    const FMatrix& InvViewProjMatrix = g_sMyScreenDataCache.m_cInvViewProjMatrix;
    FSceneView::DeprojectScreenToWorld(ScreenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ WorldPosition, /*out*/ WorldDirection);
    return true;
}

void UMyCommonUtilsLibrary::playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(const UObject* WorldContextObject, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, float &DistanceFromCamera, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    DistanceFromCamera = 100;
    CameraCenterWorldPosition = CameraCenterDirection = FVector(0, 0, 1);

    if (!g_bMyScreenDataCacheValid) {
        refillScreenDataCache(WorldContextObject);
    }

    if (!g_bMyScreenDataCacheValid) {
        return;
    }

    const FIntRect& r = g_sMyScreenDataCache.m_cSceneViewProjectionData.GetConstrainedViewRect();

    FVector2D p0, p1, pCenter;
    p0.X = r.Width() / 2;
    p0.Y = r.Height() / 2 - ConstrainedVLengthAbsoluteInCamera / 2;

    p1.X = r.Width() / 2;
    p1.Y = r.Height() / 2 + ConstrainedVLengthAbsoluteInCamera / 2;

    pCenter.X = r.Width() / 2;
    pCenter.Y = r.Height() / 2;

    FVector worldPosition0, worldDirection0, worldPosition1, worldDirection1;

    if (!myDeprojectScreenToWorld(WorldContextObject, p0, true, worldPosition0, worldDirection0)) {
        return;
    }

    if (!myDeprojectScreenToWorld(WorldContextObject, p1, true, worldPosition1, worldDirection1)) {
        return;
    }

    float l0 = (worldPosition0 - worldPosition1).Size();
    float deltal = (worldDirection1 - worldDirection0).Size();

    if (deltal < 0.00001) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("delta %f is too small, fixing it."), deltal);
        deltal = 0.00001;
    }

    DistanceFromCamera = (ModelInWorldHeight - l0) / deltal;

    if (DistanceFromCamera < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("lengthFromCamera %f is too small, fixing it."), DistanceFromCamera);
        DistanceFromCamera = 0;
    }

    if (!myDeprojectScreenToWorld(WorldContextObject, pCenter, true, CameraCenterWorldPosition, CameraCenterDirection)) {
        return;
    }

    //rotatorFacingCameraInWorld = UKismetMathLibrary::FindLookAtRotation(worldDirection0 * 100, FVector::ZeroVector);

    return;
}


void UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByAbsolute(const UObject* WorldContextObject, FVector2D ConstrainedPosiAbsoluteInCamera, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    float distanceFromCamera = 10;
    playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(WorldContextObject, ConstrainedVLengthAbsoluteInCamera, ModelInWorldHeight, distanceFromCamera, CameraCenterWorldPosition, CameraCenterDirection);

    FVector worldPosition0(0, 0, 0), worldDirection0(0, 0, 0);
    myDeprojectScreenToWorld(WorldContextObject, ConstrainedPosiAbsoluteInCamera, true, worldPosition0, worldDirection0);

    FVector loc = worldPosition0 + (worldDirection0 * distanceFromCamera);
    ResultTransform.SetLocation(loc);
    ResultTransform.SetRotation(UKismetMathLibrary::FindLookAtRotation(CameraCenterDirection * 100, FVector::ZeroVector).Quaternion());
    ResultTransform.SetScale3D(FVector(1, 1, 1));
}

void UMyCommonUtilsLibrary::helperResolveWorldTransformFromPlayerCameraByPercent(const UObject* WorldContextObject, FVector2D ConstrainedPosiPercentInCamera, float ConstrainedVLengthPercentInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    FVector2D ConstrainedPosiAbsoluteInCamera;
    playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, ConstrainedPosiPercentInCamera, ConstrainedPosiAbsoluteInCamera);

    FVector2D vPercent(0, 0), vAbsolute(0, 0);
    vPercent.Y = ConstrainedVLengthPercentInCamera;
    playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, vPercent, vAbsolute);

    helperResolveWorldTransformFromPlayerCameraByAbsolute(WorldContextObject, ConstrainedPosiAbsoluteInCamera, vAbsolute.Y, ModelInWorldHeight, ResultTransform, CameraCenterWorldPosition, CameraCenterDirection);
}

#define MyArtDirBase (TEXT("/Game/Art"))
#define MyArtDirNameCommon (TEXT("Common"))
#define MyArtDirNameCurves (TEXT("Curves"))
#define MyArtFileNameCurveVectorDefaultLinear (TEXT("CurveVectorDefaultLinear"))

UCurveVector* UMyCommonUtilsLibrary::getCurveVectorDefaultLinear()
{
    FString fullName = FString(MyArtDirBase) + TEXT("/") + MyArtDirNameCommon + TEXT("/") + MyArtDirNameCurves + TEXT("/") + MyArtFileNameCurveVectorDefaultLinear;

    UCurveVector* pRet = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UCurveVector>(NULL, fullName);
    MY_VERIFY(pRet);

    return pRet;
};

void UMyCommonUtilsLibrary::helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
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

float UMyCommonUtilsLibrary::helperGetRemainTimePercent(const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas)
{
    float total = 1;
    int32 l = stepDatas.Num();
    for (int32 i = 0; i < l; i++) {
        total -= stepDatas[i].m_fTimePercent;
    }

    if (total < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("total remain is negative: %f."), total);
        return 0;
    }

    return total;
}

void UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                                    const FMyActorTransformUpdateAnimationStepCpp& stepData,
                                                                    const TArray<UMyTransformUpdateSequenceMovementComponent *>& actorComponentsSortedGroup)
{
    float fTotalTime = meta.m_fTotalTime;
    const FTransform& pointTransform = meta.m_cPointTransform;
    const FTransform& disappearTransform = meta.m_cDisappearTransform;
    const FVector& modelBoxExtend = meta.m_cModelBoxExtend;

    const FMyActorTransformUpdateAnimationStepCpp& cStepData = stepData;

    TArray<FTransform> aNextTransforms;

    int32 l = actorComponentsSortedGroup.Num();

    aNextTransforms.Reset();
    aNextTransforms.AddDefaulted(l);

    if (l <= 0) {
        return;
    };

    for (int32 i = 0; i < l; i++) {
        if (actorComponentsSortedGroup[i] == NULL)
        {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("actorComponentsSortedGroup[%d] == NULL."), i);
            return;
        }
    }

    //find the middle one
    int32 idxCenter = l / 2;
    UMyTransformUpdateSequenceMovementComponent* componentCenter = actorComponentsSortedGroup[idxCenter];
    FTransform& nextTransformCenter = aNextTransforms[idxCenter];


    if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::PrevLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(actorComponentsSortedGroup[i]->getHelperTransformPrevRefConst().GetLocation());
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::FinalLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(actorComponentsSortedGroup[i]->getHelperTransformFinalRefConst().GetLocation());
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::offsetFromPrevLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(actorComponentsSortedGroup[i]->getHelperTransformPrevRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend * 2);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::offsetFromFinalLocation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetLocation(actorComponentsSortedGroup[i]->getHelperTransformFinalRefConst().GetLocation() + cStepData.m_cLocationOffsetPercent * modelBoxExtend * 2);
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::PointOnPlayerScreen)
    {
        FVector location = pointTransform.GetLocation();
        for (int32 i = 0; i < l; i++) {
            if (i == idxCenter) {
                aNextTransforms[i].SetLocation(location);
            }
            else {
                aNextTransforms[i].SetLocation(location + actorComponentsSortedGroup[i]->getHelperTransformFinalRefConst().GetLocation() - componentCenter->getHelperTransformFinalRefConst().GetLocation());
            }
        }
    }
    else if (cStepData.m_eLocationUpdateType == MyActorTransformUpdateAnimationLocationType::DisappearAtAttenderBorderOnPlayerScreen)
    {
        FVector location = disappearTransform.GetLocation();
        for (int32 i = 0; i < l; i++) {
            if (i == idxCenter) {
                aNextTransforms[i].SetLocation(location);
            }
            else {
                aNextTransforms[i].SetLocation(location + actorComponentsSortedGroup[i]->getHelperTransformFinalRefConst().GetLocation() - componentCenter->getHelperTransformFinalRefConst().GetLocation());
            }
        }
    }

    if (cStepData.m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::PrevRotation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(actorComponentsSortedGroup[i]->getHelperTransformPrevRefConst().GetRotation());
        }
    }
    else if (cStepData.m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::FinalRotation)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(actorComponentsSortedGroup[i]->getHelperTransformFinalRefConst().GetRotation());
        }
    }
    else if (cStepData.m_eRotationUpdateType == MyActorTransformUpdateAnimationRotationType::FacingPlayerScreen)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetRotation(pointTransform.GetRotation());
        }
    }

    if (cStepData.m_eScaleUpdateType == MyActorTransformUpdateAnimationScaleType::PrevScale)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(actorComponentsSortedGroup[i]->getHelperTransformPrevRefConst().GetScale3D());
        }
    }
    else  if (cStepData.m_eScaleUpdateType == MyActorTransformUpdateAnimationScaleType::FinalScale)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(actorComponentsSortedGroup[i]->getHelperTransformFinalRefConst().GetScale3D());
        }
    }
    else  if (cStepData.m_eScaleUpdateType == MyActorTransformUpdateAnimationScaleType::Specified)
    {
        for (int32 i = 0; i < l; i++) {
            aNextTransforms[i].SetScale3D(cStepData.m_cScaleSpecified);
        }
    }


    float fDur = fTotalTime * cStepData.m_fTimePercent;
    for (int32 i = 0; i < l; i++) {

        UMyTransformUpdateSequenceMovementComponent *pSeqComp = actorComponentsSortedGroup[i];

        FTransformUpdateSequencDataCpp data;
        data.helperSetDataBySrcAndDst(pSeqComp->getHelperTransformPrevRefConst(), aNextTransforms[i], fDur, cStepData.m_cRotationUpdateExtraCycles);
        pSeqComp->addSeqToTail(data, cStepData.m_pCurve == NULL ? UMyCommonUtilsLibrary::getCurveVectorDefaultLinear() : cStepData.m_pCurve);
    }
};

void UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                                        const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                                        const TArray<UMyTransformUpdateSequenceMovementComponent *>& actorComponentsSortedGroup)
{
    TArray<float> m_aTimePercents;
    float total = 0;

    int32 l = stepDatas.Num();
    for (int32 i = 0; i < l; i++) {
        helperSetupTransformUpdateAnimationStep(meta, stepDatas[i], actorComponentsSortedGroup);
        float f = stepDatas[i].m_fTimePercent;
        m_aTimePercents.Emplace(f);
        total += f;
    }

    if (l > 0 && !FMath::IsNearlyEqual(total, 1, MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("total time is not 100%, str %s. now it is %f."), *meta.m_sDebugString, total);
    };
}

void UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationStepsForPoint(const UObject* WorldContextObject,
                                                                                float totalDur,
                                                                                const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                                const FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                                const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                                                float extraDelayDur,
                                                                                const TArray<IMyTransformUpdateSequenceInterface*>& actorsInterfaces,
                                                                                FString debugName,
                                                                                bool clearSeq)
{
    FMyTransformUpdateAnimationMetaCpp meta, *pMeta = &meta;
    TArray<UMyTransformUpdateSequenceMovementComponent *> aComponents, *pComponents = &aComponents;

    //prepare meta
    pMeta->m_sDebugString = debugName;
    pMeta->m_fTotalTime = totalDur;
    if (actorsInterfaces.Num() <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("actorsInterfaces num is zero."));
        return;
    }
    FMyActorModelInfoBoxCpp modelInfo;
    actorsInterfaces[0]->getModelInfo(modelInfo);
    pMeta->m_cModelBoxExtend = modelInfo.m_cBoxExtend;

    helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, pointInfo.m_fShowPosiFromCenterToBorderPercent, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelBoxExtend.Z * 2, pMeta->m_cPointTransform);
    helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(WorldContextObject, pointAndCenterMeta, 1.2, pointInfo.m_cExtraOffsetScreenPercent, pointInfo.m_fTargetVLengthOnScreenScreenPercent, pMeta->m_cModelBoxExtend.Z * 2, pMeta->m_cDisappearTransform);

    bool bDelay = extraDelayDur >= MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT;

    pComponents->Reset();
    int32 l = actorsInterfaces.Num();
    for (int32 i = 0; i < l; i++) {
        UMyTransformUpdateSequenceMovementComponent* pSeq = actorsInterfaces[i]->getTransformUpdateSequence();
        pComponents->Emplace(pSeq);
        if (clearSeq) {
            pSeq->clearSeq();
        }
    }

    if (bDelay) {
        UMyCommonUtilsLibrary::helperAddWaitStep(extraDelayDur, debugName + TEXT(" wait"), *pComponents);
    }

    UMyCommonUtilsLibrary::helperSetupTransformUpdateAnimationSteps(meta, stepDatas, *pComponents);
}

void UMyCommonUtilsLibrary::helperAddWaitStep(float waitTime, FString debugStr, const TArray<UMyTransformUpdateSequenceMovementComponent*>& comps)
{
    FMyTransformUpdateAnimationMetaCpp meta;
    FMyActorTransformUpdateAnimationStepCpp stepData;

    meta.m_sDebugString = debugStr;
    meta.m_fTotalTime = waitTime;

    stepData.m_fTimePercent = 1;


    helperSetupTransformUpdateAnimationStep(meta, stepData, comps);
}

void UMyCommonUtilsLibrary::helperAddWaitStep(float waitTime, FString debugStr, const TArray<IMyTransformUpdateSequenceInterface*>& interfaces)
{

    TArray<UMyTransformUpdateSequenceMovementComponent *> actorComponentsSortedGroup;

    for (int32 i = 0; i < interfaces.Num(); i++)
    {
        actorComponentsSortedGroup.Emplace(interfaces[i]->getTransformUpdateSequence());
    }

    UMyCommonUtilsLibrary::helperAddWaitStep(waitTime, debugStr, actorComponentsSortedGroup);
}

