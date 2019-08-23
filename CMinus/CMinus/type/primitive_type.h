#pragma once

#include "../memory/memory_object.h"

#include "type_object.h"

namespace cminus::type{
	class primitive : public named_object{
	public:
		enum class id_type : char{
			nil,
			nullptr_,
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
			nan_,
		};

		explicit primitive(id_type id);

		virtual ~primitive();

		virtual std::size_t get_size() const override;

		virtual std::size_t compute_base_offset(const type::object &target) const override;

		virtual score_result_type get_score(const type::object &target, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> get_default_value(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, bool is_ref) const override;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, const std::byte *data, std::shared_ptr<type::object> target_type) const override;

		virtual bool is_same(const logic::naming::object &target) const override;

		virtual id_type get_id() const;

		virtual bool is_numeric() const;

		virtual bool is_integral() const;

		virtual bool is_unsigned_integral() const;

		virtual bool is_floating_point() const;

		static id_type convert_string_to_id(const std::string &value);

		static std::string convert_id_to_string(id_type value);

	protected:
		template <typename target_type>
		static target_type read_source_(memory::object &memory_object, std::size_t source_address){
			return memory_object.read_scalar<target_type>(source_address);
		}

		template <typename target_type>
		static target_type read_source_(memory::object &memory_object, const std::byte *source){
			auto target = target_type();
			memcpy(&target, source, sizeof(target_type));
			return target;
		}

		template <typename target_type, typename target_source_type>
		static target_type convert_source_(memory::object &memory_object, target_source_type source, const primitive &source_type){
			switch (source_type.id_){
			case id_type::int8_:
				return (target_type)read_source_<__int8>(memory_object, source);
			case id_type::uint8_:
				return (target_type)read_source_<unsigned __int8>(memory_object, source);
			case id_type::int16_:
				return (target_type)read_source_<__int16>(memory_object, source);
			case id_type::uint16_:
				return (target_type)read_source_<unsigned __int16>(memory_object, source);
			case id_type::int32_:
				return (target_type)read_source_<__int32>(memory_object, source);
			case id_type::uint32_:
				return (target_type)read_source_<unsigned __int32>(memory_object, source);
			case id_type::int64_:
				return (target_type)read_source_<__int64>(memory_object, source);
			case id_type::uint64_:
				return (target_type)read_source_<unsigned __int64>(memory_object, source);
			case id_type::float_:
				return (target_type)read_source_<float>(memory_object, source);
			case id_type::double_:
				return (target_type)read_source_<double>(memory_object, source);
			case id_type::ldouble:
				return (target_type)read_source_<long double>(memory_object, source);
			default:
				throw memory::exception(memory::error_code::incompatible_types, 0u);
				break;
			}

			return target_type();
		}

		id_type id_;
	};
}
