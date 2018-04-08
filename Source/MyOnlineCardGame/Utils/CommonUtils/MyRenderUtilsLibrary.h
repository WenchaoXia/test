// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyRenderInterface.h"

#include "RenderUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Blueprint/UserWidget.h"

#include "MyRenderUtilsLibrary.generated.h"

#define MyTransformTypeWorldTransform (MyTypeUnknown + 1)
#define MyTransformTypeWidgetTransform (MyTypeUnknown + 2)

#define FMyWithCurveUpdateStepDataWorldTransformCpp_Delta_Min (0.1f)


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
struct FMyWithCurveUpdateStepDataWorldTransformCpp : public FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataWorldTransformCpp() : Super()
    {
        reset(true);

        m_sClassName = TEXT("FMyWithCurveUpdateStepDataWorldTransformCpp");
        m_iType = MyTransformTypeWorldTransform;
    };

    virtual ~FMyWithCurveUpdateStepDataWorldTransformCpp()
    {
    };

    inline static const FMyWithCurveUpdateStepDataWorldTransformCpp& castFromBaseRefConst(const FMyWithCurveUpdateStepDataBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeWorldTransform) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepDataWorldTransformCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepDataWorldTransformCpp& castFromBaseRef(FMyWithCurveUpdateStepDataBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepDataWorldTransformCpp&>(castFromBaseRefConst(base));
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
        return new FMyWithCurveUpdateStepDataWorldTransformCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataWorldTransformCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataWorldTransformCpp *>(&other);
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


struct FMyWithCurveUpdaterWorldTransformCpp : public FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataWorldTransformCpp>
{

public:
    FMyWithCurveUpdaterWorldTransformCpp() : FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataWorldTransformCpp>()
    {
        m_iType = MyTransformTypeWorldTransform;
    };

    virtual ~FMyWithCurveUpdaterWorldTransformCpp()
    {

    };

    inline static const FMyWithCurveUpdaterWorldTransformCpp& castFromBaseRefConst(const FMyWithCurveUpdaterBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeWorldTransform) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdaterWorldTransformCpp&>(base);
    };

    inline static FMyWithCurveUpdaterWorldTransformCpp& castFromBaseRef(FMyWithCurveUpdaterBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdaterWorldTransformCpp&>(castFromBaseRefConst(base));
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
            const FMyWithCurveUpdateStepDataWorldTransformCpp *pLast = NULL;
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
class MYONLINECARDGAME_API UMyWithCurveUpdaterWorldTransformComponent : public UMovementComponent
{
    GENERATED_BODY()

public:
    UMyWithCurveUpdaterWorldTransformComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual ~UMyWithCurveUpdaterWorldTransformComponent()
    {

    };

    //following blueprint functions mainly used for test
    UFUNCTION(BlueprintCallable)
        static void helperSetUpdateStepDataTransformBySrcAndDst(FMyWithCurveUpdateStepDataWorldTransformCpp& data, float time, UCurveVector* curve, const FTransform& start, const FTransform& end, FIntVector extraRotateCycle)
    {
        data.helperSetDataBySrcAndDst(time, curve, start, end, extraRotateCycle);
    };

    UFUNCTION(BlueprintCallable)
        int32 updaterAddStepToTail(const FMyWithCurveUpdateStepDataWorldTransformCpp& data)
    {
        return m_cUpdater.addStepToTail(data);
    };

    UFUNCTION(BlueprintCallable)
        void updaterClearSteps()
    {
        m_cUpdater.clearSteps();
    }


    inline FMyWithCurveUpdaterWorldTransformCpp& getMyWithCurveUpdaterTransformRef()
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

    FMyWithCurveUpdaterWorldTransformCpp m_cUpdater;

    bool m_bShowWhenActivated;
    bool m_bHideWhenInactivated;
};

//Note: this can be split into actor and boxlike actor two classes, but now we only need one
//most functions are implemented in C++, so fast and free to call
UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp : public AActor, public IMyWithCurveUpdaterTransformInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp();

    virtual ~AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp();


    virtual MyErrorCodeCommonPartCpp getModelInfo(struct FMyModelInfoCpp& modelInfo, bool verify = true) const override;
    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformEnsured(struct FMyWithCurveUpdaterBasicCpp*& outUpdater) override;

    inline FMyWithCurveUpdaterWorldTransformCpp& getMyWithCurveUpdaterWorldTransformRef()
    {
        return FMyWithCurveUpdaterWorldTransformCpp::castFromBaseRef(getMyWithCurveUpdaterTransformRef());
    };


    inline UMyWithCurveUpdaterWorldTransformComponent* getMyTransformUpdaterComponent() const
    {
        return m_pMyTransformUpdaterComponent;
    };

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
        static void helperTestAnimationStep(float time, FString debugStr, const TArray<AMyWithCurveUpdaterWorldTransformBoxLikeActorBaseCpp*>& actors);

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
        UMyWithCurveUpdaterWorldTransformComponent* m_pMyTransformUpdaterComponent;

    UPROPERTY(EditDefaultsOnly, Category = "My Helper", meta = (DisplayName = "fake button to update settings"))
        bool m_bFakeUpdateSettings;

private:
    void createComponentsForCDO();
};


USTRUCT(BlueprintType)
struct FMyWithCurveUpdateStepDataWidgetTransformCpp : public FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataWidgetTransformCpp() : Super()
    {
        reset(true);

        m_sClassName = TEXT("FMyWithCurveUpdateStepDataWidgetTransformCpp");
        m_iType = MyTransformTypeWidgetTransform;
    };

    virtual ~FMyWithCurveUpdateStepDataWidgetTransformCpp()
    {
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        FWidgetTransform t;
        m_cWidgetTransformStart = m_cWidgetTransformEnd = t;
    };

    virtual FMyWithCurveUpdateStepDataWidgetTransformCpp* createOnHeap() override
    {
        return new FMyWithCurveUpdateStepDataWidgetTransformCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataWidgetTransformCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataWidgetTransformCpp *>(&other);
        *this = *pOther;
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform start"))
        FWidgetTransform m_cWidgetTransformStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform end"))
        FWidgetTransform m_cWidgetTransformEnd;
};


struct FMyWithCurveUpdaterWidgetTransformCpp : public FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataWidgetTransformCpp>
{

public:
    FMyWithCurveUpdaterWidgetTransformCpp() : FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataWidgetTransformCpp>()
    {
        m_iType = MyTransformTypeWidgetTransform;
    };

    virtual ~FMyWithCurveUpdaterWidgetTransformCpp()
    {

    };

    inline static const FMyWithCurveUpdaterWidgetTransformCpp& castFromBaseRefConst(const FMyWithCurveUpdaterBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeWidgetTransform) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdaterWidgetTransformCpp&>(base);
    };

    inline static FMyWithCurveUpdaterWidgetTransformCpp& castFromBaseRef(FMyWithCurveUpdaterBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdaterWidgetTransformCpp&>(castFromBaseRefConst(base));
    };

    inline void reset()
    {
        FMyWithCurveUpdaterBasicCpp::reset();
    };
};


