// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtils.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/MovementComponent.h"
#include "Components/TimelineComponent.h"

#include "MyCommonUtilsLibrary.generated.h"

#define FTransformUpdateSequencDataCpp_Delta_Min (0.1f)
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


USTRUCT(BlueprintType)
struct FTransformUpdateSequencDataCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FTransformUpdateSequencDataCpp()
    {
        reset();
    };

    void reset()
    {
        FTransform tempT;
        m_cStart = m_cEnd = tempT;
        m_cLocalRotatorExtra = FRotator::ZeroRotator;

        m_bLocationEnabledCache = false;
        m_bRotatorBasicEnabledCache = false;
        m_bRotatorExtraEnabledCache = false;
        m_bScaleEnabledCache = false;

        m_fTime = 0;
    };

    void helperSetDataBySrcAndDst(const FTransform& cStart, const FTransform& cEnd, float fTime, FIntVector extraRotateCycle = FIntVector::ZeroValue);

    //Warning:: because Rotator pitch in UE4, can't exceed +- 90D any where, so we can't store rotaion delta, but indirectly store the start and end
    FTransform m_cStart;
    FTransform m_cEnd;

    FRotator m_cLocalRotatorExtra;

    //fast flag to avoid float calc
    bool m_bLocationEnabledCache;
    bool m_bRotatorBasicEnabledCache;
    bool m_bRotatorExtraEnabledCache;
    bool m_bScaleEnabledCache;

    float m_fTime;
};

//UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent), HideCategories = Velocity)
//class ENGINE_API UInterpToMovementComponent : public UMovementComponent

/**
* make the actor move accroding to preset sequences
*
*/
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class MYONLINECARDGAME_API UMyTransformUpdateSequenceMovementComponent : public UMovementComponent
{
    //GENERATED_UCLASS_BODY()
    GENERATED_BODY()

public:
    UMyTransformUpdateSequenceMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    //UMyCurveSeqMovementComponent();

    //return the internal idx new added, < 0 means fail, do NOT use it as peek idx, use getSeqCount() or getSeqLast() to ref the last added one
    //curve's value range is supposed to 0 - 1, out of that is also allowed, 0 means start, 1 means dest, time range is 0 - N. 
    UFUNCTION(BlueprintCallable)
        int32 addSeqToTail(const FTransformUpdateSequencDataCpp& data, UCurveVector* curve);

    //return the number removed
    UFUNCTION(BlueprintCallable)
        int32 removeSeqFromHead(int32 iCount);

    //return the internal idx, < 0 means not exist, not it is slower since have struct data copy step
    UFUNCTION(BlueprintCallable)
        int32 peekSeqAt(int32 idxFromHead, FTransformUpdateSequencDataCpp& outData, UCurveVector*& outCurve) const;

    int32 peekSeqAtCpp(int32 idxFromHead, const FTransformUpdateSequencDataCpp*& poutData, UCurveVector*& outCurve) const;

    //UFUNCTION(BlueprintCallable)
    //const UCurveVector* peekSeqAt2(int32 idxFromHead, FTransformUpdateSequencDataCpp& outData) const;

    UFUNCTION(BlueprintCallable)
        int32 peekSeqLast(FTransformUpdateSequencDataCpp& outData, UCurveVector*& outCurve) const;

    UFUNCTION(BlueprintCallable)
        int32 getSeqCount() const;

    UFUNCTION(BlueprintCallable)
        void clearSeq();

    UFUNCTION(BlueprintCallable)
        static void helperSetDataBySrcAndDst(FTransformUpdateSequencDataCpp& data, const FTransform& cStart, const FTransform& cEnd, float fTime, FIntVector extraRotateCycle)
    {
        data.helperSetDataBySrcAndDst(cStart, cEnd, fTime, extraRotateCycle);
    };

    //return the last sequence's transform, if not return the updated component's transform
    const FTransform& getHelperTransformPrevRefConst() const;

    inline
        const FTransform& getHelperTransformFinalRefConst() const
    {
        return m_cHelperTransformFinal;
    };

    inline
        void setHelperTransformFinal(const FTransform& cHelperTransformFinal)
    {
        m_cHelperTransformFinal = cHelperTransformFinal;
    };

    inline void setHideWhenInactived(bool bNew)
    {
        m_bHideWhenInactived = bNew;
    };

    inline void setShowWhenActived(bool bNew)
    {
        m_bShowWhenActivated = bNew;
    };

protected:

    //Begin UActorComponent Interface
    //virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
    virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
    //End UActorComponent Interface

    //mainly used for debug and allow switch the implement quickly
    void setActivatedMyEncapped(bool bNew, FString reason = TEXT("none"));


    //return true if new play started
    bool tryStartNextSeq(FString sDebugReason);

    void onTimeLineUpdated(FVector vector);
    void onTimeLineFinished();

    UPROPERTY()
        FTimeline m_cTimeLine;

    FOnTimelineVectorStatic m_cTimeLineVectorDelegate;
    FOnTimelineEventStatic  m_cTimeLineFinishEventDelegate;

    UPROPERTY()
        TArray<FTransformUpdateSequencDataCpp> m_aDataItems;

    UPROPERTY()
        FMyCycleBufferMetaDataCpp m_cDataMeta;

    //Warning:: we assume curve value from 0 - 1. if value out of range, that means final interp out of range, maybe desired in some case
    UPROPERTY()
        TArray<UCurveVector *> m_aCurveItems;

    UPROPERTY()
        FMyCycleBufferMetaDataCpp m_cCurveMeta;

    FMyCycleBuffer<FTransformUpdateSequencDataCpp> m_cDataCycleBuffer;
    FMyCycleBuffer<UCurveVector*> m_cCurveCycleBuffer;

    //Helper data when calculating the multiple animation steps, this is the final transform
    FTransform m_cHelperTransformFinal;

    float m_fDebugTimeLineStartWorldTime;
    uint32 m_uDebugLastTickWorldTime_ms;

    bool m_bShowWhenActivated;
    bool m_bHideWhenInactived;
};


