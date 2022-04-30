#include <iostream>
#include <string>
#include "bytell_hash_map.hpp"

using namespace std;

int
main()
{	ska::bytell_hash_map<string, int> H;

	{	auto key = "aback";
		// auto key = 0;
		cout << key << endl;
		H[key] = 0;

	}
	{	auto key = "abacus";
		// auto key = 0;
		cout << key << endl;
		H[key] = 1;
	}
	{	auto key = "abandon";
		// auto key = 0;
		cout << key << endl;
		H[key] = 2;
	}
	{	auto key = "abase";
		// auto key = 0;
		cout << key << endl;
		H[key] = 3;
	}
	{	auto key = "abashed";
		// auto key = 0;
		cout << key << endl;
		H[key] = 4;
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
