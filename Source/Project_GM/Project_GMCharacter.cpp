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
#include "PhysicalMaterials/PhysicalMaterial.h"

// Include component headers to use their functionality
#include "CharacterComponents/DashComponent.h"
#include "CharacterComponents/WallMechanicsComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AProject_GMCharacter::AProject_GMCharacter()
{
	// Enable tick for sliding logic monitoring
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Disable controller rotation affecting the character mesh directly
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	// Movement defaults
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	// Enable Double Jump
	JumpMaxCount = 2;

	// Store default physics values for reset logic later
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DefaultCrouchedWalkSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;

	// Enable crouching capability on the movement component
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 
}

void AProject_GMCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Cache default movement values to restore them later
	if (GetCharacterMovement())
	{
		DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
		DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
		DefaultMaxAcceleration = GetCharacterMovement()->MaxAcceleration;
	}
}

void AProject_GMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jump is bound to DoJumpStart to allow intercepting input for Wall Jump check
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AProject_GMCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Look);

		// Dashing
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AProject_GMCharacter::OnDash);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopSprint);

		// Crouching and Sliding
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AProject_GMCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AProject_GMCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// --- MOVEMENT ABILITIES & LOGIC ---

void AProject_GMCharacter::DoJumpStart()
{
	// Try to find the Wall Mechanics Component
	UWallMechanicsComponent* WallMechComp = FindComponentByClass<UWallMechanicsComponent>();

	// Attempt Wall Jump if component exists
	if (WallMechComp && WallMechComp->AttemptWallJump())
	{
		// Reset jump count on successful wall jump
		JumpCurrentCount = 0;
		return;
	}

	// Fallback to standard jump behavior
	Jump();
}

void AProject_GMCharacter::OnDash()
{
	// Prevent dashing while on the ground to allow Sprint input priority
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	// Try to find the Dash Component representing the unlockable ability
	UDashComponent* DashComp = FindComponentByClass<UDashComponent>();

	if (DashComp)
	{
		DashComp->PerformDash();
	}
}

void AProject_GMCharacter::StartSprint()
{
	// Only allow sprint if not currently crouching or sliding
	if (!bIsCrouched && !bIsSliding)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void AProject_GMCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void AProject_GMCharacter::StartCrouch()
{
	float GroundSpeed = GetVelocity().Size2D();

	// Check if speed is sufficient to initiate a slide while on ground
	if (GetCharacterMovement()->IsMovingOnGround() && GroundSpeed >= MinSlideSpeed)
	{
		bIsSliding = true;
		
		// Reduce friction to allow sliding
		GetCharacterMovement()->GroundFriction = SlideFriction;
		
		// Allow high speed while crouched for the slide duration
		GetCharacterMovement()->MaxWalkSpeedCrouched = SprintSpeed; 
		
		Crouch();
	}
	else
	{
		// Perform standard crouch
		bIsSliding = false;
		Crouch();
	}
}

void AProject_GMCharacter::StopCrouch()
{
	StopSlide();
	UnCrouch();
}

void AProject_GMCharacter::StopSlide()
{
	if (bIsSliding)
	{
		bIsSliding = false;
		
		// Restore default movement values
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
		GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultCrouchedWalkSpeed;
	}
}

void AProject_GMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Monitor sliding state
	if (bIsSliding)
	{
		float GroundSpeed = GetVelocity().Size2D();

		// Stop sliding if speed drops too low or player is no longer grounded
		if (GroundSpeed < MinSlideSpeed || !GetCharacterMovement()->IsMovingOnGround())
		{
			StopSlide();
		}
	}
	
	// Check surface type every frame
	HandleGroundPhysics();
}

void AProject_GMCharacter::HandleGroundPhysics()
{
    // Safety Check
    if (!GetCharacterMovement()) return;

    // Surface Detection (Line Trace)
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.0f, 0.0f, 150.0f); // Trace down

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    bool bIsOnIce = false;

    // Check Material Type
    if (bHit && HitResult.PhysMaterial.IsValid())
    {
        // SurfaceType1 is "Ice" (Check Project Settings -> Physics)
        if (HitResult.PhysMaterial->SurfaceType == EPhysicalSurface::SurfaceType1)
        {
            bIsOnIce = true;
        }
    }

    // Apply Physics
    if (bIsOnIce)
    {
        // --- ICE BEHAVIOR ---
        
        // Zero friction allows sliding without losing speed
        GetCharacterMovement()->GroundFriction = 0.0f; 
        
        // Zero braking means the character won't try to stop automatically
        GetCharacterMovement()->BrakingDecelerationWalking = 0.0f; 
        
        // Harder to change direction or start moving
        GetCharacterMovement()->MaxAcceleration = IceMaxAcceleration; 
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 60.0f, 0.0f); // Slow rotation

        // --- SLOPE GRAVITY LOGIC ---
        FVector FloorNormal = HitResult.ImpactNormal;

        // If the floor is NOT flat (Z < 1.0 means it has a slope)
        if (FloorNormal.Z < 0.99f) 
        {
            // Get the direction pointing "Downhill"
            FVector SlopeDir = FVector::VectorPlaneProject(FVector::DownVector, FloorNormal).GetSafeNormal();

            // Calculate steepness factor (0 to 1)
            float Steepness = 1.0f - FloorNormal.Z;

            // Apply Force: Direction * Force * Steepness * Mass
            // This ensures the character slides down even if standing still
            FVector SlideForce = SlopeDir * IceSlopeGravityForce * Steepness * GetCharacterMovement()->Mass;
            
            GetCharacterMovement()->AddForce(SlideForce);
        }
    }
    else
    {
        // --- NORMAL BEHAVIOR (Restore Defaults) ---
        GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
        GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
        GetCharacterMovement()->MaxAcceleration = DefaultMaxAcceleration;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // Fast rotation
    }
}