//model always and must facing x axis
USTRUCT(BlueprintType)
struct FMyActorModelInfoBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyActorModelInfoBaseCpp()
    {
        m_cCenterPointRelativeLocation = FVector(0);
    };

    virtual ~FMyActorModelInfoBaseCpp()
    {

    };

    void reset()
    {
        m_cCenterPointRelativeLocation = FVector(0);
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
        FVector m_cCenterPointRelativeLocation;
};

USTRUCT(BlueprintType)
struct FMyActorModelInfoBoxCpp : public FMyActorModelInfoBaseCpp

{
    GENERATED_USTRUCT_BODY()

public:
    FMyActorModelInfoBoxCpp() : Super()
    {
        m_cBoxExtend = FVector(1);
    };

    void reset()
    {
        Super::reset();

        m_cBoxExtend = FVector(1);
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "box extend final"))
        FVector m_cBoxExtend;

};

UINTERFACE(Blueprintable)
class UMyTransformUpdateSequenceInterface : public UInterface
{
    GENERATED_BODY()
};

class IMyTransformUpdateSequenceInterface
{
    GENERATED_BODY()

public:

    //return error code, 0 means OK
    UFUNCTION(meta = (CallableWithoutWorldContext))
    virtual int32 getModelInfo(FMyActorModelInfoBoxCpp& modelInfo, bool verify = true) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getModelInfo() must be implemented!"));
        MY_VERIFY(false);
        return -1;
    };

    UFUNCTION(meta = (CallableWithoutWorldContext))
    virtual UMyTransformUpdateSequenceMovementComponent* getTransformUpdateSequence(bool verify = true)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getTransformUpdateSequence() must be implemented!"));
        MY_VERIFY(false);
        return NULL;
    };

protected:

};

UENUM()
enum class MyActorTransformUpdateAnimationLocationType : uint8
{
    PrevLocation = 0, //equal no change, previous step's location
    FinalLocation = 1, //where it should go at the end of animation
    offsetFromPrevLocation = 10,
    offsetFromFinalLocation = 11,
    PointOnPlayerScreen = 50, //one point specified on player screen, multiple actors will have their relative location same as finals
    DisappearAtAttenderBorderOnPlayerScreen = 51, //follow the line from center to attender point, the position make it out of player's screen, multiple actors will have their relative location same as finals
};

UENUM()
enum class MyActorTransformUpdateAnimationRotationType : uint8
{
    PrevRotation = 0, //equal to no change, previous step's rotation
    FinalRotation = 1, //where it should go at the end of animation

    FacingPlayerScreen = 50,
};

UENUM()
enum class MyActorTransformUpdateAnimationScaleType : uint8
{
    PrevScale = 0, //equal to no change, previous step's
    FinalScale = 1, //where it should go at the end of animation

    Specified = 50,
};

USTRUCT()
struct FMyActorTransformUpdateAnimationStepCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyActorTransformUpdateAnimationStepCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_fTimePercent = 0;

        m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PrevLocation;
        m_cLocationOffsetPercent = FVector::ZeroVector;

        m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::PrevRotation;
        m_cRotationUpdateExtraCycles = FIntVector::ZeroValue;

        m_eScaleUpdateType = MyActorTransformUpdateAnimationScaleType::PrevScale;
        m_cScaleSpecified = FVector(1, 1, 1);

        m_pCurve = NULL;
    };

    //how many time used in total
    UPROPERTY(EditAnywhere, meta = (DisplayName = "time percent"))
        float m_fTimePercent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "location update type"))
        MyActorTransformUpdateAnimationLocationType m_eLocationUpdateType;

    //By default the offset is in world space
    UPROPERTY(EditAnywhere, meta = (DisplayName = "location offset percent"))
        FVector m_cLocationOffsetPercent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "rotation update type"))
        MyActorTransformUpdateAnimationRotationType m_eRotationUpdateType;

    //how many extra 360 degrees it should animation
    UPROPERTY(EditAnywhere, meta = (DisplayName = "rotation update extra cycles"))
        FIntVector m_cRotationUpdateExtraCycles;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "scale update type"))
        MyActorTransformUpdateAnimationScaleType m_eScaleUpdateType;

    //only used when scale update type is specified
    UPROPERTY(EditAnywhere, meta = (DisplayName = "scale specified"))
        FVector m_cScaleSpecified;

    //if not specified, default linear one will be used
    UPROPERTY(EditAnywhere, meta = (DisplayName = "curve to use"))
        UCurveVector* m_pCurve;
};

