// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ElectricEffect.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GM_API UElectricEffect : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UElectricEffect();
	
	// Component constructor
	void InitializeEffect(float Duration);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	//Effect current elapsed Duration
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true))
	float Duration = 3.0f;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// This will refresh the timer of the effect
	UFUNCTION(BlueprintCallable, Category = "A Project GM | Effects | Electricity")
	void RefreshDuration(float DurationValue);

private:
	
	
	
	FTimerHandle TimerHandle_RemoveEffect;
    
	// What to do then the effect end
	void OnDurationExpired();
		
};
