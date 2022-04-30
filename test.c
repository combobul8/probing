#include <iostream>
#include <string>
#include "bytell_hash_map.hpp"

using namespace std;

int
main()
{	ska::bytell_hash_map<string, int> H;

	auto key = "aback";
	// auto key = 0;
	H[key] = 0;

	auto val_returned = H[key];
	cout << key << " -> " << val_returned << endl;
	return 0;
}
