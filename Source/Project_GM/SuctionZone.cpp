#include "SuctionZone.h"
#include "Components/BoxComponent.h"
#include "Project_GMCharacter.h"
#include "Components/BillboardComponent.h"

ASuctionZone::ASuctionZone()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("SpriteIcon"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetHiddenInGame(true);

	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	ZoneVolume->SetupAttachment(RootComponent);

	SuctionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SuctionPoint"));
	SuctionPoint->SetupAttachment(RootComponent);
}

void ASuctionZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<AActor*> OverlappingActors;
	ZoneVolume->GetOverlappingActors(OverlappingActors);

	FVector HoleLoc = SuctionPoint->GetComponentLocation();

	for (AActor* Actor : OverlappingActors)
	{
		if (AProject_GMCharacter* Char = Cast<AProject_GMCharacter>(Actor))
		{
			float Distance = FVector::Distance(Char->GetActorLocation(), HoleLoc);
			
			/** Calculate distance-based falloff (1.0 at center, 0.2 at edges) */
			float Alpha = FMath::Clamp(1.0f - (Distance / AttenuationRadius), 0.2f, 1.0f);
			
			/** Apply attenuated strength to the character */
			Char->ApplySuctionForce(HoleLoc, MaxSuctionStrength * Alpha, DeltaTime);
		}
	}
}

void ASuctionZone::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (AProject_GMCharacter* Char = Cast<AProject_GMCharacter>(OtherActor))
	{
		Char->ResetSuctionPhysics();
	}
}