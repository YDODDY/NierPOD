

#include "BossAnimInstance.h"

void UBossAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	// ����ĳ���� ��������
	boss = Cast<ABossCharacter>(GetOwningActor());


}

void UBossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (boss != nullptr)
	{	//������ Enum BossState �� ������ 
		currentState = boss->bossState;	

	}
}

void UBossAnimInstance::AnimNotify_AttackStarted()
{	//���ý��۽� �� �����ݸ��� ũ�� Ŀ��
	boss->BoxCollisionExtending();
}

void UBossAnimInstance::AnimNotify_AttackEnded()
{	//���� ������ �� �����ݸ��� ũ�� �ʱ�ȭ
	boss->BoxCollisionReset();

}

//�״� �ִϸ��̼� �� 5�� �� ���� �����
void UBossAnimInstance::AnimNotify_Destroy()
{
	if (boss != nullptr && !GetWorld()->GetTimerManager().IsTimerActive(deathTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(deathTimer, FTimerDelegate::CreateLambda([&]() {

		boss->Destroy();
			}), 5.0f, false);
	}
}
