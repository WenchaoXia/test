// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtils.h"
#include "MyCommonInterface.h"

#include "Components/TimelineComponent.h"
#include "GameFramework/MovementComponent.h"
#include "Components/TimelineComponent.h"

#include "MyCommonUtilsLibrary.generated.h"

//#define MY_ROTATOR_MIN_TOLERANCE (0.01f)
#define FMyWithCurveUpdateStepDataTransformCpp_Delta_Min (0.1f)
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



//Two ways to store * data in containor: allocator or virtual function.
//I don't have time to write a clear allocator, so for simple just use virtual functions now.

DECLARE_DELEGATE_TwoParams(FMyWithCurveUpdaterUpdateDelegate, const struct FMyWithCurveUpdateStepDataBasicCpp&, const FVector&);
DECLARE_DELEGATE_OneParam (FMyWithCurveUpdaterFinishDelegate, const struct FMyWithCurveUpdateStepDataBasicCpp&);


USTRUCT(BlueprintType)
struct FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepDataBasicCpp()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataBasicCpp");
        reset();
    };

    virtual ~FMyWithCurveUpdateStepDataBasicCpp()
    {

    };

    inline void reset()
    {
        m_fTime = 0;
        m_pCurve = NULL;

        m_cStepUpdateDelegate.Unbind();
        m_cStepFinishDelegete.Unbind();

        m_bSkipCommonUpdateDelegate = false;
        m_bSkipCommonFinishDelegate = false;
    };

    virtual bool checkValid() const
    {
        if (m_fTime <= 0) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("step time is too small: %f."), m_fTime);
            return false;
        }
        if (m_pCurve == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("curve not specifed."));
            return false;
        }

        return true;
    };

    //virtual function MUST be implemented by subclass start:

    //always succeed, better use compile time skill, but now use virtual function
    virtual FMyWithCurveUpdateStepDataBasicCpp* createOnHeap()
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("createOnHeap not implemented!"));
        return new FMyWithCurveUpdateStepDataBasicCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("copyContentFrom not implemented!"));
        *this = other;
    };

    //end

    inline bool isSameClass(const FMyWithCurveUpdateStepDataBasicCpp& other) const
    {
        if (m_sClassName == TEXT("FMyWithCurveUpdateStepDataBasicCpp")) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("isSameClass() is supposed to be called on child class instance and class name not changed!"));
        }
        return m_sClassName == other.m_sClassName;
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "time"))
    float m_fTime;

    //Warning:: we assume curve value from 0 - 1. if value out of range, that means final interp out of range, maybe desired in some case
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "curve"))
    UCurveVector* m_pCurve;

    //per-step delegate
    FMyWithCurveUpdaterUpdateDelegate m_cStepUpdateDelegate;
    FMyWithCurveUpdaterFinishDelegate m_cStepFinishDelegete;

    //common for any steps flag
    bool m_bSkipCommonUpdateDelegate;
    bool m_bSkipCommonFinishDelegate;


protected:

    //we don't have Runtime class info for struct, so manually implements one, MUST be changed for every subclass type
    FName m_sClassName;
};

USTRUCT()
struct FMyWithCurveUpdateStepDataItemCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataItemCpp(FMyWithCurveUpdateStepDataBasicCpp* pDataExternal = NULL)
    {
        if (pDataExternal) {
            m_pData = pDataExternal;
            m_bOwnData = false;
        }
        else {
            m_pData = NULL;
            m_bOwnData = true;
        }
    };

    virtual ~FMyWithCurveUpdateStepDataItemCpp()
    {
        clear();
    };

    inline void clear()
    {
        if (m_bOwnData && m_pData) {
            delete(m_pData);
        }

        m_bOwnData = true;
        m_pData = NULL;
    };

    inline FMyWithCurveUpdateStepDataBasicCpp* getData()
    {
        return m_pData;
    };

    inline const FMyWithCurveUpdateStepDataBasicCpp* getDataConst() const
    {
        return m_pData;
    };

    FMyWithCurveUpdateStepDataItemCpp& operator=(const FMyWithCurveUpdateStepDataItemCpp& rhs)
    {
        if (this == &rhs) {
            return *this;
        }

        if (rhs.m_pData) {
            if (m_bOwnData && m_pData && m_pData->isSameClass(*rhs.m_pData)) {
                //already allocated and same class, skip reallicate
            }
            else {
                clear();
                m_pData = rhs.m_pData->createOnHeap();
            }

            m_pData->copyContentFrom(*rhs.m_pData);
        }
        else {
            clear();
        }

        return *this;
    };

    //don't support comparation now
    /*
    bool operator==(const FMyWithCurveUpdateStepDataItemCpp& rhs) const
    {
        if (this == &rhs) {
            return true;
        }

        if (m_pDataOnHeap == rhs.m_pDataOnHeap) {
            return true;
        }
        else if (m_pDataOnHeap && rhs.m_pDataOnHeap) {
            return (*m_pDataOnHeap) == (*rhs.m_pDataOnHeap);
        }
        else {
            return false;
        }
    }
    */

