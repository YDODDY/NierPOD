#include "PlayerCharacter.h"


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Animation/AnimMontage.h>
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BossCharacter.h"
#include "PlayerPOD.h"
#include "EngineUtils.h"
#include "PlayerHealthWidget.h"






APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    //�÷��̾� �þ� 
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 600.0f;
    SpringArm->bUsePawnControlRotation = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box Component"));
    boxComp -> SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
    boxComp -> SetCollisionEnabled(ECollisionEnabled::NoCollision);

    weaponComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon Mesh Component"));
    weaponComp -> SetupAttachment(boxComp);
    weaponComp -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    //�÷��̾� �̵�, �þ� ���� 
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0, 540, 0);

    // ĳ������ �ִ� �̵� �ӷ°� ���ӷ��� �����Ѵ�. (cm/s)
    // 2B �ڵ����� �ٱ⿡ ������ �ɵ�
    GetCharacterMovement()->MaxWalkSpeed = 900.0f;
    GetCharacterMovement()->MaxAcceleration = 300.0f;

    // ������ ����
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.05f;

    // ���� ���� ���� ��
    JumpMaxCount = 2;
}


void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();


     //���� ���۽� currentHP > maxHP �� �ʱ�ȭ
    currentHP = maxHP;


    //�÷��̾� ��ǲ���� ����
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(InputMapping, 0);
        }
    }

    AnimInstance = GetMesh()->GetAnimInstance();
    weaponComp->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnOverlap);


    //ü�¹� UI ���� 
    if (playerHealthWidget_bp != nullptr)
    {
        playerUI = CreateWidget<UPlayerHealthWidget>(GetWorld(), playerHealthWidget_bp);
        if (playerUI != nullptr)
        {
            playerUI->AddToViewport();
        }
    }

}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // ������ 2�� �ְ� ���� �ӵ��� �ǵ�����
    if (canDash == false) {
        DashCooldown -= DeltaTime;
        if (DashCooldown <= 0) {
            //GetCharacterMovement()->Velocity = OldVelocity;
            GetCharacterMovement()->MaxWalkSpeed = 900.0f;
            GetCharacterMovement()->MaxAcceleration = 300.0f;
            DashCooldown = 0.2f;
            canDash = true;
        }
    }


}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* Input = CastChecked <UEnhancedInputComponent>(PlayerInputComponent))
    {   //��ǲ-�Լ� ���ε�
        Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
        Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::Jump);
        Input->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerCharacter::Dash);
        Input->BindAction(DodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::DodgeFunction);
        Input->BindAction(LeftAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::LeftAttack);
        Input->BindAction(RightAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::RightAttack);
        Input->BindAction(shooting, ETriggerEvent::Triggered, this, &APlayerCharacter::Shot);
    }

}

void APlayerCharacter::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABossCharacter* boss = Cast<ABossCharacter>(OtherActor);
    if (boss != nullptr)
    {
        boss->OnDamaged(damage);
    }
}



//�̵�
void APlayerCharacter::Move(const FInputActionValue& InputValue)
{
    FVector2D InputVector = InputValue.Get<FVector2D>();

    if (IsValid(Controller))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        AddMovementInput(ForwardDirection, InputVector.Y);
        AddMovementInput(RightDirection, InputVector.X);
    }
}
//�þ�
void APlayerCharacter::Look(const FInputActionValue& InputValue)
{
    FVector2D InputVector = InputValue.Get<FVector2D>();

    if (IsValid(Controller))
    {
        AddControllerYawInput(InputVector.X);
        AddControllerPitchInput(InputVector.Y);
    }
}
//����    // ���������� ���߿� �����ϸ� ���� 
void APlayerCharacter::Jump()
{
    ACharacter::Jump();
    /*
    if (GetCharacterMovement()->IsFalling() != true)
    {
        ACharacter::Jump();
    }
    */
}

