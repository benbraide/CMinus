#include "../type/string_type.h"
#include "../type/pointer_type.h"

cminus::logic::attributes::object::object(const std::string &name, naming::parent *parent)
	: single(name, parent){}

cminus::logic::attributes::object::~object() = default;

bool cminus::logic::attributes::object::is_same(const naming::object &target) const{
	return single::is_same(target);
}

bool cminus::logic::attributes::object::applies_to_function() const{
	return true;
}

void cminus::logic::attributes::object::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	return call_(runtime, stage, target, args);
}

void cminus::logic::attributes::object::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return call_(runtime, stage, target, std::vector<std::shared_ptr<memory::reference>>{});
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::object::get_pointer_target(logic::runtime &runtime) const{
	return nullptr;
}

bool cminus::logic::attributes::object::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return false;
}

std::string cminus::logic::attributes::object::get_default_message_() const{
	return "Attribute error!";
}

void cminus::logic::attributes::object::throw_error_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args, std::size_t message_index) const{
	if (message_index < args.size() && dynamic_cast<type::string *>(args[message_index]->get_type().get()) != nullptr)
		throw logic::exception(runtime.global_storage->get_string_data(runtime, args[message_index]), 0u, 0u);
	else//Default message
		throw logic::exception(get_default_message_(), 0u, 0u);
}

void cminus::logic::attributes::object::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	switch (stage){
	case stage_type::before_ref_assign:
		handle_before_ref_assign_(runtime, target, args);
		break;
	default:
		handle_stage_(runtime, stage, target, args);
		break;
	}
	
}

void cminus::logic::attributes::object::handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (prohibits_stage_(runtime, stage, target))
		throw_error_(runtime, args, 0u);
}

void cminus::logic::attributes::object::handle_before_ref_assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (!args.empty() && handles_stage(runtime, stage_type::before_ref_assign) && !args[0]->has_attribute(*this, false))
		throw logic::exception(("'" + get_qualified_naming_value() + "' attribute is required on reference destination!"), 0u, 0u);
}

cminus::logic::attributes::read_guard::read_guard(logic::runtime &runtime, memory::reference *target, bool include_context){
	target->call_attributes(runtime, object::stage_type::before_read, include_context);
	callback_ = [&runtime, target, include_context]{
		target->call_attributes(runtime, object::stage_type::after_read, include_context);
	};
}

cminus::logic::attributes::read_guard::~read_guard(){
	try{
		if (callback_ != nullptr)
			callback_();
	}
	catch (...){}
}

cminus::logic::attributes::write_guard::write_guard(logic::runtime &runtime, memory::reference *target, bool include_context){
	target->call_attributes(runtime, object::stage_type::before_write, include_context);
	callback_ = [&runtime, target, include_context]{
		target->call_attributes(runtime, object::stage_type::after_write, include_context);
	};
}

cminus::logic::attributes::write_guard::~write_guard(){
	try{
		if (callback_ != nullptr)
			callback_();
	}
	catch (...){}
}

cminus::logic::attributes::collection::collection() = default;

cminus::logic::attributes::collection::collection(const list_type &list){
	for (auto entry : list){
		if (entry != nullptr)
			list_[entry.get()] = entry;
	}
}

cminus::logic::attributes::collection::collection(const optimised_list_type &list)
	: list_(list){}

cminus::logic::attributes::collection::collection(optimised_list_type &&list)
	: list_(std::move(list)){}

void cminus::logic::attributes::collection::add(std::shared_ptr<object> value){
	if (value != nullptr)
		list_[value.get()] = value;
}

void cminus::logic::attributes::collection::remove(const std::string &name, bool global_only){
	if (list_.empty())
		return;

	for (auto it = list_.begin(); it != list_.end(); ++it){
		if ((it->first->get_naming_parent() == nullptr) == global_only && it->first->get_naming_value() == name){
			list_.erase(it);
			break;
		}
	}
}