protected:

    FMyWithCurveUpdateStepDataBasicCpp* m_pData;
    bool m_bOwnData; //if true, this object manage the data and it is allocated on heap only
};


DECLARE_DELEGATE_TwoParams(FMyWithCurveUpdaterActivateTickDelegate, bool, FString);

USTRUCT()
struct FMyWithCurveUpdaterBasicCpp
{
    GENERATED_USTRUCT_BODY()
public:
    
    FMyWithCurveUpdaterBasicCpp();
    virtual ~FMyWithCurveUpdaterBasicCpp();

    inline void reset()
    {
        clearSteps();
    };

    void tick(float deltaTime);

    //return the internal idx new added, < 0 means fail, do NOT use it as peek idx, use getSeqCount() or getSeqLast() to ref the last added one
    //curve's value range is supposed to 0 - 1, out of that is also allowed, 0 means start, 1 means dest, time range is 0 - N. 
    int32 addStepToTail(const FMyWithCurveUpdateStepDataBasicCpp& data);

    //return the number removed
    int32 removeStepsFromHead(int32 iCount);

    //return the internal idx, < 0 means not exist, not it is slower since have struct data copy step
    int32 peekStepAt(int32 idxFromHead, const FMyWithCurveUpdateStepDataBasicCpp*& poutData) const;

    int32 peekStepLast(const FMyWithCurveUpdateStepDataBasicCpp*& poutData) const;

    int32 getStepsCount() const;

    void clearSteps();

    FMyWithCurveUpdaterUpdateDelegate m_cCommonUpdateDelegate;
    FMyWithCurveUpdaterFinishDelegate m_cCommonFinishDelegete;

    FMyWithCurveUpdaterActivateTickDelegate m_cActivateTickDelegate;

protected:

private:
    //return true if new play started
    bool tryStartNextStep(FString sDebugReason);

    void onTimeLineUpdated(FVector vector);
    void onTimeLineFinished();

    void finishOneStep(const FMyWithCurveUpdateStepDataBasicCpp& stepData);

    FTimeline m_cTimeLine;

    FOnTimelineVectorStatic m_cTimeLineVectorDelegate;
    FOnTimelineEventStatic  m_cTimeLineFinishEventDelegate;

    //we don't need external storage, since template can't be used as USTRUCT, so can't directly serialize
    //TArray<FMyWithCurveUpdateStepDataItemCpp> m_aStepDataItemsStorage;
    //FMyCycleBufferMetaDataCpp m_cStepDataItemsMeta;

    //this is the manager, but using @m_aStepDataItemsStorage and @m_cStepDataItemsMeta as storage to allow UProperty work properly, allow serialization
    FMyCycleBuffer<FMyWithCurveUpdateStepDataItemCpp> m_cStepDataItemsCycleBuffer;

    float m_fDebugTimePassedForOneStep;
};

template <typename T>
struct FMyWithCurveUpdaterTemplateCpp : public FMyWithCurveUpdaterBasicCpp
{
    static_assert(std::is_base_of_v<FMyWithCurveUpdateStepDataBasicCpp, T>, "type must be subclass of FMyWithCurveUpdateStepDataBasicCpp.");

public:

    FMyWithCurveUpdaterTemplateCpp() : FMyWithCurveUpdaterBasicCpp()
    {

    };

    virtual ~FMyWithCurveUpdaterTemplateCpp()
    {

    };

    //return the internal idx, < 0 means not exist, not it is slower since have struct data copy step
    inline int32 peekStepAt(int32 idxFromHead, const T*& poutData) const
    {
        const FMyWithCurveUpdateStepDataBasicCpp* pD = NULL;
        int32 ret = FMyWithCurveUpdaterBasicCpp::peekStepAt(idxFromHead, pD);

        if (pD) {
            poutData = StaticCast<const T*>(pD);
        }
        else {
            poutData = NULL;
        }
        return ret;
    };

