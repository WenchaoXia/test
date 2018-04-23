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

//#include "Kismet/KismetRenderingLibrary.h"
//#include "Classes/PaperSprite.h"


bool FMyModelInfoBoxWidget2DCpp::equals(const FMyModelInfoBoxWidget2DCpp& other, float tolerance) const
{
    return m_cCenterPointRelativeLocation.Equals(other.m_cCenterPointRelativeLocation, tolerance) &&
            m_cBoxExtend.Equals(other.m_cBoxExtend, tolerance);
}

FString FMyModelInfoBoxWidget2DCpp::ToString() const
{
    return FString::Printf(TEXT("CenterRelative: %s, BoxExtend: %s"), *m_cCenterPointRelativeLocation.ToString(), *m_cBoxExtend.ToString());
}


bool FMyModelInfoWidget2DCpp::equals(const FMyModelInfoWidget2DCpp& other, float tolerance) const
{
    if (m_eType != other.m_eType) {
        return false;
    }

    if (m_eType == MyModelInfoType::BoxWidget2D) {
        if (!m_cBox.equals(other.m_cBox, tolerance)) {
            return false;
        }
    }
    else {

    }


    return true;
}

FString FMyModelInfoWidget2DCpp::ToString() const
{
    FString ret = FString::Printf(TEXT("type: %s. "), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyModelInfoType"), (uint8)m_eType));
    if (m_eType == MyModelInfoType::BoxWidget2D) {
        ret += m_cBox.ToString();
    }

    return ret;
}


void FMyLocationOfZRotationAroundPointCoordinateCpp::interp(const FMyLocationOfZRotationAroundPointCoordinateCpp& start, const FMyLocationOfZRotationAroundPointCoordinateCpp& end, float percent, FMyLocationOfZRotationAroundPointCoordinateCpp& result)
{
    float percentFixed = FMath::Clamp<float>(percent, 0, 1);
    result = start + (end - start) * percentFixed;
}

void FMyTransformOfZRotationAroundPointCoordinateCpp::interp(const FMyTransformOfZRotationAroundPointCoordinateCpp& start, const FMyTransformOfZRotationAroundPointCoordinateCpp& end, float percent, FMyTransformOfZRotationAroundPointCoordinateCpp& result)
{
    float percentFixed = FMath::Clamp<float>(percent, 0, 1);

    FMyLocationOfZRotationAroundPointCoordinateCpp::interp(start.m_cLocation, end.m_cLocation, percentFixed, result.m_cLocation);

    result.m_cRotatorOffsetFacingCenterPoint = UKismetMathLibrary::RLerp(start.m_cRotatorOffsetFacingCenterPoint, end.m_cRotatorOffsetFacingCenterPoint, percentFixed, true);

    //result.m_cRotatorOffsetFacingCenterPoint = FMath::RInterpTo(start.m_cRotatorOffsetFacingCenterPoint, end.m_cRotatorOffsetFacingCenterPoint, percentFixed, 1);
    
    //FQuat QNow = FMath::Lerp(start.m_cRotatorOffsetFacingCenterPoint.Quaternion(), end.m_cRotatorOffsetFacingCenterPoint.Quaternion(), percentFixed);
    //result.m_cRotatorOffsetFacingCenterPoint = QNow.Rotator();

    //result.m_cRotatorOffsetFacingCenterPoint = start.m_cRotatorOffsetFacingCenterPoint + (end.m_cRotatorOffsetFacingCenterPoint - start.m_cRotatorOffsetFacingCenterPoint) * percentFixed;
};


FString
UMyCommonUtilsLibrary::getStringFromEnum(const TCHAR *enumName, uint8 value)
{
    //const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
    const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, enumName, true);
    if (!enumPtr)
    {
        return FString("Invalid Enum type");
    }

    //return FString::Printf(TEXT("%s(%d)"), *enumPtr->GetEnumNameStringByValue(value), value);
    //return enumPtr->GetEnumNameStringByValue(value);
    return FString::Printf(TEXT("%s %d"), *enumPtr->GetNameStringByValue(value), value);
    //GetNameStringByValue
}

