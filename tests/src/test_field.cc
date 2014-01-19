#include <limits>
#include <gtest/gtest.h>
#include "Field.h"


TEST(FieldValueTest, Setint8_t) {
    FieldValue fv;
    int8_t miv { std::numeric_limits<int8_t>::min() },
     	   mav { std::numeric_limits<int8_t>::max() };
    int8_t tv  {0};
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}



TEST(FieldValueTest, Setint16_t) {
    FieldValue fv;
    int16_t miv { std::numeric_limits<int16_t>::min() },
     	    mav { std::numeric_limits<int16_t>::max() };
    int16_t tv { 0 };
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}



TEST(FieldValueTest, Setint32_t) {
    FieldValue fv;
    int32_t miv { std::numeric_limits<int32_t>::min() },
     	    mav { std::numeric_limits<int32_t>::max() };
    int32_t tv  { 0 };
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}



TEST(FieldValueTest, Setint64_t) {
    FieldValue fv;
    int64_t miv { std::numeric_limits<int64_t>::min() },
     	    mav { std::numeric_limits<int64_t>::max() };
    int64_t tv { 0 };
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}



TEST(FieldValueTest, Setfloat) {
    FieldValue fv;
    float miv { std::numeric_limits<float>::min() },
     	  mav { std::numeric_limits<float>::max() };
    float tv  { 0 };
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}



TEST(FieldValueTest, Setdouble) {
    FieldValue fv;
    double miv { std::numeric_limits<double>::min() },
    	   mav { std::numeric_limits<double>::max() };
    double tv = 0;
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}



TEST(FieldValueTest, Setbool) {
    FieldValue fv;
    bool miv { true }, mav { false };
    bool tv  { false };
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);
    
    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);    
    ASSERT_FALSE(is_null);
}

TEST(FieldValueTest, Setstring) {
    FieldValue fv;
    std::string miv{ "a" }, mav { "a longer string" };
    std::string tv{};
    bool is_null { false };

    fv.set(miv);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, miv);
    ASSERT_FALSE(is_null);

    fv.set(mav);
    fv.get(tv, is_null);
    ASSERT_EQ(tv, mav);
    ASSERT_FALSE(is_null);
}

TEST(FieldValueTest, SetNull) {
	FieldValue fv;
	fv.nullify();
	ASSERT_TRUE(fv.getnull());
}