    int32 peekStepLast(const T*& poutData) const
    {
        const FMyWithCurveUpdateStepDataBasicCpp* pD = NULL;
        int32 ret = FMyWithCurveUpdaterBasicCpp::peekStepLast(pD);

        if (pD) {
            poutData = StaticCast<const T*>(pD);
        }
        else {
            poutData = NULL;
        }
        return ret;
    };
};


USTRUCT(BlueprintType)
struct FMyWithCurveUpdateStepDataTransformCpp : public FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataTransformCpp() : Super()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformCpp");
        reset(true);
    };

    virtual ~FMyWithCurveUpdateStepDataTransformCpp()
    {
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        FTransform tempT;
        m_cStart = m_cEnd = tempT;
        m_cLocalRotatorExtra = FRotator::ZeroRotator;

        m_bLocationEnabledCache = false;
        m_bRotatorBasicEnabledCache = false;
        m_bRotatorExtraEnabledCache = false;
        m_bScaleEnabledCache = false;
    };


    virtual FMyWithCurveUpdateStepDataBasicCpp* createOnHeap() override
    {
        return new FMyWithCurveUpdateStepDataTransformCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataTransformCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataTransformCpp *>(&other);
        *this = *pOther;
    };


    void helperSetDataBySrcAndDst(float fTime, UCurveVector* pCurve, const FTransform& cStart, const FTransform& cEnd, FIntVector extraRotateCycle = FIntVector::ZeroValue);

    //Warning:: because Rotator pitch in UE4, can't exceed +- 90D any where, so we can't store rotaion delta, but indirectly store the start and end
    FTransform m_cStart;
    FTransform m_cEnd;

    FRotator m_cLocalRotatorExtra;

    //fast flag to avoid float calc
    bool m_bLocationEnabledCache;
    bool m_bRotatorBasicEnabledCache;
    bool m_bRotatorExtraEnabledCache;
    bool m_bScaleEnabledCache;
};



struct FMyWithCurveUpdaterTransformCpp : public FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataTransformCpp>
{

public:
    FMyWithCurveUpdaterTransformCpp() : FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataTransformCpp>()
    {

    };

    virtual ~FMyWithCurveUpdaterTransformCpp()
    {

    };

    inline void reset()
    {
        FMyWithCurveUpdaterBasicCpp::reset();

        FTransform t;
        m_cHelperTransformOrigin = m_cHelperTransformFinal = m_cHelperTransformGroupPoint = t;
    };

    //return the last sequence's transform, if not return the origin(start point, for example actor's transform in the beginning)
    inline
    const FTransform& getHelperTransformPrevRefConst() const
    {
        int32 l = getStepsCount();
        if (l > 0) {
            const FMyWithCurveUpdateStepDataTransformCpp *pLast = NULL;
            peekStepLast(pLast);
            MY_VERIFY(pLast);
            return pLast->m_cEnd;
        }

        return m_cHelperTransformOrigin;
    }

    inline
        void setHelperTransformOrigin(const FTransform& cHelperTransformOrigin)
    {
        m_cHelperTransformOrigin = cHelperTransformOrigin;
    };

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

    inline
        const FTransform& getHelperTransformGroupPointRefConst() const
    {
        return m_cHelperTransformGroupPoint;
    };

    inline
        void setHelperTransformGroupPoint(const FTransform& cHelperTransformGroupPoint)
    {
        m_cHelperTransformGroupPoint = cHelperTransformGroupPoint;
    };


protected:

    //Helper data when calculating the multiple animation steps
    FTransform m_cHelperTransformOrigin;
    FTransform m_cHelperTransformFinal;
    FTransform m_cHelperTransformGroupPoint;
};

//UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent), HideCategories = Velocity)
//class ENGINE_API UInterpToMovementComponent : public UMovementComponent