int64
UMyCommonUtilsLibrary::nowAsMsFromTick()
{
    return UKismetMathLibrary::Now().GetTicks() / (ETimespan::TicksPerSecond / 1000);
}

FString UMyCommonUtilsLibrary::formatStrIds(const TArray<int32> &aIds)
{
    FString str;
    int32 l = aIds.Num();

    for (int32 i = 0; i < l; i++) {
        str += FString::Printf(TEXT("(%d), "), aIds[i]);
    }

    return str;
}

FString
UMyCommonUtilsLibrary::formatStrIdsValues(const TArray<int32> &aIds, const TArray<int32> &aValues)
{
    FString str;
    int32 l = aIds.Num();
    MY_VERIFY(l == aValues.Num());
    for (int32 i = 0; i < l; i++) {
        str += FString::Printf(TEXT("(%d, %d), "), aIds[i], aValues[i]);
    }

    return str;
}

FString
UMyCommonUtilsLibrary::formatStrIdValuePairs(const TArray<FMyIdValuePair> &aIdValues)
{
    FString str;
    int32 l = aIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        str += aIdValues[i].genDebugStr() + TEXT(",");
        //str += FString::Printf(TEXT("(%d, %d), "), aIdValues[i].m_iId, aIdValues[i].m_iValue);
    }

    return str;
}


FString
UMyCommonUtilsLibrary::formatMaskString(int32 iMask, uint32 uBitsCount)
{
    FString str = FString::Printf(TEXT("%d(bin: "), iMask);

    if (uBitsCount > 32) {
        uBitsCount = 32;
    }

    while (uBitsCount > 0) {
        uBitsCount--;
        str += FString::Printf(TEXT("%d"), (iMask >> uBitsCount) & 1);
    }

    str += TEXT(")");

    return str;

}

void UMyCommonUtilsLibrary::convertIdValuePairs2Ids(const TArray<FMyIdValuePair> &aIdValues, TArray<int32> &outaValues)
{
    outaValues.Reset();

    int32 l = aIdValues.Num();
    for (int32 i = 0; i < l; i++) {
        outaValues.Emplace(aIdValues[i].m_iId);
    }
}

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


//Todo: check this function's math correctness
FRotator UMyCommonUtilsLibrary::fixRotatorValuesIfGimbalLock(const FRotator& rotator, float PitchDeltaTolerance)
{
    float PitchNormalized = FRotator::NormalizeAxis(rotator.Pitch);

    if (FMath::IsNearlyEqual(PitchNormalized, 90, PitchDeltaTolerance))
    {
        FRotator ret = rotator;
        ret.Yaw = FRotator::NormalizeAxis(rotator.Yaw - rotator.Roll);
        ret.Roll = 0;
        return ret;
    }
    else if (FMath::IsNearlyEqual(PitchNormalized, -90, PitchDeltaTolerance))
    {
        FRotator ret = rotator;
        ret.Yaw = FRotator::NormalizeAxis(rotator.Yaw + rotator.Roll);
        ret.Roll = 0;
        return ret;
    }
    else {
        return rotator;
    }

};


