#include "../type/string_type.h"
#include "../type/pointer_type.h"
#include "../node/memory_reference_node.h"

cminus::declaration::string::constructor::constructor(logic::naming::parent *parent)
	: function("string", parent){}

cminus::declaration::string::constructor::~constructor() = default;

bool cminus::declaration::string::constructor::is_defined() const{
	return true;
}

bool cminus::declaration::string::constructor::supports_return_statement() const{
	return false;
}

void cminus::declaration::string::constructor::print_return_(logic::runtime &runtime) const{}

cminus::declaration::string::default_constructor::default_constructor(logic::naming::parent *parent)
	: constructor(parent){}

cminus::declaration::string::default_constructor::~default_constructor() = default;

void cminus::declaration::string::default_constructor::evaluate_body_(logic::runtime &runtime) const{
	auto new_data_block = runtime.memory_object.allocate_block(1u, 0u);
	if (new_data_block == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_data_address = new_data_block->get_address();
	if (new_data_address == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.memory_object.write_scalar(new_data_address, '\0');//Null terminate
	runtime.current_storage->find(runtime, "data_", true)->write_scalar(runtime, new_data_address);//Copy new address
	runtime.current_storage->find(runtime, "size_", true)->write_scalar(runtime, 0ui64);//Update size
}

cminus::declaration::string::copy_constructor::copy_constructor(logic::runtime &runtime, logic::naming::parent *parent)
	: constructor(parent){
	attribute_list_type attributes{
		runtime.global_storage->find_attribute("ReadOnly", false),
		runtime.global_storage->find_attribute("Ref", false)
	};

	params_.push_back(std::make_shared<variable>(
		attributes,																		//Attributes
		runtime.global_storage->get_string_type(),										//Type
		"other",																		//Name
		nullptr																			//Initialization
	));

	min_arg_count_ = 1u;
	max_arg_count_ = 1u;
}

cminus::declaration::string::copy_constructor::~copy_constructor() = default;

void cminus::declaration::string::copy_constructor::evaluate_body_(logic::runtime &runtime) const{
	auto other_data = runtime.global_storage->get_string_data(runtime, runtime.current_storage->find(runtime, "other", true));
	if (other_data == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto size_value = strlen(other_data);
	auto new_data_block = runtime.memory_object.allocate_block((size_value + 1u), 0u);

	if (new_data_block == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_data_address = new_data_block->get_address();
	if (new_data_address == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.memory_object.write_buffer(new_data_address, other_data, size_value);//Copy value
	runtime.memory_object.write_scalar((new_data_address + size_value), '\0');//Null terminate

	runtime.current_storage->find(runtime, "data_", true)->write_scalar(runtime, new_data_address);//Copy new address
	runtime.current_storage->find(runtime, "size_", true)->write_scalar(runtime, size_value);//Update size
}

cminus::declaration::string::sub_constructor::sub_constructor(logic::runtime &runtime, logic::naming::parent *parent)
	: constructor(parent){
	attribute_list_type attributes{
		runtime.global_storage->find_attribute("ReadOnly", false),
		runtime.global_storage->find_attribute("Ref", false)
	};

	params_.push_back(std::make_shared<variable>(
		attributes,																		//Attributes
		runtime.global_storage->get_string_type(),										//Type
		"other",																		//Name
		nullptr																			//Initialization
	));

	auto int_type = runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_);
	auto max_int = std::make_shared<memory::scalar_reference<unsigned __int64>>(int_type, nullptr, static_cast<unsigned __int64>(-1));

	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		int_type,																		//Type
		"position",																		//Name
		nullptr																			//Initialization
	));

	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		int_type,																		//Type
		"size",																			//Name
		std::make_shared<node::memory_reference>(nullptr, max_int)						//Initialization
	));

	min_arg_count_ = 2u;
	max_arg_count_ = 3u;
}

cminus::declaration::string::sub_constructor::~sub_constructor() = default;

