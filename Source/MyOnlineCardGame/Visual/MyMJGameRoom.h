// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameVisualCfg.h"

#include "MyMJGameRoomDataSuite.h"

#include "MyMJGameRoom.generated.h"


#define AMyMJGameRoomVisualLoopTimeMs (17)

#define MY_CARD_ACTOR_NUM_MAX (200)
#define MY_DICE_ACTOR_NUM_MAX (10)

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

    /*
    int32 getVisualPointCfgByIdxAttenderAndSlot(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyArrangePointCfgWorld3DCpp &visualPoint)
    {
        MY_VERIFY(idxAttender >= 0 && idxAttender < 4);
        MY_VERIFY((uint8)eSlot < 0xff);

        int32 key = ((idxAttender & 0xff) << 8) & ((uint8)eSlot & 0xff);

        const FMyArrangePointCfgWorld3DCpp *pV = m_mVisualPointCache.Find(key);
        if (pV) {
            visualPoint = *pV;
            return 0;
        }

        int32 errCode = retrieveVisualPointByIdxAttenderAndSlot(idxAttender, eSlot, visualPoint);
        if (errCode == 0) {
            FMyArrangePointCfgWorld3DCpp& newAdded = m_mVisualPointCache.Add(key);
            newAdded = visualPoint;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("getVisualPointCfgByIdxAttenderAndSlot() failed, idxAttender %d, eSlot %d, errorCode: %d."), idxAttender, (uint8)eSlot, errCode);
        }

        return errCode;
    };
    */

    FMyErrorCodeMJGameCpp retrieveCfgCache(FMyMJGameDeskVisualPointCfgCacheCpp& cPointCfgCache) const;

protected:

    //return errcode, 0 means no error
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    FMyErrorCodeMJGameCpp retrieveCardVisualPointCfg(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyArrangePointCfgWorld3DCpp &visualPoint) const;

    FMyErrorCodeMJGameCpp retrieveCardVisualPointCfg_Implementation(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyArrangePointCfgWorld3DCpp &visualPoint) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDeskAreaCpp::retrieveCardVisualPointCfg() must be overrided by blueprint child class!"));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint);
    };

    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    FMyErrorCodeMJGameCpp retrieveAttenderVisualPointCfg(int32 idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp eSubtype, FMyArrangePointCfgWorld3DCpp &visualPoint) const;

    FMyErrorCodeMJGameCpp retrieveAttenderVisualPointCfg_Implementation(int32 idxAttender, MyMJGameDeskVisualElemAttenderSubtypeCpp eSubtype, FMyArrangePointCfgWorld3DCpp &visualPoint) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDeskAreaCpp::retrieveAttenderVisualPointCfg() must be overrided by blueprint child class!"));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint);
    };

    //return errcode, 0 means no error, return cfg other than card
    UFUNCTION(BlueprintNativeEvent)
    FMyErrorCodeMJGameCpp retrieveTrivalVisualPointCfg(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyArrangePointCfgWorld3DCpp &visualPoint) const;

    FMyErrorCodeMJGameCpp retrieveTrivalVisualPointCfg_Implementation(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyArrangePointCfgWorld3DCpp &visualPoint) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDeskAreaCpp::retrieveTrivalVisualPointCfg() must be overrided by blueprint child class!"));
        return FMyErrorCodeMJGameCpp(MyErrorCodeCommonPartCpp::InterfaceFunctionNotImplementedByBlueprint);
    };

    TMap<int32, FMyArrangePointCfgWorld3DCpp> m_mVisualPointCache;
};


UCLASS(Blueprintable, NotBlueprintType)
class MYONLINECARDGAME_API UMyMJGameDeskResManagerCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJGameDeskResManagerCpp();
    virtual ~UMyMJGameDeskResManagerCpp();

    bool checkSettings(bool bCheckDataInGame) const;
    void reset();

    bool OnBeginPlay();

    //return error code, 0 means ok
    int32 prepareForVisual(int32 cardActorNum);

    //never fail
    UFUNCTION(BlueprintCallable)
    AMyMJGameCardActorBaseCpp* getCardActorByIdxEnsured(int32 idx)
    {
        return UMyCommonUtilsLibrary::helperGetActorInArrayEnsured<AMyMJGameCardActorBaseCpp>(this, getVisualCfg()->m_cMainActorClassCfg.m_cCardClass, m_aCardActors, idx, MY_CARD_ACTOR_NUM_MAX, TEXT("Card Actors"), true);
    };

    //may fail
    inline AMyMJGameCardActorBaseCpp* getCardActorByIdxConst(int32 idx) const
    {
        return UMyCommonUtilsLibrary::helperGetActorInArray<AMyMJGameCardActorBaseCpp>(m_aCardActors, idx);
    };

    //never fail
    UFUNCTION(BlueprintCallable)
    AMyMJGameDiceBaseCpp* getDiceActorByIdxEnsured(int32 idx)
    {
        return UMyCommonUtilsLibrary::helperGetActorInArrayEnsured<AMyMJGameDiceBaseCpp>(this, getVisualCfg()->m_cMainActorClassCfg.m_cDiceClass, m_aDiceActors, idx, MY_DICE_ACTOR_NUM_MAX, TEXT("Dice Actors"), true);
    };

    //may fail
    inline AMyMJGameDiceBaseCpp* getDiceActorByIdxConst(int32 idx) const
    {
        return UMyCommonUtilsLibrary::helperGetActorInArray<AMyMJGameDiceBaseCpp>(m_aDiceActors, idx);
    };


    //always success, core dump if fail, returned actor managed by this class
    UFUNCTION(BlueprintCallable)
    AMyMJGameTrivalDancingActorBaseCpp* getTrivalDancingActorByClass(TSubclassOf<AMyMJGameTrivalDancingActorBaseCpp> classType, bool freeActorOnly);

    //return error code
    UFUNCTION(BlueprintPure)
    FMyErrorCodeMJGameCpp retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const;

    inline const UMyMJGameInRoomVisualCfgCpp* getVisualCfg(bool verify = true) const
    {
        if (m_pInRoomcfg == NULL && verify) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pInRoomcfg is invalid: %p"), m_pInRoomcfg);
            MY_VERIFY(false);
        }
        return m_pInRoomcfg;
    };

