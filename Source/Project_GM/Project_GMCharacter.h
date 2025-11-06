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
/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AProject_GMCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Dash Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DashAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

public:
	/** Constructor */
	AProject_GMCharacter();

	/** Dash strength */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DashStrength = 2000.f;

	/** Dash cooldown (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DashCooldown = 1.0f;

	/** Speed of the character when sprinting */
	UPROPERTY(EditAnywhere, Category="Movement")
	float SprintSpeed = 800.f;

	/** The minimum speed (cm/s) to initiate a slide */
	UPROPERTY(EditAnywhere, Category="Movement|Slide")
	float MinSlideSpeed = 600.f;

	/** The friction to apply when sliding (0 = no friction, 1 = default friction) */
	UPROPERTY(EditAnywhere, Category="Movement|Slide")
	float SlideFriction = 0.1f;

protected:
	/** Timestamp dell'ultimo scatto, per gestire il cooldown */
	float LastDashTime;

	/** The default walk speed of the character */
	float DefaultWalkSpeed;

	/** The default ground friction of the character */
	float DefaultGroundFriction;

	/** The default crouched walk speed of the character */
	float DefaultCrouchedWalkSpeed;

	/** True if the player is currently sliding */
	bool bIsSliding;
	
protected:
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for dash input */
	void OnDash();

	/** Called for crouch input start */
	void StartCrouch();

	/** Called for crouch input stop */
	void StopCrouch();

	/** Called for sprint input start */
	void StartSprint();

	/** Called for sprint input stop */
	void StopSprint();

	/** Helper function to stop the slide */
	void StopSlide();
	
public:
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

