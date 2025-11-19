#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"

/**
 * Component responsible for handling the Dash ability.
 * Designed to be attached to a Character to grant dashing capabilities.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GM_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDashComponent();

protected:
	/** The strength of the dash impulse */
	UPROPERTY(EditAnywhere, Category="Ability|Dash")
	float DashStrength = 2000.f;

	/** Cooldown time in seconds between dashes */
	UPROPERTY(EditAnywhere, Category="Ability|Dash")
	float DashCooldown = 1.0f;

	/** Timestamp of the last successful dash used for cooldown calculation */
	float LastDashTime;

public:
	/** 
	 * Attempts to perform the dash logic.
	 * Returns true if the dash was executed successfully.
	 */
	bool PerformDash();
};