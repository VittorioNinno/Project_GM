#include "DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "StuntInteractable.h"

UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	LastDashTime = -DashCooldown;
	bIsCurrentlyDashing = false;
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCurrentlyDashing)
	{
		DetectEnvironmentalInteractions();
	}
}

bool UDashComponent::PerformDash()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement()) return false;

	if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround()) return false;

	UWorld* World = GetWorld();
	if (World == nullptr || World->GetTimeSeconds() < LastDashTime + DashCooldown) return false;

	FVector DashDirection;
	FVector InputAcceleration = OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal();
	DashDirection = !InputAcceleration.IsNearlyZero() ? InputAcceleration : OwnerCharacter->GetActorForwardVector();

	OwnerCharacter->LaunchCharacter(DashDirection * DashStrength, true, true);
	
	LastDashTime = World->GetTimeSeconds();
	bIsCurrentlyDashing = true;

	World->GetTimerManager().SetTimer(InteractionTimerHandle, this, &UDashComponent::StopDashInteraction, 0.2f, false);

	return true;
}

void UDashComponent::DetectEnvironmentalInteractions()
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld()) return;

	FVector TraceLocation = Owner->GetActorLocation();
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, TraceLocation, TraceLocation, FQuat::Identity, 
		ECC_WorldDynamic, FCollisionShape::MakeSphere(DetectionRadius), Params
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->GetClass()->ImplementsInterface(UStuntInteractable::StaticClass()))
			{
				IStuntInteractable::Execute_OnStuntImpact(HitActor, Owner, FName("Dash"));
			}
		}
	}
}

void UDashComponent::StopDashInteraction()
{
	bIsCurrentlyDashing = false;
}