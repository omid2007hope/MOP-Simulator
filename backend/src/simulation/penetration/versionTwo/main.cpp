// ! Clears input stream errors and flushes invalid buffer characters
void safeCin() {
	if (!std::cin) {
		std::cin.clear();
	}

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {

	return 0;
};
