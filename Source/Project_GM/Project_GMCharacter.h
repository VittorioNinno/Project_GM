#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Project_GMCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class AProject_GMCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Dash Input Action (Shared with Sprint) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DashAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

public:
	AProject_GMCharacter();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for dash input to delegate to component */
	void OnDash();

	/** Called for jump input to delegate to component or base jump */
	void DoJumpStart();

	/** Called for sprint input start */
	void StartSprint();

	/** Called for sprint input stop */
	void StopSprint();

	/** Called for crouch input start to handle sliding */
	void StartCrouch();

	/** Called for crouch input stop */
	void StopCrouch();
	
	/** Helper function to stop the slide and reset physics */
	void StopSlide();
	
	/* Handles ground physics detection and applies movement changes */
	void HandleGroundPhysics();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	// --- Sprint Variables ---
	
	/** Speed of the character when sprinting */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed = 800.f;

	float DefaultWalkSpeed;

	// --- Slide Variables ---

	/** The minimum speed (cm/s) to initiate a slide */
	UPROPERTY(EditAnywhere, Category = "Movement|Slide")
	float MinSlideSpeed = 600.f;

	/** The friction to apply when sliding (0 = no friction) */
	UPROPERTY(EditAnywhere, Category = "Movement|Slide")
	float SlideFriction = 0.1f;

	float DefaultGroundFriction;
	float DefaultCrouchedWalkSpeed;

	/** True if the player is currently sliding */
	bool bIsSliding;
	
	// --- Ice Physics Variables ---
    
	/* Friction level on ice (Lower = more slippery) */
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceGroundFriction = 0.1f; 

	/* Deceleration when stopping on ice (Lower = sliding effect) */
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceBrakingDeceleration = 100.0f; 

	/* Max acceleration on ice (Lower = harder to start moving) */
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceMaxAcceleration = 400.0f; 
	
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceSlopeGravityForce = 5000.0f;
	
	/* Rotation rate (Yaw) when moving on ice. Lower values make turning harder/slower. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ice", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float IceRotationRate = 60.0f;

	// --- Variables to store default movement values ---
	float DefaultBrakingDeceleration;
	float DefaultMaxAcceleration;
	float DefaultRotationRate;
	
	/* Max walk speed when moving WITH the suction force (Running towards the hole) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionMoveSpeed_WithForce = 1000.0f;

	/* Ground friction when moving WITH the suction force (Lower = more slippery) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionFriction_WithForce = 0.1f;

	/* Max walk speed when resisting the suction force (Running away) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionMoveSpeed_AgainstForce = 450.0f;

	/* Ground friction when resisting. High values provide grip, but too high causes stuck behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionFriction_AgainstForce = 2.0f;

	/* Braking deceleration when resisting. Lower values prevent stuttering movement against the force. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionBraking_AgainstForce = 100.0f;

	/* Max speed when the player is idle and being dragged passively */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionMoveSpeed_Passive = 800.0f;

	/* Magnitude of the fake input vector applied in idle to ensure physics simulation remains active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suction Physics")
	float SuctionInputForce_Passive = 0.3f;

	/* True if the character is currently inside a Suction Zone */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suction State")
	bool bIsUnderSuction = false;

	/* The world location of the current suction source */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suction State")
	FVector SuctionTargetLocation;
	
	// --- Stamina Variables ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float StaminaDrainRate = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float StaminaRegenRate = 10.0f;
	
	// --- UI / HUD ---
	
	/* Widget class to spawn for the HUD (Must be set in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/* Pointer to the active HUD instance */
	UPROPERTY()
	UUserWidget* HUDWidgetInstance;
	
public:
	/* Returns the current stamina percentage (0.0 to 1.0) for UI */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetStaminaPercent() const { return MaxStamina > 0 ? CurrentStamina / MaxStamina : 0.0f; }
	
	/* Called by ASuctionZone every tick to apply physics and logic */
	void ApplySuctionForce(FVector SuctionOrigin, float Strength, float DeltaTime);
	
	/* Restores default physics values when leaving the zone */
	void ResetSuctionPhysics();
};