USTRUCT()
struct FMyCenterMetaOnPlayerScreenConstrainedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyCenterMetaOnPlayerScreenConstrainedCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_cScreenCenterMapped = FVector::ZeroVector;
    }

    FVector m_cScreenCenterMapped; //Z is 0
};

USTRUCT()
struct FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp : public FMyCenterMetaOnPlayerScreenConstrainedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp() : Super()
    {
        reset();
    };

    virtual ~FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp()
    {

    };

    inline void reset()
    {
        Super::reset();

        m_cScreenPointMapped = m_cDirectionCenterToPointMapped = FVector::ZeroVector;
        m_fCenterToPointLength = m_fCenterToPointUntilBorderLength = 0;
    };

    FVector m_cScreenPointMapped; //Z is 0

    FVector m_cDirectionCenterToPointMapped; //Z is 0
    float m_fCenterToPointLength;
    float m_fCenterToPointUntilBorderLength;
};

USTRUCT()
struct FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp()
    {
        reset();
    };

    virtual ~FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp()
    {

    };

    inline void reset()
    {
        m_fShowPosiFromCenterToBorderPercent = 0;
        m_cExtraOffsetScreenPercent = FVector2D::ZeroVector;
        m_fTargetVLengthOnScreenScreenPercent = 0.1;
    };

    //position to show on player screen, unit is percent of distance from center to border
    UPROPERTY(EditAnywhere, meta = (DisplayName = "position from center to border percent"))
        float m_fShowPosiFromCenterToBorderPercent;

    //extra postion offset on player screen, unit is percent of player screen's width and height
    UPROPERTY(EditAnywhere, meta = (DisplayName = "extra offset percent"))
        FVector2D m_cExtraOffsetScreenPercent;

    //the length to display on player screen, unit is percent of player screen's height
    UPROPERTY(EditAnywhere, meta = (DisplayName = "target height percent"))
        float m_fTargetVLengthOnScreenScreenPercent;
};


USTRUCT()
struct FMyTransformUpdateAnimationMetaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTransformUpdateAnimationMetaCpp()
    {
        reset();
    };

    inline void reset()
    {
        FTransform zeroT;

        m_fTotalTime = 0;
        m_cPointTransform = zeroT;

        m_cDisappearTransform = zeroT;
        m_cModelBoxExtend = FVector::ZeroVector;
    };

    float m_fTotalTime = 0;
    FTransform m_cPointTransform;
    FTransform m_cDisappearTransform;
    FVector m_cModelBoxExtend;
    FString m_sDebugString;
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
    static void playerScreenConstrainedVLengthAbsoluteToDistanceFromCamera(const UObject* WorldContextObject, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, float &DistanceFromCamera, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void helperResolveWorldTransformFromPlayerCameraByAbsolute(const UObject* WorldContextObject, FVector2D ConstrainedPosiAbsoluteInCamera, float ConstrainedVLengthAbsoluteInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static void helperResolveWorldTransformFromPlayerCameraByPercent(const UObject* WorldContextObject, FVector2D ConstrainedPosiPercentInCamera, float ConstrainedVLengthPercentInCamera, float ModelInWorldHeight, FTransform& ResultTransform, FVector &CameraCenterWorldPosition, FVector &CameraCenterDirection);

    //following is bond to project resource setup
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (UnsafeDuringActorConstruction = "true"))
    static UCurveVector* getCurveVectorDefaultLinear();

    static void helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
                                                                                            float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                            const FVector2D& targetPosiFixOnScreenPercent,
                                                                                            float targetVOnScreenPercent,
                                                                                            float targetModelHeightInWorld,
                                                                                            FTransform &outTargetTranform);

    static float helperGetRemainTimePercent(const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas);

    static void helperSetupTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                        const FMyActorTransformUpdateAnimationStepCpp& stepData,
                                                        const TArray<UMyTransformUpdateSequenceMovementComponent *>& actorComponentsSortedGroup);

    static void helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                        const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                        const TArray<UMyTransformUpdateSequenceMovementComponent *>& actorComponentsSortedGroup);



    //Todo:: use interface
    //
    static void helperSetupTransformUpdateAnimationStepsForPoint(const UObject* WorldContextObject,
                                                                float totalDur,
                                                                const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                const FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                                float extraDelayDur,
                                                                const TArray<IMyTransformUpdateSequenceInterface*>& actorsInterfaces,
                                                                FString debugName,
                                                                bool clearSeq);

    static void helperAddWaitStep(float waitTime, FString debugStr, const TArray<UMyTransformUpdateSequenceMovementComponent*>& comps);
    static void helperAddWaitStep(float waitTime, FString debugStr, const TArray<IMyTransformUpdateSequenceInterface*>& interfaces);
};