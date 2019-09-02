#pragma once

#include "boolean_evaluator.h"
#include "arithmetic_evaluator.h"

namespace cminus::evaluator{
	class floating_point : public object, public assignment, public compound_assignment, public explicit_comparison, public arithmetic{
	public:
		virtual ~floating_point();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const override;

	protected:
		virtual void after_value_copy_(logic::runtime &runtime, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const override;

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_binary_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			if (auto result = arithmetic::evaluate_binary_<value_type>(runtime, op, left_value, right_value); result != nullptr)
				return result;

			return arithmetic::compare_<value_type>(runtime, op, left_value, right_value);
		}
	};
}
