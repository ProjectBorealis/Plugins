// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkConnection.h
#pragma once
#include "CoreMinimal.h"
#include "Materials/Material.h"

class FJsonObject;
class FSubstanceLiveLinkWebSocket;
class UTexture2D;

struct FExpressionInput;

/**
 * Maintains connection between UE4 and Substance Painter
 */
class FSubstanceLiveLinkConnection : public TSharedFromThis<FSubstanceLiveLinkConnection>
{
public:
	/**
	 * Constructor
	 */
	FSubstanceLiveLinkConnection();

	/**
	 * Destructor
	 */
	~FSubstanceLiveLinkConnection();

	/**
	 * Send a Mesh Object to Painter
	 * @param Mesh - The mesh object to pass to Painter
	 */
	void SendAssetToSubstancePainter(UObject* Mesh);

	/**
	 * Query if the connection is active
	 * @returns true if the Connection is active
	 */
	bool IsConnected() const;

	/**
	 * Delegate called when a web socket connection has been established successfully.
	 * @returns Reference to ConnectionErrorEvent for Binding
	 */
	DECLARE_EVENT(FSubstanceLiveLinkConnection, FSubstanceLiveLinkConnectionErrorEvent);
	FSubstanceLiveLinkConnectionErrorEvent& OnConnectionError();

private:
	/**
	 * Typedef for MessageReceivedHandler
	 */
	typedef TFunction<void (TSharedRef<FJsonObject>)> MessageReceivedFunction;

	/**
	 * Data structure used to map painter source textures to ue4 material expressions
	 */
	struct MaterialSamplerMapping
	{
		/**
		 * Functor to resolve the FMaterialInput* from the Material
		 */
		TFunction<FExpressionInput* (UMaterial*)> ResolveMaterialInput;

		/**
		 * Channels that should be attached to this expression as a mask
		 */
		uint32 ChannelMask;
	};

	/**
	 * Data structure used to indicate how map information from painter should be applied to Unreal materials
	 */
	struct PainterToUnrealMapInfo
	{
		/**
		 * General Name used for socket communication between Unreal and Substance Painter
		 */
		FString Name;

		/**
		 * Painter Export Configuration Name
		 */
		FString PainterExportId;

		/**
		 * Array of Material Sampler Mappings used to build expression
		 */
		TArray<MaterialSamplerMapping> MaterialSamplerMapArray;
	};

private:
	/**
	 * Adds a message handler that can be called by OnMessageReceived
	 * @param Message the message to handle by this functor
	 * @param Functor the functor to invoke when the Message is received
	 */
	void AddMessageReceivedHandler(const FString& Message, MessageReceivedFunction Function);

	/**
	 * Binds a given texture to a material sampler expression
	 * @param MapName the map name used to communicate between Painter and UE4
	 * @param Material the material that holds the expression
	 * @param Texture the texture to bind to the expression
	 * @param Input the expression input on the material to be bound from
	 * @param Mask bitmask used to flag which channels get attached to the Input object
	 * @returns true if the MaterialInput was modified
	 */
	bool ConnectTextureToSampler(const FString& MapName, UMaterial* Material, UTexture2D* Texture, FExpressionInput* Input, uint32 Mask) const;

	/**
	 * Initiate connection to Substance Painter
	 */
	void ConnectToSubstancePainter();

	/**
	 * Close underlying connection
	 */
	void Close();

	/**
	 * Runs every tick to poll the underlying socket connection
	 * @param DeltaTime the amount of time in seconds since the last tick
	 * @returns true
	 */
	bool GameThreadTick(float DeltaTime);

	/**
	 * Generate a JSON Object representing the painter data for project open/create
	 * @param Mesh the Mesh Object to generate a painter project from
	 * @param SubstancePainterProjectPath the Painter Project Path to reference
	 * @returns a JSON Object that can be sent to Painter over the network
	 */
	TSharedPtr<FJsonObject> GeneratePainterProjectJson(UObject* Mesh, const FString& SubstancePainterProjectPath) const;

	/**
	 * Generate a Path URI for a given Path on Disk
	 * @param Path the pathname to convert
	 * @returns the generated URI
	 */
	FString GeneratePathURI(const FString& Path) const;

