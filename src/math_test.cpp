#include <stdio.h>

#include "math.h"

void print_vecfail(const char *type_name, const char *test_name, f64 result, f64 expected) {
	printf("%s: %s test failed! Result: %.f, expected: %.f.\n", type_name, test_name, result, expected);
}

void test_vector2(s64 *tests, s64 *fails) {
	{
		Vector2_s8 a = { };
		Vector2_s8 b = { };
		Vector2_s8 result = { };
		Vector2_s8 expected = { };

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			result = a + b;
			expected = 7;
			if (result != expected) {
				print_fail("Vector2_s8", "addition (+)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			result = a - b;
			expected = 3;
			if (result != expected) {
				print_fail("Vector2_s8", "substraction (-)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			result = a * b;
			expected = 10;
			if (result != expected) {
				print_fail("Vector2_s8", "multiplication (*)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			result = a / b;
			expected = 2;
			if (result != expected) {
				print_fail("Vector2_s8", "division (/)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			result = a % b;
			expected = 1;
			if (result != expected) {
				print_fail("Vector2_s8", "modulo (%)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			a += b;
			result = a;
			expected = 7;
			if (result != expected) {
				print_fail("Vector2_s8", "self-addition (+=)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			a -= b;
			result = a;
			expected = 3;
			if (result != expected) {
				print_fail("Vector2_s8", "self-substraction (-=)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			a *= b;
			result = a;
			expected = 10;
			if (result != expected) {
				print_fail("Vector2_s8", "self-multiplication (*=)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			a /= b;
			result = a;
			expected = 2;
			if (result != expected) {
				print_fail("Vector2_s8", "self-division (/=)" result, expected);
				*fails += 1;
			}
		}

		{
			*tests += 1;
			a = { 5, 5 };
			b = { 2, 2 };
			a %= b;
			result = a;
			expected = 1;
			if (result != expected) {
				print_fail("Vector2_s8", "self-modulo (%=)" result, expected);
				*fails += 1;
			}
		}
	}
}

void test_vector3() {

}

void test_vector4() {

}

void test_vectors() {
	s64 tests = 0;
	s64 fails = 0;

	test_vector2(&tests, &fails);
}