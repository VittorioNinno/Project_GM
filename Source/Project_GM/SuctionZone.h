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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBillboardComponent* SpriteComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* ZoneVolume;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SuctionPoint;

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	float MaxSuctionStrength = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float AttenuationRadius = 1500.0f;
};