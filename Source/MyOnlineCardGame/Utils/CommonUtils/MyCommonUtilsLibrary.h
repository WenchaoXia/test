// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtils.h"

#include "MyCommonUtilsLibrary.generated.h"

UENUM()
enum class MyLogVerbosity : uint8
{
    None = 0                     UMETA(DisplayName = "None"),
    Log = 10               UMETA(DisplayName = "Log"),
    Display = 20         UMETA(DisplayName = "Display"),
    Warning = 30    UMETA(DisplayName = "Warning"),
    Error = 40    UMETA(DisplayName = "Error"),

};

//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyCommonUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:


    template< class T >
    static inline T* helperTryFindAndLoadAsset(UObject* outer, const FString &resFullPath)
    {
        //T *pRes = FindObject<T>(outer, *resFullPath);
        T *pRes = NULL;
        if (!IsValid(pRes)) {
            pRes = NULL;
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("find object fail, maybe forgot preload it: %s."), *resFullPath);
            pRes = LoadObject<T>(outer, *resFullPath, NULL, LOAD_None, NULL);
            if (!IsValid(pRes)) {
                pRes = NULL;
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("load object fail: %s."), *resFullPath);
            }

            //T *pRes2 = FindObject<T>(outer, *resFullPath);
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("refind object result: %d."), pRes2 != NULL);
        }

        return pRes;

        //return (T*)StaticFindObject(T::StaticClass(), Outer, Name, ExactClass);
    }

    UFUNCTION(BlueprintCallable)
    static int32 getEngineNetMode(AActor *actor);

    inline
        static FString genTimeStrFromTimeMs(uint32 uiTime_ms)
    {
        return FString::Printf(TEXT("%u.%03u"), uiTime_ms / 1000, uiTime_ms % 1000);
    };


    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static void rotateOriginWithPivot(const FTransform& originCurrentWorldTransform, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultWorldTransform);

    static FString getDebugStringFromEWorldType(EWorldType::Type t);
    static FString getDebugStringFromENetMode(ENetMode t);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log print", AdvancedDisplay = "2"), Category = "Utilities|String")
    static void MyBpLog(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool PrintToScreen = true, bool PrintToLog = true, MyLogVerbosity eV = MyLogVerbosity::Display, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);
    //float GetRealTimeSeconds() const;

    //return like /Game/[subpath], or Empty if error
    static FString getClassAssetPath(UClass* pC);


    //Warning:: we don't support multiple local player screen in one client!
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (UnsafeDuringActorConstruction = "true"))
    static void invalidScreenDataCache();

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void refillScreenDataCache(const UObject* WorldContextObject);

    //Constrained position means the camera's coordinate system, which may have black bars in player screen
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void playerScreenFullPosiAbsoluteToPlayerScreenConstrainedPosiAbsolute(const UObject* WorldContextObject, const FVector2D& FullPosiAbsolute, FVector2D& ConstrainedPosiAbsolute);

    //Constrained position means the camera's coordinate system, which may have black bars in player screen
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void playerScreenConstrainedPosiAbsoluteToPlayerScreenFullPosiAbsolute(const UObject* WorldContextObject, const FVector2D& ConstrainedPosiAbsolute, FVector2D& FullPosiAbsolute);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void playerScreenConstrainedPosiPercentToPlayerScreenConstrainedPosiAbsolute(const UObject* WorldContextObject, const FVector2D& ConstrainedPosiPercent, FVector2D& ConstrainedPosiAbsolute);

    //got the real pixel size now, not the suppoed size like 1920 x 1080, but the running one
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void getPlayerScreenSizeAbsolute(const UObject* WorldContextObject, FVector2D& ConstrainedSize, FVector2D& FullSize);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static bool myProjectWorldToScreen(const UObject* WorldContextObject, const FVector& WorldPosition, bool ShouldOutScreenPosiAbsoluteConstrained, FVector2D& OutScreenPosiAbsolute);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static bool myDeprojectScreenToWorld(const UObject* WorldContextObject, const FVector2D& PosiAbsolute, bool IsPosiConstrained, FVector& WorldPosition, FVector& WorldDirection);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(const UObject* WorldContextObject, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, float &DistanceFromCamera, FRotator &rotatorFacingCameraInWorld);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void helperResolveWorldTransformFromPlayerCamera(const UObject* WorldContextObject, FVector2D ConstrainedPosiPercentInCamera, float ConstrainedVLengthPercentInCamera, float ModelInWorldHeight, FTransform& ResultTransform);

};