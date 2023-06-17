include(AddGTest)

AddGTest(NAME orion_math_abs FILENAME abs.cpp DEPS orion::math)
AddGTest(NAME orion_math_sqrt FILENAME sqrt.cpp DEPS orion::math)
AddGTest(NAME orion_math_vector FILENAME vector.cpp DEPS orion::math)
AddGTest(NAME orion_math_matrix FILENAME matrix.cpp DEPS orion::math)
AddGTest(NAME orion_math_angles FILENAME angles.cpp DEPS orion::math)
AddGTest(NAME orion_math_trig FILENAME trig.cpp DEPS orion::math)
AddGTest(NAME orion_math_transformation FILENAME transformation.cpp DEPS orion::math)
