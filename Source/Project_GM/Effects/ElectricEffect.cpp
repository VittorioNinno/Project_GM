// Fill out your copyright notice in the Description page of Project Settings.


#include "Effects/ElectricEffect.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UElectricEffect::UElectricEffect()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}

void UElectricEffect::InitializeEffect(float duration)
{
	Duration = duration;
}

void UElectricEffect::BeginPlay()
{
	Super::BeginPlay();
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			Character->DisableInput(PlayerController);
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("SHOCK! Input Disabled via Component"));
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, FString::Printf(TEXT("BeginPlay - Duration: %f"), Duration));
	RefreshDuration(Duration);
}

void UElectricEffect::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, TEXT("EndPlay"));
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TimerHandle_RemoveEffect);
	}
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			Character->EnableInput(PlayerController);
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("SHOCK END Input Enabled via Component"));
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UElectricEffect::RefreshDuration(float DurationValue)
{
	Duration = DurationValue;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle_RemoveEffect, this, &UElectricEffect::OnDurationExpired, DurationValue, false);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Duration Refreshed"));
}

void UElectricEffect::OnDurationExpired()
{
	DestroyComponent();
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Duration Expired"));
}