void UMyCommonUtilsLibrary::rotateOriginWithPivot(const FTransform& originCurrentTransformWorld3D, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultTransformWorld3D)
{
    //FRotator originCurrentWorldRotator(originCurrentTransformWorld3D.GetRotation());
    //FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    //FVector originZeroRotateLocWorld3Dation = originCurrentTransformWorld3D.GetLocation() - (originCurrentWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);

    //originResultTransformWorld3D.SetLocation(originZeroRotateLocWorld3Dation + (originTargetWorldRotator.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    //originResultTransformWorld3D.SetRotation(FQuat(originTargetWorldRotator));
    //originResultTransformWorld3D.SetScale3D(originCurrentTransformWorld3D.GetScale3D());

    FQuat originCurrentWorldQuat = originCurrentTransformWorld3D.GetRotation();
    FVector origin2pivotRelativeLocation = -pivot2OriginRelativeLocation;

    FVector testV = originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("testV: (%f, %f, %f)."), testV.X, testV.Y, testV.Z);
    //return;

    FVector originZeroRotateLocWorld3Dation = originCurrentTransformWorld3D.GetLocation() - (originCurrentWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation);
    UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("originZeroRotateLocWorld3Dation: (%f, %f, %f)."), originZeroRotateLocWorld3Dation.X, originZeroRotateLocWorld3Dation.Y, originZeroRotateLocWorld3Dation.Z);
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

    originResultTransformWorld3D.SetLocation(originZeroRotateLocWorld3Dation + (originTargetWorldQuat.RotateVector(origin2pivotRelativeLocation) - origin2pivotRelativeLocation));
    originResultTransformWorld3D.SetRotation(originTargetWorldQuat);
    originResultTransformWorld3D.SetScale3D(originCurrentTransformWorld3D.GetScale3D());
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

    if (deltal < MyDeNominatorAsZeroTolerance) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("delta %f is too small, fixing it."), deltal);
        deltal = MyDeNominatorAsZeroTolerance;
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


void UMyCommonUtilsLibrary::helperResolveTransformWorld3DFromPlayerCameraByAbsolute(const UObject* WorldContextObject, FVector2D ConstrainedPosiAbsoluteInCamera, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
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

void UMyCommonUtilsLibrary::helperResolveTransformWorld3DFromPlayerCameraByPercent(const UObject* WorldContextObject, FVector2D ConstrainedPosiPercentInCamera, float ConstrainedVLengthPercentInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection)
{
    FVector2D ConstrainedPosiAbsoluteInCamera;
    playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, ConstrainedPosiPercentInCamera, ConstrainedPosiAbsoluteInCamera);

    FVector2D vPercent(0, 0), vAbsolute(0, 0);
    vPercent.Y = ConstrainedVLengthPercentInCamera;
    playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(WorldContextObject, vPercent, vAbsolute);

    helperResolveTransformWorld3DFromPlayerCameraByAbsolute(WorldContextObject, ConstrainedPosiAbsoluteInCamera, vAbsolute.Y, ModelInWorldHeight, ResultTransform, CameraCenterWorldPosition, CameraCenterDirection);
}

#define MyArtDirBase (TEXT("/Game/Art"))
#define MyArtDirNameCommon (TEXT("Common"))
#define MyArtDirNameCurves (TEXT("Curves"))
#define MyArtFileNameCurveVectorDefaultLinear (TEXT("CurveVectorDefaultLinear"))
#define MyArtFileNameCurveVectorDefaultAccelerate0 (TEXT("CurveVectorDefaultAccelerate0"))

UCurveVector* UMyCommonUtilsLibrary::getCurveVectorByType(MyCurveAssetType curveType)
{
    FString fullName = FString(MyArtDirBase) + TEXT("/") + MyArtDirNameCommon + TEXT("/") + MyArtDirNameCurves + TEXT("/");

    if (curveType == MyCurveAssetType::DefaultLinear)
    {
        fullName += MyArtFileNameCurveVectorDefaultLinear;
    }
    else if (curveType == MyCurveAssetType::DefaultAccelerate0)
    {
        fullName += MyArtFileNameCurveVectorDefaultAccelerate0;
    }

    UCurveVector* pRet = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UCurveVector>(NULL, fullName);
    MY_VERIFY(pRet);

    return pRet;
};

