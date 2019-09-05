#include "../logic/runtime.h"

#include "memory_reference_node.h"

cminus::node::memory_reference::memory_reference(object *parent, const index_info &index, std::shared_ptr<cminus::memory::reference> value)
	: object_with_index(parent, index), value_(value){}

cminus::node::memory_reference::memory_reference(object *parent, std::shared_ptr<cminus::memory::reference> value)
	: object_with_index(parent, index_info{}), value_(value){}

cminus::node::memory_reference::~memory_reference() = default;

std::shared_ptr<cminus::node::object> cminus::node::memory_reference::clone() const{
	return std::make_shared<memory_reference>(nullptr, value_);
}

void cminus::node::memory_reference::print(logic::runtime &runtime) const{
	value_->get_type()->print_value(runtime, value_);
}

std::shared_ptr<cminus::memory::reference> cminus::node::memory_reference::evaluate(logic::runtime &runtime) const{
	return value_;
}

std::shared_ptr<cminus::memory::reference> cminus::node::memory_reference::get_value() const{
	return value_;
}
