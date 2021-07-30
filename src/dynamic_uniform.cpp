#include "dynamic_uniform.h"

namespace Dcf {
	struct ExtraData
	{
		struct Struct : public LayoutElement::ExtraDataBase
		{
			std::vector<std::pair<std::string, LayoutElement>> layoutElements;
		};
		struct Array : public LayoutElement::ExtraDataBase
		{
			std::optional<LayoutElement> layoutElement;
			size_t size;
		};
	};

	CookedLayout LayoutCodex::Resolve(RawLayout&& layout) 
	{
		auto sig = layout.GetSignature();
		auto& map = Get_().map;
		const auto i = map.find(sig);
		// idential layout already exists
		if (i != map.end())
		{
			// input layout is expected to be cleared after Resolve
			// so just throw away the layout tree
			layout.ClearRoot();
			return { i->second };
		}
		// otherwise add layout root element to map
		auto result = map.insert({ std::move(sig),layout.DeliverRoot() });
		// return layout with additional reference to root
		return { result.first->second };
	}

	LayoutCodex& LayoutCodex::Get_() noexcept
	{
		static LayoutCodex codex;
		return codex;
	}



	Layout::Layout(std::shared_ptr<LayoutElement> pRoot) noexcept
		:
		pRoot{ std::move(pRoot) }
	{}
	size_t Layout::GetSizeInBytes() const noexcept
	{
		return pRoot->GetSizeInBytes();
	}
	std::string Layout::GetSignature() const
	{
		return pRoot->GetSignature();
	}



	RawLayout::RawLayout() noexcept
		:
		Layout{ std::shared_ptr<LayoutElement>{ new LayoutElement(Struct) } }
	{}

	std::shared_ptr<LayoutElement> RawLayout::DeliverRoot() noexcept
	{
		auto temp = std::move(pRoot);
		temp->Finalize(0);
		*this = RawLayout();
		return std::move(temp);
	}
	void RawLayout::ClearRoot() noexcept
	{
		*this = RawLayout();
	}




	LayoutElement::LayoutElement(Type typeIn) 
		:
	type{ typeIn }
	{
		assert(typeIn != Empty);
		if (typeIn == Struct)
		{
			pExtraData = std::unique_ptr<ExtraData::Struct>{ new ExtraData::Struct() };
		}
		else if (typeIn == Array)
		{
			pExtraData = std::unique_ptr<ExtraData::Array>{ new ExtraData::Array() };
		}
	}

	std::string LayoutElement::GetSignature() const
	{
		switch (type)
		{
#define X(el) case el: return Map<el>::code;
			LEAF_ELEMENT_TYPES
#undef X
		case Struct:
			return GetSignatureForStruct();
		default:
			assert("Bad type in signature generation" && false);
			return "???";
		}
	}

	LayoutElement& LayoutElement::Add(Type addedType, std::string name) 
	{

		auto& structData = static_cast<ExtraData::Struct&>(*pExtraData);
		for (auto& mem : structData.layoutElements)
		{
			if (mem.first == name)
			{
				assert("Adding duplicate name to struct" && false);
			}
		}
		structData.layoutElements.emplace_back(std::move(name), LayoutElement{ addedType });
		return *this;
	}

	LayoutElement& LayoutElement::operator[](const std::string& key)
	{
		assert("Keying into non-struct" && type == Struct);
		for (auto& mem : static_cast<ExtraData::Struct&>(*pExtraData).layoutElements)
		{
			if (mem.first == key)
			{
				return mem.second;
			}
		}
		return GetEmptyElement();
	}

	const LayoutElement& LayoutElement::operator[](const std::string& key) const
	{
		return const_cast<LayoutElement&>(*this)[key];
	}

	std::string LayoutElement::GetSignature() const 
	{
		switch (type)
		{
#define X(el) case el: return Map<el>::code;
			LEAF_ELEMENT_TYPES
#undef X
		case Struct:
			return GetSignatureForStruct();
		default:
			assert("Bad type in signature generation" && false);
			return "???";
		}
	}

	std::string LayoutElement::GetSignatureForStruct() const
	{
		using namespace std::string_literals;
		auto sig = "St{"s;
		for (const auto& el : static_cast<ExtraData::Struct&>(*pExtraData).layoutElements)
		{
			sig += el.first + ":"s + el.second.GetSignature() + ";"s;
		}
		sig += "}"s;
		return sig;
	}

	size_t LayoutElement::Finalize(size_t offsetIn)
	{
		switch (type)
		{
#define X(el) case el: offset = AdvanceIfCrossesBoundary( offsetIn,Map<el>::glslSize ); return *offset + Map<el>::glslSize;
			LEAF_ELEMENT_TYPES
#undef X
		case Struct:
			return FinalizeForStruct(offsetIn);
		default:
			assert("Bad type in size computation" && false);
			return 0u;
		}
	}

	size_t LayoutElement::FinalizeForStruct(size_t offsetIn)
	{
		auto& data = static_cast<ExtraData::Struct&>(*pExtraData);
		assert(data.layoutElements.size() != 0u);
		offset = AdvanceToBoundary(offsetIn);
		auto offsetNext = *offset;
		for (auto& el : data.layoutElements)
		{
			offsetNext = el.second.Finalize(offsetNext);
		}
		return offsetNext;
	}

	bool LayoutElement::CrossesBoundary(size_t offset, size_t size) noexcept
	{
		const auto end = offset + size;
		const auto pageStart = offset / 16u;
		const auto pageEnd = end / 16u;
		return (pageStart != pageEnd && end % 16 != 0u) || size > 16u;
	}
	size_t LayoutElement::AdvanceIfCrossesBoundary(size_t offset, size_t size) noexcept
	{
		return CrossesBoundary(offset, size) ? AdvanceToBoundary(offset) : offset;
	}
	size_t LayoutElement::AdvanceToBoundary(size_t offset) noexcept
	{
		return offset + (16u - offset % 16u) % 16u;
	}

	size_t LayoutElement::GetSizeInBytes() const 
	{
		return GetOffsetEnd() - GetOffsetBegin();
	}

	size_t LayoutElement::GetOffsetBegin() const 
	{
		return *offset;
	}

	size_t LayoutElement::GetOffsetEnd() const 
	{
		switch (type)
		{
#define X(el) case el: return *offset + Map<el>::glslSize;
			LEAF_ELEMENT_TYPES
#undef X
		case Struct:
			{
				const auto& data = static_cast<ExtraData::Struct&>(*pExtraData);
				return AdvanceToBoundary(data.layoutElements.back().second.GetOffsetEnd());
			}
		case Array:
		{
			const auto& data = static_cast<ExtraData::Array&>(*pExtraData);
			return *offset + AdvanceToBoundary(data.layoutElement->GetSizeInBytes()) * data.size;
		}
		default:
			assert("Tried to get offset of empty or invalid element" && false);
			return 0u;
		}
	}


	std::shared_ptr<LayoutElement> CookedLayout::ShareRoot() const noexcept
	{
		return pRoot;
	}


	Buffer::Buffer(RawLayout&& lay)
		:
	Buffer(LayoutCodex::Resolve(std::move(lay)))
	{}

}