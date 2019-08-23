#pragma once

#include "evaluator_object.h"

namespace cminus::evaluator{
	class explicit_comparison{
	public:
		virtual ~explicit_comparison();

	protected:
		virtual std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const;
	};

	class equality_comparison{
	public:
		virtual ~equality_comparison();

	protected:
		virtual std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const;
	};
}
