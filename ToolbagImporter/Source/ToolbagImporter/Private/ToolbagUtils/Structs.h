#pragma once

namespace ToolbagUtils
{
enum MatFieldType
{
	MATFIELD_BOOL,
	MATFIELD_FLOAT,
	MATFIELD_COLOR,
	MATFIELD_ENUM,
	MATFIELD_INT,
	MATFIELD_TEXTURE,

	MATFIELD_MAX_ENUM
};
struct MatField
{
	char* name;
	MatFieldType type;
};

struct MatFieldBool : public MatField
{
	bool value;
};

struct MatFieldFloat : public MatField
{
	float value;
};

struct MatFieldColor : public MatField
{
	float value[3];
};

struct MatFieldEnum : public MatField
{
	int value;
};

struct MatFieldInt : public MatField
{
	int value;
};

struct Texture
{
	char* TexturePath;
	enum
	{
		TEXTURE_FILTER_LINEAR,
		TEXTURE_FILTER_NEAREST
	}
	filterMode;
	enum
	{
		TEXTURE_WRAP_CLAMP,
		TEXTURE_WRAP_REPEAT
	}
	wrapMode;
	bool sRGB;
};

struct MatFieldTexture : public MatField
{
	Texture texture;
};

struct MaterialSubroutine
{
	char* name;
	MatField** MatFields;
	unsigned int MatFieldsCount;
};

struct Material
{
	enum
	{
		SUBROUTINE_SUBDIVISION,
		SUBROUTINE_DISPLACEMENT,
		SUBROUTINE_SURFACE,
		SUBROUTINE_MICROSURFACE,
		SUBROUTINE_ALBEDO,
		SUBROUTINE_DIFFUSION,
		SUBROUTINE_REFLECTIVITY,
		SUBROUTINE_REFLECTION,
		SUBROUTINE_REFLECTION_SECONDARY,
		SUBROUTINE_OCCLUSION,
		SUBROUTINE_EMISSIVE,
		SUBROUTINE_TRANSPARENCY,
		SUBROUTINE_MERGE,
		SUBROUTINE_EXTRA,

		SUBROUTINE_MAX_ENUM
	};
	char* name;
	MaterialSubroutine* Subroutines[SUBROUTINE_MAX_ENUM];
};

struct SceneObject
{
	enum SceneObjectType
	{
		SO_SCENE_OBJECT,
		SO_MESH,
		SO_EXTERNAL,
		SO_LIGHT,
		SO_CAMERA,
		SO_SKY,
		SO_DOCUMENT,
		SO_MAX_ENUM,
	} type;
	char* name;
	bool isVisible;
	SceneObject** children;
	unsigned int childCount;
	float matrix[16];
};

struct Mesh
{
	unsigned int* indices;
	unsigned int indexCount;
	unsigned int vertexCount;
	unsigned int verticesComps;
	float* vertices;
	unsigned int colorsComps;
	float* colors;
	unsigned int tangentsComps;
	float* tangents;
	unsigned int bitangentsComps;
	float* bitangents;
	unsigned int normalsComps;
	float* normals;
	unsigned int texcoords0Comps;
	float* texcoords0;
	unsigned int texcoords1Comps;
	float* texcoords1;
	bool castShadows;
	bool cullBackFaces;
};

struct SubMesh
{
	char* materialName;
	unsigned int startIndex;
	unsigned int indexCount;
};

struct MeshSceneObject : public SceneObject
{
	Mesh mesh;
	SubMesh* subMeshes;
	unsigned int subMeshCount;
};

struct ExternalSceneObject : public SceneObject
{
};

struct LightSceneObject : public SceneObject
{
	enum LightType
	{
		LIGHT_DIRECTIONAL =	0,
		LIGHT_SPOT =		1,
		LIGHT_OMNI =		2,
	} lightType;
	float color[3];
	float brightness;
	float radius;
	float attenuation;
	float length[2];
	float size;
	float spotAngle;
	float spotSharpness;
	float spotVignette;
	bool castShadows;
	Texture gel;
	float gelTile;
};

struct CameraSceneObject : public SceneObject
{
	float verticalFieldOfViewDegrees;
};

struct SkySceneObject : public SceneObject
{
	char* path;
	float rotation;
	float brightness;
};

enum SceneUnit
{
	MILLIMETER =	0,
	CENTIMETER =	1,
	METER =			2,
	KILOMETER =		3,

	INCH =			4,
	FOOT =			5,
	YARD =			6,
	MILE =			7,

	MAX_ENUM
};

struct DocumentSceneObject : public SceneObject
{
	SceneUnit unitScene;
	SceneUnit unitImport;
	float sceneScale;
};

struct Scene
{
	Material* materials;
	unsigned int materialsCount;
	SceneUnit unitScene;
	SceneUnit unitImport;
	float sceneScale;
	SceneObject* rootObject;
};
};