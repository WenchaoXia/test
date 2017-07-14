// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "GameFramework/Actor.h"
#include "MJLocalCS/MyMJGameCoreLocalCS.h"

#include "MyMJBPUtils.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyTestActorBaseCpp : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Client, unreliable)
        void ClientRPCFunction0(float v1);


    UFUNCTION(BlueprintCallable, Server, unreliable, WithValidation)
        void ServerRPCFunction0(float v1);

    UFUNCTION(BlueprintCallable, NetMulticast, unreliable)
        void MulticastRPCFunction0(float v1);

    UFUNCTION(BlueprintCallable)
    void genPusherPointers(FMyMJGamePusherPointersCpp &pusherPointers);

    UFUNCTION(BlueprintCallable, Client, unreliable)
    void testRPCWithPusherPointers(const FMyMJGamePusherPointersCpp &pusherPointers);
};

UCLASS()
class MYONLINECARDGAME_API AMyTestParentClass0 : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY()
        int32 m_iTest;

};

UCLASS()
class MYONLINECARDGAME_API AMyTestChildClass0 : public AMyTestParentClass0
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int32 m_iTestChild;


};


UCLASS()
class UMyMJBPUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void testArrayPointerSerialize(AMyTestParentClass0 *pInParent, AMyTestChildClass0 *pInChild, AMyTestParentClass0 *pOutParent, int32 param);


    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void testPusherSerialize0(int32 param);


};