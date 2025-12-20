#include "SuctionZone.h"
#include "Components/BoxComponent.h"
#include "Project_GMCharacter.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"

ASuctionZone::ASuctionZone()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// --- EDITOR ICON SETUP ---
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("SpriteIcon"));
	SpriteComponent->SetupAttachment(RootComponent);
    
	static ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("/Game/Static/LevelPrototyping/Textures/T_Suction.T_Suction"));
	if (IconAsset.Succeeded())
	{
		SpriteComponent->SetSprite(IconAsset.Object);
	}
    
	// Hide the sprite during gameplay, show only in editor
	SpriteComponent->SetHiddenInGame(true);
    
	// Set default scale to 1/1/1
	SpriteComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
    
	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	ZoneVolume->SetupAttachment(RootComponent);
	ZoneVolume->SetBoxExtent(FVector(800.f, 800.f, 500.f));
	ZoneVolume->SetCollisionProfileName("Trigger");

	SuctionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SuctionPoint"));
	SuctionPoint->SetupAttachment(RootComponent);
	SuctionPoint->SetRelativeLocation(FVector(0, 0, 100.f));
}

void ASuctionZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive) return;

	FVector HoleLoc = SuctionPoint->GetComponentLocation();

	TArray<AActor*> OverlappingActors;
	ZoneVolume->GetOverlappingActors(OverlappingActors, AProject_GMCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (AProject_GMCharacter* Char = Cast<AProject_GMCharacter>(Actor))
		{
			float Distance = FVector::Distance(Char->GetActorLocation(), HoleLoc);

			// --- CALCULATE FALLOFF ---
			
			// Calculate a 0.0 to 1.0 value based on distance
			// 0.0 = At the edge of attenuation radius (or further)
			// 1.0 = Exactly at the hole center
			float Alpha = 1.0f - (Distance / AttenuationRadius);
			
			// Clamp it. Ensure there is always a minimum force (e.g. 20%) 
			// if they are inside the Box Volume, so the mechanic doesn't disappear completely.
			Alpha = FMath::Clamp(Alpha, 0.2f, 1.0f);

			// Scale the strength
			float CurrentStrength = MaxSuctionStrength * Alpha;

			// Apply the dynamic strength to the character
			Char->ApplySuctionForce(HoleLoc, CurrentStrength, DeltaTime);
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

void ASuctionZone::DeactivateSuction()
{
	if (!bIsActive) return;
	bIsActive = false;
	
	TArray<AActor*> OverlappingActors;
	ZoneVolume->GetOverlappingActors(OverlappingActors, AProject_GMCharacter::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		if (AProject_GMCharacter* Char = Cast<AProject_GMCharacter>(Actor))
		{
			Char->ResetSuctionPhysics();
		}
	}
}