#include "StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/** Natural stamina recovery over time */
	if (bCanRegenerate && CurrentStamina < MaxStamina)
	{
		ModifyStamina(RegenRate * DeltaTime);
	}
}

void UStaminaComponent::ModifyStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
}

float UStaminaComponent::GetStaminaPercentage() const
{
	return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}