//a widget support animation setup at runtime, since default widget's animation is static
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp : public UUserWidget, public IMyWithCurveUpdaterTransformInterfaceCpp, public IMyWidgetSizeInterfaceCpp
{
    GENERATED_BODY()

public:
    UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_cUpdater.m_cCommonUpdateDelegate.BindUObject(this, &UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::updaterOnCommonUpdate);
        m_cUpdater.m_cCommonFinishDelegete.BindUObject(this, &UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::updaterOnCommonFinish);
        m_cUpdater.m_cActivateTickDelegate.BindUObject(this, &UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp::updaterActivateTick);

        m_bUpdaterNeedTick = false;

        m_bShowWhenActivated = false;
        m_bHideWhenInactivated = false;
    };

    virtual ~UMyWithCurveUpdaterWidgetTransformBoxLikeWidgetBaseCpp()
    {

    };

    virtual MyErrorCodeCommonPartCpp getModelInfo(FMyModelInfoCpp& modelInfo, bool verify = true) const override
    {
        modelInfo.reset(MyModelInfoType::Box2D);
        MyErrorCodeCommonPartCpp ret = IMyWidgetSizeInterfaceCpp::Execute_getLocalSize(this, modelInfo.getBox2DRef().m_cBoxExtend);

        if (verify) {
            MY_VERIFY(ret == MyErrorCodeCommonPartCpp::NoError);
        }
        return ret;
    };

    virtual MyErrorCodeCommonPartCpp getMyWithCurveUpdaterTransformEnsured(FMyWithCurveUpdaterBasicCpp*& outUpdater) override
    {
        outUpdater = &m_cUpdater;
        return MyErrorCodeCommonPartCpp::NoError;
    };

    inline FMyWithCurveUpdaterWidgetTransformCpp& getMyWithCurveUpdaterWidgetTransformRef()
    {
        return FMyWithCurveUpdaterWidgetTransformCpp::castFromBaseRef(getMyWithCurveUpdaterTransformRef());
    };


    UFUNCTION(BlueprintCallable)
        int32 updaterAddStepToTail(const FMyWithCurveUpdateStepDataWidgetTransformCpp& data)
    {
        return m_cUpdater.addStepToTail(data);
    };

    UFUNCTION(BlueprintCallable)
        void updaterClearSteps()
    {
        m_cUpdater.clearSteps();
    }

    inline FMyWithCurveUpdaterWidgetTransformCpp& getUpdaterRef()
    {
        return m_cUpdater;
    };

