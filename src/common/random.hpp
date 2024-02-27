
#pragma once

#include "core.hpp"

// https://stackoverflow.com/a/35104126
class Random {

	private:

		uint32_t a, c, v;

		void init(unsigned seed, unsigned a, unsigned c) {
			this->a = a;
			this->c = c;
			this->v = seed;
		}

	public:

		Random()
		: a(0), c(0), v(0) {}

	public:

		void init(std::mt19937& generator) {
			std::uniform_int_distribution<std::mt19937::result_type> rand_dist(1, 200000);

			int ra = rand_dist(generator);
			int rb = rand_dist(generator);
			int rc = rand_dist(generator);

			init(ra, rb * 4 + 1, rc * 2 + 1);
		}

		uint32_t next() {
		    return (v = a * v + c);
		}

};
