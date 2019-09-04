#include "../node/list_node.h"
#include "../node/memory_reference_node.h"
#include "../type/string_type.h"

#include "../evaluator/byte_evaluator.h"
#include "../evaluator/boolean_evaluator.h"
#include "../evaluator/integral_evaluator.h"
#include "../evaluator/floating_point_evaluator.h"

cminus::logic::storage::global::global()
	: object("", nullptr){}

cminus::logic::storage::global::~global(){
	destroy_entries_();
}

void cminus::logic::storage::global::init(logic::runtime &runtime){
	attributes_["Private"] = std::make_shared<logic::attributes::special>("Private");
	attributes_["Protected"] = std::make_shared<logic::attributes::special>("Protected");
	attributes_["Public"] = std::make_shared<logic::attributes::special>("Public");

	attributes_["ThreadLocal"] = std::make_shared<logic::attributes::special>("ThreadLocal");
	attributes_["Static"] = std::make_shared<logic::attributes::special>("Static");
	attributes_["Final"] = std::make_shared<logic::attributes::final>();

	attributes_["ReadOnly"] = std::make_shared<logic::attributes::read_only>();
	attributes_["ReadOnlyContext"] = std::make_shared<logic::attributes::read_only_context>();
	attributes_["WriteOnly"] = std::make_shared<logic::attributes::write_only>();

	attributes_["Deprecated"] = std::make_shared<logic::attributes::deprecated>();
	attributes_["NotNull"] = std::make_shared<logic::attributes::not_null>();
	attributes_["Ref"] = std::make_shared<logic::attributes::ref>();

	attributes_["#NaN#"] = std::make_shared<logic::attributes::special>("#NaN#");
	attributes_["#Init#"] = std::make_shared<logic::attributes::special>("#Init#");

	for (auto id = type::primitive::id_type::nullptr_; id < type::primitive::id_type::string; id = static_cast<type::primitive::id_type>(static_cast<int>(id) + 1))
		primitive_types_[id] = std::make_shared<type::primitive>(id);

	for (auto id = node::named_constant::constant_type::false_; id <= node::named_constant::constant_type::true_; id = static_cast<node::named_constant::constant_type>(static_cast<int>(id) + 1)){
		named_constants_[id] = std::make_shared<memory::scalar_reference<node::named_constant::constant_type>>(
			primitive_types_[type::primitive::id_type::bool_],
			nullptr,
			id
		);
	}

	named_constants_[node::named_constant::constant_type::nullptr_] = std::make_shared<memory::scalar_reference<unsigned __int64>>(
		primitive_types_[type::primitive::id_type::nullptr_],
		nullptr,
		0ui64
	);

	named_constants_[node::named_constant::constant_type::nan_] = std::make_shared<memory::scalar_reference<node::named_constant::constant_type>>(
		primitive_types_[type::primitive::id_type::nan_],
		nullptr,
		node::named_constant::constant_type::nan_
	);

	if (auto string_type = std::make_shared<type::string>(runtime); string_type != nullptr){
		primitive_types_[type::primitive::id_type::string] = string_type;
		string_type->init(runtime);
	}

	evaluators_[evaluator::id::byte] = std::make_shared<evaluator::byte>();
	evaluators_[evaluator::id::boolean] = std::make_shared<evaluator::boolean>();

	evaluators_[evaluator::id::integral] = std::make_shared<evaluator::integral>();
	evaluators_[evaluator::id::floating_point] = std::make_shared<evaluator::floating_point>();
}

std::shared_ptr<cminus::type::object> cminus::logic::storage::global::get_primitve_type(type::primitive::id_type id) const{
	if (primitive_types_.empty())
		return nullptr;

	if (auto it = primitive_types_.find(id); it != primitive_types_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::type::object> cminus::logic::storage::global::get_string_type() const{
	return get_primitve_type(type::primitive::id_type::string);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::global::get_named_constant(node::named_constant::constant_type type) const{
	if (named_constants_.empty())
		return nullptr;

	if (auto it = named_constants_.find(type); it != named_constants_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::logic::storage::global::get_evaluator(evaluator::id id) const{
	if (evaluators_.empty())
		return nullptr;

	if (auto it = evaluators_.find(id); it != evaluators_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::global::create_string(logic::runtime &runtime, const std::string &value) const{
	auto str = std::make_shared<memory::data_reference>(runtime, get_string_type(), declaration::variable::attribute_list_type{}, nullptr);
	if (str == nullptr)//Error
		return nullptr;

	if (value.empty()){
		str->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
		str->get_type()->construct(runtime, str, nullptr);
		return str;
	}

	auto size = std::make_shared<memory::scalar_reference<unsigned __int64>>(
		get_primitve_type(type::primitive::id_type::uint64_),
		nullptr,
		value.size()
	);

	auto fill = std::make_shared<memory::scalar_reference<char>>(
		get_primitve_type(type::primitive::id_type::char_),
		nullptr,
		'\0'
	);

	std::vector<std::shared_ptr<node::object>> init_list{
		std::make_shared<node::memory_reference>(nullptr, size),
		std::make_shared<node::memory_reference>(nullptr, fill)
	};

	str->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
	str->get_type()->construct(runtime, str, std::make_shared<node::list>(nullptr, std::move(init_list)));
	memcpy(const_cast<char *>(get_string_data(runtime, str)), value.data(), value.size());

	return str;
}

const char *cminus::logic::storage::global::get_string_data(logic::runtime &runtime, std::shared_ptr<memory::reference> object) const{
	auto string_type = dynamic_cast<type::string *>(object->get_type().get());
	if (string_type == nullptr)
		return nullptr;

	auto data = string_type->find(runtime, search_options{ string_type, object, "data_", false });
	if (auto data_block = runtime.memory_object.get_block(data->read_scalar<unsigned __int64>(runtime)); data_block != nullptr)
		return reinterpret_cast<const char *>(data_block->get_data());

	return nullptr;
}