UCurveVector* UMyCommonUtilsLibrary::getCurveVectorFromSettings(const FMyCurveVectorSettingsCpp& settings)
{
    UCurveVector* ret = NULL;
    if (settings.m_pCurveOverride) {
        ret = settings.m_pCurveOverride;
        if (ret == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("curve overrided but specified a NULL one. using default now."));
            ret = getCurveVectorByType(MyCurveAssetType::DefaultLinear);
        }
    }
    else {
        ret = getCurveVectorByType(settings.m_eCurveType);
    }

    MY_VERIFY(ret);

    return ret;
};


void UMyCommonUtilsLibrary::calcPointTransformWithLocalOffset(const FTransform& pointTransform, FVector localOffset, FTransform& pointTransformFixed)
{
    FTransform offset;
    offset.SetLocation(localOffset);

    pointTransformFixed = offset * pointTransform;

    //faster path that ignored scale
    //if (FMath::IsNearlyEqual(localOffset.X, 0), MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT) {
    //
    //}
}

FString UMyCommonUtilsLibrary::Conv_MyTransformZRotation_String(const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation)
{
    return myTransformZRotation.ToString();

    return TEXT("[") + myTransformZRotation.m_cLocation.ToString() + TEXT("(") + myTransformZRotation.m_cRotatorOffsetFacingCenterPoint.ToString() + TEXT(")]");
};


//Todo: if point is above center as YAW = 90D, the rotation compute is not stable when interp, fix it later
void UMyCommonUtilsLibrary::MyTransformZRotationToTransformWorld(const FTransform& centerPointTransformWorld, const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation, FTransform& transformWorld)
{
    //MY_VERIFY(transformZRotation.m_cLocation.m_fRadius >= 0);

    float radiansYaw = FMath::DegreesToRadians(myTransformZRotation.m_cLocation.m_fYawOnXYPlane);
    float s, c;
    FMath::SinCos(&s, &c, radiansYaw);

    FVector loc0;
    loc0.X = c * myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane;
    loc0.Y = s * myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane;
    loc0.Z = myTransformZRotation.m_cLocation.m_fZoffset;

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("0 loc0: %s, s %f, r %f."), *loc0.ToString(), s, radiansYaw);

    //FQuat toCenter = FRotationMatrix::MakeFromX(-loc0).ToQuat();
    FRotator toCenterR = FRotationMatrix::MakeFromX(-loc0).Rotator();
    toCenterR.Roll = 0;
    FQuat toCenter = toCenterR.Quaternion();
    //FQuat toCenter = UKismetMathLibrary::FindLookAtRotation(loc0, FVector::ZeroVector).Quaternion();
    //toCenter = fixRotatorValuesIfGimbalLock(toCenter.Rotator()).Quaternion();
    //FQuat toCenter = UKismetMathLibrary::FindLookAtRotation(loc0, FVector::ZeroVector).Quaternion();

    //FRotator UKismetMathLibrary::FindLookAtRotation(const FVector& Start, const FVector& Target)
    //FRotator UKismetMathLibrary::MakeRotFromX(const FVector& X)

    FQuat quat0 = toCenter * myTransformZRotation.m_cRotatorOffsetFacingCenterPoint.Quaternion();

    //FRotator rot0 = UKismetMathLibrary::ComposeRotators(ringPointLocalRotator, FRotator(0, FMath::RadiansToDegrees(radiansFixed), 0));

    FTransform T0;
    T0.SetLocation(loc0);
    T0.SetRotation(quat0);

    transformWorld = T0 * centerPointTransformWorld;

    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("debug: loc0 %s, toCenter %s %s, quat0 r %s, final %s."), *loc0.ToString(), *toCenter.ToString(), *toCenter.Rotator().ToString(), *quat0.Rotator().ToString(), *transformWorld.GetRotation().Rotator().ToString());
}

