#pragma once

#include "miscStdHeaders.h"
#include "core.hpp"

//TODO-CRITICAL: TESTS
//TODO: template
//TODO: Rethink counting method: 
//-- https://en.cppreference.com/w/cpp/numeric/popcount
//--- But Byte-At-A-Time Lookup Table seems to be within ~17% of native popcount
//---- ( https://github.com/intvnut/bit_count )

namespace AZ {

	typedef struct field_st {
		uint32_t flags = 0;
		int8_t areOn = 0;
	} field_t;

	class FlagField32 {
	
	public:
		FlagField32() { }

		void loadField(uint32_t flagsToLoad) {
			field.flags = flagsToLoad;
			updateHowManyAreOn();
		}

		uint32_t getField() {
			return field.flags;
		}

		void updateHowManyAreOn() {
			field.areOn = 0;

			for (int i = 0; i < (sizeof(field) * 8); i++) {
					field.areOn += ((field.flags & BIT(i)) != 0);
			}
		}
		
		int howManyAreOn() {
			return field.areOn;
		}

		bool isBitOn(unsigned bit) {
			if (bit > 31) return false; //doesn't exist: not active

			return (field.flags & BIT(bit));
		}

		void setBitOn(int bit) {
			if (bit > 31) return; //doesn't exist

			field.areOn += ~(field.flags & BIT(bit));

			field.flags = field.flags | (BIT(bit));
		}

		void setBitOff(int bit) {
			if (bit > 31) return; //doesn't exist

			field.areOn -= field.flags & BIT(bit);

			field.flags = field.flags & (~(BIT(bit)));
		}

		/*TODO: implement toggle
		bool toggleBit(int bit) {

		}
		*/

	private:
		field_t field;
	};

	class FlagField128 {

	public:
		FlagField128() { }

		bool loadField(uint32_t flagsToLoad, unsigned toWhichField) {
			if (toWhichField > 3) return false; //doesn't exist: not active

			field[toWhichField].flags = flagsToLoad;

			updateHowManyAreOn();

			return true;
		}

		uint32_t getField(unsigned whichField) {
			if (whichField > 3) return 0; //doesn't exist: not active

			return field[whichField].flags;
		}

		void updateHowManyAreOn() {
			areOn = 0;
			for (int i = 0; i < blocks; i++) {
				updateHowManyAreOnBlock(i);
				areOn += field[i].areOn;
			}
		}

		int howManyAreOn() {
			return areOn;
		}

		bool isBitOn(int bit) {
			if (bit > 127) return false; //doesn't exist: not active

			int element = bit / 32;
			int bitOnTheElement = bit - (element * 8);

			return (field[element].flags & BIT(bitOnTheElement));
		}

		void setBitOn(int bit) {
			if (bit > 127) return; //doesn't exist

			int element = bit / 32;
			int bitOnTheElement = bit - (element * 8);

			field[element].areOn += ~(field[element].flags & BIT(bit));

			field[element].flags = field[element].flags | (BIT(bit));
		}

		void setBitOff(int bit) {
			if (bit > 127) return; //doesn't exist

			int element = bit / 32;
			int bitOnTheElement = bit - (element * 8);

			field[element].areOn -= field[element].flags & BIT(bit);

			field[element].flags = field[element].flags & (~(BIT(bit)));
		}

		int getNumberOfBlocks() {
			return blocks;
		}

		size_t sizeOfBlockInBytes() {
			return sizeof(field[0]);
		}

		/*TODO: implement toggle
		void toggleBit(int bit) {

		}
		*/

	private:
		int blocks = 4;
		field_t field[4];
		int areOn = 0;

		void updateHowManyAreOnBlock(unsigned whichField) {
			field[whichField].areOn = 0;

			for (int j = 0; j < (sizeof(field[0]) * 8); j++) {
				field[whichField].areOn += ( (field[whichField].flags & BIT(j)) != 0);
			}
		}	
	};
}