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
enum class MyLogVerbosity : uint8
{
    None = 0                     UMETA(DisplayName = "None"),
    Log = 10               UMETA(DisplayName = "Log"),
    Display = 20         UMETA(DisplayName = "Display"),
    Warning = 30    UMETA(DisplayName = "Warning"),
    Error = 40    UMETA(DisplayName = "Error"),

};

UCLASS()
class UMyMJBPUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    template< class T >
    static inline T* helperTryFindAndLoadAsset(UObject* outer, const FString &resFullPath)
    {
        //T *pRes = FindObject<T>(outer, *resFullPath);
        T *pRes = NULL;
        if (!IsValid(pRes)) {
            pRes = NULL;
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("find object fail, maybe forgot preload it: %s."), *resFullPath);
            pRes = LoadObject<T>(outer, *resFullPath, NULL, LOAD_None, NULL);
            if (!IsValid(pRes)) {
                pRes = NULL;
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("load object fail: %s."), *resFullPath);
            }

            //T *pRes2 = FindObject<T>(outer, *resFullPath);
            //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("refind object result: %d."), pRes2 != NULL);
        }
        
        return pRes;

        //return (T*)StaticFindObject(T::StaticClass(), Outer, Name, ExactClass);
    }

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
    static void testPusherSerialize0(int32 param);


    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void testGameCoreInLocalThread(int32 seed, bool bAttenderRandomSelectHighPriActionFirst);

    UFUNCTION(BlueprintCallable)
    static float testGetRealTimeSeconds(AActor *actor);

    UFUNCTION(BlueprintCallable)
    static int32 getEngineNetMode(AActor *actor);

  
    static bool haveServerLogicLayer(AActor *actor);
    static bool haveServerNetworkLayer(AActor *actor);
    static bool haveClientVisualLayer(AActor *actor);
    static bool haveClientNetworkLayer(AActor *actor);

    UFUNCTION(BlueprintCallable)
    static bool testLoadAsset(UObject* outer, FString fullPathName);

    UFUNCTION(BlueprintCallable, Category = "MyMJBPUtilsLibrary")
    static void rotateOriginWithPivot(const FTransform& originCurrentWorldTransform, const FVector& pivot2OriginRelativeLocation, const FRotator& originTargetWorldRotator, FTransform& originResultWorldTransform);

    static FString getDebugStringFromEWorldType(EWorldType::Type t);
    static FString getDebugStringFromENetMode(ENetMode t);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log print", AdvancedDisplay = "2"), Category = "Utilities|String")
    static void MyBpLog(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool bPrintToScreen = true, bool bPrintToLog = true, MyLogVerbosity eV = MyLogVerbosity::Display, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);
    //float GetRealTimeSeconds() const;

    //return like /Game/[subpath], or Empty if error
    static FString getClassAssetPath(UClass* pC);
};