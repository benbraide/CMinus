#include "../type/string_type.h"
#include "../type/pointer_type.h"
#include "../node/memory_reference_node.h"

cminus::declaration::string::default_constructor::default_constructor(logic::runtime &runtime, logic::naming::parent *parent)
	: function("string", parent){}

cminus::declaration::string::default_constructor::~default_constructor() = default;

bool cminus::declaration::string::default_constructor::is_defined() const{
	return true;
}

bool cminus::declaration::string::default_constructor::supports_return_statement() const{
	return false;
}

void cminus::declaration::string::default_constructor::print_return_(logic::runtime &runtime) const{}

void cminus::declaration::string::default_constructor::evaluate_body_(logic::runtime &runtime) const{
	auto data = runtime.current_storage->find(runtime, "data_", true);
	auto new_data_block = runtime.memory_object.allocate_block(1u, 0u);

	if (new_data_block == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_data_address = new_data_block->get_address();
	if (new_data_address == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.memory_object.write_scalar(new_data_address, '\0');//Null terminate
	data->write_scalar(runtime, new_data_address);//Copy new address
}

cminus::declaration::string::destructor::destructor(logic::runtime &runtime, logic::naming::parent *parent)
	: function("~string", parent){}

cminus::declaration::string::destructor::~destructor() = default;

bool cminus::declaration::string::destructor::is_defined() const{
	return true;
}

bool cminus::declaration::string::destructor::supports_return_statement() const{
	return false;
}

void cminus::declaration::string::destructor::print_return_(logic::runtime &runtime) const{}

void cminus::declaration::string::destructor::evaluate_body_(logic::runtime &runtime) const{
	auto size = runtime.current_storage->find(runtime, "size_", true);
	auto data = runtime.current_storage->find(runtime, "data_", true);

	size->write_scalar(runtime, 0ui64);//Update size
	if (auto data_address = data->read_scalar<unsigned __int64>(runtime); data_address != 0u){//Free
		runtime.memory_object.deallocate_block(data_address);
		data->write_scalar(runtime, 0ui64);//Update address
	}
}

cminus::declaration::string::empty::empty(logic::runtime &runtime, logic::naming::parent *parent)
	: function("empty", parent){
	return_declaration_ = std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::bool_),		//Type
		"",																				//Name
		nullptr																			//Initialization
	);

	attributes_.add(runtime.global_storage->find_attribute("ReadOnlyContext", false));
}

cminus::declaration::string::empty::~empty() = default;

bool cminus::declaration::string::empty::is_defined() const{
	return true;
}

void cminus::declaration::string::empty::evaluate_body_(logic::runtime &runtime) const{
	if (runtime.current_storage->find(runtime, "size_", true)->read_scalar<std::size_t>(runtime) == 0u)
		runtime.current_storage->raise_interrupt(logic::storage::specialized::interrupt_type::return_, runtime.global_storage->get_named_constant(node::named_constant::constant_type::true_));
	else//Not empty
		runtime.current_storage->raise_interrupt(logic::storage::specialized::interrupt_type::return_, runtime.global_storage->get_named_constant(node::named_constant::constant_type::false_));
}

cminus::declaration::string::size::size(logic::runtime &runtime, logic::naming::parent *parent)
	: function("size", parent){
	return_declaration_ = std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_),	//Type
		"",																				//Name
		nullptr																			//Initialization
	);

	attributes_.add(runtime.global_storage->find_attribute("ReadOnlyContext", false));
}

cminus::declaration::string::size::~size() = default;

bool cminus::declaration::string::size::is_defined() const{
	return true;
}

void cminus::declaration::string::size::evaluate_body_(logic::runtime &runtime) const{
	runtime.current_storage->raise_interrupt(logic::storage::specialized::interrupt_type::return_, runtime.current_storage->find(runtime, "size_", true));
}

cminus::declaration::string::data::data(logic::runtime &runtime, bool read_only, logic::naming::parent *parent)
	: function("data", parent){
	auto type = std::make_shared<type::pointer>(runtime.global_storage->get_primitve_type(type::primitive::id_type::char_));
	if (read_only){
		auto read_only_attr = runtime.global_storage->find_attribute("ReadOnlyTarget", false);
		return_declaration_ = std::make_shared<variable>(
			attribute_list_type{ read_only_attr },											//Attributes
			type,																			//Type
			"",																				//Name
			nullptr																			//Initialization
		);

		attributes_.add(runtime.global_storage->find_attribute("ReadOnlyContext", false));
	}
	else{//Not read-only
		return_declaration_ = std::make_shared<variable>(
			attribute_list_type{},															//Attributes
			type,																			//Type
			"",																				//Name
			nullptr																			//Initialization
		);
	}
}

