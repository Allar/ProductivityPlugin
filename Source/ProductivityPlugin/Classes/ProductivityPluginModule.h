// Some copyright should be here...

#pragma once

#include "ModuleManager.h"
#include "Engine.h"
#include "ProductivityTypes.h"

class FProductivityTickObject : FTickableGameObject
{
public:
	FProductivityTickObject(class FProductivityPluginModule *_Owner);

	virtual void Tick(float DeltaTime);

	virtual bool IsTickable() const
	{
		return true;
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return false;
	}
	virtual TStatId GetStatId() const override;

private:
	class FProductivityPluginModule *Owner;
};

class FProductivityPluginModule : public IModuleInterface
{
public:
	FProductivityPluginModule();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	void Tick(float DeltaTime);
#if WITH_EDITOR
	/** Tools **/
	
	/** This function will be bound to Command.*/
	void StaticToInstancedClicked();

	/** Server **/
	bool HandleListenerConnectionAccepted(class FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
#endif
	
protected:

#if WITH_EDITOR
	void AddToolbarExtension(class FToolBarBuilder &);
	void AddMenuExtension(class FMenuBuilder &);
	void ProcessMessage(const FProductivityNetworkMessage& Message);
	void ProcessAddStaticMesh(const FProductivityNetworkMessage& Message);
#endif

	// Callback for when the settings were saved.
	bool HandleSettingsSaved();

	/**
	* Checks whether the Productivity Server is supported.
	* @return true if supported, false otherwise.
	*/
	bool SupportsProductivityServer() const;


private:
#if WITH_EDITOR
	TSharedPtr<class FUICommandList> PluginCommands;
	
	FProductivityTickObject* TickObject;
	class FTcpListener *Listener;
	TQueue<class FSocket*, EQueueMode::Mpsc> PendingClients;
	TArray<class FSocket*> Clients;
#endif
};