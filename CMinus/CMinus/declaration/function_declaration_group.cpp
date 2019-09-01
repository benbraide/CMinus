#include "function_declaration_group.h"

cminus::declaration::function_group::function_group(std::string name, logic::naming::parent *parent)
	: function_group_base(name, parent){}

cminus::declaration::function_group::~function_group() = default;

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

std::shared_ptr<cminus::declaration::function_base> cminus::declaration::function_group::get_highest_ranked(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (list_.empty())
		return nullptr;

	std::shared_ptr<function_base> highest_ranked;
	int highest_rank_score = type::object::get_score_value(type::object::score_result_type::nil), current_rank_score;

	for (auto &item : list_){
		if (highest_rank_score < (current_rank_score = type::object::get_score_value(item.first->get_rank(runtime, args)))){
			highest_rank_score = current_rank_score;
			highest_ranked = item.second;
		}
	}

	return highest_ranked;
}

std::shared_ptr<cminus::memory::reference> cminus::declaration::function_group::call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	auto highest_ranked = get_highest_ranked(runtime, args);
	if (highest_ranked == nullptr)
		throw logic::exception("Function does not take the specified arguments", 0u, 0u);

	return highest_ranked->call_(runtime, context, args);
}
