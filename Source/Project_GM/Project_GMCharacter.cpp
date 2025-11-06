#include "Project_GMCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Project_GM.h"

AProject_GMCharacter::AProject_GMCharacter()
{
	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Initialize dash cooldown
	LastDashTime = -DashCooldown; // Allows dashing immediately

	// Allows the character to double jump (1 on ground + 1 in air)
	JumpMaxCount = 2;

	// Enable crouching
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	// Store default friction
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;

	// Store default walk speed
	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Store default crouched walk speed
	DefaultCrouchedWalkSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AProject_GMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Look);

		// Dashing
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AProject_GMCharacter::OnDash);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopCrouch);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopSprint);
	}
	else
	{
		UE_LOG(LogProject_GM, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

/** This is the Tick function, called every frame */
void AProject_GMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if we should stop sliding
	if (bIsSliding)
	{
		float GroundSpeed = GetVelocity().Size2D();

		// Stop sliding if we lose speed or fall off a ledge
		if (GroundSpeed < MinSlideSpeed || !GetCharacterMovement()->IsMovingOnGround())
		{
			// Stop the slide logic, but don't UnCrouch().
			// This allows the player to remain crouched if they are still
			// holding the crouch button.
			StopSlide();
		}
	}
}

void AProject_GMCharacter::Move(const FInputActionValue& Value)
{
	//	input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	//	route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AProject_GMCharacter::Look(const FInputActionValue& Value)
{
	//	input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	//	route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AProject_GMCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		//	find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		//	get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		//	get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		//	add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AProject_GMCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		//	add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AProject_GMCharacter::DoJumpStart()
{
	//	signal the character to jump
	Jump();
}

void AProject_GMCharacter::DoJumpEnd()
{
	//	signal the character to stop jumping
	StopJumping();
}

void AProject_GMCharacter::OnDash()
{
	// Check if the game world is valid and the cooldown has elapsed
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetTimeSeconds() < LastDashTime + DashCooldown)
	{
		// Cooldown not finished yet
		return;
	}

	// Calculate dash direction
	FVector DashDirection;

	// Prioritize the player's input direction
	FVector InputAcceleration = GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal();
	if (!InputAcceleration.IsNearlyZero())
	{
		DashDirection = InputAcceleration;
	}
	else
	{
		// If no input, dash in the view's forward direction
		DashDirection = FRotationMatrix(GetController()->GetControlRotation()).GetScaledAxis(EAxis::X);
		DashDirection.Z = 0;
		DashDirection.Normalize();
	}
	
	// If the direction is somehow zero, use the character's forward vector
	if (DashDirection.IsNearlyZero())
	{
		DashDirection = GetActorForwardVector();
	}

	// Apply the impulse
	LaunchCharacter(DashDirection * DashStrength, true, true);

	// Update the last dash timestamp
	LastDashTime = World->GetTimeSeconds();
}

void AProject_GMCharacter::StartCrouch()
{
	// Get current ground speed
	float GroundSpeed = GetVelocity().Size2D();

	// Check if we are on the ground and moving fast enough to slide
	if (GetCharacterMovement()->IsMovingOnGround() && GroundSpeed >= MinSlideSpeed)
	{
		bIsSliding = true;
		GetCharacterMovement()->GroundFriction = SlideFriction;
		
		//	We must allow high speed while crouched
		GetCharacterMovement()->MaxWalkSpeedCrouched = SprintSpeed; 
		
		Crouch();
	}
	else
	{
		// If not moving fast enough, just do a normal crouch
		bIsSliding = false;
		Crouch(); // Call base crouch
	}
}

void AProject_GMCharacter::StopCrouch()
{
	// Stop sliding if we are
	StopSlide();

	// Call the base UnCrouch() function
	UnCrouch();
}

void AProject_GMCharacter::StartSprint()
{
	if (!bIsCrouched && !bIsSliding)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void AProject_GMCharacter::StopSprint()
{
	// Restore default walk speed
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void AProject_GMCharacter::StopSlide()
{
	if (bIsSliding)
	{
		bIsSliding = false;
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;

		// Restore the default crouch speed
		GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultCrouchedWalkSpeed;
	}
}