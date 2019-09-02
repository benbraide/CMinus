#include "../type/pointer_type.h"

cminus::logic::attributes::object::object(const std::string &name, naming::parent *parent)
	: single(name, parent){}

cminus::logic::attributes::object::~object() = default;

bool cminus::logic::attributes::object::is_same(const naming::object &target) const{
	return single::is_same(target);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::object::get_non_pointer_object() const{
	return nullptr;
}

bool cminus::logic::attributes::object::is_included_in_comparison(logic::runtime &runtime) const{
	return false;
}

void cminus::logic::attributes::object::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	return call_(runtime, stage, target, args);
}

void cminus::logic::attributes::object::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return call_(runtime, stage, target, std::vector<std::shared_ptr<memory::reference>>{});
}

bool cminus::logic::attributes::object::prohibits_stage_(stage_type value) const{
	return false;
}

std::string cminus::logic::attributes::object::get_default_message_() const{
	return "Attribute error!";
}

void cminus::logic::attributes::object::throw_error_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args, std::size_t message_index) const{
	if (message_index < args.size()/* && args[message_index].IsStringType()*/)
		throw logic::exception(""/*args[message_index].ReadString()*/, 0u, 0u);
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
	if (prohibits_stage_(stage))
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
	for (auto entry : list)
		list_[entry.get()] = entry;
}

cminus::logic::attributes::collection::collection(const optimised_list_type &list)
	: list_(list){}

cminus::logic::attributes::collection::collection(optimised_list_type &&list)
	: list_(std::move(list)){}

void cminus::logic::attributes::collection::add(std::shared_ptr<object> value){
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

cminus::logic::attributes::pointer_object::pointer_object(std::shared_ptr<object> target)
	: object(("&" + target->get_naming_value()), target->get_naming_parent()), target_(target){}

cminus::logic::attributes::pointer_object::~pointer_object() = default;

bool cminus::logic::attributes::pointer_object::is_same(const naming::object &target) const{
	if (auto pointer_target = dynamic_cast<const pointer_object *>(&target); pointer_target != nullptr)
		return target_->is_same(*pointer_target->target_);
	return false;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::pointer_object::get_non_pointer_object() const{
	return target_;
}

bool cminus::logic::attributes::pointer_object::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}

bool cminus::logic::attributes::pointer_object::is_included_in_comparison(logic::runtime &runtime) const{
	return target_->is_included_in_comparison(runtime);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::pointer_object::get_target() const{
	return target_;
}

void cminus::logic::attributes::pointer_object::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{}

cminus::logic::attributes::bound_object::bound_object(std::shared_ptr<object> target, const std::vector<std::shared_ptr<memory::reference>> &args)
	: object(target->get_naming_value(), target->get_naming_parent()), target_(target), args_(args){}

cminus::logic::attributes::bound_object::bound_object(std::shared_ptr<object> target, std::vector<std::shared_ptr<memory::reference>> &&args)
	: object(target->get_naming_value(), target->get_naming_parent()), target_(target), args_(std::move(args)){}

cminus::logic::attributes::bound_object::~bound_object() = default;

bool cminus::logic::attributes::bound_object::handles_stage(logic::runtime &runtime, stage_type value) const{
	return target_->handles_stage(runtime, value);
}

bool cminus::logic::attributes::bound_object::is_included_in_comparison(logic::runtime &runtime) const{
	return target_->is_included_in_comparison(runtime);
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

cminus::logic::attributes::read_only::read_only()
	: external("ReadOnly"){}

cminus::logic::attributes::read_only::~read_only() = default;

bool cminus::logic::attributes::read_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_ref_assign || value == stage_type::before_write);
}

bool cminus::logic::attributes::read_only::is_included_in_comparison(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::read_only::prohibits_stage_(stage_type value) const{
	return (value == stage_type::before_write);
}

std::string cminus::logic::attributes::read_only::get_default_message_() const{
	return "Cannot write to object. Object is read-only!";
}

cminus::logic::attributes::write_only::write_only()
	: external("WriteOnly"){}

cminus::logic::attributes::write_only::~write_only() = default;

bool cminus::logic::attributes::write_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_ref_assign || value == stage_type::before_read);
}

bool cminus::logic::attributes::write_only::is_included_in_comparison(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::write_only::prohibits_stage_(stage_type value) const{
	return (value == stage_type::before_read);
}

std::string cminus::logic::attributes::write_only::get_default_message_() const{
	return "Cannot read to object. Object is write-only!";
}

cminus::logic::attributes::not_null::not_null()
	: external("NotNull"){}

cminus::logic::attributes::not_null::~not_null() = default;

bool cminus::logic::attributes::not_null::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::after_uninitialized_declaration || value == stage_type::before_ref_assign || value == stage_type::after_write);
}

bool cminus::logic::attributes::not_null::is_included_in_comparison(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::not_null::prohibits_stage_(stage_type value) const{
	return (value == stage_type::after_uninitialized_declaration);
}

std::string cminus::logic::attributes::not_null::get_default_message_() const{
	return "Cannot write null value to object!";
}

void cminus::logic::attributes::not_null::handle_stage_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (stage == stage_type::after_write && dynamic_cast<type::pointer *>(target->get_type().get()) != nullptr && target->read_scalar<unsigned __int64>(runtime) == 0u)
		throw_error_(runtime, args, 0u);
	else
		external::handle_stage_(runtime, stage, target, args);
}

cminus::logic::attributes::nan::nan()
	: external("#NaN#"){}

cminus::logic::attributes::nan::~nan() = default;

bool cminus::logic::attributes::nan::handles_stage(logic::runtime &runtime, stage_type value) const{
	return false;
}
