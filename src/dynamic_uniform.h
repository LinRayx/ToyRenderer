#pragma once

#ifndef DYNAMIC_UNIFORM_H
#define DYNAMIC_UNIFORM_H

#include <optional>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

// master list of leaf types that generates enum elements and various switches etc.
#define LEAF_ELEMENT_TYPES \
	X( Float ) \
	X( Float2 ) \
	X( Float3 ) \
	X( Float4 ) \
	X( Matrix ) \
	X( Bool )

namespace Dcf {

	using namespace glm;
	enum Type
	{
	#define X(el) el,
			LEAF_ELEMENT_TYPES
	#undef X
			Struct,
			Array,
			Empty,
	};

	// static map of attributes of each leaf type
	template<Type type>
	struct Map
	{
		static constexpr bool valid = false;
	};
	template<> struct Map<Float>
	{
		using SysType = float; // type used in the CPU side
		static constexpr size_t glslSize = sizeof(SysType); // size of type on GPU side
		static constexpr const char* code = "F1"; // code used for generating signature of layout
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>
	};
	template<> struct Map<Float2>
	{
		using SysType = vec2;
		static constexpr size_t glslSize = sizeof(SysType);
		static constexpr const char* code = "F2";
		static constexpr bool valid = true;
	};
	template<> struct Map<Float3>
	{
		using SysType = vec3;
		static constexpr size_t glslSize = sizeof(SysType);
		static constexpr const char* code = "F3";
		static constexpr bool valid = true;
	};
	template<> struct Map<Float4>
	{
		using SysType = vec4;
		static constexpr size_t glslSize = sizeof(SysType);
		static constexpr const char* code = "F4";
		static constexpr bool valid = true;
	};
	template<> struct Map<Matrix>
	{
		using SysType = mat4;
		static constexpr size_t glslSize = sizeof(SysType);
		static constexpr const char* code = "M4";
		static constexpr bool valid = true;
	};
	template<> struct Map<Bool>
	{
		using SysType = bool;
		static constexpr size_t glslSize = 4u;
		static constexpr const char* code = "BL";
		static constexpr bool valid = true;
	};

	class LayoutCodex {
	public:
		static CookedLayout Resolve(RawLayout&& layout) ;
	private:

		static LayoutCodex& Get_() noexcept;
		std::unordered_map<std::string, std::shared_ptr<LayoutElement>> map;
	};

	// CookedLayout represend a completed and registered Layout shell object
	// layout tree is fixed
	class CookedLayout : public Layout
	{
		friend class LayoutCodex;
		friend class Buffer;
	public:
		
		// key into the root Struct (const to disable mutation of the layout)
		const LayoutElement& operator[](const std::string& key) const;
		// get a share on layout tree root
		std::shared_ptr<LayoutElement> ShareRoot() const noexcept;
	private:
		// this ctor used by Codex to return cooked layouts
		CookedLayout(std::shared_ptr<LayoutElement> pRoot) noexcept;
		// use to pilfer the layout tree
		std::shared_ptr<LayoutElement> RelinquishRoot() const noexcept;
	};

	class Buffer {
	public:
		Buffer(const CookedLayout& lay);
		Buffer(RawLayout&& lay);
		// get the raw bytes
		const char* GetData() const noexcept;
		// size of the raw byte buffer
		size_t GetSizeInBytes() const noexcept;
	private:
		std::shared_ptr<LayoutElement> pLayoutRoot;
		std::vector<char> bytes;
	};

	class Layout
	{
		friend class LayoutCodex;
		friend class Buffer;
	public:
		size_t GetSizeInBytes() const noexcept;
		std::string GetSignature() const;
	protected:
		Layout(std::shared_ptr<LayoutElement> pRoot) noexcept;
		std::shared_ptr<LayoutElement> pRoot;
	};

	class RawLayout : Layout {
	public:
		RawLayout() noexcept;
		// reset this object with an empty struct at its root
		void ClearRoot() noexcept;
		// finalize the layout and then relinquish (by yielding the root layout element)
		std::shared_ptr<LayoutElement> DeliverRoot() noexcept;
	};

	class LayoutElement 
	{
	private:
		struct ExtraDataBase 
		{
			virtual	~ExtraDataBase() = default;
		};

		friend class RawLayout;
		friend struct ExtraData;

		LayoutElement() noexcept = default;
	public:
		size_t GetOffsetBegin() const;
		size_t GetOffsetEnd() const;
		size_t GetSizeInBytes() const;
		// [] only works for Structs; access member (child node in tree) by name
		LayoutElement& operator[](const std::string& key) ;
		const LayoutElement& operator[](const std::string& key) const ;
		LayoutElement& Add(Type addedType, std::string name);
		template<Type typeAdded>
		LayoutElement& Add(std::string key)
		{
			return Add(typeAdded, std::move(key));
		};

		std::string GetSignature() const;
	private:
		size_t Finalize(size_t offsetIn);
		size_t FinalizeForStruct(size_t offsetIn);
		std::string GetSignatureForStruct() const;
		static LayoutElement& GetEmptyElement() noexcept
		{
			static LayoutElement empty{};
			return empty;
		}
		LayoutElement(Type typeIn);
		// returns the value of offset bumped up to the next 16-byte boundary (if not already on one)
		static size_t AdvanceToBoundary(size_t offset) noexcept;
		// return true if a memory block crosses a boundary
		static bool CrossesBoundary(size_t offset, size_t size) noexcept;
		// advance an offset to next boundary if block crosses a boundary
		static size_t AdvanceIfCrossesBoundary(size_t offset, size_t size) noexcept;

		std::optional<size_t> offset;
		std::unique_ptr<ExtraDataBase> pExtraData;
		Type type = Empty;
	};
}

#endif // !DYNAMIC_UNIFORM_H

