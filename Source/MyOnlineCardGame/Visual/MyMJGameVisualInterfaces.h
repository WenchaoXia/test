// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "MJBPEncap/utils/MyMJBPUtils.h"

#include "MyMJGameVisualCommon.h"

#include "MyMJGameVisualInterfaces.generated.h"

//It seems UE4 have a bug that all function must have return value to let blueprint implement it
//All API here return error code, and 0 is returned if OK, if implementable by blueprint



UINTERFACE()
class UMyMJGameInRoomDeskInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

//by default, the API's card actors param passed in, are attender related, except those have tipped not only attender's 
class IMyMJGameInRoomDeskInterfaceCpp
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderThrowDices(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                                 int32 diceVisualStateKey,
                                                 const TArray<class AMyMJGameDiceBaseCpp *>& aDices);


    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderCardsDistribute(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                      const TArray<int32>& aIdsHandCards, bool isLastDistribution,
                                      const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsDistributed,
                                      const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving);

    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderTakeCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                 const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsTaken);
    

    //@handCardCount is used to tip how many cards left, and the implemention can use it to detect if this card is injecting(jump into the cards stacks)
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderGiveOutCards(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                    int32 handCardsCount,
                                    const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsGivenOut,
                                    const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving);


    //All card actors passed in, are attender related
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                            MyMJGameEventVisualTypeCpp weaveVisualType, const struct FMyMJWeaveCpp& weave,
                            const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving);
};


UINTERFACE(NotBlueprintable)
class UMyMJGameInRoomUIMainWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomUIMainWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    //some important event happend, like game start or end
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp showImportantGameStateUpdated(float dur, MyMJGameStateCpp newGameState);

    //return error code, 0 means OK
    virtual FMyErrorCodeMJGameCpp showAttenderWeave(float dur, int32 idxAttender, MyMJGameEventVisualTypeCpp weaveVisualType) = NULL;


    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPublicChanged(int32 idxAttender, const FMyMJRoleDataAttenderPublicCpp& dataAttenderPublic, int32 subType) = NULL;

    virtual FMyErrorCodeMJGameCpp showMyMJRoleDataAttenderPrivateChanged(int32 idxAttender, const FMyMJRoleDataAttenderPrivateCpp& dataAttenderPrivate, int32 subType) = NULL;

    //max is always 4
    virtual FMyErrorCodeMJGameCpp updateAttenderPositions(float XYRatioOfplayerScreen, const TArray<FVector2D>& projectedPointsInPlayerScreen_unit_absolute) = NULL;
    //{
        //UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("changeDeskPositionOfIdxScreenPosition0 must be override!"));
        //MY_VERIFY(false);
        //return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByChildClass);
    //};

protected:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getInRoomPlayerInfoWidgetByIdxAttender(int32 idxAttender, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getCfg(FMyMJGameInRoomUIMainWidgetCfgCpp& outCfg);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getOperationRootPanelWidget(class UMyMJGameInRoomOperationRootPanelWidgetBaseCpp*& widget);

    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp changeStateCanGiveCmd(bool canGiveCmd);

    UFUNCTION(BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp changeStateNeedGiveOutCards(bool needGiveOutCards);
};


#define IMyMJGameInRoomUIMainWidgetInterfaceCpp_DefaultImplementationForUObject_Bp() \
protected: \
virtual FMyErrorCodeMJGameCpp showImportantGameStateUpdated_Implementation(float dur, MyMJGameStateCpp newGameState) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: showImportantGameStateUpdated only implemented in C++."), *GetClass()->GetName()); \
    return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint); \
}; \
\
virtual MyErrorCodeCommonPartCpp getInRoomPlayerInfoWidgetByIdxAttender_Implementation(int32 idxAttender, class UMyMJGameInRoomPlayerInfoWidgetBaseCpp*& outWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getInRoomPlayerInfoWidgetByIdxAttender only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getCfg_Implementation(FMyMJGameInRoomUIMainWidgetCfgCpp& outCfg) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getCfg only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getOperationRootPanelWidget_Implementation(class UMyMJGameInRoomOperationRootPanelWidgetBaseCpp*& widget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getOperationRootPanelWidget only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp changeStateCanGiveCmd_Implementation(bool canGiveCmd) \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: changeStateCanGiveCmd only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp changeStateNeedGiveOutCards_Implementation(bool needGiveOutCards) \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: changeStateNeedGiveOutCards only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected:



UINTERFACE()
class UMyMJGameInRoomChoiceSelectWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setChoiceSelectState(bool state);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getChoiceSelectState(bool &state);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setChoiceFillState(bool state);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getChoiceFillState(bool &state);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp setChoiceInteractiveEnableState(bool state);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp getChoiceInteractiveEnableState(bool &state);
};


