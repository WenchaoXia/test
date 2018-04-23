// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtils.h"
#include "MyCommonInterface.h"

#include "Components/TimelineComponent.h"
#include "GameFramework/MovementComponent.h"
#include "Components/TimelineComponent.h"
//#include "Blueprint/UserWidget.h"

#include "MyCommonUtilsLibrary.generated.h"

//#define MY_ROTATOR_MIN_TOLERANCE (0.01f)

#define MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT (0.01f)

UENUM()
enum class MyLogVerbosity : uint8
{
    None = 0                     UMETA(DisplayName = "None"),
    Log = 10               UMETA(DisplayName = "Log"),
    Display = 20         UMETA(DisplayName = "Display"),
    Warning = 30    UMETA(DisplayName = "Warning"),
    Error = 40    UMETA(DisplayName = "Error"),

};


//model always and must facing x axis
USTRUCT(BlueprintType)
struct FMyModelInfoBoxWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyModelInfoBoxWorld3DCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_cCenterPointRelativeLocation = FVector::ZeroVector;
        m_cBoxExtend = FVector::OneVector;
    };

    inline FString ToString() const
    {
        return FString::Printf(TEXT("CenterRelative: %s, BoxExtend: %s"), *m_cCenterPointRelativeLocation.ToString(), *m_cBoxExtend.ToString());
    };

    //component scale applied, it is the center point to prigin, only origin keeps unchanged when rotate and scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
    FVector m_cCenterPointRelativeLocation;

    //final size after all actor scale, component scale applied
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "box extend"))
        FVector m_cBoxExtend;

};

//Warn: this info require the widget in canvas have same arrangement as actor in world:
//m_cCenterPointRelativeLocation: when placed at zero, where is the center
//m_cBoxExtend: the box size
//rotate is always done around point zero
USTRUCT(BlueprintType)
struct FMyModelInfoBoxWidget2DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyModelInfoBoxWidget2DCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_cCenterPointRelativeLocation = FVector2D::ZeroVector;
        m_cBoxExtend = FVector2D::UnitVector;
    };

    inline FMyModelInfoBoxWidget2DCpp operator*(float Scale) const
    {
        FMyModelInfoBoxWidget2DCpp ret;
        ret.m_cCenterPointRelativeLocation = m_cCenterPointRelativeLocation * Scale;
        ret.m_cBoxExtend = m_cBoxExtend;
        return ret;
    };

    bool equals(const FMyModelInfoBoxWidget2DCpp& other, float tolerance) const;

    FString ToString() const;


    //component scale applied, it is the center point to prigin, only origin keeps unchanged when rotate and scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
    FVector2D m_cCenterPointRelativeLocation;

    //local size before scale, like the size in design time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "box extend"))
    FVector2D m_cBoxExtend;
};

UENUM()
enum class MyModelInfoType : uint8
{
    Invalid = 0               UMETA(DisplayName = "invalid"),
    BoxWorld3D = 10           UMETA(DisplayName = "BoxWorld3D"),
    BoxWidget2D = 110         UMETA(DisplayName = "BoxWidget2D"),
};

//Model info ignore root scale
USTRUCT(BlueprintType)
struct FMyModelInfoWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyModelInfoWorld3DCpp(MyModelInfoType eType = MyModelInfoType::BoxWorld3D)
    {
        reset(eType);
    };

    virtual ~FMyModelInfoWorld3DCpp()
    {

    };

    inline void reset(MyModelInfoType eType = MyModelInfoType::BoxWorld3D)
    {
        m_cBox.reset();
        m_eType = eType;
    };

    inline MyModelInfoType getType() const
    {
        return m_eType;
    };

    inline const FMyModelInfoBoxWorld3DCpp& getBox3DRefConst() const
    {
        if (m_eType != MyModelInfoType::BoxWorld3D) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("type is not 3D box: actually %d."), (int32)m_eType);
        }

        return m_cBox;
    };

    inline FMyModelInfoBoxWorld3DCpp& getBox3DRef()
    {
        return const_cast<FMyModelInfoBoxWorld3DCpp&>(getBox3DRefConst());
    }

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "box"))
    FMyModelInfoBoxWorld3DCpp m_cBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "type"))
    MyModelInfoType m_eType;
};


