#include <sstream>

#include "io/io_stream_reader.h"
#include "io/io_stream_writer.h"
#include "type/string_type.h"

class test_string{
public:
	struct call_info{
		std::string name;
		std::shared_ptr<cminus::memory::reference> context;
		std::vector<std::shared_ptr<cminus::memory::reference>> args;
	};

	explicit test_string(cminus::logic::runtime &runtime, const std::string &value)
		: value_(runtime.global_storage->create_string(runtime, value)){
		value_->add_attribute(runtime.global_storage->find_attribute("ReadOnly", false));
	}

	explicit test_string(cminus::logic::runtime &runtime, std::shared_ptr<cminus::memory::reference> value)
		: value_(runtime.global_storage->create_string(runtime, runtime.global_storage->get_string_data(runtime, value))){
		value_->add_attribute(runtime.global_storage->find_attribute("ReadOnly", false));
	}

	const char *data(cminus::logic::runtime &runtime) const{
		return data_(runtime, value_);
	}

	char *data(cminus::logic::runtime &runtime){
		auto clone = value_->apply_offset(runtime, 0u);
		//clone->remove_attribute("ReadOnly", true);
		return data_(runtime, clone);
	}

	std::shared_ptr<cminus::memory::reference> get_value() const{
		return value_;
	}

private:
	static char *data_(cminus::logic::runtime &runtime, std::shared_ptr<cminus::memory::reference> value){
		auto data = call_(runtime, call_info{ "data", value, });
		auto data_block = runtime.memory_object.get_block(data->read_scalar<unsigned __int64>(runtime));
		return reinterpret_cast<char *>(data_block->get_data());
	}

	static std::shared_ptr<cminus::memory::reference> call_(cminus::logic::runtime &runtime, const call_info &info){
		auto string_type = dynamic_cast<cminus::type::string *>(info.context->get_type().get());
		auto function = string_type->find(runtime, cminus::logic::storage::object::search_options{ nullptr, info.context, info.name, false });

		auto callable = dynamic_cast<cminus::memory::function_reference *>(function->get_non_raw());
		return callable->get_value()->call(runtime, callable->get_context(), info.args);
	}

	std::shared_ptr<cminus::memory::reference> value_;
};

int main(){
	std::stringstream ss("ben1080");
	std::wstringstream wss(L"ben1080");

	cminus::memory::exclusive_block eb;
	cminus::memory::object mo;

	cminus::io::stream_reader isr(std::cin);
	cminus::io::wide_stream_reader iwsr(std::wcin);

	cminus::io::stream_reader issr(ss);
	cminus::io::wide_stream_reader iwssr(wss);

	auto c = issr.read_scalar<char>();
	auto wc = issr.read_scalar<wchar_t>();

	auto c2 = iwssr.read_scalar<char>();
	auto wc2 = iwssr.read_scalar<wchar_t>();

	cminus::memory::object mem_obj;
	cminus::io::stream_writer isw(std::cout);

	auto global_storage = std::make_shared<cminus::logic::storage::global>();
	cminus::logic::runtime runtime{ 
		mem_obj,
		isw,
		global_storage,
		global_storage.get()
	};

	global_storage->init(runtime);
	cminus::declaration::variable svar(cminus::declaration::variable::attribute_list_type{}, global_storage->get_string_type(), "MyString", nullptr);
	svar.evaluate(runtime, nullptr);

	auto entry = runtime.current_storage->find(runtime, "MyString", true);
	auto data = global_storage->get_string_data(runtime, entry);

	auto str = global_storage->create_string(runtime, "CreatedString");
	auto sdat = global_storage->get_string_data(runtime, str);
	str->get_type()->print_value(runtime, str);

	test_string tstr(runtime, "TestString");
	auto tdat = tstr.data(runtime);

	test_string tstr2(runtime, str);
	auto tdat2 = tstr2.data(runtime);

	return 0;
}