protected:

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "cfg object"))
    UMyMJGameInRoomVisualCfgCpp* m_pInRoomcfg;

    UPROPERTY(Transient, meta = (DisplayName = "card actors"))
    TArray<AMyMJGameCardActorBaseCpp*> m_aCardActors;

    UPROPERTY(Transient, meta = (DisplayName = "dice actors"))
    TArray<AMyMJGameDiceBaseCpp*> m_aDiceActors;

    UPROPERTY(Transient, meta = (DisplayName = "trival dancing actors"))
    TArray<AMyMJGameTrivalDancingActorBaseCpp*> m_aTrivalDancingActors;

    //it seems UBT have bug which require declare sequence
    //UPROPERTY(BlueprintSetter = setCfgCardModelAssetPath, BlueprintSetter = getCfgCardModelAssetPath, EditAnywhere, meta = (DisplayName = "cfg card model asset path"))
    //UPROPERTY(EditAnywhere, BlueprintSetter = setCfgCardModelAssetPath, BlueprintGetter = getCfgCardModelAssetPath, meta = (DisplayName = "cfg card model asset path"))

};

//#define UpdateVisualDataTypeFullSyncNormal 0
//#define UpdateVisualDataTypeFullSyncUnexpected 1
//#define UpdateVisualDataTypeDelta 2

//this is used for visual layer, no replication code goes in
//note that we handle two type data here: one is time synced data like frame sync, one is not
UCLASS(Blueprintable, HideCategories = (Collision, Rendering, "Utilities|Transformation"))
class MYONLINECARDGAME_API AMyMJGameRoomCpp : public AActor, public IMyMJGameInRoomDeskInterfaceCpp
{
    GENERATED_BODY()

public:

    AMyMJGameRoomCpp();
    virtual ~AMyMJGameRoomCpp();

    bool checkSettings() const;
    void getCameraData(int32 idxDeskPosition, FMyCardGameCameraDataCpp& cameraData) const;

    void startVisual();
    void stopVisual();
    void loopVisual();

    inline UMyMJGameRoomDataSuite* getRoomDataSuiteVerified() const
    {
        MY_VERIFY(IsValid(m_pDataSuit));
        return m_pDataSuit;
    };

    inline UMyMJGameDeskResManagerCpp* getResManagerVerified() const
    {
        MY_VERIFY(IsValid(m_pResManager));
        return m_pResManager;
    };
    
    MyMJGameRuleTypeCpp helperGetRuleTypeNow() const;

    void updateVisualData(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                          const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                          const FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                          const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                          const TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices,
                          bool bIsFullBaseReset,
                          uint32 uiFullBaseResetDur_ms);

    void tipEventApplied(const FMyMJGameDeskVisualCfgCacheCpp& cCfgCache,
                         const FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                         const TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mIdCardChanged,
                         const FMyMJEventWithTimeStampBaseCpp& cEvent);

    void tipDataSkipped();

protected:

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


    virtual FMyErrorCodeMJGameCpp
    showAttenderThrowDices_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                          int32 diceVisualStateKey,
                                          const TArray<class AMyMJGameDiceBaseCpp *>& aDices) override;

    virtual FMyErrorCodeMJGameCpp
    showAttenderCardsDistribute_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                               const TArray<int32>& aIdsHandCards, bool isLastDistribution,
                                               const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsDistributed,
                                               const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving) override;

    virtual FMyErrorCodeMJGameCpp
    showAttenderTakeCards_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                         const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsTaken) override;

    virtual FMyErrorCodeMJGameCpp
    showAttenderGiveOutCards_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                            int32 handCardsCount,
                                            const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsGivenOut,
                                            const TArray<AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving) override;

    virtual FMyErrorCodeMJGameCpp
    showAttenderWeave_Implementation(float dur, int32 idxAttender, const FTransform &visualPointTransformForAttender,
                                     MyMJGameEventVisualTypeCpp weaveVsualType, const struct FMyMJWeaveCpp& weave,
                                     const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsWeaved, const TArray<class AMyMJGameCardActorBaseCpp*>& cardActorsOtherMoving) override;

    //return error code
    FMyErrorCodeMJGameCpp retrieveCfg(FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

    //contains all range >= min and <= max
    void getDataTimeRange(uint32 &out_uiDataGotServerTimeMin_ms, uint32& out_uiDataGotServerTimeMax_ms);

    //cfg start

    UPROPERTY(BlueprintReadOnly, Instanced, Replicated, VisibleAnywhere, meta = (DisplayName = "data suite"))
    UMyMJGameRoomDataSuite* m_pDataSuit;

    UPROPERTY(BlueprintReadOnly, Instanced, VisibleAnywhere, meta = (DisplayName = "desk res manager"))
    UMyMJGameDeskResManagerCpp *m_pResManager;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "area actor"))
    AMyMJGameDeskAreaCpp* m_pDeskAreaActor;

    //end

    FTimerHandle m_cLoopTimerHandle;


};