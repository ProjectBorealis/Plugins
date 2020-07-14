// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkConnection.cpp
#include "SubstanceLiveLinkConnection.h"
#include "SubstanceLiveLinkPrivatePCH.h"
#include "SubstanceLiveLinkModule.h"
#include "SubstanceLiveLinkAsyncTaskFunctor.h"
#include "SubstanceLiveLinkTextureLoader.h"
#include "SubstanceLiveLinkWebSocket.h"
#include "Async/AsyncWork.h"
#include "EditorReimportHandler.h"
#include "EditorSupportDelegates.h"
#include "Editor/EditorEngine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Editor.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Factories/MaterialFactoryNew.h"
#include "Misc/SecureHash.h"
#include "UObject/NoExportTypes.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/MessageDialog.h"
#include "Containers/Ticker.h"

const float FSubstanceLiveLinkConnection::kDisconnectCheckTimer = 10.0f;

/**
 */
FSubstanceLiveLinkConnection::FSubstanceLiveLinkConnection()
	: bRefreshEditorViewport(false)
{
	// Setup our game thread tick
	FTicker& Ticker = FTicker::GetCoreTicker();
	FTickerDelegate TickDelegate = FTickerDelegate::CreateRaw(this, &FSubstanceLiveLinkConnection::GameThreadTick);
	TickHandle = Ticker.AddTicker(TickDelegate, 0.0f);

	// Register Handlers
	RegisterPainterToUnrealMapInfo();
	RegisterMessageHandlers();

	// Initiate Connection
	ConnectToSubstancePainter();

	fPingTimer = 0;
}

FSubstanceLiveLinkConnection::~FSubstanceLiveLinkConnection()
{
	FTicker::GetCoreTicker().RemoveTicker(TickHandle);

	Close();
}

void FSubstanceLiveLinkConnection::SendAssetToSubstancePainter(UObject* Mesh)
{
	check(Mesh);

	if (!IsConnected())
	{
		ConnectToSubstancePainter();
	}

	if (WebSocket.IsValid())
	{
		FString SubstancePainterProjectPath = GetSubstancePainterProjectPath(Mesh);
		bool bFileExists = FPaths::FileExists(SubstancePainterProjectPath);

		//determine if painter project exists to either open or create a spp project
		const TCHAR* Command = bFileExists ? TEXT("OPEN_PROJECT") : TEXT("CREATE_PROJECT");

		if (bFileExists)
		{
			UE_LOG(LogSubstanceLiveLink, Log, TEXT("Opening Painter Project at \"%s\""), *SubstancePainterProjectPath);
		}
		else
		{
			UE_LOG(LogSubstanceLiveLink, Log, TEXT("Creating New Painter Project at \"%s\""), *SubstancePainterProjectPath);
		}

		SendPainterProject(Mesh, Command, SubstancePainterProjectPath);
	}
	else
	{
		ConnectionErrorEvent.Broadcast();
	}
}

void FSubstanceLiveLinkConnection::Close()
{
	if (WebSocket.IsValid())
	{
		WebSocket.Reset();
	}
}

bool FSubstanceLiveLinkConnection::IsConnected() const
{
	if (WebSocket.IsValid())
	{
		switch (WebSocket->GetConnectionStatus())
		{
		case ESubstanceLiveLinkWebSocketConnectionStatus::Connected:
		case ESubstanceLiveLinkWebSocketConnectionStatus::ConnectionPending:
			return true;
		default:
			return false;
		}
	}
	else
	{
		return false;
	}
}

FSubstanceLiveLinkConnection::FSubstanceLiveLinkConnectionErrorEvent& FSubstanceLiveLinkConnection::OnConnectionError()
{
	return ConnectionErrorEvent;
}

void FSubstanceLiveLinkConnection::ConnectToSubstancePainter()
{
	Close();

	WebSocket = MakeUnique<FSubstanceLiveLinkWebSocket>();
	WebSocket->OnMessageReceived().AddRaw(this, &FSubstanceLiveLinkConnection::OnMessageReceived);
	WebSocket->Connect(ANSI_TO_TCHAR("localhost"), 6404);
	WebSocket->SendMessage("SEND_PROJECT_INFO {}");
}

