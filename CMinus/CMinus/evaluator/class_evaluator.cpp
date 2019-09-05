#include "../type/class_type.h"

#include "class_evaluator.h"

cminus::evaluator::class_::~class_() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::class_::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	auto callable = find_operator_(runtime, op, target);
	if (callable == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	return callable->get_value()->call(runtime, callable->get_context(), std::vector<std::shared_ptr<memory::reference>>{target});
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::class_::evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	auto callable = find_operator_(runtime, op, target);
	if (callable == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	std::vector<std::shared_ptr<memory::reference>> args{
		target,
		runtime.global_storage->create_scalar(0)
	};

	return callable->get_value()->call(runtime, callable->get_context(), args);
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::class_::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	auto callable = find_operator_(runtime, op, left_value);
	if (callable == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	return callable->get_value()->call(runtime, callable->get_context(), std::vector<std::shared_ptr<memory::reference>>{ left_value, right->evaluate(runtime) });
}

cminus::memory::function_reference *cminus::evaluator::class_::find_operator_(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	std::shared_ptr<memory::reference> entry;
	if (std::holds_alternative<operator_id>(op)){
		entry = dynamic_cast<type::class_ *>(target->get_type().get())->find_operator(
			runtime,
			std::get<operator_id>(op),
			logic::storage::object::namesless_search_options{ nullptr, target, false }
		);
	}
	else//String value
		entry = dynamic_cast<type::class_ *>(target->get_type().get())->find(runtime, logic::storage::object::search_options{ nullptr, target, std::get<std::string>(op), false });

	if (entry == nullptr)
		return nullptr;

	if (auto callable = dynamic_cast<memory::function_reference *>(entry->get_non_raw()); callable != nullptr)
		return callable;

	throw logic::exception("Bad operator definition", 0u, 0u);
	return nullptr;
}
