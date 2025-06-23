import sys

# args = "test_data/test_elf out.txt"
args = sys.argv
if (len(args) < 3):
    print("haven't enough arguments\n")
    exit(-1)

file_handler = open(args[1], 'rb')
out = open(args[2], 'w')

arr = bytearray(file_handler.read())

pointer = 0
label = dict()


def rd(count):
    global pointer
    pointer += count
    return convertToInt([i for i in arr[pointer - count:pointer]])


def read(start, count):
    return convertToInt([i for i in arr[start:start + count]])


def readWord(start):
    word = ""
    while (arr[start] != 0):
        word += chr(arr[start])
        start += 1
    return word


def convertToInt(arr):
    res = 0
    for i in arr[::-1]:
        res = res * (1 << 8) + i
    return res


def convertBitToInt(arr):
    res = 0
    for i in arr[::-1]:
        res = res * 2 + i
    return res


def readBlock(index, length):
    res = []
    for i in range(e_shentsize // 4):
        res.append(read(e_shoff + index * 40 + i * 4, 4))
    return res


# first 52 bytes
e_indent = rd(16)
e_type = rd(2)
e_machine = rd(2)
e_version = rd(4)
e_entry = rd(4)
e_phoff = rd(4)
e_shoff = rd(4)
e_flags = rd(4)
e_ehsize = rd(2)
e_pentsize = rd(2)
e_phnum = rd(2)  # количество записей в таблице заголовков
e_shentsize = rd(2)  # размер записи в таблице заголовков
e_shnum = rd(2)  # Кол-во элементов в таблице заголовков
e_shstrndx = rd(2)  # Сдвиг от начала файла до таблицы заголовков

pointer = e_shoff

headings = [readBlock(i, 10) for i in range(e_shnum)]

# headings[e_shtrndx][4]

parsedHeadings = []

text = 1
symtab = 1

for i in range(1, e_shnum):
    cur = headings[e_shstrndx][4] + headings[i][0]
    name = readWord(cur)

    if name == '.text': text = headings[i]
    if name == ".symtab": symtab = headings[i]

strtab = headings[symtab[6]]


def parseSymtab(index, output):
    blockSize = symtab[9]
    beginIndex = symtab[4] + index * blockSize
    table = [i for i in arr[beginIndex:beginIndex + blockSize]]

    types = {
        0: "NOTYPE",
        1: "OBJECT",
        2: "FUNC",
        3: "SECTION",
        4: "FILE",
        5: "COMMON",
        6: "TLS",
        10: "LOOS",
        12: "HIOS",
        13: "LOPROC",
        15: "HIPROC"
    }

    binds = {
        0: "LOCAL",
        1: "GLOBAL",
        2: "WEAK",
        10: "LOOS",
        12: "HIOS",
        13: "LOPROC",
        15: "HIPROC"
    }

    vises = {
        0: "DEFAULT",
        1: "INTERNAL",
        2: "HIDDEN",
        3: "PROTECTED"
    }

    indexes = {
        0: "UNDEF",
        0xff00: "LORESERVE",
        0xff1f: "HIPROC",
        0xff20: "LOOS",
        0xff3f: "HIOS",
        0xfff1: "ABS",
        0xfff2: "COMMON",
        0xffff: "XINDEX"
    }

    sh_name = convertToInt(table[0:4])
    name = readWord(strtab[4] + sh_name)
    value = convertToInt(table[4:8])
    size = convertToInt(table[8:12])
    info = convertToInt(table[12:13])
    other = convertToInt(table[13:14])
    shndx = convertToInt(table[14:16])

    typ = types[info % 16]
    bind = binds[(info >> 4)]
    vis = vises[other % 4]

    label[value] = name

    if shndx in indexes.keys():
        shndx = indexes[shndx]
    if output:
        out.write('[%4i] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n' % (index, value, size, typ, bind, vis, shndx, name))


for i in range(symtab[5] // symtab[9]):
    parseSymtab(i, False)


def byteToBit(val):
    ls = []
    for i in range(32):
        ls.append(val % 2)
        val //= 2
    return ls


def parseR(table, mem, flag):
    funct7 = convertBitToInt(table[25:32])
    rs2 = convertBitToInt(table[20:25])
    rs1 = convertBitToInt(table[15:20])
    funct3 = convertBitToInt(table[12:15])
    rd = convertBitToInt(table[7:12])
    opcode = convertBitToInt(table[0:7])

    R1 = {
        0b000: "mul",
        0b001: "mulh",
        0b010: "mulhsu",
        0b011: "mulhu",
        0b100: "div",
        0b101: "divu",
        0b110: "rem",
        0b111: "remu"
    }

    R2 = {
        0b000: "add",
        0b001: "sll",
        0b010: "slt",
        0b011: "sltu",
        0b100: "xor",
        0b101: "srl",
        0b110: "or",
        0b111: "and"
    }

    R3 = {
        0b000: "sub",
        0b101: "sra"
    }

    type = ""
    if funct7 == 0b0000001:
        type = R1[funct3]
    elif funct7 == 0b0000000:
        type = R2[funct3]
    elif funct7 == 0b0100000:
        type = R3[funct3]
    else:
        type = "Unsupported operation"

    if flag:
        out.write("   %05x:\t%08x\t%7s\t%s, %s, %s\n" % (mem, convertBitToInt(table), type, parseString(rd), parseString(rs1), parseString(rs2)))


def parseI(table, mem, flag):
    imm = table[20:32] + [table[31] for i in range(12, 32)]
    rs1 = convertBitToInt(table[15:20])
    funct3 = convertBitToInt(table[12:15])
    rd = convertBitToInt(table[7:12])
    opcode = convertBitToInt(table[0:7])


    I1 = {
        0b000: "addi",
        0b010: "slti",
        0b011: "sltiu",
        0b100: "xori",
        0b110: "ori",
        0b111: "andi"
    }

    I2 = {
        0b000: "lb",
        0b001: "lh",
        0b010: "lw",
        0b100: "lbu",
        0b101: "lhu"
    }

    type = ""
    g = False
    if opcode == 0b1100111:
        type = "jalr"
    elif opcode == 0b0000011 and funct3 in I2.keys():
        type = I2[funct3]
    elif opcode == 0b0010011 and funct3 in I1.keys():
        type = I1[funct3]
    else:
        g = True
        imm = table[20:25]
        if funct3 == 0b001:
            type = "slli"
        elif table[25:32] == [0, 0, 0, 0, 0, 0, 0]:
            type = "srli"
        else:
            type = "srai"

    value = convertBitToInt(imm)
    if not g and imm[-1] == 1:
        value = -((value ^ ((1 << 32) - 1)) + 1)

    if flag:
        if opcode == 0b1100111 or opcode == 0b0000011:
            out.write("   %05x:\t%08x\t%7s\t%s, %d(%s)\n" % (
                mem, convertBitToInt(table), type, parseString(rd), value, parseString(rs1)))
        else:
            out.write("   %05x:\t%08x\t%7s\t%s, %s, %s\n" % (
            mem, convertBitToInt(table), type, parseString(rd), parseString(rs1), value))

def parseS(table, mem, flag):
    imm = (table[7:12] + table[25:32] + [table[31] for i in range(12, 32)])
    rs2 = convertBitToInt(table[20:25])
    rs1 = convertBitToInt(table[15:20])
    funct3 = convertBitToInt(table[12:15])
    opcode = convertBitToInt(table[0:7])

    S = {
        0b000: "sb",
        0b001: "sh",
        0b010: "sw"
    }

    value = convertBitToInt(imm)
    if imm[-1] == 1:
        value = -((value ^ ((1 << 32) - 1)) + 1)

    if flag:
        out.write("   %05x:\t%08x\t%7s\t%s, %d(%s)\n" % (mem, convertBitToInt(table), S[funct3], parseString(rs2), value, parseString(rs1)))


def parseB(table, mem, flag):
    imm = [0] + table[8:12] + table[25:31] + [table[7]] + [table[31] for i in range(12, 32)]
    rs2 = convertBitToInt(table[20:25])
    rs1 = convertBitToInt(table[15:20])
    funct3 = convertBitToInt(table[12:15])
    opcode = convertBitToInt(table[0:7])

    B = {
        0b000: "beq",
        0b001: "bne",
        0b100: "blt",
        0b101: "bge",
        0b110: "bltu",
        0b111: "bgeu"
    }

    value = convertBitToInt(imm)
    if imm[-1] == 1:
        value = -((value ^ ((1 << 32) - 1)) + 1)

    addr = mem + value

    if addr not in label.keys():
        global last
        label[addr] = f'L{last}'
        last += 1

    if flag:
        out.write("   %05x:\t%08x\t%7s\t%s, %s, 0x%x, <%s>\n" % (mem, convertBitToInt(table), B[funct3], parseString(rs1), parseString(rs2), addr, label[addr]))


def parseU(table, mem, flag):
    imm = table[12:32] + [table[31] for i in range(12)]
    rd = convertBitToInt(table[7:12])
    opcode = convertBitToInt(table[0:7])

    U = {
        0b0110111: "lui",
        0b0010111: "auipc"
    }

    value = convertBitToInt(imm)


    if flag:
        out.write("   %05x:\t%08x\t%7s\t%s, 0x%x\n" % (mem, convertBitToInt(table), U[opcode], parseString(rd), value))


def parseJ(table, mem, flag):
    imm = [0] + table[21:31] + [table[20]] + table[12:20] + [table[31] for i in range(20, 32)]
    rd = convertBitToInt(table[7:12])
    opcode = convertBitToInt(table[0:7])

    value = convertBitToInt(imm)
    if imm[-1] == 1:
        value = -((value ^ ((1 << 32) - 1)) + 1)
    addr = value + mem
    if addr not in label.keys():
        global last
        label[addr] = f'L{last}'
        last += 1

    if flag:
        out.write("   %05x:\t%08x\t%7s\t%s, 0x%x <%s>\n" % (mem, convertBitToInt(table), "jal", parseString(rd), addr, label[addr]))


def parseString(rd):
    if rd == 0: return "zero"
    if rd == 1: return "ra"
    if rd == 2: return "sp"
    if rd == 3: return "gp"
    if rd == 4: return "tp"
    if rd < 8: return f't{rd - 5}'
    if rd < 10: return f's{rd - 8}'
    if rd < 18: return f'a{rd - 10}'
    if rd < 27: return f's{rd - 16}'
    return f't{rd - 25}'


def parseText(index, flag):
    blockSize = 4
    beginIndex = text[4] + index * blockSize
    table = [i for i in arr[beginIndex:beginIndex + blockSize]]
    table = byteToBit(convertToInt(table))

    opcode = convertBitToInt(table[0:7])

    addr = text[3] + index * blockSize

    if flag and addr in label.keys():
        out.write("\n%08x \t<%s>:\n" % (addr, label[addr]))

    if opcode == 0b0110011:
        parseR(table, addr, flag)
    elif opcode == 0b1100011:
        parseB(table, addr, flag)
    elif opcode == 0b0100011:
        parseS(table, addr, flag)
    elif opcode == 0b1101111:
        parseJ(table, addr, flag)
    elif opcode == 0b0110111 or opcode == 0b0010111:
        parseU(table, addr, flag)
    elif opcode == 0b1100111 or opcode == 0b0000011 or opcode == 0b0010011:
        parseI(table, addr, flag)
    elif opcode == 0b1110011:
        pass
        type = "ebreak"
        if convertBitToInt(table[25:32]) == 0:
            type = "ecall"
        if flag:
            out.write("   %05x:\t%08x\t%7s\n" % (addr, convertBitToInt(table), type))
    elif opcode == 0b0001111 and convertBitToInt(table[12:15]) == 0b000:
        type = "fence"
        if convertBitToInt(table) == 0b10000011001100000000000000001111: type = "fence.tso"
        elif convertBitToInt(table) == 0b00000001000000000000000000001111: type = "pause"
        if flag:
            if type == "fence":
                predecessor = ('i' if table[27] else '') + ('o' if table[26] else '') + ('r' if table[25] else '') + ('w' if table[24] else '')
                successor = ('i' if table[23] else '') + ('o' if table[22] else '') + ('r' if table[21] else '') + ('w' if table[20] else '')
                out.write("   %05x:\t%08x\t%7s\t%s, %s\n" % (addr, convertBitToInt(table), type, predecessor, successor))
            else:
                out.write("   %05x:\t%08x\t%-7s\n" % (addr, convertBitToInt(table), type))
    else:
        if flag:
            out.write("   %05x:\t%08x\t%-7s\n" % (addr, convertBitToInt(table), "Unsupported operation"))


last = 0

for i in range(text[5] // 4):
    parseText(i, False)

out.write(".text\n")
for i in range(text[5] // 4):
    parseText(i, True)

out.write('\n\n')

out.write(".symtab\n")
out.write("\nSymbol Value              Size Type     Bind     Vis       Index Name\n")
for i in range(symtab[5] // symtab[9]):
    parseSymtab(i, True)
