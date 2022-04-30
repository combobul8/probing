#include <iostream>
#include <string>
#include "sep_probing.hpp"

using namespace std;

int
main()
{	Table<string, 8> H;

	{	auto key = "aback";
		// auto key = 0;
		cout << key << endl;
		// H[key] = 0;
	}
	{	auto key = "abacus";
		cout << key << endl;
		// H[key] = 1;
	}
	{	auto key = "abandon";
		cout << key << endl;
		// H[key] = 2;
	}
	{	auto key = "abase";
		cout << key << endl;
		// H[key] = 3;
	}
	{	auto key = "abashed";
		cout << key << endl;
		// H[key] = 4;
	}
	/* cout << "---" << endl;
	{	auto key = "aback";
		auto val_returned = H[key];
		cout << key << " -> " << val_returned << endl;
	}
	{	auto key = "abacus";
		auto val_returned = H[key];
		cout << key << " -> " << val_returned << endl;
	} */
	return 0;
}
