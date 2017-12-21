// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameVisualElems.h"

#include "MyMJGameRoomDataSuite.h"

#include "MyMJGameRoom.generated.h"


#define AMyMJGameRoomVisualLoopTimeMs (50)

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
    */

    int32 retrieveCfgCache(FMyMJGameDeskVisualPointCfgCacheCpp& cPointCfgCache) const;

protected:

    //return errcode, 0 means no error
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    int32 retrieveCardVisualPointCfg(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const;

    int32 retrieveCardVisualPointCfg_Implementation(int32 idxAttender, MyMJCardSlotTypeCpp eSlot, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDeskAreaCpp::retrieveCardVisualPointCfg() must be overrided by blueprint subclass!"));
        return -1;
    };

    //return errcode, 0 means no error, return cfg other than card
    UFUNCTION(BlueprintNativeEvent)
    int32 retrieveTrivalVisualPointCfg(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const;

    int32 retrieveTrivalVisualPointCfg_Implementation(MyMJGameDeskVisualElemTypeCpp eElemType, int32 subIdx0, int32 subIdx1, FMyMJGameDeskVisualPointCfgCpp &visualPoint) const
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("AMyMJGameDeskAreaCpp::retrieveTrivalVisualPointCfg() must be overrided by blueprint subclass!"));
        return -1;
    };

    TMap<int32, FMyMJGameDeskVisualPointCfgCpp> m_mVisualPointCache;
};

UCLASS(Blueprintable, NotBlueprintType)
class MYONLINECARDGAME_API UMyMJGameDeskResManagerCpp : public UActorComponent
{
    GENERATED_BODY()

public:

    UMyMJGameDeskResManagerCpp();
    virtual ~UMyMJGameDeskResManagerCpp();

    bool checkSettings() const;

    UFUNCTION(BlueprintPure)
    inline int32 getCardModelInfoUnscaled(FMyMJGameActorModelInfoBoxCpp& modelInfo) const
    {
        const AMyMJGameCardBaseCpp* pCDO = getCardBaseCDO();
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

    //always success, core dump if fail
    UFUNCTION(BlueprintCallable)
    AMyMJGameCardBaseCpp* getCardActorByIdx(int32 idx);

    //UFUNCTION(BlueprintCallable)
    int32 prepareCardActor(int32 count2reach);

    //return error code
    UFUNCTION(BlueprintPure)
    int32 retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const;

protected:

    //Warning: it is not the actually specified one, but casted to base type
    const AMyMJGameCardBaseCpp* getCardBaseCDO() const;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "cards"))
        TArray<AMyMJGameCardBaseCpp*> m_aCards;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cfg", meta = (DisplayName = "cfg card class"))
        TSubclassOf<AMyMJGameCardBaseCpp> m_cCfgCardClass;

    //it seems UBT have bug which require declare sequence
    //UPROPERTY(BlueprintSetter = setCfgCardModelAssetPath, BlueprintSetter = getCfgCardModelAssetPath, EditAnywhere, meta = (DisplayName = "cfg card model asset path"))
    //UPROPERTY(EditAnywhere, BlueprintSetter = setCfgCardModelAssetPath, BlueprintGetter = getCfgCardModelAssetPath, meta = (DisplayName = "cfg card model asset path"))

};

//this is used for visual layer, no replication code goes in
//note that we handle two type data here: one is time synced data like frame sync, one is not
UCLASS(Blueprintable, HideCategories = (Collision, Rendering, "Utilities|Transformation"))
class MYONLINECARDGAME_API AMyMJGameRoomCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyMJGameRoomCpp();
    virtual ~AMyMJGameRoomCpp();

    void startVisual();
    void stopVisual();
    void loopVisual();

    bool checkSettings() const;

    inline UMyMJGameRoomDataSuite* getRoomDataSuiteVerified() const
    {
        MY_VERIFY(IsValid(m_pDataSuit));
        return m_pDataSuit;
    };

    
    void onDeskUpdatedWithImportantChange(FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                              FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                                              TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                                              TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices);

    void onDeskEventApplied(FMyMJDataStructWithTimeStampBaseCpp& cCoreData,
                                           FMyDirtyRecordWithKeyAnd4IdxsMapCpp& cCoreDataDirtyRecord,
                                           TMap<int32, FMyMJGameCardVisualInfoAndResultCpp>& mNewActorDataIdCards,
                                           TMap<int32, FMyMJGameDiceVisualInfoAndResultCpp>& mNewActorDataIdDices,
                                           FMyMJEventWithTimeStampBaseCpp& cEvent);


protected:

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    //return error code
    int32 retrieveCfg(FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

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