void cminus::logic::attributes::collection::remove(const logic::naming::object &name){
	if (list_.empty())
		return;

	for (auto it = list_.begin(); it != list_.end(); ++it){
		if (it->first->is_same(name)){
			list_.erase(it);
			break;
		}
	}
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::collection::find(const std::string &name, bool global_only) const{
	if (list_.empty())
		return nullptr;

	for (auto &entry : list_){
		if ((entry.first->get_naming_parent() == nullptr) == global_only && entry.first->get_naming_value() == name)
			return entry.second;
	}

	return nullptr;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::collection::find(const logic::naming::object &name) const{
	if (list_.empty())
		return nullptr;

	for (auto &entry : list_){
		if (entry.first->is_same(name))
			return entry.second;
	}

	return nullptr;
}

bool cminus::logic::attributes::collection::has(const std::string &name, bool global_only) const{
	return (find(name, global_only) != nullptr);
}

bool cminus::logic::attributes::collection::has(const naming::object &name) const{
	return (find(name) != nullptr);
}

const cminus::logic::attributes::collection::optimised_list_type &cminus::logic::attributes::collection::get_list() const{
	return list_;
}

void cminus::logic::attributes::collection::traverse(logic::runtime &runtime, const std::function<void(std::shared_ptr<object>)> &callback, object::stage_type stage) const{
	for (auto &entry : list_){
		if (stage == logic::attributes::object::stage_type::nil || entry.first->handles_stage(runtime, stage))
			callback(entry.second);
	}
}

void cminus::logic::attributes::collection::call(logic::runtime &runtime, object::stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	for (auto &entry : list_){
		if (stage == logic::attributes::object::stage_type::nil || entry.first->handles_stage(runtime, stage))
			entry.second->call(runtime, stage, target, args);
	}
}

void cminus::logic::attributes::collection::call(logic::runtime &runtime, object::stage_type stage, std::shared_ptr<memory::reference> target) const{
	call(runtime, stage, target, std::vector<std::shared_ptr<memory::reference>>{});
}

void cminus::logic::attributes::collection::print(logic::runtime &runtime) const{
	if (list_.empty())
		return;

	runtime.writer.write_scalar('[');
	auto is_first = true;

	for (auto &entry : list_){
		if (!is_first)
			runtime.writer.write_scalar(', ');
		else
			is_first = false;

		entry.second->print(runtime, true);
	}

	runtime.writer.write_scalar(']');
}

cminus::logic::attributes::bound_object::bound_object(std::shared_ptr<object> target, const std::vector<std::shared_ptr<memory::reference>> &args)
	: object(target->get_naming_value(), target->get_naming_parent()), target_(target), args_(args){}

cminus::logic::attributes::bound_object::bound_object(std::shared_ptr<object> target, std::vector<std::shared_ptr<memory::reference>> &&args)
	: object(target->get_naming_value(), target->get_naming_parent()), target_(target), args_(std::move(args)){}

cminus::logic::attributes::bound_object::~bound_object() = default;

void cminus::logic::attributes::bound_object::print(logic::runtime &runtime, bool is_qualified) const{
	target_->print(runtime, is_qualified);
	runtime.writer.write_scalar('(');

	if (!args_.empty()){//Print arguments
		auto is_first = true;
		for (auto &arg : args_){
			if (!is_first)
				runtime.writer.write_scalar(', ');
			else
				is_first = false;

			arg->get_type()->print_value(runtime, arg);
		}
	}

	runtime.writer.write_scalar(')');
}

bool cminus::logic::attributes::bound_object::handles_stage(logic::runtime &runtime, stage_type value) const{
	return target_->handles_stage(runtime, value);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::bound_object::get_pointer_target(logic::runtime &runtime) const{
	auto value = target_->get_pointer_target(runtime);
	return ((value == nullptr) ? nullptr : std::make_shared<bound_object>(value, args_));
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::bound_object::get_target() const{
	return target_;
}

const std::vector<std::shared_ptr<cminus::memory::reference>> &cminus::logic::attributes::bound_object::get_args() const{
	return args_;
}

void cminus::logic::attributes::bound_object::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (args.empty())
		return target_->call(runtime, stage, target, args_);

	std::vector<std::shared_ptr<memory::reference>> combined_args;
	combined_args.reserve(args_.size() + args.size());

	for (auto arg : args)
		combined_args.push_back(arg);

	for (auto arg : args_)
		combined_args.push_back(arg);

	return target_->call(runtime, stage, target, combined_args);
}

cminus::logic::attributes::external::external(const std::string &name)
	: object(name, nullptr){}

cminus::logic::attributes::external::~external() = default;

cminus::logic::attributes::private_access::private_access()
	: external("Private"){}

cminus::logic::attributes::private_access::~private_access() = default;

bool cminus::logic::attributes::private_access::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_protected_access || value == stage_type::before_public_access);
}

bool cminus::logic::attributes::private_access::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::before_protected_access || stage == stage_type::before_public_access);
}

std::string cminus::logic::attributes::private_access::get_default_message_() const{
	return "Private object is inaccessible";
}