//Todo: use union or cast to save memory
USTRUCT(BlueprintType)
struct FMyModelInfoWidget2DCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyModelInfoWidget2DCpp(MyModelInfoType eType = MyModelInfoType::BoxWidget2D)
    {
        reset(eType);
    };

    virtual ~FMyModelInfoWidget2DCpp()
    {

    };

    inline void reset(MyModelInfoType eType = MyModelInfoType::BoxWidget2D)
    {
        m_cBox.reset();
        m_eType = eType;
    };

    inline MyModelInfoType getType() const
    {
        return m_eType;
    };

    inline const FMyModelInfoBoxWidget2DCpp& getBox2DRefConst() const
    {
        if (m_eType != MyModelInfoType::BoxWidget2D) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("type is not 2D box: actually %d."), (int32)m_eType);
        }

        return m_cBox;
    };

    inline FMyModelInfoBoxWidget2DCpp& getBox2DRef()
    {
        return const_cast<FMyModelInfoBoxWidget2DCpp&>(getBox2DRefConst());
    }

    inline FMyModelInfoWidget2DCpp operator*(float Scale) const
    {
        FMyModelInfoWidget2DCpp ret;
        ret.m_cBox = m_cBox * Scale;
        ret.m_eType = m_eType;
        return ret;
    }

    bool equals(const FMyModelInfoWidget2DCpp& other, float tolerance) const;

    FString ToString() const;


protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "box"))
        FMyModelInfoBoxWidget2DCpp m_cBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "type"))
        MyModelInfoType m_eType;
};

UENUM()
enum class MyCurveAssetType : uint8
{
    DefaultLinear = 0,
    DefaultAccelerate0 = 10,
};

USTRUCT(BlueprintType)
struct FMyCurveVectorSettingsCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCurveVectorSettingsCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_eCurveType = MyCurveAssetType::DefaultLinear;
        m_pCurveOverride = NULL;
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "curve type"))
    MyCurveAssetType m_eCurveType;

    //if specified, it will be used instead of default one specified from "curve type"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "curve override"))
    UCurveVector* m_pCurveOverride;
};


//we can use Rotator + center transform to identify one point in world, but may slip around when animate since there are 3 rotation dimension.
//instead, we define one coordinate system which have only one rotation dimension, make sure they do not 'slip' around

USTRUCT(BlueprintType)
struct FMyLocationOfZRotationAroundPointCoordinateCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyLocationOfZRotationAroundPointCoordinateCpp(float fRadiusOnXYPlane = 0, float fYawOnXYPlane = 0, float fZoffset = 0)
    {
        m_fRadiusOnXYPlane = fRadiusOnXYPlane;
        m_fYawOnXYPlane = fYawOnXYPlane;
        m_fZoffset = fZoffset;
    };

    inline FString ToString() const
    {
        return FString::Printf(TEXT("(RadiusOnXYPlane %f, YawOnXYPlane %f, Zoffset %f)"), m_fRadiusOnXYPlane, m_fYawOnXYPlane, m_fZoffset);
    };

    inline void reset()
    {
        m_fRadiusOnXYPlane = 0;
        m_fYawOnXYPlane = 0;
        m_fZoffset = 0;
    };

    inline FMyLocationOfZRotationAroundPointCoordinateCpp operator+(const FMyLocationOfZRotationAroundPointCoordinateCpp& R) const
    {
        return FMyLocationOfZRotationAroundPointCoordinateCpp(m_fRadiusOnXYPlane + R.m_fRadiusOnXYPlane, m_fYawOnXYPlane + R.m_fYawOnXYPlane, m_fZoffset + R.m_fZoffset);
    }

    inline FMyLocationOfZRotationAroundPointCoordinateCpp operator-(const FMyLocationOfZRotationAroundPointCoordinateCpp& R) const
    {
        return FMyLocationOfZRotationAroundPointCoordinateCpp(m_fRadiusOnXYPlane - R.m_fRadiusOnXYPlane, m_fYawOnXYPlane - R.m_fYawOnXYPlane, m_fZoffset - R.m_fZoffset);
    }

    inline FMyLocationOfZRotationAroundPointCoordinateCpp operator*(float& R) const
    {
        return FMyLocationOfZRotationAroundPointCoordinateCpp(m_fRadiusOnXYPlane * R, m_fYawOnXYPlane * R, m_fZoffset * R);
    }
    
    inline static bool equals(const FMyLocationOfZRotationAroundPointCoordinateCpp& a, const FMyLocationOfZRotationAroundPointCoordinateCpp& b, float tolerance)
    {
        FVector va, vb;
        va.X = a.m_fRadiusOnXYPlane;
        va.Y = a.m_fYawOnXYPlane;
        va.Z = a.m_fZoffset;

        vb.X = b.m_fRadiusOnXYPlane;
        vb.Y = b.m_fYawOnXYPlane;
        vb.Z = b.m_fZoffset;

        return va.Equals(vb, tolerance);
    };

    static void interp(const FMyLocationOfZRotationAroundPointCoordinateCpp& start, const FMyLocationOfZRotationAroundPointCoordinateCpp& end, float percent, FMyLocationOfZRotationAroundPointCoordinateCpp& result);

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "radius on XY Plane"))
    float m_fRadiusOnXYPlane;

    //in range of [0, 360), when used as output in API
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "yaw on XY Plane"))
    float m_fYawOnXYPlane; //Degrees

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "z offset"))
    float m_fZoffset;
};

