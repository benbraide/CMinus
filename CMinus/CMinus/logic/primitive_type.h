#pragma once

#include "../memory/memory_reference.h"

namespace cminus::logic::type{
	class primitive : public type::object, public naming::single{
	public:
		using type_base_type = type::object;
		using naming_base_type = naming::single;

		enum class id_type : char{
			nil,
			bool_,
			byte_,
			char_,
			wchar_,
			int8_,
			uint8_,
			int16_,
			uint16_,
			int32_,
			uint32_,
			int64_,
			uint64_,
			int128_,
			uint128_,
			float_,
			double_,
			ldouble,
		};

		explicit primitive(id_type id);

		virtual ~primitive();

		virtual std::size_t get_size() const override;

		virtual score_result_type get_score(const type::object &target) const override;

		virtual std::shared_ptr<memory::reference> convert_value(memory::object &memory_object, std::size_t address, std::shared_ptr<type::object> target_type) const override;

		virtual std::string get_qualified_naming_value() const override;

		virtual void print(io::writer &writer, bool is_qualified) const override;

		virtual bool is_same(const naming::object &target) const override;

		virtual id_type get_id() const;

		virtual bool is_integer() const;

		virtual bool is_floating_point() const;

		static id_type convert_string_to_id(const std::string &value);

		static std::string convert_id_to_string(id_type value);

	protected:
		template <typename target_type>
		static target_type read_source_(memory::block &source){
			return source.read_scalar<target_type>(0u);
		}

		template <typename target_type>
		static target_type convert_source_(memory::block &source, const primitive &source_type){
			switch (source_type.id_){
			case id_type::int8_:
				return (target_type)read_source_<__int8>(source);
			case id_type::uint8_:
				return (target_type)read_source_<unsigned __int8>(source);
			case id_type::int16_:
				return (target_type)read_source_<__int16>(source);
			case id_type::uint16_:
				return (target_type)read_source_<unsigned __int16>(source);
			case id_type::int32_:
				return (target_type)read_source_<__int32>(source);
			case id_type::uint32_:
				return (target_type)read_source_<unsigned __int32>(source);
			case id_type::int64_:
				return (target_type)read_source_<__int64>(source);
			case id_type::uint64_:
				return (target_type)read_source_<unsigned __int64>(source);
			case id_type::float_:
				return (target_type)read_source_<float>(source);
			case id_type::double_:
				return (target_type)read_source_<double>(source);
			case id_type::ldouble:
				return (target_type)read_source_<long double>(source);
			default:
				throw memory::exception(memory::error_code::incompatible_types, source.get_address());
				break;
			}

			return target_type();
		}

		id_type id_;
	};
}
