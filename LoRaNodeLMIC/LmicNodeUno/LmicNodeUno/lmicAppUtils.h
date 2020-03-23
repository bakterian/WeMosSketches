
#ifndef LMIC_APP_UTILS_H
#define LMIC_APP_UTILS_H

#include <cstdint>
#include <array>
#include <algorithm>

    
namespace lmicAppUtils
{

/* 
 *   TypeAsArray
 *   Class to be used when packing a type value in to an array of bytes 
 */
template<typename T, size_t sizeofT = sizeof(T)>
class TypeAsArray
{
    public:
    TypeAsArray(T value)
    {

        initArray(value);
    }

    /*
    *   Returns the value type that was stored and presenting it in big endian
    */
    std::array<uint8_t, sizeofT> getBigEndianArray() const
    {
        std::array<uint8_t, sizeofT> resArray = uint8Array;
        
        if(isLittleEndianArch())
        {
            std::reverse(std::begin(resArray), std::end(resArray));
        }
        
        return resArray;
    }
    
    /*
    *   Returns the value type that was stored and presenting it in little endian
    */
    std::array<uint8_t, sizeofT> getLittleEndianArray() const
    {
        std::array<uint8_t, sizeofT> resArray = uint8Array;
        
        if(!isLittleEndianArch())
        {
            std::reverse(std::begin(resArray), std::end(resArray));
        }
        
        return resArray;
    }

    /*
    *   Returns the byte array converted to the LittlEndian uin32_t format 
    */
    uint32_t getTypeAsBigEndianUint32() const
    {
        std::array<uint8_t, sizeofT> resArray = getBigEndianArray();
        return (uint32_t)(*((uint32_t*)resArray.data()));
    }
    
    /*
    *   Returns the byte array converted to the BigEndian uin32_t format 
    */
    uint32_t getTypeAsLittleEndianUint32() const
    {
        std::array<uint8_t, sizeofT> resArray = getLittleEndianArray();
        return (uint32_t)(*((uint32_t*)resArray.data()));
    }
    
    /*
    *   Returns the byte array converted to the uin32_t format 
    *   (may not be a meaningful number - depends on the original type). 
    *   No endianess conversions done
    */
    uint32_t getTypeAsUint32() const
    {
        return (uint32_t)(*((uint32_t*)uint8Array.data()));
    }
    
    /*
    *   Returns the original type
    */
    T getOriginalType() const
    {
        return (T)(*((T*)(uint8Array.data())));
    }

    private:
    void initArray(T value)
    {
        const uint8_t* pValue = (const uint8_t *) &value;
        
        auto i = 0U;
    
        for(auto& arrElement : uint8Array)
        {
            arrElement = *(pValue + i);
            ++i;
        }
    }

    bool isLittleEndianArch() const
    {
        unsigned num = 1U; /* is this num stored as 01 00 00 00 (Little Endian) or 00 00 00 01 (Big Endian) */
        bool isLeastSigByteFirst = (*(char *)&num == 1);
        return isLeastSigByteFirst;
        /* since c++ 20 this staticexpres bool can be used (std::endian::native == std::endian::big) #include <bit> */
    }

    std::array<uint8_t, sizeofT> uint8Array{};
};

/* USAGE EXAMPLE
int main()
{
  TypeAsArray<float> arrayFloat(22.4f);
  std::cout << "Float value for conversion: " << arrayFloat.getOriginalType() << std::endl;
  std::cout << "Float value in big endian uint32_t: " << (unsigned)arrayFloat.getTypeAsBigEndianUint32() << std::endl;
  
  std::cout << "Float value as byte array:";
  std::string addStr = " ";
  
  for(uint8_t& floatByte : arrayFloat.getBigEndianArray())
  {
      std::cout << addStr;
      std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << (unsigned)floatByte;
      addStr = ", ";
  }
  std::cout << std::endl << std::endl << std::endl;
  
  uint32_t valueToCheck = 98569U;
  TypeAsArray<uint32_t> uintArray(valueToCheck);
  std::cout << "Unsigend value for conversion: " << std::dec << (unsigned)(uintArray.getOriginalType()) << std::endl;
  std::cout << "Unsigend value as LittleEndian uint32_t: " << std::dec << (unsigned)(uintArray.getTypeAsLittleEndianUint32())<< std::endl;
  std::cout << "Unsigend value as byte array:";
  addStr = " ";
  
  for(uint8_t& uintByte : uintArray.getBigEndianArray())
  {
      std::cout << addStr;
      std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << (unsigned)uintByte;
      addStr = ", ";
  }
  std::cout << std::endl;
}
*/


/* 
 *   TypeAsArray
 *   Class for checking if the value is within a valid range.
 *   Is naive and assumes that the T type is comparable
 */
template<typename T>
class ValueRangeChecker
{
 public:
 ValueRangeChecker(T min, T max) :
 minVal(min),
 maxVal(max)
 {};
 
 bool isWtihinRange(T value)
 {
     return ((value >= minVal) && (value <= maxVal));
 }
     
 private:
 T minVal;
 T maxVal;
};


/* 
 *   Function to verify if thr value is within the specified range
 */
template<typename T, size_t sizeofT>
bool isWithinRange(TypeAsArray<T,sizeofT> valueAsArray, T min, T max)
{
    ValueRangeChecker<T> checker(min, max);
    
    return checker.isWtihinRange(valueAsArray.getOriginalType());   
}



/*
* Function computes a crc using shifting operations
*
* Explenation on the inputPolynomial:
* This is the typical form of the input polynomial (maxim uses this poly: 100110001)
* b8*x8 + b7*x7 + b6*x6 + b5*x5 + b4*x4 + b3*x3 + b2*x2 + b1*x1 + b0*1.
* It is a fact that a 8 degree polynomial highest coefficient b8 = 1!
* Knowing this and the fact that 1 ^ 1 = 0 we can drop the MSB
* and deal with 8 byte divisor not a 9 bit divisor
*/
template<size_t arraySize>
uint8_t computeCrc8(const uint8_t initalCrc, const std::array<uint8_t,arraySize> inputArray)
{
    const uint8_t inputPolynomial = 0x31U;    
    uint8_t crc = initalCrc;

    for(auto const& input : inputArray)
    {
        crc = crc ^ input; /* XOR-in the next input byte */
        
        for (uint8_t i = 0U; i < 8U; i++)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (crc << 1U) ^ inputPolynomial;
            }
            else
            {
                crc = crc << 1U;
            }
        }
    }
    return crc;
}


/*
* Base Template function for calucating the crc8 on valueAsArray variables
*/
template<typename T>
uint8_t crc8Calculate(T valueAsArray) 
{
  return computeCrc8(0x00u, valueAsArray.getBigEndianArray());
}

/*
* Variadic template for computing crc8 on multiple valueAsArray types
*/
template<typename T, typename... Args>
uint8_t crc8Calculate(T valueAsArray, Args... args) 
{
  return computeCrc8(crc8Calculate(args...), valueAsArray.getBigEndianArray()); 
}



} /* namespace lmicAppUtils */
#endif /* LMIC_APP_UTILS_H */
