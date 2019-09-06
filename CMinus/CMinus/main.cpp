#include <sstream>

#include "io/io_stream_reader.h"
#include "io/io_stream_writer.h"

#include "adapters/string_adapter.h"

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

	cminus::adapter::string tstr(runtime, "TestString");
	auto tdat = tstr.data();
	auto tsz = tstr.size();
	auto temp = tstr.empty();
	auto tat = tstr.at(3);
	tstr.clear();
	tdat = tstr.data();
	tsz = tstr.size();
	temp = tstr.empty();
	
	cminus::adapter::string tstr2(runtime, str);
	auto tdat2 = tstr2.data();
	auto tsz2 = tstr2.size();
	auto temp2 = tstr2.empty();

	tstr2.swap(tstr);

	tdat = tstr.data();
	tsz = tstr.size();
	temp = tstr.empty();

	tdat2 = tstr2.data();
	tsz2 = tstr2.size();
	temp2 = tstr2.empty();

	return 0;
}