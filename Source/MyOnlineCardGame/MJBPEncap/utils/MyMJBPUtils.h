// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


//#include "GameFramework/Actor.h"
#include "MJLocalCS/MyMJGameCoreLocalCS.h"

#include "MyMJBPUtils.generated.h"


UCLASS(BlueprintType, Blueprintable)
class MYONLINECARDGAME_API AMyTestActorBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyTestActorBaseCpp()
    {

    };


    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

    UFUNCTION(BlueprintCallable)
    void createMJData();

    UFUNCTION(BlueprintCallable, Client, unreliable)
    void ClientRPCFunction0(float v1);

    UFUNCTION(BlueprintCallable, Server, unreliable, WithValidation)
    void ServerRPCFunction0(float v1);

    UFUNCTION(BlueprintCallable, NetMulticast, unreliable)
    void MulticastRPCFunction0(float v1);

    UFUNCTION(BlueprintCallable)
    void testMulticastRPCFunction0(float v1);

    //UFUNCTION(BlueprintCallable)
    //void genPusherPointers(FMyMJGamePusherPointersCpp &pusherPointers);

    //UFUNCTION(BlueprintCallable, Client, unreliable)
    //void testRPCWithPusherPointers(const FMyMJGamePusherPointersCpp &pusherPointers);

    //UFUNCTION(BlueprintCallable, Client, unreliable)
    //void testRPCWithPusherNotify(const FMyMJGamePusherMadeChoiceNotifyCpp &pusherNotify);

    //UFUNCTION(BlueprintCallable, Client, unreliable)
    //void testRPCWithPusherFillIn(const FMyMJGamePusherFillInActionChoicesCpp &pusherFillIn);

protected:

    //UFUNCTION(BlueprintImplementableEvent)
    //    void OnRep_MJData();

    /*
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("OnRep_MJData."));
    };
    */
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

UENUM()
enum class MyMJGameEventVisualTypeCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    WeaveChi = 1     UMETA(DisplayName = "WeaveChi"),
    WeavePeng = 2    UMETA(DisplayName = "WeavePeng"),
    WeaveGang = 3    UMETA(DisplayName = "WeaveGang"),
    WeaveBu = 4      UMETA(DisplayName = "WeaveBu"),

    ActionHu = 10      UMETA(DisplayName = "WeaveBu"),
};

UCLASS()
class UMyMJBPUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    //create one instance on heap, caller need call delete() after usage
    static FMyMJGameCoreCpp* helperCreateCoreByRuleType(MyMJGameRuleTypeCpp eRuleType, int32 iSeed, int32 iTrivalConfigMask)
    {
        FMyMJGameCoreCpp *pRet = NULL;
        if (eRuleType == MyMJGameRuleTypeCpp::LocalCS) {
            pRet = StaticCast<FMyMJGameCoreCpp *>(new FMyMJGameCoreLocalCSCpp(iSeed));
        }
        else if (eRuleType == MyMJGameRuleTypeCpp::GuoBiao) {
            MY_VERIFY(false);
        }
        else {
            MY_VERIFY(false);
        }

        if (pRet) {
            pRet->m_iTrivalConfigMask = iTrivalConfigMask;
        }

        return pRet;
    };

    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void testArrayPointerSerialize(AMyTestParentClass0 *pInParent, AMyTestChildClass0 *pInChild, AMyTestParentClass0 *pOutParent, int32 param);

    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
        static void testSerialize(int32 randomSeed);

    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void testPusherSerialize0(int32 param);


    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void testGameCoreInLocalThread(int32 seed, bool bAttenderRandomSelectHighPriActionFirst);

    UFUNCTION(BlueprintCallable)
    static float testGetRealTimeSeconds(AActor *actor);

    static bool haveServerLogicLayer(AActor *actor);
    static bool haveServerNetworkLayer(AActor *actor);
    static bool haveClientVisualLayer(AActor *actor);
    static bool haveClientNetworkLayer(AActor *actor);

    UFUNCTION(BlueprintCallable)
    static bool testLoadAsset(UObject* outer, FString fullPathName);

    //return "Invalid" if have error
    static MyMJGameEventVisualTypeCpp helperGetEventVisualTypeFromWeave(MyMJGameRuleTypeCpp ruleType, const FMyMJWeaveCpp& weave);

    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void helperGetDiceNumbersFromMask(int32 mask, int32 &number0, int32 &number1);
};