void UMyCommonUtilsLibrary::TransformWorldToMyTransformZRotation(const FTransform& centerPointTransformWorld, const FTransform& transformWorld, FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation)
{
    FTransform T0;
    T0 = transformWorld * centerPointTransformWorld.Inverse();

    FVector loc0 = T0.GetLocation();
    FQuat quat0 = T0.GetRotation();

    myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane = FMath::Sqrt(loc0.X * loc0.X + loc0.Y * loc0.Y);
    float s = loc0.Y / myTransformZRotation.m_cLocation.m_fRadiusOnXYPlane;;
    //float radiansYaw = FMath::Asin(s);
    float radiansYaw = FMath::Atan2(loc0.Y, loc0.X); //ATan use UE4's X Y coordinate
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("1 loc0: %s, s %f, r %f."), *loc0.ToString(), s, radiansYaw);

    myTransformZRotation.m_cLocation.m_fYawOnXYPlane = FRotator::ClampAxis(FMath::RadiansToDegrees(radiansYaw));
    myTransformZRotation.m_cLocation.m_fZoffset = loc0.Z;

    //FQuat toCenter = FRotationMatrix::MakeFromX(-loc0).ToQuat();
    
    FRotator toCenterR = FRotationMatrix::MakeFromX(-loc0).Rotator();
    toCenterR.Roll = 0;
    FQuat toCenter = toCenterR.Quaternion();

    //toCenter = fixRotatorValuesIfGimbalLock(toCenter.Rotator()).Quaternion();

    //myTransformZRotation.m_cRotatorOffsetFacingCenterPoint = (toCenter.Inverse() * quat0).Rotator();
    FRotator r0 = (toCenter.Inverse() * quat0).Rotator();

    myTransformZRotation.m_cRotatorOffsetFacingCenterPoint = r0; // fixRotatorValuesIfGimbalLock(r0);
}

