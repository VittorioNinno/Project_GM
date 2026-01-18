#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Project_GMCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UDashComponent;
class UWallMechanicsComponent;
class UStaminaComponent;
class UFlyComponent;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class PROJECT_GM_API AProject_GMCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AProject_GMCharacter();

protected:
	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDashComponent* DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWallMechanicsComponent* WallMechanicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaminaComponent* StaminaComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFlyComponent* FlyComponent;

	/** UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidgetInstance;

	/** --- MOVEMENT SETTINGS --- */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
	float SprintSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
	float MinSlideSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
	float SlideFriction = 0.1f;

	/** --- ICE SETTINGS --- */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ice")
	float IceMaxAcceleration = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ice")
	float IceRotationRate = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ice")
	float IceSlopeGravityForce = 500000.f;

	/** --- SUCTION SETTINGS --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suction State")
	bool bIsUnderSuction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suction State")
	FVector SuctionTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suction State")
	float CurrentSuctionStrength = 0.0f;
	
	
	
	/** Internal State Flags */
	bool bIsSliding = false;

private:
	/** Default values captured at BeginPlay to avoid 'Cannot resolve symbol' */
	float DefaultGroundFriction;
	float DefaultBrakingDeceleration;
	float DefaultMaxWalkSpeed;
	float DefaultMaxAcceleration;
	float DefaultRotationRate;
	float DefaultCrouchedWalkSpeed;
	UPROPERTY();
	UCharacterMovementComponent* MovementComponent;

public:
	/** Interfaces */
	void ApplySuctionForce(FVector TargetLocation, float Strength, float DeltaTime);
	void ResetSuctionPhysics();
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStaminaPercentage() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Movement and Logic Handlers */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnDashAction();
	void StartSprint();
	void StopSprint();
	void StartCrouch();
	void StopCrouch();
	void DoJumpStart();
	
	/** Main physics processing using Line Trace */
	void HandleGroundPhysics(float DeltaTime);

	void HandleFlyPhysics() const;
	
	/** Debug Section */
	void DisplaySurfaceDebugInfo();

	/** Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;
};