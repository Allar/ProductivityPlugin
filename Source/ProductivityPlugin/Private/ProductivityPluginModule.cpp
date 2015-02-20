#include "ProductivityPluginModulePCH.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "ProductivityPluginStyle.h"
#include "ProductivityPluginCommands.h"

#include "ILayers.h"
#include "LevelEditor.h"
#include "ScopedTransaction.h"
#endif

#include "ProductivityTypes.h"

static const FName ProductivityPluginTabName("ProductivityPlugin");

#define LOCTEXT_NAMESPACE "ProductivityPlugin"

/** Tick Object **/
FProductivityTickObject::FProductivityTickObject(FProductivityPluginModule *_Owner)
	: Owner(_Owner)
{
}

void FProductivityTickObject::Tick(float DeltaTime)
{
	check(Owner != NULL);
	Owner->Tick(DeltaTime);
}

TStatId FProductivityTickObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FProductivityPluginModule, STATGROUP_Tickables);
}

/** Module **/
FProductivityPluginModule::FProductivityPluginModule()
#if WITH_EDITOR
	: TickObject(nullptr),
	Listener(nullptr)
#endif
{
	
}

void FProductivityPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	
	FProductivityPluginStyle::Initialize();
	//FProductivityPluginStyle::ReloadTextures();

#if WITH_EDITOR

	FProductivityPluginCommands::Register();
	FProductivityPluginCommands::BindGlobalStaticToInstancedActions();



	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "ProductivityPlugin",
			LOCTEXT("ProductivitySettingsName", "Productivity Plugin"),
			LOCTEXT("ProductivitySettingsDescription", "Configure the Productivity Plugin."),
			GetMutableDefault<UProductivitySettings>()
			);

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FProductivityPluginModule::HandleSettingsSaved);
		}
	}
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FProductivityPluginCommands::Get().StaticToInstanced,
		FExecuteAction::CreateRaw(this, &FProductivityPluginModule::StaticToInstancedClicked),
		FCanExecuteAction());
		
	FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
	if (LevelEditorModule != nullptr)
	{
		{
			TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
			MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FProductivityPluginModule::AddMenuExtension));

			LevelEditorModule->GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}

		{
			TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			ToolbarExtender->AddToolBarExtension("Game", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FProductivityPluginModule::AddToolbarExtension));

			LevelEditorModule->GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		}

		if (SupportsProductivityServer())
		{
			Listener = new FTcpListener(PRODUCTIVITY_SERVER_DEFAULT_EDITOR_ENDPOINT);
			Listener->OnConnectionAccepted().BindRaw(this, &FProductivityPluginModule::HandleListenerConnectionAccepted);

			TickObject = new FProductivityTickObject(this);
		}
	}

#endif

}

void FProductivityPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	FProductivityPluginStyle::Shutdown();

#if WITH_EDITOR

	FProductivityPluginCommands::Unregister();

	// unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "ProductivityPlugin");
	}

	if (Listener)
	{
		Listener->Stop();
		delete Listener;
		Listener = NULL;
	}

	if (!PendingClients.IsEmpty())
	{
		FSocket *Client = NULL;
		while (PendingClients.Dequeue(Client))
		{
			Client->Close();
		}
	}
	for (TArray<class FSocket*>::TIterator ClientIt(Clients); ClientIt; ++ClientIt)
	{
		(*ClientIt)->Close();
	}

	delete TickObject;
	TickObject = NULL;
