// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomViewerPawnBase.h"
#include "MyMJGamePlayerController.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "MyMJGameRoomLevelScriptActorCpp.h"
#include "MyMJGameDeskVisualData.h"

AMyMJGameRoomViewerPawnBaseCpp::AMyMJGameRoomViewerPawnBaseCpp() : Super()
{
    //ASpectatorPawn a;

    SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
    bReplicates = true;
    NetPriority = 3.0f;
    NetUpdateFrequency = 1.f;
    bReplicateMovement = true;

    bCanBeDamaged = false;

    BaseEyeHeight = 0.0f;
    bCollideWhenPlacing = false;
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
};

AMyMJGameRoomViewerPawnBaseCpp::~AMyMJGameRoomViewerPawnBaseCpp()
{

};

void AMyMJGameRoomViewerPawnBaseCpp::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //NewController->GetClass()->GetName();
    UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PossessedBy: %s."), *NewController->GetClass()->GetName());
};

void AMyMJGameRoomViewerPawnBaseCpp::UnPossessed()
{
    AController* const OldController = Controller;
    Super::UnPossessed();
};


