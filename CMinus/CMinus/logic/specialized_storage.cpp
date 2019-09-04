#include "../type/class_type.h"

#include "../declaration/function_declaration_group.h"

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

cminus::logic::storage::double_layer::double_layer(const std::string &name, object *parent)
	: specialized(name, parent){}

cminus::logic::storage::double_layer::~double_layer() = default;

void cminus::logic::storage::double_layer::add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry){
	if (inner_layer_ == nullptr)
		specialized::add(runtime, name, entry);
	else//Use inner layer
		inner_layer_->add(runtime, name, entry);
}

void cminus::logic::storage::double_layer::add_function(logic::runtime &runtime, std::shared_ptr<declaration::function_base> entry){
	if (inner_layer_ == nullptr)
		specialized::add_function(runtime, entry);
	else//Use inner layer
		inner_layer_->add_function(runtime, entry);
}

void cminus::logic::storage::double_layer::remove(const std::string &name){
	if (inner_layer_ != nullptr)
		inner_layer_->remove(name);
	specialized::remove(name);
}

bool cminus::logic::storage::double_layer::exists(const std::string &name) const{
	return ((inner_layer_ != nullptr && inner_layer_->exists(name)) || specialized::exists(name));
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::double_layer::find(logic::runtime &runtime, const search_options &options) const{
	if (auto entry = ((inner_layer_ == nullptr) ? nullptr : inner_layer_->find(runtime, search_options{ options.scope, options.context, options.name, false })); entry != nullptr){
		if (options.branch != nullptr)
			*options.branch = this;
		return entry;
	}

	return specialized::find(runtime, options);
}

void cminus::logic::storage::double_layer::refresh(){
	inner_layer_ = std::make_shared<object>("", this);
}

void cminus::logic::storage::double_layer::invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value){
	auto object_parent = dynamic_cast<object *>(parent_);
	if (auto specialized_ancestor = ((object_parent == nullptr) ? nullptr : object_parent->get_first_of_type<specialized>()); specialized_ancestor != nullptr)
		specialized_ancestor->raise_interrupt(type, value);
	else
		specialized::invalid_interrupt_(type, value);
}

cminus::logic::storage::function::function(const declaration::function_base &owner, std::shared_ptr<memory::reference> context, object *parent)
	: specialized("", ((context == nullptr) ? parent : dynamic_cast<object *>(context->get_type().get()))), context_(context), owner_(owner){}

cminus::logic::storage::function::~function(){
	destroy_entries_();
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::function::find(logic::runtime &runtime, const search_options &options) const{
	if (options.context != nullptr)
		return specialized::find(runtime, search_options{ dynamic_cast<object *>(owner_.get_naming_parent()), options.context, options.name, options.search_tree, options.branch });

	if (owner_.get_attributes().has("Static", true))
		return specialized::find(runtime, search_options{ dynamic_cast<object *>(owner_.get_naming_parent()), nullptr, options.name, options.search_tree, options.branch });

	if (!owner_.get_attributes().has("ReadOnlyContext", true))
		return specialized::find(runtime, search_options{ dynamic_cast<object *>(owner_.get_naming_parent()), context_, options.name, options.search_tree, options.branch });

	auto context = context_->apply_offset(0u);
	if (context != nullptr)
		context->add_attribute(runtime.global_storage->find_attribute("ReadOnly", false));

	return specialized::find(runtime, search_options{ dynamic_cast<object *>(owner_.get_naming_parent()), context, options.name, options.search_tree, options.branch });
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::function::get_context() const{
	return context_;
}

void cminus::logic::storage::function::add_unnamed(std::shared_ptr<memory::reference> entry){
	if (unnamed_entries_.find(entry.get()) == unnamed_entries_.end()){
		unnamed_entries_[entry.get()] = entry;
		entries_order_.push_back(entry.get());
	}
}

const cminus::declaration::function_base &cminus::logic::storage::function::get_owner() const{
	return owner_;
}

bool cminus::logic::storage::function::interrupt_is_valid_(interrupt_type value) const{
	return (value == interrupt_type::return_ && owner_.supports_return_statement());
}

void cminus::logic::storage::function::destroy_entry_(memory::reference *entry){
	if (auto it = unnamed_entries_.find(entry); it != unnamed_entries_.end())
		unnamed_entries_.erase(it);
	else
		specialized::destroy_entry_(entry);
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
