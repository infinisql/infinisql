from io import StringIO

top = """
#include <gtest/gtest.h>
#include "engine/Catalog.h"
#include "engine/Schema.h"
#include "engine/Table.h"
#include "engine/Field.h"

const size_t serdes_test_buffer_size = 1024;

"""

template = """
TEST(FieldTest, Set{test_field_type_cap}) {{
    auto c = std::make_shared<Catalog>(0, "test_catalog");
    auto s = std::make_shared<Schema>(c, "test_schema");
    auto t = std::make_shared<Table>(s, "test_table");
    Field f{{t, "test_field", Field::type_e::{field_type}}};
    
{data_type_tests}
}}

"""

numeric_template = """
TEST(FieldTest, Set{test_field_type_cap}) {{
    auto c = std::make_shared<Catalog>(0, "test_catalog");
    auto s = std::make_shared<Schema>(c, "test_schema");
    auto t = std::make_shared<Table>(s, "test_table");
    Field f{{t, "test_field", Field::type_e::{field_type}, 64}};
    
{data_type_tests}
}}

"""


field_types = [
    ("TYPE_TINYINT", ("int8_t",)),
    ("TYPE_SMALLINT", ("int8_t","int16_t",)),
    ("TYPE_INT",("int8_t","int16_t","int32_t",)),
    ("TYPE_BIGINT",("int8_t","int16_t","int32_t","int64_t")),
    ("TYPE_BOOLEAN",("bool",)),
    ("TYPE_REAL",("float",)),
    ("TYPE_DOUBLE_PRECISION",("float","double",)),
    ("TYPE_FLOAT",("float",)),
]

numeric_field_types = [
    ("TYPE_NUMERIC",("int8_t","int16_t","int32_t","int64_t")),
    ("TYPE_DECIMAL",("int8_t","int16_t","int32_t","int64_t","float","double")),
]

string_field_types = [
#         ("TYPE_CHARACTER",("int8_t",)),
    ("TYPE_CHARACTER_VARYING",("int8_t",)),
#         ("TYPE_BIT",("int8_t",)),
#         ("TYPE_BIT_VARYING",("int8_t",)),
#         ("TYPE_DATE",("int8_t",)),
#         ("TYPE_TIME",("int8_t",)),
#         ("TYPE_TIMESTAMP",("int8_t",)),
#         ("TYPE_TIME_WITH_TIME_ZONE",("int8_t",)),
#         ("TYPE_TIMESTAMP_WITH_TIME_ZONE"("int8_t",)),
]

with open("test_field.cc", "w") as out:
    out.write(top)
    for tft, test_types in field_types:
        dtt=StringIO()
        for tt in test_types:
            dtt.write("\t{\n")
            dtt.write("\tSerdes sd1{serdes_test_buffer_size}, sd2{serdes_test_buffer_size};\n")
            dtt.write("\tFieldValue fv, tv1, tv2;\n")
            dtt.write("\tbool is_null { false };\n")
            dtt.write("\t{type} rv1, rv2;\n".format(type=tt))
            dtt.write("\t{type} miv {{ std::numeric_limits<{type}>::min() }};\n".format(type=tt))
            dtt.write("\t{type} mav {{ std::numeric_limits<{type}>::max() }};\n".format(type=tt))
            dtt.write("\tfv.set(miv);\n")
            dtt.write("\tf.serValue(fv, sd1);\n")
            dtt.write("\tsd1.rewind();\n")
            dtt.write("\tf.desValue(sd1, tv1);\n")
            dtt.write("\ttv1.get(rv1, is_null);\n")
            dtt.write("\tEXPECT_EQ(miv, rv1);\n")
            dtt.write("\tEXPECT_FALSE(is_null);\n")
            dtt.write("\tfv.set(mav);\n")
            dtt.write("\tf.serValue(fv, sd2);\n")
            dtt.write("\tsd2.rewind();\n")
            dtt.write("\tf.desValue(sd2, tv2);\n")
            dtt.write("\ttv2.get(rv2, is_null);\n")
            dtt.write("\tEXPECT_EQ(mav, rv2);\n")
            dtt.write("\tEXPECT_FALSE(is_null);\n")
            dtt.write("\t}\n")
        out.write(template.format(test_field_type_cap=tft.capitalize(), field_type=tft, data_type_tests=dtt.getvalue()))
        
    for tft, test_types in numeric_field_types:
        dtt=StringIO()
        for tt in test_types:
            dtt.write("\t{\n")
            dtt.write("\tSerdes sd1{serdes_test_buffer_size}, sd2{serdes_test_buffer_size};\n")
            dtt.write("\tFieldValue fv, tv1, tv2;\n")
            dtt.write("\tbool is_null { false };\n")
            dtt.write("\tdecimal rv1, rv2;\n")
            dtt.write("\tdecimal miv {{ std::to_string(std::numeric_limits<{type}>::min()) }};\n".format(type=tt))
            dtt.write("\tdecimal mav {{ std::to_string(std::numeric_limits<{type}>::max()) }};\n".format(type=tt))
            dtt.write("\tfv.set(miv);\n")
            dtt.write("\tf.serValue(fv, sd1);\n")
            dtt.write("\tsd1.rewind();\n")
            dtt.write("\tf.desValue(sd1, tv1);\n")
            dtt.write("\ttv1.get(rv1, is_null);\n")
            dtt.write("\tEXPECT_EQ(miv, rv1);\n")
            dtt.write("\tEXPECT_FALSE(is_null);\n")
            dtt.write("\tfv.set(mav);\n")
            dtt.write("\tf.serValue(fv, sd2);\n")
            dtt.write("\tsd2.rewind();\n")
            dtt.write("\tf.desValue(sd2, tv2);\n")
            dtt.write("\ttv2.get(rv2, is_null);\n")
            dtt.write("\tEXPECT_EQ(mav.to_string(), rv2.to_string());\n")
            dtt.write("\tEXPECT_FALSE(is_null);\n")
            dtt.write("\t}\n")
        out.write(numeric_template.format(test_field_type_cap=tft.capitalize(), field_type=tft, data_type_tests=dtt.getvalue()))
        