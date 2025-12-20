#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SuctionZone.generated.h"

class UBoxComponent;
class UBillboardComponent;

UCLASS()
class PROJECT_GM_API ASuctionZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ASuctionZone();

protected:
	virtual void Tick(float DeltaTime) override;
	
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	
	// --- EDITOR VISUALS ---
	/* Sprite to make the actor visible in the editor viewport */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBillboardComponent* SpriteComponent;
	
protected:
	/* Volume defining the pressurized room */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* ZoneVolume;

	/* The specific location of the hole/breach */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SuctionPoint;

public:
	/* Maximum force applied when closest to the hole */
	UPROPERTY(EditAnywhere, Category = "Settings")
	float MaxSuctionStrength = 5000.0f;

	/* The radius from the SuctionPoint where the force starts to be applied.
	// Inside this radius, force increases. Outside (but inside box), force is minimal. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	float AttenuationRadius = 1000.0f;

	/* Is the breach active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsActive = true;

public:
	/* Call to seal the breach */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void DeactivateSuction();
};