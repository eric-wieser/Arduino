/*
  EEPROM.h - EEPROM library
  Original Copyright (c) 2006 David A. Mellis.  All right reserved.
  New version by Christopher Andrews 2015.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROM_h
#define EEPROM_h

#include <inttypes.h>
#include <avr/eeprom.h>
#include <avr/io.h>

struct EEPtr; //Forward declaration for EERef::opreator&
struct EEBit; //Forward declaration for EERef::opreator[]

/***
    EERef class.
    
    This object references an EEPROM cell.
    Its purpose is to mimic a typical byte of RAM, however its storage is the EEPROM.
    This class has an overhead of two bytes, similar to storing a pointer to an EEPROM cell.
***/

struct EERef{

    template< typename T > EERef( T *ptr ) : index( (int) ptr ) {}
    EERef( const int index ) : index( index )                   {}
    
    //Access/read members.
    uint8_t operator*() const            { return eeprom_read_byte( (uint8_t*) index ); }
    operator uint8_t() const             { return **this; }

	EEPtr operator&() const;             //Defined below EEPtr

    //Bit access members, defined below EEBit declaration.
    EEBit operator[]( const int bidx );
    EEBit begin();
    EEBit end();
    
    //Assignment/write members.
    EERef &operator=( const EERef &ref ) { return *this = *ref; }
    EERef &operator=( uint8_t in )       { return eeprom_write_byte( (uint8_t*) index, in ), *this;  }
    EERef &operator +=( uint8_t in )     { return *this = **this + in; }
    EERef &operator -=( uint8_t in )     { return *this = **this - in; }
    EERef &operator *=( uint8_t in )     { return *this = **this * in; }
    EERef &operator /=( uint8_t in )     { return *this = **this / in; }
    EERef &operator ^=( uint8_t in )     { return *this = **this ^ in; }
    EERef &operator %=( uint8_t in )     { return *this = **this % in; }
    EERef &operator &=( uint8_t in )     { return *this = **this & in; }
    EERef &operator |=( uint8_t in )     { return *this = **this | in; }
    EERef &operator <<=( uint8_t in )    { return *this = **this << in; }
    EERef &operator >>=( uint8_t in )    { return *this = **this >> in; }
    
    EERef &update( uint8_t in )          { return  in != *this ? *this = in : *this; }
    
    // Prefix increment/decrement
    EERef& operator++()                  { return *this += 1; }
    EERef& operator--()                  { return *this -= 1; }
    
    // Postfix increment/decrement
    uint8_t operator++ (int){ 
        uint8_t ret = **this;
        return ++(*this), ret;
    }

    uint8_t operator-- (int){ 
        uint8_t ret = **this;
        return --(*this), ret;
    }
    
    int index; //Index of current EEPROM cell.
};


/***
    EEBit class.
    This object is a reference object similar to EERef, however it references
    only a single bit. Its function mimics a bool type.
***/

struct EEBit{

    //Constructor, use by passing in index of EEPROM byte, then index of bit to read.
    EEBit( int index, uint8_t bidx )
        : ref( index ), mask( 0x01 << bidx ) {}

    //Modifier functions.
    EEBit &setIndex( uint8_t bidx )          { return mask = (0x01 << bidx), *this; }
    EEBit &set()                             { return *this = true; }
    EEBit &clear()                           { return *this = false; }

    //Read/write functions.
    operator bool() const                    { return ref & mask; }
    EEBit &operator =( const EEBit &copy )   { return *this = ( const bool ) copy; }

    EEBit &operator =( const bool &copy ){
        if( copy )  ref |= mask;
        else  ref &= ~mask;
        return *this;
    }

    //Iterator functionality.
    EEBit& operator*()                  { return *this; }
    bool operator==( const EEBit &bit ) { return (mask == bit.mask) && (ref.index == bit.ref.index); }
    bool operator!=( const EEBit &bit ) { return !(*this == bit); }

    //Prefix & Postfix increment/decrement
    EEBit& operator++(){
        if( mask & 0x80 ){
            ++ref.index;
            mask = 0x01;
        }else{
            mask <<= 1;
        }
        return *this;
    }

    EEBit& operator--(){
        if( mask & 0x01 ){
            --ref.index;
            mask = 0x80;
        }else{
            mask >>= 1;
        }
        return *this;
    }

    EEBit operator++ (int) {
        EEBit cpy = *this;
        return ++(*this), cpy;
    }

