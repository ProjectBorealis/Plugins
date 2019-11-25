// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCoreHelpers.h

#pragma once
#ifdef SUBSTANCE_FRAMEWORK_INCLUDED
#include <substance/pixelformat.h>
#include <substance/framework/graph.h>
#include <substance/framework/preset.h>
#include "substance/framework/typedefs.h"
#endif

#include <Materials/Material.h>

class USubstanceImageInput;
class USubstanceInstanceFactory;
class USubstanceGraphInstance;
class USubstanceTexture2D;
class UObject;

//Struct used for capturing material linking data for post reimport
struct MaterialParameterSet
{
	UMaterial* Material;
	TMap<FString, class UMaterialExpressionTextureSample*> ParameterNames;

	//Default constructor and destructor
	MaterialParameterSet() : Material(nullptr) { }
	~MaterialParameterSet() { }

	//Comparison operator for adding unique elements to a TArray
	bool operator==(const MaterialParameterSet& rhs) const
	{
		bool IsEqual = (this->Material == rhs.Material && rhs.ParameterNames.Num() == this->ParameterNames.Num());
		return IsEqual;
	}
};

struct MaterialInstanceParameterSet
{
	UMaterialInstance* MaterialInstance;
	TMap<FString, FName> ParameterNames;

	//Default constructor and destructor
	MaterialInstanceParameterSet() : MaterialInstance(nullptr) { }
	~MaterialInstanceParameterSet() { }

	//Comparison operator for adding unique elements to a TArray
	bool operator==(const MaterialInstanceParameterSet& rhs) const
	{
		bool IsEqual = (this->MaterialInstance == rhs.MaterialInstance && rhs.ParameterNames.Num() == this->ParameterNames.Num());
		return IsEqual;
	}
};

struct OutputTextureSettings
{
	FString PackageName;
	uint8 sRGB_Enabled;

	//Default constructor and destructor
	OutputTextureSettings() : sRGB_Enabled(false) { }
	//OutputTextureSettings(const& FString packageName, uint8 srgbEnabled) : sRGB_Enabled(srgbEnabled), PackageName(packageName) { }
	~OutputTextureSettings() { }
};

//Framework forward declarations
namespace SubstanceAir
{
struct GraphDesc;
class PackageDesc;
class InputImage;
class OutputInstance;
class InputInstanceBase;
class InputInstanceImage;
class InputInstanceNumericalBase;
class ImageInput;
class GraphInstance;
}

