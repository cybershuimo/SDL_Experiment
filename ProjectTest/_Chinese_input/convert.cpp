#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

// utility function for output
// void hex_print(const std::string& s)
// {
//     std::cout << std::hex << std::setfill('0');
//     for(unsigned char c : s)
//         std::cout << std::setw(2) << static_cast<int>(c) << ' ';
//     std::cout << std::dec << '\n';
// }

// int main()
// {
//     std::wstring test = L"сапоги";

//     std::wstring_convert<std::codecvt_utf16<wchar_t>> conv1;
//     std::string u8str = conv1.to_bytes(test);
//     hex_print(u8str);

//     return 1;
// }


int main()
{
	using namespace std;

	//std::wstring_convert<std::codecvt_utf16<wchar_t>> converter;
	std::string text = "中文";
	//const std::string utf16_string = converter.to_bytes(wide_string);

	cout << text << endl;
	//cout << utf16_string << endl;

	cin.get();
	return 0;
}

