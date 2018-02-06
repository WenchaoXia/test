// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMJGameRoomViewerPawnBase.h"
#include "MyMJGamePlayerController.h"

#include "Kismet/GameplayStatics.h"

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

    m_pTransformUpdateSequence = CreateDefaultSubobject<UMyTransformUpdateSequenceMovementComponent>(TEXT("transform update sequence movement component"));
};

AMyMJGameRoomViewerPawnBaseCpp::~AMyMJGameRoomViewerPawnBaseCpp()
{

};

void AMyMJGameRoomViewerPawnBaseCpp::OnPossessedByLocalPlayerController(APlayerController* newController)
{
    //Not doing anything now
    //int32 id = UGameplayStatics::GetPlayerControllerID(newController);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnPossessedByLocalPlayerController: %s, %d."), *newController->GetClass()->GetName(), id);
};

void AMyMJGameRoomViewerPawnBaseCpp::OnUnPossessedByLocalPlayerController(APlayerController* oldController)
{
    //int32 id = UGameplayStatics::GetPlayerControllerID(oldController);
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("OnUnPossessedByLocalPlayerController: %s, id %d."), *oldController->GetClass()->GetName(), id);
};

void AMyMJGameRoomViewerPawnBaseCpp::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //NewController->GetClass()->GetName();
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PossessedBy: %s."), *NewController->GetClass()->GetName());
};

void AMyMJGameRoomViewerPawnBaseCpp::UnPossessed()
{
    AController* const OldController = Controller;
    Super::UnPossessed();
};


