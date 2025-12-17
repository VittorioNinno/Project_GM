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

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay() override;
	
	/** Called every frame */
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

	/** Helper function to stop the slide and reset physics */
	void StopSlide();
	
	// --- Ice Physics Configuration ---
    
	// Friction level on ice (Lower = more slippery)
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceGroundFriction = 0.1f; 

	// Deceleration when stopping on ice (Lower = sliding effect)
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceBrakingDeceleration = 100.0f; 

	// Max acceleration on ice (Lower = harder to start moving)
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceMaxAcceleration = 400.0f; 
	
	UPROPERTY(EditAnywhere, Category = "Movement|Ice")
	float IceSlopeGravityForce = 5000.0f;

	// --- Variables to store default movement values ---
	float DefaultBrakingDeceleration;
	float DefaultMaxAcceleration;
	
	// Handles ground physics detection and applies movement changes
	void HandleGroundPhysics();
};