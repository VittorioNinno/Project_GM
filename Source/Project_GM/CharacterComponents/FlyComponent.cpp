// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/FlyComponent.h"

#include "EnhancedInputComponent.h"

// Sets default values for this component's properties
UFlyComponent::UFlyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	FlyForce = 1;
	// ...
}


// Called when the game starts
/*void UFlyComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement()) return;

	if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround()) return;
	
	CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	// ...
	
}


// Called every frame
void UFlyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}*/

float UFlyComponent::CalculateFinalFlyForce() const
{
	//UE_LOG(LogTemp, Log, TEXT("FinalFlyForce: %f"), InputFlyForce * FlyForce);
	
	return InputFlyForce * FlyForce;
}

void UFlyComponent::FlyUp(const FInputActionValue& Value)
{
	InputFlyForce = Value.Get<float>();
	
	//UE_LOG(LogTemp, Log, TEXT("Start FlyUp: UpValue = %f"), InputFlyForce);
}

void UFlyComponent::FlyDown(const FInputActionValue& Value)
{
	InputFlyForce = -Value.Get<float>();
    
	//UE_LOG(LogTemp, Log, TEXT("Start FlyDown: UpValue = %f"), InputFlyForce);
}

FVector UFlyComponent::GetFlyingUpDirection(const AController* Controller)
{
	FlyingRotation = Controller->GetControlRotation();
	
	FlyingYawRotation = FRotator(0, FlyingRotation.Yaw, 0);
		
	return FRotationMatrix(FlyingYawRotation).GetUnitAxis(EAxis::Z);
}

void UFlyComponent::SetInput(UEnhancedInputComponent* Eic)
{
	// Fly Up Actions
	Eic->BindAction(FlyUpAction, ETriggerEvent::Started, this, &UFlyComponent::FlyUp);
	Eic->BindAction(FlyUpAction, ETriggerEvent::Completed, this, &UFlyComponent::FlyUp);
	
	// Fly Down Actions
	Eic->BindAction(FlyDownAction, ETriggerEvent::Started, this, &UFlyComponent::FlyDown);
	Eic->BindAction(FlyDownAction, ETriggerEvent::Completed, this, &UFlyComponent::FlyDown);
}