bool FSubstanceLiveLinkConnection::GameThreadTick(float DeltaTime)
{
	if (WebSocket.IsValid())
	{
		fPingTimer += DeltaTime;
		if (fPingTimer >= kDisconnectCheckTimer)
		{
			//Send a ping message using the websocket to force check if it is still valid
			//Needed to invalidate the websocket if painter has crashed or closed without a close message
			WebSocket->Ping();
			fPingTimer = 0.0f;
		}

		WebSocket->Tick();

		switch (WebSocket->GetConnectionStatus())
		{
		case ESubstanceLiveLinkWebSocketConnectionStatus::Disconnected:
		case ESubstanceLiveLinkWebSocketConnectionStatus::DisconnectPending:
			{
				Close();
			}
			break;
		}
	}

	//invoke all messages that need to operate on the game thread
	{
		FScopeLock GameThreadMessagesLock(&GameThreadMessagesCS);

		for (const auto& Message : GameThreadMessages)
		{
			Message();
		}

		GameThreadMessages.Empty();
	}

	//update all changed materials
	for (const auto& Material : ChangedMaterials)
	{
		Material->PreEditChange(nullptr);
		Material->PostEditChange();
	}

	ChangedMaterials.Empty();

	if (bRefreshEditorViewport)
	{
		bRefreshEditorViewport = false;

		FEditorSupportDelegates::RedrawAllViewports.Broadcast();

		//force a redraw of the viewport here as well (in case the editor isn't active)
		if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
		{
			EditorEngine->UpdateSingleViewportClient((FEditorViewportClient*)GCurrentLevelEditingViewportClient, false, false);
		}
	}

	return true;
}

TSharedPtr<FJsonObject> FSubstanceLiveLinkConnection::GeneratePainterProjectJson(UObject* Mesh, const FString& SubstancePainterProjectPath) const
{
	FString WorkspacePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString MeshPath;

	//Create a string to represent the location of the fbx to send to painter
	FString MeshName = Mesh->GetName();
	FString TextureLabel = FString("SP_Textures");
	FString ExportPath = FPaths::Combine(*TextureLabel, MeshName);

	bool bSourceExists = FPaths::FileExists(MeshPath);
	bool bFbxCreated = false;

	FString MeshURI;

	FString FileOutputPath = FPaths::Combine(WorkspacePath, ExportPath);

	TArray<UObject*> objArray;
	objArray.Add(Mesh);

	//check if object file exists in the project content, if not export the uobject to a .fbx file
	if (!bSourceExists)
	{
		//Create a full output path
		FString ExportedPath = FileOutputPath;

		FString MeshContentName;
		Mesh->GetOuter()->GetName(MeshContentName);

		MeshContentName.RemoveFromStart(FString("/"));
		ExportedPath.Append("/");
		ExportedPath.Append(MeshContentName);
		ExportedPath.Append(".fbx");

		if (!FPaths::FileExists(ExportedPath))
		{
			FText tex = FText::FromString(FString("No valid FBX file was found for this object, one must be exported. Please disable the \"Collision\" static mesh option in the export dialog that will open following this dialog."));
			//Export currently pops up a dialog to make sure the user has the correct settings
			if (FMessageDialog::Open(EAppMsgType::OkCancel, tex) == EAppReturnType::Ok)
			{
				FAssetToolsModule::GetModule().Get().ExportAssets(objArray, FileOutputPath);
				bFbxCreated = true;
			}
			else
			{
				return nullptr;
			}
		}

		MeshURI = GeneratePathURI(ExportedPath);
	}
	else
	{
		MeshURI = GeneratePathURI(MeshPath);
	}

	//create JSON references for material link
	TSharedRef<FJsonObject> MaterialsLink = MakeShared<FJsonObject>();
	TArray<UMaterial*> Materials = GetMeshMaterials(Mesh, bFbxCreated);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(objArray);

	for (const auto& Material : Materials)
	{
		TSharedRef<FJsonObject> MaterialLink = MakeShared<FJsonObject>();
		FSoftObjectPath MaterialSoftPath(Material);
		FString MaterialPath = MaterialSoftPath.GetAssetPathString();
		FString MaterialName = MaterialSoftPath.GetAssetName();

		MaterialLink->SetStringField(TEXT("assetPath"), MaterialPath);
		MaterialLink->SetStringField(TEXT("exportPreset"), TEXT("Unreal Engine 4 (Packed)"));
		MaterialLink->SetStringField(TEXT("resourceShader"), TEXT("pbr-metal-rough"));

		//Property Mapping to UE4 Material Inputs
		TSharedRef<FJsonObject> PropertiesAssociation = MakeShared<FJsonObject>();

		for (const auto& PainterToUnrealMapInfo : PainterToUnrealMapInfoArray)
		{
			PropertiesAssociation->SetStringField(PainterToUnrealMapInfo.PainterExportId, PainterToUnrealMapInfo.Name);
		}

		MaterialLink->SetObjectField(TEXT("spToLiveLinkProperties"), PropertiesAssociation);

		//add to materials list
		MaterialsLink->SetObjectField(SanitizeMaterialName(MaterialName), MaterialLink);
	}

	//Project Information
	TSharedRef<FJsonObject> ProjectInfo = MakeShared<FJsonObject>();
	ProjectInfo->SetStringField(TEXT("meshUrl"), MeshURI);
	ProjectInfo->SetStringField(TEXT("normal"), TEXT("OpenGL"));
	ProjectInfo->SetStringField(TEXT("template"), TEXT(""));
	ProjectInfo->SetStringField(TEXT("url"), GeneratePathURI(SubstancePainterProjectPath));

	//Container
	TSharedRef<FJsonObject> Container = MakeShared<FJsonObject>();
	Container->SetStringField(TEXT("applicationName"), TEXT("Unreal4"));
	Container->SetStringField(TEXT("exportPath"), ExportPath);
	Container->SetStringField(TEXT("workspacePath"), WorkspacePath);
	Container->SetObjectField(TEXT("materials"), MaterialsLink);
	Container->SetObjectField(TEXT("project"), ProjectInfo);
	Container->SetStringField(TEXT("linkIdentifier"), FSoftObjectPath(Mesh).GetAssetPathString());

	//GetMeshMaterials above will modify the mesh if bFbxCreated is true, so go ahead and apply changes here
	if (bFbxCreated)
	{
		Mesh->MarkPackageDirty();
	}

	return Container;
}

