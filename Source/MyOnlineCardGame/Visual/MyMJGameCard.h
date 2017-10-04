// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MyMJGameCard.generated.h"



UCLASS(Blueprintable)
class MYONLINECARDGAME_API AMyMJGameCardBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameCardBaseCpp();

    virtual ~AMyMJGameCardBaseCpp();

    //nomatter whether collsion is enabled, it just return the current size
    const class UBoxComponent& getCollisionBoxRef() const;

    //return 0 if no error happens and OK to do visual operations, even mode not changed since it equal to old fasion, otherwise errorcode
    //@modelAssetPath example /Game/Art/Models/MJCard/Type0/cardBox/
    UFUNCTION(BlueprintCallable)
    int32 changeVisualModelType(const FString &modelAssetPath);

    UFUNCTION(BlueprintCallable)
    int32 changeVisualValue(int32 newValue);


protected:

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostInitializeComponents() override;

    void createAndInitComponents();
    int32 updateCardStaticMeshMIDParams(class UTexture* InBaseColor);

    int32 changeVisualModelTypeInternal(const FString &modelAssetPath, bool bInConstruct);
    int32 changeVisualValueInternal(int32 newValue, bool bInConstruct, bool bIgnoreValueCompare);

    //return true if all res loaded
    bool helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutBaseColorTexture);


    //component doesn't need uproperty
    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card box"))
    class USceneComponent *m_pRootScene;

    class UBoxComponent *m_pCardBox;

    //UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card static mesh"))
    class UStaticMeshComponent *m_pCardStaticMesh;

    UPROPERTY()
    class UMaterialInstance* m_pResCardStaticMeshMITarget;

    //for some reason, MID can't survive across ctor process
    //UPROPERTY()
    //class UMaterialInstanceDynamic* m_pResCardStaticMeshMIDTarget;

    UPROPERTY()
    class UTexture* m_pResCardStaticMeshMIDParamInBaseColorTarget;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "value showing"))
    int32 m_iValueShowing;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "model asset path"))
    FString m_sModelAssetPath;

    int32 m_iError;
};