//A collection of helper functions used for both editor and core
namespace Substance
{
struct RenderCallbacks;

/** Used for rebuilding instances within substance factory */
struct InstanceBackup
{
	SubstanceAir::Preset preset;
	USubstanceGraphInstance* InstanceParent;
	TMap<uint32, FString> UidToName;
};

/** Contains helper functions to call from the main thread. */
namespace Helpers
{
SUBSTANCECORE_API RenderCallbacks* GetRenderCallbacks();

/** Perform an asynchronous (non-blocking) rendering of those instances */
SUBSTANCECORE_API void RenderAsync(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>&);
SUBSTANCECORE_API void RenderAsync(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);

/** Perform a blocking rendering of those instances */
SUBSTANCECORE_API void RenderSync(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>&);
SUBSTANCECORE_API void RenderSync(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);

/** UE4 Play in editor callback */
SUBSTANCECORE_API void StartPIE();
SUBSTANCECORE_API void EndPIE();

/** Queue the graph instance for later render -- Used during seek free loading */
SUBSTANCECORE_API void PushDelayedRender(USubstanceGraphInstance* Instance);
SUBSTANCECORE_API void PushDelayedRender(SubstanceAir::InputInstanceImage* ImgInput, USubstanceGraphInstance* Instance);

/** Queue an image input until its source has been rendered */
SUBSTANCECORE_API void PushDelayedImageInput(SubstanceAir::InputInstanceImage* ImgInput, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance);
SUBSTANCECORE_API void RemoveFromDelayedImageInputs(SubstanceAir::InputInstanceImage* ImgInput);

/** UE4 level callbacks */
void OnWorldInitialized();
void OnLevelAdded();

/** Render queued graph instances */
void PerformDelayedRender();

/** Update Texture Output */
void UpdateTexture(const SubstanceTexture& result, SubstanceAir::OutputInstance* Output, bool bCacheResults = true);

/** Perform per frame Substance management */
SUBSTANCECORE_API void Tick();

/** Create a texture 2D object using an output instance */
SUBSTANCECORE_API void CreateSubstanceTexture2D(SubstanceAir::OutputInstance* output, bool bTransient = false,
        FString Name = FString(), UObject* InOuter = nullptr, USubstanceGraphInstance* ParentInst = nullptr);

/** Initializes 4 levels of mip maps */
SUBSTANCECORE_API void CreatePlaceHolderMips(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance);

/** Convert a PixelFormat to the closest supported PixelFormat - If new types added, alter SubstanceToUE4Format*/
SUBSTANCECORE_API void ValidateFormat(USubstanceTexture2D* Texture, SubstanceAir::OutputInstance* Output);

/** Convert from a Substance Format to UE4 equivalent */
EPixelFormat SubstanceToUe4Format(const SubstancePixelFormat Format, const SubstanceChannelsOrder ChanOrder);

/** Convert from a UE4 pixel format to a Substance Format */
SubstancePixelFormat UE4FormatToSubstance(EPixelFormat Fmt);

/** Creates and enables a texture object*/
SUBSTANCECORE_API void EnableTexture(SubstanceAir::OutputInstance* Output, USubstanceGraphInstance* Graph, const OutputTextureSettings* TextureSettings = nullptr);

/** Create textures of empty output instances of the given graph instance */
void CreateTextures(USubstanceGraphInstance* Graph);

/** Create textures for output instances that only exist at runtime */
void CreateTransientTextures(USubstanceGraphInstance* Graph, UObject* Outer);

/** Splits RGBA value into proper channels */
SUBSTANCECORE_API void Split_RGBA_8bpp(int32 Width, int32 Height,
                                       uint8* DecompressedImageRGBA, const int32 TextureDataSizeRGBA,
                                       uint8* DecompressedImageA = nullptr, const int32 TextureDataSizeA = 0);

/** Update the outputs of each instance */
SUBSTANCECORE_API bool UpdateTextures(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>& Instances);
SUBSTANCECORE_API bool UpdateTextures(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance);

/** Compare the values of two input instances. */
bool AreInputValuesEqual(SubstanceAir::InputInstanceBase& a, SubstanceAir::InputInstanceBase& b);

/** Updates Input values */
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, class UObject* InValue);
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const FString& InValue);
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const TArray<int32>& InValue);
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const TArray<float>& InValue);
int32 UpdateInputHelper(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const uint32& Uid);

template <typename T> void SetNumericalInputValueHelper(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<T>& InValue);
SUBSTANCECORE_API void SetNumericalInputValue(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<int>& InValue);
SUBSTANCECORE_API void SetNumericalInputValue(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<float>& InValue);
SUBSTANCECORE_API void SetStringInputValue(const SubstanceAir::InputInstanceBase* BaseInput, const FString& InValue);

/** Return the string representation of the Input's value */
SUBSTANCECORE_API FString GetValueAsString(const SubstanceAir::InputInstanceBase& Input);

/** Return the value of a float input */
TArray<float> GetValueFloat(const SubstanceAir::InputInstanceBase& Input);

/** Return the value of a int input */
TArray<int32> GetValueInt(const SubstanceAir::InputInstanceBase& Input);

/** Returns a string value of a string input*/
FString GetValueString(const SubstanceAir::InputInstanceBase& Input);

/** Setup the rendering system for commandlet or things like that */
void SetupSubstance(void* library);

