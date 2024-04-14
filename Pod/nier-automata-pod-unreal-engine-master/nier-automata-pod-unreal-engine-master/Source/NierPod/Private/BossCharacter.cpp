

#include "BossCharacter.h"
#include "EngineUtils.h"
#include "PlayerCharacter.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Shield.h"
#include "BossHealthWidget.h"

ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	leftFootCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftFoot Attack Collision"));
	leftFootCollision -> SetupAttachment(GetMesh(), TEXT("LeftToeBaseSocket"));
	leftFootCollision -> SetRelativeLocation(FVector(0, 10, 0));
	leftFootCollision -> SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));

	rightFootCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightFoot Attack Collision"));
	rightFootCollision -> SetupAttachment(GetMesh(), TEXT("RightFootToeBaseSocket"));
	rightFootCollision -> SetRelativeLocation(FVector(-10, -20, 0));
	rightFootCollision -> SetRelativeScale3D(FVector(0.2f,0.2f,0.2f));

	leftFootCollision -> OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnDealDamageOverlapBegin);
	rightFootCollision -> OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnDealDamageOverlapBegin);


	//�⺻ ĸ��������Ʈ preset ����
	GetCapsuleComponent()->SetCollisionProfileName(FName("BossPreset"));

	leftFootCollision->SetCollisionProfileName(FName("BossAttackPreset"));
	rightFootCollision->SetCollisionProfileName(FName("BossAttackPreset"));

	

}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	//�⺻���� IDLE ���·� �ʱ�ȭ
	bossState = EBossState::IDLE;

	//���忡 �ִ� �÷��̾� ã�� target �� �־�� 
	for (TActorIterator<APlayerCharacter> player(GetWorld()); player; ++player)
	{
		target = *player;
	}
	
	//���� ü�� max �� �ʱ�ȭ
	currentHP = maxHP;

	//ü�¹� UI ���� 
	if (bossHealthWidget_bp != nullptr)
	{
		bossUI = CreateWidget<UBossHealthWidget>(GetWorld(), bossHealthWidget_bp);
		if (bossUI != nullptr)
		{
			bossUI->AddToViewport();
		}
	}
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (bossState)
	{
	case EBossState::IDLE:
		Idle(DeltaTime);
		break;
	case EBossState::MOVE:
		MoveToTarget(DeltaTime);
		break;
	case EBossState::ATTACKREADY:
		AttackReady();
		break;
	case EBossState::ATTACK:
		Attack();
		break;
	case EBossState::ATTACK2:
		Attack2();
		break;
	case EBossState::JUMPATTACK:
		JumpAttack();
		break;
	case EBossState::ATTACKDELAY:
		AttackDelay(DeltaTime);
		break;
	case EBossState::BLOCK:
		Blocking(DeltaTime);
		break;
	case EBossState::BLOCKATTACK:
		BlocKAttack();
		break;
	case EBossState::DAMAGED:
		DamageProcess(DeltaTime);
		break;
	case EBossState::DIE:
		break;
	default:

		break;
	}
}

void ABossCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

double ABossCharacter::GetMaxHealth()
{
	return double(maxHP);
}

double ABossCharacter::GetHealth()
{
	return double(currentHP);
}

void ABossCharacter::Idle(float DeltaSeconds)
{	
	//���� �ǰ� 900 ���϶�� MOVE ���·� ��ȯ
	if (currentHP <= 900.0f)
	{
		bossState = EBossState::MOVE;
	}
}

void ABossCharacter::AttackReady()
{
	UE_LOG(LogTemp, Warning, TEXT("READY TO ATTACK PLAYER"));
	//�÷��̾� �Ÿ��� attackDistance ���� ������
	if (FVector::Distance(GetActorLocation(), target->GetActorLocation()) < attackDistance)
		{	int32 num = FMath::RandRange(1,3);
			if(num == 1)
			{
				bossState = EBossState::ATTACK;
			}
			if(num == 2)
			{
				bossState = EBossState::ATTACK2;
			}
			if(num == 3)
			{
				bossState = EBossState::JUMPATTACK;
			}
	}
	else
	{
		bossState = EBossState::MOVE;
	}
}

