// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyRenderInterface.h"

#include "RenderUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Blueprint/UserWidget.h"

#include "MyRenderUtilsLibrary.generated.h"

#define MyTransformTypeTransformWorld3D (MyTypeUnknown + 1)
#define MyTransformTypeTransformWidget2D (MyTypeUnknown + 2)

#define FMyWithCurveUpdateStepDataTransformWorld3DCpp_Delta_Min (0.1f)


//Two ways to store * data in containor: allocator or virtual function.
//I don't have time to write a clear allocator, so for simple just use virtual functions now.

DECLARE_DELEGATE_TwoParams(FMyWithCurveUpdaterUpdateDelegate, const struct FMyWithCurveUpdateStepDataBasicCpp&, const FVector&);
DECLARE_DELEGATE_OneParam(FMyWithCurveUpdaterFinishDelegate, const struct FMyWithCurveUpdateStepDataBasicCpp&);


USTRUCT(BlueprintType)
struct FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepDataBasicCpp()
    {
        reset();

        m_sClassName = TEXT("FMyWithCurveUpdateStepDataBasicCpp");
        m_iType = MyTypeUnknown;
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
        if (m_sClassName == TEXT("FMyWithCurveUpdateStepDataBasicCpp") || m_iType == MyTypeUnknown) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("isSameClass() is supposed to be called on child class instance and class name not changed, %s: %d.!"), *m_sClassName.ToString(), m_iType);
        }
        return m_sClassName == other.m_sClassName && m_iType == other.m_iType;
    };

    inline int32 getType() const
    {
        return m_iType;
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

    int32 m_iType;
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

    FMyWithCurveUpdateStepDataBasicCpp * m_pData;
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

    inline int32 getType() const
    {
        return m_iType;
    };

protected:

    //this member help user to do type cast
    int32 m_iType;

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
struct FMyWithCurveUpdateStepDataTransformWorld3DCpp : public FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataTransformWorld3DCpp() : Super()
    {
        reset(true);

        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformWorld3DCpp");
        m_iType = MyTransformTypeTransformWorld3D;
    };

    virtual ~FMyWithCurveUpdateStepDataTransformWorld3DCpp()
    {
    };

    /*
    inline static const FMyWithCurveUpdateStepDataTransformWorld3DCpp& castFromBaseRefConst(const FMyWithCurveUpdateStepDataBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeTransformWorld3D) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepDataTransformWorld3DCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepDataTransformWorld3DCpp& castFromBaseRef(FMyWithCurveUpdateStepDataBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepDataTransformWorld3DCpp&>(castFromBaseRefConst(base));
    };
    */

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
        return new FMyWithCurveUpdateStepDataTransformWorld3DCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataTransformWorld3DCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataTransformWorld3DCpp *>(&other);
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


struct FMyWithCurveUpdaterTransformWorld3DCpp : public FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataTransformWorld3DCpp>
{

public:
    FMyWithCurveUpdaterTransformWorld3DCpp() : FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataTransformWorld3DCpp>()
    {
        m_iType = MyTransformTypeTransformWorld3D;
    };

    virtual ~FMyWithCurveUpdaterTransformWorld3DCpp()
    {

    };

    /*
    inline static const FMyWithCurveUpdaterTransformWorld3DCpp& castFromBaseRefConst(const FMyWithCurveUpdaterBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeTransformWorld3D) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdaterTransformWorld3DCpp&>(base);
    };

    inline static FMyWithCurveUpdaterTransformWorld3DCpp& castFromBaseRef(FMyWithCurveUpdaterBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdaterTransformWorld3DCpp&>(castFromBaseRefConst(base));
    };
    */

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
            const FMyWithCurveUpdateStepDataTransformWorld3DCpp *pLast = NULL;
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
class MYONLINECARDGAME_API UMyWithCurveUpdaterTransformWorld3DComponent : public UMovementComponent
{
    GENERATED_BODY()

public:
    UMyWithCurveUpdaterTransformWorld3DComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual ~UMyWithCurveUpdaterTransformWorld3DComponent()
    {

    };

    //following blueprint functions mainly used for test
    UFUNCTION(BlueprintCallable)
        static void helperSetUpdateStepDataTransformBySrcAndDst(FMyWithCurveUpdateStepDataTransformWorld3DCpp& data, float time, UCurveVector* curve, const FTransform& start, const FTransform& end, FIntVector extraRotateCycle)
    {
        data.helperSetDataBySrcAndDst(time, curve, start, end, extraRotateCycle);
    };

    UFUNCTION(BlueprintCallable)
        int32 updaterAddStepToTail(const FMyWithCurveUpdateStepDataTransformWorld3DCpp& data)
    {
        return m_cUpdater.addStepToTail(data);
    };

    UFUNCTION(BlueprintCallable)
        void updaterClearSteps()
    {
        m_cUpdater.clearSteps();
    }


    inline FMyWithCurveUpdaterTransformWorld3DCpp& getMyWithCurveUpdaterTransformWorld3DRef()
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

    FMyWithCurveUpdaterTransformWorld3DCpp m_cUpdater;

    bool m_bShowWhenActivated;
    bool m_bHideWhenInactivated;
};

//Note: this can be split into actor and boxlike actor two classes, but now we only need one
//most functions are implemented in C++, so fast and free to call
UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyWithCurveUpdaterTransformWorld3DBoxLikeActorBaseCpp : public AActor, public IMyWithCurveUpdaterTransformWorld3DInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyWithCurveUpdaterTransformWorld3DBoxLikeActorBaseCpp();

    virtual ~AMyWithCurveUpdaterTransformWorld3DBoxLikeActorBaseCpp();

    //BP should only use it for test purpose
    virtual MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoWorld3DCpp& modelInfo, bool verify) const override;
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformWorld3DEnsured(struct FMyWithCurveUpdaterTransformWorld3DCpp*& outUpdater) override;


    inline UMyWithCurveUpdaterTransformWorld3DComponent* getMyTransformUpdaterComponent() const
    {
        return m_pMyTransformUpdaterComponent;
    };


    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
        static void helperTestAnimationStep(float time, FString debugStr, const TArray<AMyWithCurveUpdaterTransformWorld3DBoxLikeActorBaseCpp*>& actors);

protected:

    //virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    //return errcode if got wrong, 0 if OK, here it just adjust boxExtend to cover the mesh exactly, and box will have zero relative location
    virtual MyErrorCodeCommonPartCpp updateSettings();

    //components
    //root scene
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
        class USceneComponent *m_pRootScene;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "main box"))
        class UBoxComponent *m_pMainBox;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "main static mesh"))
        class UStaticMeshComponent *m_pMainStaticMesh;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, meta = (DisplayName = "my transform updater component"))
        UMyWithCurveUpdaterTransformWorld3DComponent* m_pMyTransformUpdaterComponent;

    UPROPERTY(EditDefaultsOnly, Category = "My Helper", meta = (DisplayName = "fake button to update settings"))
        bool m_bFakeUpdateSettings;

private:
    void createComponentsForCDO();
};


USTRUCT(BlueprintType)
struct FMyWithCurveUpdateStepDataTransformWidget2DCpp : public FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataTransformWidget2DCpp() : Super()
    {
        reset(true);

        m_sClassName = TEXT("FMyWithCurveUpdateStepDataTransformWidget2DCpp");
        m_iType = MyTransformTypeTransformWidget2D;
    };

    virtual ~FMyWithCurveUpdateStepDataTransformWidget2DCpp()
    {
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        FWidgetTransform t;
        m_cTransformWidget2DStart = m_cTransformWidget2DEnd = t;
    };

    virtual FMyWithCurveUpdateStepDataTransformWidget2DCpp* createOnHeap() override
    {
        return new FMyWithCurveUpdateStepDataTransformWidget2DCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataTransformWidget2DCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataTransformWidget2DCpp *>(&other);
        *this = *pOther;
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform start"))
        FWidgetTransform m_cTransformWidget2DStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform end"))
        FWidgetTransform m_cTransformWidget2DEnd;
};


struct FMyWithCurveUpdaterTransformWidget2DCpp : public FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataTransformWidget2DCpp>
{

public:
    FMyWithCurveUpdaterTransformWidget2DCpp() : FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataTransformWidget2DCpp>()
    {
        m_iType = MyTransformTypeTransformWidget2D;
    };

    virtual ~FMyWithCurveUpdaterTransformWidget2DCpp()
    {

    };

    /*
    inline static const FMyWithCurveUpdaterTransformWidget2DCpp& castFromBaseRefConst(const FMyWithCurveUpdaterBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeTransformWidget2D) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdaterTransformWidget2DCpp&>(base);
    };

    inline static FMyWithCurveUpdaterTransformWidget2DCpp& castFromBaseRef(FMyWithCurveUpdaterBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdaterTransformWidget2DCpp&>(castFromBaseRefConst(base));
    };
    */

    inline void reset()
    {
        FMyWithCurveUpdaterBasicCpp::reset();
    };
};


//a widget support animation setup at runtime, since default widget's animation is static
//Warn: Center is always zero, which means instances's rotate pivot should always ne center and it's zero transition always making center at canvas position zero
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp : public UUserWidget, public IMyWithCurveUpdaterTransformWidget2DInterfaceCpp, public IMySizeWidget2DInterfaceCpp
{
    GENERATED_BODY()

public:

    UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_cUpdater.m_cCommonUpdateDelegate.BindUObject(this, &UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp::updaterOnCommonUpdate);
        m_cUpdater.m_cCommonFinishDelegete.BindUObject(this, &UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp::updaterOnCommonFinish);
        m_cUpdater.m_cActivateTickDelegate.BindUObject(this, &UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp::updaterActivateTick);

        m_bUpdaterNeedTick = false;

        m_bShowWhenActivated = false;
        m_bHideWhenInactivated = false;
    };

    virtual ~UMyWithCurveUpdaterTransformWidget2DBoxLikeWidgetBaseCpp()
    {

    };

    virtual MyErrorCodeCommonPartCpp getLocalSizeFromCache_Implementation(FVector2D& localSize) override final
    {
        MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;
        if (!m_cMySizeWidget2DInterfaceCachedData.m_bValid) {
            m_cMySizeWidget2DInterfaceCachedData.reset();
            ret = IMySizeWidget2DInterfaceCpp::Execute_getLocalSize(this, m_cMySizeWidget2DInterfaceCachedData.m_cLocalSize);
            m_cMySizeWidget2DInterfaceCachedData.m_bValid = ret == MyErrorCodeCommonPartCpp::NoError;
        }
        localSize = m_cMySizeWidget2DInterfaceCachedData.m_cLocalSize;
        return ret;
    };

    virtual MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoWidget2DCpp& modelInfo, bool verify) const override;

    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformWidget2DEnsured(struct FMyWithCurveUpdaterTransformWidget2DCpp*& outUpdater) override
    {
        outUpdater = &m_cUpdater;
        return MyErrorCodeCommonPartCpp::NoError;
    };



    UFUNCTION(BlueprintCallable)
        int32 updaterAddStepToTail(const FMyWithCurveUpdateStepDataTransformWidget2DCpp& data)
    {
        return m_cUpdater.addStepToTail(data);
    };

    UFUNCTION(BlueprintCallable)
        void updaterClearSteps()
    {
        m_cUpdater.clearSteps();
    }

    inline FMyWithCurveUpdaterTransformWidget2DCpp& getUpdaterRef()
    {
        return m_cUpdater;
    };