// �뽬
void APlayerCharacter::Dash(const FInputActionValue& InputValue)
{
    if (canDash == true) {
        canDash = false;
        //OldVelocity = GetCharacterMovement()->Velocity;
        //GetCharacterMovement()->Velocity = GetActorForwardVector() * 5000.0f;
        // �ִ�ӵ� 5000 �ֱ�
        GetCharacterMovement()->MaxWalkSpeed = 5000.0f;
        // ���ӵ� 100000 �ֱ� - �� ������
        GetCharacterMovement()->MaxAcceleration = 100000.0f;
        AnimInstance->Montage_Play(DashMontage);
    }
}

void APlayerCharacter::DodgeFunction(const FInputActionValue& InputValue)
{
    bool bCondition = (eActionState == EStateType::Dodge);

    bCondition ^= true;

    if (bCondition)
    {
        PerformDodge();
    }
    else
    {
        bSaveDodge = true;
    }
}

void APlayerCharacter::Shot(const FInputActionValue& InputValue)
{
    for(TActorIterator<APlayerPOD> it(GetWorld());it;++it)
      {
        APlayerPOD* podshot = *it;
        if(podshot !=nullptr)
        {
            podshot->Shooting();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("null"));
        }

    }
}


void APlayerCharacter::SetActionState(EStateType eState)
{
    if (eActionState != eState)
    {
        eActionState = eState;
    }
}

void APlayerCharacter::PerformDodge()
{
    SetActionState(EStateType::Nothing);

    if (DodgeMontage != nullptr)
    {
        PlayAnimMontage(DodgeMontage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DodgeMontage Is Null"));
    }


}

// PerformLightAttack
bool APlayerCharacter::PerformLightAttack(int32 attackIndex)
{
    UAnimMontage* selectedLightAttackMontage = lightAttackMontages[attackIndex];

    if (IsValid(selectedLightAttackMontage))
    {   
        SetActionState(EStateType::Attack);

        PlayAnimMontage(selectedLightAttackMontage);

        LightAttackIndex++;


        if (LightAttackIndex >= lightAttackMontages.Num())
        {
            LightAttackIndex = 0;
        }

        return true;
    }
    else
    {
        FString strMessage = FString(TEXT("Attack Montage Not Valid"));
        UKismetSystemLibrary::PrintString(GetWorld(), *strMessage);

        return false;
    }

    return false;
}

// Reset Light Attack Variables
void APlayerCharacter::ResetLightAttackVariables()
{
    LightAttackIndex = 0;
    LightAttackSaved = false;


}

bool APlayerCharacter::PerformHeavyAttack(int32 attackIndex)
{
    UAnimMontage* selectedHeavyAttackMontage = heavyAttackMontages[attackIndex];

    if (IsValid(selectedHeavyAttackMontage)) {
     
        SetActionState(EStateType::Attack);

        PlayAnimMontage(selectedHeavyAttackMontage);

        HeavyAttackIndex++;


        if (HeavyAttackIndex >= heavyAttackMontages.Num()) {
            HeavyAttackIndex = 0;
        }
        return true;
    }
    else {
        FString strMessage = FString(TEXT("Attack Montage Not Valid"));
        UKismetSystemLibrary::PrintString(GetWorld(), *strMessage);

        return false;
    }
    return false;
}

void APlayerCharacter::ResetHeavyAttackVariables()
{
    HeavyAttackIndex = 0;
    HeavyAttackSaved = false;

}

void APlayerCharacter::PlayerDamaged()
{
    currentHP = currentHP - 10;
    UE_LOG(LogTemp, Warning, TEXT("Player HP : %d"), currentHP);
}

float APlayerCharacter::GetCurrentHealth()
{
    return float(currentHP);
}

float APlayerCharacter::GetMaxHealth()
{
    return float(maxHP);
}



void APlayerCharacter::LeftAttack(const FInputActionValue& InputValue)
{
    UE_LOG(LogTemp, Warning, TEXT("LeftAttack"));

    PerformLightAttack(LightAttackIndex);

}

void APlayerCharacter::RightAttack(const FInputActionValue& InputValue)
{
    UE_LOG(LogTemp, Warning, TEXT("RightAttack"));

    PerformHeavyAttack(HeavyAttackIndex);

}
