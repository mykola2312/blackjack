import re
import sys
import xml.etree.ElementTree as ET
from enum import Enum

class InstructionType(Enum):
    STANDARD = 0
    VEX = 1
    EVEX = 2

    def __str__(self):
        if self == InstructionType.STANDARD: return "std"
        elif self == InstructionType.VEX: return "vex"
        elif self == InstructionType.EVEX: return "evex"

    def value(self):
        if self == InstructionType.STANDARD: return 0
        elif self == InstructionType.VEX: return 1
        elif self == InstructionType.EVEX: return 2

class Instruction:
    RT_TARGETS = {
        "90"    : 1,
        "C3"    : 2,
        "C2 iw" : 3,
        "CC"    : 4,
        "CD ib" : 5,
        "0F 34" : 6,
        "0F 05" : 7,
    }

    def __init__(self, ins):
        self._opc = ins.find("opc").text
        self.x32m = ins.attrib["x32m"]
        self.x64m = ins.attrib["x64m"]
        self.mnemonic = ins.find("mnem").text
        
        self.bytes = None

    def get_type(self):
        pass

    def has_rex(self):
        return False

    def has_digit(self):
        return False
    
    def has_modrm(self):
        return False

    def has_imm(self):
        return False

    def has_value(self):
        return False

    def has_opreg(self):
        return False

    def encode_rt_target(self):
        return Instruction.RT_TARGETS.get(self._opc, 0)

    def __str__(self):
        return f"<{self.get_type()}> {self.mnemonic} bytes {self.bytes} rex {self.has_rex()} digit {self.has_digit()} modrm {self.has_modrm()} imm {self.has_imm()} value {self.has_value()} opreg {self.has_opreg()}"

class InstructionCommon:
    REX_REGEX = re.compile("^REX\\.(.)")
    BYTES_REGEX = re.compile("([0-9A-F][0-9A-F])")
    DIGIT_REGEX = re.compile("\\/(\\d)")
    MODRM_REGEX = re.compile("\\/r")
    IMM_REGEX = re.compile("i([b,w,d,o])")
    VALUE_REGEX = re.compile("c([b,w,d,p,o,t])")
    OPREG_REGEX = re.compile("r([b,w,d,o])")

class StandardInstruction(Instruction):
    def __init__(self, ins):
        super().__init__(ins)

        rex = InstructionCommon.REX_REGEX.search(self._opc)
        bytes = InstructionCommon.BYTES_REGEX.findall(self._opc)
        digit = InstructionCommon.DIGIT_REGEX.search(self._opc)
        modrm = InstructionCommon.MODRM_REGEX.search(self._opc)
        imm = InstructionCommon.IMM_REGEX.search(self._opc)
        value = InstructionCommon.VALUE_REGEX.search(self._opc)
        opreg = InstructionCommon.OPREG_REGEX.search(self._opc)
        self.bytes = bytes

        self.rex = None
        self.digit = None
        self.modrm = False
        self.imm = None
        self.value = None
        self.opreg = None

        if rex: self.rex = rex.group(1)
        if digit: self.digit = int(digit.group(1))
        if modrm: self.modrm = True
        
        if imm:
            self.imm = imm.group(1)
        elif "imm8" in self._opc: # because intel keeps breaking their own convention
            self.imm = "b"
        
        if value: self.value = value.group(1)
        if opreg: self.opreg = opreg.group(1)
    
    def get_type(self):
        return InstructionType.STANDARD

    def has_rex(self):
        return self.rex is not None

    def has_digit(self):
        return self.digit is not None

    def has_modrm(self):
        return self.modrm or (self.digit is not None)

    def has_imm(self):
        return self.imm is not None

    def has_value(self):
        return self.value is not None

    def has_opreg(self):
        return self.opreg is not None

class VEXInstruction(Instruction):
    def __init__(self, ins):
        super().__init__(ins)

        # fix string because intel employees keep bashing keyboard with random keys
        self._opc = re.sub(r"\. ", ".", self._opc)

        parts = self._opc.split(" ")
        (vex, opc) = (parts[0], "".join(parts[1:]))
        vex_parts = vex.split(".")
        
        self.lig = False
        if "128" in vex_parts or "L0" in vex_parts or "LZ" in vex_parts:
            self.l = 128
        elif "256" in vex_parts or "L1" in vex_parts:
            self.l = 256
        elif "LIG" in vex_parts:
            self.l = 0
            self.lig = True
        else: raise RuntimeError("VEX.L is unknown!")
 
        self.wig = False
        if "W0" in vex_parts: self.w = False
        elif "W1" in vex_parts: self.w = True
        elif "WIG" in vex_parts:
            self.wig = True
            self.w = False
        else: self.w = False # just default it to False, it's not a big deal

        self.bytes = InstructionCommon.BYTES_REGEX.findall(opc)
        
        modrm = InstructionCommon.MODRM_REGEX.search(opc)
        imm = InstructionCommon.IMM_REGEX.search(opc)

        self.modrm = True if modrm else False
        self.imm = imm.group(1) if imm else None
    
    def get_type(self):
        return InstructionType.VEX

    def has_modrm(self):
        return self.modrm
    
    def has_imm(self):
        return self.imm is not None

