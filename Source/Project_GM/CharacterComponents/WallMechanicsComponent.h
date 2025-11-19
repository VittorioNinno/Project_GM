#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallMechanicsComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GM_API UWallMechanicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWallMechanicsComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Wall Jump")
	float WallJumpHorizontalForce = 200.f; // Increased default for parabolic arc

	UPROPERTY(EditAnywhere, Category="Wall Jump")
	float WallJumpUpForce = 200.f;

	UPROPERTY(EditAnywhere, Category="Wall Jump")
	float WallJumpAirControlLockout = 0.4f;

	bool DetectWall(FVector& OutWallNormal);
	void ResetAirControl();

	// Function called when the character hits the ground
	UFUNCTION()
	void OnLanded(const FHitResult& Hit);

private:
	FTimerHandle TimerHandle_ResetAirControl;
	float DefaultAirControl;
	float DefaultBrakingDeceleration;

public:
	bool AttemptWallJump();
};