// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameCard.h"

#include "MyMJGameDeskSuite.generated.h"

UENUM()
enum class MyMJGameHorizontalAlignmentCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Left = 1     UMETA(DisplayName = "Left"),
    Mid = 2    UMETA(DisplayName = "Mid"),
    Right = 3    UMETA(DisplayName = "Right")
};

UENUM()
enum class MyMJGameVerticalAlignmentCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    Bottom = 1     UMETA(DisplayName = "Bottom"),
    Mid = 2    UMETA(DisplayName = "Mid"),
    Top = 3    UMETA(DisplayName = "Top")
};


//Can change in runtime, and caller should invalidate cache after change
USTRUCT(BlueprintType)
struct FMyMJGameDeskVisualPointCfgCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameDeskVisualPointCfgCpp() : m_cAreaBoxExtendFinal(0)
    {
        m_eRowAlignment = MyMJGameVerticalAlignmentCpp::Invalid;
        //m_iRowMaxNum = 0;
        m_eColInRowAlignment = MyMJGameHorizontalAlignmentCpp::Invalid;
        //m_iColInRowMaxNum = 0;

        m_fColInRowExtraMarginAbs = 0;
    };

    virtual ~FMyMJGameDeskVisualPointCfgCpp()
    {

    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point world transform"))
    FTransform m_cCenterPointWorldTransform;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "area box extend final"))
    FVector m_cAreaBoxExtendFinal;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row alignment"))
    MyMJGameVerticalAlignmentCpp m_eRowAlignment;

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "row max num"))
    //int32 m_iRowMaxNum;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row alignment"))
    MyMJGameHorizontalAlignmentCpp m_eColInRowAlignment;

    //assumed card num, if exceed the position may be out of range but will not crash the program
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "column in row max num"))
    //int32 m_iColInRowMaxNum;

    //must >= 0
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin abs"))
    float m_fColInRowExtraMarginAbs;

};

UCLASS(Blueprintable, Abstract)
class MYONLINECARDGAME_API AMyMJGameDeskAreaCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameDeskAreaCpp() : Super()
    {

    };

    virtual ~AMyMJGameDeskAreaCpp()
    {

    };

    inline
    void invalidVisualPointCache()
    {
        m_mVisualPointCache.Reset();
    };

    int32 getVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyMJGameDeskVisualPointCfgCpp &visualPoint)
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
        MY_VERIFY((uint8)eSlot < 0xff);

        int32 key = ((idxAttender & 0xff) << 8) & ((uint8)eSlot & 0xff);

        const FMyMJGameDeskVisualPointCfgCpp *pV = m_mVisualPointCache.Find(key);
        if (pV) {
            visualPoint = *pV;
            return 0;
        }

        int32 errCode = retrieveVisualPointByIdxAttenderAndSlot(idxAttender, eSlot, visualPoint);
        if (errCode == 0) {
            FMyMJGameDeskVisualPointCfgCpp& newAdded = m_mVisualPointCache.Add(key);
            newAdded = visualPoint;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getVisualPointCfgByIdxAttenderAndSlot() failed, idxAttender %d, eSlot %d, errorCode: %d."), idxAttender, (uint8)eSlot, errCode);
        }

        return errCode;
    };

protected:

    //return errcode, 0 means no error
    UFUNCTION(BlueprintNativeEvent)
    int32 retrieveVisualPointByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp slot, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const;

    int32 retrieveVisualPointByIdxAttenderAndSlot_Implementation(int32 idxAttender, MyMJCardSlotTypeCpp slot, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const
    {
        //must be override
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDeskAreaCpp::retrieveVisualPointByIdxAttenderAndSlot() must be overrided by blueprint subclass!"));
        MY_VERIFY(false);
        return 0;
    };

    TMap<int32, FMyMJGameDeskVisualPointCfgCpp> m_mVisualPointCache;
};

