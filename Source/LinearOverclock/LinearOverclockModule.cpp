#include "LinearOverclockModule.h"

#include "KismetAnimationLibrary.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturerVariablePower.h"
#include "Kismet/KismetMathLibrary.h"
#include "Patching/NativeHookManager.h"

void FLinearOverclockModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGBuildableFactory::GetProducingPowerConsumption, [](auto& scope, const AFGBuildableFactory* self)
	{
		if(auto variablePowerMachine = Cast<AFGBuildableManufacturerVariablePower>(self))
		{
			auto recipe = variablePowerMachine->GetCurrentRecipe().GetDefaultObject();
			if(!recipe) return;
			auto minConsumption = recipe->GetPowerConsumptionConstant() * variablePowerMachine->GetCurrentPotential();
			auto maxConsumption = (recipe->GetPowerConsumptionConstant() + recipe->GetPowerConsumptionFactor()) * variablePowerMachine->GetCurrentPotential();
			
			auto currentUsage = FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(minConsumption, maxConsumption), variablePowerMachine->mPowerConsumptionCurve->GetFloatValue(self->GetProductionProgress()));

			scope.Override(currentUsage);
		}
		else scope.Override(self->GetDefaultProducingPowerConsumption() * self->GetCurrentPotential());
	});


	SUBSCRIBE_METHOD(AFGBuildableManufacturerVariablePower::GetMinPowerConsumption, [](auto& scope, const AFGBuildableManufacturerVariablePower* self)
	{
		auto recipe = self->GetCurrentRecipe().GetDefaultObject();
		if(!recipe) return;
		
		scope.Override(recipe->GetPowerConsumptionConstant() * self->GetCurrentPotential());
	});

	SUBSCRIBE_METHOD(AFGBuildableManufacturerVariablePower::GetMaxPowerConsumption, [](auto& scope, const AFGBuildableManufacturerVariablePower* self)
	{
		auto recipe = self->GetCurrentRecipe().GetDefaultObject();
		if(!recipe) return;
		
		scope.Override( (recipe->GetPowerConsumptionConstant() + recipe->GetPowerConsumptionFactor()) * self->GetCurrentPotential() );
	});
#endif
}

IMPLEMENT_GAME_MODULE(FLinearOverclockModule, LinearOverclock);