FString FSubstanceLiveLinkConnection::GeneratePathURI(const FString& Path) const
{
	FString ResultPath = Path;

	FPaths::NormalizeFilename(ResultPath);

	//NOTE: This code was largely lifted from GenericPlatformHttp::UrlEncode, however that function will encode path separators, which is bad!
	FTCHARToUTF8 Converter(*ResultPath);
	const UTF8CHAR* UTF8Data = (UTF8CHAR*)Converter.Get();
	FString EncodedString;

	TCHAR Buffer[2] = { 0, 0 };

	for (int32 ByteIdx = 0, Length = Converter.Length(); ByteIdx < Length; ++ByteIdx)
	{
		UTF8CHAR ByteToEncode = UTF8Data[ByteIdx];

		bool bIsAllowedChar = (ByteToEncode == '~');
		bIsAllowedChar |= (ByteToEncode == '_');
		bIsAllowedChar |= (ByteToEncode >= 45 && ByteToEncode <= 58);	//_./0123456789;
		bIsAllowedChar |= (ByteToEncode >= 65 && ByteToEncode <= 90);	//ABCDEFGHIJKLMNOPQRSTUVWXYZ
		bIsAllowedChar |= (ByteToEncode >= 97 && ByteToEncode <= 122);	//abcdefghijklmnopqrstuvwxyz

		if (bIsAllowedChar)
		{
			Buffer[0] = ByteToEncode;
			FString TmpString = Buffer;
			EncodedString += TmpString;
		}
		else if (ByteToEncode != '\0')
		{
			EncodedString += TEXT("%");
			EncodedString += FString::Printf(TEXT("%.2X"), ByteToEncode);
		}
	}

	return FString(TEXT("file:///")) + EncodedString;
}

FString FSubstanceLiveLinkConnection::GetJsonAsString(TSharedRef<FJsonObject> Json) const
{
	FString JsonString;
	auto writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
	FJsonSerializer::Serialize(Json, writer);
	writer->Close();
	return JsonString;
}

