#pragma once

#include "assignment_evaluator.h"
#include "comparison_evaluator.h"

namespace cminus::evaluator{
	class boolean : public object, public assignment, public explicit_comparison, public equality_comparison{
	public:
		using constant_type = node::named_constant::constant_type;

		virtual ~boolean();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const override;

	protected:
		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::less:
				return runtime.global_storage->get_named_constant((left_value->read_scalar<value_type>(runtime) < right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
			case operator_id::less_or_equal:
				return runtime.global_storage->get_named_constant((left_value->read_scalar<value_type>(runtime) <= right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
			case operator_id::equality:
				return runtime.global_storage->get_named_constant((left_value->read_scalar<value_type>(runtime) == right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
			case operator_id::inverse_equality:
				return runtime.global_storage->get_named_constant((left_value->read_scalar<value_type>(runtime) != right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
			case operator_id::more_or_equal:
				return runtime.global_storage->get_named_constant((left_value->read_scalar<value_type>(runtime) >= right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
			case operator_id::more:
				return runtime.global_storage->get_named_constant((left_value->read_scalar<value_type>(runtime) > right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
			default:
				break;
			}

			return nullptr;
		}
	};
}
