#pragma once

#include "assignment_evaluator.h"
#include "comparison_evaluator.h"

namespace cminus::evaluator{
	class string : public object, public assignment, public compound_assignment, public explicit_comparison{
	public:
		using constant_type = node::named_constant::constant_type;

		virtual ~string();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const override;

	protected:
		virtual int compare_strings_(const char *left, const char *right, std::size_t left_size, std::size_t right_size) const;
	};
}
