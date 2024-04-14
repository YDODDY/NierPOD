

#include "BossController.h"
#include "NavigationSystem.h"

void ABossController::BeginPlay()
{
	Super::BeginPlay();

	//�������� ���ƴٴ� ��ü����
	NavArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);

	RandomPatrol();

}

void ABossController::RandomPatrol()
{	
	if (GetPawn() != nullptr)
	{
		//������ ���� ������ ���� �� �ִ� ������ ��ġ�� ��
		NavArea->K2_GetRandomReachablePointInRadius(GetWorld(), GetPawn()->GetActorLocation(), RandomLocation, 15000.0f);

		MoveToLocation(RandomLocation);
	}
}
