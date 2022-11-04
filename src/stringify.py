import re

output = "#include \"enum.h\"\nstruct PtpEnum ptp_enums[] = {\n"

string = open("ptp.h", "r").read()
string = string + open("canon.h", "r").read()

regex = r"#define PTP_(OC|PC|OF)_(CANON|FUJI|NIKON|SONY|)[_]?([A-Za-z0-9_]+)[\t ]+(0x[0-9a-fA-Z]+)"
matches = re.findall(regex, string)
for i in matches:
    vendor = i[1]
    if i[1] == "":
        vendor = "GENERIC"
    output += "{PTP_" + i[0] + ", PTP_VENDOR_" + vendor + ", " + "\"" + i[2] + "\", " + i[3] + "},\n"
output += "\n};"
print("Compiled", len(matches), "enums")
f = open("enum.c", "w")
f.write(output)
f.close()
