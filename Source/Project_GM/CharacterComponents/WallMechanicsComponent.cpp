#include "WallMechanicsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StuntInteractable.h"

UWallMechanicsComponent::UWallMechanicsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWallMechanicsComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->LandedDelegate.AddDynamic(this, &UWallMechanicsComponent::OnLanded);
	}
}

bool UWallMechanicsComponent::DetectWall(FVector& OutWallNormal, AActor*& OutHitActor)
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	FVector Start = Owner->GetActorLocation();
	FVector End = Start + (Owner->GetActorForwardVector() * 250.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit && FMath::Abs(HitResult.ImpactNormal.Z) < 0.7f)
	{
		OutWallNormal = HitResult.ImpactNormal;
		OutHitActor = HitResult.GetActor();
		return true;
	}
	return false;
}

bool UWallMechanicsComponent::AttemptWallJump()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return false;

	FVector WallNormal;
	AActor* HitWallActor = nullptr;

	if (DetectWall(WallNormal, HitWallActor))
	{
		if (HitWallActor && HitWallActor->GetClass()->ImplementsInterface(UStuntInteractable::StaticClass()))
		{
			IStuntInteractable::Execute_OnStuntImpact(HitWallActor, Character, FName("WallJump"));
		}

		UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
		FVector TotalPower = (WallNormal * WallJumpHorizontalForce) + (FVector::UpVector * WallJumpUpForce);
		
		FVector FinalVelocity = Character->GetVelocity();
		FinalVelocity.Z = 0.f;
		FinalVelocity += TotalPower;

		Character->LaunchCharacter(FinalVelocity, true, true);
		Character->SetActorRotation(WallNormal.Rotation());

		DefaultAirControl = MoveComp->AirControl;
		DefaultBrakingDeceleration = MoveComp->BrakingDecelerationFalling;
		MoveComp->AirControl = 0.0f;
		MoveComp->BrakingDecelerationFalling = 0.0f; 

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetAirControl, this, &UWallMechanicsComponent::ResetAirControl, WallJumpAirControlLockout, false);
		return true;
	}
	return false;
}

void UWallMechanicsComponent::ResetAirControl()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->AirControl = DefaultAirControl;
		}
	}
}

void UWallMechanicsComponent::OnLanded(const FHitResult& Hit)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->BrakingDecelerationFalling = DefaultBrakingDeceleration;
		}
	}
}