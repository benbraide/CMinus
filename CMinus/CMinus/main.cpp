#include <sstream>

#include "memory/memory_object.h"
#include "memory/memory_reference.h"
#include "io/io_stream_reader.h"

int main(){
	std::stringstream ss("ben1080");
	std::wstringstream wss(L"ben1080");

	cminus::memory::composite_block cb;
	cminus::memory::object mo;

	cminus::io::stream_reader isr(std::cin);
	cminus::io::wide_stream_reader iwsr(std::wcin);

	cminus::io::stream_reader issr(ss);
	cminus::io::wide_stream_reader iwssr(wss);

	auto c = issr.read_scalar<char>();
	auto wc = issr.read_scalar<wchar_t>();

	auto c2 = iwssr.read_scalar<char>();
	auto wc2 = iwssr.read_scalar<wchar_t>();

	return 0;
}