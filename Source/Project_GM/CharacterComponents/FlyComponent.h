// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "FlyComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GM_API UFlyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlyComponent();

protected:
	
	/** --- FLY SETTINGS --- */
	UPROPERTY(EditAnywhere, Category = "Fly Settings")
	float FlyForce = 0;
	
	FRotator FlyingRotation;
	FRotator FlyingYawRotation;
	float InputFlyForce = 0;
	
	
	// Called when the game starts
	//virtual void BeginPlay() override;
	void FlyUp(const FInputActionValue& Value);
	void FlyDown(const FInputActionValue& Value);
	

public:	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FlyUpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FlyDownAction;
	
	FVector FlyingUpDirection;

	
	FVector GetFlyingUpDirection(const AController* Controller);
	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float CalculateFinalFlyForce() const;
	void SetInput(UEnhancedInputComponent* Eic);
};
