#pragma once

#include "evaluator_object.h"

namespace cminus::evaluator{
	class arithmetic{
	public:
		using constant_type = node::named_constant::constant_type;

		virtual ~arithmetic() = default;;

	protected:
		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::minus:
				return std::make_shared<memory::reference_with_value<value_type>>(value->get_type(), nullptr, static_cast<value_type>(-value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			return evaluate_unsigned_unary_left_<value_type>(runtime, op, value);
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unsigned_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::increment:
				value->write_scalar(runtime, static_cast<value_type>(value->read_scalar<value_type>(runtime) + static_cast<value_type>(1)));
				return value;
			case operator_id::decrement:
				value->write_scalar(runtime, static_cast<value_type>(value->read_scalar<value_type>(runtime) - static_cast<value_type>(1)));
				return value;
			case operator_id::plus:
				return std::make_shared<memory::reference_with_value<value_type>>(value->get_type(), nullptr, value->read_scalar<value_type>(runtime));
			default:
				break;
			}

			return nullptr;
		}

		std::shared_ptr<memory::reference> evaluate_nan_unary_left_(logic::runtime &runtime, operator_id op) const{
			switch (op){
			case operator_id::minus:
				return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
			default:
				break;
			}

			return evaluate_unsigned_nan_unary_left_(runtime, op);
		}

		std::shared_ptr<memory::reference> evaluate_unsigned_nan_unary_left_(logic::runtime &runtime, operator_id op) const{
			switch (op){
			case operator_id::plus:
				return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
			default:
				break;
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unsigned_unary_right_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			value_type computed_value;
			switch (op){
			case operator_id::increment:
				computed_value = static_cast<value_type>(value->read_scalar<value_type>(runtime) + static_cast<value_type>(1));
				value->write_scalar(runtime, computed_value);
				return std::make_shared<memory::reference_with_value<value_type>>(value->get_type(), nullptr, computed_value);
			case operator_id::decrement:
				computed_value = static_cast<value_type>(value->read_scalar<value_type>(runtime) - static_cast<value_type>(1));
				value->write_scalar(runtime, computed_value);
				return std::make_shared<memory::reference_with_value<value_type>>(value->get_type(), nullptr, computed_value);
			default:
				break;
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_binary_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::plus:
				return std::make_shared<memory::reference_with_value<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) + right_value->read_scalar<value_type>(runtime)));
			case operator_id::minus:
				return std::make_shared<memory::reference_with_value<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) - right_value->read_scalar<value_type>(runtime)));
			case operator_id::times:
				return std::make_shared<memory::reference_with_value<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) * right_value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			if (op == operator_id::divide){
				auto right_scalar_value = right_value->read_scalar<value_type>(runtime);
				if (right_scalar_value == static_cast<value_type>(0))//NaN
					return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
				return std::make_shared<memory::reference_with_value<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) / right_value->read_scalar<value_type>(runtime)));
			}

			return nullptr;
		}

		std::shared_ptr<memory::reference> evaluate_nan_(logic::runtime &runtime, operator_id op) const{
			switch (op){
			case operator_id::plus:
			case operator_id::minus:
			case operator_id::times:
			case operator_id::divide:
				return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
			default:
				break;
			}

			return nullptr;
		}

		std::shared_ptr<memory::reference> compare_nan_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value) const{
			switch (op){
			case operator_id::less:
			case operator_id::more:
				return runtime.global_storage->get_named_constant(constant_type::false_);
			case operator_id::inverse_equality:
				return runtime.global_storage->get_named_constant(left_value->is_nan() ? constant_type::false_ : constant_type::true_);
			case operator_id::less_or_equal:
			case operator_id::equality:
			case operator_id::more_or_equal:
				return runtime.global_storage->get_named_constant(left_value->is_nan() ? constant_type::true_ : constant_type::false_);
			default:
				break;
			}

			return nullptr;
		}
	};
}