protected:

    IMySizeWidget2DInterfaceCpp_DefaultEmptyImplementationForUObject_Bp()
    
    virtual void OnWidgetRebuilt() override
    {
        Super::OnWidgetRebuilt();

        invalidCachedData();
    };
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    void updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector);
    void updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data);
    void updaterActivateTick(bool activate, FString debugString);

    void invalidCachedData()
    {
        m_cMySizeWidget2DInterfaceCachedData.m_bValid = false;
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "show when activated"))
        bool m_bShowWhenActivated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "hide when Inactivated"))
        bool m_bHideWhenInactivated;

    FMyWithCurveUpdaterTransformWidget2DCpp m_cUpdater;
    bool m_bUpdaterNeedTick;

    FMySizeWidget2DInterfaceCachedDataCpp m_cMySizeWidget2DInterfaceCachedData;
};


UENUM()
enum class MyWithCurveUpdateStepSettingsLocationType : uint8
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
enum class MyWithCurveUpdateStepSettingsRotationType : uint8
{
    PrevRotation = 0, //equal to no change, previous step's rotation
    FinalRotation = 1, //where it should go at the end of animation

    FacingPlayerScreen = 50,
};

UENUM()
enum class MyWithCurveUpdateStepSettingsScaleType : uint8
{
    PrevScale = 0, //equal to no change, previous step's
    FinalScale = 1, //where it should go at the end of animation

    Specified = 50,
};


USTRUCT()
struct FMyWithCurveUpdateStepSettingsBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepSettingsBasicCpp()
    {
        reset();
        m_iType = MyTypeUnknown;
    };

    inline void reset()
    {
        m_fTimePercent = 0;
        m_bTimePecentTotalExpectedNot100Pecent = false;
        m_cCurve.reset();
    };

    inline int32 getType() const
    {
        return m_iType;
    };

    //how many time used in total
    UPROPERTY(EditAnywhere, meta = (DisplayName = "time percent"))
        float m_fTimePercent;

    //is the total number not expected to be 100%
    UPROPERTY(EditAnywhere, meta = (DisplayName = "time percent total expected not 100 percent"))
        bool m_bTimePecentTotalExpectedNot100Pecent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "curve"))
        FMyCurveVectorSettingsCpp m_cCurve;

protected:

    UPROPERTY(meta = (DisplayName = "type"))
    int32 m_iType;
};

USTRUCT()
struct FMyWithCurveUpdateStepSettingsTransformWorld3DCpp : public FMyWithCurveUpdateStepSettingsBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepSettingsTransformWorld3DCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeTransformWorld3D;
    };

    /*
    inline static const FMyWithCurveUpdateStepSettingsTransformWorld3DCpp& castFromBaseConst(const FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeTransformWorld3D) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepSettingsTransformWorld3DCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepSettingsTransformWorld3DCpp& castFromBase(FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp&>(castFromBaseConst(base));
    };
    */

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        m_eLocationUpdateType = MyWithCurveUpdateStepSettingsLocationType::PrevLocation;
        m_cLocationOffsetPercent = FVector::ZeroVector;

        m_eRotationUpdateType = MyWithCurveUpdateStepSettingsRotationType::PrevRotation;
        m_cRotationUpdateExtraCycles = FIntVector::ZeroValue;

        m_eScaleUpdateType = MyWithCurveUpdateStepSettingsScaleType::PrevScale;
        m_cScaleSpecified = FVector(1, 1, 1);
    };



    UPROPERTY(EditAnywhere, meta = (DisplayName = "location update type"))
        MyWithCurveUpdateStepSettingsLocationType m_eLocationUpdateType;

    //By default the offset is in world space, unit is percent of model size, like FVector::Size()
    UPROPERTY(EditAnywhere, meta = (DisplayName = "location offset percent"))
        FVector m_cLocationOffsetPercent;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "rotation update type"))
        MyWithCurveUpdateStepSettingsRotationType m_eRotationUpdateType;

    //how many extra 360 degrees it should animation
    UPROPERTY(EditAnywhere, meta = (DisplayName = "rotation update extra cycles"))
        FIntVector m_cRotationUpdateExtraCycles;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "scale update type"))
        MyWithCurveUpdateStepSettingsScaleType m_eScaleUpdateType;

    //only used when scale update type is specified
    UPROPERTY(EditAnywhere, meta = (DisplayName = "scale specified"))
        FVector m_cScaleSpecified;


};

USTRUCT()
struct FMyWithCurveUpdateStepSettingsTransformWidget2DCpp : public FMyWithCurveUpdateStepSettingsBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepSettingsTransformWidget2DCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeTransformWidget2D;
    };

    /*
    inline static const FMyWithCurveUpdateStepSettingsTransformWidget2DCpp& castFromBaseConst(const FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeTransformWidget2D) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepSettingsTransformWidget2DCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepSettingsTransformWidget2DCpp& castFromBase(FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepSettingsTransformWidget2DCpp&>(castFromBaseConst(base));
    };
    */

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }
    };
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
        m_cScreenCenter = FVector2D::ZeroVector;
    }

    FVector2D m_cScreenCenter;
};

USTRUCT()
struct FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp : public FMyCenterMetaOnPlayerScreenConstrainedCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp() : Super()
    {
        reset(true);
    };

    virtual ~FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp()
    {

    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        m_cScreenPoint = m_cDirectionCenterToPoint = FVector2D::ZeroVector;
        m_fCenterToPointLength = m_fCenterToPointUntilBorderLength = 0;
    };

    FVector2D m_cScreenPoint;

    FVector2D m_cDirectionCenterToPoint;
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

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }
        m_fTargetVLengthOnScreenScreenPercent = 0.1;
    };

    //the length to display on player screen, unit is percent of player screen's height
    UPROPERTY(EditAnywhere, meta = (DisplayName = "target height percent"))
        float m_fTargetVLengthOnScreenScreenPercent;
};


USTRUCT()
struct FMyWithCurveUpdateStepMetaBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepMetaBaseCpp()
    {
        reset();
        m_iType = MyTypeUnknown;
    };

    virtual ~FMyWithCurveUpdateStepMetaBaseCpp()
    {

    };

    inline void reset()
    {
        m_fTotalTime = 0;
        m_sDebugString.Reset();
    };

    inline int32 getType() const
    {
        return m_iType;
    }

    float m_fTotalTime;
    FString m_sDebugString;

protected:

    int32 m_iType;
};

USTRUCT()
struct FMyWithCurveUpdateStepMetaTransformWorld3DCpp : public FMyWithCurveUpdateStepMetaBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepMetaTransformWorld3DCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeTransformWorld3D;
    };

    /*
    inline static const FMyWithCurveUpdateStepMetaTransformWorld3DCpp& castFromBaseRefConst(const FMyWithCurveUpdateStepMetaBaseCpp& base)
    {
        if (base.getType() != MyTransformTypeTransformWorld3D) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepMetaTransformWorld3DCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepMetaTransformWorld3DCpp& castFromBaseRef(FMyWithCurveUpdateStepMetaBaseCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepMetaTransformWorld3DCpp&>(castFromBaseRefConst(base));
    };
    */

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        m_cModelInfo.reset();

        FTransform zeroT;

        m_cPointTransform = zeroT;
        m_cDisappearTransform = zeroT;

    };

    FMyModelInfoWorld3DCpp m_cModelInfo;

    FTransform m_cPointTransform;
    FTransform m_cDisappearTransform;

};


USTRUCT()
struct FMyTransformUpdateAnimationMetaTransformWidget2DCpp : public FMyWithCurveUpdateStepMetaBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTransformUpdateAnimationMetaTransformWidget2DCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeTransformWidget2D;
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        m_cModelInfo.reset();
    };

    FMyModelInfoWidget2DCpp m_cModelInfo;
};


USTRUCT()
struct FMyFlipImageElemCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyFlipImageElemCpp()
    {
        m_pTexture = NULL;
        m_iFrameOccupy = 1;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "texture"))
    UTexture2D* m_pTexture;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "frame occupy", ClampMin = 1))
    int32 m_iFrameOccupy;
};

USTRUCT()
struct FMyFlipImageSettingsCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyFlipImageSettingsCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_aFlipImageElems.Reset();
        m_fFlipTime = 1 / 30;
        m_bMatchSize = false;
        m_iLoopNum = 1;
    };

    inline int32 getTotalFrameNum() const
    {
        int32 ret = 0;
        int32 l = m_aFlipImageElems.Num();
        for (int32 i = 0; i < l; i++) {
            ret += m_aFlipImageElems[i].m_iFrameOccupy;
            MY_VERIFY(m_aFlipImageElems[i].m_iFrameOccupy > 0);
            if (m_aFlipImageElems[i].m_pTexture == NULL) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("found a flip image elem with NULL texture, idx %d."), i);
            }
        }

        return ret;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "flip image elems"))
    TArray<FMyFlipImageElemCpp> m_aFlipImageElems;

    //the time one frame occupy
    UPROPERTY(EditAnywhere, meta = (DisplayName = "flip time"))
        float m_fFlipTime;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "match size"))
        bool m_bMatchSize;

    //if <= 0, means loop for ever
    UPROPERTY(EditAnywhere, meta = (DisplayName = "loop num"))
        int32 m_iLoopNum;
};