USTRUCT(BlueprintType)
struct FMyTransformOfZRotationAroundPointCoordinateCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTransformOfZRotationAroundPointCoordinateCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_cLocation.reset();
        m_cRotatorOffsetFacingCenterPoint = FRotator::ZeroRotator;
    };

    inline FString ToString() const
    {
        return TEXT("[") + m_cLocation.ToString() + TEXT("(") + m_cRotatorOffsetFacingCenterPoint.ToString() + TEXT(")]");
    };

    inline static bool equals(const FMyTransformOfZRotationAroundPointCoordinateCpp& a, const FMyTransformOfZRotationAroundPointCoordinateCpp& b, float tolerance)
    {
        FVector ra = a.m_cRotatorOffsetFacingCenterPoint.Vector();
        FVector rb = a.m_cRotatorOffsetFacingCenterPoint.Vector();

        return FMyLocationOfZRotationAroundPointCoordinateCpp::equals(a.m_cLocation, b.m_cLocation, tolerance) && ra.Equals(rb, tolerance);
    };

    static void interp(const FMyTransformOfZRotationAroundPointCoordinateCpp& start, const FMyTransformOfZRotationAroundPointCoordinateCpp& end, float percent, FMyTransformOfZRotationAroundPointCoordinateCpp& result);

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "location of Z rotation coordinate"))
    FMyLocationOfZRotationAroundPointCoordinateCpp m_cLocation;

    //by default, the point should always facing the center point
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "rotator offset facing center point"))
    FRotator m_cRotatorOffsetFacingCenterPoint;
};

#define MyActorsInArrayNumMax (4096)

