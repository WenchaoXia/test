// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MyMJGameCard.generated.h"

#define MyCardStaticMeshMIDParamInBaseColor (TEXT("InBaseColor"))

#define MyCardAssetPartialNameStaticMesh (TEXT("cardBox"))
#define MyCardAssetPartialNameStaticMeshDefaultMI (TEXT("cardBoxMat0_defaultInst"))

#define MyCardAssetPartialNamePrefixValueNormal (TEXT("v%02d"))
#define MyCardAssetPartialNamePrefixValueUnknown (TEXT("vUnknown"))
#define MyCardAssetPartialNamePrefixValueMiss (TEXT("vMiss"))

#define MyCardAssetPartialNameSuffixValueBaseColorTexture (TEXT("_baseColor"))

#define MyMJGameCardBaseCppDefaultShowingValue (0)

USTRUCT(BlueprintType)
struct FMyMJGameCardVisualInfoCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardVisualInfoCpp()
    {
        m_iIdxAttender = 0;
        m_eSlot = MyMJCardSlotTypeCpp::Invalid;
        m_iIdxRow = 0;
        m_iIdxColInRow = 0;
        //m_iCountOfColInRow = 0;
        m_iIdxStackInCol = 0;
        m_eFlipState = MyMJCardFlipStateCpp::Invalid;
        m_iXRotate90D = 0;
        m_iXRotate90DBeforeCount = 0;
        m_iColInRowExtraMarginCount = 0;
    };

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx attender"))
        int32 m_iIdxAttender;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card slot"))
        MyMJCardSlotTypeCpp m_eSlot;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx row"))
    int32 m_iIdxRow;

    //when colInRow Aligment is left or mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx col in row"))
    int32 m_iIdxColInRow;

    //UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "count of col in row"))
    //int32 m_iCountOfColInRow;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "idx stack in col"))
    int32 m_iIdxStackInCol;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "flip state"))
    MyMJCardFlipStateCpp m_eFlipState;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "X Rotate 90D"))
    int32 m_iXRotate90D;

    //when colInRow Aligment is left mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "X Rotate 90D before count"))
    int32 m_iXRotate90DBeforeCount;

    //when colInRow Aligment is left mid, this count from left to right, othwise reverse
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "col in row extra margin count"))
    int32 m_iColInRowExtraMarginCount;
};

//model always and must facing x axis
USTRUCT(BlueprintType)
struct FMyMJGameCardActorModelInfoCpp

{
    GENERATED_USTRUCT_BODY()

public:
    FMyMJGameCardActorModelInfoCpp() : m_cBoxExtend(0), m_cCenterPointRelativeLocation(0)
    {
    };

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "box extend final"))
    FVector m_cBoxExtend;

    //final size after all actor scale, component scale applied
    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "center point final relative location"))
    FVector m_cCenterPointRelativeLocation;

};

UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    //AMyMJGameCardBaseCpp(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    void getModelInfo(FMyMJGameCardActorModelInfoCpp& modelInfo);

    UFUNCTION(BlueprintSetter)
    void setValueShowing(int32 newValue);

    UFUNCTION(BlueprintGetter)
    int32 getValueShowing() const;

    UFUNCTION(BlueprintSetter)
    void setResPath(const FDirectoryPath& newResPath);

    UFUNCTION(BlueprintGetter)
    const FDirectoryPath& getResPath() const;

protected:

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostInitializeComponents() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    void createComponentsForCDO();
    int32 checkAndLoadCardBasicResources(const FString &inPath);

    //the update rule is: always update if new settings arrive, and always reflect it even fail, never revert values
    int32 updateVisual();
    int32 updateWithCardBasicResources();
    int32 updateWithValue(int32 newValue);

    int32 updateCardStaticMeshMIDParams(class UTexture* InBaseColor);

    //return true if all res loaded
    bool helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture);


    //important values:
    //value showing now
    UPROPERTY(EditAnywhere, BlueprintSetter = setValueShowing, BlueprintGetter = getValueShowing, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;


    //components
    //root scene
    UPROPERTY(VisibleAnywhere, Instanced, meta = (DisplayName = "root scene"))
    class USceneComponent *m_pRootScene;

    UPROPERTY(VisibleAnywhere, Instanced, meta = (DisplayName = "card box"))
    class UBoxComponent *m_pCardBox;

    UPROPERTY(VisibleAnywhere, Instanced, meta = (DisplayName = "card static mesh"))
    class UStaticMeshComponent *m_pCardStaticMesh;


    //resouce settings, the child calss should specify them
    //Note: only one of ContentDir or RelativeToGameContentDir need to be specified to resulting relative path, their difference is dialog 
    //UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "card static mesh", ContentDir = "true", RelativeToGameContentDir = "true"))

    //where the card resource is, have special requirement such as mesh, material, texture, etc. example: /Game/Art/Models/MJCard/Type0
    UPROPERTY(EditDefaultsOnly, BlueprintSetter = setResPath, BlueprintGetter = getResPath, meta = (DisplayName = "resource path", ContentDir = "true"))
    FDirectoryPath m_cResPath;

    UPROPERTY(BlueprintReadOnly)
    class UStaticMesh *m_pResMesh;

    UPROPERTY(BlueprintReadOnly)
    class UMaterialInstance *m_pResMI;



};