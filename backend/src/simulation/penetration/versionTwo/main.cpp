#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

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


int main(int argc, char* argv[]) {

	int choice = 1;
	bool jsonMode = false;
	std::vector<std::string> jsonFiles;

	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--json-input" && i + 1 < argc) {
			jsonMode = true;
			jsonFiles.push_back(argv[++i]);
		}
	}

	if (argc > 0) {
		std::error_code ec;

		std::filesystem::path current =
			std::filesystem::absolute(argv[0], ec).parent_path();

		while (!current.empty() && current != current.root_path()) {
			if (std::filesystem::exists(current / "data" / "targets.json", ec)) {
				basePath = current.string();
				break;
			}
			current = current.parent_path();
		}
	}


	return 0;
}
