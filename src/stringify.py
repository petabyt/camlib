import re

output = "#include \"enum.h\"\nstruct PtpEnum ptp_enums[] = {\n"

string = open("ptp.h", "r").read()
string = string + open("canon.h", "r").read()

regex = r"#define ([A-Za-z0-9_]+)[ \t](0x[0-9a-fA-Z]+)"
matches = re.findall(regex, string)
for i in matches:
    t = re.findall(r"PTP_(OC|PC|OF|EC|RC)_(CANON|FUJI|NIKON|SONY|)[_]?([A-Za-z0-9_]+)", i[0])
    if len(t) == 0:
        output += "{PTP_ENUM, 0, \"" + i[0] + "\", " + i[1] + "},\n"
    else:
        vendor = t[0][1]
        if t[0][1] == "":
            vendor = "GENERIC"
        output += "{PTP_" + t[0][0] + ", PTP_VENDOR_" + vendor + ", " + "\"" + t[0][2] + "\", " + i[1] + "},\n"
output += "\n};"

output += "int ptp_enums_length = " + str(len(matches)) + ";\n"

print("Compiled", len(matches), "enums")
f = open("data.c", "w")
f.write(output)
f.close()