/** Cleans up the rendering system */
void TearDownSubstance();

/** Disable an output -- This will delete the outputs texture */
SUBSTANCECORE_API void Disable(SubstanceAir::OutputInstance* output, bool FlagToDelete = true);

/** Find which desc an instance belongs to */
const SubstanceAir::GraphDesc* FindParentGraph(const std::vector<SubstanceAir::GraphDesc>& Graphs, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);
const SubstanceAir::GraphDesc* FindParentGraph(const std::vector<SubstanceAir::GraphDesc>& Graphs, const FString& URL);

/** Decompress a jpeg buffer in RAW RGBA */
SUBSTANCECORE_API void DecompressJpeg(const void* Buffer, const int32 Length, TArray<uint8>& outRawData,
                                      int32* outW, int32* outH, int32 NumComp = 4);

/** Compress a jpeg buffer in RAW RGBA */
SUBSTANCECORE_API void CompressJpeg(
    const void* InBuffer, const int32 InLength, const int32 W, const int32 H, const int32 NumComp,
    TArray<uint8>& CompressedImage, const int32 Quality = 85);

/** Sets up an InputImage and returns the shared pointer */
SubstanceAir::shared_ptr<SubstanceAir::InputImage> PrepareImageInput(class UObject* InValue,
        SubstanceAir::InputInstanceImage* Input, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Parent);

/** Calculates image size per mip level - UE4's implementation doesn't support low mips leves! */
SIZE_T CalculateSubstanceImageBytes(uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format);

/** Checks to see if a format type is currently supported */
bool IsSupportedImageInput(UObject*);

/** Link the image */
void LinkImageInput(SubstanceAir::InputInstanceImage* ImgInputInst, USubstanceImageInput* SrcImageInput);

/** Sets the properties of an Image Input Instance */
void SetImageInput(SubstanceAir::InputInstanceImage* Input, UObject* InValue, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Parent,
                   bool unregisterOutput = true, bool isTransacting = false);

/** Clears a graph instance from the Renderer */
void ClearFromRender(USubstanceGraphInstance* GraphInstance);

/** Resets the graph instance image inputs */
void ResetGraphInstanceImageInputs(USubstanceGraphInstance* GraphInstance);

/** brief Create an Unreal Material for the given graph-instance */
SUBSTANCECORE_API TWeakObjectPtr<UMaterial> CreateMaterial(USubstanceGraphInstance* Instance, const FString& MatName, UObject* Outer = nullptr);

/** Sets all of the material inputs to the proper outputs from a graph instance */
SUBSTANCECORE_API void ResetMaterialTexturesFromGraph(USubstanceGraphInstance* Graph, UMaterial* OwnedMaterial, const TArray<MaterialParameterSet>& Materials);

/** Sets all of the material instance parameters to the proper outputs from a graph instance */
SUBSTANCECORE_API void ResetMaterialInstanceTexturesFromGraph(USubstanceGraphInstance* Graph, const TArray<MaterialInstanceParameterSet>& Materials);

/** Reset an instance's inputs to their default values -- Does not trigger rendering of the instance */
SUBSTANCECORE_API void ResetToDefault(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);

/** Reset an instance's input to its default value -- Does no trigger rendering of the instance */
SUBSTANCECORE_API void ResetToDefault(SubstanceAir::InputInstanceBase* Input);

/** Fills out string values with a unique name */
SUBSTANCECORE_API void GetSuitableName(SubstanceAir::OutputInstance* Instance,
                                       FString& OutTextureName, FString& OutPackageName, USubstanceGraphInstance* OptionalParentGraph = nullptr);

/** Create instance of a graph desc */
SUBSTANCECORE_API USubstanceGraphInstance* InstantiateGraph(USubstanceInstanceFactory* ParentFactory, const SubstanceAir::GraphDesc& Graph, UObject* Outer,
        FString InstanceName = FString(), bool bCreateOutputs = false, EObjectFlags Flags = RF_Standalone | RF_Public);