void ABossCharacter::Attack()
{	
	//�÷��̾� �Ÿ��� attackDistance ���� ������
	if (FVector::Distance(GetActorLocation(), target->GetActorLocation()) < attackDistance + 10.0f)
	{	

		UE_LOG(LogTemp, Warning, TEXT("Boss Kick!"));
		bossState = EBossState::ATTACKDELAY;
	}
	//�÷��̾� �Ÿ��� attackDistance ���� ũ��
	else
	{	//�÷��̾� �ٽ� ����
		bossState = EBossState::MOVE;
	}
}

void ABossCharacter::Attack2()
{
	//�÷��̾� �Ÿ��� attackDistance ���� ������
	if (FVector::Distance(GetActorLocation(), target->GetActorLocation()) < attackDistance + 10.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Kick 2 !"));
		bossState = EBossState::ATTACKDELAY;
	}
	//�÷��̾� �Ÿ��� attackDistance ���� ũ��
	else
	{	//�÷��̾� �ٽ� ����
		bossState = EBossState::MOVE;
	}
}

void ABossCharacter::JumpAttack()
{
	//�÷��̾� �Ÿ��� attackDistance ���� ������
	if (FVector::Distance(GetActorLocation(), target->GetActorLocation()) < attackDistance + 10.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Jump Kick !"));
		bossState = EBossState::ATTACKDELAY;
	}
	//�÷��̾� �Ÿ��� attackDistance ���� ũ��
	else
	{	//�÷��̾� �ٽ� ����
		bossState = EBossState::MOVE;
	}
}

void ABossCharacter::MoveToTarget(float deltaSeconds)
{	
	//�÷��̾� �������
	FVector targetDir = target->GetActorLocation() - GetActorLocation();
	targetDir.Z = 0;

	// �÷��̾ ���ݹ��� ���̶��
	if (targetDir.Length() > attackDistance)
	{
		//�÷��̾� ����
		GetCharacterMovement()->MaxWalkSpeed = traceSpeed;
		AddMovementInput(targetDir.GetSafeNormal());

		//�̵� �������� ȸ��

		FRotator currentRot = GetActorRotation();
		FRotator targetRot = targetDir.ToOrientationRotator();

		FRotator calcRot = FMath::Lerp(currentRot, targetRot, deltaSeconds * rotSpeed);

		SetActorRotation(calcRot);
	}
	// �÷��̾ ���ݹ��� �ȿ� �ִٸ� 
	else
	{	// �����غ�� ��ȯ
		bossState = EBossState::ATTACKREADY;
	}
}

void ABossCharacter::AttackDelay(float deltaSeconds)
{

	currentTime += deltaSeconds;
	if (currentTime > attackDelayTime)
	{
		currentTime = 0;
		bossState = EBossState::ATTACKREADY;
	}
	
	if(FVector::Distance(GetActorLocation(), target->GetActorLocation()) > attackDistance + 10.f)
	{	
		if(currentTime > attackDelayTime)
			{ 
				bossState = EBossState::MOVE;
			}
	}
}

void ABossCharacter::Blocking(float deltaSeconds)
{	//���� �ǰ� 800 ���Ϸ� ���� �� ���� ��� ����

		//���ڸ����� ���� ����
		GetWorld()->SpawnActor<AShield>(shield, GetActorLocation(), GetActorRotation());
		SetActorLocation(GetActorLocation());
		//2 �� ������ �� Ȯ��������(Shield : 1.5�� �� ������ �����)
		currentTime += deltaSeconds;
		if(currentTime > 2.0f)
		{	
			currentTime = 0;
			if(shield != nullptr)
			{ 
				int32 num = FMath::RandRange(1,2);
				if (num == 1)
				{		//MOVE ���·� ��ȯ
					bossState = EBossState::MOVE;
				}
				if (num == 2)
				{ 
					bossState = EBossState::BLOCKATTACK;
				}
			}
		}
}