class EVEXInstruction(Instruction):
    def __init__(self, ins):
        super().__init__(ins)

        # fix string because intel employees keep bashing keyboard with random keys
        self._opc = re.sub(r"\. ", ".", self._opc)

        parts = self._opc.split(" ")
        (evex, opc) = (parts[0], "".join(parts[1:]))
        evex_parts = evex.split(".")

        print(evex, opc)

        self.lig = False
        if "128" in evex_parts: self.l = 128
        elif "256" in evex_parts: self.l = 256
        elif "512" in evex_parts: self.l = 512
        elif "LIG" in evex_parts or "LLIG" in evex_parts:
            self.l = 0
            self.lig = True
        else: raise RuntimeError("EVEX.L and EVEX.LIG is unknown!")
        
        self.wig = False
        if "W0" in evex_parts: self.w = False
        elif "W1" in evex_parts: self.w = True
        elif "WIG" in evex_parts:
            self.w = False
            self.wig = True
        else: self.w = False

        self.bytes = InstructionCommon.BYTES_REGEX.findall(opc)
        
        modrm = InstructionCommon.MODRM_REGEX.search(opc)
        imm = InstructionCommon.IMM_REGEX.search(opc)
        
        self.modrm = True if modrm else False
        self.imm = imm.group(1) if imm else None
    
    def get_type(self):
        return InstructionType.EVEX

    def has_modrm(self):
        return self.modrm

    def has_imm(self):
        return self.imm is not None

def parse_instruction(ins):
    opc = ins.find("opc").text
    if "EVEX" in opc: return EVEXInstruction(ins)
    elif "VEX" in opc: return VEXInstruction(ins)
    else: return StandardInstruction(ins)

class InstructionGroup:
    def __init__(self, common):
        self.brief = common.find("brief").text
        self.instructions = [parse_instruction(ins) for ins in common.iter("ins")]

def parse_file(path):
    tree = ET.parse(path)
    root = tree.getroot()

    groups = [InstructionGroup(common) for common in root.iter("common")]
    return groups

def convert_rex(rex):
    if rex == "B": return 0
    elif rex == "X": return 1
    elif rex == "R": return 2
    elif rex == "W": return 3
    else: raise RuntimeError(f"convert_rex failed for {rex}")

def convert_imm(imm):
    if imm == "b": return 0
    elif imm == "w": return 1
    elif imm == "d": return 2
    elif imm == "o": return 3
    else: raise RuntimeError(f"convert_imm failed for {imm}")

def convert_value(value):
    if value == "b": return 0
    elif value == "w": return 1
    elif value == "d": return 2
    elif value == "p": return 3
    elif value == "o": return 4
    elif value == "t": return 5
    else: raise RuntimeError(f"convert_value failed for {value}")

def generate_table(groups):
    table_len = 0
    # header
    print("#include \"rtdisasm_table.h\"\n")
    print("const instruction_t rtdisasm_table[] = {")
    # entries
    for group in groups:
        for i in group.instructions:
            opcode = ",".join(["0x{}".format(byte) for byte in i.bytes])
            opcode_len = len(i.bytes)
            print("\t{{ .config = {{ .type = {}, .has_rex = {}, .has_digit = {}, .has_modrm = {}, .has_imm = {}, .has_value = {}, .has_opreg = {} }}, ".format(
                i.get_type().value(), int(i.has_rex()), int(i.has_digit()), int(i.has_modrm()), int(i.has_imm()), int(i.has_value()), int(i.has_opreg())
                ), end = '')
            
            if i.get_type() == InstructionType.STANDARD:
                print(" .std = {{ .rex = {}, .digit = {}, .imm = {}, .value = {} }},".format(
                    convert_rex(i.rex) if i.has_rex() else 0,
                    int(i.digit) if i.has_digit() else 0,
                    convert_imm(i.imm) if i.has_imm() else 0,
                    convert_value(i.value) if i.has_value() else 0
                ), end = '')
            elif i.get_type() == InstructionType.VEX:
                print(" .vex = {{ .lig = {}, .l = {}, .wig = {}, .w = {} }},".format(
                    int(i.lig),
                    i.l,
                    int(i.wig),
                    int(i.w)
                ), end = '')
            elif i.get_type() == InstructionType.EVEX:
                print(" .evex = {{ .lig = {}, .l = {}, .wig = {}, .w = {} }},".format(
                    int(i.lig),
                    i.l,
                    int(i.wig),
                    int(i.w)
                ), end = '')

            print(" .rt_target = {}, .opcode_len = {}, .opcode = {{ {} }}  }},".format(i.encode_rt_target(), opcode_len, opcode))
            table_len += 1
    # footer
    print("}};\n\nconst unsigned rtdisasm_table_len = {};".format(table_len))

if __name__ == "__main__":
    groups = []
    for filename in sys.argv[1:]:
        groups.extend(parse_file(filename))

    if not groups:
        print("no instructions were parsed!", file=sys.stderr)
        sys.exit(1)

    generate_table(groups)
