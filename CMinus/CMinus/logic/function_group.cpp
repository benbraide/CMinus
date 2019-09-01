#include "function_group.h"

cminus::logic::function_group::function_group(std::string name, naming::parent *parent)
	: single(name, parent){}

cminus::logic::function_group::~function_group() = default;

void cminus::logic::function_group::add(std::shared_ptr<function_object> value){
	list_[value.get()] = value;
}

void cminus::logic::function_group::replace(function_object &existing_entry, std::shared_ptr<function_object> new_entry){
	if (auto it = list_.find(&existing_entry); it != list_.end()){
		list_.erase(it);
		list_[new_entry.get()] = new_entry;
	}
}

std::shared_ptr<cminus::logic::function_object> cminus::logic::function_group::find(logic::runtime &runtime, const type::object &type) const{
	if (list_.empty())
		return nullptr;

	for (auto &item : list_){
		if (item.first->get_computed_type()->is_exact(runtime, type))
			return item.second;
	}

	return nullptr;
}

std::shared_ptr<cminus::logic::function_object> cminus::logic::function_group::get_highest_ranked(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (list_.empty())
		return nullptr;

	std::shared_ptr<function_object> highest_ranked;
	int highest_rank_score = type::object::get_score_value(type::object::score_result_type::nil), current_rank_score;

	for (auto &item : list_){
		if (highest_rank_score < (current_rank_score = type::object::get_score_value(item.first->get_rank(runtime, args)))){
			highest_rank_score = current_rank_score;
			highest_ranked = item.second;
		}
	}

	return highest_ranked;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::function_group::call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	auto highest_ranked = get_highest_ranked(runtime, args);
	if (highest_ranked == nullptr)
		throw exception("Function does not take the specified arguments", 0u, 0u);

	return highest_ranked->call_(runtime, context, args);
}
