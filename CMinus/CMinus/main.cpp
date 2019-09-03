#include <sstream>

#include "io/io_stream_reader.h"
#include "io/io_stream_writer.h"
#include "type/string_type.h"

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
		global_storage
	};

	global_storage->init(runtime);
	cminus::declaration::variable svar(cminus::declaration::variable::attribute_list_type{}, global_storage->get_string_type(), "MyString", nullptr);
	svar.evaluate(runtime, nullptr);

	auto entry = runtime.current_storage->find(runtime, "MyString", true);
	auto data = global_storage->get_string_data(runtime, entry);

	return 0;
}