UCLASS(BlueprintType)
class MYONLINECARDGAME_API UMyFlipImageSettingsAssetCpp : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, meta = (DisplayName = "settings"))
    FMyFlipImageSettingsCpp m_cSettings;
};


/**
* We can use UE4's flipbook as data containor, but that would increase package size about 20kb per image, which is bad for mobile game.
* So we use custom data struct.
* * No Children
*/
UCLASS()
class MYONLINECARDGAME_API UMyFlipImage : public UImage
{
    GENERATED_BODY()


public:

    UMyFlipImage() : Super()
    {
        m_pSettingsAsset = NULL;

        m_iIdxOfImageShowing = -1;
        m_iFrameOfImageShowing = 0;
        m_iLoopShowing = 0;

        m_bShowWhenFlipStart = false;
        m_bHideWhenFlipAllOver = false;
    };

    //DECLARE_DYNAMIC_DELEGATE_RetVal(int32, FOnFlipEnd);
    //@loopFinished means how many loops done, it should never <= 0 unless overspin
    DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnFlipEnd, int32, loopFinished, bool, isLastLoop);

    //when calling, it will ensure a image will be set immedietlly if expected one is not set.
    //if @loopTime is 0, it will use preset interval, otherwise interval will be calculated to match it
    //if @loopNum is 0, it means loop for ever
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    void startImageFlip(float loopTime, int32 loopNum = 1, bool matchSize = false);

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    void stopImageFlip();

    UFUNCTION(BlueprintPure, meta = (UnsafeDuringActorConstruction = "true"))
    bool isImageFlipping() const;

    UFUNCTION(BlueprintGetter)
    UMyFlipImageSettingsAssetCpp* getSettingsAsset() const
    {
        return m_pSettingsAsset;
    }

    //return true if new path is OK
    UFUNCTION(BlueprintSetter)
    void setSettingsAsset(UMyFlipImageSettingsAssetCpp* pSettingsAsset)
    {
        m_pSettingsAsset = pSettingsAsset;
    }

    //return errcode, 0 means no errro. @size will return the first flip image's size, or zero if settings is empty or invalid
    UFUNCTION(BlueprintCallable)
    int32 getImageSize(FVector2D& size) const;

    UPROPERTY(EditAnywhere, BlueprintSetter = setSettingsAsset, BlueprintGetter = getSettingsAsset, meta = (DisplayName = "settings Asset"))
    UMyFlipImageSettingsAssetCpp* m_pSettingsAsset;

    //return value is ignored, called on every step end
    UPROPERTY(EditAnywhere, meta = (IsBindableEvent = "True", DisplayName = "on flip end"))
    FOnFlipEnd OnFlipEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "show when FlipStart"))
    bool m_bShowWhenFlipStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "hide when FlipAllOver"))
    bool m_bHideWhenFlipAllOver;

protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    void BeginDestroy() override;

    void loop();
    void tryNextFrame(int32 idxOfImage, int32 frameOfImage);

    FMyFlipImageSettingsCpp m_cSettingUsing; //copy of asset, allow modifying
    int32 m_iIdxOfImageShowing;
    int32 m_iFrameOfImageShowing;
    int32 m_iLoopShowing;

    FTimerHandle m_cLoopTimerHandle;
};

//support auto scale when pressed, which saves one image texture
UCLASS()
class MYONLINECARDGAME_API UMyButton : public UButton
{
    GENERATED_BODY()

public:

    UMyButton(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual ~UMyButton() {};

    /** Called when the button is pressed */
    //UPROPERTY(BlueprintAssignable, Category = "Button|MyEvent")
    //FOnButtonPressedEvent OnPressedOverride;

    /** Called when the button is released */
    //UPROPERTY(BlueprintAssignable, Category = "Button|MyEvent")
    //FOnButtonReleasedEvent OnReleasedOverride;

protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    TSharedRef<SWidget> RebuildWidget() override;

    inline FReply SlateHandleClickedMy()
    {
        return SlateHandleClicked();
    };

    void SlateHandlePressedMy();
    void SlateHandleReleasedMy();

    inline void SlateHandleHoveredMy()
    {
        SlateHandleHovered();
    };

    inline void SlateHandleUnhoveredMy()
    {
        SlateHandleUnhovered();
    }

    FString genStyleString() const;

    //virtual void SynchronizeProperties() override;

    //UFUNCTION()
    //void HideFunction();

    //void onPressedInner();
    //void onReleasedInner();

    //Todo: use setter to update the state when modify in BP

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform normal"))
    FWidgetTransform m_cTransformWidget2DNormal;

    //If true, that transform will be applied when pressed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "enable render transform pressed"))
    bool m_bEnableTransformWidget2DPressed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = m_bEnableTransformWidget2DPressed, DisplayName = "render transform pressed"))
    FWidgetTransform m_cTransformWidget2DPressed;
};



UCLASS(Blueprintable, HideCategories = (Collision, Rendering))
class MYONLINECARDGAME_API AMyTextureGenSuitBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyTextureGenSuitBaseCpp() : Super()
    {
        reset();
    };

    virtual ~AMyTextureGenSuitBaseCpp()
    {

    };

    void reset()
    {
        m_pSceneCapture2D = NULL;
        m_aTargetActors.Reset();
        m_pRenderTarget = NULL;
        m_pRenderMaterial = NULL;
        m_sRenderMaterialTextureParamName = TEXT("InBaseColor");
        m_pTempRenderTarget = NULL;
        m_pTempRenderMaterialInstance = NULL;
    };

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    virtual bool checkSettings() const;

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    int32 genPrepare();

    //return error code, and by default it have no mips. Caller should clear target package before calling, otherwise a rename of new texture will happen, and still return 0.
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    int32 genDo(FString newTextureName);

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "scene capture 2d"))
    class ASceneCapture2D* m_pSceneCapture2D;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "target actors"))
    TArray<AActor*> m_aTargetActors;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "render target"))
    UTextureRenderTarget2D* m_pRenderTarget;

    //the material to handle the generated render target datas, we will have alpha inversed in that, and the material should handle the case
    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "render material"))
    UMaterialInterface* m_pRenderMaterial;

    //will use it to set texture parameter to "render material"
    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "render material texture param name"))
    FName m_sRenderMaterialTextureParamName;

    UPROPERTY(BlueprintReadOnly, Transient, meta = (DisplayName = "temp render target"))
    UTextureRenderTarget2D* m_pTempRenderTarget;

    UPROPERTY(BlueprintReadOnly, Transient, meta = (DisplayName = "temp render material instance"))
    UMaterialInstanceDynamic* m_pTempRenderMaterialInstance;
};


USTRUCT(BlueprintType)
struct FMyMargin1D
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMargin1D(float UniformMargin = 0)
    {
        m_fPrev = m_fNext = UniformMargin;
    };

    FMyMargin1D(float inPrev, float inNext)
    {
        m_fPrev = inPrev;
        m_fNext = inNext;
    };

    inline void reset()
    {
        m_fPrev = m_fNext = 0;
    };

    FMyMargin1D operator*(float Scale) const
    {
        return FMyMargin1D(m_fPrev * Scale, m_fNext * Scale);
    }

    FMyMargin1D operator*(const FMyMargin1D& InScale) const
    {
        return FMyMargin1D(m_fPrev * InScale.m_fPrev, m_fNext * InScale.m_fNext);
    }

    FMyMargin1D operator+(const FMyMargin1D& InDelta) const
    {
        return FMyMargin1D(m_fPrev + InDelta.m_fPrev, m_fNext + InDelta.m_fNext);
    }

    FMyMargin1D operator-(const FMyMargin1D& Other) const
    {
        return FMyMargin1D(m_fPrev - Other.m_fPrev, m_fNext - Other.m_fNext);
    }

    bool operator==(const FMyMargin1D& Other) const
    {
        return (m_fPrev == Other.m_fPrev) && (m_fNext == Other.m_fNext);
    }

    bool operator!=(const FMyMargin1D& Other) const
    {
        return m_fPrev != Other.m_fPrev || m_fNext != Other.m_fNext;
    }

    inline float GetDesiredSize1D() const
    {
        return m_fPrev + m_fNext;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (DisplayName = "previous"))
    float m_fPrev;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (DisplayName = "next"))
    float m_fNext;
};

USTRUCT(BlueprintType)
struct FMyMargin3D : public FMargin
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMargin3D(float UniformMargin = 0) : Super(0)
    {
        ZPositive = ZNegative = UniformMargin;
    };

    FMyMargin3D(float InLeft, float InTop, float InRight, float InBottom, float InZPositive, float InZNegative)
        : Super(InLeft, InTop, InRight, InBottom)
        , ZPositive(InZPositive)
        , ZNegative(InZNegative)
    { };

    inline void reset()
    {
        Left = Top = Right = Bottom = ZPositive = ZNegative = 0;
    };

    FMyMargin3D operator*(float Scale) const
    {
        return FMyMargin3D(Left * Scale, Top * Scale, Right * Scale, Bottom * Scale, ZPositive * Scale, ZNegative * Scale);
    }

    FMyMargin3D operator*(const FMyMargin3D& InScale) const
    {
        return FMyMargin3D(Left * InScale.Left, Top * InScale.Top, Right * InScale.Right, Bottom * InScale.Bottom, ZPositive * InScale.ZPositive, ZNegative * InScale.ZNegative);
    }

    FMyMargin3D operator+(const FMyMargin3D& InDelta) const
    {
        return FMyMargin3D(Left + InDelta.Left, Top + InDelta.Top, Right + InDelta.Right, Bottom + InDelta.Bottom, ZPositive + InDelta.ZPositive, ZNegative + InDelta.ZNegative);
    }

    FMyMargin3D operator-(const FMyMargin3D& Other) const
    {
        return FMyMargin3D(Left - Other.Left, Top - Other.Top, Right - Other.Right, Bottom - Other.Bottom, ZPositive - Other.ZPositive, ZNegative - Other.ZNegative);
    }

    bool operator==(const FMyMargin3D& Other) const
    {
        return (Left == Other.Left) && (Right == Other.Right) && (Top == Other.Top) && (Bottom == Other.Bottom) && (ZPositive == Other.ZPositive) && (ZNegative == Other.ZNegative);
    }

    bool operator!=(const FMyMargin3D& Other) const
    {
        return Left != Other.Left || Right != Other.Right || Top != Other.Top || Bottom != Other.Bottom || ZPositive != Other.ZPositive || ZNegative != Other.ZNegative;
    }


    inline FVector GetDesiredSize3D() const
    {
        return FVector(GetDesiredSize(), ZPositive + ZNegative);
    }


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance)
    float ZPositive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance)
    float ZNegative;
};

