// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MJ/Utils/MyMJUtils.h"

#include "MJBPEncap/MyMJGameCoreBP.h"
#include "MyMJGameVisualCard.h"

#include "MyMJGameRoomDataSuite.h"

#include "MyMJGameRoom.generated.h"

#define MyMJGameProgressLatenceAboveNetworkMax (5000)
#define AMyMJGameRoomVisualLoopTimeMs (50)

#define MyMJGameVisualStateCatchUpMinTimeToStayMs (1000)


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

    inline int32 getCardModelInfoUnscaled(FMyMJGameActorModelInfoBoxCpp& modelInfo) const
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

    //return error code
    int32 retrieveCfgCache(FMyMJGameDeskVisualActorModelInfoCacheCpp& cModelInfoCache) const;

protected:

    const AMyMJGameCardBaseCpp* getCardCDO() const;

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "cards"))
        TArray<AMyMJGameCardBaseCpp*> m_aCards;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cfg", meta = (DisplayName = "cfg card class"))
        TSubclassOf<AMyMJGameCardBaseCpp> m_cCfgCardClass;

    //it seems UBT have bug which require declare sequence
    //UPROPERTY(BlueprintSetter = setCfgCardModelAssetPath, BlueprintSetter = getCfgCardModelAssetPath, EditAnywhere, meta = (DisplayName = "cfg card model asset path"))
    //UPROPERTY(EditAnywhere, BlueprintSetter = setCfgCardModelAssetPath, BlueprintGetter = getCfgCardModelAssetPath, meta = (DisplayName = "cfg card model asset path"))

};

UENUM()
enum class AMyMJGameRoomVisualStateCpp : uint8
{
    Invalid = 0     UMETA(DisplayName = "Invalid"),
    WaitingForDataInitSync = 1     UMETA(DisplayName = "WaitingForDataInitSync"),
    WaitingForDataInGame = 2     UMETA(DisplayName = "WaitingForDataInGame"),
    NormalPlay = 3    UMETA(DisplayName = "NormalPlay"),
    CatchUp = 4    UMETA(DisplayName = "CatchUp")
};

USTRUCT()
struct FMyGameProgressDataCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyGameProgressDataCpp()
    {
        reset();
    };

    inline void reset() {
        m_uiServerTimePlayed_ms;
        m_cLastBond.clearBond();
    };

    uint32 m_uiServerTimePlayed_ms;
    FMyMJServerClientTimeBondCpp m_cLastBond;
};

//this is used for visual layer, no replication code goes in
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

    void loop();

    //play all events <= uiServerTime_ms
    void playGameProgressTo(uint32 uiServerTime_m, bool bCatchUp);

    bool checkSettings() const;

    inline UMyMJGameRoomDataSuite* getRoomDataSuiteVerified() const
    {
        MY_VERIFY(IsValid(m_pDataSuit));
        return m_pDataSuit;
    };

protected:

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    //return error code
    int32 retrieveCfg(FMyMJGameDeskVisualCfgCacheCpp& cCfgCache);

    void changeVisualState(AMyMJGameRoomVisualStateCpp eNewState, uint32 uiClientTimeNow_ms)
    {
        if (m_eVisualState != eNewState) {
            m_eVisualState = eNewState;
            m_uiVisualStateStartClientTime_ms = uiClientTimeNow_ms;
        }

    };

    //contains all range >= min and <= max
    void getDataTimeRange(uint32 &out_uiDataGotServerTimeMin_ms, uint32& out_uiDataGotServerTimeMax_ms);

    //cfg start

    UPROPERTY(BlueprintReadOnly, Replicated, VisibleAnywhere, meta = (DisplayName = "data suite"))
    UMyMJGameRoomDataSuite* m_pDataSuit;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (DisplayName = "desk res manager"))
    UMyMJGameDeskResManagerCpp *m_pResManager;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cfg", meta = (DisplayName = "area actor"))
    AMyMJGameDeskAreaCpp* m_pDeskAreaActor;

    //end

    FTimerHandle m_cLoopTimerHandle;

    FMyGameProgressDataCpp m_cGameProgressData;
    AMyMJGameRoomVisualStateCpp m_eVisualState;
    uint32 m_uiVisualStateStartClientTime_ms;
};