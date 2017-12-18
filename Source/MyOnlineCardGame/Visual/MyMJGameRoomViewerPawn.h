// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "MJBPEncap/MyMJGameViewerPawnBase.h"

#include "MyMJGameRoom.h"

#include "MJBPEncap/Utils/MyMJBPUtils.h"

#include "MyMJGameRoomViewerPawn.generated.h"

//TOdo: we can make it slower on Dedicated server
#define MyMJGameRoomViewerPawnLoopTimeMs (500)

#define MyMJGameRoomViewerAskSyncGapTimeMs (500)
#define MyMJGameRoomViewerAnswerSyncGapTimeMs (500)

/**
* Our goal is a network system allow actors to have "Full" and "Delta" datas, which can track the actor's state and accurate history, and it only transfor delta at normal case and transfer full for clients
* that fall behind in rare case.
* the 4.18 UE4 replication system: a actor will be destroyed when it is 'killed' on server, so for a actor need to keep history to match time progress, it causes problem unless we delay the 'kill'
* on server, which is not that reliable. And another problem is that, UActorChannel haven't the data struct for "Full" and "Delta" processing, and we don't have a way to implement it gracefully in APP
* level, unless do it in PlayerController making code orgnization bad.
* Unless replication system improvement in UE'4's ource code arrives, our design is: All actors need to record accurate history, will not replication directly, but gather their information and transfered together on
* on one or two static actors, like frame sync mechnism(actullay not per frame but per time segment). Here we use PlayerController since it contains per role data. Other actors doesn't need accurate history, use
* UE4's default actor replication mechnism per actor. Another benifit of this, by using struct instead of uobject for "Full" and "Delta", we can do calculation in subthread.
*/

UCLASS()
class MYONLINECARDGAME_API AMyMJGameRoomViewerPawnCpp : public AMyMJGameViewerPawnBaseCpp
{
	GENERATED_BODY()

public:
    AMyMJGameRoomViewerPawnCpp();
    virtual ~AMyMJGameRoomViewerPawnCpp();

    void clearInGame();

    inline
    void markNeedAnswerSyncForMJCoreFullData()
    {
        MY_VERIFY(UMyMJBPUtilsLibrary::haveServerNetworkLayer(this));

        if (!m_bNeedAnswerSyncForMJCoreFullData) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("marking need answer sync for full data at server side."));
            m_bNeedAnswerSyncForMJCoreFullData = true;
            loopOfSyncForMJCoreFullDataOnNetworkServer();
        }
    };

    inline
    void markNeedAskSyncForMJCoreFullData()
    {
        MY_VERIFY(UMyMJBPUtilsLibrary::haveClientNetworkLayer(this));

        if (!m_bNeedAskSyncForMJCoreFullData) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("marking need ask sync for full data at client side."));
            m_bNeedAskSyncForMJCoreFullData = true;
            loopOfSyncForMJCoreFullDataOnNetworkClient();
        }

    };

    //only with authority it can be changed
    void setRoleTypeWithAuth(MyMJGameRoleTypeCpp eRoleType);

    UFUNCTION(BlueprintCallable)
    void setDebugHaltFeedData(bool bDebugHaltFeedData)
    {
        if (m_bDebugHaltFeedData != bDebugHaltFeedData) {
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("changing DebugHaltFeedData %d->%d."), m_bDebugHaltFeedData, bDebugHaltFeedData);
            m_bDebugHaltFeedData = bDebugHaltFeedData;
        }
    };

    UFUNCTION(BlueprintCallable)
    bool getDebugHaltFeedData() const
    {
        return m_bDebugHaltFeedData;
    };

protected:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Server, unreliable, WithValidation)
        void askSyncForMJCoreFullDataOnServer();

    UFUNCTION(BlueprintCallable, Client, unreliable)
        void answerSyncForMJCoreFullDataOnClient(MyMJGameRoleTypeCpp eRole, const FMyMJDataStructWithTimeStampBaseCpp& cFullData);

    //will assert it have auth
    bool resetupWithRoleWithAuth(MyMJGameRoleTypeCpp eRoleType);

    void loop();

    void loopOfSyncForMJCoreFullDataOnNetworkServer();
    void loopOfSyncForMJCoreFullDataOnNetworkClient();


    UFUNCTION()
    void OnRep_ExtRoomActorPointer()
    {
        MY_VERIFY(IsValid(m_pExtRoomActor));
        tryFeedDataToConsumerWithFilter();
    };

    UFUNCTION()
    void OnRep_ExtRoomCoreDataSourceSeqPointer()
    {
        MY_VERIFY(IsValid(m_pExtRoomCoreDataSourceSeq));

        //trick is that, onRep only happen on client, and in that case only one instance need to be notified
        m_pExtRoomCoreDataSourceSeq->m_cReplicateDelegate.Clear();
        m_pExtRoomCoreDataSourceSeq->m_cReplicateDelegate.AddUObject(this, &AMyMJGameRoomViewerPawnCpp::tryFeedDataToConsumerWithFilter);
    };

    UFUNCTION()
    void OnRep_ExtRoomCoreDataSourceSeqContent()
    {
        tryFeedDataToConsumerWithFilter();
    };

    UFUNCTION()
    void OnRep_ExtRoomTrivalDataSourceSeqPointer()
    {
    };

    UFUNCTION()
    void OnRep_ExtRoomTrivalDataSourceSeqContent()
    {
    };

    void tryFeedDataToConsumerWithFilter();

    //return true if need to sync full, note it will already marked sync flag inside if needed
    bool tryFeedDataToConsumer();

    UPROPERTY(ReplicatedUsing = OnRep_ExtRoomActorPointer)
    AMyMJGameRoomCpp* m_pExtRoomActor;

    UPROPERTY(ReplicatedUsing = OnRep_ExtRoomCoreDataSourceSeqPointer)
    UMyMJDataSequencePerRoleCpp* m_pExtRoomCoreDataSourceSeq;

    UPROPERTY(ReplicatedUsing = OnRep_ExtRoomTrivalDataSourceSeqPointer)
    AMyMJGameTrivalDataSourceCpp *m_pExtRoomTrivalDataSource;


    uint32 m_fHelperFilterLastRepClientRealtTime;

    FTimerHandle m_cLoopTimerHandle;
    ENetMode m_eDebugNetmodeAtStart;

    float m_fLastAnswerSyncForMJCoreFullDataWorldRealTime;
    bool m_bNeedAnswerSyncForMJCoreFullData;
    float m_fLastAskSyncForMJCoreFullDataWorldRealTime;
    bool m_bNeedAskSyncForMJCoreFullData;
    bool m_bNeedRetryFeedDataForCore;

    bool m_bDebugHaltFeedData; //used to force client fall behind the progress

};
