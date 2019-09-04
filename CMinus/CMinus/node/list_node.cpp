#include "../logic/runtime.h"
#include "list_node.h"

cminus::node::list::list(object *parent, const std::vector<std::shared_ptr<object>> &value)
	: object(parent), value_(value){}

cminus::node::list::list(object *parent, std::vector<std::shared_ptr<object>> &&value)
	: object(parent), value_(std::move(value)){}

cminus::node::list::~list() = default;

const cminus::node::object::index_info &cminus::node::list::get_index() const{
	return value_[0]->get_index();
}

std::shared_ptr<cminus::node::object> cminus::node::list::clone() const{
	std::vector<std::shared_ptr<object>> value;
	value.reserve(value_.size());

	for (auto item : value_)
		value.push_back(item->clone());

	return std::make_shared<list>(nullptr, value);
}

void cminus::node::list::print(logic::runtime &runtime) const{
	auto is_first = true;
	for (auto item : value_){
		if (!is_first)
			runtime.writer.write_scalar(', ');
		else
			is_first = false;

		item->print(runtime);
	}
}

std::shared_ptr<cminus::memory::reference> cminus::node::list::evaluate(logic::runtime &runtime) const{
	std::shared_ptr<memory::reference> result;
	for (auto item : value_)
		result = item->evaluate(runtime);

	return result;
}

const std::vector<std::shared_ptr<cminus::node::object>> &cminus::node::list::get_value() const{
	return value_;
}
