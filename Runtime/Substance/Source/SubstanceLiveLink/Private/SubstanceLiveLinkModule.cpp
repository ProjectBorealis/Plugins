// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkModule.cpp
#include "SubstanceLiveLinkModule.h"
#include "SubstanceLiveLinkPrivatePCH.h"
#include "SubstanceLiveLinkStyle.h"
#include "SubstanceLiveLinkConnection.h"
#include "SubstanceLiveLinkTextureLoader.h"
#include "ContentBrowserDelegates.h"
#include "ContentBrowserModule.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UIAction.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Textures/SlateIcon.h"

#define LOCTEXT_NAMESPACE "SubstanceLiveLinkModule"

namespace SubstanceLiveLinkModule
{
const FName SubstanceLiveLinkAppIdentifier = FName(TEXT("SubstanceLiveLinkApp"));
}

class FSubstanceLiveLinkModule : public ISubstanceLiveLinkModule
{
public:
	/**
	 * Constructor
	 */
	FSubstanceLiveLinkModule()
	{
	}

	/**
	 * Called right after the module DLL has been loaded and the module object has been created
	 */
	virtual void StartupModule() override
	{
		if (!IsRunningCommandlet())
		{
			FSubstanceLiveLinkStyle::Initialize();

			TextureLoader = MakeShared<FSubstanceLiveLinkTextureLoader>();

			//connect to Substance Painter
			LiveLinkConnection = MakeShared<FSubstanceLiveLinkConnection>();
			LiveLinkConnection->OnConnectionError().AddLambda([]
			{
				ErrorPopup(TEXT(
				               "Substance Painter is not detected.\n"
				               "Please verify Substance Painter is running and that the \"live-link\" plugin is enabled."
				           ));
			});

			//add our content browser menu extension
			ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FSubstanceLiveLinkModule::OnExtendContentBrowserAssetSelectionMenu);

			TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = GetContentBrowserExtenderDelegates();
			CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
			ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
		}
	}

	/**
	 * Called before the module is unloaded, right before the module object is destroyed
	 */
	virtual void ShutdownModule() override
	{
		if (UObjectInitialized())
		{
			//remove out contente browser menu extender
			TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = GetContentBrowserExtenderDelegates();
			CBMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets & Delegate)
			{
				return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle;
			});
		}

		LiveLinkConnection.Reset();
		TextureLoader.Reset();

		FSubstanceLiveLinkStyle::Shutdown();
	}

	virtual TSharedRef<FSubstanceLiveLinkTextureLoader> GetTextureLoader() override
	{
		return TextureLoader.ToSharedRef();
	}

private:
	/**
	 * Acquire ContextMenuExtenders from ContentBrowser
	 * @return Returns the Content Browser Extender Delegates for registering/unregistering our custom extension object
	 */
	static TArray<FContentBrowserMenuExtender_SelectedAssets>& GetContentBrowserExtenderDelegates()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		return ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	}

	/**
	 * Extend the Content Browser If a Static Mesh is Selected
	 * @param SelectedAssets The currently selected assets in the content browser
	 * @return Returns the Menu Extension object with our context menu addon
	 */
	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		if (SelectedAssets.Num() == 1 && IsMeshClass(SelectedAssets[0].GetClass()))
		{
			// Add the sprite actions sub-menu extender
			Extender->AddMenuExtension(
			    "GetAssetActions",
			    EExtensionHook::After,
			    nullptr,
			    FMenuExtensionDelegate::CreateStatic(&FSubstanceLiveLinkModule::CreateLiveLinkActionsSubMenu, SelectedAssets));
		}

		return Extender;
	}

	/**
	 * Create the menu items that allow the user to send a mesh to Substance Painter
	 * MenuBuilder Used to add our context menu entry
	 * SelectedAssets List of assets currently selected, should just be one and should just be a mesh
	 */
	static void CreateLiveLinkActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		check(SelectedAssets.Num() == 1);
		check(IsMeshClass(SelectedAssets[0].GetClass()));

		auto Command = [SelectedAssets]()
		{
			if (UObject* Mesh = SelectedAssets[0].GetAsset())
			{
				static_cast<FSubstanceLiveLinkModule*>(ISubstanceLiveLinkModule::Get())->OnSendToSubstancePainter(Mesh);
			}
		};

		MenuBuilder.AddMenuEntry(
		    NSLOCTEXT("AssetTypeActions_SubstanceLiveLink", "ObjectContext_SendToPainter", "Send to Substance Painter"),
		    NSLOCTEXT("AssetTypeActions_SubstanceLiveLink", "ObjectContext_SendToPainterTooltip", "Send the mesh and material set to Substance Painter for linked editing"),
		    FSlateIcon(FSubstanceLiveLinkStyle::GetStyleSetName(), "Icon.SubstancePainter16px"),
		    FUIAction(
		        FExecuteAction::CreateLambda(Command),
		        FCanExecuteAction()
		    )
		);
	}

	/**
	 * Executed when user selects the "Send to Substance Painter" Entry in the Context Menu
	 * @param Mesh the Mesh Object to send to Substance Painter
	 */
	void OnSendToSubstancePainter(UObject* Mesh)
	{
		check(LiveLinkConnection.IsValid());

		LiveLinkConnection->SendAssetToSubstancePainter(Mesh);
	}

	/**
	 * Submit a modal dialog box to send error messages to the user
	 * @param ErrorMessage Error Message to display to user
	 */
	static void ErrorPopup(const FString& ErrorMessage)
	{
		FText DialogTitle = FText::FromString("Substance Painter Live Link");
		FText DialogText = FText::FromString(ErrorMessage);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
	}

private:
	/** Add our hooks into the ContentBrowser context menus using this delegate */
	FContentBrowserMenuExtender_SelectedAssets	ContentBrowserExtenderDelegate;
	FDelegateHandle								ContentBrowserExtenderDelegateHandle;

	/** Current painter connection */
	TSharedPtr<FSubstanceLiveLinkConnection>	LiveLinkConnection;

	/** Texture Loader used to help importing / loading UTexture2Ds */
	TSharedPtr<FSubstanceLiveLinkTextureLoader>	TextureLoader;
};

ISubstanceLiveLinkModule* ISubstanceLiveLinkModule::Get()
{
	return &FModuleManager::LoadModuleChecked<FSubstanceLiveLinkModule>("SubstanceLiveLink");
}

bool ISubstanceLiveLinkModule::IsMeshClass(UClass* Class)
{
	return (Class == UStaticMesh::StaticClass() || Class == USkeletalMesh::StaticClass());
}

//==================================================
IMPLEMENT_MODULE(FSubstanceLiveLinkModule, SubstanceLiveLink);

#undef LOCTEXT_NAMESPACE