//elemSize based, percent means elemSize's percent
USTRUCT(BlueprintType)
struct FMyElemAndGroupDynamicArrangeMetaCpp
{
    GENERATED_USTRUCT_BODY()

public:
    
    FMyElemAndGroupDynamicArrangeMetaCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_cElemSize = FVector::ZeroVector;

        m_cAllMarginPercent.reset();

        m_cCellMarginPercent = FVector::ZeroVector;
        m_cGroupMarginPercent = FVector::ZeroVector;
    };


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "elem size"))
    FVector m_cElemSize;

    //the padding to elem that will be arranged, unit is elem size's precent
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "padding percent"))
    FMyMargin3D m_cAllMarginPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "elem spacing percent"))
    FVector m_cCellMarginPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "group spacing percent"))
    FVector m_cGroupMarginPercent;
};


//currently only pose enumed, is supported. To implement more, add enum define and implementions
USTRUCT(BlueprintType)
struct FMyRotateState90DWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyRotateState90DWorld3DCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_eFlipState = MyBoxLikeFlipStateCpp::Stand;
        m_eRollState = MyRotateState90DCpp::Zero;
    };

    inline bool operator==(const FMyRotateState90DWorld3DCpp& Other) const
    {
        return (m_eFlipState == Other.m_eFlipState) && (m_eRollState == Other.m_eRollState);
    };

    inline FString ToString() const
    {
        return FString::Printf(TEXT("%s:%s"),
            *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyBoxLikeFlipStateCpp"), (uint8)m_eFlipState),
            *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyRotateState90DCpp"), (uint8)m_eRollState));
    };


    //same as limited pitch
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "flip state"))
        MyBoxLikeFlipStateCpp m_eFlipState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "roll state"))
        MyRotateState90DCpp m_eRollState;
};


USTRUCT(BlueprintType)
struct FMyRotateState90DWidget2DCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyRotateState90DWidget2DCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_eAngleState = MyRotateState90DCpp::Zero;
    };

    inline bool operator==(const FMyRotateState90DWidget2DCpp& Other) const
    {
        return (m_eAngleState == Other.m_eAngleState);
    };

    inline FString ToString() const
    {
        return FString::Printf(TEXT("%s"),
            *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyBoxLikeFlipStateCpp"), (uint8)m_eAngleState));
    };


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "angle state"))
        MyRotateState90DCpp m_eAngleState;
};


//To caclulate the arrange, it have two steps: find the cell, fill elem in cell, cell does NOT contain any spacing or padding inside!
USTRUCT(BlueprintType)
struct FMyArrangeCfgOneDimensionCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyArrangeCfgOneDimensionCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_eCellOrderType = MyOrderTypeCpp::ASC;
        m_eCellGatherType = MyAxisAlignmentTypeCpp::Negative;
        m_eElemInCellAlignmentType = MyContinuousAlignmentTypeCpp::Prev;
        m_cAllMarginPercent.reset();
        m_cCellMarginPercent.reset();
        m_cGroupMarginPercent.reset();
        m_iNumMaxAtDimNext0Override = 0;
        m_iReduceDegree = 0;
    };

    FMyArrangeCfgOneDimensionCpp inv() const;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "cell order type"))
        MyOrderTypeCpp m_eCellOrderType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "elem in cell alignment type"))
        MyContinuousAlignmentTypeCpp m_eElemInCellAlignmentType;

    //unit is elem's size's percent, use unified size for 2D and 3D
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "all margin percent"))
        FMyMargin1D m_cAllMarginPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "cell margin percent"))
        FMyMargin1D m_cCellMarginPercent;

    //this is how many extra margin will be added to cell's when group idx is not 0
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "group margin percent"))
        FMyMargin1D m_cGroupMarginPercent;


protected:

    //members only affect coordinate calculation, not related to corordinate transform resolve

    //Every dimension have numMax unless last dimension, by default it is calculated from area.
    //If it > 0, it will override the calculated value which at next dim value 0. For example, col num max at row 0.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "num max at dim next 0 override"))
        int32 m_iNumMaxAtDimNext0Override;

    //where the cell should gather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "cell gather type"))
        MyAxisAlignmentTypeCpp m_eCellGatherType;

    // |  /   E   \
    // |D/  E E E  \
    // |/ E E E E E \
    // if 0, no reduce. If positive, next ones of this dimension will reduce. If Negative, increase. Range is (-90, 90).
    // Warn: will always affect the elem coordinate, not the elem axis value, which means elem always align in axis.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "reduce degree"))
        int32 m_iReduceDegree;
};


//Warn: any idx < 0 is valid
USTRUCT(BlueprintType)
struct FMyArrangeCoordinateOneDimensionCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyArrangeCoordinateOneDimensionCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_iIdxElem = 0;
        m_iIdxGroup = 0;
        m_iCountElemArrangedUnexpected = 0;
    };

    inline bool operator==(const FMyArrangeCoordinateOneDimensionCpp& Other) const
    {
        return (m_iIdxElem == Other.m_iIdxElem) && (m_iIdxGroup == Other.m_iIdxGroup) && (m_iCountElemArrangedUnexpected == Other.m_iCountElemArrangedUnexpected);
    }

    inline FString ToString(bool verbose = false) const
    {
        FString ret = FString::Printf(TEXT("%d"), m_iIdxElem);
        if (verbose) {
            ret += FString::Printf(TEXT(" %d %d"), m_iIdxGroup, m_iCountElemArrangedUnexpected);
        }

        return ret;
    };

    inline int32 getIdxElemUnexpected() const
    {
        int32 unexpected = m_iCountElemArrangedUnexpected >= 0 ? m_iCountElemArrangedUnexpected : 0;

        if (m_iIdxElem >= 0) {
            return unexpected;
        }
        else {
            return -unexpected;
        }
    };

    inline int32 getIdxElemExpected() const
    {
        return m_iIdxElem - getIdxElemUnexpected();
    };


    //idx is all in this dimension, NOT in group, including expected ones and unexpected ones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "idx elem"))
    int32 m_iIdxElem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "idx group"))
    int32 m_iIdxGroup;

    //count how many unexpected elem arranged before, always >= 0, < 0 is considered as 0
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "count elem arranged unexpected"))
    int32 m_iCountElemArrangedUnexpected;

};


USTRUCT(BlueprintType)
struct FMyArrangePointCfgWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyArrangePointCfgWorld3DCpp()
    {
        reset();
    };

    virtual ~FMyArrangePointCfgWorld3DCpp()
    {

    };

    inline void reset()
    {
        FTransform t;
        m_cCenterPointTransform = t;

        m_cAreaBoxExtendFinal = FVector::OneVector;

        m_eColAxisType = MyAxisTypeCpp::Y;
        m_eRowAxisType = MyAxisTypeCpp::X;

        m_cColArrange.reset();
        m_cRowArrange.reset();
        m_cStackArrange.reset();

        m_cLimitedRotationAllExpected.reset();
        m_cLimitedRotationAllUnexpected.reset();
    };


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "center point transform"))
        FTransform m_cCenterPointTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "area box extend final"))
        FVector m_cAreaBoxExtendFinal;

    //Must be valid and can't be sme as RowAxisType
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "col axis type"))
        MyAxisTypeCpp m_eColAxisType;

    //Must be valid and can't be same as ColAxisType
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "row axis type"))
        MyAxisTypeCpp m_eRowAxisType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "col arrange"))
        FMyArrangeCfgOneDimensionCpp m_cColArrange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "row arrange"))
        FMyArrangeCfgOneDimensionCpp m_cRowArrange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "stack arrange"))
        FMyArrangeCfgOneDimensionCpp m_cStackArrange;

    //all elems should have same rotation expected, to calculate col, row, stack position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "limited rotation all expected"))
        FMyRotateState90DWorld3DCpp m_cLimitedRotationAllExpected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "limited rotation all unexpected"))
        FMyRotateState90DWorld3DCpp m_cLimitedRotationAllUnexpected;

protected:

};

//the box like model have limited rotation: Pitch is actually flip state, some card game will use it
//dimention arrange order: col -> row -> stack, call could arrange idx as stack:row:col(higher dimention takes bigger priority)
//Three dimensions are always orthogonal, they don't bend
USTRUCT(BlueprintType)
struct FMyArrangeCoordinateWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyArrangeCoordinateWorld3DCpp()
    {
        reset();
    };

    //virtual function to allow reset by base pointer
    virtual void reset(bool resetSubClassDataonly = false)
    {
        m_cLimitedRotation.reset();
        m_cCol.reset();
        m_cRow.reset();
        m_cStack.reset();
    };


    inline bool equal(const FMyArrangeCoordinateWorld3DCpp& other) const
    {
        if (&other == this) {
            return true;
        }

        return m_cLimitedRotation == other.m_cLimitedRotation &&
            m_cCol == other.m_cCol && m_cRow == other.m_cRow && m_cStack == other.m_cStack;
    };

    virtual FString ToString() const
    {
        return FString::Printf(TEXT("rot %s, idxs %s: %s: %s"), *m_cLimitedRotation.ToString(),
            *m_cCol.ToString(), *m_cRow.ToString(), *m_cStack.ToString());
    };

    void helperSetIdxColRowStack(const struct FMyArrangePointResolvedMetaWorld3DCpp& meta, int32 colArranged, int32 rowArranged, int32 stackArranged);

    //Warning: whether one dim is used, is decided by whether previous dim have num limit enabled
    void helperSetIdxColRowStackByMyArrangeCoordinateMeta(const struct FMyArrangePointResolvedMetaWorld3DCpp& meta, const struct FMyArrangeCoordinateMetaCpp& myArrangeCoordinateMeta);


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "limited rotation"))
        FMyRotateState90DWorld3DCpp m_cLimitedRotation;

    //when colInRow Aligment is left or mid, this count from left to right, othwise reverse
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "col"))
        FMyArrangeCoordinateOneDimensionCpp m_cCol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "row"))
        FMyArrangeCoordinateOneDimensionCpp m_cRow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "stack"))
        FMyArrangeCoordinateOneDimensionCpp m_cStack;
};