TArray<UMaterial*> FSubstanceLiveLinkConnection::GetMeshMaterials(UObject* Mesh, bool bForceReplace) const
{
	TArray<UMaterial*> Materials;

	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Mesh))
	{
		for (auto& StaticMaterial : StaticMesh->StaticMaterials)
		{
			UMaterial* Material = Cast<UMaterial>(StaticMaterial.MaterialInterface);

			if (Material == nullptr || bForceReplace)
			{
				UMaterial* UnrealMaterial = ReplaceMaterial(Mesh, nullptr, StaticMaterial.MaterialInterface->GetFName().ToString());
				if (UnrealMaterial == nullptr)
					continue;

				StaticMaterial.MaterialInterface = UnrealMaterial;

				Material = UnrealMaterial;
			}
			Materials.Add(Material);
		}
	}
	else if (USkeletalMesh* SkelMesh = Cast<USkeletalMesh>(Mesh))
	{
		for (auto& SkelMaterial : SkelMesh->Materials)
		{
			UMaterial* Material = Cast<UMaterial>(SkelMaterial.MaterialInterface);

			if (Material == nullptr || bForceReplace)
			{
				UMaterial* UnrealMaterial = ReplaceMaterial(Mesh, nullptr, SkelMaterial.MaterialInterface->GetFName().ToString());
				if (UnrealMaterial == nullptr)
					continue;

				SkelMaterial.MaterialInterface = UnrealMaterial;

				Material = UnrealMaterial;
			}
			Materials.Add(Material);
		}
	}

	return Materials;
}

bool FSubstanceLiveLinkConnection::GetSourceFile(UObject* Obj, FString& OutFilename) const
{
	TArray<FString> Filenames;

	if (FReimportManager::Instance()->CanReimport(Obj, &Filenames))
	{

		if (Filenames.Num() == 1)
		{
			if (Filenames[0].IsEmpty())
				return false;
			OutFilename = MoveTemp(Filenames[0]);
			return true;
		}
	}

	return false;
}

FString FSubstanceLiveLinkConnection::GetSubstancePainterProjectPath(UObject* Mesh) const
{
	FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	ProjectPath = FPaths::Combine(ProjectPath, TEXT(".sp"));
	ProjectPath = FPaths::Combine(ProjectPath, *FPaths::SetExtension(FSoftObjectPath(Mesh).GetAssetPathString(), TEXT(".spp")));
	return ProjectPath;
}

void FSubstanceLiveLinkConnection::OnMessageReceived(const ANSICHAR* AnsiMessage)
{
	int32 Separator = 0;
	FString Message(UTF8_TO_TCHAR(AnsiMessage));

	if (Message.FindChar(' ', Separator))
	{
		FString Command = Message.Left(Separator);

		//deserialize JSON message
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Message.Mid(Separator + 1));
		TSharedPtr<FJsonObject> PayloadJson;

		if (!FJsonSerializer::Deserialize(JsonReader, PayloadJson))
		{
			UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Invalid Payload Received from Painter \"%s\""), *Message);
			return;
		}

		Command.ToUpperInline();

		if (MessageReceivedFunction* Function = MessageReceivedFunctionMap.Find(Command))
		{
			(*Function)(PayloadJson.ToSharedRef());
		}
		else
		{
			UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Invalid Message ID Received from Painter \"%s\""), *Message);
		}
	}
}