/**
* make the actor move accroding to preset sequences
*
*/
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class MYONLINECARDGAME_API UMyTransformUpdaterComponent : public UMovementComponent
{
    GENERATED_BODY()

public:
    UMyTransformUpdaterComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //following blueprint functions mainly used for test
    UFUNCTION(BlueprintCallable)
    static void helperSetUpdateStepDataTransformBySrcAndDst(FMyWithCurveUpdateStepDataTransformCpp& data, float time, UCurveVector* curve, const FTransform& start, const FTransform& end, FIntVector extraRotateCycle)
    {
        data.helperSetDataBySrcAndDst(time, curve, start, end, extraRotateCycle);
    };

    UFUNCTION(BlueprintCallable)
    int32 updaterAddStepToTail(const FMyWithCurveUpdateStepDataTransformCpp& data)
    {
        return m_cUpdater.addStepToTail(data);
    };

    UFUNCTION(BlueprintCallable)
    void updaterClearSteps()
    {
        m_cUpdater.clearSteps();
    }


    inline FMyWithCurveUpdaterTransformCpp& getMyWithCurveUpdaterTransformRefRef()
    {
        return m_cUpdater;
    };

    inline bool getShowWhenActivated() const
    {
        return m_bShowWhenActivated;
    };

    void setShowWhenActivated(bool bShowWhenActivated)
    {
        m_bShowWhenActivated = bShowWhenActivated;
    };

    inline bool getHideWhenInactivated() const
    {
        return m_bHideWhenInactivated;
    };

    void setHideWhenInactivated(bool bHideWhenInactivated)
    {
        m_bHideWhenInactivated = bHideWhenInactivated;
    };

protected:

    //Begin UActorComponent Interface
    //virtual void BeginPlay() override;
    //virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
    //End UActorComponent Interface


    void updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector);
    void updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data);
    void updaterActivateTick(bool activate, FString debugString);

    FMyWithCurveUpdaterTransformCpp m_cUpdater;

    bool m_bShowWhenActivated;
    bool m_bHideWhenInactivated;
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

    inline void reset()
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

    inline void reset()
    {
        Super::reset();

        m_cBoxExtend = FVector(1);
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "box extend final"))
    FVector m_cBoxExtend;

};

UENUM()
enum class MyActorTransformUpdateAnimationLocationType : uint8
{
    PrevLocation = 0, //equal no change, previous step's location
    FinalLocation = 1, //where it should go at the end of animation
    OffsetFromPrevLocation = 10,
    OffsetFromFinalLocation = 11,
    PointOnPlayerScreen = 50, //one point specified on player screen, multiple actors will have their relative location same as finals
    DisappearAtAttenderBorderOnPlayerScreen = 51, //follow the line from center to attender point, the position make it out of player's screen, multiple actors will have their relative location same as finals
    OffsetFromGroupPoint = 100, //special one that goto the group point, and the actor should specify it in the component's data by calling its API, which means game visual code should arrange it
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
        m_bTimePecentTotalExpectedNot100Pecent = false;

        m_eLocationUpdateType = MyActorTransformUpdateAnimationLocationType::PrevLocation;
        m_cLocationOffsetPercent = FVector::ZeroVector;

        m_eRotationUpdateType = MyActorTransformUpdateAnimationRotationType::PrevRotation;
        m_cRotationUpdateExtraCycles = FIntVector::ZeroValue;

        m_eScaleUpdateType = MyActorTransformUpdateAnimationScaleType::PrevScale;
        m_cScaleSpecified = FVector(1, 1, 1);

        m_cCurve.reset();

    };

    //how many time used in total
    UPROPERTY(EditAnywhere, meta = (DisplayName = "time percent"))
    float m_fTimePercent;

    //is the total number not expected to be 100%
    UPROPERTY(EditAnywhere, meta = (DisplayName = "time percent total expected not 100 percent"))
    bool m_bTimePecentTotalExpectedNot100Pecent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "location update type"))
        MyActorTransformUpdateAnimationLocationType m_eLocationUpdateType;

    //By default the offset is in world space, unit is percent of model size, like FVector::Size()
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

    UPROPERTY(EditAnywhere, meta = (DisplayName = "curve"))
    FMyCurveVectorSettingsCpp m_cCurve;
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


    inline void reset()
    {
        m_fShowPosiFromCenterToBorderPercent = 0;
        m_cExtraOffsetScreenPercent = FVector2D::ZeroVector;
    };

    //position to show on player screen, unit is percent of distance from center to border
    UPROPERTY(EditAnywhere, meta = (DisplayName = "position from center to border percent"))
    float m_fShowPosiFromCenterToBorderPercent;

    //extra postion offset on player screen, unit is percent of player screen's width and height
    UPROPERTY(EditAnywhere, meta = (DisplayName = "extra offset percent"))
    FVector2D m_cExtraOffsetScreenPercent;
};

