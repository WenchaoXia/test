// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/DataAsset.h"

#include "Utils/CardGameUtils/MyCardGameElems.h"

#include "MJ/Utils/MyMJUtils.h"
#include "MJBPEncap/MyMJGameEventBase.h"

#include "MyMJGameVisualCommon.h"
#include "MyMJGameVisualInterfaces.h"

#include "MyMJGameVisualElems.generated.h"


USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoCpp : public FMyArrangeCoordinateWorld3DCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardVisualInfoCpp() : Super()
    {
        reset(true);
    };

    virtual void reset(bool resetSubClassDataOnly = false) override
    {
        if (!resetSubClassDataOnly) {
            Super::reset();
        }

        m_iIdxAttender = 0;
        m_eSlot = MyMJCardSlotTypeCpp::Invalid;
        m_iCardValue = MyCardGameValueUnknown;
    };

    
    inline bool equals(const FMyMJGameCardVisualInfoCpp& other) const
    {
        if (&other == this) {
            return true;
        }

        return Super::equals(other) && m_iIdxAttender == other.m_iIdxAttender && m_eSlot == other.m_eSlot && m_iCardValue == other.m_iCardValue;
    };

    virtual FString ToString() const override
    {
        return Super::ToString() + FString::Printf(TEXT(",posi %d:%s, value %d"), m_iIdxAttender, *UMyCommonUtilsLibrary::getStringFromEnum(TEXT("MyMJCardSlotTypeCpp"), (uint8)m_eSlot), m_iCardValue);
    };


    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx attender"))
        int32 m_iIdxAttender;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card slot"))
        MyMJCardSlotTypeCpp m_eSlot;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card value"))
        int32 m_iCardValue;

};


//seperate the data from actor, to allow subthread handling in the future
USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoAndResultCpp

{
    GENERATED_USTRUCT_BODY()

public:

    inline void reset()
    {
        m_cVisualInfo.reset();
        m_cVisualResult.reset();
    };

    inline
    FString ToString() const
    {
        return m_cVisualInfo.ToString() + m_cVisualResult.ToString();
    };

    //do the final resolve with same point cfg and card model
    static void helperResolveCardVisualResultChanges(const FMyArrangePointCfgWorld3DCpp& cVisualPointCfg,
                                                     const FMyModelInfoBoxWorld3DCpp& cCardModelInfo,
                                                     const TMap<int32, FMyMJGameCardVisualInfoCpp>& mIdCardVisualInfoKnownChanges,
                                                     TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mOutIdCardVisualInfoAndResultAccumulatedChanges);


    UPROPERTY(BlueprintReadOnly)
    FMyMJGameCardVisualInfoCpp  m_cVisualInfo;

    UPROPERTY(BlueprintReadOnly)
    FMyMJGameActorVisualResultBaseCpp m_cVisualResult;
};


UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardActorBaseCpp : public AMyCardGameCardActorBaseCpp, public IMySelectableObjectInterfaceCpp, public IMyDraggableObjectInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyMJGameCardActorBaseCpp() : Super(), m_cTargetToGoHistory(TEXT("card actor TargetToGoHistory"), NULL, NULL, 4)
    {
        FTransform t;

        m_bSelectable = true;
        m_bSelected = false;
        m_cTransformBeforeSelection = t;


        m_cTransformBeginDrag = t;
        m_cProjectedScreenPositionBeginDrag = FVector2D::ZeroVector;
        m_fProjectedDistanceBeginDrag = 0;
        m_bProjectionOKBeginDrag = false;

        m_fSelectedZOffsetPercent = 1;
        m_fDragOutSizeOffsetPercent = 2;
    };

    virtual ~AMyMJGameCardActorBaseCpp()
    {

    };


    inline void addTargetToGoHistory(const FMyMJGameCardVisualInfoAndResultCpp& cTargetToGo)
    {
        if (m_cTargetToGoHistory.isFull()) {
            m_cTargetToGoHistory.removeFromHead(1);
        };

        m_cTargetToGoHistory.addToTail(&cTargetToGo, NULL);
    };

    inline const FMyMJGameCardVisualInfoAndResultCpp* getTargetToGoHistory(int32 idxFromLast, bool bVerifyValid = true) const
    {
        const FMyMJGameCardVisualInfoAndResultCpp* ret;
        int32 idx = m_cTargetToGoHistory.getCount() - 1 - idxFromLast;
        if (idx < m_cTargetToGoHistory.getCount()) {
            ret = m_cTargetToGoHistory.peekAt(idx);
        }
        else {
            ret = NULL;
        }

        if (bVerifyValid)
        {
            MY_VERIFY(ret != NULL);
        }

        return ret;
    };

    float getDragOutSizeOffsetPercent() const
    {
        return m_fDragOutSizeOffsetPercent;
    };


    virtual MyErrorCodeCommonPartCpp setSelected(bool selected) override;
    virtual MyErrorCodeCommonPartCpp getSelected(bool &selected) const override;
    virtual MyErrorCodeCommonPartCpp setIsSelectable(bool selectable) override;
    virtual MyErrorCodeCommonPartCpp getIsSelectable(bool &selectable) const override;

    virtual MyErrorCodeCommonPartCpp markBeginDrag() override;
    virtual MyErrorCodeCommonPartCpp getDataBeginDrag(FTransform& transform, bool& projectionOK, FVector2D& projectedScreenPosition, float& projectedDistance) const override;

    static void helperMyMJGameCardActorBaseToMyTransformUpdaters(const TArray<AMyMJGameCardActorBaseCpp*>& aSub, bool bSort, TArray<IMyWithCurveUpdaterTransformWorld3DInterfaceCpp*> &aBase);

protected:

    virtual void BeginPlay() override;

    void tryMoveTransformForSelection(const FTransform& targetTransform, float dur);


    //where this card should go, but allow it not be there now(should move smoothly there)
    FMyCycleBuffer<FMyMJGameCardVisualInfoAndResultCpp> m_cTargetToGoHistory;


    bool m_bSelectable;
    bool m_bSelected;
    FTransform m_cTransformBeforeSelection;

    FTransform m_cTransformBeginDrag;
    FVector2D m_cProjectedScreenPositionBeginDrag;
    float m_fProjectedDistanceBeginDrag;
    bool m_bProjectionOKBeginDrag;


    //unit is actor model's Z length
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Selected Z Offset Percent"))
    float m_fSelectedZOffsetPercent;

    //unit is actor model's size
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "DragOut Size Offset Percent"))
    float m_fDragOutSizeOffsetPercent;
};


UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyMJGameCardWidgetBaseCpp : public UMyCardGameCardWidgetBaseCpp
{
    GENERATED_BODY()

public:

    UMyMJGameCardWidgetBaseCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {

    };

    virtual ~UMyMJGameCardWidgetBaseCpp()
    {

    };
};

/*
USTRUCT(BlueprintType)
struct FMyMJGameDiceVisualInfoCpp
{
GENERATED_USTRUCT_BODY()

public:
FMyMJGameDiceVisualInfoCpp()
{
reset();
};

inline void reset()
{
m_eFlipState = MyBoxFlipStateCpp::Invalid;
}

UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "dice visual state key"))
int32 m_iDiceVisualStateKey = 0;

UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "dice value"))
int32 m_iDiceValue;
}£»
*/

USTRUCT(BlueprintType)
struct FMyMJGameDiceVisualInfoAndResultCpp

{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameDiceVisualInfoAndResultCpp()
    {
        m_iVisualInfoValue = 0;
    };

    int32  m_iVisualInfoValue;
    FMyMJGameActorVisualResultBaseCpp m_cVisualResult;
};


UCLASS(BlueprintType, Blueprintable, Abstract)
class MYONLINECARDGAME_API AMyMJGameDiceBaseCpp : public AMyCardGameDiceActorBaseCpp
{
    GENERATED_BODY()

public:

    AMyMJGameDiceBaseCpp();

    virtual ~AMyMJGameDiceBaseCpp();


protected:

};


UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyMJGameTrivalDancingActorBaseCpp : public AMyWithCurveUpdaterTransformWorld3DBoxActorBaseCpp
{
    GENERATED_BODY()

public:

    AMyMJGameTrivalDancingActorBaseCpp();

    virtual ~AMyMJGameTrivalDancingActorBaseCpp();

protected:

};