protected:

    IMyWidgetSizeInterfaceCpp_DefaultEmptyImplementationForUObject()

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    void updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector);
    void updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data);
    void updaterActivateTick(bool activate, FString debugString);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "show when activated"))
        bool m_bShowWhenActivated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "hide when Inactivated"))
        bool m_bHideWhenInactivated;

    FMyWithCurveUpdaterWidgetTransformCpp m_cUpdater;
    bool m_bUpdaterNeedTick;
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
struct FMyWithCurveUpdateStepSettingsWorldTransformCpp : public FMyWithCurveUpdateStepSettingsBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepSettingsWorldTransformCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeWorldTransform;
    };

    inline static const FMyWithCurveUpdateStepSettingsWorldTransformCpp& castFromBaseConst(const FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeWorldTransform) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepSettingsWorldTransformCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepSettingsWorldTransformCpp& castFromBase(FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepSettingsWorldTransformCpp&>(castFromBaseConst(base));
    };

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
struct FMyWithCurveUpdateStepSettingsWidgetTransformCpp : public FMyWithCurveUpdateStepSettingsBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyWithCurveUpdateStepSettingsWidgetTransformCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeWidgetTransform;
    };

    inline static const FMyWithCurveUpdateStepSettingsWidgetTransformCpp& castFromBaseConst(const FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        if (base.getType() != MyTransformTypeWidgetTransform) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyWithCurveUpdateStepSettingsWidgetTransformCpp&>(base);
    };

    inline static FMyWithCurveUpdateStepSettingsWidgetTransformCpp& castFromBase(FMyWithCurveUpdateStepSettingsBasicCpp& base)
    {
        return const_cast<FMyWithCurveUpdateStepSettingsWidgetTransformCpp&>(castFromBaseConst(base));
    };

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
struct FMyTransformUpdateAnimationMetaBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTransformUpdateAnimationMetaBaseCpp()
    {
        reset();
        m_iType = MyTypeUnknown;
    };

    virtual ~FMyTransformUpdateAnimationMetaBaseCpp()
    {

    };

    inline void reset()
    {
        m_fTotalTime = 0;
        m_cModelInfo.reset();
        m_sDebugString.Reset();
    };

    inline int32 getType() const
    {
        return m_iType;
    }

    float m_fTotalTime;
    FMyModelInfoCpp m_cModelInfo;
    FString m_sDebugString;

protected:

    int32 m_iType;
};

USTRUCT()
struct FMyTransformUpdateAnimationMetaWorldTransformCpp : public FMyTransformUpdateAnimationMetaBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTransformUpdateAnimationMetaWorldTransformCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeWorldTransform;
    };

    inline static const FMyTransformUpdateAnimationMetaWorldTransformCpp& castFromBaseRefConst(const FMyTransformUpdateAnimationMetaBaseCpp& base)
    {
        if (base.getType() != MyTransformTypeWorldTransform) {
            MY_VERIFY(false);
        }

        return StaticCast<const FMyTransformUpdateAnimationMetaWorldTransformCpp&>(base);
    };

    inline static FMyTransformUpdateAnimationMetaWorldTransformCpp& castFromBaseRef(FMyTransformUpdateAnimationMetaBaseCpp& base)
    {
        return const_cast<FMyTransformUpdateAnimationMetaWorldTransformCpp&>(castFromBaseRefConst(base));
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }
        FTransform zeroT;

        m_cPointTransform = zeroT;
        m_cDisappearTransform = zeroT;

    };

    FTransform m_cPointTransform;
    FTransform m_cDisappearTransform;

};