cminus::declaration::string::data::~data() = default;

bool cminus::declaration::string::data::is_defined() const{
	return true;
}

void cminus::declaration::string::data::evaluate_body_(logic::runtime &runtime) const{
	runtime.current_storage->raise_interrupt(logic::storage::specialized::interrupt_type::return_, runtime.current_storage->find(runtime, "data_", true));
}

cminus::declaration::string::resize::resize(logic::runtime &runtime, logic::naming::parent *parent)
	: function("resize", parent){
	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_),	//Type
		"value",																		//Name
		nullptr																			//Initialization
	));

	auto char_type = runtime.global_storage->get_primitve_type(type::primitive::id_type::char_);
	auto null_char = std::make_shared<memory::scalar_reference<char>>(char_type, nullptr, '\0');

	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		char_type,																		//Type
		"fill",																			//Name
		std::make_shared<node::memory_reference>(nullptr, null_char)					//Initialization
	));

	min_arg_count_ = 1u;
	max_arg_count_ = 2u;
}

cminus::declaration::string::resize::~resize() = default;

bool cminus::declaration::string::resize::is_defined() const{
	return true;
}

void cminus::declaration::string::resize::evaluate_body_(logic::runtime &runtime) const{
	auto size = runtime.current_storage->find(runtime, "size_", true);
	auto value = runtime.current_storage->find(runtime, "value", true)->read_scalar<unsigned __int64>(runtime);

	auto size_value = size->read_scalar<unsigned __int64>(runtime);
	if (size_value != value){//Allocate new block
		auto new_data_block = runtime.memory_object.allocate_block((value + 1u), 0u);
		if (new_data_block == nullptr)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		auto new_data_address = new_data_block->get_address();
		if (new_data_address == 0u)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		auto data = runtime.current_storage->find(runtime, "data_", true);
		if (auto data_address = data->read_scalar<unsigned __int64>(runtime); data_address != 0u){//Copy and free
			runtime.memory_object.write(data_address, new_data_address, ((size_value < value) ? size_value : value));
			runtime.memory_object.deallocate_block(data_address);
		}

		if (size_value < value)//Fill
			runtime.memory_object.set((new_data_address + size_value), runtime.current_storage->find(runtime, "fill", true)->read_scalar<std::byte>(runtime), ((value - size_value) + 1u));
		else//Null terminate
			runtime.memory_object.write_scalar((new_data_address + value), '\0');

		data->write_scalar(runtime, new_data_address);//Copy new address
		size->write_scalar(runtime, value);//Update size
	}

	runtime.current_storage->raise_interrupt(logic::storage::specialized::interrupt_type::return_, nullptr);
}

cminus::declaration::string::clear::clear(logic::runtime &runtime, logic::naming::parent *parent)
	: function("clear", parent){}

cminus::declaration::string::clear::~clear() = default;

bool cminus::declaration::string::clear::is_defined() const{
	return true;
}

void cminus::declaration::string::clear::evaluate_body_(logic::runtime &runtime) const{
	auto size = runtime.current_storage->find(runtime, "size_", true);
	if (0u < size->read_scalar<unsigned __int64>(runtime)){
		auto new_data_block = runtime.memory_object.allocate_block(1u, 0u);
		if (new_data_block == nullptr)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		auto new_data_address = new_data_block->get_address();
		if (new_data_address == 0u)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		auto data = runtime.current_storage->find(runtime, "data_", true);
		if (auto data_address = data->read_scalar<unsigned __int64>(runtime); data_address != 0u)//Free
			runtime.memory_object.deallocate_block(data_address);

		runtime.memory_object.write_scalar(new_data_address, '\0');//Null terminate
		data->write_scalar(runtime, new_data_address);//Copy new address
		size->write_scalar(runtime, 0ui64);//Update size
	}

	runtime.current_storage->raise_interrupt(logic::storage::specialized::interrupt_type::return_, nullptr);
}