#endif
}
#if WITH_EDITOR
void FProductivityPluginModule::StaticToInstancedClicked()
{

	const FScopedTransaction Transaction(LOCTEXT("StaticToInstanced", "Convert Statics to Instances and back"));

	{

		/* Set up selected info */
		TArray<AStaticMeshActor*> SelectedSMAs;
		USelection* SMASelection = GEditor->GetSelectedSet(AStaticMeshActor::StaticClass());
		SMASelection->GetSelectedObjects<AStaticMeshActor>(SelectedSMAs);

		TArray<AInstancedMeshWrapper*> SelectedIMWs;
		USelection* IMWSelection = GEditor->GetSelectedSet(AInstancedMeshWrapper::StaticClass());
		IMWSelection->GetSelectedObjects<AInstancedMeshWrapper>(SelectedIMWs);

		SMASelection->Modify();
		IMWSelection->Modify();

		GEditor->GetSelectedActors()->DeselectAll();
		GEditor->GetSelectedObjects()->DeselectAll();
		GEditor->SelectNone(true, true, false);
		GEditor->NoteSelectionChange();

		/* Static Mesh to Instanced */
		TArray<FMeshInfo> MeshInfos;
		TArray< TArray<FTransform> > Transforms;

		for (AStaticMeshActor* MeshActor : SelectedSMAs)
		{
			FMeshInfo info;
			info.StaticMesh = MeshActor->GetStaticMeshComponent()->StaticMesh;
			MeshActor->GetStaticMeshComponent()->GetUsedMaterials(info.Materials);

			int32 idx = 0;

			if (MeshInfos.Find(info, idx))
			{
				Transforms[idx].Add(MeshActor->GetTransform());
			}
			else
			{
				TArray<FTransform> newTransformArray;
				newTransformArray.Add(MeshActor->GetTransform());
				MeshInfos.Add(info);
				Transforms.Add(newTransformArray);
			}
		}

		for (int i = 0; i < SelectedSMAs.Num(); ++i)
		{
			SelectedSMAs[i]->GetLevel()->Modify();
			GEditor->Layers->DisassociateActorFromLayers(SelectedSMAs[i]);
			SelectedSMAs[i]->GetWorld()->EditorDestroyActor(SelectedSMAs[i], false);
		}

		SelectedSMAs.Empty();

		for (int i = 0; i < MeshInfos.Num(); ++i)
		{
			AInstancedMeshWrapper* Wrapper = Cast<AInstancedMeshWrapper>(GEditor->AddActor(GEditor->LevelViewportClients[0]->GetWorld()->GetLevel(0), AInstancedMeshWrapper::StaticClass(), FTransform::Identity));
			if (Wrapper)
			{
				Wrapper->Modify();
				Wrapper->InstancedMeshes->SetStaticMesh(MeshInfos[i].StaticMesh);
				for (int j = 0; j < MeshInfos[i].Materials.Num(); ++j)
				{
					Wrapper->InstancedMeshes->SetMaterial(j, MeshInfos[i].Materials[j]);
				}

				for (FTransform aTransform : Transforms[i])
				{
					Wrapper->InstancedMeshes->AddInstanceWorldSpace(aTransform);
				}
			}
		}

		/* Instanced To Static Mesh */

		for (AInstancedMeshWrapper* IMW : SelectedIMWs)
		{
			int32 InstanceCount = IMW->InstancedMeshes->GetInstanceCount();
			UStaticMesh* IMWMesh = IMW->InstancedMeshes->StaticMesh;
			UE_LOG(LogProductivityPlugin, Verbose, TEXT("IMW Mesh: %s"), *IMWMesh->GetFullName());
			
			bool bGroupResultingMeshes = FProductivityPluginCommandCallbacks::OnToggleStaticToInstancedResultGroupedEnabled();
			
			TArray<AStaticMeshActor*> ActorsToGroup;

			for (int i = 0; i < InstanceCount; ++i)
			{
				FTransform InstanceTransform;
				IMW->InstancedMeshes->GetInstanceTransform(i, InstanceTransform, true);

				AStaticMeshActor* SMA = Cast<AStaticMeshActor>(GEditor->AddActor(GEditor->LevelViewportClients[0]->GetWorld()->GetLevel(0), AStaticMeshActor::StaticClass(), InstanceTransform));
				SMA->Modify();
				//@TODO: Figure out why editor is skipping names
				SMA->SetActorLabel(*IMWMesh->GetName());
				SMA->SetMobility(EComponentMobility::Movable);
				SMA->GetStaticMeshComponent()->SetStaticMesh(IMWMesh);
				SMA->SetMobility(EComponentMobility::Static);

				TArray<UMaterialInterface*> Materials;
				IMW->InstancedMeshes->GetUsedMaterials(Materials);

				for (int j = 0; j < Materials.Num(); ++j)
				{
					SMA->GetStaticMeshComponent()->SetMaterial(j, Materials[j]);
				}

				ActorsToGroup.Add(SMA);
			}

			if (bGroupResultingMeshes)
			{
				if (ActorsToGroup.Num() > 1)
				{

					// Store off the current level and make the level that contain the actors to group as the current level
					UWorld* World = ActorsToGroup[0]->GetWorld();
					check(World);
					{
						FActorSpawnParameters SpawnInfo;
						SpawnInfo.OverrideLevel = GEditor->LevelViewportClients[0]->GetWorld()->GetLevel(0);
						AGroupActor* SpawnedGroupActor = World->SpawnActor<AGroupActor>(SpawnInfo);

						for (int32 ActorIndex = 0; ActorIndex < ActorsToGroup.Num(); ++ActorIndex)
						{
							SpawnedGroupActor->Add(*ActorsToGroup[ActorIndex]);
						}

						SpawnedGroupActor->CenterGroupLocation();
						SpawnedGroupActor->bLocked = true;
					}
				}
			}

			IMW->Modify();
			IMW->GetLevel()->Modify();
			GEditor->Layers->DisassociateActorFromLayers(IMW);
			IMW->GetWorld()->EditorDestroyActor(IMW, false);
		}

		SelectedIMWs.Empty();

		// Remove all references to destroyed actors once at the end, instead of once for each Actor destroyed..
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
}
#endif

#if WITH_EDITOR

void FProductivityPluginModule::AddMenuExtension(FMenuBuilder& builder)
{
	{
		
		//builder.AddMenuEntry(
		//	FProductivityPluginCommands::Get().StaticToInstanced,
		//	NAME_None,
		//	FProductivityPluginCommands::Get().StaticToInstanced->GetLabel(),
		//	FProductivityPluginCommands::Get().StaticToInstanced->GetDescription(),
		//	FProductivityPluginCommands::Get().StaticToInstanced->GetIcon(),
		//	NAME_None);
	}
}



void FProductivityPluginModule::AddToolbarExtension(FToolBarBuilder &builder)
{
	builder.BeginSection("Productivity");

	builder.AddToolBarButton(
		FProductivityPluginCommands::Get().StaticToInstanced,
		NAME_None,
		FProductivityPluginCommands::Get().StaticToInstanced->GetLabel(),
		FProductivityPluginCommands::Get().StaticToInstanced->GetDescription(),
		FProductivityPluginCommands::Get().StaticToInstanced->GetIcon(),
		NAME_None);

	FUIAction StaticToInstancedOptionsMenuAction;

	builder.AddComboButton(
		StaticToInstancedOptionsMenuAction,
		FOnGetContent::CreateStatic(&FProductivityPluginCommands::GenerateStaticToInstancedMenuContent, FProductivityPluginCommands::GlobalStaticToInstancedActions.ToSharedRef()),
		LOCTEXT("StaticToInstancedOptions_Label", "Static<>Instanced Options"),
		LOCTEXT("StaticToInstancedOptions_Tooltip", "Options for converting static meshes to instanced meshes and vice versa."),
		FProductivityPluginCommands::Get().StaticToInstanced->GetIcon(),
		true
		);

	builder.EndSection();
}

#endif

bool FProductivityPluginModule::SupportsProductivityServer() const
{
	// disallow in Shipping and Test configurations
	if ((FApp::GetBuildConfiguration() == EBuildConfigurations::Shipping) || (FApp::GetBuildConfiguration() == EBuildConfigurations::Test))
	{
		return false;
	}

	// disallow for commandlets
	if (IsRunningCommandlet())
	{
		return false;
	}

	if (GEngine->IsEditor())
	{
		return true;
	}

	return false;
}

bool FProductivityPluginModule::HandleSettingsSaved()
{
	return true;
}

#if WITH_EDITOR
bool FProductivityPluginModule::HandleListenerConnectionAccepted(class FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	PendingClients.Enqueue(ClientSocket);
	return true;
}
#endif

void FProductivityPluginModule::Tick(float DeltaTime)
{
#if WITH_EDITOR
	if (!PendingClients.IsEmpty())
	{
		FSocket *Client = NULL;
		while (PendingClients.Dequeue(Client))
		{
			Clients.Add(Client);
		}
	}

	// remove closed connections
	for (int32 ClientIndex = Clients.Num() - 1; ClientIndex >= 0; --ClientIndex)
	{
		if (Clients[ClientIndex]->GetConnectionState() != SCS_Connected)
		{
			Clients.RemoveAtSwap(ClientIndex);
		}
	}

	//poll for data
	for (TArray<class FSocket*>::TIterator ClientIt(Clients); ClientIt; ++ClientIt)
	{
		FSocket *Client = *ClientIt;
		uint32 DataSize = 0;
		while (Client->HasPendingData(DataSize))
		{
			FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
			Datagram->Init(FMath::Min(DataSize, 1024u));
			int32 BytesRead = 0;
			if (Client->Recv(Datagram->GetData(), Datagram->Num(), BytesRead))
			{
				FProductivityNetworkMessage Message;
				*Datagram << Message;

				ProcessMessage(Message);
				uint8 ack = 1;
				int32 sent;
				Client->Send(&ack, 1, sent);
			}
		}
	}
#endif
}

#if WITH_EDITOR

void FProductivityPluginModule::ProcessMessage(const FProductivityNetworkMessage& Message)
{
	ProcessAddStaticMesh(Message);
}

void FProductivityPluginModule::ProcessAddStaticMesh(const FProductivityNetworkMessage& Message)
{
	UE_LOG(LogProductivityPlugin, Verbose, TEXT("Recieved Producitivty Message: Static mesh %s"), *Message.Payload.OriginalSceneName);

	const UProductivitySettings* ProductivitySettings = GetDefault<UProductivitySettings>();
	for (FBatchPlaceMeshInfo BatchMeshInfo : ProductivitySettings->BatchPlaceSettings)
	{
		if (Message.Payload.OriginalSceneName.Contains(BatchMeshInfo.ImportNameSubstring) && BatchMeshInfo.MeshInfo.StaticMesh != nullptr)
		{
			const FScopedTransaction Transaction(LOCTEXT("BatchPlaceAddMesh", "Added mesh from Batch Placer"));

			FVector Location = FVector(FCString::Atof(*Message.Payload.LocationX), FCString::Atof(*Message.Payload.LocationY), FCString::Atof(*Message.Payload.LocationZ));
			FVector Scale = FVector(FCString::Atof(*Message.Payload.ScaleX), FCString::Atof(*Message.Payload.ScaleY), FCString::Atof(*Message.Payload.ScaleZ));
			FRotator Rotation = FRotator::MakeFromEuler(FVector(FCString::Atof(*Message.Payload.RotationX), FCString::Atof(*Message.Payload.RotationY), FCString::Atof(*Message.Payload.RotationZ)));
			FTransform Transform = FTransform(Rotation, Location, Scale);

			AStaticMeshActor* SMA = Cast<AStaticMeshActor>(GEditor->AddActor(GEditor->LevelViewportClients[0]->GetWorld()->GetLevel(0), AStaticMeshActor::StaticClass(), Transform));
			SMA->Modify();
			//@TODO: Figure out why editor is skipping names
			SMA->SetActorLabel(BatchMeshInfo.ImportNameSubstring);
			SMA->SetMobility(EComponentMobility::Movable);
			SMA->GetStaticMeshComponent()->SetStaticMesh(BatchMeshInfo.MeshInfo.StaticMesh);
			SMA->SetMobility(EComponentMobility::Static);

			for (int i = 0; i < BatchMeshInfo.MeshInfo.Materials.Num(); ++i)
			{
				SMA->GetStaticMeshComponent()->SetMaterial(i, BatchMeshInfo.MeshInfo.Materials[i]);
			}
			break;
		}
	}
}
#endif

DEFINE_LOG_CATEGORY(LogProductivityPlugin)

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProductivityPluginModule, ProductivityPlugin)
