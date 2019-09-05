// Copyright Unreal Engine Community.

#include "WheeledVehicleNW.h"
#include "truckProject2_0WheelFront.h"
#include "truckProject2_0WheelRear.h"
#include "truckProject2_0Hud.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "VehicleMovementComponentNW.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"

#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "vehicle\PxVehicleComponents.h"


#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif // HMD_MODULE_INCLUDED

const FName AWheeledVehicleNW::LookUpBinding("LookUp");
const FName AWheeledVehicleNW::LookRightBinding("LookRight");
const FName AWheeledVehicleNW::EngineAudioRPM("RPM");



#define LOCTEXT_NAMESPACE "VehicleNW"

AWheeledVehicleNW::AWheeledVehicleNW(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UVehicleMovementComponentNW>(VehicleMovementComponentName))
{
	// Car mesh.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/PolygonScifi/Meshes/Vehicles/SK_Veh_Garbage_01.SK_Veh_Garbage_01"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/PolygonScifi/Meshes/Vehicles/SK_Veh_Garbage_01_animation"));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

	// Setup friction materials.
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> SlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/Slippery.Slippery"));
	SlipperyMaterial = SlipperyMat.Object;

	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> NonSlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/NonSlippery.NonSlippery"));
	NonSlipperyMaterial = NonSlipperyMat.Object;

	UVehicleMovementComponentNW* VehicleNW = CastChecked<UVehicleMovementComponentNW>(GetVehicleMovementComponent());

	check(VehicleNW->WheelSetups.Num() == 6);

	

	// Wheels/Tires.
	// Setup the wheels.
	VehicleNW->WheelSetups[0].WheelClass = UtruckProject2_0WheelFront::StaticClass();
	VehicleNW->WheelSetups[0].BoneName = FName("SK_Veh_Garbage_01_Wheel_fl");
	VehicleNW->WheelSetups[0].AdditionalOffset = FVector(0.f, 0.f, 0.f);
	
	VehicleNW->WheelSetups[1].WheelClass = UtruckProject2_0WheelFront::StaticClass();
	VehicleNW->WheelSetups[1].BoneName = FName("SK_Veh_Garbage_01_Wheel_fr");
	VehicleNW->WheelSetups[1].AdditionalOffset = FVector(0.f, 0.f, 0.f);

	VehicleNW->WheelSetups[2].WheelClass = UtruckProject2_0WheelRear::StaticClass();
	VehicleNW->WheelSetups[2].BoneName = FName("SK_Veh_Garbage_01_Wheel_rl_02");
	VehicleNW->WheelSetups[2].AdditionalOffset = FVector(0.f, 0.f, 0.f);

	VehicleNW->WheelSetups[3].WheelClass = UtruckProject2_0WheelRear::StaticClass();
	VehicleNW->WheelSetups[3].BoneName = FName("SK_Veh_Garbage_01_Wheel_rr_02");
	VehicleNW->WheelSetups[3].AdditionalOffset = FVector(0.f, 0.f, 0.f);

	VehicleNW->WheelSetups[4].WheelClass = UtruckProject2_0WheelRear::StaticClass();
	VehicleNW->WheelSetups[4].BoneName = FName("SK_Veh_Garbage_01_Wheel_rl_01");
	VehicleNW->WheelSetups[4].AdditionalOffset = FVector(0.f, 0.f, 0.f);

	VehicleNW->WheelSetups[5].WheelClass = UtruckProject2_0WheelRear::StaticClass();
	VehicleNW->WheelSetups[5].BoneName = FName("SK_Veh_Garbage_01_Wheel_rr_01");
	VehicleNW->WheelSetups[5].AdditionalOffset = FVector(0.f, 0.f, 0.f);


	// Adjust the tire loading.
	VehicleNW->MinNormalizedTireLoad = 0.0f;
	VehicleNW->MinNormalizedTireLoadFiltered = 0.2f;
	VehicleNW->MaxNormalizedTireLoad = 2.0f;
	VehicleNW->MaxNormalizedTireLoadFiltered = 2.0f;

	// Engine.
	// Torque setup.
	VehicleNW->MaxEngineRPM = 5700.0f;
	VehicleNW->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	VehicleNW->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	VehicleNW->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	VehicleNW->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);

	// Adjust the steering .
	VehicleNW->SteeringCurve.GetRichCurve()->Reset();
	VehicleNW->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	VehicleNW->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	VehicleNW->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	// Automatic gearbox.
	VehicleNW->TransmissionSetup.bUseGearAutoBox = true;
	VehicleNW->TransmissionSetup.GearSwitchTime = 0.15f;
	VehicleNW->TransmissionSetup.GearAutoBoxLatency = 1.0f;

	// Physics settings.
	// Adjust the center of mass - the buggy is quite low.
	UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleNW->UpdatedComponent);
	if (UpdatedPrimitive)
	{
		UpdatedPrimitive->BodyInstance.COMNudge = FVector(0.0f, 0.0f, 0.0f);
	}

	// Set the inertia scale. This controls how the mass of the vehicle is distributed.
	VehicleNW->InertiaTensorScale = FVector(1.0f, 1.333f, 1.2f);

	// Create a spring arm component for our chase camera.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 34.0f));
	SpringArm->SetWorldRotation(FRotator(-20.0f, 0.0f, 0.0f));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 125.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	// Create the chase camera component .
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->SetRelativeRotation(FRotator(10.0f, 0.0f, 0.0f));
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component .
	InternalCameraOrigin = FVector(-34.0f, 0.0f, 50.0f);
	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	//InternalCamera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetRelativeLocation(InternalCameraOrigin);
	InternalCamera->AttachTo(GetMesh());

	// In car HUD.
	// Create text render component for in car speed display.
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarSpeed->SetRelativeLocation(FVector(35.0f, -6.0f, 20.0f));
	InCarSpeed->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarSpeed->AttachTo(GetMesh());

	// Create text render component for in car gear display.
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarGear->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
	InCarGear->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarGear->AttachTo(GetMesh());

	// Setup the audio component and allocate it a sound cue.
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("/Game/VehicleAdv/Sound/Engine_Loop_Cue.Engine_Loop_Cue"));
	EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	EngineSoundComponent->SetSound(SoundCue.Object);
	EngineSoundComponent->AttachTo(GetMesh());

	// Colors for the in-car gear display. One for normal one for reverse.
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bIsLowFriction = false;
	bInReverseGear = false;
}