USTRUCT(BlueprintType)
struct FMyArrangePointCfgWidget2DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyArrangePointCfgWidget2DCpp()
    {
        reset();
    };

    virtual ~FMyArrangePointCfgWidget2DCpp()
    {

    };

    inline void reset()
    {
        FWidgetTransform t;
        m_cCenterPointTransform = t;

        m_cAreaBoxExtendFinal = FVector2D::UnitVector;

        m_eColAxisType = MyAxisTypeCpp::X;
        m_cColArrange.reset();
        m_cRowArrange.reset();

        m_cLimitedRotationAllExpected.reset();
        m_cLimitedRotationAllUnexpected.reset();
    };


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "center point transform"))
        FWidgetTransform m_cCenterPointTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "area box extend final"))
        FVector2D m_cAreaBoxExtendFinal;

    //Must be valid and can't be sme as RowAxisType
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "col axis type"))
        MyAxisTypeCpp m_eColAxisType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "col arrange"))
        FMyArrangeCfgOneDimensionCpp m_cColArrange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "row arrange"))
        FMyArrangeCfgOneDimensionCpp m_cRowArrange;


    //all elems should have same rotation expected, to calculate col, row, stack position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "limited rotation all expected"))
        FMyRotateState90DWidget2DCpp m_cLimitedRotationAllExpected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "limited rotation all unexpected"))
        FMyRotateState90DWidget2DCpp m_cLimitedRotationAllUnexpected;
};


USTRUCT(BlueprintType)
struct FMyArrangeCoordinateWidget2DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyArrangeCoordinateWidget2DCpp()
    {
        reset();
    };

    //virtual function to allow reset by base pointer
    virtual void reset(bool resetSubClassDataonly = false)
    {
        m_cLimitedRotation.reset();
        m_cCol.reset();
        m_cRow.reset();
    };


    inline bool equal(const FMyArrangeCoordinateWidget2DCpp& other) const
    {
        if (&other == this) {
            return true;
        }

        return m_cLimitedRotation == other.m_cLimitedRotation &&
            m_cCol == other.m_cCol && m_cRow == other.m_cRow;
    };

    virtual FString ToString() const
    {
        return FString::Printf(TEXT("rot %s, idxs %s: %s"), *m_cLimitedRotation.ToString(),
            *m_cCol.ToString(), *m_cRow.ToString());
    };



    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "limited rotation"))
        FMyRotateState90DWidget2DCpp m_cLimitedRotation;

    //when colInRow Aligment is left or mid, this count from left to right, othwise reverse
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "col"))
        FMyArrangeCoordinateOneDimensionCpp m_cCol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "row"))
        FMyArrangeCoordinateOneDimensionCpp m_cRow;

};

USTRUCT()
struct FMyArrangeCfgResolvedOneDimCpp : public FMyArrangeCfgOneDimensionCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyArrangeCfgResolvedOneDimCpp() : Super()
    {
        reset(true);
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        m_fAreaExtend = 1;
        m_fCellExtendExpected = 1;
        m_fCellExtendUnexpected = 1;
        m_eAxisType = MyAxisTypeCpp::Invalid;

        m_iNumMaxAtDimNext0 = 0;
        m_iNumMaxByAreaAtDimNext0 = 0;
        m_iNumReduceHalfPerDimNext_100 = 0;
    };

    inline float getCellStart(float elemUnifiedSize, const FMyArrangeCoordinateOneDimensionCpp& coordinate) const
    {
        return elemUnifiedSize * m_cAllMarginPercent.m_fPrev +
            coordinate.getIdxElemExpected() * m_fCellExtendExpected * 2 + coordinate.getIdxElemUnexpected() * m_fCellExtendUnexpected * 2 +
            (coordinate.getIdxElemExpected() + coordinate.getIdxElemUnexpected()) * elemUnifiedSize * m_cCellMarginPercent.GetDesiredSize1D() +
            coordinate.m_iIdxGroup * elemUnifiedSize * m_cGroupMarginPercent.GetDesiredSize1D();
    };

    inline float getCellExtend() const
    {
        return m_fCellExtendExpected;
    };

    inline float getCellOccupiedLen(float fModelUnifiedSize) const
    {
        return getCellExtend() * 2 + m_cCellMarginPercent.GetDesiredSize1D() * fModelUnifiedSize;
    };

    //for last dimmension, caller should ignore the num max cap, and continue arrange elem in
    inline int32 getNumMax(int32 idxDimNext) const
    {
        int32 ret = m_iNumMaxAtDimNext0 - m_iNumReduceHalfPerDimNext_100 * idxDimNext / 100 * 2;
        if (ret < 1) {
            ret = 1;
        }

        return ret;
    };

    inline int32 getIdxElem(int32 idxDimNext, int32 elemArrangedBefore) const
    {
        MyOrderTypeCpp eCellOrderType;
        SetValidValue_MyOrderTypeCpp(eCellOrderType, m_eCellOrderType);

        MyAxisAlignmentTypeCpp eCellGatherType;
        SetValidValue_MyAxisAlignmentTypeCpp(eCellGatherType, m_eCellGatherType)

        int32 ret = elemArrangedBefore;
        if (eCellGatherType == MyAxisAlignmentTypeCpp::Mid) {
            ret += (m_iNumMaxByAreaAtDimNext0 - m_iNumMaxAtDimNext0) / 2;
        }
        else if (eCellGatherType == MyAxisAlignmentTypeCpp::Negative) {
            if (eCellOrderType == MyOrderTypeCpp::DESC) {
                ret += (m_iNumMaxByAreaAtDimNext0 - m_iNumMaxAtDimNext0);
            }

        }
        else if (eCellGatherType == MyAxisAlignmentTypeCpp::Positive) {
            if (eCellOrderType == MyOrderTypeCpp::ASC) {
                ret += (m_iNumMaxByAreaAtDimNext0 - m_iNumMaxAtDimNext0);
            }
        }
        else {
            MY_VERIFY(false);
        }

        ret += m_iNumReduceHalfPerDimNext_100 * idxDimNext / 100;

        //debug
        //if (m_iReduceDegree == 45) {
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("reduce angle 45: %d, %d, %d, ret %d."), idxDimNext, elemArrangedBefore, (uint8)eCellGatherType, ret);
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("m_iNumReduceHalfPerDimNext_100: %d, m_iNumMaxByAreaAtDimNext0 %d,m_iNumMaxAtDimNext0 %d."), m_iNumReduceHalfPerDimNext_100, m_iNumMaxByAreaAtDimNext0, m_iNumMaxAtDimNext0);
        //}

        return ret;
    };

    void rebuildFrom(const FMyArrangeCfgOneDimensionCpp& cfgDim, MyAxisTypeCpp axisType,
                    const FVector& areaExtend,
                    const FMyModelInfoBoxWorld3DCpp& modelInfoExpected,
                    const FMyModelInfoBoxWorld3DCpp& modelInfoUnexpected);

    void postBuild(const FMyArrangeCfgResolvedOneDimCpp* pdimNext, float fModelUnifiedSize)
    {
        float cellOccupiedLenDimThis = getCellOccupiedLen(fModelUnifiedSize);
        m_iNumMaxByAreaAtDimNext0 = (m_fAreaExtend * 2 - fModelUnifiedSize * m_cAllMarginPercent.GetDesiredSize1D()) / cellOccupiedLenDimThis;
        if (m_iNumMaxByAreaAtDimNext0 < 1) {
            m_iNumMaxByAreaAtDimNext0 = 1;

        }

        m_iNumMaxAtDimNext0 = m_iNumMaxByAreaAtDimNext0;
        if (m_iNumMaxAtDimNext0Override > 0) {
            m_iNumMaxAtDimNext0 = m_iNumMaxAtDimNext0Override;
        }


        if (pdimNext) {
            m_iNumReduceHalfPerDimNext_100 = getNumReducedHalfPerDimNext_100(pdimNext, fModelUnifiedSize);
        }
        else {
            //no limit
            m_iNumReduceHalfPerDimNext_100 = 0;
        }
    };


    float m_fAreaExtend;
    float m_fCellExtendExpected;
    float m_fCellExtendUnexpected;
    MyAxisTypeCpp m_eAxisType;

protected:

    //Only used expected model size yet, unexpected is ignored now, but enough for most cases
    inline int32 getNumReducedHalfPerDimNext_100(const FMyArrangeCfgResolvedOneDimCpp* pdimNext, float fModelUnifiedSize) const
    {
        if (pdimNext) {
            int32 reduceAngle = FMath::Clamp<int32>(m_iReduceDegree, -89, 89);
            if (reduceAngle != 0) {

                float cellOccupiedLenDimThis = getCellOccupiedLen(fModelUnifiedSize);
                float cellOccupiedLenDimNext = pdimNext->getCellOccupiedLen(fModelUnifiedSize);

                float fT = FMath::Tan(FMath::DegreesToRadians((float)reduceAngle));
                float fLenReducedHalfPerNextDim = cellOccupiedLenDimNext * fT;
                return fLenReducedHalfPerNextDim * 100 / cellOccupiedLenDimThis;
            }
        }

        return 0;
    };


    //post build datas
    int32 m_iNumMaxAtDimNext0;
    int32 m_iNumMaxByAreaAtDimNext0;
    int32 m_iNumReduceHalfPerDimNext_100;
};


