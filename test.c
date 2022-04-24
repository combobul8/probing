#include <iostream>
#include <string>
#include "bytell_hash_map.hpp"

using namespace std;

int
main()
{	ska::bytell_hash_map<string, int> H;

	string key = "aback";
	H[key] = 0;

	cout << key << " -> " << H[key] << endl;
	return 0;
}
