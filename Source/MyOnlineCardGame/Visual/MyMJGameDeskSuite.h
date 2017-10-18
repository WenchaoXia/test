// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

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

    UFUNCTION(BlueprintNativeEvent)
    void retrieveVisualPointByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp slot, int32& errCode, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const;

    void retrieveVisualPointByIdxAttenderAndSlot_Implementation(int32 idxAttender, MyMJCardSlotTypeCpp slot, int32& errCode, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const
    {
        //must be override
        MY_VERIFY(false);
    };

    void getVisualPointByIdxAttenderAndSlotInternal(int32 idxAttender, MyMJCardSlotTypeCpp slot, int32& errCode, FMyMJGameDeskVisualPointCfgCpp &visualPoint)
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
        MY_VERIFY((uint8)slot < 0xff);

        int32 key = ((idxAttender & 0xff) << 8) & ((uint8)slot & 0xff);

        const FMyMJGameDeskVisualPointCfgCpp *pV = m_mVisualPointCache.Find(key);
        if (pV) {
            visualPoint = *pV;
            errCode = 0;
            return;
        }

        retrieveVisualPointByIdxAttenderAndSlot(idxAttender, slot, errCode, visualPoint);
        if (errCode == 0) {
            FMyMJGameDeskVisualPointCfgCpp& newAdded = m_mVisualPointCache.Add(key);
            newAdded = visualPoint;
        }

    };

protected:



    TMap<int32, FMyMJGameDeskVisualPointCfgCpp> m_mVisualPointCache;
};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API UMyMJGameDeskDynamicResManagerCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJGameDeskDynamicResManagerCpp();
    virtual ~UMyMJGameDeskDynamicResManagerCpp();

    bool checkSettings() const;

    FMyMJGameCardActorModelInfoCpp& getCardModelInfoUnscaled();

    UFUNCTION(BlueprintSetter)
    void setCfgCardModelAssetPath(const FString& inPath);

    UFUNCTION(BlueprintGetter)
    const FString& getCfgCardModelAssetPath() const;

    UPROPERTY()
    TArray<AMyMJGameCardBaseCpp*> m_aCards;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "cfg card class"))
    TSubclassOf<AMyMJGameCardBaseCpp> m_cCfgCardClass;

protected:

    void prepareCardActor();

    //it seems UBT have bug which require declare sequence
    //UPROPERTY(BlueprintSetter = setCfgCardModelAssetPath, BlueprintSetter = getCfgCardModelAssetPath, EditAnywhere, meta = (DisplayName = "cfg card model asset path"))

    //example: /Game/Art/Models/MJCard/Type0/cardBox/
    UPROPERTY(EditAnywhere, BlueprintSetter = setCfgCardModelAssetPath, BlueprintGetter = getCfgCardModelAssetPath, meta = (DisplayName = "cfg card model asset path"))
    FString m_sCfgCardModelAssetPath;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card model mesh asset"))
        class UStaticMesh *m_pCardModelMeshAsset;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "card model material default instance asset"))
        class UMaterialInstance *m_pCardModelMaterialDefaultInstanceAsset;

    FMyMJGameCardActorModelInfoCpp* m_pActorModelInfoCached;
};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameDeskSuiteCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameDeskSuiteCpp();

    virtual ~AMyMJGameDeskSuiteCpp();



    void helperCalcCardTransform(const FMyMJGameCardVisualInfoCpp& cCardVisualInfo, FTransform &outTransform);

    UFUNCTION(BlueprintCallable)
    static int32 helperCalcCardTransformFromvisualPointCfg(const FMyMJGameCardActorModelInfoCpp& cardModelInfoFinal, const FMyMJGameCardVisualInfoCpp& cardVisualInfoFinal, const FMyMJGameDeskVisualPointCfgCpp& visualPointCfg, FVector& outLocationWorld, FRotator& outRotatorWorld);

protected:

    virtual void PostInitializeComponents() override;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, meta = (DisplayName = "res manager"))
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, meta = (DisplayName = "res manager"))
    UMyMJGameDeskDynamicResManagerCpp *m_pResManager;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "area actor"))
    AMyMJGameDeskAreaCpp* m_pDeskAreaActor;

    UPROPERTY(EditAnywhere, meta = (MakeEditWidget, DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;


};