#define IMyMJGameInRoomChoiceSelectWidgetInterfaceCpp_DefaultImplementationForUObject_Bp() \
protected: \
virtual MyErrorCodeCommonPartCpp setChoiceSelectState_Implementation(bool state) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setChoiceSelectState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getChoiceSelectState_Implementation(bool &state) override \
{ \
    state = false; \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getChoiceSelectState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp setChoiceFillState_Implementation(bool state) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setChoiceFillState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getChoiceFillState_Implementation(bool &state) override \
{ \
    state = false; \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getChoiceFillState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp setChoiceInteractiveEnableState_Implementation(bool state) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setChoiceInteractiveEnableState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp getChoiceInteractiveEnableState_Implementation(bool &state) override \
{ \
    state = false; \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: getChoiceInteractiveEnableState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected:

/*
UENUM()
enum class MyMJGameInRoomOperationUIType : uint8
{
    None = 0                    UMETA(DisplayName = "None"),
    lv1Action = 1               UMETA(DisplayName = "lv1Action"),
    lv2Chi = 10                 UMETA(DisplayName = "lv2Chi"),
};
*/


USTRUCT(BlueprintType)
struct FMyMJGameActionChoiceDataChiCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyMJGameActionChoiceDataChiCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_aValues.Reset();
        m_iIdxOfTriggerCardInValues = -1;
        m_iIdxOfSelection = -1;
        m_bSelected = false;
        m_bInteractiveEnabled = false;
    };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "values"))
        TArray<int32> m_aValues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "idx of trigger card in values"))
        int32 m_iIdxOfTriggerCardInValues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "idx of selection"))
        int32 m_iIdxOfSelection;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "selected"))
        bool m_bSelected;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Interactive Enabled"))
        bool m_bInteractiveEnabled;
};


UINTERFACE()
class UMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updateChi(bool haveAnySelection, bool selected, bool InteractiveEnabled);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updatePeng(int32 idxOfSelection, bool selected, bool InteractiveEnabled);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updateGang(int32 idxOfSelection, bool selected, bool InteractiveEnabled);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updateBu(int32 idxOfSelection, bool selected, bool InteractiveEnabled);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updateHu(int32 idxOfSelection, bool selected, bool InteractiveEnabled);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updateGuo(int32 idxOfSelection, bool selected, bool InteractiveEnabled);

};

#define IMyMJGameInRoomOperationLvl1ActionPanelWidgetInterfaceCpp_DefaultImplementationForUObject_Bp() \
protected: \
virtual MyErrorCodeCommonPartCpp updateChi_Implementation(bool haveAnySelection, bool selected, bool InteractiveEnabled) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateChi only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp updatePeng_Implementation(int32 idxOfSelection, bool selected, bool InteractiveEnabled) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updatePeng only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp updateGang_Implementation(int32 idxOfSelection, bool selected, bool InteractiveEnabled) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateGang only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp updateBu_Implementation(int32 idxOfSelection, bool selected, bool InteractiveEnabled) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateBu only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp updateHu_Implementation(int32 idxOfSelection, bool selected, bool InteractiveEnabled) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateHu only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp updateGuo_Implementation(int32 idxOfSelection, bool selected, bool InteractiveEnabled) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateGuo only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected: \
;


UINTERFACE()
class UMyMJGameInRoomOperationLvl2ChiPanelWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomOperationLvl2ChiPanelWidgetInterfaceCpp
{
    GENERATED_BODY()

public:

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
        MyErrorCodeCommonPartCpp updateChiChoiceDatas(const TArray<FMyMJGameActionChoiceDataChiCpp>& choiceDatas);
};

#define IMyMJGameInRoomOperationLvl2ChiPanelWidgetInterfaceCpp_DefaultImplementationForUObject_Bp() \
protected: \
virtual MyErrorCodeCommonPartCpp updateChiChoiceDatas_Implementation(const TArray<FMyMJGameActionChoiceDataChiCpp>& choiceDatas) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: updateChiChoiceDatas only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
};


//don't go too deep into UI,where BP is good enough to handle
//C++ try touch data logic only


UINTERFACE()
class UMyMJGameInRoomOperationRootPanelWidgetInterfaceCpp : public UInterface
{
    GENERATED_BODY()
};

class IMyMJGameInRoomOperationRootPanelWidgetInterfaceCpp
{
    GENERATED_BODY()

public:


protected:


    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp get1stLvlActionPanelWidget(int32& idxInSwitchPanel, class UMyMJGameInRoomOperationLvl1ActionPanelWidgetBaseCpp*& outWidget);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp get2ndLvlChiPanelWidget(int32& idxInSwitchPanel, class UMyMJGameInRoomOperationLvl2ChiPanelWidgetBaseCpp*& outWidget);

    //Must be implemented by Blueprint
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    MyErrorCodeCommonPartCpp setChoiceFillState(bool haveAnyChoice);
};


#define IMyMJGameInRoomOperationRootPanelWidgetInterface_DefaultImplementationForUObject_Bp() \
protected: \
virtual MyErrorCodeCommonPartCpp get1stLvlActionPanelWidget_Implementation(int32& idxInSwitchPanel, class UMyMJGameInRoomOperationLvl1ActionPanelWidgetBaseCpp*& outWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: get1stLvlActionPanelWidget only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp get2ndLvlChiPanelWidget_Implementation(int32& idxInSwitchPanel, class UMyMJGameInRoomOperationLvl2ChiPanelWidgetBaseCpp*& outWidget) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: get2ndLvlChiPanelWidget only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
virtual MyErrorCodeCommonPartCpp setChoiceFillState_Implementation(bool haveAnyChoice) override \
{ \
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("%s: setChoiceFillState only implemented in C++."), *GetClass()->GetName()); \
    return MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint; \
}; \
protected: \
;