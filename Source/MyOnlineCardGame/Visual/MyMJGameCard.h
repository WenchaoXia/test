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

    //return true if no error happens and OK to do visual operations, even mode not changed since it equal to old fasion
    //@modelAssetPath example /Game/Art/Models/MJCard/Type0/cardBox/
    UFUNCTION(BlueprintCallable)
    bool changeModelType(const FString &modelAssetPath);

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card box"))
    class UBoxComponent *m_pCardBox;

    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "card static mesh"))
    class UStaticMeshComponent *m_pCardStaticMesh;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "model asset path"))
    FString m_sModelAssetPath;
};