void FSubstanceLiveLinkConnection::RegisterMessageHandlers()
{
	AddMessageReceivedHandler(TEXT("SET_MATERIAL_PARAMS"), [this](TSharedRef<FJsonObject> PayloadJson)
	{
		const FString MaterialName = PayloadJson->GetStringField(TEXT("material"));
		TSharedPtr<FJsonObject> ParamsJson = PayloadJson->GetObjectField(TEXT("params"));

		if (!ParamsJson.IsValid())
		{
			UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Params object not provided for SET_MATERIAL_PARAMS"));
			return;
		}

		//Check current material for all texture nodes needed by the current painter project to see if it has already been initialized
		bool bFoundAll = true;
		for (const auto& KeyValue : ParamsJson->Values)
		{
			bool bFoundExpression = false;
			if (UMaterial* Material = Cast<UMaterial>(FSoftObjectPath(MaterialName).TryLoad()))
			{
				for (const auto& MaterialExpression : Material->Expressions)
				{
					UMaterialExpressionTextureSampleParameter2D* Expression = Cast<UMaterialExpressionTextureSampleParameter2D>(MaterialExpression);

					if (Expression == nullptr || Expression->Texture == nullptr)
					{
						continue;
					}
					else if (Expression->Texture->GetName() == FPaths::GetBaseFilename(KeyValue.Value->AsString()))
					{
						bFoundExpression = true;
						break;
					}
				}
			}
			if (!bFoundExpression)
			{
				bFoundAll = false;
			}
		}

		//If there are any missing nodes recreate the material
		if (!bFoundAll)
		{
			UMaterial* Material = Cast<UMaterial>(FSoftObjectPath(MaterialName).TryLoad());

			if (Material)
			{
				if (Material->Expressions.Num() > 0)
				{
					FString MaterialNameTemp;
					Material->GetName(MaterialNameTemp);

					FString OuterName;
					OuterName = Material->GetOuter()->GetName();

					FString Left;
					FString Right;
					//Create an unreal material asset
					UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

					OuterName.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
					FString PkgName = Left + TEXT("/") + MaterialNameTemp;
					UPackage* Pkg = CreatePackage(nullptr, *PkgName);
					Pkg->FullyLoad();
					UMaterial* NewMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(
					                             UMaterial::StaticClass(),
					                             Pkg,
					                             *MaterialNameTemp,
					                             RF_Standalone | RF_Public, nullptr, GWarn);

					NewMaterial->PreEditChange(nullptr);
					NewMaterial->PostEditChange();

					Material = NewMaterial;
				}
			}
		}

		for (const auto& KeyValue : ParamsJson->Values)
		{
			FString MapName = KeyValue.Key;
			FString SourceImagePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::ProjectDir(), *KeyValue.Value->AsString()));
			TSharedRef<FSubstanceLiveLinkConnection> Self = AsShared();	//store a ref to ourselves so we don't get deleted during the async task
			TSharedRef<FSubstanceLiveLinkTextureLoader> TextureLoader = ISubstanceLiveLinkModule::Get()->GetTextureLoader();

			//sometimes painter sends us maps that don't actually change,
			//so we should verify that here in a worker thread
			auto HashTask = new FAutoDeleteAsyncTask<FSubstanceLiveLinkAutoDeleteAsyncTask>([Self, TextureLoader, this, MaterialName, MapName, SourceImagePath, bFoundAll]()
			{
				if (!TextureLoader->HasFileChanged(SourceImagePath) && bFoundAll)
				{
					return;
				}

				//Queue main thread functor to update the material
				QueueGameThreadMessage([ = ]()
				{
					if (UMaterial* Material = Cast<UMaterial>(FSoftObjectPath(MaterialName).TryLoad()))
					{
						if (UpdateMaterialSamplers(Material, MapName, SourceImagePath))
						{
							ChangedMaterials.Emplace(Material);
						}

						bRefreshEditorViewport = true;
					}
					else
					{
						UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unable to load Material \"%s\" referenced from Substance Painter"), *MaterialName);
					}
				});
			});
			HashTask->StartBackgroundTask();
		}
	});

	AddMessageReceivedHandler(TEXT("OPENED_PROJECT_INFO"), [this](TSharedRef<FJsonObject> PayloadJson)
	{
		const FString url = PayloadJson->GetStringField(TEXT("projectUrl"));
		const FString linkIdentifier = PayloadJson->GetStringField(TEXT("linkIdentifier"));

		if (UObject* Mesh = FSoftObjectPath(linkIdentifier).TryLoad())
		{
			//verify we're loading a qualified mesh
			if (!ISubstanceLiveLinkModule::IsMeshClass(Mesh->GetClass()))
			{
				UE_LOG(LogSubstanceLiveLink, Log, TEXT("Painter sent link identifier \"%s\" which is a valid asset but not a Mesh"), *linkIdentifier);
				return;
			}

			//verify our uri's are the same as well
			FString compareURI = GeneratePathURI(GetSubstancePainterProjectPath(Mesh));

			if (compareURI != url)
			{
				UE_LOG(LogSubstanceLiveLink, Log, TEXT("Painter Project URI does not match what we expect, skipping connection"));
				return;
			}

			SendAssetToSubstancePainter(Mesh);
		}
	});
}

void FSubstanceLiveLinkConnection::AddMessageReceivedHandler(const FString& Message, MessageReceivedFunction Function)
{
	MessageReceivedFunctionMap.Add(Message, Function);
}

FString FSubstanceLiveLinkConnection::SanitizeMaterialName(const FString& MaterialName) const
{
	if (MaterialName == TEXT("No Name"))
	{
		return SanitizeMaterialName(TEXT("DefaultMaterial"));
	}

	FString ResultString = MaterialName;
	ResultString = ResultString.Replace(TEXT("\\"), TEXT("_"), ESearchCase::CaseSensitive);
	ResultString = ResultString.Replace(TEXT("/"), TEXT("_"), ESearchCase::CaseSensitive);
	ResultString = ResultString.Replace(TEXT("#"), TEXT("_"), ESearchCase::CaseSensitive);

	return ResultString;
}