/*
void UMyCommonUtilsLibrary::calcRingPointTransformAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, FTransform& ringPointTransform, FVector ringPointLocalOffset, FRotator ringPointLocalRotator)
{
 */
 /*
    FTransform TA, TB;
    FVector vA = ringPointLocalOffset;
    vA.X += ringRadius;

    TA.SetLocation(vA);
    TA.SetRotation(ringPointLocalRotator.Quaternion());


    FRotator rB = FRotator::ZeroRotator;
    rB.Yaw = degree;

    TB.SetRotation(rB.Quaternion());

    FTransform T0 = TA * TB;

    ringPointTransform = T0 * centerPointTransform;
    */

    //use 2d->3d translate style so we can map in two sides in one-one way

    /*
    float radiusFixed, radiansDelta;
    if (0 != fixRadiusAndRadiansForLocalOffsetOn2DCycle(ringRadius, ringPointLocalOffset.X, ringPointLocalOffset.Y, radiusFixed, radiansDelta))
    {
        return;
    }

    float radiansFixed = FMath::DegreesToRadians(degree) + radiansDelta;

    calcRingPointTransformAroundCenterPointZAxisWithFixedData(centerPointTransform, radiusFixed, radiansFixed, ringPointTransform, ringPointLocalOffset.Z, ringPointLocalRotator);
}


void UMyCommonUtilsLibrary::calcTransformWorld3DFromWorldOffsetAndDegreeForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, const FTransform& worldOffset, FTransform& TransformWorld3D, FVector ringPointLocalOffset, FRotator ringPointLocalRotator)
{
    float radiusFixed, radiansDelta;
    if (0 != fixRadiusAndRadiansForLocalOffsetOn2DCycle(ringRadius, ringPointLocalOffset.X, ringPointLocalOffset.Y, radiusFixed, radiansDelta))
    {
        return;
    }

    float radiansFixed = FMath::DegreesToRadians(degree) + radiansDelta;

    FTransform ringPointTransform;
    calcRingPointTransformAroundCenterPointZAxisWithFixedData(centerPointTransform, radiusFixed, radiansFixed, ringPointTransform, ringPointLocalOffset.Z, ringPointLocalRotator);

    FQuat ringPointFRotator(0, 0, FMath::RadiansToDegrees(radiansFixed)).Quaternion() * centerPointTransform.GetRotation();

    FVector locLocalOffsetTowardOuter;

    TransformWorld3D.SetLocation(worldOffset.GetLocation() + ringPointTransform.GetLocation());
    TransformWorld3D.SetRotation((worldOffset.GetRotation().Rotator() + ringPointTransform.GetRotation().Rotator()).Quaternion());
    TransformWorld3D.SetScale3D(FVector(1, 1, 1));
}

void UMyCommonUtilsLibrary::calcWorldOffsetAndDegreeFromTransformWorld3DForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, const FTransform& TransformWorld3D, float &degree, FTransform& worldOffset, FVector ringPointLocalOffset, FRotator ringPointLocalRotator)
{
    float radiusFixed, radiansDelta;
    if (0 != fixRadiusAndRadiansForLocalOffsetOn2DCycle(ringRadius, ringPointLocalOffset.X, ringPointLocalOffset.Y, radiusFixed, radiansDelta))
    {
        return;
    }

    FQuat centerPointQuat = centerPointTransform.GetRotation();
    FVector worldPointRelativePosi = TransformWorld3D.GetLocation() - centerPointTransform.GetLocation();

    FVector posiMapped;
    posiMapped.X = FVector::DotProduct(centerPointQuat.GetAxisX(), worldPointRelativePosi);
    posiMapped.Y = FVector::DotProduct(centerPointQuat.GetAxisY(), worldPointRelativePosi);
    float radiansMapped = posiMapped.HeadingAngle();

    FTransform ringPointTransform;
    calcRingPointTransformAroundCenterPointZAxisWithFixedData(centerPointTransform, radiusFixed, radiansMapped, ringPointTransform, ringPointLocalOffset.Z, ringPointLocalRotator);

    degree = FMath::RadiansToDegrees(radiansMapped - radiansDelta);

    worldOffset.SetLocation(TransformWorld3D.GetLocation() - ringPointTransform.GetLocation());
    worldOffset.SetRotation((TransformWorld3D.GetRotation().Rotator() - ringPointTransform.GetRotation().Rotator()).Quaternion());
    worldOffset.SetScale3D(FVector(1, 1, 1));
}

void UMyCommonUtilsLibrary::calcRingPointTransformAroundCenterPointZAxisWithFixedData(const FTransform& centerPointTransform, float radiusFixed, float radiansFixed, FTransform& ringPointTransform, float ringPointLocalZOffset, const FRotator& ringPointLocalRotator)
{
    float s, c;
    FMath::SinCos(&s, &c, radiansFixed);

    //enter 3d domain
    FVector loc0;
    loc0.X = c * radiusFixed;
    loc0.Y = s * radiusFixed;
    loc0.Z = ringPointLocalZOffset;

    FRotator rot0 = UKismetMathLibrary::ComposeRotators(ringPointLocalRotator, FRotator(0, FMath::RadiansToDegrees(radiansFixed), 0));

    FTransform T0;
    T0.SetLocation(loc0);
    T0.SetRotation(rot0.Quaternion());

    ringPointTransform = T0 * centerPointTransform;
}
*/

int32 UMyCommonUtilsLibrary::fixRadiusAndRadiansForLocalOffsetOn2DCycle(float radius, float xOffset, float yOffset, float &radiusFixed, float &radiansDelta)
{
    if (radius < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("radius is negative: %f."), radius);
        return -1;
    }

    radiusFixed = FMath::Sqrt(radius * radius + yOffset * yOffset) + xOffset;

    if (radiusFixed < 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("radiusFixed is negative: %f, radius %f, xOffset %f."), radiusFixed, radius, xOffset);
        return -2;
    }

    radiansDelta = FMath::Atan2(yOffset, radius);

    return 0;
}