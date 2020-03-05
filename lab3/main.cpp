#include <iostream>

template<typename T, unsigned exp>
struct Pow {
	constexpr static T pow(T n) {
		return n * Pow<T, exp - 1>::pow(n);
	}
};

template<typename T>
struct Pow<T, 1> {
	constexpr static T pow(T n) {
		return n;
	}
};

template<typename T>
struct Pow<T, 0> {
	constexpr static T pow(T n) {
		return 1;
	}
};

/*
template<typename T, T val, unsigned exp>
struct Pow {
	constexpr static T value = val * Pow<T, val, exp - 1>::value;
};

template<typename T, T val>
struct Pow<T, val, 1> {
	constexpr static T value = val;
};

template<typename T, T val>
struct Pow<T, val, 0> {
	constexpr static T value = 1;
};
*/

int main() {
	std::cout << Pow<int, 4>::pow(16) << '\n';
	//std::cout << Pow<int, 16, 4>::value << '\n';
}
