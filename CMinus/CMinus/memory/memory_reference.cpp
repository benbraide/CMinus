#include "memory_reference.h"

cminus::memory::reference::reference(memory::object &object, std::shared_ptr<logic::type::object> type, unsigned int attributes)
	: object_(object), type_(type), attributes_(attributes | attribute_uninitialized){
	try{
		address_ = object_.allocate_exclusive_block(type->get_size(), memory::block::attribute_none)->get_address();
	}
	catch (...){
		address_ = 0u;
	}
}

cminus::memory::reference::~reference(){
	try{
		object_.deallocate_block(address_);
	}
	catch (...){}
}

void cminus::memory::reference::write(const reference &source){
	if ((source.attributes_ & attribute_uninitialized) != 0u)
		throw exception(error_code::uninitialized_memory, source.address_);
	write(source.address_, *source.type_);
}

void cminus::memory::reference::write(std::size_t address, const logic::type::object &type){
	auto score_result = type_->get_score(type);
	if (score_result == logic::type::object::score_result_type::nil)
		throw exception(error_code::incompatible_types, 0u);

	if ((attributes_ & (attribute_constant | attribute_uninitialized)) == attribute_constant)
		throw exception(error_code::write_protected, address_);

	std::shared_ptr<reference> temp_ref;
	if (score_result != logic::type::object::score_result_type::exact){
		if ((temp_ref = type.convert_value(object_, address, type_)) == nullptr)
			throw exception(error_code::incompatible_types, address_);
		address = temp_ref->address_;
	}

	attributes_ &= ~reference::attribute_uninitialized;//Remove uninitialized if active
	object_.write(address_, address, type_->get_size());//Copy bytes
}

std::shared_ptr<cminus::logic::type::object> cminus::memory::reference::get_type() const{
	return type_;
}

std::size_t cminus::memory::reference::get_address() const{
	return address_;
}

unsigned int cminus::memory::reference::get_attributes() const{
	return attributes_;
}
