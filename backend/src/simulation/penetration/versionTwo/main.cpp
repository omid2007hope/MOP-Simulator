#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

void safeCin() {
	if (!std::cin) {
		std::cin.clear();
	}

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template <typename T>
T getValidInput(const std::string& prompt) {
	std::string input;
	T data;

	while (true) {
		std::cout << prompt;

		if (!std::getline(std::cin, input)) {
			std::cerr << "\n[!] Input stream closed. Exiting.\n";

			std::exit(1);
		}

		std::istringstream iss(input);
		char extra;

		if ((iss >> data) && !(iss >> extra)) {
			return data;
		}

		std::cout << "Invalid input! Please enter a valid value.\n";
	}
}


int main() {

	return 0;
};
