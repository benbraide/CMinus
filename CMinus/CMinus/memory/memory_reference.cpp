#include "../logic/runtime.h"

cminus::memory::reference::reference(std::shared_ptr<logic::type::object> type, unsigned int attributes)
	: type_(type), attributes_(attributes | attribute_uninitialized){}

cminus::memory::reference::~reference() = default;

void cminus::memory::reference::write(logic::runtime &runtime, std::shared_ptr<reference> source){
	throw exception(error_code::write_protected, 0u);
}

void cminus::memory::reference::write(logic::runtime &runtime, const std::byte *source, std::shared_ptr<logic::type::object> type){
	throw exception(error_code::write_protected, 0u);
}

std::shared_ptr<cminus::logic::type::object> cminus::memory::reference::get_type() const{
	return type_;
}

unsigned int cminus::memory::reference::get_attributes() const{
	return attributes_;
}

std::size_t cminus::memory::reference::get_address() const{
	return 0u;
}

cminus::memory::hard_reference::hard_reference(logic::runtime &runtime, std::shared_ptr<logic::type::object> type, unsigned int attributes)
	: reference(type, attributes), object_(runtime.memory_object){
	try{
		address_ = object_.allocate_block(type->get_size(), memory::block::attribute_none)->get_address();
	}
	catch (...){
		address_ = 0u;
	}
}

cminus::memory::hard_reference::~hard_reference(){
	try{
		object_.deallocate_block(address_);
	}
	catch (...){}
}

void cminus::memory::hard_reference::write(logic::runtime &runtime, std::shared_ptr<reference> source){
	if ((source->get_attributes() & attribute_uninitialized) != 0u)
		throw exception(error_code::uninitialized_memory, source->get_address());

	auto score_result = type_->get_score(*source->get_type());
	if (score_result == logic::type::object::score_result_type::nil)
		throw exception(error_code::incompatible_types, 0u);

	if ((attributes_ & (attribute_constant | attribute_uninitialized)) == attribute_constant)
		throw exception(error_code::write_protected, address_);

	std::shared_ptr<reference> temp_ref;
	if (score_result != logic::type::object::score_result_type::exact && (source = source->get_type()->convert_value(runtime, source, type_)) == nullptr)
		throw exception(error_code::incompatible_types, address_);

	attributes_ &= ~reference::attribute_uninitialized;//Remove uninitialized if active
	if (source->get_address() == 0u)//Use data
		runtime.memory_object.write(address_, source->get_data(), type_->get_size());
	else//Use address
		runtime.memory_object.write(address_, source->get_address(), type_->get_size());
}

void cminus::memory::hard_reference::write(logic::runtime &runtime, const std::byte *source, std::shared_ptr<logic::type::object> type){
	auto score_result = type_->get_score(*type);
	if (score_result == logic::type::object::score_result_type::nil)
		throw exception(error_code::incompatible_types, 0u);

	if ((attributes_ & (attribute_constant | attribute_uninitialized)) == attribute_constant)
		throw exception(error_code::write_protected, address_);

	std::shared_ptr<reference> temp_ref;
	if (score_result != logic::type::object::score_result_type::exact && (temp_ref = type->convert_value(runtime, source, type_)) == nullptr)
		throw exception(error_code::incompatible_types, address_);

	attributes_ &= ~reference::attribute_uninitialized;//Remove uninitialized if active
	if (temp_ref != nullptr){//Value converted
		if (temp_ref->get_address() == 0u)//Use data
			runtime.memory_object.write(address_, temp_ref->get_data(), type_->get_size());
		else//Use address
			runtime.memory_object.write(address_, temp_ref->get_address(), type_->get_size());
	}
	else//No conversion
		runtime.memory_object.write(address_, source, type_->get_size());
}

std::size_t cminus::memory::hard_reference::get_address() const{
	return address_;
}

const std::byte *cminus::memory::hard_reference::get_data() const{
	auto block = object_.find_block(address_);
	return ((block == nullptr) ? nullptr : block->get_data());
}
