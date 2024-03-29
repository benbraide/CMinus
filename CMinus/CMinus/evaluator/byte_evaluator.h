#pragma once

#include <bitset>

#include "boolean_evaluator.h"

namespace cminus::evaluator{
	class byte : public boolean, public compound_assignment{
	public:
		using constant_type = node::named_constant::constant_type;

		virtual ~byte();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const override;

	protected:
		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::bitwise_inverse:
				return runtime.global_storage->create_scalar(static_cast<value_type>(~value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_binary_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::bitwise_and:
				return runtime.global_storage->create_scalar(static_cast<value_type>(left_value->read_scalar<value_type>(runtime) & right_value->read_scalar<value_type>(runtime)));
			case operator_id::bitwise_or:
				return runtime.global_storage->create_scalar(static_cast<value_type>(left_value->read_scalar<value_type>(runtime) | right_value->read_scalar<value_type>(runtime)));
			case operator_id::bitwise_xor:
				return runtime.global_storage->create_scalar(static_cast<value_type>(left_value->read_scalar<value_type>(runtime) ^ right_value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_shift_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			if (right_value->is_nan())
				return nullptr;

			auto right_type = right_value->get_type();
			if (auto right_primitive_type = dynamic_cast<type::primitive *>(right_type.get()); right_primitive_type != nullptr){
				switch (right_primitive_type->get_id()){
				case type::primitive::id_type::int16_:
					return evaluate_shift_<value_type, __int16>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint16_:
					return evaluate_shift_<value_type, unsigned __int16>(runtime, op, left_value, right_value);
				case type::primitive::id_type::int32_:
					return evaluate_shift_<value_type, __int32>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint32_:
					return evaluate_shift_<value_type, unsigned __int32>(runtime, op, left_value, right_value);
				case type::primitive::id_type::int64_:
					return evaluate_shift_<value_type, __int64>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint64_:
					return evaluate_shift_<value_type, unsigned __int64>(runtime, op, left_value, right_value);
				default:
					break;
				}
			}

			return nullptr;
		}

		template <typename value_type, typename shift_type>
		std::shared_ptr<memory::reference> evaluate_shift_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::left_shift:
				return runtime.global_storage->create_scalar(static_cast<value_type>(left_value->read_scalar<value_type>(runtime) << right_value->read_scalar<shift_type>(runtime)));
			case operator_id::right_shift:
				return runtime.global_storage->create_scalar(static_cast<value_type>(left_value->read_scalar<value_type>(runtime) >> right_value->read_scalar<shift_type>(runtime)));
			default:
				break;
			}

			if (op == operator_id::index){//Get bit
				auto index = static_cast<std::size_t>(right_value->read_scalar<shift_type>(runtime));
				if (8u <= index)//Out of bounds
					return runtime.global_storage->get_named_constant(node::named_constant::constant_type::false_);

				auto value = left_value->read_scalar<value_type>(runtime);
				if (std::bitset<8u>(static_cast<std::size_t>(value)).test(index))
					return runtime.global_storage->get_named_constant(node::named_constant::constant_type::true_);

				return runtime.global_storage->get_named_constant(node::named_constant::constant_type::false_);
			}

			return nullptr;
		}
	};
}