    EEBit operator-- (int) {
        EEBit cpy = *this;
        return --(*this), cpy;
    }

    EERef ref;     //Reference to EEPROM cell.
    uint8_t mask;  //Mask of bit to read/write.
};

//Deferred definition till EEBit becomes available.
inline EEBit EERef::operator[]( const int bidx ) { return EEBit( index, bidx ); }
inline EEBit EERef::begin()                      { return EEBit( index, 0 ); }
inline EEBit EERef::end()                        { return EEBit( index + 1, 0 ); }


/***
    EEPtr class.
    
    This object is a bidirectional pointer to EEPROM cells represented by EERef objects.
    Just like a normal pointer type, this can be dereferenced and repositioned using 
    increment/decrement operators.
***/

struct EEPtr{

    template< typename T > EEPtr( T *ptr ) : index( (int) ptr ) {}    
    EEPtr( const int index ) : index( index )                   {}

	//Pointer read/write.
    operator int() const                { return index; }
    EEPtr &operator=( int in )          { return index = in, *this; }
	EERef operator[]( int idx )         { return index + idx; }
    
    //Iterator functionality.
    bool operator!=( const EEPtr &ptr ) { return index != ptr.index; }
    EEPtr& operator+=( int idx )        { return index += idx, *this; }
    EEPtr& operator-=( int idx )        { return index -= idx, *this; }

    //Dreference & member access.
	EERef operator*()                   { return index; }
    EERef *operator->()                 { return (EERef*) this; }

    // Prefix & Postfix increment/decrement
    EEPtr& operator++()                 { return ++index, *this; }
    EEPtr& operator--()                 { return --index, *this; }
    EEPtr operator++ (int)              { return index++; }
    EEPtr operator-- (int)              { return index--; }

    int index; //Index of current EEPROM cell.
};

inline EEPtr EERef::operator&() const { return index; } //Deferred definition till EEPtr becomes available.

/***
    EEPROMClass class.
    
    This object represents the entire EEPROM space.
    It wraps the functionality of EEPtr and EERef into a basic interface.
    This class is also 100% backwards compatible with earlier Arduino core releases.
***/

class EEPROMClass{
	protected:

		/***
			EEIterator interface.
			This interface allows creating customized ranges within
			the EEPROM. Essentially intended for use with ranged for
			loops, or STL style iteration on subsections of the EEPROM.
		***/

		struct EEIterator{
			EEIterator( EEPtr _start, int _length ) : start(_start), length(_length) {}
			EEPtr begin() { return start; }
			EEPtr end()   { return start + length; }
			EEPtr start;
			int length;
		};

	public:

		//Basic user access methods.
		EERef operator[]( EERef ref )         { return ref; }
		EERef read( EERef ref )               { return ref; }
		void write( EERef ref, uint8_t val )  { ref = val; }
		void update( EERef ref, uint8_t val ) { ref.update( val ); }

		//STL and C++11 iteration capability.
		EEPtr begin()                        { return 0x00; }
		EEPtr end()                          { return length(); } //Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
		uint16_t length()                    { return E2END + 1; }

		//Extended iteration functionality (arbitrary regions).
		//These can make serialized reading/writing easy.
		template< typename T > EEIterator iterate( T *t ) { return EEIterator( t, sizeof(T) ); }
		EEIterator iterate( EEPtr ptr, int length )       { return EEIterator( ptr, length ); }

		//Bit access methods.
		EEBit readBit( EERef ref, uint8_t bidx )                 { return ref[ bidx ]; }
		void writeBit( EERef ref, uint8_t bidx, const bool val ) { ref[ bidx ] = val; }

		//A helper function for the builtin eeprom_is_ready macro.
		bool ready()                         { return eeprom_is_ready(); }

		//Functionality to 'get' and 'put' objects to and from EEPROM.
		template< typename T > T &get( EEPtr ptr, T &t ){
			uint8_t *dest = (uint8_t*) &t;
			for( int count = sizeof(T) ; count ; --count, ++ptr ) *dest++ = *ptr;
			return t;
		}

		template< typename T > const T &put( EEPtr ptr, const T &t ){
			const uint8_t *src = (const uint8_t*) &t;
			for( int count = sizeof(T) ; count ; --count, ++ptr ) (*ptr).update( *src++ );
			return t;
		}
};

static EEPROMClass EEPROM;
#endif