USTRUCT()
struct FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp : public FMyPointFromCenterInfoOnPlayerScreenConstrainedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp() : Super()
    {
        reset(true);
    };

    virtual ~FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp()
    {

    };

    inline void reset(bool bResetSubclassDataOnly = false)
    {
        if (!bResetSubclassDataOnly) {
            Super::reset();
        }
        m_fTargetVLengthOnScreenScreenPercent = 0.1;
    };

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
            //AMyMJGameCardBaseCpp *pNewCardActor = w->SpawnActor<AMyMJGameCardBaseCpp>(pCDO->StaticClass(), SpawnParams); //Warning: staticClass is not virtual class, so you can't get actual class
            T *pNewActor = w->SpawnActor<T>(actorClass, FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
            pNewActor->setMyId(i);
            //pNewCardActor->setResPathWithRet(m_cCfgCardResPath);

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
            MY_VERIFY(idx == pRet->getMyId())
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
    static UCurveVector* getCurveVectorByType(MyCurveAssetType curveType = MyCurveAssetType::DefaultLinear);

    //always succeed. never return NULL
    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary", meta = (UnsafeDuringActorConstruction = "true"))
    static UCurveVector* getCurveVectorFromSettings(const FMyCurveVectorSettingsCpp& settings);

    static void helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
                                                                                            float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                            const FVector2D& targetPosiFixOnScreenPercent,
                                                                                            float targetVOnScreenPercent,
                                                                                            float targetModelHeightInWorld,
                                                                                            FTransform &outTargetTranform);

    static float helperGetRemainTimePercent(const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas);

    static void helperSetupTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                        const FMyActorTransformUpdateAnimationStepCpp& stepData,
                                                        const TArray<FMyWithCurveUpdaterTransformCpp *>& updatersSorted);

    static void helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaCpp& meta,
                                                        const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                        const TArray<FMyWithCurveUpdaterTransformCpp *>& updatersSorted);



    static void helperSetupTransformUpdateAnimationStepsForPoint(const UObject* WorldContextObject,
                                                                float totalDur,
                                                                const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                const FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                const TArray<FMyActorTransformUpdateAnimationStepCpp>& stepDatas,
                                                                float extraDelayDur,
                                                                const TArray<IMyTransformUpdaterInterfaceCpp *>& updaterInterfaces,
                                                                FString debugName,
                                                                bool clearPrevSteps);

    //a step take 100% of time @waitTime, will be added
    static void helperAddWaitStep(float waitTime, FString debugStr, const TArray<FMyWithCurveUpdaterTransformCpp *>& updaters);
    static void helperAddWaitStep(float waitTime, FString debugStr, const TArray<IMyTransformUpdaterInterfaceCpp*>& updaterInterfaces);

    UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    static void calcPointTransformWithLocalOffset(const FTransform& pointTransform, FVector localOffset, FTransform& pointTransformFixed);
    
    //round related calc API, begin
    //note scale3D component in transform, is ignored

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (MyTransformZRotation)", CompactNodeTitle = "->", BlueprintAutocast), Category = "UMyCommonUtilsLibrary")
    static FString Conv_MyTransformZRotationToString(const FMyTransformOfZRotationAroundPointCoordinateCpp& myTransformZRotation);

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
    //static void calcWorldTransformFromWorldOffsetAndDegreeForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, float degree, const FTransform& worldOffset, FTransform& worldTransform,  FVector ringPointLocalOffset = FVector::ZeroVector, FRotator ringPointLocalRotator = FRotator(0, 180, 0));
    
    //offset's values are all linear delta
    //UFUNCTION(BlueprintCallable, Category = "UMyCommonUtilsLibrary")
    //static void calcWorldOffsetAndDegreeFromWorldTransformForRingPointAroundCenterPointZAxis(const FTransform& centerPointTransform, float ringRadius, const FTransform& worldTransform, float &degree, FTransform& worldOffset, FVector ringPointLocalOffset = FVector::ZeroVector, FRotator ringPointLocalRotator = FRotator(0, 180, 0));

    //round related calc API, end

protected:
    
    //static void calcRingPointTransformAroundCenterPointZAxisWithFixedData(const FTransform& centerPointTransform, float radiusFixed, float radiansFixed, FTransform& ringPointTransform, float ringPointLocalZOffset, const FRotator& ringPointLocalRotator);

    //return error code
    static int32 fixRadiusAndRadiansForLocalOffsetOn2DCycle(float radius, float xOffset, float yOffset, float &radiusFixed, float &radiansDelta);
};