USTRUCT(BlueprintType)
struct FMyArrangePointResolvedMetaWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyArrangePointResolvedMetaWorld3DCpp()
    {
        reset();
    };

    inline void reset()
    {
        FTransform t;
        m_cCenterPointTransform = t;

        m_fModelUnifiedSize = 0;

        m_aDimCfgs.Reset();
        m_aDimCfgs.AddDefaulted(3);

        m_cModelInfo.reset();
    };


    inline int32 getColNumMax(int32 idxRow) const
    {
        return m_aDimCfgs[0].getNumMax(idxRow);
    };

    inline int32 getColIdx(int32 idxRow, int32 elemArrangedBefore) const
    {
        return m_aDimCfgs[0].getIdxElem(idxRow, elemArrangedBefore);
    };

    inline int32 getRowNumMax(int32 idxStack) const
    {
        return m_aDimCfgs[1].getNumMax(idxStack);
    };

    inline int32 getRowIdx(int32 idxStack, int32 elemArrangedBefore) const
    {
        return m_aDimCfgs[1].getIdxElem(idxStack, elemArrangedBefore);
    };

    inline int32 getStackNumMax() const
    {
        return m_aDimCfgs[2].getNumMax(0);
    };

    inline int32 getStackIdx(int32 elemArrangedBefore) const
    {
        return m_aDimCfgs[2].getIdxElem(0, elemArrangedBefore);
    };


    void rebuildFrom(const FMyArrangePointCfgWorld3DCpp& cPointCfg, const FMyModelInfoBoxWorld3DCpp& cModelInfo);



    FTransform m_cCenterPointTransform;

    float m_fModelUnifiedSize;

    //always size 3, and is col->row->stack, unless not build
    TArray<FMyArrangeCfgResolvedOneDimCpp> m_aDimCfgs;

    FMyModelInfoBoxWorld3DCpp m_cModelInfo;

protected:

};



USTRUCT()
struct FMyArrangeCoordinateMetaCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyArrangeCoordinateMetaCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_iColArranged = -1;
        m_iRowArranged = -1;
        m_iStackArranged = -1;

        m_iColNumMax = -1;
        m_bColNumMaxLimitEnabled = true;
        m_iRowNumMax = -1;
        m_bRowNumMaxLimitEnabled = true;
    };

    inline void helperUpdateMyArrangeCoordinateMetaAfterOneElemArranged(const FMyArrangePointResolvedMetaWorld3DCpp& meta)
    {
        m_iColArranged++;

        if (m_bColNumMaxLimitEnabled && m_iColArranged >= m_iColNumMax) {
            m_iRowArranged++;
            if (m_bRowNumMaxLimitEnabled && m_iRowArranged >= m_iRowNumMax) {
                m_iStackArranged++;
                m_iRowArranged = 0;
                m_iRowNumMax = meta.getRowNumMax(m_iStackArranged);
            }
            m_iColArranged = 0;
            m_iColNumMax = meta.getColNumMax(m_iRowArranged);
        }

    };


    int32 m_iColArranged;
    int32 m_iRowArranged;
    int32 m_iStackArranged;

    int32 m_iColNumMax;
    bool m_bColNumMaxLimitEnabled;
    int32 m_iRowNumMax;
    bool m_bRowNumMaxLimitEnabled;
};