void AWheeledVehicleNW::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings.
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWheeledVehicleNW::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWheeledVehicleNW::MoveRight);
	PlayerInputComponent->BindAxis(LookUpBinding);
	PlayerInputComponent->BindAxis(LookRightBinding);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AWheeledVehicleNW::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AWheeledVehicleNW::OnHandbrakeReleased);
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AWheeledVehicleNW::OnToggleCamera);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AWheeledVehicleNW::OnResetVR);
}

void AWheeledVehicleNW::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);
}

void AWheeledVehicleNW::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void AWheeledVehicleNW::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AWheeledVehicleNW::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AWheeledVehicleNW::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void AWheeledVehicleNW::EnableIncarView(const bool bState)
{
	if (bState != bInCarCameraActive)
	{
		bInCarCameraActive = bState;

		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}

		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}

void AWheeledVehicleNW::Tick(float Delta)
{
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear.
	bInReverseGear = GetVehicleMovementComponent()->GetCurrentGear() < 0;

	// Update physics material.
	UpdatePhysicsMaterial();

	// Update the strings used in the HUD (In-car and On-screen).
	UpdateHUDStrings();

	// Set the string in the In-car HUD.
	SetupInCarHUD();

	bool bHMDActive = false;
#ifdef HMD_INTGERATION
	if ((GEngine->HMDDevice.IsValid() == true) && ((GEngine->HMDDevice->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
	{
		bHMDActive = true;
	}
#endif // HMD_INTGERATION.
	if (bHMDActive == false)
	{
		if ((InputComponent) && (bInCarCameraActive == true))
		{
			FRotator HeadRotation = InternalCamera->RelativeRotation;
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->RelativeRotation = HeadRotation;
		}
	}

	// Pass the engine RPM to the sound component.
	float RPMToAudioScale = 2500.0f / GetVehicleMovementComponent()->GetEngineMaxRotationSpeed();
	EngineSoundComponent->SetFloatParameter(EngineAudioRPM, GetVehicleMovementComponent()->GetEngineRotationSpeed()*RPMToAudioScale);
}

void AWheeledVehicleNW::BeginPlay()
{
	Super::BeginPlay();

	bool bWantInCar = false;
	// First disable both speed/gear displays.
	bInCarCameraActive = false;
	InCarSpeed->SetVisibility(bInCarCameraActive);
	InCarGear->SetVisibility(bInCarCameraActive);

#ifdef HMD_INTGERATION
	// Enable in car view if HMD is attached.
	bWantInCar = GEngine->HMDDevice.IsValid()
#endif // HMD_INTGERATION

	EnableIncarView(bWantInCar);
	// Start an engine sound playing.
	EngineSoundComponent->Play();
}

void AWheeledVehicleNW::OnResetVR()
{
#ifdef HMD_INTGERATION
	if (GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_INTGERATION.
}

void AWheeledVehicleNW::UpdateHUDStrings()
{
	float KPH = FMath::Abs(GetVehicleMovementComponent()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);

	// Using FText because this is display text that should be localizable.
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));

	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		int32 Gear = GetVehicleMovementComponent()->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}
}

void AWheeledVehicleNW::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr))
	{
		// Setup the text render component strings.
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);

		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

void AWheeledVehicleNW::UpdatePhysicsMaterial()
{
	if (GetActorUpVector().Z < 0)
	{
		if (bIsLowFriction == true)
		{
			GetMesh()->SetPhysMaterialOverride(NonSlipperyMaterial);
			bIsLowFriction = false;
		}
		else
		{
			GetMesh()->SetPhysMaterialOverride(SlipperyMaterial);
			bIsLowFriction = true;
		}
	}
}