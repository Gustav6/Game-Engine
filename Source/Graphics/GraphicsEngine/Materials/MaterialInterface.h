#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class PipelineStateObject;
class Texture;

/**
 * Which Domain this Material belongs to. Controls the Vertex type, Vertex Shader and Topology.
 */
enum class MaterialDomain : uint8_t
{
	None,
	Surface,
};

/**
 * The Shading Model of this Material. Controls the Pixel Shader.
 */
enum class ShadingModel : uint8_t
{
	None,
	Unlit,
	Lit
};

/**
 * How this Material is Blended. Controls how a finished pixel is composited.
 */
enum class BlendMode : uint8_t
{
	None,
	Opaque,
};

/**
 * Enum of types used in HLSL during the AGP Course. To make matching easier.
 */
enum class MaterialParameterType : uint8_t
{
	Unknown,
	Float, Float2, Float3, Float4,
	Int, Int2, Int3, Int4,
	Uint, Uint2, Uint3, Uint4,
	Bool, Matrix4x4
};

/**
 * Information about a Material Parameter found in the HLSL MaterialParametersBuffer
 */
struct MaterialParameterInfo
{
	std::string Name;
	MaterialParameterType Type = MaterialParameterType::Unknown;
	size_t Offset = 0;
	size_t Size = 0;
	size_t Index = 0;
};

/**
 * Generic Interface for Material-like objects.
 * Provides a concrete method of communication with both Material and MaterialInstance
 * which both look the same outwards but work differently internally.
 */
class MaterialInterface
{
public:
	// Maximum allowed size of the HLSL MaterialParametersBuffer constant buffer.
	// This is to make sure we don't underflow data storage on the C++ side.
	// This cannot be more than 65535 (64 kB)! Hard limit for CBuffers.
	static constexpr unsigned MATERIAL_BUFFER_SIZE = 128;

	// This cannot ever be more than 16.
	// VS and GS stages are limited to 16 textures while PS and CS can have 128.
	// However, to keep things simple we assume that >= 16 are system slots
	// which cannot be used by materials.
	static constexpr unsigned MAX_MATERIAL_TEXTURE_COUNT = 16;

	MaterialInterface();
	virtual ~MaterialInterface();

	MaterialInterface(const MaterialInterface&) = delete;
	MaterialInterface& operator=(const MaterialInterface&) = delete;

	MaterialInterface(MaterialInterface&&) noexcept = default;
	MaterialInterface& operator=(MaterialInterface&&) noexcept = default;

	virtual std::string_view GetName() const = 0;

	virtual MaterialDomain GetDomain() const = 0;
	virtual ShadingModel GetShadingModel() const = 0;
	virtual BlendMode GetBlendMode() const = 0;

	virtual const PipelineStateObject& GetPSO() const = 0;

	/**
	 * If True there has been changes to the C++ side MaterialParametersBuffer
	 * and you need to call RefreshMaterialData(). Bubbles upwards along Child-Parent relationships
	 * so that if a Parent MaterialParametersBuffer is Dirty we need to rebuild.
	 * @return True if the MaterialParametersBuffer has been changed since last call to RefreshMaterialData.
	 */
	virtual bool IsMaterialDataDirty() const = 0;

	/**
	 * Refreshes the MaterialParametersBuffer settings. Also asks our Parent, if there is one, for
	 * updated MaterialParametersBuffer settings.
	 * @return 
	 */
	virtual void RefreshMaterialData() const = 0;

	/**
	 * Retrieves a pointer to the C++ MaterialParametersBuffer data.
	 * @return 
	 */
	virtual const uint8_t* GetParameterDataBlock() const = 0;

	/**
	 * If this Material has any Parameters or not.
	 * @return 
	 */
	virtual bool HasParameters() const = 0;

	/**
	 * Retrieve the list of Parameters for this Material.
	 * @return 
	 */
	virtual const std::vector<MaterialParameterInfo>& GetParameters() const = 0;	

	/**
	 * Retrieves a pointer to the MaterialParameterInfo based on the Parameter Index.
	 * @param aIndex The Index of the Parameter to retrieve info about or nullptr if Index is invalid.
	 */
	virtual const MaterialParameterInfo* GetParameterByIndex(unsigned aIndex) const = 0;

	/**
	 * Retrieves a pointer to the MaterialParameterInfo based on the Parameter Name.
	 * @param aName The Name of the Parameter to retrieve info about or nullptr if Name is invalid.
	 */
	virtual const MaterialParameterInfo* GetParameterByName(const std::string& aName) const = 0;

	/**
	 * Gets the texture slot index bound to the given name.
	 * @param aName The name of the texture slot to look up.
	 * @return The slot index associated with aName.
	*/
	virtual unsigned int GetTextureSlotByName(const std::string& aName) const = 0;

	/**
	 * Sets a texture in the slot associated with the given name.
	 * @param aName The name of the texture slot to set.
	 * @param aTexture The texture to bind to the slot.
	 * @return True if the texture was set successfully, false otherwise.
	*/
	virtual bool SetTexture(const std::string& aName, const std::shared_ptr<Texture>& aTexture) = 0;

	/**
	 * Sets a texture in the given slot.
	 * @param aSlot The index of the slot to set.
	 * @param aTexture The texture to bind to the slot.
	 * @return True if the texture was set successfully, false otherwise.
	*/
	virtual bool SetTexture(unsigned int aSlot, const std::shared_ptr<Texture>& aTexture) = 0;

	/**
	 * Gets the texture bound to the slot associated with the given name.
	 * @param aName The name of the texture slot to look up.
	 * @return The texture bound to the named slot, or nullptr if none is set.
	*/
	virtual std::shared_ptr<Texture> GetTexture(const std::string aName) const = 0;

	/**
	 * Gets the texture bound to the given slot.
	 * @param aSlot The index of the slot to look up.
	 * @return The texture bound to the slot, or nullptr if none is set.
	*/
	virtual std::shared_ptr<Texture> GetTexture(unsigned int aSlot) const = 0;
};