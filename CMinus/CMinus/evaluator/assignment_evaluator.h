#pragma once

#include "evaluator_object.h"

namespace cminus::evaluator{
	class assignment{
	public:
		virtual ~assignment();

	protected:
		virtual std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const;

		virtual void after_value_copy_(logic::runtime &runtime, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const;
	};

	class compound_assignment{
	public:
		virtual ~compound_assignment();

	protected:
		virtual std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const;
	};
}
