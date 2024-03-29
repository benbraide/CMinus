#include "../type/string_type.h"
#include "../type/pointer_type.h"

cminus::logic::attributes::object::object(const std::string &name, naming::parent *parent)
	: single(name, parent){}

cminus::logic::attributes::object::~object() = default;

bool cminus::logic::attributes::object::is_same(const naming::object &target) const{
	return single::is_same(target);
}

bool cminus::logic::attributes::object::is_required_on_ref_assignment(logic::runtime &runtime) const{
	return false;
}

bool cminus::logic::attributes::object::is_required_on_pointer_assignment(logic::runtime &runtime) const{
	return false;
}

bool cminus::logic::attributes::object::inherits_from_context(logic::runtime &runtime) const{
	return false;
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

bool cminus::logic::attributes::object::is_required_on_pointer_assignment_(logic::runtime &runtime) const{
	return false;
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
	handle_stage_(runtime, stage, target, args);
}

void cminus::logic::attributes::object::handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (prohibits_stage_(runtime, stage, target))
		throw_error_(runtime, args, 0u);
}

cminus::logic::attributes::read_guard::read_guard(logic::runtime &runtime, memory::reference *target){
	target->call_attributes(runtime, object::stage_type::before_read);
	callback_ = [&runtime, target]{
		target->call_attributes(runtime, object::stage_type::after_read);
	};
}

cminus::logic::attributes::read_guard::~read_guard(){
	try{
		if (callback_ != nullptr)
			callback_();
	}
	catch (...){}
}

cminus::logic::attributes::write_guard::write_guard(logic::runtime &runtime, memory::reference *target){
	target->call_attributes(runtime, object::stage_type::before_write);
	callback_ = [&runtime, target]{
		target->call_attributes(runtime, object::stage_type::after_write);
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
	if (value != nullptr && find(*value) == nullptr)
		list_[value.get()] = value;
}

void cminus::logic::attributes::collection::add(const list_type &list){
	for (auto entry : list)
		add(entry);
}

void cminus::logic::attributes::collection::add(const optimised_list_type &list){
	for (auto &entry : list)
		add(entry.second);
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

void cminus::logic::attributes::collection::clear(){
	list_.clear();
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

void cminus::logic::attributes::collection::traverse(logic::runtime &runtime, const type::object &target_type, const std::function<void(std::shared_ptr<object>)> &callback, object::stage_type stage) const{
	for (auto &entry : list_){
		if (entry.first->applies_to_type(runtime, target_type) && (stage == logic::attributes::object::stage_type::nil || entry.first->handles_stage(runtime, stage)))
			callback(entry.second);
	}
}

void cminus::logic::attributes::collection::traverse(logic::runtime &runtime, const std::function<void(std::shared_ptr<object>)> &callback, object::stage_type stage) const{
	for (auto &entry : list_){
		if (stage == logic::attributes::object::stage_type::nil || entry.first->handles_stage(runtime, stage))
			callback(entry.second);
	}
}

void cminus::logic::attributes::collection::traverse(const std::function<void(std::shared_ptr<object>)> &callback) const{
	for (auto &entry : list_)
		callback(entry.second);
}

void cminus::logic::attributes::collection::call(logic::runtime &runtime, object::stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	for (auto &entry : list_){
		if (entry.first->applies_to_type(runtime, *target->get_type()) && (stage == logic::attributes::object::stage_type::nil || entry.first->handles_stage(runtime, stage)))
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

cminus::logic::attributes::pointer_object::pointer_object(std::shared_ptr<object> target)
	: object(("&" + target->get_naming_value()), target->get_naming_parent()), target_(target){}

cminus::logic::attributes::pointer_object::~pointer_object() = default;

bool cminus::logic::attributes::pointer_object::is_same(const naming::object &target) const{
	if (auto pointer_target = dynamic_cast<const pointer_object *>(&target); pointer_target != nullptr)
		return target_->is_same(*pointer_target->target_);
	return false;
}

bool cminus::logic::attributes::pointer_object::is_required_on_pointer_assignment(logic::runtime &runtime) const{
	return target_->is_required_on_pointer_assignment_(runtime);
}

bool cminus::logic::attributes::pointer_object::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	if (auto pointer_target_type = dynamic_cast<const type::pointer *>(&target_type); pointer_target_type != nullptr)
		return target_->applies_to_type(runtime, *pointer_target_type->get_base_type());
	return false;
}

bool cminus::logic::attributes::pointer_object::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::pointer_object::get_pointer_target(logic::runtime &runtime) const{
	return target_;
}

bool cminus::logic::attributes::pointer_object::is_required_on_pointer_assignment_(logic::runtime &runtime) const{
	return target_->is_required_on_pointer_assignment_(runtime);
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

bool cminus::logic::attributes::bound_object::is_required_on_ref_assignment(logic::runtime &runtime) const{
	return target_->is_required_on_ref_assignment(runtime);
}

bool cminus::logic::attributes::bound_object::is_required_on_pointer_assignment(logic::runtime &runtime) const{
	return target_->is_required_on_pointer_assignment(runtime);
}

bool cminus::logic::attributes::bound_object::inherits_from_context(logic::runtime &runtime) const{
	return target_->inherits_from_context(runtime);
}

bool cminus::logic::attributes::bound_object::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	return target_->applies_to_type(runtime, target_type);
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

bool cminus::logic::attributes::bound_object::is_required_on_pointer_assignment_(logic::runtime &runtime) const{
	return target_->is_required_on_pointer_assignment_(runtime);
}

void cminus::logic::attributes::bound_object::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (args.empty())
		return target_->call_(runtime, stage, target, args_);

	std::vector<std::shared_ptr<memory::reference>> combined_args;
	combined_args.reserve(args_.size() + args.size());

	for (auto arg : args)
		combined_args.push_back(arg);

	for (auto arg : args_)
		combined_args.push_back(arg);

	return target_->call_(runtime, stage, target, combined_args);
}

cminus::logic::attributes::external::external(const std::string &name)
	: object(name, nullptr){}

cminus::logic::attributes::external::~external() = default;

cminus::logic::attributes::final::final()
	: external("Final"){}

cminus::logic::attributes::final::~final() = default;

bool cminus::logic::attributes::final::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	return false;
}

bool cminus::logic::attributes::final::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_inheritance);
}

bool cminus::logic::attributes::final::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::before_inheritance);
}

