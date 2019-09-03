#include "function_declaration_group.h"

cminus::declaration::function_group::function_group(std::string name, logic::naming::parent *parent)
	: function_group_base(name, parent){}

cminus::declaration::function_group::~function_group() = default;

const cminus::logic::attributes::collection &cminus::declaration::function_group::get_attributes() const{
	return (list_.empty() ? attributes_ : list_.begin()->first->get_attributes());
}

void cminus::declaration::function_group::add(std::shared_ptr<function_base> value){
	list_[value.get()] = value;
}

void cminus::declaration::function_group::replace(function_base &existing_entry, std::shared_ptr<function_base> new_entry){
	if (auto it = list_.find(&existing_entry); it != list_.end()){
		list_.erase(it);
		list_[new_entry.get()] = new_entry;
	}
}

std::shared_ptr<cminus::declaration::function_base> cminus::declaration::function_group::find(logic::runtime &runtime, const function_base &tmpl) const{
	if (list_.empty())
		return nullptr;

	for (auto &item : list_){
		if (item.first->is_exact(runtime, tmpl))
			return item.second;
	}

	return nullptr;
}

std::shared_ptr<cminus::declaration::function_base> cminus::declaration::function_group::get_highest_ranked(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (list_.empty())
		return nullptr;

	auto is_const = (context != nullptr && context->has_attribute("ReadOnly", true, true)), is_const_ctx = false;
	int highest_rank_score = type::object::get_score_value(type::object::score_result_type::nil), current_rank_score;

	std::shared_ptr<function_base> highest_ranked;
	for (auto &item : list_){
		if (is_const && !(is_const_ctx = item.first->get_attributes().has("ReadOnlyContext", true)))
			continue;

		if (highest_rank_score < (current_rank_score = (type::object::get_score_value(item.first->get_rank(runtime, args)) - ((is_const_ctx == is_const) ? 0 : 1)))){
			highest_rank_score = current_rank_score;
			highest_ranked = item.second;
		}
	}

	return highest_ranked;
}

std::shared_ptr<cminus::memory::reference> cminus::declaration::function_group::call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	auto highest_ranked = get_highest_ranked(runtime, context, args);
	if (highest_ranked == nullptr)
		throw logic::exception("Function does not take the specified arguments", 0u, 0u);

	return highest_ranked->call_(runtime, context, args);
}
