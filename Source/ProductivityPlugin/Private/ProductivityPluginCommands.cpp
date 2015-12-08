#include "ProductivityPluginModulePCH.h"
#include "ProductivityPluginCommands.h"
#include "ProductivityPluginEditorSettings.h"

TSharedPtr<FUICommandList> FProductivityPluginCommands::GlobalStaticToInstancedActions;

FProductivityPluginCommands::FProductivityPluginCommands() : TCommands<FProductivityPluginCommands>(TEXT("ProductivityPlugin"), NSLOCTEXT("Contexts", "ProductivityPlugin", "ProductivityPlugin Plugin"), NAME_None, FProductivityPluginStyle::GetStyleSetName())
{

}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE ""

void FProductivityPluginCommands::RegisterCommands()
{
	UI_COMMAND(StaticToInstanced, "Statics<>Instanced", "Batch converts all selected static mesh actors to instanced meshes and vice versa.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StaticToInstancedIsResultGrouped, "Group Static Meshes", "If checked, when static meshes are created from an instanced mesh wrapper, they will be grouped.", EUserInterfaceActionType::ToggleButton, FInputGesture());
}

#undef LOCTEXT_NAMESPACE

TSharedRef< SWidget > FProductivityPluginCommands::GenerateStaticToInstancedMenuContent(TSharedRef<FUICommandList> InCommandList)
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, InCommandList);

	MenuBuilder.BeginSection("StaticToInstancedOptions");
	{
		MenuBuilder.AddMenuEntry(FProductivityPluginCommands::Get().StaticToInstancedIsResultGrouped);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FProductivityPluginCommands::BindGlobalStaticToInstancedActions()
{
	check(!GlobalStaticToInstancedActions.IsValid());

	GlobalStaticToInstancedActions = MakeShareable(new FUICommandList);

	const FProductivityPluginCommands& Commands = FProductivityPluginCommands::Get();
	FUICommandList& ActionList = *GlobalStaticToInstancedActions;

	ActionList.MapAction(
		FProductivityPluginCommands::Get().StaticToInstancedIsResultGrouped,
		FExecuteAction::CreateStatic(&FProductivityPluginCommandCallbacks::OnToggleStaticToInstancedResultGrouped),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FProductivityPluginCommandCallbacks::OnToggleStaticToInstancedResultGroupedEnabled)
	);
}

#endif

void FProductivityPluginCommandCallbacks::OnToggleStaticToInstancedResultGrouped()
{
	UProductivityPluginEditorSettings* PlayInSettings = GetMutableDefault<UProductivityPluginEditorSettings>();	
	PlayInSettings->SetGroupStaticToInstancedResults(!PlayInSettings->GetGroupStaticToInstancedResults());
}

bool FProductivityPluginCommandCallbacks::OnToggleStaticToInstancedResultGroupedEnabled()
{
	UProductivityPluginEditorSettings* PlayInSettings = GetMutableDefault<UProductivityPluginEditorSettings>();
	return PlayInSettings->GetGroupStaticToInstancedResults();
}
