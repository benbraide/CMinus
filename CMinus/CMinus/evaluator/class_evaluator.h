#pragma once

#include "comparison_evaluator.h"

namespace cminus::evaluator{
	class class_ : public object, public explicit_comparison{
	public:
		virtual ~class_();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const override;

	protected:
		virtual memory::function_reference *find_operator_(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const;
	};
}