	/**
	 * Serialize Json Object to String
	 * @param Json The Json Object to Serialize
	 * @returns The serialized string representation
	 */
	FString GetJsonAsString(TSharedRef<FJsonObject> Json) const;

	/**
	 * Get Mesh Materials in an array
	 * @param Mesh incoming Mesh object
	 * @param bForceReplace force replace of all existing materials on the mesh
	 * @returns Array of UMaterials
	 */
	TArray<UMaterial*> GetMeshMaterials(UObject* Mesh, bool bForceReplace) const;

	/**
	 * Get source path filenames for a given UObject
	 * @param Obj the object to resolve source pathnames
	 * @param OutFilename the filename used to import this source asset
	 * @returns true if source filenames were found
	 */
	bool GetSourceFile(UObject* Obj, FString& OutFilename) const;

	/**
	 * Get Substance Painter Project Path for Mesh
	 * @param Mesh the mesh object to use to generate the path name
	 * @returns The SPP Path
	 */
	FString GetSubstancePainterProjectPath(UObject* Mesh) const;

	/**
	 * Handle messages received from Substance Painter
	 * @param Message the message transmitted from Painter
	 */
	void OnMessageReceived(const ANSICHAR* Message);

	/**
	 * Register Message Handlers for use by underlying socket connection
	 */
	void RegisterMessageHandlers();

	/**
	 * Register Painter to Unreal Map Information
	 */
	void RegisterPainterToUnrealMapInfo();

	/**
	 * Queue a functor to be executed by the game thread
	 * @param The functor to execute the next time the game thread is ticked
	 */
	void QueueGameThreadMessage(TFunction<void ()> Message);

	/**
	 * Sanitize Material Names Similar to Painter
	 * @param MaterialName The Material Name to be Sanitized
	 * @returns Sanitized Material Name
	 */
	FString SanitizeMaterialName(const FString& MaterialName) const;

	/**
	 * Send command to painter with the painter project information
	 * @param Mesh the Mesh to send to Painter
	 * @param Command the Command to issue to Painter
	 * @param SubstancePainterProjectPath the path we will create the .spp file
	 */
	void SendPainterProject(UObject* Mesh, const TCHAR* Command, const FString& SubstancePainterProjectPath) const;

	/**
	 * Update Material Sampler from Painter
	 * @param Material the Material to modify
	 * @param MapName the reference Substance Painter is using to control where the source image should be applied
	 * @param SourceImagePath the qualified path on disk where the source image is located (relative to ProjectDir)
	 * @returns true if the material was changed
	 */
	bool UpdateMaterialSamplers(UMaterial* Material, const FString& MapName, const FString& SourceImagePath) const;

	/*
	 * Replaces a material that is currently on a mesh with a new one
	 * @param Mesh the mesh that the current material belongs to
	 * @param Material a pointer to the specific material to recreate
	 * @param MaterialSlotName the name set to the material slot on the mesh, used if there is no current material name to use
	 * @returns a pointer to the new material
	*/
	UMaterial* ReplaceMaterial(const UObject* Mesh, const UMaterial* Material, const FString& MaterialSlotName) const;

private:
	/** Internal Web Socket Object */
	TUniquePtr<FSubstanceLiveLinkWebSocket>		WebSocket;

	/** Connection Error Callback */
	FSubstanceLiveLinkConnectionErrorEvent		ConnectionErrorEvent;

	/** Tick Handle for periodic updates */
	FDelegateHandle								TickHandle;

	/** Map to keep track of message handlers */
	TMap<FString, MessageReceivedFunction>		MessageReceivedFunctionMap;

	/** Array of the mapping between Painter export files and Unreal materials */
	TArray<PainterToUnrealMapInfo>				PainterToUnrealMapInfoArray;

	/** Critical Section to restrict control for Game Thread messages */
	FCriticalSection							GameThreadMessagesCS;

	/** Stores a list of messages to be executed on the Main Thread */
	TArray<TFunction<void ()>>					GameThreadMessages;

	/** Stores a list of changed materials that we should update */
	TSet<UMaterial*>							ChangedMaterials;

	/** If true, we should force editor viewport refresh */
	bool										bRefreshEditorViewport;

	/** Timer for checking when a ping message should be sent to Painter*/
	float										fPingTimer;

	/** Interval for ping message to be sent to painter in seconds */
	static const float							kDisconnectCheckTimer;
};
