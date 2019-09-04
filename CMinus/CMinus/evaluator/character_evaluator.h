#pragma once

#include <bitset>

#include "boolean_evaluator.h"

namespace cminus::evaluator{
	class character : public boolean, public compound_assignment{
	public:
		using constant_type = node::named_constant::constant_type;

		virtual ~character();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const override;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const override;
	};
}