//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyRenderUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static inline const float& getAxisFromVectorRefConst(const FVector &data, MyAxisTypeCpp type)
    {
        if (type == MyAxisTypeCpp::X) {
            return data.X;
        }
        else if (type == MyAxisTypeCpp::Y) {
            return data.Y;
        }
        else if (type == MyAxisTypeCpp::Z) {
            return data.Z;
        }
        else {
            MY_VERIFY(false);
            return data.X;
        }
    };

    static inline float& getAxisFromVectorRef(FVector &data, MyAxisTypeCpp type)
    {
        return const_cast<float&>(getAxisFromVectorRefConst(data, type));
    };

    //@aSorted will have col, row, stack axiss
    static inline void getSortedAxis3D(MyAxisTypeCpp colAxis, MyAxisTypeCpp rawAxis, TArray<MyAxisTypeCpp>& aSorted)
    {
        aSorted.Reset();

        TSet<MyAxisTypeCpp> sAxiss;
        sAxiss.Emplace(MyAxisTypeCpp::X);
        sAxiss.Emplace(MyAxisTypeCpp::Y);
        sAxiss.Emplace(MyAxisTypeCpp::Z);

        if (sAxiss.Find(colAxis) == NULL) {
            MyAxisTypeCpp newT = *sAxiss.CreateIterator();
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("fixing col axis type since it invalid or already used: %s -> %s"),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyAxisTypeCpp"), (uint8)colAxis),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyAxisTypeCpp"), (uint8)newT));
            colAxis = newT;
        }
        sAxiss.Remove(colAxis);
        aSorted.Emplace(colAxis);

        if (sAxiss.Find(rawAxis) == NULL) {
            MyAxisTypeCpp newT = *sAxiss.CreateIterator();
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("fixing row axis type since it invalid or already used: %s -> %s"),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyAxisTypeCpp"), (uint8)rawAxis),
                *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyAxisTypeCpp"), (uint8)newT));
            rawAxis = newT;
        }
        sAxiss.Remove(rawAxis);
        aSorted.Emplace(rawAxis);

        aSorted.Emplace(*sAxiss.CreateIterator());
    };

    static inline MyAxisTypeCpp Conv_AxisTypeWidget2D_AxisTypeWorld3D(MyAxisTypeCpp type2D)
    {
        if (type2D == MyAxisTypeCpp::Invalid || type2D == MyAxisTypeCpp::Z) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("2D to 3D: invalid MyAxisTypeCpp %s, forceing to default!."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyAxisTypeCpp"), (uint8)type2D));
            type2D = MyAxisTypeCpp::X;
        }

        if (type2D == MyAxisTypeCpp::X) {
            return MyAxisTypeCpp::Y;
        }
        else if (type2D == MyAxisTypeCpp::Y) {
            return MyAxisTypeCpp::Z;
        }
        else {
            MY_VERIFY(false);
            return MyAxisTypeCpp::Invalid;
        }
    };

    static inline MyAxisTypeCpp Conv_AxisTypeWorld3D_AxisTypeWidget2D(MyAxisTypeCpp type3D)
    {
        if (type3D == MyAxisTypeCpp::Invalid || type3D == MyAxisTypeCpp::X) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("3D to 2D: invalid MyAxisTypeCpp %s, forceing to default!."), *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyAxisTypeCpp"), (uint8)type3D));
            type3D = MyAxisTypeCpp::Y;
        }

        if (type3D == MyAxisTypeCpp::Y) {
            return MyAxisTypeCpp::X;
        }
        else if (type3D == MyAxisTypeCpp::Z) {
            return MyAxisTypeCpp::Y;
        }
        else {
            MY_VERIFY(false);
            return MyAxisTypeCpp::Invalid;
        }
    };

    static inline FVector Conv_LocWidget2D_LocWorld3D(const FVector2D& locWidget2D)
    {
        return FVector(0, -locWidget2D.X, -locWidget2D.Y);
    };

    static inline FVector2D Conv_LocWorld3D_LocWidget2D(const FVector& locWorld3D)
    {
        return FVector2D(-locWorld3D.Y, -locWorld3D.Z);
    };

    static inline FRotator Conv_AngleWidget2D_RotationWorld3D(float angleWidget2D)
    {
        return FRotator(0, 0, -angleWidget2D);
    };

    static inline float Conv_RotationWorld3D_AngleWidget2D(const FRotator& rotatorWorld3D)
    {
        return -rotatorWorld3D.Roll;
    };

    //suitable for box extend, and scale
    static inline FVector Conv_SizeWidget2D_SizeWorld3D(const FVector2D& sizeWidget2D, float defaultX = 1)
    {
        return FVector(defaultX, sizeWidget2D.X, sizeWidget2D.Y);
    };

    static inline FVector2D Conv_SizeWorld3D_SizeWidget2D(const FVector& sizeWorld3D)
    {
        return FVector2D(sizeWorld3D.Y, sizeWorld3D.Z);
    };

    //world model facing camera with x positive£º X->-Y, Y->-Z, angle->-roll
    //Warn: rotator's X, Y is ignored now, since in widget it is coupled in scale and shear
    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTransformWorld3D (TransformWidget2D)", CompactNodeTitle = "->", BlueprintAutocast), Category = "UMyRenderUtilsLibrary")
        static inline FTransform Conv_TransformWidget2D_TransformWorld3D(const FWidgetTransform& TransformWidget2D)
    {
        FTransform ret;

        ret.SetLocation(Conv_LocWidget2D_LocWorld3D(TransformWidget2D.Translation));
        ret.SetRotation(Conv_AngleWidget2D_RotationWorld3D(TransformWidget2D.Angle).Quaternion());
        ret.SetScale3D(Conv_SizeWidget2D_SizeWorld3D(TransformWidget2D.Scale));

        return ret;
    };

    //world model facing camera with x positive£º Y->-X, Z->-Y, roll->-Angle
    //Warn: rotator's X, Y is ignored now, since in widget it is coupled in scale and shear
    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTransformWidget2D (TransformWorld3D)", CompactNodeTitle = "->", BlueprintAutocast), Category = "UMyRenderUtilsLibrary")
        static inline FWidgetTransform Conv_TransformWorld3D_TransformWidget2D(const FTransform& TransformWorld3D)
    {
        FWidgetTransform ret;

        FVector loc = TransformWorld3D.GetLocation();
        FVector scale3D = TransformWorld3D.GetScale3D();

        ret.Translation = Conv_LocWorld3D_LocWidget2D(loc);
        ret.Angle = Conv_RotationWorld3D_AngleWidget2D(TransformWorld3D.GetRotation().Rotator());
        ret.Scale = Conv_SizeWorld3D_SizeWidget2D(scale3D);

        ret.Shear = FVector2D::ZeroVector;

        return ret;
    };

    static inline MyRotateState90DCpp Inv_MyRotateState90D(MyRotateState90DCpp rotateState)
    {
        if (rotateState == MyRotateState90DCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("rotate state is invalid, setting as default one now."));
            rotateState = MyRotateState90DCpp::Zero;
        }

        if (rotateState == MyRotateState90DCpp::Zero) {
            return MyRotateState90DCpp::Zero;
        }
        else if (rotateState == MyRotateState90DCpp::Positive90D) {
            return MyRotateState90DCpp::Negative90D;
        }
        else if (rotateState == MyRotateState90DCpp::Negative90D) {
            return MyRotateState90DCpp::Positive90D;
        }
        else {
            MY_VERIFY(false);
            return MyRotateState90DCpp::Zero;
        }
    };

    static inline MyOrderTypeCpp Inv_MyOrderType(MyOrderTypeCpp type)
    {
        if (type == MyOrderTypeCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("type is invalid, setting as default one now."));
            type = MyOrderTypeCpp::ASC;
        }

        if (type == MyOrderTypeCpp::ASC) {
            return MyOrderTypeCpp::DESC;
        }
        else if (type == MyOrderTypeCpp::DESC) {
            return MyOrderTypeCpp::ASC;
        }
        else {
            MY_VERIFY(false);
            return MyOrderTypeCpp::Invalid;
        }
    };

    static inline MyAxisAlignmentTypeCpp Inv_MyAxisAlignmentType(MyAxisAlignmentTypeCpp type)
    {
        if (type == MyAxisAlignmentTypeCpp::Invalid) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("type is invalid, setting as default one now."));
            type = MyAxisAlignmentTypeCpp::Negative;
        }

        if (type == MyAxisAlignmentTypeCpp::Negative) {
            return MyAxisAlignmentTypeCpp::Positive;
        }
        else if (type == MyAxisAlignmentTypeCpp::Mid) {
            return MyAxisAlignmentTypeCpp::Mid;
        }
        else if (type == MyAxisAlignmentTypeCpp::Positive) {
            return MyAxisAlignmentTypeCpp::Negative;
        }
        else {
            MY_VERIFY(false);
            return MyAxisAlignmentTypeCpp::Invalid;
        }
    };

    //quick path for 90D rotate, instead of float calculation
    static inline FVector applyFlipState(const FVector& old, MyBoxLikeFlipStateCpp state)
    {
        FVector ret = old;
        if (state == MyBoxLikeFlipStateCpp::Up)
        {
            ret.X = -old.Z;
            ret.Z = old.X;
        }
        else if (state == MyBoxLikeFlipStateCpp::Down)
        {
            ret.X = old.Z;
            ret.Z = -old.X;
        }

        return ret;
    };

    static inline FVector applyRollState90D(const FVector& old, MyRotateState90DCpp state)
    {
        FVector ret = old;
        if (state == MyRotateState90DCpp::Positive90D)
        {
            ret.Y = old.Z;
            ret.Z = -old.Y;
        }
        else if (state == MyRotateState90DCpp::Negative90D)
        {
            ret.Y = -old.Z;
            ret.Z = old.Y;
        }

        return ret;
    };

    static inline FRotator Conv_MyRotateState90DWorld3D_Rotator(const FMyRotateState90DWorld3DCpp& data)
    {
        FRotator ret(0, 0, 0);
        if (data.m_eRollState == MyRotateState90DCpp::Negative90D) {
            ret.Roll = -90;
        }
        else if (data.m_eRollState == MyRotateState90DCpp::Zero) {

        }
        else if (data.m_eRollState == MyRotateState90DCpp::Positive90D) {
            ret.Roll = 90;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got unexpected roll state %d."), (uint8)data.m_eRollState);
        }

        if (data.m_eFlipState == MyBoxLikeFlipStateCpp::Down) {
            ret.Pitch = -90;
        }
        else if (data.m_eFlipState == MyBoxLikeFlipStateCpp::Stand) {

        }
        else if (data.m_eFlipState == MyBoxLikeFlipStateCpp::Up) {
            ret.Pitch = 90;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("got unexpected flip state %d."), (uint8)data.m_eFlipState);
        }

        return ret;
    };

    static inline FMyRotateState90DWorld3DCpp Conv_MyRotateState90DWidget2D_MyRotateState90DWorld3D(const FMyRotateState90DWidget2DCpp& data)
    {
        FMyRotateState90DWorld3DCpp ret;
        ret.m_eFlipState = MyBoxLikeFlipStateCpp::Stand;
        ret.m_eRollState = Inv_MyRotateState90D(data.m_eAngleState);

        return ret;
    };

    static inline FMyRotateState90DWidget2DCpp Conv_MyRotateState90DWorld3D_MyRotateState90DWidget2D(const FMyRotateState90DWorld3DCpp& data)
    {
        FMyRotateState90DWidget2DCpp ret;
        ret.m_eAngleState = Inv_MyRotateState90D(data.m_eRollState);

        return ret;
    };

    UFUNCTION(BlueprintCallable)
    static inline FVector applyMyRotateState90D_Vector_World3D(const FVector& old, const FMyRotateState90DWorld3DCpp& state)
    {
        //The interesting thing is that, UE4 apply with sequence yaw->pitch->roll in local space
        //equal to roll->pitch->yaw in global space
        //here we need global space result

        //FVector ret = applyFlipState(old, state.m_eFlipState);
        //return applyRollState90D(ret, state.m_eRollState);

        FVector ret = applyRollState90D(old, state.m_eRollState);
        return applyFlipState(ret, state.m_eFlipState);
    };

    UFUNCTION(BlueprintCallable)
    static inline FVector2D applyMyRotateState90D_Vector_Widget2D(const FVector2D& old, const FMyRotateState90DWidget2DCpp& state)
    {
        FVector ret3D = applyMyRotateState90D_Vector_World3D(Conv_LocWidget2D_LocWorld3D(old), Conv_MyRotateState90DWidget2D_MyRotateState90DWorld3D(state));
        return Conv_LocWorld3D_LocWidget2D(ret3D);
    };

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (MyModelInfoBoxWorld3D)", CompactNodeTitle = "->", BlueprintAutocast))
        static inline FString Conv_MyModelInfoBoxWorld3D_String(const FMyModelInfoBoxWorld3DCpp& data)
    {
        return data.ToString();
    };

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (MyModelInfoBoxWidget2D)", CompactNodeTitle = "->", BlueprintAutocast))
        static inline FString Conv_MyModelInfoBoxWidget2D_String(const FMyModelInfoBoxWidget2DCpp& data)
    {
        return data.ToString();
    };

    UFUNCTION(BlueprintCallable)
    static inline FMyModelInfoBoxWorld3DCpp applyMyRotateState90D_ModelInfoBox_World3D(const FMyModelInfoBoxWorld3DCpp& old, const FMyRotateState90DWorld3DCpp& state)
    {
        FMyModelInfoBoxWorld3DCpp ret = old;
        ret.m_cCenterPointRelativeLocation = applyMyRotateState90D_Vector_World3D(old.m_cCenterPointRelativeLocation, state);
        ret.m_cBoxExtend = applyMyRotateState90D_Vector_World3D(ret.m_cBoxExtend, state).GetAbs();
        return ret;
    };

    static inline FMyArrangePointCfgWorld3DCpp Conv_MyArrangePointCfgWidget2D_MyArrangePointCfgWorld3D(const FMyArrangePointCfgWidget2DCpp& data)
    {
        FMyArrangePointCfgWorld3DCpp ret;

        ret.m_cCenterPointTransform = Conv_TransformWidget2D_TransformWorld3D(data.m_cCenterPointTransform);
        ret.m_cAreaBoxExtendFinal = Conv_SizeWidget2D_SizeWorld3D(data.m_cAreaBoxExtendFinal);
        ret.m_eColAxisType = Conv_AxisTypeWidget2D_AxisTypeWorld3D(data.m_eColAxisType);
        if (ret.m_eColAxisType == MyAxisTypeCpp::Y) {
            ret.m_eRowAxisType = MyAxisTypeCpp::Z;
        }
        else {
            MY_VERIFY(ret.m_eColAxisType == MyAxisTypeCpp::Z);
            ret.m_eRowAxisType = MyAxisTypeCpp::Y;
        }

        ret.m_cColArrange = data.m_cColArrange.inv();
        ret.m_cRowArrange = data.m_cRowArrange.inv();
        ret.m_cStackArrange.reset();

        ret.m_cLimitedRotationAllExpected = Conv_MyRotateState90DWidget2D_MyRotateState90DWorld3D(data.m_cLimitedRotationAllExpected);
        ret.m_cLimitedRotationAllUnexpected = Conv_MyRotateState90DWidget2D_MyRotateState90DWorld3D(data.m_cLimitedRotationAllUnexpected);

        return ret;
    };

    static inline FMyArrangePointCfgWidget2DCpp Conv_MyArrangePointCfgWorld3D_MyArrangePointCfgWidget2D(const FMyArrangePointCfgWorld3DCpp& data)
    {
        FMyArrangePointCfgWidget2DCpp ret;

        ret.m_cCenterPointTransform = Conv_TransformWorld3D_TransformWidget2D(data.m_cCenterPointTransform);
        ret.m_cAreaBoxExtendFinal = Conv_SizeWorld3D_SizeWidget2D(data.m_cAreaBoxExtendFinal);
        ret.m_eColAxisType = Conv_AxisTypeWorld3D_AxisTypeWidget2D(data.m_eColAxisType);

        ret.m_cColArrange = data.m_cColArrange.inv();
        ret.m_cRowArrange = data.m_cRowArrange.inv();

        ret.m_cLimitedRotationAllExpected = Conv_MyRotateState90DWorld3D_MyRotateState90DWidget2D(data.m_cLimitedRotationAllExpected);
        ret.m_cLimitedRotationAllUnexpected = Conv_MyRotateState90DWorld3D_MyRotateState90DWidget2D(data.m_cLimitedRotationAllUnexpected);

        return ret;
    };

    static inline FMyArrangeCoordinateWorld3DCpp Conv_MyArrangeCoordinateWidget2D_MyArrangeCoordinateWorld3D(const FMyArrangeCoordinateWidget2DCpp& data)
    {
        FMyArrangeCoordinateWorld3DCpp ret;

        ret.m_cLimitedRotation = Conv_MyRotateState90DWidget2D_MyRotateState90DWorld3D(data.m_cLimitedRotation);
        ret.m_cCol = data.m_cCol;
        ret.m_cRow = data.m_cRow;
        ret.m_cStack.reset();

        return ret;
    };

    static inline FMyArrangeCoordinateWidget2DCpp Conv_MyArrangeCoordinateWorld3D_MyArrangeCoordinateWidget2D(const FMyArrangeCoordinateWorld3DCpp& data)
    {
        FMyArrangeCoordinateWidget2DCpp ret;

        ret.m_cLimitedRotation = Conv_MyRotateState90DWorld3D_MyRotateState90DWidget2D(data.m_cLimitedRotation);
        ret.m_cCol = data.m_cCol;
        ret.m_cRow = data.m_cRow;

        return ret;
    };

    static inline FMyModelInfoBoxWorld3DCpp Conv_MyModelInfoBoxWidget2D_MyModelInfoBoxWorld3D(const FMyModelInfoBoxWidget2DCpp& data)
    {
        FMyModelInfoBoxWorld3DCpp ret;
        ret.m_cCenterPointRelativeLocation = Conv_LocWidget2D_LocWorld3D(data.m_cCenterPointRelativeLocation);
        ret.m_cBoxExtend = Conv_SizeWidget2D_SizeWorld3D(data.m_cBoxExtend);
        return ret;
    };

    static inline FMyModelInfoBoxWidget2DCpp Conv_MyModelInfoBoxWorld3D_MyModelInfoBoxWidget2D(const FMyModelInfoBoxWorld3DCpp& data)
    {
        FMyModelInfoBoxWidget2DCpp ret;
        ret.m_cCenterPointRelativeLocation = Conv_LocWorld3D_LocWidget2D(data.m_cCenterPointRelativeLocation);
        ret.m_cBoxExtend = Conv_SizeWorld3D_SizeWidget2D(data.m_cBoxExtend);
        return ret;
    };

    UFUNCTION(BlueprintPure)
        static inline FVector2D getRenderTransformPivotByMyModelInfoBoxWidget2D(const FMyModelInfoBoxWidget2DCpp& cModelInfo)
    {
        FVector2D pivotTransform = (cModelInfo.m_cBoxExtend - cModelInfo.m_cCenterPointRelativeLocation) / (cModelInfo.m_cBoxExtend * 2);
        return pivotTransform;
    };

    UFUNCTION(BlueprintPure)
    static inline FMyArrangePointResolvedMetaWorld3DCpp genMyArrangePointResolvedMetaWorld3D_World3D(const FMyArrangePointCfgWorld3DCpp& data, const FMyModelInfoBoxWorld3DCpp& cModelInfo)
    {
        FMyArrangePointResolvedMetaWorld3DCpp ret;
        ret.rebuildFrom(data, cModelInfo);
        return ret;
    };

    UFUNCTION(BlueprintPure)
    static inline FMyArrangePointResolvedMetaWorld3DCpp genMyArrangePointResolvedMetaWorld3D_Widget2D(const FMyArrangePointCfgWidget2DCpp& data, const FMyModelInfoBoxWidget2DCpp& cModelInfo)
    {
        return genMyArrangePointResolvedMetaWorld3D_World3D(Conv_MyArrangePointCfgWidget2D_MyArrangePointCfgWorld3D(data), Conv_MyModelInfoBoxWidget2D_MyModelInfoBoxWorld3D(cModelInfo));
    };


    //get the stable size that always same when 2D 3D mapped each other
    static inline float getUnifiedSize2D3D(const FMyModelInfoBoxWorld3DCpp& modelInfo)
    {
        return UMyRenderUtilsLibrary::Conv_SizeWorld3D_SizeWidget2D(modelInfo.m_cBoxExtend).Size();
    };


    //
    // target model:
    // U: flip up, D: flip down, ST: flip stand
    //           ____
    //          |    |
    //  ________| ST |
    // | U or D |    |
    // |________|____|
    //
    // row and stack base line is always aligned
    // not completely free: all box always arrange at the bottom(model's Z negative after rotation), simulate the box on desk
    UFUNCTION(BlueprintCallable)
    static void helperBoxModelResolveTransformWorld3D(const FMyArrangePointResolvedMetaWorld3DCpp& meta,
                                                      const FMyArrangeCoordinateWorld3DCpp& coordinate,
                                                      FTransform& outTransform);

    UFUNCTION(BlueprintCallable)
    static void helperBoxModelResolveTransformWidget2D(const FMyArrangePointResolvedMetaWorld3DCpp& meta,
                                                       const FMyArrangeCoordinateWidget2DCpp& coordinate,
                                                       FWidgetTransform& outTransform);



    static void helperResolveTransformWorld3DFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
                                                                                            float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                            const FVector2D& targetPosiFixOnScreenPercent,
                                                                                            float targetVOnScreenPercent,
                                                                                            float targetModelHeightInWorld,
                                                                                            FTransform &outTargetTranform);

    static float helperGetRemainTimePercent(const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp>& stepDatas);

    static void helperUpdatersSetupStep(const FMyWithCurveUpdateStepMetaTransformWorld3DCpp& meta,
                                        const FMyWithCurveUpdateStepSettingsTransformWorld3DCpp& stepData,
                                        const TArray<FMyWithCurveUpdaterTransformWorld3DCpp *>& updatersSorted);

    static void helperUpdatersSetupSteps(const FMyWithCurveUpdateStepMetaTransformWorld3DCpp& meta,
                                         const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp>& stepDatas,
                                         const TArray<FMyWithCurveUpdaterTransformWorld3DCpp *>& updatersSorted);



    static void helperUpdatersSetupStepsForPointTransformWorld3D(const UObject* WorldContextObject,
                                                                 float totalDur,
                                                                 const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                 const FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                 const TArray<FMyWithCurveUpdateStepSettingsTransformWorld3DCpp>& stepDatas,
                                                                 float extraDelayDur,
                                                                 const TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp *>& updaterInterfaces,
                                                                 FString debugName,
                                                                 bool clearPrevSteps);

    //a step take 100% of time @waitTime, will be added
    static void helperUpdatersAddWaitStep(float waitTime, FString debugStr, const TArray<FMyWithCurveUpdaterTransformWorld3DCpp *>& updaters);
    static void helperUpdatersAddWaitStep(float waitTime, FString debugStr, const TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp *>& updaterInterfaces);
    //static void helperUpdatersAddWaitStep(float waitTime, FString debugStr, const TArray<IMyWithCurveUpdaterTransformWidget2DInterfaceCpp *>& updaterInterfaces);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (SlateBrush)", CompactNodeTitle = "->", BlueprintAutocast), Category = "UMyRenderUtilsLibrary")
    static FString Conv_SlateBrush_String(const FSlateBrush& brush);

    //Some feature like mips can only work whe the size met the size requirement, this function check if it mets
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"), Category = "UMyRenderUtilsLibrary")
    static bool RenderTargetIsSizePowerOfTwo(UTextureRenderTarget2D* inTextureRenderTarget);

    //assume size is not power of two, so feature like mips will be disabled
    static UTexture2D* RenderTargetConstructTexture2DIgnoreSizeLimit(UTextureRenderTarget2D* inTextureRenderTarget, UObject* Outer, const FString& NewTexName, EObjectFlags InObjectFlags, uint32 Flags = CTF_Default, TArray<uint8>* AlphaOverride = NULL);

    //will try create one even if the size is not power of two
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"), Category = "UMyRenderUtilsLibrary")
    static UTexture2D* RenderTargetCreateStaticTexture2DTryBest(UTextureRenderTarget2D* inTextureRenderTarget, FString InName = "Texture", enum TextureCompressionSettings CompressionSettings = TC_Default, enum TextureMipGenSettings MipSettings = TMGS_FromTextureGroup);

    //return error code.
    //before calling, make sure widget is anchored in one point, and its offset right and bottom reflect the width and height.
    //@canvasSize in most case can be get by UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry() if canvas fill the screen, or specified manually if canvas is a "small" widget
    //@positionInCanvas coordinate is right as x positive, down as y positive in canvas space, mostly like screen position in UWidgetLayoutLibrary API
    UFUNCTION(BlueprintCallable, Category = "UMyRenderUtilsLibrary")
    static int32 getCenterPointPositionForWidgetInCanvasWithPointAnchor(const FVector2D& canvasSize, const UWidget* widgetUnderCanvas, FVector2D& positionInCanvas);



    UFUNCTION(BlueprintCallable, Category = "UMyRenderUtilsLibrary")
    static void myElemAndGroupDynamicArrangeCalcTotalSize(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupCount, const FIntVector& elemCount, FVector& totalSize);

    //@arrangeDirectionNegative means whether it arrange negtive, if its X !=0, it means arrange from max to min, like right to left in 2D screen case
    //UFUNCTION(BlueprintCallable, Category = "UMyRenderUtilsLibrary")
    //static void myElemAndGroupDynamicArrangeGetElemCenterPositionArrangeDirectionAllPositive(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, FIntVector idxGroup, FIntVector idxElem, FVector totalSize, FIntVector arrangeDirectionNegative, FVector& centerPosition);

    //@idxElem is idx of total elem, not idx in group
    //@centerPosition the coordinate is always start from 0
    UFUNCTION(BlueprintCallable, Category = "UMyRenderUtilsLibrary")
    static void myElemAndGroupDynamicArrangeGetElemCenterPositionArrangeDirectionAllPositive(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& idxGroup, const FIntVector& idxElem, FVector& centerPosition);

    UFUNCTION(BlueprintCallable, Category = "UMyRenderUtilsLibrary")
    static void myElemAndGroupCalcDelimiterNumber(const FIntVector &groupWalked, const FIntVector &elemWalked, FIntVector &groupDelimiterNumber, FIntVector &elemDelimiterNumber);


