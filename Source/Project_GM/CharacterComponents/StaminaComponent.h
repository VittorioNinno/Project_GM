#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GM_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStaminaComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Modifies current stamina by a delta amount */
	UFUNCTION(BlueprintCallable, Category = "Stats|Stamina")
	void ModifyStamina(float Amount);

	/** Returns normalized stamina value (0 to 1) */
	UFUNCTION(BlueprintPure, Category = "Stats|Stamina")
	float GetStaminaPercentage() const;

	/** Toggles natural recovery on/off */
	UFUNCTION(BlueprintCallable, Category = "Stats|Stamina")
	void SetIsRegenerating(bool bShouldRegen) { bCanRegenerate = bShouldRegen; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Stamina")
	float RegenRate = 10.0f;

private:
	bool bCanRegenerate = true;
};