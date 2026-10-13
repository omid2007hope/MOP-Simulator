mkdir test_dir\src
mkdir test_dir\include
echo #pragma once > test_dir\include\hdr.hpp
echo #include ^<iostream^> >> test_dir\include\hdr.hpp
echo inline void p() { std::cout ^<^< __FILE__ ^<^< std::endl; } >> test_dir\include\hdr.hpp
echo #include "../include/hdr.hpp" > test_dir\src\main.cpp
echo int main() { p(); return 0; } >> test_dir\src\main.cpp
g++ -Itest_dir\include test_dir\src\main.cpp -o test_dir\test_path.exe
.\test_dir\test_path.exe
