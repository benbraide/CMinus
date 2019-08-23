#pragma once

#include <variant>

#include "../logic/runtime.h"

#include "operator_id.h"

namespace cminus::evaluator{
	class object{
	public:
		using operator_type = std::variant<operator_id, std::string>;
		using operand_type = std::variant<std::shared_ptr<memory::reference>, std::shared_ptr<node::object>>;

		virtual ~object();

		virtual std::shared_ptr<memory::reference> evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const = 0;

		virtual std::shared_ptr<memory::reference> evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const = 0;

		virtual std::shared_ptr<memory::reference> evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const = 0;

		static std::string convert_operator_to_string(const operator_type &value);

		static operator_id convert_compound_operator_to_non_compound(operator_id id);

		static bool operator_is_compound_assignment(operator_id id);

		static bool operator_is_arithmetic(operator_id id);

		static bool operator_is_integral(operator_id id);

		static bool operator_is_relational(operator_id id);

		static std::shared_ptr<memory::reference> convert_operand_to_memory_reference(logic::runtime &runtime, const operand_type &value);
	};

	class write_attribute_guard{
	public:
		write_attribute_guard(logic::runtime &runtime, std::shared_ptr<memory::reference> target, bool include_context);

		~write_attribute_guard();

	private:
		std::function<void()> callback_;
	};

	class read_attribute_guard{
	public:
		read_attribute_guard(logic::runtime &runtime, std::shared_ptr<memory::reference> target, bool include_context);

		~read_attribute_guard();

	private:
		std::function<void()> callback_;
	};
}