void cminus::declaration::string::sub_constructor::evaluate_body_(logic::runtime &runtime) const{
	auto other_data = runtime.global_storage->get_string_data(runtime, runtime.current_storage->find(runtime, "other", true));
	if (other_data == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto other_size_value = strlen(other_data);
	auto position_value = runtime.current_storage->find(runtime, "position", true)->read_scalar<unsigned __int64>(runtime);

	if (other_size_value <= position_value)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto size_value = runtime.current_storage->find(runtime, "size", true)->read_scalar<unsigned __int64>(runtime);
	if ((other_size_value - position_value) < size_value)//Limit size
		size_value = (other_size_value - position_value);

	auto new_data_block = runtime.memory_object.allocate_block((size_value + 1u), 0u);
	if (new_data_block == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_data_address = new_data_block->get_address();
	if (new_data_address == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.memory_object.write_buffer(new_data_address, (other_data + position_value), size_value);//Copy value
	runtime.memory_object.write_scalar((new_data_address + size_value), '\0');//Null terminate

	runtime.current_storage->find(runtime, "data_", true)->write_scalar(runtime, new_data_address);//Copy new address
	runtime.current_storage->find(runtime, "size_", true)->write_scalar(runtime, size_value);//Update size
}

cminus::declaration::string::assignment_constructor::assignment_constructor(logic::runtime &runtime, logic::naming::parent *parent)
	: constructor(parent){
	attribute_list_type attributes{
		runtime.global_storage->find_attribute("ReadOnlyTarget", false)
	};

	auto type = std::make_shared<type::pointer>(runtime.global_storage->get_primitve_type(type::primitive::id_type::char_));
	params_.push_back(std::make_shared<variable>(
		attributes,																		//Attributes
		type,																			//Type
		"data",																			//Name
		nullptr																			//Initialization
	));

	auto int_type = runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_);
	auto max_int = std::make_shared<memory::scalar_reference<unsigned __int64>>(int_type, nullptr, static_cast<unsigned __int64>(-1));

	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		int_type,																		//Type
		"size",																			//Name
		std::make_shared<node::memory_reference>(nullptr, max_int)						//Initialization
	));

	min_arg_count_ = 1u;
	max_arg_count_ = 2u;
}

cminus::declaration::string::assignment_constructor::~assignment_constructor() = default;

void cminus::declaration::string::assignment_constructor::evaluate_body_(logic::runtime &runtime) const{
	auto in_data = runtime.current_storage->find(runtime, "data", true)->read_scalar<unsigned __int64>(runtime);
	auto data_value = reinterpret_cast<const char *>(runtime.memory_object.get_block(in_data)->get_data());

	auto size_value = runtime.current_storage->find(runtime, "size", true)->read_scalar<unsigned __int64>(runtime);
	if (size_value == static_cast<unsigned __int64>(-1))//Compute size
		size_value = strlen(data_value);

	auto new_data_block = runtime.memory_object.allocate_block((size_value + 1u), 0u);
	if (new_data_block == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_data_address = new_data_block->get_address();
	if (new_data_address == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.memory_object.write_buffer(new_data_address, data_value, size_value);//Copy value
	runtime.memory_object.write_scalar((new_data_address + size_value), '\0');//Null terminate

	runtime.current_storage->find(runtime, "data_", true)->write_scalar(runtime, new_data_address);//Copy new address
	runtime.current_storage->find(runtime, "size_", true)->write_scalar(runtime, size_value);//Update size
}

cminus::declaration::string::fill_constructor::fill_constructor(logic::runtime &runtime, logic::naming::parent *parent)
	: constructor(parent){
	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_),	//Type
		"size",																			//Name
		nullptr																			//Initialization
	));

	params_.push_back(std::make_shared<variable>(
		attribute_list_type{},															//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::char_),		//Type
		"fill",																			//Name
		nullptr																			//Initialization
	));

	min_arg_count_ = 2u;
	max_arg_count_ = 2u;
}

cminus::declaration::string::fill_constructor::~fill_constructor() = default;

void cminus::declaration::string::fill_constructor::evaluate_body_(logic::runtime &runtime) const{
	auto size_value = runtime.current_storage->find(runtime, "size", true)->read_scalar<unsigned __int64>(runtime);
	auto fill_value = runtime.current_storage->find(runtime, "fill", true)->read_scalar<std::byte>(runtime);

	auto new_data_block = runtime.memory_object.allocate_block((size_value + 1u), 0u);
	if (new_data_block == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_data_address = new_data_block->get_address();
	if (new_data_address == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.memory_object.set(new_data_address, fill_value, size_value);//Fill
	runtime.memory_object.write_scalar((new_data_address + size_value), '\0');//Null terminate

	runtime.current_storage->find(runtime, "data_", true)->write_scalar(runtime, new_data_address);//Copy new address
	runtime.current_storage->find(runtime, "size_", true)->write_scalar(runtime, size_value);//Update size
}

cminus::declaration::string::destructor::destructor(logic::naming::parent *parent)
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
		auto read_only_attr = runtime.global_storage->find_attribute("ReadOnly", false);
		auto ptr_read_only_attr = std::make_shared<logic::attributes::pointer_object>(read_only_attr);

		return_declaration_ = std::make_shared<variable>(
			attribute_list_type{ ptr_read_only_attr },										//Attributes
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
