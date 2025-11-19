#include "WallMechanicsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UWallMechanicsComponent::UWallMechanicsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWallMechanicsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind the OnLanded event to restore physics when touching the ground
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->LandedDelegate.AddDynamic(this, &UWallMechanicsComponent::OnLanded);
	}
}

bool UWallMechanicsComponent::DetectWall(FVector& OutWallNormal)
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	FVector Start = Owner->GetActorLocation();
	FVector End = Start + (Owner->GetActorForwardVector() * 250.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_Visibility, Params
	);

	if (bHit)
	{
		if (FMath::Abs(HitResult.ImpactNormal.Z) < 0.7f)
		{
			OutWallNormal = HitResult.ImpactNormal;
			return true;
		}
	}
	return false;
}

bool UWallMechanicsComponent::AttemptWallJump()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return false;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	
	if (!MoveComp || !MoveComp->IsFalling()) return false;

	FVector WallNormal;
	if (DetectWall(WallNormal))
	{
		MoveComp->StopMovementImmediately();

		FVector JumpDirection = (WallNormal + FVector::UpVector).GetSafeNormal();
		
		// Apply force
		float TotalPower = FMath::Max(WallJumpHorizontalForce, WallJumpUpForce) * 1.5f;
		FVector FinalVelocity = JumpDirection * TotalPower;

		Character->LaunchCharacter(FinalVelocity, true, true);

		FRotator NewRotation = WallNormal.Rotation();
		NewRotation.Pitch = 0.f;
		Character->SetActorRotation(NewRotation);

		// Store defaults
		DefaultAirControl = MoveComp->AirControl;
		DefaultBrakingDeceleration = MoveComp->BrakingDecelerationFalling;

		// Disable control and friction for a pure physics arc
		MoveComp->AirControl = 0.0f;
		MoveComp->BrakingDecelerationFalling = 0.0f; 

		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_ResetAirControl,
			this,
			&UWallMechanicsComponent::ResetAirControl,
			WallJumpAirControlLockout,
			false
		);
		
		return true;
	}

	return false;
}

void UWallMechanicsComponent::ResetAirControl()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			// Restore original air control to allow player steering
			MoveComp->AirControl = DefaultAirControl;
          
			// Braking deceleration remains disabled to preserve horizontal momentum until landing
		}
	}
}

void UWallMechanicsComponent::OnLanded(const FHitResult& Hit)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			// Restore standard falling friction upon ground contact
			MoveComp->BrakingDecelerationFalling = 1500.0f; 
		}
	}
}