//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyCommonUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure)
        static inline MyErrorCodeCommonPartCpp getMyErrorCodeCommonPartNoError()
    {
        return MyErrorCodeCommonPartCpp::NoError;
    };

    //Return true if have error
    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToBool (MyErrorCodeCommonPartCpp)", CompactNodeTitle = "->", BlueprintAutocast))
        static inline bool Conv_MyErrorCodeCommonPartCpp_Bool(MyErrorCodeCommonPartCpp errorCode)
    {
        return errorCode != MyErrorCodeCommonPartCpp::NoError;
    };

    //ToString
    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (MyErrorCodeCommonPartCpp)", CompactNodeTitle = "->", BlueprintAutocast))
        static inline FString Conv_MyErrorCodeCommonPartCpp_String(MyErrorCodeCommonPartCpp errorCode)
    {
        return UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyErrorCodeCommonPartCpp"), (uint8)errorCode);
    };


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

    //return the number of actors created, return < 0 means fail
    template< class T >
    static int32 helperPrepareActorsInArray(const UObject* WorldContextObject, const TSubclassOf<T>& actorClass, TArray<T*>& managedActorArray, int32 count2reach, bool debugLog = false)
    {
        static_assert(std::is_base_of_v<AActor, T>, "type must be subclass of AActor.");
        static_assert(std::is_base_of_v<IMyIdInterfaceCpp, T>, "type must be subclass of IMyIdInterfaceCpp.");

        MY_VERIFY(count2reach >= 0);
        MY_VERIFY(count2reach < MyActorsInArrayNumMax);
        if (!IsValid(actorClass)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("actorClass is invalid: %p, type name %s."), actorClass.Get(), *T::StaticClass()->GetName());
            return -1;
        }

        double s0;
        if (debugLog) {
            s0 = FPlatformTime::Seconds();
        }

        int32 l = managedActorArray.Num();
        for (int32 i = (l - 1); i >= count2reach; i--) {
            T* pPoped = managedActorArray.Pop();
            pPoped->K2_DestroyActor();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        UWorld *w = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
        MY_VERIFY(IsValid(w));

        l = managedActorArray.Num();
        int32 countCreated = 0;
        for (int32 i = l; i < count2reach; i++) {
            //AMyMJGameCardActorBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardActorBaseCpp>(pCDO->StaticClass(), SpawnParams); //Warning: staticClass is not virtual class, so you can't get actual class
            T *pNewActor = w->SpawnActor<T>(actorClass, FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
            pNewActor->setMyId(i);
            //pNewCardActor->setResourcePathWithRet(m_cCfgCardResPath);

            MY_VERIFY(IsValid(pNewActor));
            pNewActor->SetActorHiddenInGame(true);
            MY_VERIFY(managedActorArray.Emplace(pNewActor) == i);
            countCreated++;
        }

        MY_VERIFY(managedActorArray.Num() == count2reach);

        if (debugLog) {
            double s1 = FPlatformTime::Seconds();
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("prepare actors in array done: class %s,  %d created, total now %d, time used %f."), *actorClass->GetName(), countCreated, count2reach, s1 - s0);
        }

        MY_VERIFY(countCreated >= 0); //we don't allow interger spin over

        return countCreated;
    };

    //may return NULL
    template< class T >
    static T* helperGetActorInArray(const TArray<T*>& managedActorArray, int32 idx)
    {
        static_assert(std::is_base_of_v<AActor, T>, "type must be subclass of AActor.");
        static_assert(std::is_base_of_v<IMyIdInterfaceCpp, T>, "type must be subclass of IMyIdInterfaceCpp.");

        MY_VERIFY(idx >= 0);

        if (idx < managedActorArray.Num()) {
            T* pRet = managedActorArray[idx];
            MY_VERIFY(pRet);
            MY_VERIFY(idx == pRet->getMyId2())
            return pRet;
        }
        else {
            return NULL;
        }
    };

    //never fail, core dump in that case. It will prepare the actor if idx out of range
    template< class T >
    static T* helperGetActorInArrayEnsured(const UObject* WorldContextObject, const TSubclassOf<T>& actorClass, TArray<T*>& managedActorArray, int32 idx, int32 maxCount = MyActorsInArrayNumMax, FString debugStr = TEXT(""), bool debugLog = false)
    {
        static_assert(std::is_base_of_v<AActor, T>, "type must be subclass of AActor.");
        static_assert(std::is_base_of_v<IMyIdInterfaceCpp, T>, "type must be subclass of IMyIdInterfaceCpp.");

        MY_VERIFY(idx >= 0);
        MY_VERIFY(idx < maxCount);

        if (idx >= managedActorArray.Num()) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("'%s': requiring instance not prepared before, existing %d, required idx %d, preparing them now."), *debugStr, managedActorArray.Num(), idx);
            helperPrepareActorsInArray<T>(WorldContextObject, actorClass, managedActorArray, idx + 1, debugLog);
        }

        T* pRet = helperGetActorInArray(managedActorArray, idx);

        MY_VERIFY(IsValid(pRet));

        return pRet;
    };


    template< class T >
    static inline bool isSubClassValidAndChild(const TSubclassOf<T> cClass, FString debugStr)
    {
        static_assert(std::is_base_of_v<UObject, T>, "type must be subclass of UObject.");

        if (!IsValid(cClass)) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: sub class is not valid."), *debugStr);
            return false;
        }

        if (cClass == T::StaticClass()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: sub class must be a child class, but the inputted one is parent itself."), *debugStr);
            return false;
        }

        return true;
    }

    //@RS must be inited before calling, you can init it with same seed to acheive same shuffer result before calling
    template< class T >
    static void shuffleArrayWithRandomStream(FRandomStream& RS, TArray<T>& targetArray)
    {
        int32 remainingCount = targetArray.Num();

        while (remainingCount > 1) {
            int32 idxSetting = remainingCount - 1;
            int32 idxPicked = RS.RandRange(0, idxSetting);

            if (idxSetting != idxPicked) {
                T pickedValue = targetArray[idxPicked];
                targetArray[idxPicked] = targetArray[idxSetting];
                targetArray[idxSetting] = pickedValue;
            }

            remainingCount--;
        }
    };


    static FString getStringFromEnum(const TCHAR *enumName, uint8 value);

    static int64 nowAsMsFromTick();

    static FString formatStrIds(const TArray<int32> &aIds);

    static FString formatStrIdsValues(const TArray<int32> &aIds, const TArray<int32> &aValues);

    static FString formatStrIdValuePairs(const TArray<FMyIdValuePair> &aIdValues);

    static FString formatMaskString(int32 iMask, uint32 uBitsCount);

    static void convertIdValuePairs2Ids(const TArray<FMyIdValuePair> &aIdValues, TArray<int32> &outaValues);


    UFUNCTION(BlueprintCallable)
    static int32 getEngineNetMode(AActor *actor);

    inline
        static FString genTimeStrFromTimeMs(uint32 uiTime_ms)
    {
        return FString::Printf(TEXT("%u.%03u"), uiTime_ms / 1000, uiTime_ms % 1000);
    };

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static FRotator fixRotatorValuesIfGimbalLock(const FRotator& rotator, float PitchDeltaTolerance = 0.01);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static void rotateOriginWithPivot(const FTransform& originCurrentTransformWorld3D, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultTransformWorld3D);

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
    static void playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(const UObject* WorldContextObject, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, float &DistanceFromCamera, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void helperResolveTransformWorld3DFromPlayerCameraByAbsolute(const UObject* WorldContextObject, FVector2D ConstrainedPosiAbsoluteInCamera, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void helperResolveTransformWorld3DFromPlayerCameraByPercent(const UObject* WorldContextObject, FVector2D ConstrainedPosiPercentInCamera, float ConstrainedVLengthPercentInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection);

    //following is bond to project resource setup
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (UnsafeDuringActorConstruction = "true"))
    static UCurveVector* getCurveVectorByType(MyCurveAssetType curveType = MyCurveAssetType::DefaultLinear);

    //always succeed. never return NULL
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (UnsafeDuringActorConstruction = "true"))
    static UCurveVector* getCurveVectorFromSettings(const FMyCurveVectorSettingsCpp& settings);


    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static void calcPointTransformWithLocalOffset(const FTransform& pointTransform, FVector localOffset, FTransform& pointTransformFixed);
    
    //round related calc API, begin
    //note scale3D component in transform, is ignored

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (MyTransformZRotation)", CompactNodeTitle = "->", BlueprintAutocast), Category = "UMyCommonUtilsLibrary")
    static FString Conv_MyTransformZRotation_String(const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static void MyTransformZRotationToTransformWorld(const FTransform& centerPointTransformWorld, const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation, FTransform& transformWorld);
    
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static void TransformWorldToMyTransformZRotation(const FTransform& centerPointTransformWorld, const FTransform& transformWorld, FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation);

    /*
    local X Y as blow, UE4's coordinate style:
    
             X
          \ /
      |    /
      |   / \
      |  /   Y
      |d/
      |/
    center
    */
    //UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    //static void calcRingPointTransformAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, FTransform& ringPointTransform, FVector ringPointLocalOffset = FVector::ZeroVector, FRotator ringPointLocalRotator = FRotator(0, 180, 0));
    
    //offset's values are all linear delta
    //UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    //static void calcTransformWorld3DFromWorldOffsetAndDegreeForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, const FTransform& worldOffset, FTransform& TransformWorld3D,  FVector ringPointLocalOffset = FVector::ZeroVector, FRotator ringPointLocalRotator = FRotator(0, 180, 0));
    
    //offset's values are all linear delta
    //UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    //static void calcWorldOffsetAndDegreeFromTransformWorld3DForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, const FTransform& TransformWorld3D, float &degree, FTransform& worldOffset, FVector ringPointLocalOffset = FVector::ZeroVector, FRotator ringPointLocalRotator = FRotator(0, 180, 0));

    //round related calc API, end

protected:
    
    //static void calcRingPointTransformAroundCenterPointZAxisWithFixedData(const FTransform& centerPointTransform, float radiusFixed, float radiansFixed, FTransform& ringPointTransform, float ringPointLocalZOffset, const FRotator& ringPointLocalRotator);

    //return error code
    static int32 fixRadiusAndRadiansForLocalOffsetOn2DCycle(float radius, float xOffset, float yOffset, float &radiusFixed, float &radiansDelta);
};