#pragma once

#include "miscStdHeaders.h"
#include "core.hpp"

//TO DO: eventually template each
//TO DO: TEST
namespace AZ {
	class FlagField32 {
	
	public:
		FlagField32() { field = 0; }

		void loadField(uint32_t flagsToLoad) {
			field = flagsToLoad;
		}

		uint32_t getField(unsigned whichField) {
			return field;
		}

		bool isBitOn(unsigned bit) {
			if (bit > 31) return false; //doesn't exist: not active

			return (field & BIT(bit));
		}

		void setBitOn(int bit) {
			if (bit > 31) return; //doesn't exist

			field = field | (BIT(bit));
		}

		void setBitOff(int bit) {
			if (bit > 31) return; //doesn't exist

			field = field & (~(BIT(bit)));
		}

		/*TO DO: implement toggle
		bool toggleBit(int bit) {

		}
		*/

	private:
		uint32_t field;
	};

	class FlagField128 {

	public:
		FlagField128() { field[0] = 0; field[1] = 0; field[2] = 0; field[3] = 0;}

		bool loadField(uint32_t flagsToLoad, unsigned toWhichField) {
			if (toWhichField > 3) return false; //doesn't exist: not active

			field[toWhichField] = flagsToLoad;
			return true;
		}

		uint32_t getField(unsigned whichField) {
			if (whichField > 3) return 0; //doesn't exist: not active

			return field[whichField];
		}

		bool isBitOn(int bit) {
			if (bit > 127) return false; //doesn't exist: not active

			int element = bit / 32;
			int bitOnTheElement = bit - (element * 8);

			return (field[element] & BIT(bitOnTheElement));
		}

		void setBitOn(int bit) {
			if (bit > 127) return; //doesn't exist

			int element = bit / 32;
			int bitOnTheElement = bit - (element * 8);

			field[element] = field[element] | (BIT(bit));
		}

		void setBitOff(int bit) {
			if (bit > 127) return; //doesn't exist

			int element = bit / 32;
			int bitOnTheElement = bit - (element * 8);

			field[element] = field[element] & (~(BIT(bit)));
		}

		int getNumberOfBlocks() {
			return blocks;
		}

		size_t sizeOfBlockInBytes() {
			return sizeof(field[0]);
		}

		/*TO DO: implement toggle
		void toggleBit(int bit) {

		}
		*/

	private:
		int blocks = 4;
		uint32_t field[4];
	};
}