UCLASS(Blueprintable, NotBlueprintType)
class MYONLINECARDGAME_API UMyMJGameDeskDynamicResManagerCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJGameDeskDynamicResManagerCpp();
    virtual ~UMyMJGameDeskDynamicResManagerCpp();

    bool checkSettings() const;

    inline int32 getCardModelInfoUnscaled(FMyMJGameCardActorModelInfoCpp& modelInfo) const
    {
        const AMyMJGameCardBaseCpp* pCDO = getCardCDO();
        if (IsValid(pCDO)) {
            pCDO->getModelInfo(modelInfo);
            return 0;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("pCDO is not valid %p."), pCDO);
            modelInfo.reset();
            return -1;
        }
    };

    //return null if it is not spawned yet, user should call prepareCardActor() to create it.
    UFUNCTION(BlueprintCallable)
    AMyMJGameCardBaseCpp* getCardActorByIdx(int32 idx)
    {
        MY_VERIFY(idx >= 0);
        if (idx < m_aCards.Num()) {
            return m_aCards[idx];
        }

        return NULL;
    }

    UFUNCTION(BlueprintCallable)
    int32 prepareCardActor(int32 count2reach);


protected:

    const AMyMJGameCardBaseCpp* getCardCDO() const;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "cards"))
    TArray<AMyMJGameCardBaseCpp*> m_aCards;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "cfg card class"))
    TSubclassOf<AMyMJGameCardBaseCpp> m_cCfgCardClass;

    //it seems UBT have bug which require declare sequence
    //UPROPERTY(BlueprintSetter = setCfgCardModelAssetPath, BlueprintSetter = getCfgCardModelAssetPath, EditAnywhere, meta = (DisplayName = "cfg card model asset path"))
    //UPROPERTY(EditAnywhere, BlueprintSetter = setCfgCardModelAssetPath, BlueprintGetter = getCfgCardModelAssetPath, meta = (DisplayName = "cfg card model asset path"))

};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameDeskSuiteCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameDeskSuiteCpp();

    virtual ~AMyMJGameDeskSuiteCpp();

    //warn: this function wouldn't update visual info automatically
    UFUNCTION(BlueprintCallable)
    void helperResolveTargetCardVisualState(int32 idxCard, FMyMJGameCardVisualStateCpp& outTargetCardVisualState);

protected:

    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;

    bool checkSettings() const;

    //return errcode
    int32 helperGetColCountPerRowForDefaultAligment(int32 idxAtttender, MyMJCardSlotTypeCpp eSlot, int32& outCount) const;
    void helperTryUpdateCardVisualInfoPack();


    //blue print can call this to help verify test
    UFUNCTION(BlueprintCallable)
    static int32 helperCalcCardTransformFromvisualPointCfg(const FMyMJGameCardActorModelInfoCpp& cardModelInfoFinal, const FMyMJGameCardVisualInfoCpp& cardVisualInfoFinal, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FTransform& outTransform);
    //static int32 helperCalcCardTransformFromvisualPointCfg(const FMyMJGameCardActorModelInfoCpp& cardModelInfoFinal, const FMyMJGameCardVisualInfoCpp& cardVisualInfoFinal, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FVector& outLocationWorld, FRotator& outRotatorWorld);


    //UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, meta = (DisplayName = "res manager"))
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Instanced, meta = (DisplayName = "res manager"))
    UMyMJGameDeskDynamicResManagerCpp *m_pResManager;

    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Instanced, meta = (DisplayName = "core with visual"))
    UMyMJGameCoreWithVisualCpp *m_pCoreWithVisual;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "area actor"))
    AMyMJGameDeskAreaCpp* m_pDeskAreaActor;

    UPROPERTY()
    UMyMJDataSequencePerRoleCpp* m_pDataHistoryBuffer;

    UPROPERTY()
    UMyTestObject* m_pTestObj;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "MJ Core actor"))
    //UMyMJGameCoreWithVisualCpp* m_pMJCore;

    FMyMJCardVisualInfoPackCpp m_cCardVisualInfoPack;

    //UPROPERTY(VisibleAnywhere, meta = (MakeEditWidget, DisplayName = "root scene"))
    //class USceneComponent *m_pRootScene;


};