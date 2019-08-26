#include "specialized_storage.h"

cminus::logic::storage::specialized::specialized(const std::string &name, object *parent)
	: object(name, parent){}

cminus::logic::storage::specialized::~specialized() = default;

void cminus::logic::storage::specialized::raise_interrupt(interrupt_type type, std::shared_ptr<memory::reference> value){
	if (interrupt_is_valid_(type)){
		interrupt_value_ = value;
		throw type;
	}

	invalid_interrupt_(type, value);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::specialized::get_raised_interrupt_value() const{
	return interrupt_value_;
}

void cminus::logic::storage::specialized::invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value){
	switch (type){
	case interrupt_type::return_:
		throw logic::exception("A 'return' statement requires a function scope", 0u, 0u);
		break;
	case interrupt_type::break_:
		throw logic::exception("A 'return' statement requires a loop or switch scope", 0u, 0u);
		break;
	case interrupt_type::continue_:
		throw logic::exception("A 'return' statement requires a loop scope", 0u, 0u);
		break;
	default:
		break;
	}
}

cminus::logic::storage::double_layer::double_layer(const std::string &name, object *parent)
	: specialized(name, parent){}

cminus::logic::storage::double_layer::~double_layer() = default;

void cminus::logic::storage::double_layer::add(const std::string &name, std::shared_ptr<memory::reference> entry){
	if (inner_layer_ != nullptr)
		inner_layer_->add(name, entry);
	else
		specialized::add(name, entry);
}

void cminus::logic::storage::double_layer::remove(const std::string &name){
	if (inner_layer_ != nullptr)
		inner_layer_->remove(name);
	specialized::remove(name);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::double_layer::find(const std::string &name, bool search_tree, const object **branch) const{
	if (auto entry = ((inner_layer_ == nullptr) ? nullptr : inner_layer_->find(name, false, branch)); entry != nullptr){
		if (branch != nullptr)
			*branch = this;
		return entry;
	}

	return specialized::find(name, search_tree, branch);
}

void cminus::logic::storage::double_layer::refresh(){
	inner_layer_ = std::make_shared<object>("", this);
}

void cminus::logic::storage::double_layer::invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value){
	for (auto ancestor = parent_; ancestor != nullptr; ancestor = ancestor->get_naming_parent()){//Get next specialized ancestor
		if (auto specialized_ancestor = dynamic_cast<specialized *>(ancestor); specialized_ancestor != nullptr){
			specialized_ancestor->raise_interrupt(type, value);
			return;
		}
	}

	specialized::invalid_interrupt_(type, value);
}

cminus::logic::storage::function::function(std::shared_ptr<memory::reference> context, object *parent)
	: specialized("", ((context_ == nullptr) ? parent : dynamic_cast<object *>(context_->get_type().get()))), context_(context){}

cminus::logic::storage::function::~function() = default;

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::function::find(const std::string &name, bool search_tree, const object **branch) const{
	const object *my_branch = nullptr;

	auto entry = specialized::find(name, search_tree, &my_branch);
	if (entry == nullptr)//Entry not found
		return nullptr;

	if (branch != nullptr)//Update branch
		*branch = my_branch;

	if (my_branch != parent_ || entry->find_attribute("Static", true, false) != nullptr)//Branch is not context's type OR entry is static
		return entry;

	if (context_ == nullptr)//Entry must be static
		throw logic::exception("A non-static member requires an object context", 0u, 0u);

	return std::make_shared<memory::hard_reference>(entry, context_, context_->get_address());
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::function::get_context() const{
	return context_;
}

void cminus::logic::storage::function::add_unnamed(std::shared_ptr<memory::reference> entry){
	unnamed_entries_[entry.get()] = entry;
}

bool cminus::logic::storage::function::interrupt_is_valid_(interrupt_type value) const{
	return (value == interrupt_type::return_);
}

cminus::logic::storage::loop::loop(object *parent)
	: double_layer("", parent){}

cminus::logic::storage::loop::~loop() = default;

bool cminus::logic::storage::loop::interrupt_is_valid_(interrupt_type value) const{
	return (value == interrupt_type::break_ || value == interrupt_type::continue_);
}

cminus::logic::storage::switch_::switch_(object *parent)
	: double_layer("", parent){}

cminus::logic::storage::switch_::~switch_() = default;

bool cminus::logic::storage::switch_::interrupt_is_valid_(interrupt_type value) const{
	return (value == interrupt_type::break_);
}

cminus::logic::storage::selection::selection(object *parent)
	: double_layer("", parent){}

cminus::logic::storage::selection::~selection() = default;

bool cminus::logic::storage::selection::interrupt_is_valid_(interrupt_type value) const{
	return false;
}
