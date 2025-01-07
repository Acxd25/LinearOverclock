#include "LinearOverclockModule.h"

#include "KismetAnimationLibrary.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturerVariablePower.h"
#include "Kismet/KismetMathLibrary.h"
#include "Patching/NativeHookManager.h"

void FLinearOverclockModule::StartupModule() {
	
	if(!WITH_EDITOR)
	{
		SUBSCRIBE_METHOD(AFGBuildableFactory::GetProducingPowerConsumption, [](auto& Scope, const AFGBuildableFactory* Self)
		{
			if (!Self->IsProducing()) return;
			
			if(auto VariablePowerMachine = Cast<AFGBuildableManufacturerVariablePower>(Self))
			{
				auto Recipe = VariablePowerMachine->GetCurrentRecipe().GetDefaultObject();
				if(!Recipe) return;
			
				auto MinConsumption = Recipe->GetPowerConsumptionConstant() * VariablePowerMachine->GetCurrentPotential();
				auto MaxConsumption = (Recipe->GetPowerConsumptionConstant() + Recipe->GetPowerConsumptionFactor()) * VariablePowerMachine->GetCurrentPotential();

				auto CurveValue = VariablePowerMachine->mPowerConsumptionCurve->GetFloatValue(Self->GetProductionProgress());
				
				auto CurrentUsage = MinConsumption + (MaxConsumption - MinConsumption) * CurveValue;
				
				Scope.Override(FMath::Max(0.1f, CurrentUsage * Self->GetCurrentProductionBoost()));
			}
			else Scope.Override(Self->GetDefaultProducingPowerConsumption() * Self->GetCurrentPotential());
		});


		SUBSCRIBE_METHOD(AFGBuildableManufacturerVariablePower::GetMinPowerConsumption, [](auto& scope, const AFGBuildableManufacturerVariablePower* Self)
		{
			auto Recipe = Self->GetCurrentRecipe().GetDefaultObject();
			if(!Recipe) return;

			auto OutputValue = Recipe->GetPowerConsumptionConstant() * Self->GetCurrentPotential();
			scope.Override(OutputValue);
		});

		SUBSCRIBE_METHOD(AFGBuildableManufacturerVariablePower::GetMaxPowerConsumption, [](auto& scope, const AFGBuildableManufacturerVariablePower* Self)
		{
			auto Recipe = Self->GetCurrentRecipe().GetDefaultObject();
			if(!Recipe) return;

			auto OutputValue = (Recipe->GetPowerConsumptionConstant() + Recipe->GetPowerConsumptionFactor()) * Self->GetCurrentPotential();
			scope.Override(OutputValue);
		});
	}
}

IMPLEMENT_GAME_MODULE(FLinearOverclockModule, LinearOverclock);