void FSubstanceLiveLinkConnection::SendPainterProject(UObject* Mesh, const TCHAR* Command, const FString& SubstancePainterProjectPath) const
{
	TSharedPtr<FJsonObject> PainterJson = GeneratePainterProjectJson(Mesh, SubstancePainterProjectPath);

	if (PainterJson.IsValid())
	{
		FString FullCommand = FString::Printf(TEXT("%s %s"), Command, *GetJsonAsString(PainterJson.ToSharedRef()));
		WebSocket->SendMessage(TCHAR_TO_UTF8(*FullCommand));
	}
	else
	{
		UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unable to send project to Substance Painter"));
	}
}

bool FSubstanceLiveLinkConnection::UpdateMaterialSamplers(UMaterial* Material, const FString& MapName, const FString& SourceImagePath) const
{
	FString TextureDestinationPath = FPaths::GetPath(FSoftObjectPath(Material).GetAssetPathString());
	bool bMaterialChanged = false;

	for (const auto& MapInfo : PainterToUnrealMapInfoArray)
	{
		if (MapInfo.Name == MapName)
		{
			for (const auto& MaterialSamplerMapping : MapInfo.MaterialSamplerMapArray)
			{
				if (FExpressionInput* Expression = MaterialSamplerMapping.ResolveMaterialInput(Material))
				{
					UTexture2D* Texture = ISubstanceLiveLinkModule::Get()->GetTextureLoader()->ResolveTexture(MapName, SourceImagePath, TextureDestinationPath);

					if (Texture == nullptr)
					{
						UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unable to resolve texture \"%s\", skipping"), *SourceImagePath);
						return bMaterialChanged;
					}

					bMaterialChanged |= ConnectTextureToSampler(MapName, Material, Texture, Expression, MaterialSamplerMapping.ChannelMask);
				}
			}
		}
	}

	return bMaterialChanged;
}

bool FSubstanceLiveLinkConnection::ConnectTextureToSampler(const FString& MapName, UMaterial* Material, UTexture2D* Texture, FExpressionInput* Input, uint32 Mask) const
{
	//sanity checks
	check(Material);
	check(Texture);
	check(Input);

	//Try to detect texture sampler parameter
	if (Input->IsConnected())
	{
		//We can replace vector parameters with little concern, so disconnect it and run again
		if (Cast<UMaterialExpressionVectorParameter>(Input->Expression) != nullptr)
		{
			Input->Expression = nullptr;
			return ConnectTextureToSampler(MapName, Material, Texture, Input, Mask);
		}

		UMaterialExpressionTextureSampleParameter2D* Expression = Cast<UMaterialExpressionTextureSampleParameter2D>(Input->Expression);

		if (Expression == nullptr)
		{
			UE_LOG(LogSubstanceLiveLink, Log, TEXT("Material \"%s\" has a complex expression bound for MapName \"%s\", skipping"), *FSoftObjectPath(Material).GetAssetPathString(), *MapName);
			return false;
		}
		else if (Expression->Texture != Texture)
		{
			Expression->Texture = Texture;
			return true;
		}

		return false;
	}
	else
	{
		UMaterialExpressionTextureSampleParameter2D* Expression = nullptr;

		//See if we can find a material expression that already holds our texture
		for (const auto& MaterialExpression : Material->Expressions)
		{
			UMaterialExpressionTextureSampleParameter2D* TextureExpression = Cast<UMaterialExpressionTextureSampleParameter2D>(MaterialExpression);

			if (TextureExpression != nullptr && TextureExpression->Texture == Texture)
			{
				Expression = TextureExpression;
				break;
			}
		}

		if (Expression == nullptr)
		{
			Expression = NewObject<UMaterialExpressionTextureSampleParameter2D>(Material);
			Expression->MaterialExpressionEditorX = -400;
			Expression->MaterialExpressionEditorY = Material->Expressions.Num() * 180;
			Expression->Texture = Texture;
			Expression->ParameterName = FName(*MapName);
			Expression->SamplerType = Expression->GetSamplerTypeForTexture(Texture);

			Material->Expressions.Add(Expression);
		}

		Input->Expression = Expression;

		TArray<FExpressionOutput> Outputs = Input->Expression->GetOutputs();

		FExpressionOutput* Output = &Outputs[0];
		Input->Mask = Output->Mask;
		Input->MaskR = (Mask & 0x8) ? Output->MaskR : 0;
		Input->MaskG = (Mask & 0x4) ? Output->MaskG : 0;
		Input->MaskB = (Mask & 0x2) ? Output->MaskB : 0;
		Input->MaskA = (Mask & 0x1) ? Output->MaskA : 0;

		return true;
	}
}