USTRUCT()
struct FMyTransformUpdateAnimationMetaWidgetTransformCpp : public FMyTransformUpdateAnimationMetaBaseCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyTransformUpdateAnimationMetaWidgetTransformCpp() : Super()
    {
        reset(true);
        m_iType = MyTransformTypeWidgetTransform;
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }
    };
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
    FWidgetTransform m_cWidgetTransformNormal;

    //If true, that transform will be applied when pressed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "enable render transform pressed"))
    bool m_bEnableWidgetTransformPressed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = m_bEnableWidgetTransformPressed, DisplayName = "render transform pressed"))
    FWidgetTransform m_cWidgetTransformPressed;
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

        m_cPaddingPercent.reset();

        m_cElemSpacingPercent = FVector::ZeroVector;
        m_cGroupSpacingPercent = FVector::ZeroVector;
    };

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "elem size"))
    FVector m_cElemSize;

    //the padding to elem that will be arranged, unit is elem size's precent
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "padding percent"))
    FMyMargin3D m_cPaddingPercent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "elem spacing percent"))
    FVector m_cElemSpacingPercent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "group spacing percent"))
    FVector m_cGroupSpacingPercent;
};


//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyRenderUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static void helperResolveWorldTransformFromPointAndCenterMetaOnPlayerScreenConstrained(const UObject* WorldContextObject, const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp &meta,
                                                                                            float targetPosiFromCenterToBorderOnScreenPercent,
                                                                                            const FVector2D& targetPosiFixOnScreenPercent,
                                                                                            float targetVOnScreenPercent,
                                                                                            float targetModelHeightInWorld,
                                                                                            FTransform &outTargetTranform);

    static float helperGetRemainTimePercent(const TArray<FMyWithCurveUpdateStepSettingsWorldTransformCpp>& stepDatas);

    static void helperSetupWorldTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaWorldTransformCpp& meta,
                                                             const FMyWithCurveUpdateStepSettingsWorldTransformCpp& stepData,
                                                             const TArray<FMyWithCurveUpdaterWorldTransformCpp *>& updatersSorted);

    static void helperSetupTransformUpdateAnimationStep(const FMyTransformUpdateAnimationMetaBaseCpp& meta,
                                                        const FMyWithCurveUpdateStepSettingsBasicCpp& stepData,
                                                        const TArray<FMyWithCurveUpdaterBasicCpp *>& updatersSorted);

    static void helperSetupTransformUpdateAnimationSteps(const FMyTransformUpdateAnimationMetaBaseCpp& meta,
                                                         const TArray<const FMyWithCurveUpdateStepSettingsBasicCpp *>& stepDatas,
                                                         const TArray<FMyWithCurveUpdaterBasicCpp *>& updatersSorted);



    static void helperSetupWorldTransformUpdateAnimationStepsForPoint(const UObject* WorldContextObject,
                                                                        float totalDur,
                                                                        const FMyPointAndCenterMetaOnPlayerScreenConstrainedCpp& pointAndCenterMeta,
                                                                        const FMyPointFromCenterAndLengthInfoOnPlayerScreenConstrainedCpp& pointInfo,
                                                                        const TArray<FMyWithCurveUpdateStepSettingsWorldTransformCpp>& stepDatas,
                                                                        float extraDelayDur,
                                                                        const TArray<IMyWithCurveUpdaterTransformInterfaceCpp *>& updaterInterfaces,
                                                                        FString debugName,
                                                                        bool clearPrevSteps);

    //a step take 100% of time @waitTime, will be added
    static void helperAddWaitStep(float waitTime, FString debugStr, const TArray<FMyWithCurveUpdaterBasicCpp *>& updaters);
    static void helperAddWaitStep(float waitTime, FString debugStr, const TArray<IMyWithCurveUpdaterTransformInterfaceCpp *>& updaterInterfaces);


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

    // -------------
    // | E   EE   E
    // | E   E*
    //calc the * coordinate, ignore * self's occupization and margin
    static void myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupWalked, const FIntVector& elemWalked, FVector& distanceIgnorePadding);
};