// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Transform = CreateDefaultSubobject<USceneComponent>(FName(TEXT("USceneComponent")));
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	
    m_count = 0;

    m_com = NewObject<UMyActorComponent>(this, FName("UMyActorComponent"));
    m_com->RegisterComponent();
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    m_count += 1;
    GLog->Log(FString::Format(TEXT("AMyActor::Tick count:{0}"), { m_count }));
}

