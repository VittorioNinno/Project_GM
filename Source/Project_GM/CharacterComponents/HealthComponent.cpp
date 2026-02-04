// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentHealth = MaxHealth;
}

void UHealthComponent::SetHealth(float NewHealth)
{
	CurrentHealth = NewHealth;
	
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UHealthComponent::AddHealth(float Amount)
{
	SetHealth(CurrentHealth + Amount);
}

void UHealthComponent::RemoveHealth(float Amount)
{
	SetHealth(CurrentHealth - Amount);
}

