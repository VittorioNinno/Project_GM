#include "DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDashComponent::UDashComponent()
{
	// Disable tick as this component only reacts to events
	PrimaryComponentTick.bCanEverTick = false;
	
	// Initialize with a value that allows immediate usage
	LastDashTime = -DashCooldown;
}

bool UDashComponent::PerformDash()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	// Validate owner and movement component existence
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return false;
	}

	// Prevent dashing while on the ground to ensure the input key can be shared with Sprint when on ground
	if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}

	UWorld* World = GetWorld();
	
	// Check if the ability is on cooldown
	if (World == nullptr || World->GetTimeSeconds() < LastDashTime + DashCooldown)
	{
		return false;
	}

	// Calculate dash direction based on input or character orientation
	FVector DashDirection;
	
	// GetCurrentAcceleration returns the requested movement direction from input (WASD / Stick)
	FVector InputAcceleration = OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal();

	if (!InputAcceleration.IsNearlyZero())
	{
		// If the player is pressing a direction, dash towards that input
		DashDirection = InputAcceleration;
	}
	else
	{
		// If there is no input, dash in the direction the character mesh is currently facing
		// This is required for fixed-camera setups where ControlRotation does not match character facing
		DashDirection = OwnerCharacter->GetActorForwardVector();
	}

	// Ensure Z velocity is overridden to provide a consistent impulse (Air Dash)
	OwnerCharacter->LaunchCharacter(DashDirection * DashStrength, true, true);

	// Update cooldown timestamp
	LastDashTime = World->GetTimeSeconds();

	return true;
}