/** Create instance of a graph desc */
SUBSTANCECORE_API USubstanceGraphInstance* InstantiateGraph(const SubstanceAir::GraphDesc& Graph, UObject* Outer,
        FString InstanceName = FString(), bool bCreateOutputs = false, EObjectFlags Flags = RF_Standalone | RF_Public);

/** Copy constructor for USubstanceGraphInstance */
USubstanceGraphInstance* DuplicateGraphInstance(USubstanceGraphInstance* Instance);

/** Copy constructor for USubstanceGraphInstance */
void CopyInstance(USubstanceGraphInstance* RefInstance, USubstanceGraphInstance* NewInstance, bool bCopyOutputs = true);

/** Queues different wrapper types for cleanup */
SUBSTANCECORE_API void RegisterForDeletion(class USubstanceGraphInstance* InstanceContainer);
SUBSTANCECORE_API void RegisterForDeletion(class USubstanceTexture2D* InstanceContainer);
SUBSTANCECORE_API void RegisterForDeletion(class USubstanceInstanceFactory* InstanceContainer);
SUBSTANCECORE_API void PerformDelayedDeletion();

/** Creates a package for an object */
SUBSTANCECORE_API UPackage* CreateObjectPackage(UObject* Outer, FString ObjectName);

/** Returns the loading progress percentage */
float GetSubstanceLoadingProgress();

/** Creates a substance air package desc and return it - This is here for the editor module to use*/
SUBSTANCECORE_API SubstanceAir::shared_ptr<SubstanceAir::PackageDesc> InstantiatePackage(void* assemData, uint32 size);

/** Assigns a graph instance input deltas to the preset passed in */
SUBSTANCECORE_API void CopyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Source, SubstanceAir::Preset& Dest);

/** Get empty preset as shared */
SUBSTANCECORE_API SubstanceAir::shared_ptr<SubstanceAir::Preset> EmptyPresetAsShared();

/** Assigns a graph instance input deltas to the preset passed in */
SUBSTANCECORE_API void CopyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Source, SubstanceAir::Preset& Dest);

/** Apply Preset data to a graph instance*/
SUBSTANCECORE_API void ApplyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Destination, SubstanceAir::Preset& Source);

/** Exposes the frameworks instantiate graphs for the editor*/
SUBSTANCECORE_API void InstatiateGraphsFromPackage(SubstanceAir::GraphInstances& Instances, SubstanceAir::PackageDesc* Package);

/** Exports input deltas from graph instance */
SUBSTANCECORE_API bool ExportPresetFromGraph(USubstanceGraphInstance*);

/** Loads and sets input deltas for graph instance */
SUBSTANCECORE_API bool ImportAndApplyPresetForGraph(USubstanceGraphInstance*);

/** Imports a preset file */
SUBSTANCECORE_API FString ImportPresetFile();

/** Exports a preset file */
SUBSTANCECORE_API FString ExportPresetFile(FString SuggestedFilename);

/** Load and apply presets to a given graph instance*/
SUBSTANCECORE_API void ParsePresetAndApply(SubstanceAir::Presets& CurrentPresets, FString& xmlString, USubstanceGraphInstance* Graph);

/** Saves the current input deltas to a file */
SUBSTANCECORE_API void WriteSubstanceXmlPreset(SubstanceAir::Preset& preset, FString& XmlContent);

/** Clear current cache to free up memory */
SUBSTANCECORE_API void ClearCache();

#if WITH_EDITOR
/** Recomputes all substance graph instances! - Only available from editor */
SUBSTANCECORE_API void RebuildAllSubstanceGraphInstances();

/** Creates a default unique material name and creates a material */
SUBSTANCECORE_API void CreateDefaultNamedMaterial(USubstanceGraphInstance* Graph);
#endif // WITH_EDITOR

} // namespace Helpers
} // namespace Substance
