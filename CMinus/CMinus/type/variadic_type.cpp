#include "variadic_type.h"

cminus::type::variadic::variadic(std::shared_ptr<object> base_type)
	: base_type_(base_type){}

cminus::type::variadic::~variadic() = default;

void cminus::type::variadic::print(logic::runtime &runtime, bool is_qualified) const{
	base_type_->print(runtime, is_qualified);
	runtime.writer.write_buffer("...", 3u);
}

void cminus::type::variadic::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{
	auto variadic_data = dynamic_cast<memory::reference_with_value<std::vector<std::shared_ptr<memory::reference>>> *>(data.get());
	if (variadic_data == nullptr)
		return;

	runtime.writer.write_scalar('[');
	auto is_first = true;

	for (auto entry : variadic_data->get_value()){
		if (!is_first)
			runtime.writer.write_scalar(', ');
		else
			is_first = false;

		entry->get_type()->print_value(runtime, entry);
	}

	runtime.writer.write_scalar(']');
}

std::size_t cminus::type::variadic::get_size() const{
	return 0u;
}

std::size_t cminus::type::variadic::compute_base_offset(const object &target) const{
	return 0u;
}

cminus::type::object::score_result_type cminus::type::variadic::get_score(logic::runtime &runtime, const object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const variadic *>(&target);
	if (type_target == nullptr)
		return score_result_type::nil;

	return base_type_->get_score(runtime, *type_target->base_type_, is_ref);
}

std::shared_ptr<cminus::memory::reference> cminus::type::variadic::get_default_value(logic::runtime &runtime) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::variadic::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const{
	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::type::variadic::get_evaluator(logic::runtime &runtime) const{
	return nullptr;
}

std::shared_ptr<cminus::type::object> cminus::type::variadic::get_base_type() const{
	return base_type_;
}
