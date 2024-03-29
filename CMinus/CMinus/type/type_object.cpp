#include "../node/memory_reference_node.h"
#include "../evaluator/evaluator_object.h"

#include "../logic/runtime.h"

cminus::type::object::~object() = default;

bool cminus::type::object::is_auto() const{
	return false;
}

bool cminus::type::object::converts_auto(const object &target) const{
	return target.is_auto();
}

void cminus::type::object::construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const{
	if (initialization != nullptr)
		get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, initialization);
	else if (auto default_value = get_default_value(runtime); default_value != nullptr)
		get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, std::make_shared<node::memory_reference>(nullptr, default_value));
	else//Zero memory
		target->set(runtime, static_cast<std::byte>(0), get_size());

	target->remove_attribute("#Init#", true);
}

void cminus::type::object::destruct(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const{}

std::size_t cminus::type::object::compute_base_offset(const object &target) const{
	return static_cast<std::size_t>(-1);
}

bool cminus::type::object::is_exact(logic::runtime &runtime, const object &target) const{
	return (&target == this || get_score(runtime, target, false) == score_result_type::exact);
}

int cminus::type::object::get_score_value(score_result_type score){
	switch (score){
	case score_result_type::exact:
		return 100;
	case score_result_type::assignable:
		return 90;
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
