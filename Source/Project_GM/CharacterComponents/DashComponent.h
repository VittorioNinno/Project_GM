#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StuntInteractable.h"
#include "DashComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GM_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDashComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempts to perform the dash logic. Returns true if executed successfully. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Dash")
	bool PerformDash();

protected:
	UPROPERTY(EditAnywhere, Category = "Ability|Dash")
	float DashStrength = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Ability|Dash")
	float DashCooldown = 1.0f;

	/** Radius used to detect interactable environmental objects while dashing */
	UPROPERTY(EditAnywhere, Category = "Ability|Dash")
	float DetectionRadius = 150.0f;

private:
	/** Checks for actors implementing IStuntInteractable in character's vicinity */
	void DetectEnvironmentalInteractions();

	/** Stops the active interaction detection window */
	void StopDashInteraction();

	float LastDashTime;
	bool bIsCurrentlyDashing;
	FTimerHandle InteractionTimerHandle;
};