cminus::logic::attributes::protected_access::protected_access()
	: external("Protected"){}

cminus::logic::attributes::protected_access::~protected_access() = default;

bool cminus::logic::attributes::protected_access::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_public_access);
}

bool cminus::logic::attributes::protected_access::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::before_public_access);
}

std::string cminus::logic::attributes::protected_access::get_default_message_() const{
	return "Protected object is inaccessible";
}

cminus::logic::attributes::public_access::public_access()
	: external("Public"){}

cminus::logic::attributes::public_access::~public_access() = default;

bool cminus::logic::attributes::public_access::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

cminus::logic::attributes::read_only::read_only()
	: external("ReadOnly"){}

cminus::logic::attributes::read_only::~read_only() = default;

bool cminus::logic::attributes::read_only::applies_to_function() const{
	return false;
}

bool cminus::logic::attributes::read_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_ref_assign || value == stage_type::before_write);
}

bool cminus::logic::attributes::read_only::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::before_write);
}

std::string cminus::logic::attributes::read_only::get_default_message_() const{
	return "Cannot write to object. Object is read-only!";
}

cminus::logic::attributes::read_only_context::read_only_context()
	: external("ReadOnlyContext"){}

cminus::logic::attributes::read_only_context::~read_only_context() = default;

bool cminus::logic::attributes::read_only_context::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

cminus::logic::attributes::read_only_target::read_only_target()
	: external("ReadOnlyTarget"){}

cminus::logic::attributes::read_only_target::~read_only_target() = default;

bool cminus::logic::attributes::read_only_target::applies_to_function() const{
	return false;
}

bool cminus::logic::attributes::read_only_target::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::read_only_target::get_pointer_target(logic::runtime &runtime) const{
	return runtime.global_storage->find_attribute("ReadOnly", false);
}

cminus::logic::attributes::write_only::write_only()
	: external("WriteOnly"){}

cminus::logic::attributes::write_only::~write_only() = default;

bool cminus::logic::attributes::write_only::applies_to_function() const{
	return false;
}

bool cminus::logic::attributes::write_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_ref_assign || value == stage_type::before_read);
}

bool cminus::logic::attributes::write_only::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::before_read);
}

std::string cminus::logic::attributes::write_only::get_default_message_() const{
	return "Cannot read to object. Object is write-only!";
}

cminus::logic::attributes::write_only_target::write_only_target()
	: external("WriteOnlyTarget"){}

cminus::logic::attributes::write_only_target::~write_only_target() = default;

bool cminus::logic::attributes::write_only_target::applies_to_function() const{
	return false;
}

bool cminus::logic::attributes::write_only_target::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::write_only_target::get_pointer_target(logic::runtime &runtime) const{
	return runtime.global_storage->find_attribute("WriteOnly", false);
}

cminus::logic::attributes::not_null::not_null()
	: external("NotNull"){}

cminus::logic::attributes::not_null::~not_null() = default;

bool cminus::logic::attributes::not_null::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::after_uninitialized_declaration || value == stage_type::before_ref_assign || value == stage_type::after_write);
}

bool cminus::logic::attributes::not_null::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	if (stage == stage_type::after_uninitialized_declaration)
		return true;

	return (stage == stage_type::after_write && dynamic_cast<type::pointer *>(target->get_type().get()) != nullptr && target->read_scalar<unsigned __int64>(runtime) == 0u);
}

std::string cminus::logic::attributes::not_null::get_default_message_() const{
	return "Cannot write null value to object!";
}

cminus::logic::attributes::ref::ref()
	: external("Ref"){}

cminus::logic::attributes::ref::~ref() = default;

bool cminus::logic::attributes::ref::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

cminus::logic::attributes::deprecated::deprecated()
	: external("Deprecated"){}

cminus::logic::attributes::deprecated::~deprecated() = default;

bool cminus::logic::attributes::deprecated::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::after_lookup);
}

void cminus::logic::attributes::deprecated::handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (!args.empty() && dynamic_cast<type::string *>(args[0]->get_type().get()) != nullptr)
		runtime.writer.write_scalar(std::string(runtime.global_storage->get_string_data(runtime, args[0])));
	else//Default message
		runtime.writer.write_buffer("The function or variable is deprecated", 38u);
}

cminus::logic::attributes::special::special(const std::string &name)
	: external(name){}

cminus::logic::attributes::special::~special() = default;

bool cminus::logic::attributes::special::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}
