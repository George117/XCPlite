import sys

def generate_asap(number):
    # source file
    source_file = open("asap.c", "w")
    source_file.write("""#include "main.h"
#include "main_cfg.h"
#include "platform.h"
#include "xcpLite.h"
#include "A2L.h"
#include "ecu.h"
#include "asap.h"

extern gXcpEvent_EcuCyclic;

// Create demo A2L file 
void ecuCreateA2lDescription() {
    // Measurements
    A2lSetEvent(gXcpEvent_EcuCyclic); // Associate XCP event "EcuCyclic" to the variables created below\n""")

    for i in range(0, number):
        source_file.write("\tA2lCreatePhysMeasurement(channel_{}, A2L_TYPE_DOUBLE, \"channel_{}\", 1.0, 0.0, \"\");\n".format(i, i))
    source_file.write("}\n\n")

    source_file.write("""void updateVars() {\n\n""")

    for i in range(0, number):
        source_file.write("\tchannel_{}++;\n".format(i))
    source_file.write("""}\n""")

    source_file.close()

    # header
    header_file = open("asap.h", "w")
    header_file.write("\nvoid updateVars();\n")
    for i in range(0, number):
        header_file.write("double channel_{} = 0;\n".format(i))
    header_file.close()


if __name__== "__main__":
    print("Generating {} dummy ASAPS".format(sys.argv[1]))
    generate_asap(int(sys.argv[1]))