protected:

    struct FMyArrangeCoordinateResolvedOneDimCpp : public FMyArrangeCoordinateOneDimensionCpp
    {
    public:
        FMyArrangeCoordinateResolvedOneDimCpp() : FMyArrangeCoordinateOneDimensionCpp()
        {
            reset(true);
        };

        inline void reset(bool resetSubClassDataonly = false)
        {
            if (!resetSubClassDataonly) {
                FMyArrangeCoordinateOneDimensionCpp::reset();
            }

            m_fElemExtend = 0;
            m_fElemCenter = 0;

            m_fOutAxisValue = 0;

            m_eAxisType = MyAxisTypeCpp::Invalid;
        };

        inline void rebuildFrom(const FMyArrangeCoordinateOneDimensionCpp& base, MyAxisTypeCpp axisType,
                                const FMyModelInfoBoxWorld3DCpp& modelInfoThisElem)
        {

            *StaticCast<FMyArrangeCoordinateOneDimensionCpp *>(this) = base;
            m_fElemExtend = getAxisFromVectorRefConst(modelInfoThisElem.m_cBoxExtend, axisType);
            m_fElemCenter = getAxisFromVectorRefConst(modelInfoThisElem.m_cCenterPointRelativeLocation, axisType);

            m_eAxisType = axisType;
        };

        float m_fElemExtend;
        float m_fElemCenter;

        float m_fOutAxisValue;

        MyAxisTypeCpp m_eAxisType;
    };
 

    // -------------
    // | E   EE   E
    // | E   E*
    //calc the * coordinate, ignore * self's occupization and margin
    static void myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupWalked, const FIntVector& elemWalked, FVector& distanceIgnorePadding);
};