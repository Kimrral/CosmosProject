// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
APlayerWeapon::APlayerWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(boxComp);	
	boxComp->SetSimulatePhysics(true);
	boxComp->SetBoxExtent(FVector(60, 3, 16));

	rifleComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("rifleComp"));
	rifleComp->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void APlayerWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

