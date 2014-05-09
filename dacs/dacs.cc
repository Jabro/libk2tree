extern "C" {
  #include "src/directcodes.h"
}
#include <fstream>

/* 
 * Saves a value into an ofstream.
 */
template <typename T>
void SaveValue(std::ofstream *out, T val) {
  out->write(reinterpret_cast<char *>(&val), sizeof(T));
}

/* 
 * Loads a value from an istream.
 */
template <typename T>
T LoadValue(std::ifstream *in) {
  T ret;
  in->read(reinterpret_cast<char *>(&ret), sizeof(T));
  return ret;
}

/* 
 * Saves len values into an ofstream.
 */
template <typename T>
void SaveValue(std::ofstream *out, T *val, size_t length) {
  out->write(reinterpret_cast<char *>(val), length * sizeof(T));
}

/* 
 * Loads len values from an istream.
 */
template <typename T>
T *LoadValue(std::ifstream *in, size_t length) {
  T *ret = new T[length];
  in->read(reinterpret_cast<char *>(ret), length * sizeof(T));
  return ret;
}


void save_bitrank(bitRankW32Int * br, std::ofstream *out) {
	uint s,n;
	s=br->s;
	n=br->n;
  SaveValue(out, n);
  SaveValue(out, br->factor);
  SaveValue(out, br->data, n/W+1);
  SaveValue(out, br->Rs,n/s+1);
}

void load_bitrank(bitRankW32Int * br, std::ifstream *in) {
  br->n = LoadValue<uint>(in);
  br->b=32;    
  uint b=br->b;                      // b is a word
  br->factor = LoadValue<uint>(in);
  br->s=b*br->factor;
  uint s=br->s;
  uint n= br->n;
  br->integers = n/W;
  br->data= (uint *) malloc(sizeof( uint) *(n/W+1));

  in->read(reinterpret_cast<char *>(br->data),sizeof(uint)*br->n/W+1);
  br->owner = 1;
  br->Rs=(uint*)malloc(sizeof(uint)*(n/s+1));
  in->read(reinterpret_cast<char *>(br->Rs),sizeof(uint)*n/s+1);
}

void SaveFT(std::ofstream *out, FTRep *rep) {
	SaveValue(out, rep->listLength);
	SaveValue(out, rep->nLevels);
	SaveValue(out, rep->tamCode);
	SaveValue(out, rep->tamtablebase);
	SaveValue(out, rep->tablebase, rep->tamtablebase);	
	SaveValue(out, rep->base_bits, rep->nLevels);
	SaveValue(out, rep->base, rep->nLevels);
	SaveValue(out, rep->levelsIndex, rep->nLevels+1);
	SaveValue(out, rep->iniLevel, rep->nLevels);
	SaveValue(out, rep->rankLevels, rep->nLevels);

	SaveValue(out, rep->levels, rep->tamCode/W+1);

	save_bitrank(rep->bS, out);
}

FTRep* LoadFT(std::ifstream *in) {
	FTRep * rep = (FTRep *) malloc(sizeof(struct sFTRep));
	rep->listLength = LoadValue<uint>(in);
	rep->nLevels = LoadValue<byte>(in);
	rep->tamCode = LoadValue<uint>(in);
	
	rep->tamtablebase = LoadValue<uint>(in);
	rep->tablebase = (uint *) malloc(sizeof(uint)*rep->tamtablebase);
	in->read(reinterpret_cast<char *>(rep->tablebase), sizeof(uint)*rep->tamtablebase);	
	
	rep->base_bits = (ushort *) malloc(sizeof(ushort)*rep->nLevels);
	in->read(reinterpret_cast<char *>(rep->base_bits),sizeof(ushort)*rep->nLevels);
	
	
	rep->base = (uint *) malloc(sizeof(uint)*rep->nLevels);
	in->read(reinterpret_cast<char *>(rep->base),sizeof(uint)*rep->nLevels);
	
	
	rep->levelsIndex = (uint *) malloc(sizeof(uint)*(rep->nLevels+1));
	in->read(reinterpret_cast<char *>(rep->levelsIndex),sizeof(uint)*rep->nLevels+1);
	
	rep->iniLevel = (uint *) malloc(sizeof(uint)*rep->nLevels);
	in->read(reinterpret_cast<char *>(rep->iniLevel),sizeof(uint)*rep->nLevels);

	rep->rankLevels = (uint *) malloc(sizeof(uint)*(rep->nLevels));
	in->read(reinterpret_cast<char *>(rep->rankLevels),sizeof(uint)*rep->nLevels);
	
	rep->levels = (uint *) malloc(sizeof(uint)*(rep->tamCode/W+1));	
	in->read(reinterpret_cast<char *>(rep->levels),sizeof(uint)*(rep->tamCode/W+1));
		
	
	rep->bS = (bitRankW32Int *) malloc(sizeof(struct sbitRankW32Int));
	load_bitrank(rep->bS, in);	
	
	
	return rep;
}
