#pragma once

#include "evaluator_object.h"

namespace cminus::evaluator{
	class assignment{
	public:
		virtual ~assignment();

	protected:
		virtual std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const;

		virtual std::shared_ptr<memory::reference> assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> destination, std::shared_ptr<memory::reference> source) const;
	};

	class compound_assignment{
	public:
		virtual ~compound_assignment();

	protected:
		virtual std::shared_ptr<memory::reference> evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const;
	};
}
