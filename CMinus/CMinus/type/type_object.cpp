#include "../evaluator/evaluator_object.h"

#include "../logic/runtime.h"

cminus::type::object::~object() = default;

void cminus::type::object::construct_default(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const{
	if (auto default_value = get_default_value(runtime); default_value != nullptr){
		target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
		get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, default_value);
	}
	else//Zero memory
		target->set(runtime, static_cast<std::byte>(0), get_size());
}

void cminus::type::object::construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const{
	if (initialization != nullptr){
		if (auto value = initialization->evaluate(runtime); value != nullptr){
			target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
			get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, value);
		}
		else
			throw logic::exception("Failed to evaluate initialization", 0u, 0u);
	}
	else
		construct_default(runtime, target);
}

int cminus::type::object::get_score_value(score_result_type score){
	switch (score){
	case score_result_type::exact:
		return 100;
	case score_result_type::ancestor:
	case score_result_type::widened:
		return 80;
	case score_result_type::too_widened:
		return 70;
	case score_result_type::shortened:
		return 60;
	case score_result_type::too_shortened:
		return 50;
	case score_result_type::compatible:
		return 30;
	case score_result_type::class_compatible:
		return 20;
	default:
		break;
	}

	return 0;
}

cminus::type::named_object::named_object(const std::string &value, logic::naming::parent *parent)
	: single(value, parent){}

cminus::type::named_object::~named_object() = default;

void cminus::type::named_object::print(logic::runtime &runtime, bool is_qualified) const{
	if (parent_ == nullptr)
		runtime.writer.write_buffer(value_.data(), value_.size());
	else//Default
		single::print(runtime, is_qualified);
}

std::string cminus::type::named_object::get_qualified_naming_value() const{
	return ((parent_ == nullptr) ? value_ : single::get_qualified_naming_value());
}