void ABossCharacter::BlocKAttack()
{	
	//������� �� MOVE ��ȯ
	UE_LOG(LogTemp, Warning, TEXT("SHIELD ATTACK"));
	bossState = EBossState::MOVE;
}

void ABossCharacter::DamageProcess(float deltaSeconds)
{
	FVector targetLoc = hitLocation + GetActorLocation() * -1.0f;
	FVector knockBackLocation = FMath::Lerp(GetActorLocation(), targetLoc, deltaSeconds * 1.5f);
	//���� ü���� 800 ���ϰ� �ƴ϶�� 
	if(currentHP > 900)
	{	
		//���� �ڷ� �����ɱ� �з��� 
		SetActorLocation(knockBackLocation, true);
		UE_LOG(LogTemp, Warning, TEXT("%d"), currentHP);
		bossState = EBossState::IDLE;
	}
	//���� ü���� 800 ���϶�� 
	else
	{ 
			//Ȯ��������
			int32 num = FMath::RandRange(0,5);
			if(num > 0)
			{ 
				//�°ų�
				if (FVector::Distance(GetActorLocation(), targetLoc) > 10)
				{	
					//���� �ڷ� �����ɱ� �з��� 
					SetActorLocation(knockBackLocation, true);
					UE_LOG(LogTemp, Warning, TEXT("%d"), currentHP);
				}
				else
				{
					bossState = EBossState::MOVE;
				}
			}
			//���
			else
			{
				bossState = EBossState::BLOCK;
			}
	}	
}

void ABossCharacter::BoxCollisionExtending()
{	//���� �ִϸ��̼ǽ� �� �ݸ��� Ŀ�� 
	rightFootCollision->SetRelativeScale3D(FVector(3.0f));
	leftFootCollision->SetRelativeScale3D(FVector(3.0f));
}

void ABossCharacter::BoxCollisionReset()
{	//���� �ִϸ��̼� ������ �� �ݸ��� �����·� ���ư� 
	rightFootCollision->SetRelativeScale3D(FVector(0.2f));
	leftFootCollision->SetRelativeScale3D(FVector(0.2f));
}

void ABossCharacter::OnDamaged(int32 dmg)
{
	if (bossState == EBossState::BLOCK)
	{
		return;
	}
	if (bossState == EBossState::DAMAGED)
	{
		return;
	}
	//HP���� 0 ~ maxHP �� ���̿��� ���� �� �ְ� ����
	currentHP = FMath::Clamp(currentHP - dmg, 0, maxHP);

	// ������ ��� ���, ���� ü���� 0 ���� ũ��
	if (currentHP > 0)
	{
		//DamageProcess �� ��ȯ
		bossState = EBossState::DAMAGED;
		hitLocation = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("%d"), currentHP);
	}
	// ������ ��� ���, ���� ü���� 0 ���϶�� 
	if (currentHP <= 0)
	{
		//DIE 
		bossState = EBossState::DIE;
		Die();
	}
}

//�÷��̾� ������ �ֱ�
void ABossCharacter::OnDealDamageOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SewwpResult)
{
	// �޹�, ������ ���� �ݸ����� �÷��̾ ��Ҵٸ�
	APlayerCharacter* player = Cast<APlayerCharacter>(OtherActor);
	if (player != nullptr)
	{	
		player->PlayerDamaged();
		UE_LOG(LogTemp, Warning, TEXT("PlayerDamaged"));
	}
}

void ABossCharacter::Die()
{	
	//�ݸ��� �� ������
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	leftFootCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	rightFootCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->DisableMovement();

	PlayAnimMontage(death_Montage);
}