std::string cminus::logic::attributes::final::get_default_message_() const{
	return "Cannot inherit a class marked as 'final'!";
}

cminus::logic::attributes::read_only::read_only()
	: external("ReadOnly"){}

cminus::logic::attributes::read_only::~read_only() = default;

bool cminus::logic::attributes::read_only::is_required_on_ref_assignment(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::read_only::inherits_from_context(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::read_only::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	if (auto primitive_target_type = dynamic_cast<const type::primitive *>(&target_type); primitive_target_type != nullptr)
		return (primitive_target_type->get_id() != type::primitive::id_type::function);
	return true;
}

bool cminus::logic::attributes::read_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_write);
}

bool cminus::logic::attributes::read_only::is_required_on_pointer_assignment_(logic::runtime &runtime) const{
	return true;
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

bool cminus::logic::attributes::read_only_context::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	if (auto primitive_target_type = dynamic_cast<const type::primitive *>(&target_type); primitive_target_type != nullptr)
		return (primitive_target_type->get_id() == type::primitive::id_type::function);
	return false;
}

bool cminus::logic::attributes::read_only_context::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

cminus::logic::attributes::write_only::write_only()
	: external("WriteOnly"){}

cminus::logic::attributes::write_only::~write_only() = default;

bool cminus::logic::attributes::write_only::is_required_on_ref_assignment(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::write_only::inherits_from_context(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::write_only::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	if (auto primitive_target_type = dynamic_cast<const type::primitive *>(&target_type); primitive_target_type != nullptr)
		return (primitive_target_type->get_id() != type::primitive::id_type::function);
	return true;
}

bool cminus::logic::attributes::write_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_read);
}

bool cminus::logic::attributes::write_only::is_required_on_pointer_assignment_(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::write_only::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::before_read);
}

std::string cminus::logic::attributes::write_only::get_default_message_() const{
	return "Cannot read to object. Object is write-only!";
}

cminus::logic::attributes::not_null::not_null()
	: external("NotNull"){}

cminus::logic::attributes::not_null::~not_null() = default;

bool cminus::logic::attributes::not_null::is_required_on_ref_assignment(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::not_null::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	return (dynamic_cast<const type::pointer *>(&target_type) != nullptr);
}

bool cminus::logic::attributes::not_null::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::after_uninitialized_declaration || value == stage_type::after_write);
}

bool cminus::logic::attributes::not_null::is_required_on_pointer_assignment_(logic::runtime &runtime) const{
	return true;
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

bool cminus::logic::attributes::ref::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	if (auto primitive_target_type = dynamic_cast<const type::primitive *>(&target_type); primitive_target_type != nullptr)
		return (primitive_target_type->get_id() != type::primitive::id_type::function);
	return true;
}

bool cminus::logic::attributes::ref::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::after_uninitialized_declaration);
}

bool cminus::logic::attributes::ref::prohibits_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return (stage == stage_type::after_uninitialized_declaration);
}

std::string cminus::logic::attributes::ref::get_default_message_() const{
	return "Reference variable requires initialization!";
}

cminus::logic::attributes::deprecated::deprecated()
	: external("Deprecated"){}

cminus::logic::attributes::deprecated::~deprecated() = default;

bool cminus::logic::attributes::deprecated::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	return true;
}

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

bool cminus::logic::attributes::special::applies_to_type(logic::runtime &runtime, const type::object &target_type) const{
	return true;
}

bool cminus::logic::attributes::special::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}