void FSubstanceLiveLinkConnection::RegisterPainterToUnrealMapInfo()
{
	//BaseColor
	PainterToUnrealMapInfo BaseColor;
	BaseColor.Name = TEXT("BaseColor");
	BaseColor.PainterExportId = TEXT("$mesh_$textureSet_BaseColor");
	BaseColor.MaterialSamplerMapArray.Add(
	{
		[](UMaterial * Material)
		{
			return &Material->BaseColor;
		},
		0xF
	});

	PainterToUnrealMapInfoArray.Add(BaseColor);

	//Occlussion/Roughness/Metallic
	PainterToUnrealMapInfo OcclusionRoughnessMetallic;
	OcclusionRoughnessMetallic.Name = TEXT("OcclusionRoughnessMetallic");
	OcclusionRoughnessMetallic.PainterExportId = TEXT("$mesh_$textureSet_OcclusionRoughnessMetallic");
	OcclusionRoughnessMetallic.MaterialSamplerMapArray.Add(
	{
		[](UMaterial * Material)
		{
			return &Material->AmbientOcclusion;
		},
		0x8
	});
	OcclusionRoughnessMetallic.MaterialSamplerMapArray.Add(
	{
		[](UMaterial * Material)
		{
			return &Material->Roughness;
		},
		0x4
	});
	OcclusionRoughnessMetallic.MaterialSamplerMapArray.Add(
	{
		[](UMaterial * Material)
		{
			return &Material->Metallic;
		},
		0x2
	});
	PainterToUnrealMapInfoArray.Add(OcclusionRoughnessMetallic);

	//Emission
	PainterToUnrealMapInfo Emissive;
	Emissive.Name = TEXT("Emissive");
	Emissive.PainterExportId = TEXT("$mesh_$textureSet_Emissive");
	Emissive.MaterialSamplerMapArray.Add(
	{
		[](UMaterial * Material)
		{
			return &Material->EmissiveColor;
		},
		0xF
	});
	PainterToUnrealMapInfoArray.Add(Emissive);

	//Normal
	PainterToUnrealMapInfo Normal;
	Normal.Name = TEXT("Normal");
	Normal.PainterExportId = TEXT("$mesh_$textureSet_Normal");
	Normal.MaterialSamplerMapArray.Add(
	{
		[](UMaterial * Material)
		{
			return &Material->Normal;
		},
		0xF
	});
	PainterToUnrealMapInfoArray.Add(Normal);
}

void FSubstanceLiveLinkConnection::QueueGameThreadMessage(TFunction<void ()> Message)
{
	FScopeLock Lock(&GameThreadMessagesCS);
	GameThreadMessages.Add(Message);
}

UMaterial* FSubstanceLiveLinkConnection::ReplaceMaterial(const UObject* Mesh, const UMaterial* Material, const FString& MaterialSlotName) const
{
	FString MaterialName;
	if (Material)
		Material->GetName(MaterialName);
	else
		MaterialName = MaterialSlotName;

	if (MaterialName == "None")
	{
		return nullptr;
	}

	FString OuterName;
	if (Material)
		OuterName = Material->GetOuter()->GetName();
	else
		OuterName = Mesh->GetOuter()->GetName();

	FString Left;
	FString Right;
	//Create an unreal material asset
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	OuterName.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FString PkgName = Left + TEXT("/") + MaterialName;
	UPackage* Pkg = CreatePackage(nullptr, *PkgName);
	Pkg->FullyLoad();
	UMaterial* NewMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(
	                             UMaterial::StaticClass(),
	                             Pkg,
	                             *MaterialName,
	                             RF_Standalone | RF_Public, nullptr, GWarn);

	NewMaterial->PreEditChange(nullptr);
	NewMaterial->PostEditChange();

	return NewMaterial;
}
